
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gsm_burst.h"
#include <gr_math.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <assert.h>
#include "system.h"
#include "gsmstack.h"


gsm_burst::gsm_burst (gr_feval_ll *t) :
		p_tuner(t),
		d_clock_options(DEFAULT_CLK_OPTS),
		d_print_options(0),
		d_test_options(0),
		d_hop_good_arfcn(1),
		d_hop_bad_arfcn(2),
		d_equalizer_type(EQ_FIXED_DFE)
{
 
//	fprintf(stderr,"gsm_burst: enter constructor (t=%8.8x)\n",(unsigned int)t);
	  	
//	M_PI = M_PI; //4.0 * atan(1.0); 

	full_reset();
	
	//encode sync bits
	float tsync[N_SYNC_BITS];
	
	for (int i=0; i < N_SYNC_BITS; i++) {
		tsync[i] = 2.0*SYNC_BITS[i] - 1.0;
	}

	diff_encode(tsync,corr_sync,N_SYNC_BITS);

/*
	fprintf(stderr," Sync: ");
	print_bits(tsync,N_SYNC_BITS);
	fprintf(stderr,"\n");

	fprintf(stderr,"DSync: ");
	print_bits(corr_sync,N_SYNC_BITS);
	fprintf(stderr,"\n\n");
*/

	for (int i=0; i < 10; i++) {
		for (int j=0; j < N_TRAIN_BITS; j++) {
			tsync[j] = 2.0*train_seq[i][j] - 1.0;
		}
		diff_encode(tsync,corr_train_seq[i],N_TRAIN_BITS);

/*
		fprintf(stderr,"TSC%d: ",i);
		print_bits(corr_train_seq[i],N_TRAIN_BITS);
		fprintf(stderr,"\n");
*/
	}
		
	/* Initialize GSM Stack */
	GS_new(&d_gs_ctx);
}

gsm_burst::~gsm_burst ()
{
}

void gsm_burst::sync_reset(void)
{
	d_sync_state = WAIT_FCCH;
	d_last_good = 0;
	d_last_sch = 0;
	d_burst_count = 0;
}

//TODO: check this for thread safeness
void gsm_burst::full_reset(void)
{
	sync_reset();

	d_sync_loss_count=0;
	d_fcch_count=0;
	d_part_sch_count=0;
	d_sch_count=0;
	d_normal_count=0;
	d_dummy_count=0;
	d_unknown_count=0;
	d_total_count=0;
		  	
	d_freq_offset=0.0;
	d_freq_off_sum=0.0;
	d_freq_off_weight=0;
	
	d_ts=0;

	d_bbuf_pos=0;
	d_burst_start=MAX_CORR_DIST;
	d_sample_count=0;
	d_last_burst_s_count=0;
	d_corr_pattern=0;
	d_corr_pat_size=0;
	d_corr_max=0.0;
	d_corr_maxpos=0;
	d_corr_center=0;
	d_last_sync_state=WAIT_FCCH;

}

float gsm_burst::mean_freq_offset(void)
{
	if (d_freq_off_weight)
		return d_freq_off_sum / d_freq_off_weight;
	else
		return 0.0;
}

void gsm_burst::diff_encode(const float *in,float *out,int length,float lastbit) {
	
	for (int i=0; i < length; i++) {
		out[i] = in[i] * lastbit;
		lastbit=in[i];
		
	}
}

void gsm_burst::diff_decode(const float *in,float *out,int length,float lastbit) {
	
	for (int i=0; i < length; i++) {
		out[i] = in[i] * lastbit;
		lastbit = out [i];
	}
}

void gsm_burst::diff_decode_burst(void) {
	char lastbit = 0;
	
	//slice
	for (int i = 0; i < USEFUL_BITS; i++) {
		d_decoded_burst[i] = d_burst_buffer[d_burst_start + i] > 0 ? 0 : 1;
	}

	//diff decode
	for (int i=0; i < USEFUL_BITS; i++) {
		d_decoded_burst[i] ^= lastbit;
		lastbit = d_decoded_burst[i];
	}

}

