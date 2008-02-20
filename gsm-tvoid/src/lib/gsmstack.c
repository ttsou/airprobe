/*
 * Invoke gsmstack() with any kind of burst. Automaticly decode and retrieve
 * information.
 */
#include "system.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gsmstack.h"
#include "gsm_constants.h"
#include "interleave.h"
#include "sch.h"

INTERLEAVE_CTX ictx;

static void
diff_decode(char *dst, char *src, int len)
{
	const unsigned char *end = src + len;
	unsigned char last;

	src += 3;
	last = 0;
	memset(dst, 0, 3);
	dst += 3;

	while (src < end)
	{
		*dst = !*src ^ last;
		last = *dst;
		src++;
		dst++;
	}
}

/*
 * Initialize a new GSMSTACK context.
 */
int
GS_new(GS_CTX *ctx)
{
	memset(ctx, 0, sizeof *ctx);
	interleave_init(&ictx, 456, 114);

	return 0;
}

/*
 * 156 bit
 */
int
GS_process(GS_CTX *ctx, int ts, int type, char *data)
{
	int fn;
	int bsic;
	int ret;
	char buf[156];

	diff_decode(buf, data, 156);

	switch (type)
	{
	case SCH:
		ret = decode_sch(buf, &fn, &bsic);
		if (ret != 0)
			break;
		DEBUGF("FN %d, BSIC %d\n", fn, bsic);
		break;
	case NORMAL:
		break;
	}
}


