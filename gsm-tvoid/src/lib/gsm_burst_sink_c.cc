
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gsm_burst_sink_c.h"
#include <gr_io_signature.h>
#include <gr_math.h>
#include <stdio.h>
#include <string.h>
#include <gri_mmse_fir_interpolator_cc.h>

gsm_burst_sink_c_sptr gsm_make_burst_sink_c (gr_feval_ll *t,float sample_rate)
{
  return gsm_burst_sink_c_sptr (new gsm_burst_sink_c(t,sample_rate));
}

static const int MIN_IN = 1;	// minimum number of input streams
static const int MAX_IN = 1;	// maximum number of input streams

gsm_burst_sink_c::gsm_burst_sink_c (gr_feval_ll *t, float sample_rate) : 
	gsm_burst(t),
	gr_sync_block (	"burst_sink_c",
				gr_make_io_signature (MIN_IN, MAX_IN, sizeof (gr_complex)),
				gr_make_io_signature (0,0,0)
			),
	d_clock_counter(0.0),
	d_mu(0.5),
	d_last_sample(0.0,0.0),
	d_ii(0),
	d_interp(new gri_mmse_fir_interpolator_cc())

{

	//clocking parameters
	d_sample_interval = 1.0 / sample_rate;
	d_relative_sample_rate = sample_rate / GSM_SYMBOL_RATE;
	
	fprintf(stderr,"Sample interval      : %e\n",d_sample_interval);
	fprintf(stderr,"Relative sample rate : %g\n",d_relative_sample_rate);
	
	//we need history for interpolator taps and some saftey relative to relative rate
	int hist = d_interp->ntaps(); // + 16;  // interpolator need -4/+3 samples NTAPS = 8  , 16 for safety margin
	set_history(hist); //need history for interpolator
	
}

gsm_burst_sink_c::~gsm_burst_sink_c ()
{
	delete d_interp;
}

void gsm_burst_sink_c::shift_burst(int shift_bits)
{
	//fprintf(stderr,"sft:%d\n",shift_bits);

	assert(shift_bits >= 0);
	assert(shift_bits < BBUF_SIZE );

	gr_complex *p_src = d_complex_burst + shift_bits;
	gr_complex *p_dst = d_complex_burst;
	int num = BBUF_SIZE - shift_bits;
	
	memmove(p_dst,p_src,num * sizeof(gr_complex));  //need memmove because of overlap

	//DON'T adjust the buffer positions, the superclass method will do that...
	//d_bbuf_pos -= shift_bits;
	//call the parent method to shift the float version
	gsm_burst::shift_burst(shift_bits);
}

//TODO: put everything but GR stuff in a common complex type class (share w/ gsm_burst_cf)
int gsm_burst_sink_c::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)

{
	const gr_complex *in = (const gr_complex *) input_items[0];
	
//	fprintf(stderr,"#o=%d",noutput_items);

	assert( d_ii >= 0 );

	while ( d_ii < noutput_items ) {
		//clock symbols 

		gr_complex sample = d_interp->interpolate (&in[d_ii], d_mu);
		
		gr_complex conjprod = sample * conj(d_last_sample);
		float diff_angle = gr_fast_atan2f(imag(conjprod), real(conjprod));

		d_last_sample = sample;

#if 1		
		assert(d_bbuf_pos <= BBUF_SIZE );
		
		if (d_bbuf_pos >= 0) {	//could be negative offset from burst alignment.  TODO: perhaps better just to add some padding to the buffer
			d_burst_buffer[d_bbuf_pos] = diff_angle;
			d_complex_burst[d_bbuf_pos] = sample;
		}
		
		d_bbuf_pos++;
		
		if ( d_bbuf_pos >= BBUF_SIZE ) { 
			if (get_burst()) {
				//adjust timing
				//TODO: generate timing error from burst buffer (phase & freq)
							
				switch ( d_clock_options & QB_MASK ) {
				case QB_QUARTER: //extra 1/4 bit each burst
					d_mu -= d_relative_sample_rate / 4.0;
					break;
				case QB_FULL04:	//extra bit on timeslot 0 & 4
					if (!(d_ts%4))
						d_mu -= d_relative_sample_rate;
					break;
				case QB_NONE:	//don't adjust for quarter bits at all
				default:
					break;
				}
				
				d_last_burst_s_count = d_sample_count;
				
				//fprintf(stderr,"clock: %f, pos: %d\n",d_clock_counter,d_bbuf_pos);
			}
		}	   
#endif			

		d_mu += d_relative_sample_rate;
		d_ii += (int)floor(d_mu);
		//d_sample_count += (int)floor(d_mu);		//TODO: outside loop?
		d_mu -= floor(d_mu);
	}

	//reset d_ii, accounting for advance
	d_ii -= noutput_items;
	
//	fprintf(stderr,"/mu=%f",d_mu);
//	fprintf(stderr,"/ii=%d\n",d_ii);

	return noutput_items;
}
