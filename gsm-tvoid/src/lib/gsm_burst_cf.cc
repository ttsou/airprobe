
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gsm_burst_cf.h>
#include <gr_io_signature.h>
#include <gr_math.h>
#include <stdio.h>
#include <string.h>
#include <gri_mmse_fir_interpolator_cc.h>

gsm_burst_cf_sptr gsm_make_burst_cf (gr_feval_ll *t,float sample_rate)
{
  return gsm_burst_cf_sptr (new gsm_burst_cf (t,sample_rate));
}

static const int MIN_IN = 1;	// minimum number of input streams
static const int MAX_IN = 1;	// maximum number of input streams
static const int MIN_OUT = 0;	// minimum number of output streams
static const int MAX_OUT = 1;	// maximum number of output streams

gsm_burst_cf::gsm_burst_cf (gr_feval_ll *t, float sample_rate) : 
	gr_block(	"burst_cf",
				gr_make_io_signature (MIN_IN, MAX_IN, sizeof (gr_complex)),
				gr_make_io_signature (MIN_OUT, MAX_OUT, USEFUL_BITS * sizeof (float))  //TODO: pad to ^2 = 256 ?
			),		
	gsm_burst(t),
	d_clock_counter(0.0),
	d_last_sample(0.0,0.0),
	mm(sample_rate / GSM_SYMBOL_RATE),
	d_interp(new gri_mmse_fir_interpolator_cc()
	)

{

	//clocking parameters
	//d_sample_interval = 1.0 / sample_rate;
	//d_omega = sample_rate / GSM_SYMBOL_RATE;
	
//	fprintf(stderr,"Sample interval      : %e\n",d_sample_interval);
//	fprintf(stderr,"Relative sample rate : %g\n",d_omega);
	
	
	//set_relative_rate( mm.d_omega / 156);
	set_relative_rate( 1.0 / (mm.d_omega * 156) );
			
	set_history(4); //need history for interpolator
	
}

gsm_burst_cf::~gsm_burst_cf ()
{
	delete d_interp;
}

void gsm_burst_cf::forecast (int noutput_items, gr_vector_int &ninput_items_required)
{
	unsigned ninputs = ninput_items_required.size ();
	for (unsigned i = 0; i < ninputs; i++) {
		ninput_items_required[i] = noutput_items * (int)ceil(mm.d_omega) * TS_BITS;
		//fprintf(stderr,"forecast[%d]: %d = %d\n",i,noutput_items,ninput_items_required[i]);
	}
}

int gsm_burst_cf::general_work (int noutput_items,
				   gr_vector_int &ninput_items,
				   gr_vector_const_void_star &input_items,
				   gr_vector_void_star &output_items)
{
	const gr_complex *in = (const gr_complex *) input_items[0];
	float *out = (float *) output_items[0];
	
	int ii=0;
	int rval = 0;  //default to no output
	int num_outputs = output_items.size();
	int do_output = num_outputs > 0 ? 1 : 0;

	int ninput = ninput_items[0];
	//fprintf(stderr,"#i=%d/#o=%d",ninput,noutput_items);

	int  ni = ninput - d_interp->ntaps() - 16;  // interpolator need -4/+3 samples NTAPS = 8  , - 16 for safety margin
	
	while (( rval < noutput_items) && ( ii < ni ) ) {
		//clock symbols 
		//TODO: this is very basic and can be improved.  Need tracking...
		//TODO: save complex samples for Viterbi EQ
		
		//get interpolated sample
		gr_complex x_0 = d_interp->interpolate (&in[ii], mm.d_mu);
		
		//calulate phase difference (demod)
		gr_complex conjprod = x_0 * conj(d_last_sample);
		float diff_angle = gr_fast_atan2f(imag(conjprod), real(conjprod));

		//mM&M
		//mm.update(x_0);  //mm_c
		mm.update(diff_angle);  //mm_f
		
		assert(d_bbuf_pos <= BBUF_SIZE );
		
		if (d_bbuf_pos >= 0)	//could be negative offset from burst alignment.  TODO: perhaps better just to add some padding to the buffer
			d_burst_buffer[d_bbuf_pos] = diff_angle;
		
		d_bbuf_pos++;
		
		if ( d_bbuf_pos >= BBUF_SIZE ) { 
		
			if (get_burst()) {
				//found a burst, send to output
				if (do_output) {
					//ensure that output data is in range
					int b = d_burst_start;
					if (b < 0)
						b = 0;
					else if (b >= 2 * MAX_CORR_DIST)
						b = 2 * MAX_CORR_DIST - 1;
		
					memcpy(out+rval*USEFUL_BITS, d_burst_buffer + b, USEFUL_BITS*sizeof(float));
				}
				rval++;
				
				switch ( d_clock_options & QB_MASK ) {
				case QB_QUARTER: //extra 1/4 bit each burst
					mm.d_mu -= mm.d_omega / 4.0;
					//d_clock_counter -= GSM_SYMBOL_PERIOD / 4.0; 
					break;
				case QB_FULL04:	//extra bit on timeslot 0 & 4
					if (!(d_ts%4))
						mm.d_mu -= mm.d_omega;
						//d_clock_counter -= GSM_SYMBOL_PERIOD; 
					break;
				case QB_NONE:	//don't adjust for quarter bits at all
				default:
					break;
				}
				
				d_last_burst_s_count = d_sample_count;
				
				//fprintf(stderr,"clock: %f, pos: %d\n",d_clock_counter,d_bbuf_pos);
			}
		}	   
		
		//process mu / ii advance
		ii += (int)floor(mm.d_mu);
		d_sample_count += (int)floor(mm.d_mu);
		mm.d_mu -= floor(mm.d_mu);
		
		d_last_sample = x_0;
	}
	
	//fprintf(stderr,"/ii=%d/rval=%d\n",ii,rval);

	consume_each (ii);
	
	return rval;
}
