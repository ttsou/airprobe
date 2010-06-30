/*
 * Invoke gsmstack() with any kind of burst. Automaticly decode and retrieve
 * information.
 */
#include "system.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "gsmstack.h"
//#include "gsm_constants.h"
#include "interleave.h"
//#include "sch.h"
#include "cch.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <osmocore/msgb.h>
#include <osmocore/gsmtap.h>
#include <osmocore/gsmtap_util.h>

static const int USEFUL_BITS = 142;

enum BURST_TYPE {
        UNKNOWN,
        FCCH, 
        PARTIAL_SCH,            //successful correlation, but missing data ^
        SCH,
        CTS_SCH,
        COMPACT_SCH, 
        NORMAL,
        DUMMY,
        ACCESS
};
static void out_gsmdecode(char type, int arfcn, int ts, int fn, char *data, int len);

/* encode a decoded burst (1 bit per byte) into 8-bit-per-byte */
static void burst_octify(unsigned char *dest, 
			 const unsigned char *data, int length)
{
	int bitpos = 0;

	while (bitpos < USEFUL_BITS) {
		unsigned char tbyte;
		int i; 

		tbyte = 0;
		for (i = 0; (i < 8) && (bitpos < length); i++) {
			tbyte <<= 1;
			tbyte |= data[bitpos++];
		}
		if (i < 8)
			tbyte <<= 8 - i;
		*dest++ = tbyte;
	}	
}


#if 0
static void
diff_decode(char *dst, char *src, int len)
{
	const char *end = src + len;
	unsigned char last;

	src += 3;
	last = 0;
	memset(dst, 0, 3);
	dst += 3;

	while (src < end)
	{
		*dst = !*src ^ last;
		last = *dst;
		src++;
		dst++;
	}
}
#endif

/*
 * Initialize a new GSMSTACK context.
 */
int
GS_new(GS_CTX *ctx)
{
	int rc;
	struct sockaddr_in sin;

	sin.sin_family = AF_INET;
	sin.sin_port = htons(GSMTAP_UDP_PORT);
	inet_aton("127.0.0.1", &sin.sin_addr);

	memset(ctx, 0, sizeof *ctx);
	interleave_init(&ctx->interleave_ctx, 456, 114);
	ctx->fn = -1;
	ctx->bsic = -1;
	ctx->gsmtap_fd = -1;

	rc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (rc < 0) {
		perror("creating UDP socket\n");
		return rc;
	}
	ctx->gsmtap_fd = rc;
	rc = connect(rc, (struct sockaddr *)&sin, sizeof(sin));
	if (rc < 0) {
		perror("connectiong UDP socket");
		close(ctx->gsmtap_fd);
		return rc;
	}

	return 0;
}

#define BURST_BYTES	((USEFUL_BITS/8)+1)
/*
 * 142 bit
 */
int
GS_process(GS_CTX *ctx, int ts, int type, const unsigned char *src, int fn)
{
// 	int fn;
	int bsic;
	int ret;
	unsigned char *data;
	int len;
	struct gs_ts_ctx *ts_ctx = &ctx->ts_ctx[ts];

	memset(ctx->msg, 0, sizeof(ctx->msg));

#if 0
	if (ts != 0) {
		/* non-0 timeslots should end up in PCAP */
		data = decode_cch(ctx, ctx->burst, &len);
		if (data == NULL)
			return -1;
//		write_pcap_packet(ctx->pcap_fd, 0 /* arfcn */, ts, ctx->fn, data, len);
		return;
	}
#endif

/*	if (ts == 0) {
		if (type == SCH) {
//			ret = decode_sch(src, &fn, &bsic);
			if (ret != 0)
				return 0;
			if ((ctx->bsic > 0) && (bsic != ctx->bsic))
				fprintf(stderr, "WARN: BSIC changed.\n");
			//DEBUGF("FN %d, BSIC %d\n", fn, bsic);
			ctx->fn = fn;
			ctx->bsic = bsic;
			/* Reset message concatenator */
//			ts_ctx->burst_count = 0;
//			return 0;
//		}

		/* If we did not get Frame Number yet then return */
//		if (ctx->fn < 0)
//			return 0;

//		ctx->fn++;
//	}
        ctx->fn = fn;
	if (type == NORMAL) {
		/* Interested in these frame numbers (cch)
 		 * 2-5, 12-15, 22-25, 23-35, 42-45
 		 * 6-9, 16-19, 26-29, 36-39, 46-49
 		 */
		/* Copy content data into new array */
		//DEBUGF("burst count %d\n", ctx->burst_count);
		memcpy(ts_ctx->burst + (116 * ts_ctx->burst_count), src, 58);
		memcpy(ts_ctx->burst + (116 * ts_ctx->burst_count) + 58, src + 58 + 26, 58);
		ts_ctx->burst_count++;
		/* Return if not enough bursts for a full gsm message */
		if (ts_ctx->burst_count < 4)
			return 0;

		ts_ctx->burst_count = 0;
		data = decode_cch(ctx, ts_ctx->burst, &len);
		if (data == NULL) {
			DEBUGF("cannot decode fnr=0x%08x ts=%d\n", ctx->fn, ts);
			return -1;
		}
		//DEBUGF("OK TS %d, len %d\n", ts, len);

		out_gsmdecode(0, 0, ts, ctx->fn - 4, data, len);

		if (ctx->gsmtap_fd >= 0) {
			struct msgb *msg;
			/* arfcn, ts, chan_type, ss, fn, signal, snr, data, len */
			msg = gsmtap_makemsg(0, ts, GSMTAP_CHANNEL_BCCH, 0,
					     ctx->fn-4, 0, 0, data, len);
			if (msg)
				write(ctx->gsmtap_fd, msg->data,
				      msg->len);
		}

#if 0
		if (ctx->fn % 51 != 0) && ( (((ctx->fn % 51 + 5) % 10 == 0) || (((ctx->fn % 51) + 1) % 10 ==0) ) )
			ready = 1;
#endif
		
		return 0;
	}
}


/*
 * Output data so that it can be parsed from gsmdeocde.
 */
static void
out_gsmdecode(char type, int arfcn, int ts, int fn, char *data, int len)
{
	char *end = data + len;

	/* FIXME: speed this up by first printing into an array */
	while (data < end)
		printf(" %02.2x", (unsigned char)*data++);
	printf("\n");
	fflush(stdout);
}
