// $Id: rrm.h,v 1.1.1.1 2007-06-01 04:26:57 jl Exp $

#pragma once

typedef struct {
	unsigned char	b0:1,
			b1:1,
			len:6;
} l2_pseudo_length_s;

typedef union {
	unsigned char		v;
	l2_pseudo_length_s	b;
} l2_pseudo_length_t;


typedef struct {
	unsigned char		pd:4,
				si:4;
	unsigned char		mt;
	unsigned char		ie[0];
} l3_h_t;
