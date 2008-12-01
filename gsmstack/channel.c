#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "gsmstack.h"
#include "cch.h"
#include "tch.h"

/* convert an 18byte 8-bit-per-byte burst to a 142byte 1bit-per-byte */
static void bit_per_byte(unsigned char *dest, const unsigned char *src)
{
	int bit, byte;	

	for (byte = 0; byte < BURST_BYTES; src++) {
		for (bit = 0; bit < 8; bit++) {
			if (src[byte] >> bit &= 0x01)
				*dest++ = 0x01;
			else
				*dest++ = 0x00;
		}
	}
}

static int add_burst_to_lchan(struct gsm_logi_chan *lchan,
			      struct gsm_burst *burst)
{
	struct gsm_phys_chan *pchan = burst->pchan;
	int rc = 0;

	/* copy burst to burst buffer */
	memcpy(lchan->burst_buf[lchan->next_burst], burst, sizeof(*burst));
	lchan->next_burst++;

	if (lchan->next_burst == 4) {
		lchan->next_burst = 0;
		/* decode the four bursts into a MAC block */
		switch (pchan->config) {
		case GSM_PCHAN_TCH_H:
			/* FIXME */
			break;
		case GSM_PCHAN_TCH_F:
			rc = tch_decode();
			break;
		case GSM_PCHAN_CCCH:	
		case GSM_PCHAN_SDCCH8_SACCH8C:
			rc = cch_decode();
			break;
		default:
			fprintf(stderr, "unknown pchan config %u\n",
				pchan->config);
		}
		/* pass the resulting MAC block up the stack */
		if (rc)
			rc = gsm_lchan_macblock()
	}

	return rc;
}

static int gsm_rx_sdcch8(struct gsm_burst *burst)
{
	struct gsm_phys_chan *pchan = burst->gsm_pchan;
	int rc = -EINVAL;

	if (burst_type == GSM_BURST_DUMMY)
		return;

	if (burst->type != GSM_BURST_NORMAL) {
		fprintf(stderr, "Burst type %u not allowed in SDCCH8\n",
			burst->type);
		/* FIXME: statistics */
		return rc;
	}

	lchan = get_lchan(pchan, burst->fnr);
	return add_burst_to_lchan(lchan, burst);
}

static int gsm_rx_tch_f(struct gsm_burst *burst)
{
	struct gsm_phys_chan *pchan = burst->gsm_pchan;
	struct gsm_logi_chan *lchan;
	int rc = -EINVAL;

	if (burst->type != GSM_BURST_NORMAL)
		return rc;

	lchan = get_lchan(pchan, fnr);
	return add_burst_to_lchan(lchan, burst);
}

/* input a new GSM Um burst on a CCCH */
static int gsm_rx_ccch(struct gsm_burst *burst)
{
	struct gsm_phys_chan *pchan = burst->gsm_pchan;
	struct gsm_logi_chan *lchan;
	int rc = -EINVAL;

	switch (burst->type) {
	case GSM_BURST_FCCH:
		/* FIXME */
		/* obtain the frequency offset and report to caller */
		break;
	case GSM_BURST_SCH:
		/* obtain the RFN from the SCH burst */
		/* FIXME */
		break;
	case GSM_BURST_NORMAL:
		/* determine logical channel and append burst */
		lchan = get_lchan(pchan, burst->fnr);
		rc = add_burst_to_lchan(lchan, burst);
		break;
	default:
		break;
	};
	return rc;
}

/* input a new GSM Um interface burst into the stack */
int gsm_rx_burst(struct gsm_burst *burst, int bits)
{
	struct gsm_phys_chan *pchan;
	int rc = -EINVAL;

	/* we assume the following fields have already been
	 * filled-in by the caller:
	 * phys_chan, rx_time, rx_frame_nr, decoded/decoded_bits */

	pchan = burst->phys_chan;

	if (!bits)
		bit_per_byte(burst->decoded_bits, burst->decoded);

	pchan->stats.rx_total++;
	pchan->stats.rx_type[burst->type]++;

	switch (pchan->config) {
	case GSM_PCHAN_CCCH:
		rc = gsm_rx_ccch(burst);
		break;
	case GSM_PCHAN_SDCCH8_SACCH8C:
		rc = gsm_rx_sdcch8(burst);
		break;
	case GSM_PCHAN_TCH_F:
		rc = gsm_rx_tch_h(burst);
		break;
	case GSM_PCHAN_TCH_H:
	case GSM_PCHAN_UNKNOWN:
	default:
		fprintf(stderr, "unknown pchan config (ts=%u\n)\n",
			pchan->timeslot);
		return;
		break;
	}
}
