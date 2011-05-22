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

#include <osmocom/core/msgb.h>
#include <osmocom/core/gsmtap.h>
#include <osmocom/core/gsmtap_util.h>

static const int USEFUL_BITS = 142;

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

/* TODO: handle mapping in a more elegant way or simplify the function */

uint8_t
get_chan_type(enum TIMESLOT_TYPE type, int fn, uint8_t *ss)
{
  uint8_t chan_type = GSMTAP_CHANNEL_BCCH;
  *ss = 0;
  int mf51 = fn % 51;

  if(type == TST_FCCH_SCH_BCCH_CCCH_SDCCH4)
  {
      if(mf51 == 22) /* SDCCH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH4;
          *ss = 0;
      }
      else if(mf51 == 26) /* SDCCH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH4;
          *ss = 1;
      }
      else if(mf51 == 32) /* SDCCH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH4;
          *ss = 2;
      }
      else if(mf51 == 36) /* SDCCH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH4;
          *ss = 3;
      }
      else if(mf51 == 42) /* SAACH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH4 | GSMTAP_CHANNEL_ACCH;
          *ss = ((fn % 102) > 51) ? 2 :  0;
      }
      else if(mf51 == 46) /* SAACH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH4 | GSMTAP_CHANNEL_ACCH;
          *ss = ((fn % 102) > 51) ? 3 :  1;
      }
  }
  else if(type == TST_FCCH_SCH_BCCH_CCCH)
  {
      if(mf51 != 2) /* BCCH */
      {
          chan_type = GSMTAP_CHANNEL_CCCH;
          *ss = 0;
      }
  }
  else if(type == TST_SDCCH8)
  {
      if(mf51 == 0) /* SDCCH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH8;
          *ss = 0;
      }
      else if(mf51 == 4) /* SDCCH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH8;
          *ss = 1;
      }
      else if(mf51 == 8) /* SDCCH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH8;
          *ss = 2;
      }
      else if(mf51 == 12) /* SDCCH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH8;
          *ss = 3;
      }
      else if(mf51 == 16) /* SDCCH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH8;
          *ss = 4;
      }
      else if(mf51 == 20) /* SDCCH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH8;
          *ss = 5;
      }
      else if(mf51 == 24) /* SDCCH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH8;
          *ss = 6;
      }
      else if(mf51 == 28) /* SDCCH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH8;
          *ss = 7;
      }
      else if(mf51 == 32) /* SAACH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH8 | GSMTAP_CHANNEL_ACCH;
          *ss = ((fn % 102) > 51) ? 4 :  0;
      }
      else if(mf51 == 36) /* SAACH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH8 | GSMTAP_CHANNEL_ACCH;
          *ss = ((fn % 102) > 51) ? 5 :  1;
      }
      else if(mf51 == 40) /* SAACH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH8 | GSMTAP_CHANNEL_ACCH;
          *ss = ((fn % 102) > 51) ? 6 :  2;
      }
      else if(mf51 == 44) /* SAACH */
      {
          chan_type = GSMTAP_CHANNEL_SDCCH8 | GSMTAP_CHANNEL_ACCH;
          *ss = ((fn % 102) > 51) ? 7 :  3;
      }
  }
  else if (type == TST_TCHF) {
    chan_type = GSMTAP_CHANNEL_TCH_F | GSMTAP_CHANNEL_ACCH;
  }

  return chan_type;
}

/*
 * Initialize a new GSMSTACK context.
 */
int
GS_new(GS_CTX *ctx)
{
	struct sockaddr_in sin;

	sin.sin_family = AF_INET;
	sin.sin_port = htons(GSMTAP_UDP_PORT);
	inet_aton("127.0.0.1", &sin.sin_addr);

	memset(ctx, 0, sizeof *ctx);
	interleave_init(&ctx->interleave_ctx, 456, 114);
	interleave_init_facch_f(&ctx->interleave_facch_f1_ctx, 456, 114, 0);
	interleave_init_facch_f(&ctx->interleave_facch_f2_ctx, 456, 114, 4);
	ctx->fn = -1;
	ctx->bsic = -1;

	ctx->gsmtap_inst = gsmtap_source_init("127.0.0.1", GSMTAP_UDP_PORT, 0);
	if (!ctx->gsmtap_inst) {
		perror("creating gsmtap socket\n");
		return -EIO;
	}
	/* Add a local sink to the existing GSMTAP source */
	gsmtap_source_add_sink(ctx->gsmtap_inst);

	return 0;
}

