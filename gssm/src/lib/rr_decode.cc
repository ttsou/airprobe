// $Id: rr_decode.cc,v 1.1.1.1 2007-06-01 04:26:57 jl Exp $

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char *pd_string(unsigned char pd) {

	switch(pd) {
		case 0:
			return "group call control";
		case 1:
			return "broadcast call control";
		case 2:
			return "reserved (PDSS1 in earlier phases)";
		case 3:
			return "call control; call related SS messages";
		case 4:
			return "GPRS Transparent Transport Protocol (GTTP)";
		case 5:
			return "mobility management messages";
		case 6:
			return "radio resources management messages";
		case 8:
			return "GPRS mobility management messages";
		case 9:
			return "SMS messages";
		case 10:
			return "GPRS session management messages";
		case 11:
			return "non-call related SS messages";
		case 12:
			return "location services";
		case 14:
			return "reserved for extension of the PD";
		case 15:
			return "reserved for tests procedures";
		default:
			return "unknown PD";
	}
}


static char *message_type_rrm_string(unsigned char mt) {

	switch(mt) {
		case 0x3c:
			return "channel establishment message: RR "
			   "initialization request";
		case 0x3b:
			return "channel establishment message: aditional "
			   "assignment";
		case 0x3f:
			return "channel establishment message: immediate "
			   "assignment";
		case 0x39:
			return "channel establishment message: immediate "
			   "assignment extended";
		case 0x3a:
			return "channel establishment message: immediate "
			   "assignment reject";

		case 0x35:
			return "ciphering message: ciphering mode command";
		case 0x32:
			return "ciphering message: ciphering mode complete";

		case 0x30:
			return "configuration change message: configuration "
			   "change command";
		case 0x31:
			return "configuration change message: configuration "
			   "change acknowledgement";
		case 0x33:
			return "configuration change message: configuration "
			   "change reject";

		case 0x2e:
			return "handover message: assignment command";
		case 0x29:
			return "handover message: assignment complete";
		case 0x2f:
			return "handover message: assignment failure";
		case 0x2b:
			return "handover message: handover command";
		case 0x2c:
			return "handover message: handover complete";
		case 0x28:
			return "handover message: handover failure";
		case 0x2d:
			return "handover message: physical information";

		case 0x08:
			return "RR-cell change order";

		case 0x23:
			return "PDCH assignment command";

		case 0x0d:
			return "channel release message: channel release";
		case 0x0a:
			return "channel release message: partial release";
		case 0x0f:
			return "channel release message: partial release "
			   "complete";

		case 0x21:
			return "paging and notification message: paging "
			   "request type 1";
		case 0x22:
			return "paging and notification message: paging "
			   "request type 2";
		case 0x24:
			return "paging and notification message: paging "
			   "request type 3";
		case 0x27:
			return "paging and notification message: paging "
			   "response";
		case 0x20:
			return "paging and notification message: "
			   "notification / NCH";
		case 0x25:
			return "paging and notification message: "
			   "notification / FACCH";
		case 0x26:
			return "paging and notification message: "
			   "notification response";

		case 0x0b:
			return "reserved";

		case 0x18:
			return "system information message: type 8";
		case 0x19:
			return "system information message: type 1";
		case 0x1a:
			return "system information message: type 2";
		case 0x1b:
			return "system information message: type 3";
		case 0x1c:
			return "system information message: type 4";
		case 0x1d:
			return "system information message: type 5";
		case 0x1e:
			return "system information message: type 6";
		case 0x1f:
			return "system information message: type 7";
		case 0x02:
			return "system information message: type 2bis";
		case 0x03:
			return "system information message: type 2ter";
		case 0x05:
			return "system information message: type 5bis";
		case 0x06:
			return "system information message: type 5ter";
		case 0x04:
			return "system information message: type 9";
		case 0x00:
			return "system information message: type 13";
		case 0x3d:
			return "system information message: type 16";
		case 0x3e:
			return "system information message: type 17";

		case 0x10:
			return "miscellaneous message: channel mode modify";
		case 0x12:
			return "miscellaneous message: RR status";
		case 0x17:
			return "miscellaneous message: channel mode modify "
			   "acknowledge";
		case 0x14:
			return "miscellaneous message: frequency redefinition";
		case 0x15:
			return "miscellaneous message: measurement report";
		case 0x16:
			return "miscellaneous message: classmark change";
		case 0x13:
			return "miscellaneous message: classmark enquiry";
		case 0x36:
			return "miscellaneous message: extended measurement "
			   "report";
		case 0x37:
			return "miscellaneous message: extended measurement "
			   "order";
		case 0x34:
			return "miscellaneous message: GPRS suspension request";

		case 0x09:
			return "VGCS uplink control message: uplink grant";
		case 0x0e:
			return "VGCS uplink control message: uplink release";
		case 0x0c:
			return "VGCS uplink control message: uplink free";
		case 0x2a:
			return "VGCS uplink control message: uplink busy";
		case 0x11:
			return "VGCS uplink control message: talker indication";

		case 0x38:
			return "application message: application information";

		default:
			return "unknown radio resource management message "
			   "type";
	}
}


void display_l3(unsigned char *buf, unsigned int buflen) {

	printf("(%d) PD: %s: (%2.2x) %s\n", buflen, pd_string(buf[0] & 0xf),
	   buf[1], message_type_rrm_string(buf[1]));
}


void display_ns_l3(unsigned char *data, unsigned int datalen) {

        int len;

	// bit 1 == 1, bit 2 == 0
	if((data[0] & 3) != 1) {
		fprintf(stderr, "error: display_ns_l3: pseudo-length reserved "
		   "bits bad (%2.2x)\n", data[0] & 3);
		return;
	}
        len = data[0] >> 2;
	if(datalen < len) {
		fprintf(stderr, "error: display_ns_l3: bad data length "
		   "(%d < %d)\n", datalen, len);
		return;
	}

	printf("L3 length: %d\n", len);
	display_l3(data + 1, datalen - 1);
}
