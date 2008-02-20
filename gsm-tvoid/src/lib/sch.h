
#ifndef __GSMSTACK_H__
#define __GSMSTACK_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

int decode_sch(const unsigned char *src, int *fn_o, int *bsic_o);

#ifdef __cplusplus
}
#endif

#endif