void gsm_burst::print_hex(const unsigned char *data,int length)
{
	unsigned char tbyte;
	int i,bitpos=0;
	
	assert(data);
	assert(length >= 0);
	
	
	
	while (bitpos < length) {
		tbyte = 0;
		for (i=0; (i < 8) && (bitpos < length); i++) {
			tbyte <<= 1;
			tbyte |= data[bitpos++];
		}
		if (i<8)
			tbyte <<= 8 - i;
		
		fprintf(stdout,"%2.2X ",tbyte);
	}	
}

void gsm_burst::print_bits(const float *data,int length)
{
	assert(data);
	assert(length >= 0);
	
	for (int i=0; i < length; i++)
		data[i] < 0 ? fprintf(stderr,"+") : fprintf(stderr,".");
		
}

#if 0
void gsm_burst::soft2hardbit(char *dst, const float *data, int len)
{
	for (int i=0; i < len; i++)
	{
		if (data[i] < 0)
			dst[i] = 0;
		else
			dst[i] = 1;
	}
}
#endif

void gsm_burst::print_burst(void)
{
	int bursts_since_sch;

	int print = 0;

	//fprintf(stderr,"p=%8.8X ",	d_print_options);

	if ( PRINT_GSM_DECODE & d_print_options ) {

		/*
		 * Pass information to GSM stack. GSM stack will try to extract
		 * information (fn, layer 2 messages, ...)
		 */
		diff_decode_burst();		
		GS_process(&d_gs_ctx, d_ts, d_burst_type, d_decoded_burst);
	}
		
	if ( PRINT_EVERYTHING == d_print_options )
		print = 1;
	else if ( (!d_ts) && (d_print_options & PRINT_TS0) )
		print = 1;
	else if ( (DUMMY == d_burst_type) && (d_print_options & PRINT_DUMMY) )
		print = 1;
	else if ( (NORMAL == d_burst_type) && (d_print_options & PRINT_NORMAL) )
		print = 1;
	else if ( (SCH == d_burst_type) && (d_print_options & PRINT_SCH) )
		print = 1;
	else if ( (FCCH == d_burst_type) && (d_print_options & PRINT_FCCH) )
		print = 1;
	else if ( (UNKNOWN == d_burst_type) && (d_print_options & PRINT_UNKNOWN) )
		print = 1;

	if ( print && (d_print_options & PRINT_BITS) ) {
		if (d_print_options & PRINT_ALL_BITS)
		{
			print_bits(d_burst_buffer,BBUF_SIZE);
		} else {
			/* 142 useful bits: 2*58 + 26 training */
			print_bits(d_burst_buffer + d_burst_start,USEFUL_BITS);
		}
		
		fprintf(stderr," ");
	}

	if (print) {

		fprintf(stderr,"%d/%d/%+d/%lu/%lu ",
						d_sync_state,
						d_ts,
						d_burst_start - MAX_CORR_DIST,
						d_sample_count,
						d_sample_count - d_last_burst_s_count);

		switch (d_burst_type) {
		case FCCH:
			fprintf(stderr,"[FCCH] foff:%g cnt:%lu",d_freq_offset,d_fcch_count);
			break;
		case PARTIAL_SCH:
			bursts_since_sch = d_burst_count - d_last_sch;
			
			fprintf(stderr,"[P-SCH] cor:%.2f last:%d cnt: %lu",
					d_corr_max,bursts_since_sch,d_sch_count);
			break;
		case SCH:
			bursts_since_sch = d_burst_count - d_last_sch;
			
			fprintf(stderr,"[SCH] cor:%.2f last:%d cnt: %lu",
					d_corr_max,bursts_since_sch,d_sch_count);
			break;
		case DUMMY:
			fprintf(stderr,"[DUMMY] cor:%.2f",d_corr_max);
			break;
		case ACCESS:
			fprintf(stderr,"[ACCESS]");		//We don't detect this yet
			break;
		case NORMAL:
			fprintf(stderr,"[NORM] clr:%d cor:%.2f",d_color_code,d_corr_max);
			break;
		case UNKNOWN:
			fprintf(stderr,"[?]");
			break;
		default:
			fprintf(stderr,"[oops! default]");
			break;		
		}

		fprintf(stderr,"\n");


		//print the correlation pattern for visual inspection
		if ( (UNKNOWN != d_burst_type) && 
				(d_sync_state > WAIT_SCH_ALIGN) && 
				(d_print_options & PRINT_CORR_BITS) ) 
		{
			
			int pat_indent;
			
			if (d_print_options & PRINT_ALL_BITS)
				pat_indent = d_corr_center + d_corr_maxpos;
			else
			 	pat_indent = d_corr_center - MAX_CORR_DIST;		//useful bits will already be offset
				
			for (int i = 0; i < pat_indent; i++)
				fprintf(stderr," ");
			
			fprintf(stderr," ");	//extra space for skipped bit
			print_bits(d_corr_pattern+1,d_corr_pat_size-1);	//skip first bit (diff encoding)
			
			fprintf(stderr,"\t\toffset:%d, max: %.2f \n",d_corr_maxpos,d_corr_max);
		}
	
	}
	
	//Print Burst data in hex
	if ( d_print_options & PRINT_HEX ) {
		fprintf(stdout,"%d,%d,",d_ts,d_burst_type);
		diff_decode_burst();		
		print_hex(d_decoded_burst,USEFUL_BITS);
		fprintf(stdout,"\n");
	}
	
	//Print State related messages
	if ( d_print_options & PRINT_STATE ) {
		if ( (SYNCHRONIZED == d_sync_state) && (SYNCHRONIZED != d_last_sync_state) ) {
			fprintf(stderr,"====== SYNC GAINED (FOff: %g Corr: %.2f, Color: %d ) ======\n",d_freq_offset,d_corr_max,d_color_code);
		}
		else if ( (SYNCHRONIZED != d_sync_state) && (SYNCHRONIZED == d_last_sync_state) ) {
			fprintf(stderr,"====== SYNC LOST (%ld) ======\n",d_sync_loss_count);
		}
	}
	
}

