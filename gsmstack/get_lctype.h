#ifndef _GSM_LCTYPE_H
#define _GSM_LCTYPE_H

extern int get_lctype(struct gsm_phys_chan *pchan, int fnr);
extern struct gsm_logi_chan *get_lchan(struct gsm_phys_chan *pchan, int fnr);

#endif /* _GSM_LCTYPE_H */
