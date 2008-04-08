
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gsm_burst_cf.h>
#include <gr_io_signature.h>
#include <gr_math.h>
#include <stdio.h>
#include <gri_mmse_fir_interpolator_cc.h>

gsm_burst_cf_sptr gsm_make_burst_cf (gr_feval_ll *t,float sample_rate)
{
  return gsm_burst_cf_sptr (new gsm_burst_cf (t,sample_rate));
}

static const int MIN_IN = 1;	// minimum number of input streams
static const int MAX_IN = 1;	// maximum number of input streams
static const int MIN_OUT = 1;	// minimum number of output streams
static const int MAX_OUT = 1;	// maximum number of output streams

gsm_burst_cf::gsm_burst_cf (gr_feval_ll *t, float sample_rate) : 
	gsm_burst(t),
	gr_block (	"burst_cf",
				gr_make_io_signature (MIN_IN, MAX_IN, sizeof (gr_complex)),
				gr_make_io_signature (MIN_OUT, MAX_OUT, USEFUL_BITS * sizeof (float))),
	d_clock_counter(0.0),
	d_last_sample(0.0,0.0),
	d_interp(new gri_mmse_fir_interpolator_cc())

{

	//clocking parameters
	d_sample_interval = 1.0 / sample_rate;
	d_relative_sample_rate = sample_rate / GSM_SYMBOL_RATE;
	
	fprintf(stderr,"Sample interval      : %e\n",d_sample_interval);
	fprintf(stderr,"Relative sample rate : %g\n",d_relative_sample_rate);
		
	set_history(4); 
	
}

gsm_burst_cf::~gsm_burst_cf ()
{
	delete d_interp;
}

void gsm_burst_cf::forecast (int noutput_items, gr_vector_int &ninput_items_required)
{
  unsigned ninputs = ninput_items_required.size ();
  for (unsigned i = 0; i < ninputs; i++)
    ninput_items_required[i] = noutput_items * (int)ceil(d_relative_sample_rate) * BBUF_SIZE + history();
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

	int ninput = ninput_items[0];
	//fprintf(stderr,"#i=%d/#o=%d",n_input,noutput_items);

	int  ni = ninput - d_interp->ntaps();  // interpolator need -4/+3 samples NTAPS = 8
	
	while (( rval < noutput_items) && ( ii < ni ) ) {
		//clock symbols 
		//TODO: this is very basic and can be improved.  Need tracking...
		//TODO: use burst_start offsets as timing feedback
		//TODO: save complex samples for Viterbi EQ
		if ( d_clock_counter >= GSM_SYMBOL_PERIOD) {

			d_clock_counter -= GSM_SYMBOL_PERIOD; //reset clock for next sample, keep the remainder

			//float mu = 1.0 - d_clock_counter / GSM_SYMBOL_PERIOD;
			float mu = d_clock_counter / GSM_SYMBOL_PERIOD;
			gr_complex sample = d_interp->interpolate (&in[ii], mu);	//FIXME: this seems noisy, make sure it is being used correctly

			gr_complex conjprod = sample * conj(d_last_sample);
			float diff_angle = gr_fast_atan2f(imag(conjprod), real(conjprod));

			d_last_sample = sample;
	
			assert(d_bbuf_pos <= BBUF_SIZE );
			
			if (d_bbuf_pos >= 0)	//could be negative offset from burst alignment.  TODO: perhaps better just to add some padding to the buffer
				d_burst_buffer[d_bbuf_pos] = diff_angle;
			
			d_bbuf_pos++;
			
			if ( d_bbuf_pos >= BBUF_SIZE ) { 
			
				if (get_burst()) {
					//found a burst, send to output
					//ensure that output data is in range
					int b = d_burst_start;
					if (b < 0)
						b = 0;
					else if (b >= 2 * MAX_CORR_DIST)
						b = 2 * MAX_CORR_DIST - 1;
		
					memcpy(out+rval*USEFUL_BITS, d_burst_buffer + b, USEFUL_BITS*sizeof(float));
					rval++;

					switch ( d_clock_options & QB_MASK ) {
					case QB_QUARTER: //extra 1/4 bit each burst
						d_clock_counter -= GSM_SYMBOL_PERIOD / 4.0; 
						break;
					case QB_FULL04:	//extra bit on timeslot 0 & 4
						if (!(d_ts%4))
							d_clock_counter -= GSM_SYMBOL_PERIOD; 
						break;
					case QB_NONE:	//don't adjust for quarter bits at all
					default:
						break;
					}
					
					d_last_burst_s_count = d_sample_count;	
					
					//fprintf(stderr,"clock: %f, pos: %d\n",d_clock_counter,d_bbuf_pos);
				}
			}	   
		}

		d_clock_counter += d_sample_interval;
		d_sample_count++;
 		ii++;
	}
	
	//fprintf(stderr,"/ii=%d/rval=%d\n",ii,rval);

	consume_each (ii);
	
	return rval;
}