void gsm_burst::shift_burst(int shift_bits)
{
	//fprintf(stderr,"sft:%d\n",shift_bits);

	assert(shift_bits >= 0);
	assert(shift_bits < BBUF_SIZE );

	float *p_src = d_burst_buffer + shift_bits;
	float *p_dst = d_burst_buffer;
	int num = BBUF_SIZE - shift_bits;
	
	memmove(p_dst,p_src,num * sizeof(float));  //need memmove because of overlap

	//adjust the buffer positions
	d_bbuf_pos -= shift_bits;

	assert(d_bbuf_pos >= 0);
}


//Calculate frequency offset of an FCCH  burst  from  the  mean  phase  difference
//FCCH  should  be  a constant frequency and  equivalently  a  constant  phase 
//increment  (pi/2)  per sample. Calculate the frequency offset by the difference 
//of the mean phase  from pi/2.
void gsm_burst::calc_freq_offset(void) 
{
	const int padding = 20;
	int start = d_burst_start + padding;
	int end = d_burst_start + USEFUL_BITS - padding;
	
	float sum = 0.0;
	for (int j = start; j <= end; j++) {
		sum += d_burst_buffer[j];
	}
	float mean = sum / ((float)USEFUL_BITS - (2.0 * (float)padding) );
	
	float p_off = mean - (M_PI / 2);
	d_freq_offset = p_off * 1625000.0 / (12.0 * M_PI);
	

	//maintain a 100 weight mean
	if (d_freq_off_weight < 100) 
		d_freq_off_weight++;
	else
		d_freq_off_sum *= 99.0/100.0;
		
	d_freq_off_sum += d_freq_offset;
}

