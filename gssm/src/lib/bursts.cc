// $Id: bursts.cc,v 1.1.1.1 2007-06-01 04:26:57 jl Exp $

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "burst_types.h"

void display_burst_type(burst_t t) {

	switch(t) {
		case burst_n_0:
		case burst_n_1:
		case burst_n_2:
		case burst_n_3:
		case burst_n_4:
		case burst_n_5:
		case burst_n_6:
		case burst_n_7:
			printf("normal burst %d", t);
			break;
		case burst_fc:
			printf("frequency correction burst");
			break;
		case burst_fc_c:
			printf("frequency correction burst (COMPACT)");
			break;
		case burst_s:
			printf("synchronization burst");
			break;
		case burst_s_cts:
			printf("synchronization burst (CTS)");
			break;
		case burst_s_c:
			printf("synchronization burst (COMPACT)");
			break;
		case burst_d:
			printf("dummy burst");
			break;
		case burst_a:
			printf("access burst");
			break;
		case burst_a_ts1:
			printf("access burst (TS1)");
			break;
		case burst_a_ts2:
			printf("access burst (TS2)");
			break;
		default:
			printf("unknown burst type");
			break;
	}
}


static int burst_diff(const unsigned char *b1, const unsigned char *b2,
   unsigned int l) {

	int d = 0;
	unsigned int i;

	for(i = 0; i < l; i++)
		d += (b1[i] ^ b2[i]);
	return d;
}


int search_fc(unsigned char *buf) {

	return !(burst_diff(buf + TB_OS1, tail_bits, TB_LEN) +
	   burst_diff(buf + TB_OS2, tail_bits, TB_LEN) +
	   burst_diff(buf + FC_OS, fc_fb, FC_CODE_LEN));
}


int is_dummy_burst(const unsigned char *buf) {

	int i;

	for(i = 0; i < D_CODE_LEN; i++)
		if(buf[i + D_MB_OS] != d_mb[i])
			return 0;
	return 1;
}


burst_t search_burst(unsigned char *buf, int max_burst_errors, int *rmin_o) {

	int i, d_burst[N_BURST_TYPES], t_d, d_min, rmin;
	burst_t bt_min;

	for(i = 0; i < N_BURST_TYPES; i++)
		d_burst[i] = max_burst_errors + 1;

	// access burst (uplink only)
	t_d = burst_diff(buf + AB_ETB_OS, ab_etb, AB_ETB_CODE_LEN);
	d_burst[(int)burst_a] =
	   t_d + burst_diff(buf + AB_SSB_OS, ab_ssb, AB_SSB_CODE_LEN);

	d_burst[(int)burst_a_ts1] =
	   t_d + burst_diff(buf + AB_SSB_OS, ab_ts1_ssb, AB_SSB_CODE_LEN);

	d_burst[(int)burst_a_ts2] =
	   t_d + burst_diff(buf + AB_SSB_OS, ab_ts2_ssb, AB_SSB_CODE_LEN);

	// check tail bits
	t_d =
	   burst_diff(buf + TB_OS1, tail_bits, TB_LEN) +
	   burst_diff(buf + TB_OS2, tail_bits, TB_LEN);

	// normal bursts
	for(i = 0; i < N_TSC_NUM; i++)
		d_burst[(int)burst_n_0 + i] =
		   t_d + burst_diff(buf + N_TSC_OS, n_tsc[i], N_TSC_CODE_LEN);

	// frequency correction
	d_burst[(int)burst_fc] =
	   t_d + burst_diff(buf + FC_OS, fc_fb, FC_CODE_LEN);

	d_burst[(int)burst_fc_c] =
	   t_d + burst_diff(buf + FC_OS, fc_compact_fb, FC_CODE_LEN);

	// synchronization burst
	d_burst[(int)burst_s] =
	   t_d + burst_diff(buf + SB_ETS_OS, sb_etsc, SB_CODE_LEN);

	d_burst[(int)burst_s_cts] =
	   t_d + burst_diff(buf + SB_ETS_OS, sb_cts_etsc, SB_CODE_LEN);

	d_burst[(int)burst_s_c] =
	   t_d + burst_diff(buf + SB_ETS_OS, sb_compact_etsc, SB_CODE_LEN);

	// dummy
	d_burst[(int)burst_d] =
	   t_d + burst_diff(buf + D_MB_OS, d_mb, D_CODE_LEN);

	d_burst[(int)burst_not_a_burst] = max_burst_errors;

	rmin = BURST_LENGTH;
	d_min = max_burst_errors + 1;
	bt_min = burst_not_a_burst;
	for(i = 0; i < N_BURST_TYPES; i++) {
		if(d_burst[i] < d_min) {
			bt_min = (burst_t)i;
			d_min = d_burst[i];
		}
		if((d_burst[i] < rmin) && ((burst_t)i != burst_not_a_burst))
			rmin = d_burst[i];
	}
	if(rmin_o)
		*rmin_o = rmin;
	return bt_min;
}
