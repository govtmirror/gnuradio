/* -*- c++ -*- */
/*
 * Copyright 2015 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "c4fm_demod_cf_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/digital/pfb_clock_sync_fff.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/freq_xlating_fir_filter_ccf.h>
#include <gnuradio/filter/fir_filter_ccf.h>
#include <iostream>

#define _USE_MATH_DEFINES
#include <cmath>

namespace gr {
  namespace digital {

    c4fm_demod_cf::sptr
    c4fm_demod_cf::make(float sample_rate,
                        float symbol_rate,
                        float max_deviation,
                        float loop_gain,
                        float rrc_rolloff,
                        float max_timing_offset,
                        bool  use_chan_filt,
                        bool  use_freq_xlat,
                        float offset_freq,
                        float min_sps)
    {
      return gnuradio::get_initial_sptr
	(new c4fm_demod_cf_impl(sample_rate, symbol_rate, max_deviation,
                                loop_gain, rrc_rolloff, max_timing_offset,
                                use_chan_filt, use_freq_xlat, offset_freq,
                                min_sps));
    }

    // Hierarchical block constructor
    c4fm_demod_cf_impl::c4fm_demod_cf_impl(float sample_rate,
                                           float symbol_rate,
                                           float max_deviation,
                                           float loop_gain,
                                           float rrc_rolloff,
                                           float max_timing_offset,
                                           bool  use_chan_filt,
                                           bool  use_freq_xlat,
                                           float offset_freq,
                                           float min_sps)
      : hier_block2("c4fm_demod_cf",
                    io_signature::make(1, 1, sizeof(gr_complex)),
                    io_signature::make(1, 1, sizeof(float)))
    {
      d_sample_rate = sample_rate;
      d_symbol_rate = symbol_rate;
      d_max_deviation = max_deviation;
      d_loop_gain = loop_gain;
      d_rrc_rolloff = rrc_rolloff;
      d_max_timing_offset = max_timing_offset;
      d_offset_freq = offset_freq;
      d_min_sps = min_sps;

      if (use_chan_filt) {
        // Construct a set of channel filter taps to be used either in a
        // frequency translating or stand-alone FIR filter. Using
        // Carson's rule, ~98% of the channel energy will be contained
        // within twice the sum of the maximum deviation and symbol
        // rate. In addition, allow for 1/2 symbol deviation extra to
        // accomodate tuning offset. Finally, set the corner frequency
        // such that the transition band *starts* at the required
        // frequency, and is equal to 10% of passband.

        float carson_bw = 2*(symbol_rate + max_deviation);
        float extra_bw = max_deviation/6.0;
        float total_bw = carson_bw + extra_bw;
        float transition = total_bw*0.10;                          // 10% transition
        float corner_freq = total_bw/2.0 + transition/2.0;         // one-sided bandwidth plus half-transition

        d_chan_taps = gr::filter::firdes::low_pass(1.0,
          sample_rate, corner_freq, transition);

        // Calculate maximum channel decimation that results in no
        // aliasing of passband and transition band, and also a
        // resulting sps greater than requested minimum.
        float bw_rate = total_bw+2*transition;
        float needed_rate = bw_rate;
        if (d_min_sps > 0) {
          float min_sps_rate = d_symbol_rate*d_min_sps;
          needed_rate = std::max(needed_rate, min_sps_rate);
        }

        d_chan_decim = int(sample_rate/needed_rate);
        if (d_chan_decim == 0)
          throw std::runtime_error("Unable to achieve required sps");

        // If requested, use a frequency translating FIR filter to
        // downconvert from offset_freq to baseband before channel
        // filtering.  Otherwise, create dedicated FIR filter to
        // accomplish filtering.
        if (use_freq_xlat) {
          d_chan_filt = gr::filter::freq_xlating_fir_filter_ccf::make(
            d_chan_decim, d_chan_taps, offset_freq, sample_rate);
        }
        else {
          d_chan_filt = gr::filter::fir_filter_ccf::make(
            d_chan_decim, d_chan_taps);
        }

        // Connect channel filter to input
        connect(self(), 0, d_chan_filt, 0);
      }
      else {
        // Input is provided already filtered and at desired sample
        // rate.
        d_chan_taps = std::vector<float>(0);
        d_chan_decim = 1;
      }

      // Sample rate and sps after decimation
      d_chan_rate = sample_rate/d_chan_decim;
      d_sps = d_chan_rate/symbol_rate;

      // Create an FM demodulator that outputs symbols -3 to +3 for
      // maximum deviation.
      float gain = 3.0*(d_chan_rate/(2.0*M_PI*d_max_deviation));
      d_quad_demod = gr::analog::quadrature_demod_cf::make(gain);

      if (use_chan_filt)
        connect(d_chan_filt, 0, d_quad_demod, 0);
      else
        connect(self(), 0, d_quad_demod, 0);

      // Calculate a root raised-cosine matched filter tap set
      // appropriate for use with the polyphase clock sync module.
      const int nfilts = 32;
      const int nsyms = 11;
      int ntaps = int(nsyms*d_sps*nfilts);

      d_rrc_taps = gr::filter::firdes::root_raised_cosine(
        nfilts, d_chan_rate*nfilts, d_symbol_rate, d_rrc_rolloff, ntaps);

      // Create a polyphase clock sync block with RRC tap set and
      // configured parameters.
      d_pfb_clock_sync = gr::digital::pfb_clock_sync_fff::make(
        d_sps, d_loop_gain, d_rrc_taps, nfilts, 0.5, d_max_timing_offset, 1);

      connect(d_quad_demod, 0, d_pfb_clock_sync, 0);
      connect(d_pfb_clock_sync, 0, self(), 0);
    }

    c4fm_demod_cf_impl::~c4fm_demod_cf_impl()
    {
    }

  } /* namespace digital */
} /* namespace gr */
