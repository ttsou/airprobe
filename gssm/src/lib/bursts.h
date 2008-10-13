// $Id: bursts.h,v 1.1.1.1 2007-06-01 04:26:57 jl Exp $

#pragma once

#include "burst_types.h"

void		display_burst_type(burst_t);
burst_t		search_burst(unsigned char *, int, int *);
int		search_fc(unsigned char *);
int		is_dummy_burst(const unsigned char *);