// This will look for a series of positive phase differences comprising
// a FCCH burst.  When we find one, we calculate the frequency offset and 
// adjust the burst timing so that it will be at least coarsely aligned 
// for SCH detection.
//
// TODO: Adjust start pos on long hits
//			very large hit counts may indicate an unmodulated carrier.
BURST_TYPE gsm_burst::get_fcch_burst(void) 
{
	int hit_count = 0;
	int miss_count = 0;
	int start_pos = -1;
	
	for (int i=0; i < BBUF_SIZE; i++) {
		if (d_burst_buffer[i] > 0) {
			if ( ! hit_count++ )
				start_pos = i;
		} 
		else {
			if (hit_count >= FCCH_HITS_NEEDED) {
				break;
			} 
			else if ( ++miss_count > FCCH_MAX_MISSES ) {
					start_pos = -1;
					hit_count = miss_count = 0;
			}
		}
	}

	//Do we have a match?
	if ( start_pos >= 0 ) {
		//Is it within range? (we know it's long enough then too)
		if ( start_pos < 2*MAX_CORR_DIST ) {
			d_burst_start = start_pos;
			d_bbuf_pos = 0; //load buffer from start
			return FCCH;
		
		} 
		else {
			//TODO: don't shift a tiny amount
			shift_burst(start_pos - MAX_CORR_DIST);
		}
	} 
	else {
		//Didn't find anything
		d_burst_start = MAX_CORR_DIST;
		d_bbuf_pos = 0; //load buffer from start
	}
	
	return UNKNOWN;
}


void gsm_burst::equalize(void) 
{
	float last = 0.0;
	
	switch ( d_equalizer_type ) {
	case EQ_FIXED_LINEAR:
		//TODO: should filter w/ inverse freq response
		//this is just for giggles
		for (int i = 1; i < BBUF_SIZE - 1; i++) {
			d_burst_buffer[i] = - 0.4 * d_burst_buffer[i-1] + 1.1 * d_burst_buffer[i] - 0.4 * d_burst_buffer[i+1];
		}
		break;
	case EQ_FIXED_DFE:
		//TODO: allow coefficients to be options?
		for (int i = 0; i < BBUF_SIZE; i++) {
			d_burst_buffer[i] -=  0.4 * last;
			d_burst_buffer[i] > 0.0 ? last = M_PI/2 : last = -M_PI/2;
		}
		break;
	default:
		fprintf(stderr,"!EQ");
	case EQ_NONE:
		break;
	}	
}

//TODO: optimize by working incrementally out from center and returning when a provided threshold is reached
float gsm_burst::correlate_pattern(const float *pattern,const int pat_size,const int center,const int distance) 
{
	float corr;
	
	//need to save these for later printing, etc
	//TODO: not much need for function params when we have the member vars
	d_corr_pattern = pattern;
	d_corr_pat_size = pat_size;		
	d_corr_max = 0.0;
	d_corr_maxpos = 0;
	d_corr_center = center;

	for (int j=-distance;j<=distance;j++) {
		corr = 0.0;
		for (int i = 1; i < pat_size; i++) {	//Start a 1 to skip first bit due to diff encoding
			//d_corr[j+distance] += d_burst_buffer[center+i+j] * pattern[i];
			//corr += SIGNUM(d_burst_buffer[center+i+j]) * pattern[i];  //binary corr/sliced
			corr += d_burst_buffer[center+i+j] * pattern[i];
		}
		corr /= pat_size - 1; //normalize, -1 for skipped first bit
		if (corr > d_corr_max) {
			d_corr_max = corr;
			d_corr_maxpos = j;
		}
	}		

	return d_corr_max;
}

