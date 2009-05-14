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


class gsm_receiver_cf;

/*
 * We use boost::shared_ptr's instead of raw pointers for all access
 * to gr_blocks (and many other data structures).  The shared_ptr gets
 * us transparent reference counting, which greatly simplifies storage
 * management issues.  This is especially helpful in our hybrid
 * C++ / Python system.
 *
 * See http://www.boost.org/libs/smart_ptr/smart_ptr.htm
 *
 * As a convention, the _sptr suffix indicates a boost::shared_ptr
 */
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
    gr_complex d_sch_training_seq[N_SYNC_BITS]; //encoded training sequence of a SCH burst

    gr_feval_dd *d_tuner;
    int d_counter;

    //variables used to store result of the find_fcch_burst fuction
    int d_fcch_start_pos;
    float d_freq_offset;
    double d_best_sum;
    
    int d_fcch_count; //!!!
    double d_x_temp, d_x2_temp, d_mean;//!!
    
    vector_complex d_channel_imp_resp;

    enum states {
      first_fcch_search, next_fcch_search, sch_search, read_bcch
    } d_state;

    friend gsm_receiver_cf_sptr gsm_make_receiver_cf(gr_feval_dd *tuner, int osr);
    gsm_receiver_cf(gr_feval_dd *tuner, int osr);

    bool find_fcch_burst(const gr_complex *in, const int nitems);
    double compute_freq_offset();
    void set_frequency(double freq_offset);
    inline float compute_phase_diff(gr_complex val1, gr_complex val2);
    
    bool find_sch_burst(const gr_complex *in, const int nitems , float *out);
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
