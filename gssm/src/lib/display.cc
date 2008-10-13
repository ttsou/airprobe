// $Id: display.cc,v 1.1.1.1 2007-06-01 04:26:57 jl Exp $

#include <stdio.h>
#include <stdlib.h>


void dump_raw(unsigned char *buf, unsigned int len) {

	unsigned int i;

	for(i = 0; i < len; i++) {
		printf("%2.2x", buf[i]);
		if(!((i + 1) % 4))
			printf(" ");
	}
	printf("\n");
}


void display_raw(unsigned char *buf, unsigned int len) {

	unsigned int i, j, c;
	unsigned long long v;

	for(i = 0; i < len; i += 64) {
		v = 0;
		for(j = 0; (j < 64) && (i + j < len); j++) {
			printf("%d", buf[i + j]);
			if(!((j + 1) % 4))
				printf(" ");
			v = (v << 1) | buf[i + j];
		}
		for(; j < 64; j++) {
			printf(" ");
			if(!((j + 1) % 4))
				printf(" ");
			v <<= 1;
		}
		printf("\t");
		for(j = 0; j < 8; j++) {
			c = (v >> (8 * (7 - j))) & 0xff;
			printf("%2.2x ", c);
		}
		printf("\n");
	}
}
