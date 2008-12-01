#ifndef _GSMSTACK_H
#define _GSMSTACK_H

#define NR_TIMESLOTS	8
#define NR_ARFCN	1024

#define USEFUL_BITS	142
#define BURST_BITS	(USEFUL_BITS/8 + 1)

#include "gsmtap.h"
enum gsm_burst_type {
	GSM_BURST_UNKNOWN = GSMTAP_BURST_UNKNOWN,
	GSM_BURST_FCCH = GSMTAP_BURST_FCCH,
	GSM_BURST_PARTIAL_SCH = GSMTAP_BURST_PARTIAL_SCH,
	GSM_BURST_SCH = GSMTAP_BURST_SCH,
	GSM_BURST_CTS_SCH = GSMTAP_BURST_CTS_SCH,
	GSM_BURST_COMPACT_SCH = GSMTAP_BURST_COMPACT_SCH,
	GSM_BURST_NORMAL = GSMTAP_BURST_NORMAL,
	GSM_BURST_DUMMY = GSMTAP_BURST_DUMMY,
	GSM_BURST_ACCESS = GSMTAP_BURST_ACCESS,
	GSM_BURST_NONE = GSMTAP_BURST_NONE,
	_GSM_BURST_CNT
};


struct gsm_burst {
	/* time at which we were received */
	struct timeval rx_time;

	/* the physical channel which we're part of.
	 * always guaranteed to be != NULL */
	struct gsm_phys_chan *phys_chan;

	/* the logical channel to which we belong.
	 * only filled-in if we actually know about it */
	struct gsm_logi_chan *logi_chan;

	/* the burst type (as per gsmtap.h) */
	unsigned char burst_type;

	/* the relative receive TDMA frame */
	unsigned int rx_frame_nr;

	/* the timeslot number is part of the phys_chan */

	/* the burst after differential decode, 8 bit per byte */
	unsigned char decoded[BURST_BITS];

	/* the burst after differential decode, 1 bit per byte */
	unsigned char decoded_bits[USEFUL_BITS];
};

struct gsm_burst_stats {
	unsigned int rx_total;
	unsigned int rx_type[_GSM_BURST_CNT];
};

enum gsm_logical_channel_type {
	GSM_LCHAN_UNKNOWN,
	GSM_LCHAN_NONE,

	/* CBCH */
	GSM_LCHAN_FCCH,		/* Frequency Correction CH */
	GSM_LCHAN_SCH,		/* Synchronization CH */
	GSM_LCHAN_BCCH,		/* Broadcast Control CH */
	GSM_LCHAN_PCH,		/* Paging CH */
	GSM_LCHAN_NCH,		/* Notification CH */
	GSM_LCHAN_AGCH,		/* Access Grant CH */
	GSM_LCHAN_CBCH,		/* Cell Broadcast CH */

	/* SDCCH */
	GSM_LCHAN_SDCCH8,	/* Slow Dedicated Control CH */
	GSM_LCHAN_SACCH8C,	/* Slow Associated Control CH */

	/* TCH */
	GSM_LCHAN_TCH_F,	/* Traffic CH */
	GSM_LCHAN_TCH_H,	/* Traffic CH */
	GSM_LCHAN_SACCH,	/* Slow Associated Control CH */

	/* uplink */
	GSM_LCHAN_RACH,		/* Random Access CH */
};

struct gsm_logi_chan {
	enum gsm_logical_channel_type type;
	/* here we aggregate the bursts for this logical channel
	 * until we have found enough bursts for one MAC block */
	struct gsm_burst burst_buf[4];
	int next_burst;

	struct gsm_burst_stats stats;
};

enum gsm_phys_chan_config {
	GSM_PCHAN_UNKNOWN,
	GSM_PCHAN_CCCH,
	GSM_PCHAN_CCCH_WITH_SDCCH8,
	GSM_PCHAN_TCH_F,
	GSM_PCHAN_TCH_H,
	GSM_PCHAN_SDCCH8_SACCH8C,
};

/* maximum logical channels for one physical channel */
#define NR_LOGI_CHANS	16

/* A GSM physical channel configuration */
struct gsm_phys_chan {
	/* in which timeslot is this channel? */
	unsigned int timeslot;
	/* to which RF channel do we belong? */
	struct gsm_rf_chan *rf_chan;
	/* how is our physical configuration */
	enum gsm_phys_chan_config config;
	/* those are our logical channels */
	struct gsm_logi_chan logi_chan[NR_LOGI_CHANS];
	int nr_logi_chans;

	struct gsm_burst_stats stats;
};

struct gsm_rf_chan {
	/* ARFCN (frequency) of the RF channel */
	unsigned int arfcn;

	/* current RFN as determined by SCH + frame count */
	unsigned int rfn;

	/* the physical channel for each timeslot */
	struct gsm_phys_chan phys_chan[NR_TIMESLOTS];
};


extern struct gsm_rf_chan *gsm_init_rfchan(unsigned int arfcn);

#endif /* _GSMSTACK_H */
