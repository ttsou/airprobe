#ifndef INCLUDED_GSM_BURST_FF_H
#define INCLUDED_GSM_BURST_FF_H

#include <gr_block.h>
#include <gsm_burst.h>

class gsm_burst_ff;

typedef boost::shared_ptr<gsm_burst_ff> gsm_burst_ff_sptr;

gsm_burst_ff_sptr gsm_make_burst_ff(gr_feval_ll *);

class gsm_burst_ff : public gr_block, public gsm_burst
{
private:
	
	friend gsm_burst_ff_sptr gsm_make_burst_ff(gr_feval_ll *);
	gsm_burst_ff(gr_feval_ll *t);  
	
public:
	~gsm_burst_ff ();	

//	void forecast (int noutput_items, gr_vector_int &ninput_items_required);
	
	int general_work (	int noutput_items,
						gr_vector_int &ninput_items,
						gr_vector_const_void_star &input_items,
						gr_vector_void_star &output_items);
};

#endif /* INCLUDED_GSM_BURST_FF_H */
