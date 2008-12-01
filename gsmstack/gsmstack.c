#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#include "gsmstack.h"


struct gsm_rf_chan *gsm_init_rfchan(unsigned int arfcn)
{
	struct gsm_rf_chan *rf;
	int i;

	rf = malloc(sizeof(*rf));
	if (!rf)
		return NULL;
	memset(rf, 0, sizeof(*rf));

	rf->arfcn = arfcn;

	for (i = 0; i < NR_TIMESLOTS; i++) {
		struct gsm_phys_chan *pchan;

		pchan = &rf->phys_chan[i];
		pchan->timeslot = i;
		pchan->rf_chan = rf;
	}
	
	return rf;
}
