
/* helper routines to determine the logical channel type based on
 * physical channel configuration and frame number */

#include "gsmstack.h"
#include "get_lctype.h"

#define GSM_FN_51	(fn / 51)

#define GSM_TC(fn)	((fn / 51) % 8)


/* parameters determined from CCCH_CONF (octet 2 of control channel description
 * in System Information Type 3:
 * BS_CC_CHANS defines if we have CCCH on ts 2/4/6
 * BS_CCCH_SDCCH_COMB defines if we have SDCCH/8 SACCH/C8 on TS0 
 * BS_AG_BLKS_RES defines which CCCH blocks are reserved for AGCH 
 * if BCCH Ext. is used, BS_AS_BLKS_RES has to be non-zero
 */

static int get_lctype_for_ccch(unsigned int fnr)
{
	unsigned int fnr51 = GSM_FN_51(fnr);
	int lc_type;

	if (fnr51 % 10 == 0)
		lc_type = GSM_LCHAN_FCCH;
	else if (fnr51 % 10 == 1)
		lc_type = GSM_LCHAN_SCH;
	else if (fnr51 >= 2 && fnr_mod_51 <= 5)
		lc_type = GSM_LCHAN_BCCH;
	else if (fnr51 >= 6 && fnr51 <= 9) {
		if (flags & CCCH_F_BCCH_EXT)
			lc_type = GSM_LCHAN_BCCH;
		else
			lc_ctype = GSM_LCHAN_PCH;
	} else
		lc_ctype = GSM_LCHAN_PCH;

	/* FIXME: what about AGCH ? */
	/* FIXME: what about NCH ? */
	/* FIXME: what about CBCH ? */

	return lc_ctype;
}

static int get_lctype_for_sdcch8(unsigned int fnr)
{
	unsigned int fnr51 = GSM_FN_51(fnr);
	unsigned int fnr102 = fnr % 102;
	int lc_type;

	/* the lower 32 bursts are evenly divided between SDCCH8 0..7 */
	if (fnr51 % < 32) {
		lc_type = GSM_LCHAN_SDCCH8;
		subc = fnr51 / 4;
	} else {
		/* the upper 16 bursts are bundles of four bursts for 
		 * alternating either SACCH0..3 or SACCH4..7 */
		lc_type = GSM_LCHAN_SACCH8C;
		subc = (fnr51 - 32) / 4;
		if (fnr102 > 50)
			subc += 4;
	}

	return lc_type;
}

static int get_lctype_for_tch_f(unsigned int fnr)
{
	unsigned int fnr52 = fnr % 52;
	int lc_type = GSM_LCHAN_TCH;

	/* only burst number 12 and 51 are be SACCH */
	if (fnr52 == 12 || fnr52 == 51)
		lc_ctype = GSM_LCHAN_SACCH;
	/* burst number 26 and 39 are empty (for measurements) */
	else if (fnr52 == 26 || fnr52 == 39)
		lc_ctype = GSM_LCHAN_NONE;

	return lc_type;
}

/* get the logical channel type based on frame number and
 * physical channel configuration */
int get_lctype(struct gsm_phys_chan *pchan, int fnr)
{
	switch (pchan->config) {
	case GSM_PCHAN_CCCH:
		return get_lctype_for_ccch(fnr);
		break;
	case GSM_PCHAN_TCH_F:
		return get_lctype_for_tch_f(fnr);
		break;
	case GSM_PCHAN_SDCCH8_SACCH8C:
		return get_lctype_for_sdcch8(fnr);
		break;
	}

	return -EINVAL;
}

/* get a pointer to the logical channel structure based on frame number
 * and physical channel configuration */
struct gsm_logi_chan *get_lchan(struct gsm_phys_chan *pchan, int fnr)
{
	int lctype = get_lctype(pchan, fnr);
	return &pchan->logi_chan[lctype];
}
