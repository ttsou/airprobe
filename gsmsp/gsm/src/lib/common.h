#ifndef __GSMSP_COMMON_H__
#define __GSMSP_COMMON_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>

#define MIN(a,b)	((a)<(b)?(a):(b))
#define MAX(a,b)	((a)>(b)?(a):(b))

/* DISABLE me for release build. Otherwise with debug output. */
//#define GSMSP_DEBUG 1

#ifdef GSMSP_DEBUG
# define DEBUGF(a...)	do { \
	fprintf(stderr, "DEBUG %s:%d ", __func__, __LINE__); \
	fprintf(stderr, a); \
} while (0)
#else
# define DEBUGF(a...)
#endif

# define HEXDUMPF(data, len, a...)	do { \
	printf("HEX %s:%d ", __func__, __LINE__); \
	printf(a); \
	hexdump(data, len); \
} while (0)

void hexdump(const unsigned char *data, size_t len);

#ifndef GSMDECODE
#include "interleave.h"
struct _opt
{
	INTERLEAVE_CTX ictx;
};
#endif

#ifdef __cplusplus
}
#endif

#endif /* !__GSMSP_COMMON_H__ */


