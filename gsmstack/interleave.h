/* This file was taken from gsm-tvoid */
/*
 * $Id:$
 */

#ifndef __GSMSP_INTERLEAVE_H__
#define __GSMSP_INTERLEAVE_H__ 1

typedef struct _interleave_ctx
{
	unsigned short *trans;
	int trans_size;
} INTERLEAVE_CTX;

int interleave_init(INTERLEAVE_CTX *ictx, int size, int block_size);
int interleave_deinit(INTERLEAVE_CTX *ictx);
void interleave_decode(INTERLEAVE_CTX *ictx, unsigned char *dst, unsigned char *src);

#endif
