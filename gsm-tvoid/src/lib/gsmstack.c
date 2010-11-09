/*
 * Invoke gsmstack() with any kind of burst. Automaticly decode and retrieve
 * information.
 */
#include "system.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "gsmstack.h"
#include "gsm_constants.h"
#include "interleave.h"
#include "sch.h"
#include "cch.h"

#include "out_pcap.h"

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
	memset(ctx, 0, sizeof *ctx);
	interleave_init(&ctx->interleave_ctx, 456, 114);
	ctx->fn = -1;
	ctx->bsic = -1;

	ctx->pcap_fd = open_pcap_file("tvoid.pcap");
	if (ctx->pcap_fd < 0)
		fprintf(stderr, "cannot open PCAP file: %s\n", strerror(errno));

	ctx->burst_pcap_fd = open_pcap_file("tvoid-burst.pcap");
	if (ctx->burst_pcap_fd < 0)
		fprintf(stderr, "cannot open burst PCAP file: %s\n", strerror(errno));

	return 0;
}

#define BURST_BYTES	((USEFUL_BITS/8)+1)
/*
 * 142 bit
 */
int
GS_process(GS_CTX *ctx, int ts, int type, const unsigned char *src)
{
	int fn;
	int bsic;
	int ret;
	unsigned char *data;
	int len;
	struct gs_ts_ctx *ts_ctx = &ctx->ts_ctx[ts];
	unsigned char octified[BURST_BYTES];

	memset(ctx->msg, 0, sizeof(ctx->msg));

	/* write burst to burst PCAP file */
	burst_octify(octified, src, USEFUL_BITS);
	write_pcap_packet(ctx->burst_pcap_fd, 0 /* arfcn */, ts, ctx->fn,
			  1, type, octified, BURST_BYTES);
	
#if 0
	if (ts != 0) {
		/* non-0 timeslots should end up in PCAP */
		data = decode_cch(ctx, ctx->burst, &len);
		if (data == NULL)
			return -1;
		write_pcap_packet(ctx->pcap_fd, 0 /* arfcn */, ts, ctx->fn, data, len);
		return;
	}
#endif

	if (ts == 0) {
		if (type == SCH) {
			ret = decode_sch(src, &fn, &bsic);
			if (ret != 0)
				return 0;
			if ((ctx->bsic > 0) && (bsic != ctx->bsic))
				fprintf(stderr, "WARN: BSIC changed.\n");
			//DEBUGF("FN %d, BSIC %d\n", fn, bsic);
			ctx->fn = fn;
			ctx->bsic = bsic;
			/* Reset message concatenator */
			ts_ctx->burst_count = 0;
			return 0;
		}

		/* If we did not get Frame Number yet then return */
		if (ctx->fn < 0)
			return 0;

		ctx->fn++;
	}

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
		write_pcap_packet(ctx->pcap_fd, 0 /* arfcn */, ts, ctx->fn,
				  0, NORMAL, data, len);
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