BURST_TYPE gsm_burst::get_sch_burst(void) 
{
	BURST_TYPE type = UNKNOWN;
	int tpos = 0;  //default d_bbuf_pos

	equalize();
	
//	if (!d_ts) {	// wait for TS0

		//correlate over a range to detect and align on the sync pattern
	 	correlate_pattern(corr_sync,N_SYNC_BITS,MAX_CORR_DIST+SYNC_POS,20);

		if (d_corr_max > SCH_CORR_THRESHOLD) {
			d_burst_start += d_corr_maxpos;

			//It's possible that we will corelate far enough out that some burst data will be lost.
			//	In this case we should be in aligned state, and wait until next SCH to decode it
			if (d_burst_start < 0) {
				//We've missed the beginning of the data, wait for the next SCH
				//TODO: verify timing in this case
				type = PARTIAL_SCH;
			} else if (d_burst_start > 2 * MAX_CORR_DIST) {
				//The rest of our data is still coming, get it...
				shift_burst(d_burst_start - MAX_CORR_DIST);
				d_burst_start = MAX_CORR_DIST;
				tpos = d_bbuf_pos;
			} else {
				type = SCH;
			}
		} 
		else {
			d_burst_start = MAX_CORR_DIST;
		}

//	} else {
//		d_burst_start = MAX_CORR_DIST;
//	} 

	d_bbuf_pos = tpos;

	return type;			
}

BURST_TYPE gsm_burst::get_norm_burst(void) 
{
	int eq = 0;
 	BURST_TYPE type = UNKNOWN;
	

	if (!d_ts) {
		// Don't equalize before checking FCCH
		if ( FCCH_CORR_THRESHOLD < correlate_pattern(corr_train_seq[TS_FCCH],N_TRAIN_BITS,MAX_CORR_DIST+TRAIN_POS,0) ) {
			type = FCCH;
			d_burst_start = MAX_CORR_DIST;
			d_corr_maxpos = 0;  //we don't want to affect timing
		
		} 
		else {
			equalize();
			eq=1;
			
			//TODO: check CTS & COMPACT SYNC
			if (SCH_CORR_THRESHOLD < correlate_pattern(corr_sync,N_SYNC_BITS,MAX_CORR_DIST+SYNC_POS,MAX_CORR_DIST) )
				type = SCH;
		}
	}	

	if (UNKNOWN == type) { //no matches yet 
		if (!eq) equalize();
		
		//Match dummy sequence
		if ( NORM_CORR_THRESHOLD < correlate_pattern(corr_train_seq[TS_DUMMY],N_TRAIN_BITS,MAX_CORR_DIST+TRAIN_POS,MAX_CORR_DIST) ) { 
			type = DUMMY;
		
		} 
		else {
			//Match normal training sequences
			//TODO: start with current color code
			for (int i=0; i < 8; i++) {
				if ( NORM_CORR_THRESHOLD < correlate_pattern(corr_train_seq[i],N_TRAIN_BITS,MAX_CORR_DIST+TRAIN_POS,MAX_CORR_DIST) ) {
					type = NORMAL;
					d_color_code = i;
					break;
				}
			}
		}
	}
		
	if ( UNKNOWN == type ) {
		d_burst_start = MAX_CORR_DIST;
	
	} else {
		d_burst_start += d_corr_maxpos;
	}

	return type;
}


