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
#ifndef INCLUDED_GSM_RECEIVER_CF_H
#define INCLUDED_GSM_RECEIVER_CF_H

#include <gr_block.h>
#include <gr_complex.h>
#include <gr_feval.h>
#include <gsm_constants.h>
#include <vector>


//TODO !! - move this classes to some other place
#include <vector>
#include <algorithm>
typedef enum {empty, fcch_burst, sch_burst, normal_burst, rach_burst, dummy} burst_type;
typedef enum {unknown, multiframe_26, multiframe_51} multiframe_type;

class multiframe_configuration
{
  private:
    multiframe_type d_type;
    std::vector<burst_type> d_burst_types;
  public:
    multiframe_configuration() {
      d_type = unknown;
      fill(d_burst_types.begin(), d_burst_types.end(), empty);
    }

//     multiframe_configuration(multiframe_type type, const std::vector<burst_type> & burst_types):
//         d_type(type) {
//       d_burst_types.resize(burst_types.size());
//       copy(burst_types.begin(), burst_types.end(), d_burst_types.begin());
//     }

//     multiframe_configuration(multiframe_configuration & conf) {
//       d_type = conf.d_type;
//       copy(conf.d_burst_types.begin(), conf.d_burst_types.end(), d_burst_types.begin());
//     }

    ~multiframe_configuration() {}

    void set_type(multiframe_type type) {
      if (type == multiframe_26) {
        d_burst_types.resize(26);
      } else {
        d_burst_types.resize(51);
      }

      d_type = type;
    }

    void set_burst_type(int nr, burst_type type) {
      d_burst_types[nr] = type;
    }

    multiframe_type get_type() {
      return d_type;
    }

    burst_type get_burst_type(int nr) {
      return d_burst_types[nr];
    }
};

class burst_counter
{
  private:
    uint32_t d_t1, d_t2, d_t3, d_timeslot_nr;
  public:
    burst_counter():
        d_t1(0),
        d_t2(0),
        d_t3(0),
        d_timeslot_nr(0) {
    }

    burst_counter(uint32_t t1, uint32_t t2, uint32_t t3, uint32_t timeslot_nr):
        d_t1(t1),
        d_t2(t2),
        d_t3(t3),
        d_timeslot_nr(timeslot_nr) {
    }

    burst_counter & operator++(int) {
      d_timeslot_nr++;
      if (d_timeslot_nr == TS_PER_FRAME) {
        d_timeslot_nr = 0;

        if ((d_t2 == 25) && (d_t3 == 50)) {
          d_t1 = (d_t1 + 1) % (1 << 11);
        }

        d_t2 = (d_t2 + 1) % 26;
        d_t3 = (d_t3 + 1) % 51;
      }
      return (*this);
    }

    void set(uint32_t t1, uint32_t t2, uint32_t t3, uint32_t timeslot_nr) {
      d_t1 = t1;
      d_t2 = t2;
      d_t3 = t3;
      d_timeslot_nr = timeslot_nr;
    }

    uint32_t get_t1() {
      return d_t1;
    }

    uint32_t get_t2() {
      return d_t2;
    }

    uint32_t get_t3() {
      return d_t3;
    }

    uint32_t get_timeslot_nr() {
      return d_timeslot_nr;
    }

    uint32_t get_frame_nr() {
      return   (51 * 26 * d_t1) + (51 * (((d_t3 + 26) - d_t2) % 26)) + d_t3;
    }
};

class channel_configuration
{
  private:
    multiframe_configuration d_timeslots_descriptions[TS_PER_FRAME];
  public:
    channel_configuration() {
      for (int i = 0; i < TS_PER_FRAME; i++) {
        d_timeslots_descriptions[i].set_type(unknown);
      }
    }
//     void set_timeslot_desc(int timeslot_nr, multiframe_configuration conf){
//       d_timeslots_descriptions[timeslot_nr] = conf;
//     }
    void set_multiframe_type(int timeslot_nr, multiframe_type type) {
      d_timeslots_descriptions[timeslot_nr].set_type(type);
    }

