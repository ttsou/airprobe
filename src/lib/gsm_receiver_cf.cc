/* -*- c++ -*- */
/*
 * Copyright 2004 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gsm_receiver_cf.h>
#include <gr_io_signature.h>
#include <gr_math.h>
#include <gsm_constants.h>
#include <Assert.h>


gsm_receiver_cf_sptr
gsm_make_receiver_cf(int osr)
{
  return gsm_receiver_cf_sptr(new gsm_receiver_cf(osr));
}

static const int MIN_IN = 1; // mininum number of input streams
static const int MAX_IN = 1; // maximum number of input streams
static const int MIN_OUT = 0; // minimum number of output streams
static const int MAX_OUT = 1; // maximum number of output streams

/*
 * The private constructor
 */
gsm_receiver_cf::gsm_receiver_cf(int osr)
    : gr_block("gsm_receiver",
               gr_make_io_signature(MIN_IN, MAX_IN, sizeof(gr_complex)),
               gr_make_io_signature(MIN_OUT, MAX_OUT, 142 * sizeof(float))),
    d_counter(0)
{
}

/*
 * Virtual destructor.
 */
gsm_receiver_cf::~gsm_receiver_cf()
{
}

void gsm_receiver_cf::forecast(int noutput_items, gr_vector_int &ninput_items_required)
{
  ninput_items_required[0] = noutput_items * TS_BITS; //TODO include oversampling ratio here
}

int
gsm_receiver_cf::general_work(int noutput_items,
                              gr_vector_int &ninput_items,
                              gr_vector_const_void_star &input_items,
                              gr_vector_void_star &output_items)
{
  const gr_complex *in = (const gr_complex *) input_items[0];
  float *out = (float *) output_items[0];

//  DCOUT(ninput_items[0]);
//  DCOUT(noutput_items);


  d_return = 0; //!
  int start_pos = find_fcch_burst(in, ninput_items[0]);
  for (int i = 0; i < TS_BITS; i++) {
    out[i] = d_phase_diff_buffer[i+start_pos-USEFUL_BITS];
  }
  consume_each(start_pos);
  d_counter += start_pos;
  return d_return;
}

int gsm_receiver_cf::find_fcch_burst(const gr_complex *in, const int nitems)
{
  for (int i = 0; (i < nitems - 1) && (i < BUFFER_SIZE); i++) {
    gr_complex conjprod = in[i+1] * conj(in[i]);
    float diff_angle = gr_fast_atan2f(imag(conjprod), real(conjprod));
    d_phase_diff_buffer[i] = diff_angle;
  }

  int hit_count = 0;
  int miss_count = 0;
  int start_pos = -1;

  d_return = 0;//!
  
  for (int i = 0; ; i++) {
    if (i > nitems - USEFUL_BITS - 1) {
      start_pos = nitems - USEFUL_BITS - 1;
      break;
    }

    if (d_phase_diff_buffer[i] > 0) {
      if (! hit_count) {
        start_pos = i+USEFUL_BITS;
      }
      hit_count++;
    } else if (hit_count >= FCCH_HITS_NEEDED) {
      DCOUT("znalezione fcch na pozycji" << d_counter+start_pos-USEFUL_BITS);
      d_return = 1;//!
      break;
    } else if (++miss_count > FCCH_MAX_MISSES) {
      start_pos = -1;
      hit_count = miss_count = 0;
    }
  }

  /*
    //Do we have a match?
    if (start_pos >= 0) {
      //Is it within range? (we know it's long enough then too)
      if (start_pos < 2*MAX_CORR_DIST) {
        d_burst_start = start_pos;
        d_bbuf_pos = 0; //load buffer from start
        return FCCH;
      } else {
        //TODO: don't shift a tiny amount
        shift_burst(start_pos - MAX_CORR_DIST);
      }
    } else {
      //Didn't find anything
      d_burst_start = MAX_CORR_DIST;
      d_bbuf_pos = 0; //load buffer from start
    }
  */
  //DCOUT("start_pos: " << start_pos);

  return start_pos;
}

