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

#include <gr_io_signature.h>
#include <gr_math.h>
#include <math.h>
#include <Assert.h>
#include <list>
#include <boost/circular_buffer.hpp>
#include <algorithm>
#include <gsm_receiver_cf.h>
#include <viterbi_detector.h>
#include <sch.h>

#define FCCH_BUFFER_SIZE (FCCH_HITS_NEEDED)
#define SYNC_SEARCH_RANGE 30
#define TRAIN_SEARCH_RANGE 40

//TODO !! - move this methods to some else place

// - move it to some else place !!
typedef std::list<float> list_float;
typedef std::vector<float> vector_float;

typedef boost::circular_buffer<float> circular_buffer_float;

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
    d_OSR(osr),
    d_chan_imp_length(CHAN_IMP_RESP_LENGTH),
    d_tuner(tuner),
    d_counter(0),
    d_fcch_start_pos(0),
    d_freq_offset(0),
    d_state(first_fcch_search),
    d_burst_nr(osr)
{
  int i;
  gmsk_mapper(SYNC_BITS, N_SYNC_BITS, d_sch_training_seq, gr_complex(0.0,-1.0));

  for (i = 0; i < TRAIN_SEQ_NUM; i++) {
    gmsk_mapper(train_seq[i], N_TRAIN_BITS, d_norm_training_seq[i], gr_complex(1.0,0.0));
  }
}

/*
 * Virtual destructor.
 */
gsm_receiver_cf::~gsm_receiver_cf()
{
}

void gsm_receiver_cf::forecast(int noutput_items, gr_vector_int &ninput_items_required)
{
  ninput_items_required[0] = noutput_items * (TS_BITS + 2 * SAFETY_MARGIN) * d_OSR;
}