    void set_burst_types(int timeslot_nr, const unsigned mapping[], unsigned mapping_size, burst_type b_type) {
      unsigned i;
      for (i = 0; i < mapping_size; i++) {
        d_timeslots_descriptions[timeslot_nr].set_burst_type(mapping[i], b_type);
      }
    }

    void set_single_burst_type(int timeslot_nr, int burst_nr, burst_type b_type) {
      d_timeslots_descriptions[timeslot_nr].set_burst_type(burst_nr, b_type);
    }

    burst_type get_burst_type(burst_counter burst_nr);
};

burst_type channel_configuration::get_burst_type(burst_counter burst_nr)
{
  uint32_t timeslot_nr = burst_nr.get_timeslot_nr();
  multiframe_type m_type = d_timeslots_descriptions[timeslot_nr].get_type();
  uint32_t nr;
  
  switch (m_type) {
    case multiframe_26:
      nr = burst_nr.get_t2();
      break;
    case multiframe_51:
      nr = burst_nr.get_t3();
      break;
    default:
      nr = 0;
      break;
  }

  return d_timeslots_descriptions[timeslot_nr].get_burst_type(nr);
}
// move it to some other place !!

class gsm_receiver_cf;

typedef boost::shared_ptr<gsm_receiver_cf> gsm_receiver_cf_sptr;
typedef std::vector<gr_complex> vector_complex;

/*!
 * \brief Return a shared_ptr to a new instance of gsm_receiver_cf.
 *
 * To avoid accidental use of raw pointers, gsm_receiver_cf's
 * constructor is private.  howto_make_square_ff is the public
 * interface for creating new instances.
 */
gsm_receiver_cf_sptr gsm_make_receiver_cf(gr_feval_dd *tuner, int osr);

/*!
 * \brief Receives fcch
 * \ingroup block
 * \sa
 */

class gsm_receiver_cf : public gr_block
{

  private:
    const int d_OSR;
    const int d_chan_imp_length;

    gr_complex d_sch_training_seq[N_SYNC_BITS]; //encoded training sequence of a SCH burst

    gr_feval_dd *d_tuner;
    int d_counter;

    //variables used to store result of the find_fcch_burst fuction
    int d_fcch_start_pos;
    float d_freq_offset;
    double d_best_sum;

//    int d_fcch_count; //!!!
//    double d_x_temp, d_x2_temp, d_mean;//!!

    burst_counter d_burst_nr;
    channel_configuration d_channel_conf;

    vector_complex d_channel_imp_resp;

    int d_ncc;
    int d_bcc;

    enum states {
      //synchronization search part
      first_fcch_search, next_fcch_search, sch_search, synchronized
      //
    } d_state;

    friend gsm_receiver_cf_sptr gsm_make_receiver_cf(gr_feval_dd *tuner, int osr);
    gsm_receiver_cf(gr_feval_dd *tuner, int osr);

    bool find_fcch_burst(const gr_complex *in, const int nitems);
    double compute_freq_offset();
    void set_frequency(double freq_offset);
    inline float compute_phase_diff(gr_complex val1, gr_complex val2);

    bool find_sch_burst(const gr_complex *in, const int nitems , float *out);
    int get_sch_chan_imp_resp(const gr_complex *in, gr_complex * chan_imp_resp);
    void detect_burst(const gr_complex * in, gr_complex * chan_imp_resp, int burst_start, unsigned char * output_binary);
    void gmsk_mapper(const int * input, gr_complex * gmsk_output, int ninput);
    gr_complex correlate_sequence(const gr_complex * sequence, const gr_complex * input_signal, int ninput);
    inline void autocorrelation(const gr_complex * input, gr_complex * out, int length);
    inline void mafi(const gr_complex * input, int input_length, gr_complex * filter, int filter_length, gr_complex * output);

  public:
    ~gsm_receiver_cf();
    void forecast(int noutput_items, gr_vector_int &ninput_items_required);
    int general_work(int noutput_items,
                     gr_vector_int &ninput_items,
                     gr_vector_const_void_star &input_items,
                     gr_vector_void_star &output_items);
};

#endif /* INCLUDED_GSM_RECEIVER_CF_H */
