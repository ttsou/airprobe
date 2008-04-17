#ifndef __GSMSP_COMMON_H__
#define __GSMSP_COMMON_H__ 1

#ifdef HAVE_CONFIG_H
#include <config.h>
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

/* True if bit at position 'pos' in 'data' is set */
#define BIT(data, pos)		((data) >> (pos)) & 1

# define HEXDUMPF(data, len, a...)	do { \
	printf("HEX %s:%d ", __func__, __LINE__); \
	printf(a); \
	hexdump(data, len); \
} while (0)

void hexdump(const unsigned char *data, size_t len);

struct _opt
{
	char format;
	char flags;
};

#define MSG_FORMAT_BBIS		(1)
#define MSG_FORMAT_B		(2)
#define MSG_FORMAT_XML		(3)

#define FL_MOTOROLA		(0x01)

#endif /* !__GSMSP_COMMON_H__ */


