/* This file was taken from gsm-tvoid */

#include "system.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "burst_types.h"
#include "cch.h"
#include "conv.h"
#include "fire_crc.h"

/*
 * GSM SACCH -- Slow Associated Control Channel
 *
 * These messages are encoded exactly the same as on the BCCH.
 * (Broadcast Control Channel.)
 *
 * 	Input: 184 bits
 * 	
 * 	1. Add parity and flushing bits. (Output 184 + 40 + 4 = 228 bit)
 * 	2. Convolutional encode. (Output 228 * 2 = 456 bit)
 * 	3. Interleave. (Output 456 bit)
 * 	4. Map on bursts. (4 x 156 bit bursts with each 2x57 bit content data)
 */


/*
 * Parity (FIRE) for the GSM SACCH channel.
 *
 * 	g(x) = (x^23 + 1)(x^17 + x^3 + 1)
 * 	     = x^40 + x^26 + x^23 + x^17 + x^3 + 1
 */

static const unsigned char parity_polynomial[PARITY_SIZE + 1] = {
   1, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 1, 0,
   0, 1, 0, 0, 0, 0, 0, 1,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 1, 0, 0,
   1
};

// remainder after dividing data polynomial by g(x)
static const unsigned char parity_remainder[PARITY_SIZE] = {
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1
};


/*
static void parity_encode(unsigned char *d, unsigned char *p) {

	int i;
	unsigned char buf[DATA_BLOCK_SIZE + PARITY_SIZE], *q;

	memcpy(buf, d, DATA_BLOCK_SIZE);
	memset(buf + DATA_BLOCK_SIZE, 0, PARITY_SIZE);

	for(q = buf; q < buf + DATA_BLOCK_SIZE; q++)
		if(*q)
			for(i = 0; i < PARITY_SIZE + 1; i++)
				q[i] ^= parity_polynomial[i];
	for(i = 0; i < PARITY_SIZE; i++)
		p[i] = !buf[DATA_BLOCK_SIZE + i];
}
 */


static int parity_check(unsigned char *d) {

	unsigned int i;
	unsigned char buf[DATA_BLOCK_SIZE + PARITY_SIZE], *q;

	memcpy(buf, d, DATA_BLOCK_SIZE + PARITY_SIZE);

	for(q = buf; q < buf + DATA_BLOCK_SIZE; q++)
		if(*q)
			for(i = 0; i < PARITY_SIZE + 1; i++)
				q[i] ^= parity_polynomial[i];
	return memcmp(buf + DATA_BLOCK_SIZE, parity_remainder, PARITY_SIZE);
}

static void decode_burstmap(unsigned char *iBLOCK, unsigned char *eBLOCK,
   unsigned char *hl, unsigned char *hn) {

	int j;

	for(j = 0; j < 57; j++) {
		iBLOCK[j] = eBLOCK[j];
		iBLOCK[j + 57] = eBLOCK[j + 59];
	}
	*hl = eBLOCK[57];
	*hn = eBLOCK[58];
}


/*
 * Transmitted bits are sent least-significant first.
 */
static int compress_bits(unsigned char *dbuf, unsigned int dbuf_len,
   unsigned char *sbuf, unsigned int sbuf_len) {

	unsigned int i, j, c, pos = 0;

	if(dbuf_len < ((sbuf_len + 7) >> 3))
		return -1;

	for(i = 0; i < sbuf_len; i += 8) {
		for(j = 0, c = 0; (j < 8) && (i + j < sbuf_len); j++)
			c |= (!!sbuf[i + j]) << j;
		dbuf[pos++] = c & 0xff;
	}
	return pos;
}


#if 0
int get_ns_l3_len(unsigned char *data, unsigned int datalen) {

	if((data[0] & 3) != 1) {
		fprintf(stderr, "error: get_ns_l3_len: pseudo-length reserved "
		   "bits bad (%2.2x)\n", data[0] & 3);
		return -1;
	}
	return (data[0] >> 2);
}

#endif

/*
 * decode_cch
 *
 * 	Decode a "common" control channel.  Most control channels use
 * 	the same burst, interleave, Viterbi and parity configuration.
 * 	The documentation for the control channels defines SACCH first
 * 	and then just keeps referring to that.
 *
 * 	The current (investigated) list is as follows:
 *
 * 		BCCH Norm
 * 		BCCH Ext
 * 		PCH
 * 		AGCH
 * 		CBCH (SDCCH/4)
 * 		CBCH (SDCCH/8)
 * 		SDCCH/4
 * 		SACCH/C4
 * 		SDCCH/8
 * 		SACCH/C8
 *
 * 	We provide two functions, one for where all four bursts are
 * 	contiguous, and one where they aren't.
 */

static unsigned char *decode_cch(GS_CTX *ctx, unsigned char *burst,
				 unsigned int *datalen)
{

	int errors, len, data_size;
	unsigned char conv_data[CONV_SIZE], iBLOCK[BLOCKS][iBLOCK_SIZE],
		      hl, hn, decoded_data[PARITY_OUTPUT_SIZE];
	FC_CTX fc_ctx;

	data_size = sizeof ctx->msg;
	if (datalen)
		*datalen = 0;

	// unmap the bursts
	decode_burstmap(iBLOCK[0], burst, &hl, &hn); // XXX ignore stealing bits
	decode_burstmap(iBLOCK[1], burst + 116, &hl, &hn);
	decode_burstmap(iBLOCK[2], burst + 116 * 2, &hl, &hn);
	decode_burstmap(iBLOCK[3], burst + 116 * 3, &hl, &hn);

	// remove interleave
	interleave_decode(&ctx->interleave_ctx, conv_data, (unsigned char *)iBLOCK);

	// Viterbi decode
	errors = conv_decode(decoded_data, conv_data, CONV_INPUT_SIZE_CCH);
	if (errors) {
		DEBUGF("conv_decode: %d\n", errors);
		return NULL;
	}

	// check parity
	// If parity check error detected try to fix it.
	if (parity_check(decoded_data))
	{
		FC_init(&fc_ctx, 40, 184);
		unsigned char crc_result[224];
		if (FC_check_crc(&fc_ctx, decoded_data, crc_result) == 0)
		{
			errors = -1;
			DEBUGF("error: sacch: parity error (%d)\n", errors);
			return NULL;
		} else {
			DEBUGF("Successfully corrected parity bits!\n");
			memcpy(decoded_data, crc_result, sizeof crc_result);
			errors = 0;
		}
	}

	if((len = compress_bits(ctx->msg, data_size, decoded_data,
	   DATA_BLOCK_SIZE)) < 0) {
		fprintf(stderr, "error: compress_bits\n");
		return NULL;
	}
	if(len < data_size) {
		fprintf(stderr, "error: buf too small (%d < %d)\n",
		   sizeof(ctx->msg), len);
		return NULL;
	}

	if(datalen)
		*datalen = (unsigned int)len;
	return ctx->msg;
}
