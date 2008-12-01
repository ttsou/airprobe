
/* GSM TCH/F channel coding 
 *
 * (C) 2008 by Harald Welte <laforge@gnumonks.org>
 */

#include "system.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

//#include "burst_types.h"
#include "tch.h"
//#include "fire_crc.h"

/*
 * GSM TCH/F -- Traffic Channel (Full Rate)
 *
 * 	Input: 260 bits (182 class-1, 78 class-2 bits)
 * 	
 *      1. Rearrange according to Table 2 (TS 05.03)
 * 	2. Add 3 parity bits for first 50 class-1 bits (d0...49)
 *	3. Add four tailing bits to class-1. (Output 182 + 3 + 4 = 189 bit)
 * 	4. Convolutional encode of class-1 (Output = 189 * 2 = 378 bit)
 *      5. Append class-2 bits (Output = 378 + 78 = 456bit)
 * 	3. Interleave. (Output 456 bit)
 * 	4. Map on bursts. (4 x 156 bit bursts with each 2x57 bit content data)
 */


#if 0
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
#endif

static unsigned char *decode_tch_f(GS_CTX *ctx, unsigned char *burst,
				   unsigned int *datalen)
{
	int errors, len, data_size;
	unsigned char conv_data[CONV_SIZE], iBLOCK[BLOCKS][iBLOCK_SIZE],
		      hl, hn, decoded_data[PARITY_OUTPUT_SIZE];
	//FC_CTX fc_ctx;

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
	//decode_interleave(conv_data, (unsigned char *)iBLOCK);

	// Viterbi decode of class-1 bits
	errors = conv_decode(decoded_data, conv_data, CONV_INPUT_SIZE_TCH_F);
	if (errors) {
		DEBUGF("conv_decode: %d\n", errors);
		return NULL;
	}
	// reordering + remove four tailing bits (185..188)
	for (i = 0; i <= 90; i++) {
		ctx->msg[2*i] = decoded_data[i];
		ctx->msg[2*i+1] = decoded_data[184-i];
	}
	len = 182;
	// check 3 bit parity (91,92,93) of class-1 bits
	/* FIXME */
	// append class-2 bits
	memcpy(ctx->msg+185, conv_data+(2*CONV_INPUT_SIZE_TCH_F), 78);
	len += 78; 	/* should be 260 bits now */

	if (len < data_size) {
		fprintf(stderr, "error: buf too small (%d < %d)\n",
		   sizeof(ctx->msg), len);
		return NULL;
	}

	if (datalen)
		*datalen = (unsigned int)len;
	return ctx->msg;
}
