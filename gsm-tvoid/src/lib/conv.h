
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

#define DATA_BLOCK_SIZE		184
#define PARITY_SIZE		40
#define FLUSH_BITS_SIZE		4
#define PARITY_OUTPUT_SIZE (DATA_BLOCK_SIZE + PARITY_SIZE + FLUSH_BITS_SIZE)

#define CONV_INPUT_SIZE		PARITY_OUTPUT_SIZE
#define CONV_SIZE		(2 * CONV_INPUT_SIZE)

#define BLOCKS			4
#define iBLOCK_SIZE		(CONV_SIZE / BLOCKS)
#define eBLOCK_SIZE		(iBLOCK_SIZE + 2)

int conv_decode(unsigned char *output, unsigned char *data);
int parity_check(unsigned char *data);
int compress_bits(unsigned char *dbuf, int dlen, unsigned char *src, int len);

