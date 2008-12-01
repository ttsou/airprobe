
#ifndef __GSMSTACK_CCH_H__
#define __GSMSTACK_CCH_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gsmstack.h"

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
unsigned char *decode_cch(GS_CTX *ctx, unsigned char *burst, unsigned int *len);

#ifdef __cplusplus
}
#endif

#endif
