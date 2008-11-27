#ifndef _GSMTAP_H
#define _GSMTAP_H

/* gsmtap header, pseudo-header in front of the actua GSM payload*/

#include <sys/types.h>

#define GSMTAP_VERSION		0x02

#define GSMTAP_TYPE_UM		0x01
#define GSMTAP_TYPE_ABIS	0x02

struct gsmtap_hdr {
	u_int8_t version;		/* version, set to 0x01 currently */
	u_int8_t hdr_len;		/* length in number of 32bit words */
	u_int8_t type;			/* see GSMTAP_TYPE_* */
	u_int8_t timeslot;		/* timeslot (0..7 on Um) */

	u_int16_t arfcn;		/* ARFCN (frequency) */
	u_int8_t noise_db;
	u_int8_t signal_db;

	u_int32_t frame_number;

} __attribute__((packed));
#endif /* _GSMTAP_H */
