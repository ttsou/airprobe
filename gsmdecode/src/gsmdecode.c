#include "common.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "data_out.h"

/*
 * B: 112, 176, 128
 * Bbis: 96, 80, 
 */

#if 0
- if there is layer 2 always use layer 2 (BBIS!)
- if there is layer 1 but no layer 2 then generate layer2 from l1! (BBIS!)

or

- if there is layer1 use layer 1
- if there is layer2 but no layer 1 generate layer1 msg
#endif

struct _opt opt;

struct _ch_info
{
	unsigned char flags;
	int logical;
	int physical;
	int direction;
	int fn;
	unsigned char data[23];
	int data_len;
};

#define FL_CH_UP		(0x01)
#define FL_CH_DOWN		(0x02)

static int hex2bin(unsigned char *out, unsigned char *in);

static void
usage(char *str)
{
	fprintf(stderr,
"Usage: gsmdecode [ options ]\n"
" Raw hex input: It decodes one line at a time. The input format should look\n"
" like this:\n"
" 00 01 02 03 04 05 2b 2b 2b 2b 2b 2b 2b ...\n"
" Nokia DCT3 trace debug input (XML) is supported with the -x option.\n"
"\n"
" Options:\n"
"   -h  This message\n"
"   -b  Force Format B (e.g. raw hex SDCCH)\n"
"   -i  Force Format Bbis (e.g. raw hex BCCH)\n"
"   -x  Force OpenGPA xml input format [default]\n"
"   -m  Accept Motorola input (experimental)\n"
"");

	if (str)
	{
		fprintf(stderr, str);
		exit(-1);
	}
	exit(0);
}

static int
xml_get_int(int *val, const unsigned char *str, const unsigned char *pattern)
{
	char *ptr;
	char *start, *end;
	char buf[32];

	ptr = strstr((char *)str, (char *)pattern);
	if (ptr == NULL)
		return -1;
	start = strchr(ptr, '"');
	if (start == NULL)
		return -2;
	start++;
	end = strchr(start, '"');
	if (end == NULL)
		return -3;

	memcpy(buf, start, end - start);
	buf[end - start] = '\0';
	*val = atoi(buf);
	return 0;
}

/*
 * - if there is layer1 use layer 1
 * - if there is layer2 but no layer 1 generate layer1 msg
 * Return -1 if no structure has been found.
 */
static int
xml_in(struct _ch_info *chinfo, unsigned char *str)
{
	unsigned char *dst;
	char *ptr;
	int len;
	int layer = 0;

	if (memcmp(str, "<l1 ", 4) == 0)
	{
		memset(chinfo, 0, sizeof *chinfo);
		memset(chinfo->data, 0x2b, sizeof chinfo->data);
		layer = 1;
	} else if (memcmp(str, "<l2 ", 4) == 0) {
		/* If layer2 exists but layer1 also then do not decode
		 * layer2 again.
		 */
		if (chinfo->data_len > 0)
			return -1;
		layer = 2;
	} else
		return -1;

	/* First read all kind of meta information (logical channel, fn, ..) */
	xml_get_int(&chinfo->logical, str, (const unsigned char *)"logicalchannel");
//		fprintf(stderr, "logical %u\n", chinfo->logical);

	if (strstr((char *)str, "direction=\"up\"") != NULL)
	{
		chinfo->direction = 1;
	} else if (strstr((char *)str, "direction=\"down\"") != NULL) {
		chinfo->direction = 0;
	}

	/* Last see if there is a data="..." section. if so convert it into
	 * binary and prefix it with pseudo length and up/down indicator
	 * if required
	 */
	ptr = strstr((char *)str, "data=\"");
	/* Return 0 (true) here so that caller can decide if data_len
	 * contains other data to decode them...
	 * (This can happen if there is l1 but no l2 data)
	 */
	if (ptr == NULL)
		return 0;

	ptr += 6;
	dst = chinfo->data;
	/* For layer 2 the xml input is missing the length data */
	/* We prefix it! */
	if (layer == 2)
	{
		/* HERE: Layer 1 missing, layer 2 available */
		if ((chinfo->logical ==  112) || (chinfo->logical == 176) || (chinfo->logical == 128))
		{
			/* B header */
			memcpy(dst, "\x01\x01", 3);
			chinfo->data_len = 3;
		} else {
			/* BBis header */
			chinfo->data_len = 1;
		}
		dst += chinfo->data_len;
	}
	
	len = hex2bin(dst, (unsigned char *)ptr);
	if (len <= 0)
		return -3;
	chinfo->data_len += len;
	/* If layer 2 exist but not layer1 then we add a fake header */
	if (layer == 2)
		*(dst - 1) = len << 2;

	return 0;
}

static void
init_defaults()
{
	opt.format = MSG_FORMAT_XML;
}

static void
do_getopt(int argc, char *argv[])
{
	int c;

	while ((c = getopt(argc, argv, "hbixm")) != -1)
	{
		switch (c)
		{
		case 'm':
			opt.flags |= FL_MOTOROLA;
			break;
		case 'x':
			opt.format = MSG_FORMAT_XML;
			break;
		case 'b':
			opt.format = MSG_FORMAT_B;
			break;
		case 'i':
			opt.format = MSG_FORMAT_BBIS;
			break;
		case 'h':
		default:
			usage(NULL);
		}

	}
}

static int
hc2b(unsigned char hex)
{
	hex = tolower(hex);
	if ((hex >= '0') && (hex <= '9'))
		return hex - '0';
	if ((hex >= 'a') && (hex <= 'f'))
		return hex - 'a' + 10;
	return -1;
}

static int
hex2bin(unsigned char *out, unsigned char *in)
{
	unsigned char *out_start = out;
	unsigned char *end = in + strlen((char *)in);
	int is_low = 0;
	int c;

	while (in < end)
	{
		c = hc2b(in[0]);
		if (c < 0)
		{
			in++;
			continue;
		}
		if (is_low == 0)
		{
			out[0] = c << 4;
			is_low = 1;
		} else {
			out[0] |= (c & 0x0f);
			is_low = 0;
			out++;
		}
		in++;
	}

	return out - out_start;
}

int
main(int argc, char *argv[])
{
	unsigned char buf[1024];
	unsigned char bin[sizeof buf / 2 + 1];
	int len;
	struct _ch_info chi;

	init_defaults();
	do_getopt(argc, argv);
	while (fgets((char *)buf, sizeof buf, stdin) != NULL)
	{
		if (opt.format == MSG_FORMAT_XML)
		{
			if (xml_in(&chi, buf) != 0)
				continue;
			if (chi.data_len <= 0)
				continue;
			if ((chi.logical ==  112) || (chi.logical == 176) || (chi.logical == 128))
			{
				l2_data_out_B(0, chi.data, chi.data_len, chi.logical, chi.direction);
			} else {
				l2_data_out_Bbis(0, chi.data, chi.data_len);
			}

			memset(buf, 0, sizeof buf);
			continue;
		}
		memset(bin, 0, sizeof bin);
		len = hex2bin(bin, buf);
		if (opt.format == MSG_FORMAT_B)
			l2_data_out_B(0, bin, len, chi.logical, chi.direction);
		else
			l2_data_out_Bbis(0, bin, len);
	}

	exit(0);
	return 0;
}

