#ifndef INCLUDED_GSM_BURST_CF_H
#define INCLUDED_GSM_BURST_CF_H

#include <gr_block.h>
#include "gsm_burst.h"
#include "mm_f.h"

class gsm_burst_cf;

typedef boost::shared_ptr<gsm_burst_cf> gsm_burst_cf_sptr;

gsm_burst_cf_sptr gsm_make_burst_cf(gr_feval_ll *,float);

class gri_mmse_fir_interpolator_cc;

class gsm_burst_cf : public gr_block, public gsm_burst
{
private:
	
	friend gsm_burst_cf_sptr gsm_make_burst_cf(gr_feval_ll *,float);
	gsm_burst_cf(gr_feval_ll *,float);  
	
	//clocking parameters
	float			d_sample_interval;
	float			d_clock_counter;		//??? sample count ???

	gr_complex		d_last_sample;

	//M&M clock recovery
	mm_f			mm;
		
	gri_mmse_fir_interpolator_cc 	*d_interp;  //sub-sample interpolator from GR

	gr_complex		slicer(gr_complex x);
			
public:
	~gsm_burst_cf ();	

	void forecast (int noutput_items, gr_vector_int &ninput_items_required);
	
	int general_work (	int noutput_items,
						gr_vector_int &ninput_items,
						gr_vector_const_void_star &input_items,
						gr_vector_void_star &output_items);

	float			get_omega() {return mm.d_omega;}

};

#endif /* INCLUDED_GSM_BURST_CF_H */
