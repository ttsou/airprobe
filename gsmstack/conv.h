/* This file was taken from gsm-tvoid */
#ifndef _GSM_CONV_H
#define _GSM_CONV_H

#define DATA_BLOCK_SIZE		184
#define PARITY_SIZE		40
#define FLUSH_BITS_SIZE		4
#define PARITY_OUTPUT_SIZE (DATA_BLOCK_SIZE + PARITY_SIZE + FLUSH_BITS_SIZE)

#define CONV_INPUT_SIZE_TCH_F	189
#define CONV_INPUT_SIZE_CCH	PARITY_OUTPUT_SIZE
#define CONV_MAX_INPUT_SIZE	PARITY_OUTPUT_SIZE
#define CONV_SIZE		(2 * CONV_MAX_INPUT_SIZE)

#define BLOCKS			4
#define iBLOCK_SIZE		(CONV_SIZE / BLOCKS)
#define eBLOCK_SIZE		(iBLOCK_SIZE + 2)

int conv_decode(unsigned char *output, unsigned char *data,
		unsigned int input_size);
int parity_check(unsigned char *data);
int compress_bits(unsigned char *dbuf, int dlen, unsigned char *src, int len);

#endif /* _GSM_CONV_H */
