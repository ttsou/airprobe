#ifndef INCLUDED_GSM_BURST_H
#define INCLUDED_GSM_BURST_H

//TODO: rename to gsm_burst_receiver ?  use gsm_burst as encapsulation of an actual burst, incl bbuf, etc.
//			need to determine what is a decoder vs. burst function.  E.g. calc_freq_offset
//			everything but I/O & clocking & sync_state?
//			What about handling complex&diff&bin data?

#include <gsm_constants.h>
#include <gr_math.h>

//Console printing options
#define PRINT_NOTHING		0x00000000
#define PRINT_EVERYTHING	0x7FFFFFFF		//7 for SWIG overflow check work around
#define PRINT_BITS			0x00000001
#define PRINT_ALL_BITS		0x00000002
#define PRINT_CORR_BITS		0x00000004

#define PRINT_ALL_TYPES		0x00000FF0
#define PRINT_KNOWN			0x00000008
#define PRINT_UNKNOWN		0x00000010
#define PRINT_TS0			0x00000020
#define PRINT_FCCH			0x00000040
#define PRINT_SCH			0x00000080
#define PRINT_DUMMY			0x00000100
#define PRINT_NORMAL		0x00000200

//Timing/clock options
#define QB_NONE				0x00000000
#define QB_QUARTER			0x00000001		//only for internal clocked versions
#define QB_FULL04			0x00000003
#define QB_MASK				0x0000000F

#define CLK_CORR_TRACK		0x00000010		//adjust timing based on correlation offsets

#define DEFAULT_CLK_OPTS	( QB_QUARTER | CLK_CORR_TRACK )

#define SIGNUM(x)	((x>0)-(x<0))
		  
#define BBUF_SIZE	   TS_BITS

// Center bursts in the TS, splitting the guard period
//
// +--+--+---...-----+--...---+----...----+--+--+
//  G  T     D1         TS         D2      T   G
// Start ^

#define MAX_SYNC_WAIT	176	//Bursts between SCH before reverting to WAIT_FCCH. (TODO: 88 should be max?) 

#define MAX_CORR_DIST   7	// 4 + 3 =  1/2 GUARD + TAIL
#define SCH_CORR_THRESHOLD	0.80
#define FCCH_CORR_THRESHOLD 0.90
#define NORM_CORR_THRESHOLD 0.80

#define	FCCH_HITS_NEEDED	(USEFUL_BITS - 4)
#define FCCH_MAX_MISSES		1

enum EQ_TYPE {
	EQ_NONE,
	EQ_FIXED_LINEAR,
	EQ_ADAPTIVE_LINEAR,
	EQ_FIXED_DFE,
	EQ_ADAPTIVE_DFE,
	EQ_VITERBI
};

class gsm_burst;

class gsm_burst
{
protected:
	
	gsm_burst();  

	//Burst Buffer: Storage for burst data
	float			d_burst_buffer[BBUF_SIZE];
	int				d_bbuf_pos;			//write position
	int				d_burst_start;		//first useful bit (beginning of output)
	unsigned long   d_sample_count;		//sample count at end (TODO:beginning) of BBUF (bit count if external clock)
	unsigned long   d_last_burst_s_count;		//sample count from previous burst

	///// Sync/training sequence correlation
	//TODO: need all sync patterns
	float			corr_sync[N_SYNC_BITS];	//encoded sync bits for correlation
	float			corr_train_seq[10][N_TRAIN_BITS];
//	float 			d_corr[50];	
	const float		*d_corr_pattern;
	int				d_corr_pat_size;
	float 			d_corr_max;
	int 			d_corr_maxpos;
	int				d_corr_center;
		
	///// Burst information
	SYNC_STATE		d_sync_state;
	BURST_TYPE		d_burst_type;
	unsigned		d_ts;					//timeslot 0-7
	unsigned long   d_burst_count;			//Bursts received starting w/ initial FCCH reset after lost sync
	unsigned long   d_last_sch;				//Burst count of last SCH
	float			d_freq_offset;
	int				d_color_code;

	//////// Methods
	int				get_burst(void);
	BURST_TYPE		get_fcch_burst(void);
	BURST_TYPE		get_sch_burst(void);
	BURST_TYPE		get_norm_burst(void);
	
	void	shift_burst(int);
	void	calc_freq_offset(void); 
	void	equalize(void);
	float	correlate_pattern(const float *,const int,const int,const int);

	void	print_bits(const float *data,int length);
	void	print_burst(void);

	void	diff_encode(const float *in,float *out,int length,float lastbit = 1.0);	
	void	diff_decode(const float *in,float *out,int length,float lastbit = 1.0);	

		
public:
	~gsm_burst ();	

	////// General Stats
	//TODO: Maybe there should be a burst_stats class?
	long			d_sync_loss_count;
	long			d_fcch_count;
	long			d_part_sch_count;
	long			d_sch_count;
	long			d_normal_count;
	long			d_dummy_count;
	long			d_unknown_count;
	
	////// Options
	unsigned long	d_clock_options;
	unsigned long	d_print_options;
	EQ_TYPE			d_equalizer_type;
	
	int sync_state() { return d_sync_state;}
	float freq_offset() {return d_freq_offset;}
	
	//Methods
	//TODO: reset state (e.g. channel change)
	//void reset(void);
};


#endif /* INCLUDED_GSM_BURST_H */