int
gsm_receiver_cf::general_work(int noutput_items,
                              gr_vector_int &ninput_items,
                              gr_vector_const_void_star &input_items,
                              gr_vector_void_star &output_items)
{
  const gr_complex *in = (const gr_complex *) input_items[0];
  float *out = (float *) output_items[0];
  int produced_out = 0;
  float prev_freq_offset;

  switch (d_state) {
      //bootstrapping
    case first_fcch_search:
      if (find_fcch_burst(in, ninput_items[0])) {
        set_frequency(d_freq_offset);
        produced_out = 0;
        d_state = next_fcch_search;
      } else {
        produced_out = 0;
        d_state = first_fcch_search;
      }
      break;

    case next_fcch_search:
      prev_freq_offset = d_freq_offset;
      if (find_fcch_burst(in, ninput_items[0])) {
        if (abs(d_freq_offset) > 100) {
          set_frequency(d_freq_offset);
        }
        produced_out = 0;
        d_state = sch_search;
      } else {
        produced_out = 0;
        d_state = next_fcch_search;
      }
      break;

    case sch_search: {
      gr_complex chan_imp_resp[CHAN_IMP_RESP_LENGTH*d_OSR];
      int t1, t2, t3;
      int burst_start = 0;
      unsigned char output_binary[BURST_SIZE];

      if (find_sch_burst(in, ninput_items[0], out)) {
        burst_start = get_sch_chan_imp_resp(in, chan_imp_resp);
        detect_burst(in, chan_imp_resp, burst_start, output_binary);
        if (decode_sch(&output_binary[3], &t1, &t2, &t3, &d_ncc, &d_bcc) == 0) {
          DCOUT("sch burst_start: " << burst_start);
          d_burst_nr.set(t1, t2, t3, 0);
          DCOUT("bcc: " << d_bcc << " ncc: " << d_ncc << " t1: " << t1 << " t2: " << t2 << " t3: " << t3);
          d_channel_conf.set_multiframe_type(TSC0, multiframe_51);
          d_channel_conf.set_burst_types(TSC0, FCCH_FRAMES, sizeof(FCCH_FRAMES) / sizeof(unsigned), fcch_burst);
          d_channel_conf.set_burst_types(TSC0, SCH_FRAMES, sizeof(SCH_FRAMES) / sizeof(unsigned), sch_burst);
          d_channel_conf.set_burst_types(TSC0, BCCH_FRAMES, sizeof(BCCH_FRAMES) / sizeof(unsigned), normal_burst);
          d_burst_nr++;

          consume_each(burst_start + BURST_SIZE * d_OSR);
          d_state = synchronized;
        } else {
          d_state = next_fcch_search;
        }
      } else {
        d_state = sch_search;
      }
      break;
    }

    //in this state receiver is synchronized and it processes bursts according to burst type for given burst number
    case synchronized: {
      gr_complex chan_imp_resp[d_chan_imp_length*d_OSR];
      burst_type b_type = d_channel_conf.get_burst_type(d_burst_nr);
      int burst_start;
      int offset = 0;
      int to_consume = 0;
      unsigned char output_binary[BURST_SIZE];

      switch (b_type) {
        case fcch_burst: {
          int ii;
          int first_sample = ceil((GUARD_PERIOD + 2 * TAIL_BITS) * d_OSR) + 1;
          int last_sample = first_sample + USEFUL_BITS * d_OSR;
          double phase_sum = 0;
          for (ii = first_sample; ii < last_sample; ii++) {
            double phase_diff = compute_phase_diff(in[ii], in[ii-1]) - (M_PI / 2) / d_OSR;
            phase_sum += phase_diff;
          }
          double freq_offset = compute_freq_offset(phase_sum, last_sample - first_sample);
          if (abs(freq_offset) > FCCH_MAX_FREQ_OFFSET) {
            d_freq_offset -= freq_offset;
            set_frequency(d_freq_offset);
            DCOUT("adjusting frequency, new frequency offset: " << d_freq_offset << "\n");
          }
        }
        break;

        case sch_burst: {
          int t1, t2, t3, d_ncc, d_bcc;
          burst_start = get_sch_chan_imp_resp(in, chan_imp_resp);
          detect_burst(in, &d_channel_imp_resp[0], burst_start, output_binary);
          if (decode_sch(&output_binary[3], &t1, &t2, &t3, &d_ncc, &d_bcc) == 0) {
//                d_burst_nr.set(t1, t2, t3, 0);
            DCOUT("bcc: " << d_bcc << " ncc: " << d_ncc << " t1: " << t1 << " t2: " << t2 << " t3: " << t3);
            offset =  burst_start - floor((GUARD_PERIOD) * d_OSR);
            DCOUT(offset);
            to_consume += offset;
          }
        }
        break;

        case normal_burst:
          burst_start = get_norm_chan_imp_resp(in, chan_imp_resp, TRAIN_SEARCH_RANGE);
          detect_burst(in, &d_channel_imp_resp[0], burst_start, output_binary);
//           printf("burst = [ ");
// 
//           for (int i = 0; i < BURST_SIZE ; i++) {
//             printf(" %d", output_binary[i]);
//           }
//           printf("];\n");

          break;

        case rach_burst:
          break;
        case dummy:
          break;
        case empty:
          break;
      }

      d_burst_nr++;

      to_consume += TS_BITS * d_OSR + d_burst_nr.get_offset();
      consume_each(to_consume);
    }
    break;
  }

  return produced_out;
}

