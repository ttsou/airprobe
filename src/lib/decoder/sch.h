
#ifndef __SCH_H__
#define __SCH_H__ 1

#ifdef __cplusplus
extern "C"
{
#endif

  int decode_sch(const unsigned char *buf, int * t1_o, int * t2_o, int * t3_o, int * ncc, int * bcc);

#ifdef __cplusplus
}
#endif

#endif

