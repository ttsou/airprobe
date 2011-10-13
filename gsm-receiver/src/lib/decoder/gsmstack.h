//TODO: this file shouldn't be part of the GSM Receiver
#ifndef __GSMSTACK_H__
#define __GSMSTACK_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

//#include <linux/if_ether.h>
#include <inttypes.h>
#include "interleave.h"

enum BURST_TYPE {
        UNKNOWN = 0,
        FCCH,
        PARTIAL_SCH,            //successful correlation, but missing data ^
        SCH,
        CTS_SCH,
        COMPACT_SCH,
        NORMAL,
        DUMMY,
        ACCESS
};

enum TIMESLOT_TYPE {
	TST_UNKNOWN = 0,
	TST_OFF, /* timeslot is not decoded */
	TST_FCCH_SCH_BCCH_CCCH_SDCCH4,
	TST_FCCH_SCH_BCCH_CCCH,
	TST_SDCCH8,
	TST_TCHF,
	TST_PDCH
};

struct gs_ts_ctx {
	/* FIXME: later do this per each ts per each arfcn */
	unsigned char burst[4 * 58 * 2];
	unsigned char burst2[8 * 58 * 2]; /* buffer for FACCH on TCH */
	int burst_count;
	int burst_count2; /* counter for FACCH on TCH */
	enum TIMESLOT_TYPE type;
};

struct gsmtap_inst;
// Defines and header structure from gprsdecode
#define ARFCN_UPLINK 0x4000
#define BI_FLG_DUMMY    (1<<4)
#define BI_FLG_SACCH    (1<<5)
#define RSL_CHAN_Bm_ACCHs  0x08
struct l1ctl_burst_ind {
    uint32_t frame_nr;
    uint16_t band_arfcn;    /* ARFCN + band + ul indicator               */
    uint8_t chan_nr;        /* GSM 08.58 channel number (9.3.1)          */
    uint8_t flags;          /* BI_FLG_xxx + burst_id = 2LSBs             */
    uint8_t rx_level;       /* 0 .. 63 in typical GSM notation (dBm+110) */
    uint8_t snr;            /* Reported SNR >> 8 (0-255)                 */
    uint8_t bits[15];       /* 114 bits + 2 steal bits. Filled MSB first */
} __attribute__((packed));

typedef struct
{
	int flags;
	int fn;
	int bsic;
	char msg[23];	/* last decoded message */

	INTERLEAVE_CTX interleave_ctx;
	INTERLEAVE_CTX interleave_facch_f1_ctx;
	INTERLEAVE_CTX interleave_facch_f2_ctx;

	struct gs_ts_ctx ts_ctx[8];

	struct gsmtap_inst *gsmtap_inst;

	// GPRS fields
    FILE *gprsdecode_file;
	struct l1ctl_burst_ind *gprsdecode_burst;
} GS_CTX;

int GS_new(GS_CTX *ctx, int init_gprs);
int GS_process(GS_CTX *ctx, int ts, int type, const unsigned char *src, int fn, int first_burst);

#ifdef __cplusplus
}
#endif

#endif