bool gsm_receiver_cf::find_fcch_burst(const gr_complex *in, const int nitems)
{
  circular_buffer_float phase_diff_buffer(FCCH_BUFFER_SIZE * d_OSR);

  float phase_diff = 0;
  gr_complex conjprod;
  int hit_count = 0;
  int miss_count = 0;
  int start_pos = -1;
  float min_phase_diff;
  float max_phase_diff;
  double best_sum;
  float lowest_max_min_diff = 99999;

  int to_consume = 0;
  int sample_number = 0;
  bool end = false;
  bool result = false;
  double freq_offset;
  circular_buffer_float::iterator buffer_iter;

  enum states {
    init, search, found_something, fcch_found, search_fail
  } fcch_search_state;

  fcch_search_state = init;

  while (!end) {
    switch (fcch_search_state) {

      case init:
        hit_count = 0;
        miss_count = 0;
        start_pos = -1;
        lowest_max_min_diff = 99999;
        phase_diff_buffer.clear();
        fcch_search_state = search;

        break;

      case search:
        sample_number++;

        if (sample_number > nitems - FCCH_BUFFER_SIZE * d_OSR) {
          to_consume = sample_number;
          fcch_search_state = search_fail;
        } else {
          phase_diff = compute_phase_diff(in[sample_number], in[sample_number-1]);

          if (phase_diff > 0) {
            to_consume = sample_number;
            fcch_search_state = found_something;
          } else {
            fcch_search_state = search;
          }
        }

        break;

      case found_something:
        if (phase_diff > 0) {
          hit_count++;
        } else {
          miss_count++;
        }

        if ((miss_count >= FCCH_MAX_MISSES * d_OSR) && (hit_count <= FCCH_HITS_NEEDED * d_OSR)) {
          fcch_search_state = init;
          continue;
        } else if (((miss_count >= FCCH_MAX_MISSES * d_OSR) && (hit_count > FCCH_HITS_NEEDED * d_OSR)) || (hit_count > 2 * FCCH_HITS_NEEDED * d_OSR)) {
          fcch_search_state = fcch_found;
          continue;
        } else if ((miss_count < FCCH_MAX_MISSES * d_OSR) && (hit_count > FCCH_HITS_NEEDED * d_OSR)) {
          //find difference between minimal and maximal element in the buffer
          //for FCCH this value should be low
          //this part is searching for a region where this value is lowest
          min_phase_diff = * (min_element(phase_diff_buffer.begin(), phase_diff_buffer.end()));
          max_phase_diff = * (max_element(phase_diff_buffer.begin(), phase_diff_buffer.end()));

          if (lowest_max_min_diff > max_phase_diff - min_phase_diff) {
            lowest_max_min_diff = max_phase_diff - min_phase_diff;
            start_pos = sample_number - FCCH_HITS_NEEDED * d_OSR - FCCH_MAX_MISSES * d_OSR;
            best_sum = 0;

            for (buffer_iter = phase_diff_buffer.begin();
                 buffer_iter != (phase_diff_buffer.end());
                 buffer_iter++) {
              best_sum += *buffer_iter - (M_PI / 2) / d_OSR;
            }
          }
        }

        sample_number++;

        if (sample_number >= nitems) {
          fcch_search_state = search_fail;
          continue;
        }

        phase_diff = compute_phase_diff(in[sample_number], in[sample_number-1]);
        phase_diff_buffer.push_back(phase_diff);
        fcch_search_state = found_something;

        break;

      case fcch_found:
        DCOUT("fcch found on position: " << d_counter + start_pos);
        to_consume = start_pos + FCCH_HITS_NEEDED * d_OSR + 1;

        d_fcch_start_pos = d_counter + start_pos;
        freq_offset = compute_freq_offset(best_sum, FCCH_HITS_NEEDED);
        d_freq_offset -= freq_offset;
        DCOUT("freq_offset: " << d_freq_offset);

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

double gsm_receiver_cf::compute_freq_offset(double best_sum, unsigned denominator)
{
  float phase_offset, freq_offset;
  phase_offset = best_sum / denominator;
  freq_offset = phase_offset * 1625000.0 / (12.0 * M_PI);
  return freq_offset;
}

void gsm_receiver_cf::set_frequency(double freq_offset)
{
  d_tuner->calleval(freq_offset);
}

inline float gsm_receiver_cf::compute_phase_diff(gr_complex val1, gr_complex val2)
{
  gr_complex conjprod = val1 * conj(val2);
  return gr_fast_atan2f(imag(conjprod), real(conjprod));
}

bool gsm_receiver_cf::find_sch_burst(const gr_complex *in, const int nitems , float *out)
{
  int to_consume = 0;
  bool end = false;
  bool result = false;
  int sample_nr_near_sch_start = d_fcch_start_pos + (FRAME_BITS - SAFETY_MARGIN) * d_OSR;

  enum states {
    start, reach_sch, search_not_finished, sch_found
  } sch_search_state;

  sch_search_state = start;

  while (!end) {
    switch (sch_search_state) {

      case start:
        if (d_counter < sample_nr_near_sch_start) {
          sch_search_state = reach_sch;
        } else {
          sch_search_state = sch_found;
        }
        break;

      case reach_sch:
        if (d_counter + nitems >= sample_nr_near_sch_start) {
          to_consume = sample_nr_near_sch_start - d_counter;
        } else {
          to_consume = nitems;
        }
        sch_search_state = search_not_finished;
        break;

      case search_not_finished:
        result = false;
        end = true;
        break;

      case sch_found:
        to_consume = 0;
        result = true;
        end = true;
        break;
    }
  }

  d_counter += to_consume;
  consume_each(to_consume);
  return result;
}

int gsm_receiver_cf::get_sch_chan_imp_resp(const gr_complex *in, gr_complex * chan_imp_resp)
{
  vector_complex correlation_buffer;
  vector_float power_buffer;
  vector_float window_energy_buffer;

  int strongest_window_nr;
  int burst_start = 0;
  int chan_imp_resp_center;
  float max_correlation = 0;
  float energy = 0;

  for (int ii = SYNC_POS * d_OSR; ii < (SYNC_POS + SYNC_SEARCH_RANGE) *d_OSR; ii++) {
    gr_complex correlation = correlate_sequence(&d_sch_training_seq[5], &in[ii], N_SYNC_BITS - 10);
    correlation_buffer.push_back(correlation);
    power_buffer.push_back(pow(abs(correlation), 2));
  }

  //compute window energies
  vector_float::iterator iter = power_buffer.begin();
  bool loop_end = false;
  while (iter != power_buffer.end()) {
    vector_float::iterator iter_ii = iter;
    energy = 0;

    for (int ii = 0; ii < (d_chan_imp_length) *d_OSR; ii++, iter_ii++) {
      if (iter_ii == power_buffer.end()) {
        loop_end = true;
        break;
      }
      energy += (*iter_ii);
    }
    if (loop_end) {
      break;
    }
    iter++;
    window_energy_buffer.push_back(energy);
  }

  strongest_window_nr = max_element(window_energy_buffer.begin(), window_energy_buffer.end()) - window_energy_buffer.begin();
  d_channel_imp_resp.clear();

  max_correlation = 0;
  for (int ii = 0; ii < (d_chan_imp_length) *d_OSR; ii++) {
    gr_complex correlation = correlation_buffer[strongest_window_nr + ii];
    if (abs(correlation) > max_correlation) {
      chan_imp_resp_center = ii;
      max_correlation = abs(correlation);
    }
    d_channel_imp_resp.push_back(correlation);
    chan_imp_resp[ii] = correlation;
  }

  burst_start = strongest_window_nr + chan_imp_resp_center - 48 * d_OSR - 2 * d_OSR + 2 + SYNC_POS * d_OSR;
  return burst_start;
}

void gsm_receiver_cf::detect_burst(const gr_complex * in, gr_complex * chan_imp_resp, int burst_start, unsigned char * output_binary)
{
  float output[BURST_SIZE];
  gr_complex rhh_temp[CHAN_IMP_RESP_LENGTH*d_OSR];
  gr_complex rhh[CHAN_IMP_RESP_LENGTH];
  gr_complex filtered_burst[BURST_SIZE];
  int start_state = 3;
  unsigned int stop_states[2] = {4, 12};

  autocorrelation(chan_imp_resp, rhh_temp, d_chan_imp_length*d_OSR);
  for (int ii = 0; ii < (d_chan_imp_length); ii++) {
    rhh[ii] = conj(rhh_temp[ii*d_OSR]);
  }

  mafi(&in[burst_start], BURST_SIZE, chan_imp_resp, d_chan_imp_length*d_OSR, filtered_burst);

  viterbi_detector(filtered_burst, BURST_SIZE, rhh, start_state, stop_states, 2, output);

  for (int i = 0; i < BURST_SIZE ; i++) {
    output_binary[i] = (output[i] > 0);
  }
}

//TODO consider placing this funtion in a separate class for signal processing
void gsm_receiver_cf::gmsk_mapper(const unsigned char * input, int ninput, gr_complex * gmsk_output, gr_complex start_point)
{
  gr_complex j = gr_complex(0.0, 1.0);

  int current_symbol;
  int encoded_symbol;
  int previous_symbol = 2 * input[0] - 1;
  gmsk_output[0] = start_point;

  for (int i = 1; i < ninput; i++) {
    //change bits representation to NRZ
    current_symbol = 2 * input[i] - 1;
    //differentially encode
    encoded_symbol = current_symbol * previous_symbol;
    //and do gmsk mapping
    gmsk_output[i] = j * gr_complex(encoded_symbol, 0.0) * gmsk_output[i-1];
    previous_symbol = current_symbol;
  }
}

//TODO consider use of some generalized function for correlation and placing it in a separate class  for signal processing
gr_complex gsm_receiver_cf::correlate_sequence(const gr_complex * sequence, const gr_complex * input_signal, int length)
{
  gr_complex result(0.0, 0.0);
  int sample_number = 0;

  for (int ii = 0; ii < length; ii++) {
    sample_number = (ii * d_OSR) ;
    result += sequence[ii] * conj(input_signal[sample_number]);
  }

  result = result / gr_complex(length, 0);
  return result;
}

//computes autocorrelation for positive values
//TODO consider placing this funtion in a separate class for signal processing
inline void gsm_receiver_cf::autocorrelation(const gr_complex * input, gr_complex * out, int length)
{
  int i, k;
  for (k = length - 1; k >= 0; k--) {
    out[k] = gr_complex(0, 0);
    for (i = k; i < length; i++) {
      out[k] += input[i] * conj(input[i-k]);
    }
  }
}

//TODO consider use of some generalized function for filtering and placing it in a separate class  for signal processing
//funkcja matched filter
inline void gsm_receiver_cf::mafi(const gr_complex * input, int input_length, gr_complex * filter, int filter_length, gr_complex * output)
{
  int ii = 0, n, a;

  for (n = 0; n < input_length; n++) {
    a = n * d_OSR;
    output[n] = 0;
    ii = 0;

    while (ii < filter_length) {
      if ((a + ii) >= input_length*d_OSR)
        break;
      output[n] += input[a+ii] * filter[ii];
      ii++;
    }
  }
}

int gsm_receiver_cf::get_norm_chan_imp_resp(const gr_complex *in, gr_complex * chan_imp_resp, unsigned search_range)
{
  vector_complex correlation_buffer;
  vector_float power_buffer;
  vector_float window_energy_buffer;

  int strongest_window_nr;
  int burst_start = 0;
  int chan_imp_resp_center;
  float max_correlation = 0;
  float energy = 0;
  
  int search_start_pos = floor((TRAIN_POS + GUARD_PERIOD) * d_OSR);
  int search_stop_pos = search_start_pos + search_range * d_OSR;

  for (int ii = search_start_pos; ii < search_stop_pos; ii++) {
    gr_complex correlation = correlate_sequence(&d_norm_training_seq[d_bcc][5], &in[ii], N_TRAIN_BITS - 10);

    correlation_buffer.push_back(correlation);
    power_buffer.push_back(pow(abs(correlation), 2));
  }

  //compute window energies
  vector_float::iterator iter = power_buffer.begin();
  bool loop_end = false;
  while (iter != power_buffer.end()) {
    vector_float::iterator iter_ii = iter;
    energy = 0;

    for (int ii = 0; ii < (d_chan_imp_length)*d_OSR; ii++, iter_ii++) {
      if (iter_ii == power_buffer.end()) {
        loop_end = true;
        break;
      }
      energy += (*iter_ii);
    }
    if (loop_end) {
      break;
    }
    iter++;
    window_energy_buffer.push_back(energy);
  }
  


  strongest_window_nr = max_element(window_energy_buffer.begin(), window_energy_buffer.end()) - window_energy_buffer.begin();
  d_channel_imp_resp.clear();
  strongest_window_nr = 36;

  max_correlation = 0;
  for (int ii = 0; ii < (d_chan_imp_length)*d_OSR; ii++) {
    gr_complex correlation = correlation_buffer[strongest_window_nr + ii];
    if (abs(correlation) > max_correlation) {
      chan_imp_resp_center = ii;
      max_correlation = abs(correlation);
    }
    d_channel_imp_resp.push_back(correlation);
    chan_imp_resp[ii] = correlation;
  }

  std::cout << "center: " << strongest_window_nr + chan_imp_resp_center  << " stronegest window nr: " <<  strongest_window_nr << "\n";
  
  burst_start = search_start_pos + strongest_window_nr + chan_imp_resp_center - 66 * d_OSR - 2 * d_OSR + 2;
  return burst_start;
}
