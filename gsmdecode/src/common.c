/*
 */

#include "common.h"
#include <stdio.h>
#include <string.h>

void
hexdump(const unsigned char *data, size_t len)
{
        size_t n = 0;
	int line = 0;

	if (!len)
		return;

	printf("%03x: ", line++);
	while (1)
	{
		printf("%2.2x ", data[n++]);
		if (n >= len)
			break;
		if ((n % 8 == 0) && (n % 16 != 0))
			printf(" - "); 
		if (n % 16 == 0)
			printf("\n%03x: ", line++);
	}
	printf("\n");
}

#ifndef HAVE_STRLCPY
/*           
 * bsd'sh strlcpy().
 * The strlcpy() function copies up to size-1 characters from the
 * NUL-terminated string src to dst, NUL-terminating the result.
 * Return: total length of the string tried to create.
 */
size_t
strlcpy(char *dst, const char *src, size_t size)
{
        size_t len = strlen(src);
        size_t ret = len;

        if (size <= 0)
		return 0;
	if (len >= size)
		len = size - 1;
	memcpy(dst, src, len);
	dst[len] = 0;

	return ret;
}
#endif

/*
 * Debuggging...
 * Convert an interger to a bit string and output it.
 * Most significatn bit first.
 */
char *
int2bit(unsigned int val)
{
	static char buf[33 + 3];
	char *ptr = buf;
	unsigned int i = 0x1 << 31;
	int round = 0;

	while (i > 0)
	{

		if (val & i)
			*ptr++ = '1';
		else
			*ptr++ = '0';

		i = i >> 1;

		if ((++round % 8 == 0) && (i > 0))
			*ptr++ = '.';
	}

	*ptr = '\0';

	return buf;
}