int gsm_burst::get_burst(void) 
{
	//TODO: should we output data while looking for FCCH?  Maybe an option.
	int got_burst=1;	//except for the WAIT_FCCH case we always have output
	d_burst_type = UNKNOWN;	//default
	
	//begin with the assumption the the burst will be in the correct position
	d_burst_start = MAX_CORR_DIST;
	
	//process the burst			
	switch (d_sync_state) {
	case WAIT_FCCH:
		d_ts = 0;

		if ( FCCH == ( d_burst_type = get_fcch_burst()) ) {
			d_sync_state = WAIT_SCH_ALIGN;
			d_bbuf_pos = 0; //load buffer from start
		
		} 
		else {
			got_burst = 0;
		} 
		
		break;

	case WAIT_SCH_ALIGN:
		d_burst_type = get_sch_burst();
		
		switch ( d_burst_type ) {
		case PARTIAL_SCH:
			d_sync_state = WAIT_SCH;
			break;
		//case SCH:
		//let the burst type switch handle this so it knows if new or old sync
		//	d_sync_state = SYNCHRONIZED;
		break;
		default:
			break;
		}

		break;
	
	case WAIT_SCH:	//TODO: check this case 
	case SYNCHRONIZED:
		d_burst_type = get_norm_burst();
		d_bbuf_pos = 0; //load buffer from start

		break;
	} 

	//Update stats
	switch (d_burst_type) {
	case FCCH:
		if (SYNCHRONIZED == d_sync_state)
			d_burst_count++;
		else 
			d_burst_count = 0;
		
		d_fcch_count++;
		calc_freq_offset();
		d_ts = 0;
		break;
	case PARTIAL_SCH:
		d_burst_count++;
		d_part_sch_count++;
		d_last_sch = d_burst_count;
		d_ts = 0;		//TODO: check this
		break;
	case SCH:
		//TODO: it would be better to adjust tuning on first FCCH (for better SCH detection),
		//		but tuning can run away with false FCCHs
		//		Some logic to retune back to original offset on false FCCH might work
		if (p_tuner) {
			if (SYNCHRONIZED == d_sync_state)
				p_tuner->calleval(BURST_CB_ADJ_OFFSET);
			else
				p_tuner->calleval(BURST_CB_SYNC_OFFSET);
				
		}
		d_burst_count++;
		d_sch_count++;
		d_last_sch = d_burst_count;
		d_sync_state = SYNCHRONIZED;	//handle WAIT_SCH
		d_ts = 0;
		break;
	case NORMAL:
		d_burst_count++;
		d_normal_count++;
		break;
	case DUMMY:
		d_burst_count++;
		d_dummy_count++;
		break;
	default:
	case UNKNOWN:
		if (SYNCHRONIZED == d_sync_state) {
			d_burst_count++;
			d_unknown_count++;
		}
		break;
	}	

	if (UNKNOWN != d_burst_type) {
		d_last_good = d_burst_count;
	}

	//Check for loss of sync
	int bursts_since_good = d_burst_count - d_last_good;
	if (bursts_since_good > MAX_SYNC_WAIT) {
		d_sync_loss_count++;
		sync_reset();
	}

	if (got_burst) {
		d_total_count++;
		
		//print info
		print_burst();

		/////////////////////
		//start tune testing
#ifdef TEST_HOP_SPEED
		static int good_count = -1; //-1: wait sch, >=0: got sch, counting
		static int wait_count = 0;

		if (OPT_TEST_HOP_SPEED & d_test_options ) {
			//have we started counting?
			if ( good_count >= 0 ) {
				
				if (UNKNOWN == d_burst_type) {
					if (good_count >= 0) {
						fprintf(stdout,"good_count: %d\n",good_count);
			
						if (p_tuner) {
							next_arfcn = d_hop_good_arfcn;
							p_tuner->calleval(BURST_CB_TUNE);
						}
					}
					good_count = -1;	// start again at resync
				
				} else {
					//count good bursts
					good_count++;
				}
						
			} else {
				//haven't started counting
				// get some good syncs before trying again
				if ((SCH == d_burst_type) && (++wait_count > 5)) {	
					//fprintf(stdout,"restarting good_count\n");
					good_count = wait_count = 0;
					//tune away
					if (p_tuner) { 
						next_arfcn = d_hop_bad_arfcn;
						p_tuner->calleval(BURST_CB_TUNE);
					}
				}
			}
		}
#endif
		//end tune testing	
		/////////////////////


		//Adjust the buffer write position to align on MAX_CORR_DIST
		if ( d_clock_options & CLK_CORR_TRACK )
			d_bbuf_pos += MAX_CORR_DIST - d_burst_start;
	}

	d_last_sync_state = d_sync_state;
	
	d_ts = (++d_ts)%8;	//next TS

	return got_burst;
}

