#ifndef INCLUDED_gsm_burst_sink_c_H
#define INCLUDED_gsm_burst_sink_c_H

#include <gr_sync_block.h>
#include <gsm_burst.h>

class gsm_burst_sink_c;

typedef boost::shared_ptr<gsm_burst_sink_c> gsm_burst_sink_c_sptr;

gsm_burst_sink_c_sptr gsm_make_burst_sink_c(gr_feval_ll *,float);

class gri_mmse_fir_interpolator_cc;

//class gsm_burst_sink_c : public gr_block, public gsm_burst
class gsm_burst_sink_c : public gr_sync_block, public gsm_burst
{
private:
	
	friend gsm_burst_sink_c_sptr gsm_make_burst_sink_c(gr_feval_ll *,float);
	gsm_burst_sink_c(gr_feval_ll *,float);  
	
	//clocking parameters
	double			d_sample_interval;
	double			d_clock_counter;
	gr_complex		d_last_sample;

	float			d_relative_sample_rate;		//omega
	float			d_mu;
	int				d_ii;	//save index between work calls for interp advance
	
	gri_mmse_fir_interpolator_cc 	*d_interp;  //sub-sample interpolator from GR
	
	gr_complex		d_complex_burst[BBUF_SIZE];

	void 			shift_burst(int shift_bits);
		
public:
	~gsm_burst_sink_c ();	

//	void forecast (int noutput_items, gr_vector_int &ninput_items_required);

	int work(int noutput_items,
				gr_vector_const_void_star &input_items,
				gr_vector_void_star &output_items);
	
/*	int general_work (	int noutput_items,
						gr_vector_int &ninput_items,
						gr_vector_const_void_star &input_items,
						gr_vector_void_star &output_items);
*/
};

#endif /* INCLUDED_gsm_burst_sink_c_H */
