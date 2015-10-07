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

#ifndef INCLUDED_DIGITAL_C4FM_DEMOD_CF_IMPL_H
#define INCLUDED_DIGITAL_C4FM_DEMOD_CF_IMPL_H

#include <gnuradio/digital/c4fm_demod_cf.h>

namespace gr {
  namespace digital {

    class DIGITAL_API c4fm_demod_cf_impl : public c4fm_demod_cf
    {
    private:
      // Configuration parameters
      float d_sample_rate;
      float d_symbol_rate;
      float d_max_deviation;
      float d_loop_gain;
      float d_rrc_rolloff;
      float d_max_timing_offset;
      bool  d_use_chan_filter;
      bool  d_use_freq_xlat;
      float d_offset_freq;
      float d_min_sps;

      // Derived parameters
      std::vector<float> d_chan_taps;
      std::vector<float> d_rrc_taps;
      int   d_chan_decim;
      float d_chan_rate;
      float d_sps;

      // Internal blocks
      gr::basic_block_sptr d_chan_filt;
      gr::basic_block_sptr d_quad_demod;
      gr::basic_block_sptr d_pfb_clock_sync;

    public:
      c4fm_demod_cf_impl(float sample_rate,
                         float symbol_rate,
                         float max_deviation,
                         float loop_gain,
                         float rrc_rolloff,
                         float max_timing_offset,
                         bool  use_chan_filt,
                         bool  use_freq_xlat,
                         float offset_freq,
                         float min_sps);

      ~c4fm_demod_cf_impl();
    };

  } /* namespace digital */
} /* namespace gr */

#endif /* INCLUDED_DIGITAL_C4FM_DEMOD_CF_IMPL_H */
