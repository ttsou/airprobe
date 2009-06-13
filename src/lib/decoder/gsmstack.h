//TODO: this file shouldn't be part of the GSM Receiver
#ifndef __GSMSTACK_H__
#define __GSMSTACK_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/if_ether.h>
#include "interleave.h"

struct gs_ts_ctx {
	/* FIXME: later do this per each ts per each arfcn */
	unsigned char burst[4 * 58 * 2];
	int burst_count;
};

typedef struct
{
	int flags;
	int fn;
	int bsic;
	char msg[23];	/* last decoded message */

	INTERLEAVE_CTX interleave_ctx;

	struct gs_ts_ctx ts_ctx[8];

	int tun_fd;
	unsigned char ether_addr[ETH_ALEN];

//	int pcap_fd;
//	int burst_pcap_fd;
} GS_CTX;

int GS_new(GS_CTX *ctx);
int GS_process(GS_CTX *ctx, int ts, int type, const unsigned char *src, int fn);

#ifdef __cplusplus
}
#endif

#endif