#define BURST_BYTES	((USEFUL_BITS/8)+1)
/*
 * 142 bit
 */
int
GS_process(GS_CTX *ctx, int ts, int type, const unsigned char *src, int fn, int first_burst)
{
	int bsic;
	int ret;
	unsigned char *data;
	int len;
	struct gs_ts_ctx *ts_ctx = &ctx->ts_ctx[ts];

	memset(ctx->msg, 0, sizeof(ctx->msg));

	if (ts_ctx->type == TST_TCHF && type == NORMAL &&
	    (fn % 26) != 12 && (fn % 26) != 25) {
		/* Dieter: we came here because the burst might contain FACCH bits */
		ctx->fn = fn;

		/* get burst index to TCH bursts only */
		ts_ctx->burst_count2 = fn % 26;

		if (ts_ctx->burst_count2 >= 12)
			ts_ctx->burst_count2--;
		ts_ctx->burst_count2 = ts_ctx->burst_count2 % 8;

		/* copy data bits and stealing flags to buffer */
		memcpy(ts_ctx->burst2 + (116 * ts_ctx->burst_count2), src, 58);
		memcpy(ts_ctx->burst2 + (116 * ts_ctx->burst_count2) + 58, src + 58 + 26, 58);

		/* Return if not enough bursts for a full gsm message */
		if ((ts_ctx->burst_count2 % 4) != 3)
			return 0;

		data = decode_facch(ctx, ts_ctx->burst2, &len, (ts_ctx->burst_count2 == 3) ? 1 : 0);
		if (data == NULL) {
			DEBUGF("cannot decode FACCH fnr=%d ts=%d\n", ctx->fn, ts);
			return -1;
		}

		out_gsmdecode(0, 0, ts, ctx->fn, data, len);

		if (ctx->gsmtap_inst) {
			struct msgb *msg;
			uint8_t chan_type = GSMTAP_CHANNEL_TCH_F;
			uint8_t ss = 0;
			int fn = (ctx->fn - 3); /*  "- 3" for start of frame */

			msg = gsmtap_makemsg(0, ts, chan_type, ss, ctx->fn, 0, 0, data, len);
			if (msg)
				gsmtap_sendmsg(ctx->gsmtap_inst, msg);
		}
		return 0;
	}

	/* normal burst processing */
	if (first_burst) /* Dieter: it is important to start with the correct burst */
		ts_ctx->burst_count = 0;

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
			DEBUGF("cannot decode fnr=0x%06x (%6d) ts=%d\n", ctx->fn, ctx->fn, ts);
			return -1;
		}
		//DEBUGF("OK TS %d, len %d\n", ts, len);

		out_gsmdecode(0, 0, ts, ctx->fn, data, len);

		if (ctx->gsmtap_inst) {
			/* Dieter: set channel type according to configuration */
			struct msgb *msg;
			uint8_t chan_type = GSMTAP_CHANNEL_BCCH;
			uint8_t ss = 0;
			int fn = (ctx->fn - 3); /*  "- 3" for start of frame */

			chan_type = get_chan_type(ts_ctx->type, fn, &ss);

			/* arfcn, ts, chan_type, ss, fn, signal, snr, data, len */
			msg = gsmtap_makemsg(0, ts, chan_type, ss,
					     ctx->fn, 0, 0, data, len);
			if (msg)
				gsmtap_sendmsg(ctx->gsmtap_inst, msg);
		}

#if 0
		if (ctx->fn % 51 != 0) && ( (((ctx->fn % 51 + 5) % 10 == 0) || (((ctx->fn % 51) + 1) % 10 ==0) ) )
			ready = 1;
#endif
		
		return 0;
	}
}


/*
 * Output data so that it can be parsed from gsmdecode.
 */
static void
out_gsmdecode(char type, int arfcn, int ts, int fn, char *data, int len)
{
	char *end = data + len;

	printf("%6d %d:", (fn + 0), ts);

	/* FIXME: speed this up by first printing into an array */
	while (data < end)
		printf(" %02.2x", (unsigned char)*data++);
	printf("\n");
	fflush(stdout);
}
