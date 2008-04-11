
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gsm_burst_ff.h>
#include <gr_io_signature.h>
#include <gr_math.h>
#include <stdio.h>
#include <gri_mmse_fir_interpolator_cc.h>

gsm_burst_ff_sptr gsm_make_burst_ff (gr_feval_ll *t)
{
  return gsm_burst_ff_sptr (new gsm_burst_ff(t));
}

static const int MIN_IN = 1;	// minimum number of input streams
static const int MAX_IN = 1;	// maximum number of input streams
static const int MIN_OUT = 0;	// minimum number of output streams
static const int MAX_OUT = 1;	// maximum number of output streams

gsm_burst_ff::gsm_burst_ff (gr_feval_ll *t) : 
	gsm_burst(t),
	gr_block(	"burst_ff",
				gr_make_io_signature (MIN_IN, MAX_IN, sizeof (float)),
//				gr_make_io_signature (MIN_OUT, MAX_OUT, USEFUL_BITS * sizeof (float)))
				gr_make_io_signature (0, 0, 0))
//				gr_make_io_signature (MIN_OUT, MAX_OUT, sizeof (float)))
{
		
	set_history(1); 
	
}

gsm_burst_ff::~gsm_burst_ff ()
{
}

/*
void gsm_burst_ff::forecast (int noutput_items, gr_vector_int &ninput_items_required)
{
  unsigned ninputs = ninput_items_required.size ();
  for (unsigned i = 0; i < ninputs; i++)
    ninput_items_required[i] = noutput_items * BBUF_SIZE + history();
}
*/

int gsm_burst_ff::general_work (int noutput_items,
				   gr_vector_int &ninput_items,
				   gr_vector_const_void_star &input_items,
				   gr_vector_void_star &output_items)
{
	const float *in = (const float *) input_items[0];
	float *out = (float *) output_items[0];
	
	int ii=0;
	//int rval = 0;  //default to no output
	int rval = noutput_items;  //default to no output

	//int do_output = output_items.size() > 0 ? 1 : 0;
	int do_output = 0;
	
	int n_input = ninput_items[0];
//	fprintf(stderr,"out=%8.8x/#i=%d/#o=%d",(unsigned)out,n_input,noutput_items);
//	fprintf(stderr,"#i=%d/#o=%d",n_input,noutput_items);

//	while (( rval < noutput_items) && ( ii < n_input ) ) {
	while ( ii < n_input ) {

		assert(d_bbuf_pos <= BBUF_SIZE );
		
		if (d_bbuf_pos >= 0)	//could have been offset negative.  TODO: perhaps better just to add some slack to the buffer
			d_burst_buffer[d_bbuf_pos] = in[ii];
		
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
				//rval++;
				//rval += USEFUL_BITS*sizeof(float);

				switch ( d_clock_options & QB_MASK  ) {
				case QB_QUARTER: //Can't do this in the FF version
				case QB_FULL04:	//extra bit on timeslot 0 & 4
					if (!(d_ts%4))
						d_bbuf_pos--;
					break;
				case QB_NONE:	//don't adjust for quarter bits at all
				default:
					break;
				}

				d_last_burst_s_count = d_sample_count;	

			}
		}
		d_sample_count++;
 		ii++;
	}
	
//	fprintf(stderr,"/ii=%d/rval=%d\n",ii,rval);

	consume_each (ii);
	
	return rval;
}
