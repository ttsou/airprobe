/* -*- c++ -*- */
/*
 * @file
 * @author Piotr Krysik <pkrysik@stud.elka.pw.edu.pl>
 * @section LICENSE 
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

#include <vector>
#include <gr_block.h>
#include <gr_complex.h>
#include <gr_feval.h>
#include <gsm_constants.h>
#include <gsm_receiver_config.h>

class gsm_receiver_cf;

typedef boost::shared_ptr<gsm_receiver_cf> gsm_receiver_cf_sptr;
typedef std::vector<gr_complex> vector_complex;

gsm_receiver_cf_sptr gsm_make_receiver_cf(gr_feval_dd *tuner, int osr);

/** GSM Receiver GNU Radio block
 * GSM Receiver class supports frequency correction, synchronisation and
 * MLSE (Maximum Likelihood Sequence Estimation) estimation of synchronisation 
 * bursts and normal bursts.
 * \ingroup block
 */

class gsm_receiver_cf : public gr_block
{
  private:
    
    const int d_OSR;
    const int d_chan_imp_length;

    gr_complex d_sch_training_seq[N_SYNC_BITS]; ///<encoded training sequence of a SCH burst
    gr_complex d_norm_training_seq[TRAIN_SEQ_NUM][N_TRAIN_BITS]; ///<encoded training sequences of a normal bursts and dummy bursts

    gr_feval_dd *d_tuner; ///<callback to a python object which is used for frequency tunning
    unsigned d_samples_counter; ///<samples counter - this is used in beetween find_fcch_burst and find_sch_burst

    //variables used to store result of the find_fcch_burst fuction
//     unsigned d_fcch_start_pos; 
    float d_freq_offset;

    burst_counter d_burst_nr;
    channel_configuration d_channel_conf;

    vector_complex d_channel_imp_resp;

    int d_ncc;
    int d_bcc;

    enum states {
      first_fcch_search, next_fcch_search, sch_search, //synchronization search part
      synchronized //receiver is synchronized in this state
    } d_state;

    friend gsm_receiver_cf_sptr gsm_make_receiver_cf(gr_feval_dd *tuner, int osr);
    gsm_receiver_cf(gr_feval_dd *tuner, int osr);

    
    /**
     * 
     * @param in 
     * @param nitems 
     * @return 
     */
    bool find_fcch_burst(const gr_complex *in, const int nitems);
    
    /**
     * 
     * @param best_sum 
     * @param denominator 
     * @return 
     */
    double compute_freq_offset(double best_sum, unsigned denominator);
    
    /**
     * 
     * @param freq_offset 
     */
    void set_frequency(double freq_offset);
    
    /**
     * 
     * @param val1 
     * @param val2 
     * @return 
     */
    inline float compute_phase_diff(gr_complex val1, gr_complex val2);

    /**
     * 
     * @param in 
     * @param nitems 
     * @param out 
     * @return 
     */
    bool find_sch_burst(const gr_complex *in, const int nitems , float *out);
    
    /**
     * 
     * @param in 
     * @param chan_imp_resp 
     * @return 
     */
    int get_sch_chan_imp_resp(const gr_complex *in, gr_complex * chan_imp_resp);
    
    /**
     * 
     * @param in 
     * @param chan_imp_resp 
     * @param burst_start 
     * @param output_binary 
     */
    void detect_burst(const gr_complex * in, gr_complex * chan_imp_resp, int burst_start, unsigned char * output_binary);
    
    /**
     * 
     * @param input 
     * @param ninput 
     * @param gmsk_output 
     * @param start_point 
     */
    void gmsk_mapper(const unsigned char * input, int ninput, gr_complex * gmsk_output, gr_complex start_point);
    
    /**
     * 
     * @param sequence 
     * @param input_signal 
     * @param ninput 
     * @return 
     */
    gr_complex correlate_sequence(const gr_complex * sequence, const gr_complex * input_signal, int ninput);
    
    /**
     * 
     * @param input 
     * @param out 
     * @param length 
     */
    inline void autocorrelation(const gr_complex * input, gr_complex * out, int length);
    
    /**
     * 
     * @param input 
     * @param input_length 
     * @param filter 
     * @param filter_length 
     * @param output 
     */
    inline void mafi(const gr_complex * input, int input_length, gr_complex * filter, int filter_length, gr_complex * output);
    
    /**
     * 
     * @param in 
     * @param chan_imp_resp 
     * @param search_range 
     * @param bcc 
     * @return 
     */
    int get_norm_chan_imp_resp(const gr_complex *in, gr_complex * chan_imp_resp, unsigned search_range, int bcc);
    
    /**
     * 
     * @param burst_nr 
     * @param pakiet 
     */
    void przetwarzaj_normalny_pakiet(burst_counter burst_nr, unsigned char * pakiet);
    
    /**
     * 
     */
    void konfiguruj_odbiornik();

  public:
    ~gsm_receiver_cf();
    void forecast(int noutput_items, gr_vector_int &ninput_items_required);
    int general_work(int noutput_items,
                     gr_vector_int &ninput_items,
                     gr_vector_const_void_star &input_items,
                     gr_vector_void_star &output_items);
};

#endif /* INCLUDED_GSM_RECEIVER_CF_H */
