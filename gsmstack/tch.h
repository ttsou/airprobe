#ifndef __GSMSTACK_TCH_H__
#define __GSMSTACK_TCH_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "gsmstack.h"

unsigned char *decode_tch(GS_CTX *ctx, unsigned char *burst, unsigned int *len);

#ifdef __cplusplus
}
#endif

#endif
