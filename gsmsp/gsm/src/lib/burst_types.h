// $Id: burst_types.h,v 1.5 2007/03/14 05:44:53 jl Exp $

#pragma once

#include "gsm_constants.h"

static const int TB_LEN	= 3;
static const int TB_OS1	= 0;
static const int TB_OS2	= 145;
static const unsigned char tail_bits[TB_LEN] = {0, 0, 0};

/*
 * The normal burst is used to carry information on traffic and control
 * channels.
 */
static const int N_TSC_NUM	= 8;	// number of training sequence codes
static const int N_TSC_CODE_LEN	= 26;	// length of tsc
static const int N_TSC_OS	= 61;	// tsc offset
static const int N_EDATA_LEN_1	= 58;	// length of first data section
static const int N_EDATA_OS_1	= 3;	// offset of first data section
static const int N_EDATA_LEN_2	= 58;	// length of second data section
static const int N_EDATA_OS_2	= 87;	// offset of second data section
static const unsigned char n_tsc[N_TSC_NUM][N_TSC_CODE_LEN] = {
/* 0 */	{
		0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0,
		0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1
	},
/* 1 */	{
		0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1,
		1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1 
	},
/* 2 */	{
		0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1,
		0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0
	},
/* 3 */	{
		0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0,
		1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0
	},
/* 4 */	{
		0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0,
		1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1
	},
/* 5 */	{
		0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0,
		0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0
	},
/* 6 */	{
		1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1,
		0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1
	},
/* 7 */	{
		1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0,
		0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0
	}
};


/*
 * The frequency correction burst is used for frequency synchronization
 * of the mobile.  This is broadcast in TS0 together with the SCH and
 * BCCH.
 *
 * Modulating the bits below causes a spike at 62.5kHz above (below for
 * COMPACT) the center frequency.  One can use this spike with a narrow
 * band filter to accurately determine the center of the channel.
 */
static const int FC_CODE_LEN	= 142;
static const int FC_OS		= 3;
static const unsigned char fc_fb[FC_CODE_LEN] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const unsigned char fc_compact_fb[FC_CODE_LEN] = {
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0
};


/*
 * The synchronization burst is used for time synchronization of the
 * mobile.  The bits given below were chosen for their correlation
 * properties.  The synchronization channel (SCH) contains a long
 * training sequence (given below) and carries the TDMA frame number and
 * base station identity code.  It is broadcast in TS0 in the frame
 * following the frequency correction burst.
 */
static const int SB_CODE_LEN	= 64;
static const int SB_ETS_OS	= 42;
static const int SB_EDATA_LEN_1	= 39;
static const int SB_EDATA_OS_1	= 3;
static const int SB_EDATA_LEN_2	= 39;
static const int SB_EDATA_OS_2	= 106;
static const unsigned char sb_etsc[SB_CODE_LEN] = {
	1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
	0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1,
	0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1
};

static const unsigned char sb_cts_etsc[SB_CODE_LEN] = {
	1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1,
	0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0,
	1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0,
	1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1
};

static const unsigned char sb_compact_etsc[SB_CODE_LEN] = {
	1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1,
	0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
	0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0
};


/*
 * A base tranceiver station must transmit a burst in every timeslot of
 * every TDMA frame in channel C0.  The dummy burst will be transmitted
 * on all timeslots of all TDMA frames for which no other channel
 * requires a burst to be transmitted.
 */
static const int D_CODE_LEN	= 142;
static const int D_MB_OS	= 3;
static const unsigned char d_mb[D_CODE_LEN] = {
	1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0,
	0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0,
	0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
	0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0,
	0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0,
	0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0,
	0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1,
	1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1,
	0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0
};


/*
 * The access burst is used for random access from a mobile.
 */
static const int AB_ETB_CODE_LEN	= 8;
static const int AB_ETB_OS		= 0;
static const unsigned char ab_etb[AB_ETB_CODE_LEN] = {
	0, 0, 1, 1, 1, 0, 1, 0
};

static const int AB_SSB_CODE_LEN	= 41;
static const int AB_SSB_OS		= 8;
static const unsigned char ab_ssb[AB_SSB_CODE_LEN] = {
	0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0,
	0, 0, 1, 1, 1, 1, 0, 0, 0
};

static const unsigned char ab_ts1_ssb[AB_SSB_CODE_LEN] = {
	0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
	1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1,
	0, 0, 1, 0, 0, 1, 1, 0, 1
};

static const unsigned char ab_ts2_ssb[AB_SSB_CODE_LEN] = {
	1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1,
	0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1,
	1, 0, 1, 1, 1, 0, 1, 1, 1
};


typedef enum {
	burst_n_0,
	burst_n_1,
	burst_n_2,
	burst_n_3,
	burst_n_4,
	burst_n_5,
	burst_n_6,
	burst_n_7,
	burst_fc,
	burst_fc_c,
	burst_s,
	burst_s_cts,
	burst_s_c,
	burst_d,
	burst_a,
	burst_a_ts1,
	burst_a_ts2,
	burst_not_a_burst
} burst_t;

static const int N_BURST_TYPES	= ((int)(burst_not_a_burst) + 1);
