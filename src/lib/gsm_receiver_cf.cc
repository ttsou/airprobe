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
#include <math.h>
#include <gsm_constants.h>
#include <Assert.h>
#include <algorithm>

#define SAFETY_MARGIN 50
#define BUFFER_SIZE (FCCH_HITS_NEEDED)

gsm_receiver_cf_sptr
gsm_make_receiver_cf(gr_feval_dd *tuner, int osr)
{
  return gsm_receiver_cf_sptr(new gsm_receiver_cf(tuner, osr));
}

static const int MIN_IN = 1; // mininum number of input streams
static const int MAX_IN = 1; // maximum number of input streams
static const int MIN_OUT = 0; // minimum number of output streams
static const int MAX_OUT = 1; // maximum number of output streams

/*
 * The private constructor
 */
gsm_receiver_cf::gsm_receiver_cf(gr_feval_dd *tuner, int osr)
    : gr_block("gsm_receiver",
               gr_make_io_signature(MIN_IN, MAX_IN, sizeof(gr_complex)),
               gr_make_io_signature(MIN_OUT, MAX_OUT, 142 * sizeof(float))),
    d_osr(osr),
    d_tuner(tuner),
    d_prev_freq_offset(0),
    d_phase_diff_buffer(BUFFER_SIZE),
    d_counter(0),
    d_x_temp(0),
    d_x2_temp(0),
    d_fcch_count(0),
    d_state(fcch_search)
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
  int produced_out;

  switch (d_state) {

    case fcch_search:

      if (find_fcch_burst(in, ninput_items[0])) {
        produced_out = 1;
        d_state = fcch_search;
      } else {
        produced_out = 0;
        d_state = fcch_search;
      }

      break;

    case sch_search:
      break;
  }

//  for (int i = 0; i < TS_BITS; i++) {
//    out[i] = d_phase_diff_buffer[i+start_pos-USEFUL_BITS];
//  }

  return produced_out;
}

bool gsm_receiver_cf::find_fcch_burst(const gr_complex *in, const int nitems)
{
  float phase_diff = 0;
  gr_complex conjprod;
  int hit_count, miss_count, start_pos;
  float min_phase_diff, max_phase_diff, lowest_max_min_diff;
  float sum, best_sum;

  int to_consume = 0;
  int i = 0;
  bool end = false;
  bool result;

  circular_buffer_float::iterator buffer_iter;

  enum states {
    init, search, found_something, fcch_found, search_fail
  } fcch_search_state;

  fcch_search_state = init;

  while ((!end) && (i < nitems)) {
    switch (fcch_search_state) {

      case init:
        hit_count = 0;
        miss_count = 0;
        start_pos = -1;
        lowest_max_min_diff = 99999;
        d_phase_diff_buffer.clear();
        fcch_search_state = search;

        break;

      case search:
        i++;

        if (i > nitems - BUFFER_SIZE) {
          to_consume = i;
          fcch_search_state = search_fail;
        }

        conjprod = in[i] * conj(in[i-1]);

        phase_diff = gr_fast_atan2f(imag(conjprod), real(conjprod));

        if (phase_diff > 0) {
//          start_pos = i - 1;
          to_consume = i;
          fcch_search_state = found_something;
        } else {
          fcch_search_state = search;
        }

        break;

      case found_something:

        if (phase_diff > 0) {
          hit_count++;
        } else {
          miss_count++;
        }

        //DCOUT("d_phase_diff_buffer.size(): " << d_phase_diff_buffer.size() << " hit_count: " << hit_count);
        
        if ((miss_count >= FCCH_MAX_MISSES) && (hit_count <= FCCH_HITS_NEEDED)) {
          fcch_search_state = init;
          continue;
        } else if ((miss_count >= FCCH_MAX_MISSES) && (hit_count > FCCH_HITS_NEEDED)) {
          fcch_search_state = fcch_found;
          continue;
        } else if ((miss_count < FCCH_MAX_MISSES) && (hit_count > FCCH_HITS_NEEDED)) {
          //find difference between minimal and maximal element in the buffer
          //for FCCH this value should be low
          //this part is searching for a region where this value is lowest
          min_phase_diff = *(min_element(d_phase_diff_buffer.begin(), d_phase_diff_buffer.end()));
          max_phase_diff = *(max_element(d_phase_diff_buffer.begin(), d_phase_diff_buffer.end()));
          
          if(lowest_max_min_diff > max_phase_diff - min_phase_diff){
            lowest_max_min_diff = max_phase_diff - min_phase_diff;
            start_pos = i - FCCH_HITS_NEEDED;
            d_best_sum = 0;
            for (buffer_iter = (d_phase_diff_buffer.begin());
                 buffer_iter != (d_phase_diff_buffer.end());
                 buffer_iter++) {
                   d_best_sum += *buffer_iter;
            }
            DCOUT(d_best_sum);
          }
        }

        i++;

        if (i >= nitems) {
          fcch_search_state = search_fail;
          continue;
        }

        conjprod = in[i] * conj(in[i-1]);
        phase_diff = gr_fast_atan2f(imag(conjprod), real(conjprod));
        d_phase_diff_buffer.push_back(phase_diff);

        fcch_search_state = found_something;

        break;

      case fcch_found:
        DCOUT("znalezione fcch na pozycji" << d_counter + start_pos);
        to_consume = start_pos + FCCH_HITS_NEEDED + 1;
        /*  mean = d_best_sum / FCCH_HITS_NEEDED;
        phase_offset = mean - (M_PI / 2);
        freq_offset = phase_offset * 1625000.0 / (12.0 * M_PI);*/
        compute_freq_offset();
        end = true;
        result = true;
        break;

      case search_fail:
        end = true;
        result = false;
        break;
    }
  }

  d_counter += to_consume;
  consume_each(to_consume);

  return result;
}

bool find_sch_burst( const gr_complex *in, const int nitems , gr_complex *out);


double gsm_receiver_cf::compute_freq_offset()
{
  float mean, phase_offset, freq_offset;

  DCOUT(" d_phase_diff_buffer.size(): " <<  d_phase_diff_buffer.size());

  mean = d_best_sum / FCCH_HITS_NEEDED;
  phase_offset = mean - (M_PI / 2);
  freq_offset = phase_offset * 1625000.0 / (12.0 * M_PI);
  DCOUT("freq_offset: " << freq_offset);
  d_fcch_count++;
  d_x_temp += freq_offset;
  d_x2_temp += freq_offset * freq_offset;
  d_mean = d_x_temp / d_fcch_count;
  //d_mean = 0.9 * freq_offset + 0.1 * d_mean;


  //d_tuner->calleval(freq_offset);

  d_prev_freq_offset -= freq_offset;

  DCOUT("wariancja: " << sqrt((d_x2_temp / d_fcch_count - d_mean * d_mean)) << " fcch_count:" << d_fcch_count << " d_mean: " << d_mean);

  return freq_offset;
}

void gsm_receiver_cf::set_frequency()
{

}

bool gsm_receiver_cf::find_sch_burst( const gr_complex *in, const int nitems , gr_complex *out)
{
  
}
