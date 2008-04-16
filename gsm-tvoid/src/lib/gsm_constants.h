#ifndef INCLUDED_GSM_CONSTANTS_H
#define INCLUDED_GSM_CONSTANTS_H

#define GSM_SYMBOL_RATE		(1625000.0/6.0)	//symbols per second
#define GSM_SYMBOL_PERIOD	(1.0/GSM_SYMBOL_RATE)	//seconds per symbol

//Burst timing
#define TAIL_BITS		3
#define GUARD_BITS		8	   //8.25
#define DATA_BITS		58	  //size of 1 data block in normal burst
#define N_TRAIN_BITS 	26
#define N_SYNC_BITS		64
#define USEFUL_BITS		142	 //(2*DATA_BITS + N_TRAIN_BITS )
#define FCCH_BITS		USEFUL_BITS

#define TS_BITS			(TAIL_BITS+USEFUL_BITS+TAIL_BITS+GUARD_BITS)  //a full TS (156)
#define TS_PER_FRAME	8
#define FRAME_BITS		(TS_PER_FRAME * TS_BITS + 2)	// +2 for extra 8*0.25 guard bits
#define FCCH_POS		TAIL_BITS
#define SYNC_POS		39
#define TRAIN_POS		58

static const float gaussian_impulse[] = { 0.03455935, 0.39947558, 0.90323022, 0.39947558, 0.03455935};

enum SYNC_STATE {
	WAIT_FCCH, 
	WAIT_SCH_ALIGN,
	WAIT_SCH, 
	SYNCHRONIZED
};

enum BURST_TYPE {
	UNKNOWN,
	FCCH, 
	PARTIAL_SCH,		//successful correlation, but missing data 
	SCH,
	CTS_SCH,
	COMPACT_SCH, 
	NORMAL,
	DUMMY,
	ACCESS
};

	
static const int SYNC_BITS[] = {
	1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 
	0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 
	0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1
};

static const int CTS_SYNC[] = {
	1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 
	0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 
	1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 
	1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1
};

static const int COMPACT_SYNC[] = {
	1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 
	0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 
	0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 
	0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0
};

// Sync             : .+...++.+..+++.++++++.++++++....++.+..+.+.+++.+.+...+..++++..+..
// Diff Encoded Sync: .++..+.+++.+..++.....++.....+...+.+++.+++++..+++++..++.+...+.++.

#define TSC0		0
#define TSC1		1
#define TSC2		2
#define TSC3		3
#define TSC4		4
#define TSC5		5
#define TSC6		6
#define TSC7		7
#define TS_FCCH		8
#define TS_DUMMY	9

static const unsigned char train_seq[10][N_TRAIN_BITS] = {
	{0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1},
	{0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1},
	{0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0},
	{0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0},
	{0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1},
	{0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0},
	{1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1},
	{1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},		//#9 FCCH ;-)
	{0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1}		// DUMMY  
};

//Dummy burst 0xFB 76 0A 4E 09 10 1F 1C 5C 5C 57 4A 33 39 E9 F1 2F A8
static const unsigned char dummy_burst[] = {
	1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 
	1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 
	1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 
	1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 
	0, 0, 0, 1, 0, 0, 0, 0,	0, 0, 
	0, 1, 1, 1, 1, 1, 0, 0, 
	
	0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 
	1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 
	0, 0, 0, 1, 0, 1,
	
	0, 1, 1, 1, 0, 1, 0, 0, 1, 0,
	1, 0, 0, 0, 1, 1, 0, 0, 1, 1,
	0, 0, 1, 1, 1, 0, 0, 1, 1, 1,
	1, 0, 1, 0, 0, 1, 1, 1, 1, 1,
	0, 0, 0, 1, 0, 0, 1, 0, 1, 1,
	1, 1, 1, 0, 1, 0, 1, 0
};

//Diff encoded train_seq
//TSC0: +.++.+++..+...++..++.+++..
//TSC1: +.+++.++..++...+..++.+++..
//TSC2: +++...+..++..+++.++...+..+
//TSC3: +++..+...++.+++..++..+...+
//TSC4: +..+.++++..+.++....+.++++.
//TSC5: +++.+..++++.+....++.+..+++
//TSC6: .+++.+....++.+..++++.+....
//TSC7: ...++...+..++.+++..++...+.
//TSC8: ..........................
//TSC9: ++..+..+++..+..+++..+..+++


#endif /* INCLUDED_GSM_CONSTANTS_H */
