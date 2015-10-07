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

#ifndef INCLUDED_DIGITAL_C4FM_DEMOD_CF_H
#define INCLUDED_DIGITAL_C4FM_DEMOD_CF_H

#include <gnuradio/digital/api.h>
#include <gnuradio/hier_block2.h>

namespace gr {
  namespace digital {

    /*!
     * \brief C4FM Demodulator
     * \ingroup modulators
     */
    class DIGITAL_API c4fm_demod_cf : virtual public hier_block2
    {
    public:

      // gr::digital::c4fm_demod_cf::sptr
      typedef boost::shared_ptr<c4fm_demod_cf> sptr;

      /*! \brief Create an instance of a C4FM Demodulator
       */
      static sptr make(float sample_rate,
                       float symbol_rate,
                       float max_deviation,
                       float rrc_rolloff,
                       float loop_gain,
                       float max_timing_offset,
                       bool  use_chan_filt=true,
                       bool  use_freq_xlat=false,
                       float offset_freq=0.0,
                       float min_sps=0.0);
    };

  } /* namespace digital */
} /* namespace gr */

#endif /* INCLUDED_DIGITAL_C4FM_DEMOD_CF_H */
