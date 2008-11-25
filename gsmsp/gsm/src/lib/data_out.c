#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "id_list.h"
#include "gsm_desc.h"

#define OUTF(a...)	do { \
	printf("  %3d: %02x ", (int)(data - start), data[0]); \
	printf(a); \
} while (0)

#define OUT(a...)	do { \
	printf(a); \
} while (0)

#define RETTRUNK()	do { \
	printf("%s:%d TRUNKATED (0x%p - 0x%p)\n", __func__, __LINE__, data, end); \
	return; \
} while (0)

static void l2_rrm();
static void l2_sms();
static void l2_cc();
static void l2_RRsystemInfo1();
static void l2_MccMncLac();
static void l2_RRsystemInfo2();
//static void l2_RRsystemInfo2ter();
static void l2_RRsystemInfo3C();
static void l2_RRsystemInfo4C();
static void l2_RRsystemInfo6();
static void l2_RRsystemInfo13C();
static void l2_RRimmediateAssTBFC();
static void l2_RRassignCommand();
static void l2_RRassignComplete();
static void l2_RRpagingrequest1();
static void l2_RRpagingrequest2();
static void l2_RRpagingrequest3();
static void l2_RRimmediateAssignment();
static void l2_RRimmAssTBFDirEncHoChaC();
static void l2_MobId();
static void l2_mmm();
static void l2_HoppingChannel();
static void l2_SingleChannel();
static void l2_HoppingChannelAssCom();
static void l2_SingleChannelAssCom();
static void l2_MobileAllocation();
static void l2_BcchAllocation();
static void l2_TmsiReallocCommand();
static void l2_RachControlParameters();
static void l2_bcc();
static void l2_Bbis();
static void l2_ChannelRelease();
static void l2_MMcmServiceRequest();
static void l2_RRciphModCmd();
static void l2_RRciphModCompl();
static void l2_RRpagingresponse();
static void l2_RRclassmarkChange();

static void l2_ChannelNeeded(char *str, unsigned char ch);
static void l2_MNCC(const char *str, unsigned char a, unsigned char b, unsigned char c);

static char *BitRow(unsigned char c, int pos);
static char *PageMode(unsigned char mode);
static char *BitRowFill(unsigned char c, unsigned char mask);

static void dcch_address();
static void dcch_control();
static void ControlChannelDescription();
static void CellOptionsBcch();
static void CellSelectionParameters();
static void RequestReference();
static void TimingAdvance();
static void StartingTime();
static void l2_NeighbourCellDescription();
static void CellIdentity();
static void MSClassMarkTwo();
static void cpData();
static void Address(const char *str);
static void ChannelDescriptionTwo();
static void CCalerting();
static void CCsetup();
static void ProgressIndicator();
static void Cause();
static void SmsProtocolDataValidity();
static void BearerCap();
static void AuthenticationRequest();
static void AuthenticationResponse();

static const unsigned char *start;
static const unsigned char *data;
static const unsigned char *end;

/*
 * B-format (and also A-Format)
 */
void
l2_data_out_B(int fn, const unsigned char *input_data, int len)
{
	data = input_data;
	start = data;
	end = data + len;
	HEXDUMPF(data, 23 /*len*/, "Format B DATA\n");
	/* Need at least 3 octets */
	if (data + 2 >= end)
		RETTRUNK();

	dcch_address();
	data++;
	dcch_control();
	data++;
	/* FIXME: Why is extended length always set to 1? */
	OUTF("%s EL, Extended Length: %s\n", BitRow(data[0], 0), (data[0] & 1)?"y":"n");
	OUTF("%s M, segmentation: %c\n", BitRow(data[0], 1), ((data[0] >> 1) & 1)?'Y':'N');
	OUTF("%s Length: %u\n", BitRowFill(data[0], 0xfc), data[0] >> 2);
	if (data + (data[0] >> 2) < end)
		end = data + (data[0] >> 2) + 1;
	data++;
	if (data >= end)
		return;
	l2_Bbis();
}

static void
dcch_control()
{
	if ((data[0] & 0x03) != 3)
	{
		if ((data[0] & 1) == 0)
		{
			OUTF("-------0 Information Frame\n");
			OUTF("%s N(S), Sequence counter: %u\n", BitRowFill(data[0], 0x0e), (data[0] >> 1) & 0x07);
			OUTF("%s P\n", BitRow(data[0], 4));
		} else if ((data[0] & 0x03) == 1) {
			OUTF("------01 Supvervisory Frame\n");
			if (((data[0] >> 2) & 0x03) == 0)
				OUTF("----00-- RR Frame (Receive ready)\n");
			else if (((data[0] >> 2) & 0x03) == 1)
				OUTF("----01-- RNR Frame (Receive not ready)\n");
			else if (((data[0] >> 2) & 0x03) == 2)
				OUTF("----10-- REJ Frame (REJect)\n");
			else
				OUTF("----11-- UNKNOWN\n");
			OUTF("%s Poll/Final bit (P/F)\n", BitRow(data[0], 4));
		} 
		OUTF("%s N(R), Retransmission counter: %u\n", BitRowFill(data[0], 0xe0), (data[0] >> 5) & 0x07);
		return;
	}
	OUTF("------11 Unnumbered Frame\n");
	switch (data[0] & 0xec) /* 11101100 */
	{
	case 0x6c: /* 011-11-- */
		OUTF("%s P\n", BitRow(data[0], 4));
		OUTF("011-11-- SABM frame (Set asynchonous balance mode)\n");
		break;
	case 0x0c: /* 000-11-- */
		OUTF("%s F\n", BitRow(data[0], 4));
		OUTF("000-11-- DM frame (Disconnected mode)\n");
		break;
	case 0x00:
		OUTF("%s P\n", BitRow(data[0], 4));
		OUTF("000-00-- UI frame (Unnumbered information)\n");
		break;
	case 0x40:
		OUTF("%s P\n", BitRow(data[0], 4));
		OUTF("010-00-- DISC frame (DISConnect)\n");
		break;
	case 0x60:
		OUTF("%s P\n", BitRow(data[0], 4));
		OUTF("011-00-- UA frame (Unnumbered achknowledgement)\n");
		break;
	default:
		OUTF("%s P/F\n", BitRow(data[0], 4));
		OUTF("%s UNKNOWN\n", BitRowFill(data[0], 0xec));
		break;
	}
}

static void
dcch_address()
{
	if (data[0] & 1)
		OUTF("-------1 Extended Address: 1 octet long\n");
	else
		OUTF("-------0 Extended Address: more octets follow\n");

	if ((data[0] >> 1) & 1)
		OUTF("------1- C/R: Command\n");
	else
		OUTF("------0- C/R: Response\n");

	if (data[0] & 1)
	{
		/* SAPI */
		switch ((data[0] >> 2) & 0x07)
		{
		case 0x03:
			OUTF("---011-- SAPI: SMS and SS\n");
			break;
		case 0x00:
			OUTF("---000-- SAPI: RR, MM and CC\n");
			break;
		default:
			OUTF("%s SAPI: UNKNWON\n", BitRowFill(data[0], 0x1c));
			break;
		}
	
		switch ((data[0] >> 4 ) & 0x03)
		{
		case 0x00:
			OUTF("%s Link Protocol Disciminator: GSM (not Cell Broadcasting)\n", BitRowFill(data[0], 0x60));
			break;
		case 0x01:
			OUTF("%s Link Protocol Disciminator: Cell Broadcasting (CBS)\n", BitRowFill(data[0], 0x60));
			break;
		default:
			OUTF("%s Link Protocol Disciminator: UNKNOWN %u\n", BitRowFill(data[0], 0x60), (data[0] >> 5) & 0x03);
		}
	} else {
		switch ((data[0] >> 2))
		{
		case 0x03:
			OUTF("000011-- SAPI: SMS and SS\n");
			break;
		case 0x00:
			OUTF("000000-- SAPI: RR, MM and CC\n");
			break;
		default:
			OUTF("%s SAPI: UNKNOWN\n", BitRowFill(data[0], 0xfc));
			break;
		}
		data++;
		if (data >= end)
			RETTRUNK();
		if (data[0] & 1)
			OUTF("-------1 Extended Address: 1 octet long\n");
		else
			OUTF("-------0 Extended Address: more octets follow [ERROR]\n");
		OUTF("%s Terminal Endpoint Identifier (TEI): %u\n", BitRowFill(data[0], 0xfe), data[0] >> 1);
	}
}

void
l2_data_out_Bbis(int fn, const unsigned char *input_data, int len)
{
	int i;


	if (len <= 0)
		return;

	data = input_data;
	start = data;

	i = data[0] >> 2;
	if (len - 1 < i)
		OUTF("WARN: packet to short\n");
	len = MIN(len - 1, i);
	/* len = number of octets following the length field */
	end = data + len + 1;

	HEXDUMPF(data, 23 /*len*/, "Format Bbis DATA\n");
	if (len <= 0)
		return;

	OUTF("%s Pseudo Length: %d\n", BitRowFill(data[0], 0xfc), data[0] >> 2);
	data++;
	l2_Bbis();
}

static void
l2_Bbis()
{
	if (data >= end)
		RETTRUNK();

	switch (data[0] >> 7)
	{
	case 1:
		OUTF("1------- Direction: To originating site\n");
		break;
	default:
		OUTF("0------- Direction: From originating site\n");
	}

	OUTF("%s %d TransactionID\n", BitRowFill(data[0], 0x70), (data[0] >> 4) & 7);

	switch (data[0] & 0x0f)
	{
	case 0:
		OUTF("----0000 Group Call Control [FIXME]\n");
		break;
	case 1:
		OUTF("----0001 Broadcast call control [FIXME]\n");
		data++;
		l2_bcc();
		/* TS GSM 04.69 */
		break;
	case 2:
		OUTF("----0010 PDSS1 [FIXME]\n");
		break;
	case 3:
		OUTF("----0011 Call control. call related SS messages\n");
		data++;
		l2_cc();
		/* TS 24.008 */
		break;
	case 4:
		OUTF("----01-- PDSS2 [FIXME]\n");
		break;
	case 5:
		OUTF("----0101 Mobile Management Message (non GPRS)\n");
		data++;
		/* TS 24.008 */
		l2_mmm();
		break;
	case 6:
		OUTF("----0110 Radio Resouce Management\n");
		data++;
		l2_rrm();
		break;
	case 7:
		OUTF("----0111 RFU [FIXME]\n");
		break;
	case 8:
		OUTF("----1000 GPRS Mobile Management\n");
		/* in GMMattachAccept */
		/* in GMMidentityRequest */
		OUTF("FIXME: possible IMEI in here\n");
		break;
	case 9:
		OUTF("----1001 SMS messages\n");
		data++;
		l2_sms();
		/* TS 04.11 */
		break;
	case 0x0a:
		OUTF("----1011 GRPS session management messages [FIXME]\n");
		break;
	case 0x0b:
		OUTF("----1011 Non-call related SS messages [FIXME]\n");
		/* GSM 04.80 */
		break;
	case 0x0c:
		OUTF("----1100 Location services [FIXME]\n");
		break;
	case 0x0d:
		OUTF("----1101 RFU [FIXME]\n");
		break;
	case 0x0e:
		OUTF("----1110 Extension of the PD to one octet length [FIXME]\n");
		break;
	case 0x0f:
		OUTF("----1111 Tests procedures describe in TS GSM 11.10 [FIXME]\n");
		break;
	default:
		OUTF("%s 0x%02x UNKNOWN\n", BitRowFill(data[0], 0x0f), data[0] & 0x0f);
	}

}

/*
 * Broadcast Call Control (04.69)
 */
static void
l2_bcc()
{
	if (data >= end)
		RETTRUNK();

	/* Message type 04.69:9.3*/
	switch (data[0] & 0x3f)
	{
	case 0x06: /* 0-000110 */
		OUTF("--000110 ???\n");
		break;
	default:
		OUTF("--?????? UNKNOWN [FIXME]\n");
		return;
	}

	/* Call reference 04.69:9.4.1*/

	/* Orig indication 04.69:9.5.5*/
	/* Spare half octet 04.69:9.4.5*/

}

/*
 * ProtDisc3
 */
static void
l2_cc()
{
	if (data >= end)
		RETTRUNK();
	OUTF("%s Send Sequence Number: %u\n", BitRowFill(data[0], 0xc0), data[0] >> 6);

	if ((data[0] & 0x3f) == 0x01)
	{
		OUTF("--000001 Call Alerting\n");
		data++;
		CCalerting();
	} else if ((data[0] & 0x3f) == 0x02) {
		OUTF("--000010 Call Proceesing\n");
		if (++data >= end)
			return;
		OUTF("FIXME %s\n", __func__);
	} else if ((data[0] & 0x3f) == 0x07) {
		OUTF("--000111 Call Connect\n");
		if (++data >= end)
			return;
		OUTF("FIXME %s\n", __func__);
	} else if ((data[0] & 0x3f) == 0x08) {
		OUTF("--001000 Call Confirmed\n");
		if (++data >= end)
			return;
		if (data[0] != 0x04)
			return;
		OUTF("--000010 Bearer Capability\n");
		data++;
		BearerCap();
	} else if ((data[0] & 0x3f) == 0x05) {
		OUTF("--000101 Call Setup\n");
		data++;
		CCsetup();
	} else if ((data[0] & 0x3f) == 0x03) {
		OUTF("--000011 Call Progress\n");
		data++;
		ProgressIndicator();
	} else if ((data[0] & 0x3f) == 0x0f) {
		OUTF("--001111 Connect Acknowledge\n");
	} else if ((data[0] & 0x3f) == 0x25) {
		OUTF("--100101 Disconnect\n");
		data++;
		Cause();
	} else if ((data[0] & 0x3f) == 0x2d) {
		OUTF("--101101 CC Release\n");
		if (++data >= end)
			RETTRUNK();
		if (data[0] == 0x08)
		{
			data++;
			Cause();
		}
	} else if ((data[0] & 0x3f) == 0x2a) {
		OUTF("--101010 CC Release Complete\n");
		if (++data >= end)
			RETTRUNK();
		if (data[0] == 0x08)
		{
			data++;
			Cause();
		} 
		if (data >= end)
			RETTRUNK();
		if (data[0] == 0x1c)
		{
			/* facility */
			OUTF("FIXME\n");
		}
	} else {
		OUTF("%s FIXME %s\n", BitRowFill(data[0], 0xff), __func__);
	}
}

/*
 * ----0101
 * ProtDisc5 - Mobile Management message (non GPRS)
 */
static void
l2_mmm()
{
	if (data >= end)
		return;
	OUTF("%s SendSequenceNumber: %d\n", BitRowFill(data[0], 0xc0), data[0] >> 6);
	switch (data[0] & 0x3f)
	{
	case 1:
		//l2_MMimsiDetIndication(data + 1, end);
		OUTF("--000001 Imsi Det Indication\n"); /* FIXME */
		OUTF("FIXME: Possible IMSI in here\n");
		OUTF("FIXME: Possible cipher mode here\n");
		break;
	case 2:
		OUTF("--000010 Location Update Accept\n"); /* FIXME */
		break;
	case 4:
		OUTF("--000100 Message Type: Locatoin Updating Reject\n");
		break;
	case 8:
		OUTF("--001000 MM Location Update Request\n"); /* FIXME */
		OUTF("FIXME: Possible IMSI in here\n");
		OUTF("FIXME: Possible cipher mode here\n");
		break;
	case 0x12:
		OUTF("--010010 Authentication Request\n");
		data++;
		AuthenticationRequest();
		break;
	case 0x14:
		OUTF("--010100 Authentication Response\n");
		data++;
		AuthenticationResponse();
		break;
	case 0x19:
		OUTF("--011001 MMidentityResponse\n");
		OUTF("FIXME: Possible IMSI in here\n");
		break;
	case 0x1a:
		OUTF("--011010 TMSI Realloc Command\n");
		data++;
		l2_TmsiReallocCommand();
		break;
	case 0x24:	/* --100100 */
		OUTF("--100100 MMcmServiceRequest\n");
		data++;
		l2_MMcmServiceRequest();
		/* in multisupport2 and others! */
		break;
	default:
		OUTF("UNKNWON\n");
	}
}

/*
 * ProtDisc6 - Radio Resource Management Messages
 */
static void
l2_rrm()
{
	if (data >= end)
		return;

	switch (data[0] & 0x3f)
	{
	case 0x00:
		OUTF("00000000 System Information Type 13\n");
		data++;
		l2_RRsystemInfo13C();
		break;
	case 0x06:
		OUTF("00000110 System Information Type 5ter\n");
		data++;
		l2_BcchAllocation();
		break;
	case 0x0d:
		OUTF("00001101 Channel Release\n");
		data++;
		l2_ChannelRelease();
		break;
	case 0x15:
		OUTF("00010101 RR Measurement Report C [FIXME]\n");
		break;
	case 0x16:
		OUTF("00010110 RRclassmarkChange\n");
		data++;
		l2_RRclassmarkChange();
		break;
	case 0x19:
		OUTF("00011001 RRsystemInfo1\n");
		data++;
		l2_RRsystemInfo1();
		break;
	case 0x1a:
		OUTF("00011010 RRsystemInfo2\n");
		data++;
		l2_RRsystemInfo2();
		break;
	case 0x1B:	/* 0001 1011 */
		OUTF("00011011 RRsystemInfo3C\n");
		data++;
		l2_RRsystemInfo3C();
		break;
	case 0x1c:
		OUTF("00011100 RRsystemInfo4-C\n");
		data++;
		l2_RRsystemInfo4C();
		break;
	case 0x1d:
		/* From SDCCH */
		OUTF("00011101 Neighbour Cells Description\n");
		data++;
		l2_NeighbourCellDescription();
		break;
	case 0x1e:
		/* From SDCCH */
		OUTF("00011110 System Information Type 6\n");
		data++;
		l2_RRsystemInfo6();
		break;
	case 0x21:
		OUTF("00100001 Paging Request Type 1\n");
		data++;
		l2_RRpagingrequest1();
		break;
	case 0x22:
		OUTF("00100010 Paging Request Type 2\n");
		data++;
		l2_RRpagingrequest2();
		break;
	case 0x24:
		OUTF("00100100 Paging Request Type 3\n");
		data++;
		l2_RRpagingrequest3();
		break;
	case 0x27:
		OUTF("0-100111 RRpagingResponse\n");
		OUTF("-x------ Send sequence number: %d\n", (data[0] >> 7) & 0x01);
		data++;
		l2_RRpagingresponse();
		break;
	case 0x29:
		OUTF("0-101001 RR Assign Complete\n");
		data++;
		l2_RRassignComplete();
		break;
	case 0x2e:
		OUTF("00101110 RR Assign Command\n");
		data++;
		l2_RRassignCommand();
		break;
	case 0x32:
		OUTF("00110010 RR Cipher Mode Complete\n");
		data++;
		l2_RRciphModCompl();
		break;
	case 0x35:
		OUTF("00110101 RR Cipher Mode Command\n");
		data++;
		l2_RRciphModCmd();
		break;
	case 0x3f:
		OUTF("0-111111 RRimmediateAssignment\n");
		OUTF("-x------ Send sequence number: %d\n", (data[0] >> 7) & 0x01);
		data++;
		l2_RRimmediateAssignment();
		break;
	default:
		OUTF("???????? UNKNOWN. FIXME\n");
	}
}
static void
l2_RRsystemInfo13C()
{
	if (data >= end);
		return;
	if (data[0] >> 7)
		OUTF("1------- SI 13 Restoctet present\n");
	else
		OUTF("0------- SI 13 Restoctet NOT present\n");
	OUTF("%s BCCH_CHANGE_MARK : %d\n", BitRowFill(data[0], 0x70), (data[0] >> 4) & 0x07);
	switch (data[0] & 0x0f)
	{
	case 0x00:
		OUTF("----0000 SI_CHANGE_FIELD : Update of unspecified SI messages\n");
		break;
	case 0x01:
		OUTF("----00001 SI_CHANGE_FIELD : Update of unspecified SI1 messages\n");
		break;
	case 0x02:
		OUTF("----0010 SI_CHANGE_FIELD : Update of unspecified SI2 messages\n");
		break;
	case 0x03:
		OUTF("----0011 SI_CHANGE_FIELD : Update of unspecified SI3,4,7,8 messages\n");
		break;
	case 0x04:
		OUTF("----0100 SI_CHANGE_FIELD : Update of unspecified SI9 messages\n");
		break;
	default:
		OUTF("----???? Unknown %d\n", data[0] & 0x0f);
		break;
	}
	OUTF("FIXME: implement me\n");

}

static void
l2_RRimmediateAssignment()
{
	if (data >= end)
		return;

	/* Octect 4, 0x79 */
	OUTF("%s\n", PageMode(data[0] & 0x03));

	if ((data[0] >> 6) & 0x01)
		OUTF("-1------ Two messages assign.: 1. message of..(continue)\n");
	else
		OUTF("-0------ No meaning\n");
	if ((data[0] >> 5) & 0x01)
		OUTF("--1----- Assigns a resource identified in the IA rest octets.\n");
	else
		OUTF("--0----- Downlink assig to MS: No meaning\n");
	if ((data[0] >> 4) & 0x01)
	{
		OUTF("---1---- Temporary Block Flow (TBF)\n");
		data++;
		l2_RRimmediateAssTBFC();
		return;
	}
	else
		OUTF("---0---- This messages assigns a dedicated mode resource\n");
	data++;
	if (data >= end)
		return;
	
	/* Channel Description */
	ChannelDescriptionTwo();

	if (data >= end)
		return;
	if (((data[0] >> 2) & 0x07) == 0)
		l2_SingleChannel();
	else if (((data[0] >> 4) & 0x01) == 1)
	{
		l2_HoppingChannel();
	} else {
		OUTF("xxx0??xxx UNKNOWN %d\n", (data[0] >> 3) & 0x3);
	}
}

static void
l2_SingleChannel()
{
	int freq;
	OUTF("%s Training seq. code: %d\n", BitRowFill(data[0], 0xe0), data[0] >> 5);
	OUTF("---000-- Single channel\n");
	freq = (data[0] & 0x03) << 8;
	data++;
	if (data >= end)
		RETTRUNK();
	freq |= data[0];
	OUTF("........ Absolute RF channel number: %u\n", freq);
	data++;
	RequestReference();
	TimingAdvance();
	l2_MobileAllocation();
	if (data >= end)
		return;
}

static void
l2_HoppingChannel()
{
	unsigned char maio = 0;
	OUTF("%s Training seq. code : %d\n", BitRowFill(data[0], 0xe0), data[0] >> 5);
	OUTF("---1---- HoppingChannel\n");
	maio = (data[0] & 0x0f) << 2;
	data++;
	if (data >= end)
		RETTRUNK();
	maio |= (data[0] >> 6);
	OUTF("........ MAIO %d\n", maio);
	OUTF("%s Hopping Seq. Number: %d\n", BitRowFill(data[0], 0x3f), data[0] & 0x3f);

	data++;
	RequestReference();
	TimingAdvance();
	l2_MobileAllocation();
	if (data >= end)
		return;	/* finished. not truncated! */

	OUTF("FIXME, more data left here???\n");
}

static void
l2_MobileAllocation()
{
	int c = 64, pos;
	char *str = "Mobile allocation RF chann.";
	const unsigned char *thisend;

	if (data >= end)
		RETTRUNK();
	OUTF("%s Length of Mobile Allocation: %d\n", BitRowFill(data[0], 0xff), data[0]);
	thisend = data + data[0] + 1;
	if (thisend > end)
	{
		OUTF("xxxxxxxx ERROR: Packet to short or length to long\n");
		thisend = end;
	}

	data++;
	/* If mobile allocation has length 0 */
	if (data >= thisend)
		return;

	while (data < thisend)
	{
		pos = 7;
		while (pos >= 0)
		{
			if ((data[0] >> pos) & 1)
				OUTF("%s %s%d\n", BitRow(data[0], pos), str, c - (7 - pos));
			pos--;

		}

		c -= 8;
		data++;
		if (c <= 0)
			break;
	}
}

/*
 * From RRsystemInfo2
 */
static void
l2_BcchAllocation()
{
	int c, pos;
	char *str = "BCCH alloc. RF chan.: ";

#if 0
	/* goeller script for Info2 outputs channels 128 + 127
	 * but opengpa outputs bitmap format for info2.
	 * We do what opengpa does. (correct?)
	 */
	if ((data[0] >> 7))
		OUTF("1------- %s%d\n", str, 128);
	if ((data[0] >> 6) & 1)
		OUTF("-1------ %s%d\n", str, 127);
#endif
	if ((data[0] >> 6) == 0x00)
		OUTF("00------ Bitmap format: 0\n");
	else {
		OUTF("%s Bitmap format: UNKNOWN [FIXME]\n", BitRowFill(data[0], 0xc0));
		RETTRUNK();
	}

	if ((data[0] & 0x8e) == 0x8e)
	{
		/* From System Information Type 5ter */
		OUTF("1---111- Variable Bitmap SI5ter [FIXME]\n");
		return;
	}

	if ((data[0] >> 5) & 1)
		OUTF("--1----- Extension Indicator: The IE carries only a part of the BA\n");
	else
		OUTF("--0----- Extension Indicator: The IE carries the complete BA\n");
	OUTF("---x---- BCCH alloc. seq. num: %d\n", (data[0] >> 4) & 1);
	if ((data[0] >> 3) & 1)
		OUTF("----1--- %s%d\n", str, 124);
	if ((data[0] >> 2) & 1)
		OUTF("-----1-- %s%d\n", str, 123);
	if ((data[0] >> 1) & 1)
		OUTF("------1- %s%d\n", str, 122);
	if (data[0] & 1)
		OUTF("-------1 %s%d\n", str, 121);

	data++;
	c = 120;
	while (data < end)
	{
		pos = 7;
		while (pos >= 0)
		{
			if ((data[0] >> pos) & 1)
				OUTF("%s %s%d\n", BitRow(data[0], pos), str, c - (7 - pos));
			pos--;

		}

		c -= 8;
		data++;
		if (c <= 0)
			break;
	}
}

static void
l2_RRimmediateAssTBFC()
{
	if (data >= end)
		return;

	/* GPRS Packet Channel Description */
	OUTF("%s Channel Type : %d\n", BitRowFill(data[0], 0xf8), data[0] >> 3);
	OUTF("%s Time Slot Number : %d\n", BitRowFill(data[0], 0x07), data[0] & 0x07);
	data++;
	if (data >= end)
		return;

	OUTF("%s Tranining Sequence Code: %d\n", BitRowFill(data[0], 0xe0), data[0] >> 5);
	if ((data[0] >> 4) & 0x01)
	{
		OUTF("---1---- Direct Encoding of Hopping Channels\n");
		l2_RRimmAssTBFDirEncHoChaC();
		return;
	} else {
		OUTF("---0---- non-hopping RF channel config or indirect encoding of hopping RFCC\n");
	}

	if ((data[0] >> 3) & 0x01)
	{
		OUTF("----1--- indirect encoding of hopping RF channel config\n");
	} else {
		OUTF("----0--- RRimmAssTBFaRFCN-C FIXME\n");
		return;
	}

	data++;
	if (data >= end)
		RETTRUNK();
	OUTF("xxxxxxxx MAIO [FIXME]\n");

	data++;
	RequestReference();
	TimingAdvance();
	l2_MobileAllocation();
	if (data >= end)
		RETTRUNK();
	OUTF("FIXME: implenet\n");
}

static void
l2_RRsystemInfo1()
{
	int ca;

	if (data + 1 >= end)
		return;
	switch (data[0] >> 6)
	{
		case 0x00:
			OUTF("00------ Bitmap 0 format\n");
			break;
		case 0x01:
			OUTF("10------ Bitmap format: (FIXME)\n");
			break;
		default:
			OUTF("%s Bitmap %d format (FIXME)\n", BitRowFill(data[0], 0xc0), data[0] >> 6);
	}
	if ((data[0] >> 3) & 1)
		OUTF("----1--- Cell Allocation   : ARFCN 124\n");
	if ((data[0] >> 2) & 1)
		OUTF("-----1-- Cell Allocation   : ARFCN 123\n");
	if ((data[0] >> 1) & 1)
		OUTF("------1- Cell Allocation   : ARFCN 122\n");
	if (data[0] & 1)
		OUTF("-------1 Cell Allocation   : ARFCN 121\n");

	ca = 120;
	while (ca > 0)
	{
		data++;
		if (data >= end)
			return;
		if ((data[0] >> 7) & 1)
			OUTF("1------- Cell Allocation   : ARFCN %d\n", ca);
		if ((data[0] >> 6) & 1)
			OUTF("-1------ Cell Allocation   : ARFCN %d\n", ca - 1);
		if ((data[0] >> 5) & 1)
			OUTF("--1----- Cell Allocation   : ARFCN %d\n", ca - 2);
		if ((data[0] >> 4) & 1)
			OUTF("---1---- Cell Allocation   : ARFCN %d\n", ca - 3);
		if ((data[0] >> 3) & 1)
			OUTF("----1--- Cell Allocation   : ARFCN %d\n", ca - 4);
		if ((data[0] >> 2) & 1)
			OUTF("-----1-- Cell Allocation   : ARFCN %d\n", ca - 5);
		if ((data[0] >> 1) & 1)
			OUTF("------1- Cell Allocation   : ARFCN %d\n", ca - 6);
		if (data[0] & 1)
			OUTF("-------1 Cell Allocation   : ARFCN %d\n", ca - 7);

		ca -= 8;
	}

	data++;

	l2_RachControlParameters();
	OUTF("FIXME: NCH Position\n");
	if (data >= end)
		return;
}

static void
l2_RachControlParameters()
{
	int ca = -1;

	if (data >= end)
		return;

	if (((data[0] >> 6) & 0x03) == 0)
		ca = 1;
	else if (((data[0] >> 6) & 0x03) == 0x01)
		ca = 2;
	else if (((data[0] >> 6) & 0x03) == 0x02)
		ca = 4;
	else if (((data[0] >> 6) & 0x03) == 0x03)
		ca = 7;
	OUTF("%s Max. of retransmiss : %u\n", BitRowFill(data[0], 0xc0), ca);
	if (((data[0] >> 2) & 0x0f) <= 9)
	{
		ca = ((data[0] >> 2) & 0x0f) + 3;
	} else {
		switch ((data[0] >> 2) & 0x0f)
		{
		case 10:	/* --1010-- */
			ca = 14;
			break;
		case 11:
			ca = 16;
			break;
		case 12:
			ca = 20;
			break;
		case 13:
			ca = 25;
			break;
		case 14:
			ca = 32;
			break;
		case 15:
			ca = 50;
			break;
		default:
			ca = -1;
			break;
		}
	}
	OUTF("%s slots to spread TX : %u\n", BitRowFill(data[0], 0x3c), ca);
	switch ((data[0] >> 1) & 0x01)
	{
	case 0:
		OUTF("------0- The cell is barred  : no\n");
		break;
	case 1:
		OUTF("------1- The cell is barred  : yes\n");
		break;
	}

	switch (data[0] & 0x01)
	{
	case 0:
		OUTF("-------0 Call reestabl.i.cell: allowed\n");
		break;
	case 1:
		OUTF("-------1 Cell reestabl.i.cell: not allowed\n");
	}

	data++;
	if (data >= end)
		return;
	switch ((data[0] >> 2) & 0x01)
	{
	case 0:
		OUTF("-----0-- Emergency call EC 10: allowed\n");
		break;
	case 1:
		OUTF("-----1-- Emergency call EC 10: not allowed\n");
		break;
	}
	OUTF("%s Acc ctrl cl 11-15: 0 = permitted, 1 = forbidden\n", BitRowFill(data[0], 0xf8));
	OUTF("%s Acc ctrl cl  8- 9: 0 = permitted, 1 = forbidden\n", BitRowFill(data[0], 0x03));
	OUTF("%s Ordinary subscribers (8)\n", BitRowFill(data[0], 0x01));
	OUTF("%s Ordinary subscribers (9)\n", BitRowFill(data[0], 0x02));
	OUTF("%s Emergency call (10): %s\n", BitRowFill(data[0], 0x04), ((data[0] >> 4) & 1)?"Class 11-15 only":"Everyone");
	OUTF("%s Operator Specific (11)\n", BitRowFill(data[0], 0x08));
	OUTF("%s Security service (12)\n", BitRowFill(data[0], 0x10));
	OUTF("%s Public service (13)\n", BitRowFill(data[0], 0x20));
	OUTF("%s Emergency service (14)\n", BitRowFill(data[0], 0x40));
	OUTF("%s Network Operator (15)\n", BitRowFill(data[0], 0x80));
	data++;
	if (data >= end)
		RETTRUNK();
	OUTF("%s Acc ctrl cl  0- 7: 0 = permitted, 1 = forbidden\n", BitRowFill(data[0], 0xff));
	OUTF("%s Ordinary subscribers (0-7)\n", BitRowFill(data[0], 0xff));
	data++;
}

static char *
BitRow(unsigned char c, int pos)
{
	unsigned char bit = 0;
	static char buf[9];

	if ((c >> pos) & 1)
		bit = 1;

	if (pos == 0)
		snprintf(buf, sizeof buf, "-------%d", bit);
	else if (pos == 1)
		snprintf(buf, sizeof buf, "------%d-", bit);
	else if (pos == 2)
		snprintf(buf, sizeof buf, "-----%d--", bit);
	else if (pos == 3)
		snprintf(buf, sizeof buf, "----%d---", bit);
	else if (pos == 4)
		snprintf(buf, sizeof buf, "---%d----", bit);
	else if (pos == 5)
		snprintf(buf, sizeof buf, "--%d-----", bit);
	else if (pos == 6)
		snprintf(buf, sizeof buf, "-%d------", bit);
	else if (pos == 7)
		snprintf(buf, sizeof buf, "%d-------", bit);

	return buf;
}

static char *
BitRowFill(unsigned char c, unsigned char mask)
{
	static char buf[9];
	
	memset(buf, '-', sizeof buf);
	buf[sizeof buf - 1] = '\0';
	int i = 0;
	while (i < 8)
	{
		if ((mask >> i) & 1)
		{
			if ((c >> i) & 1)
				buf[7 - i] = '1';
			else
				buf[7 - i] = '0';
		}
		i++;
	}

	return buf;
}

static void
l2_RRsystemInfo2()
{
	if (data >= end)
		RETTRUNK();

	l2_BcchAllocation();
	if (data >= end)
		RETTRUNK();

	int c = 7;
	while (c >= 0)
	{
		if ((data[0] >> c) & 1)
			OUTF("%s BCCH carrier with NCC = %d is permitted for monitoring\n", BitRow(data[0], c), c);
		c--;
	}
	data++;
	if (data >= end)
		RETTRUNK();

	l2_RachControlParameters();

	if (data > end)	/* Note: not >= */
		RETTRUNK();
}

#if 0
static void
l2_RRsystemInfo2ter()
{
	if (data >= end)
		return;
	if ((data[0] >> 7) == 0)
		OUTF("%s Bitmap 0 format\n", BitRowFill(data[0], 0x8e));
	else {
		/* 0x8e = 10001110 */
		if (((data[0] >> 1) & 0x07) == 0x04)
			OUTF("1---100- 1024 range\nFIXME\n");
		else if (((data[0] >> 1) & 0x07) == 0x05)
			OUTF("1---101- 512 range\nFIXME\n");
		else if (((data[0] >> 1) & 0x07) == 0x06)
			OUTF("1---110- 128 range\nFIXME\n");
		else if (((data[0] >> 1) & 0x07) == 0x07)
			OUTF("1---111- variable Bitmap\nFIXME\n");
		else
			OUTF("1---xxx- UNKNOWN 0x%08x\n", data[0]);
	}
	OUTF("FIXME\n");
}
#endif


/*
 * RRsystemInfo4-C
 */
static void
l2_RRsystemInfo4C()
{
	if (data + 2 >= end)
		RETTRUNK();
	l2_MccMncLac();
	CellSelectionParameters();
	l2_RachControlParameters();
	if (data >= end)
		RETTRUNK();
	OUTF("FIXME\n");
}

/*
 * Output MCC, MNC and LAC. consume 5 bytes.
 */
static void
l2_MccMncLac()
{
	if (data + 2 >= end)
		return;
	unsigned short lac;
	
	l2_MNCC("Mobile Country Code", data[0] & 0x0f, (data[0] >> 4) & 0x0f, data[1] & 0x0f);
	data++;
	l2_MNCC("Mobile Network Code", data[1] & 0x0f, (data[1] >> 4) & 0x0f, (data[0] >> 4) & 0x0f);
	data += 2;

	if (data + 1 >= end)
		return;

	lac = data[0];
	lac = (lac << 8) | data[1];
	OUTF("%-8u [0x%02x%02x] Local Area Code\n", lac, data[0], data[1]);
	data += 2;
}
/*
 * RRsystemINfo3-C
 */
static void
l2_RRsystemInfo3C()
{
	CellIdentity();
	l2_MccMncLac();

	ControlChannelDescription();
	CellOptionsBcch();
	CellSelectionParameters();
	l2_RachControlParameters();

	/* FIXME: complete here */
}

static void
l2_RRsystemInfo6()
{
	CellIdentity();
	l2_MccMncLac();
	CellOptionsBcch();
	if (data >= end)
		RETTRUNK();
	OUTF("%s Network Colour Code: %u\n", BitRowFill(data[0], 0xff), data[0]);
}

static void
CellIdentity()
{
	unsigned short id;

	if (data + 1 >= end)
		return;

	id = data[0];
	id = (id << 8) | data[1];
	OUTF("%-8u [0x%02x%02x] Cell identity\n", id, data[0], data[1]);
	data += 2;
}

static void
ControlChannelDescription()
{
	if (data >= end)
		RETTRUNK();
	OUTF("%s Spare bit (should be 0)\n", BitRow(data[0], 7));
	if ((data[0] >> 6) & 1)
		OUTF("-1------ MSs in the cell shall apply IMSI attach/detach procedure\n");
	else
		OUTF("-0------ MSs in cell are not allowed attach/detach procedure\n");
	OUTF("%s Number of blocks: %u\n", BitRowFill(data[0], 0x38), (data[0] >> 3) & 0x07);

	switch (data[0] & 0x07)
	{
	case 0x00:
		OUTF("-----000 1 basic physical channel for CCCH, not combined with SDCCHs\n");
		break;
	case 0x01:
		OUTF("-----001 1 basic physical channel for CCCH,     combined with SDCCHs\n");
		break;
	case 0x02:
		OUTF("-----010 2 basic physical channel for CCCH, not combined with SDCCHs\n");
		break;
	case 0x04:
		OUTF("-----100 3 basic physical channel for CCCH, not combined with SDCCHs\n");
		break;
	case 0x06:
		OUTF("-----110 4 basic physical channel for CCCH, not combined with SDCCHs\n");
		break;
	default:
		OUTF("%s Unknown CCCH config (ERROR)\n", BitRowFill(data[0], 0x07));
		break;
	}

	data++;
	if (data >= end)
		RETTRUNK();

	OUTF("%s spare bits (should be 0)\n", BitRowFill(data[0], 0xf8));
	OUTF("%s %u multi frames period for paging request\n", BitRowFill(data[0], 0x07), (data[0] & 0x07) + 2);

	data++;
	if (data >= end)
		RETTRUNK();
	OUTF("%s T3212 TimeOut value: %u\n", BitRowFill(data[0], 0xff), data[0]);
	data++;
}

static void
CellOptionsBcch()
{
	if (data >= end)
		RETTRUNK();
	OUTF("%s spare bit (should be 0)\n", BitRowFill(data[0], 0x80));
	if ((data[0] >> 6) & 1)
		OUTF("-1------ Power control indicator is set\n");
	else
		OUTF("-0------ Power control indicator is not set\n");

	if (((data[0] >> 4) & 0x03) == 0x00)
		OUTF("--00---- MSs may use uplink DTX\n");
	else if (((data[0] >> 4) & 0x03) == 0x01)
		OUTF("--01---- MSs shall use uplink DTX\n");
	else if (((data[0] >> 4) & 0x03) == 0x02)
		OUTF("--10---- MSs shall not use uplink DTX\n");
	else
		OUTF("%s DTX UNKNOWN [ERROR]\n", BitRowFill(data[0], 0x30));

	OUTF("%s Radio Link Timeout: %u\n", BitRowFill(data[0], 0x0f), ((data[0] & 0x0f) + 1 ) * 4);
	data++;
}

static void
l2_MNCC(const char *str, unsigned char a, unsigned char b, unsigned char c)
{
	char buf[128];
	char f[12];

	snprintf(f, sizeof f, "%x%x%x", a, b, c);
	/* Nokia netmonitor shows NC's like '30F' and '10F' */
	snprintf(buf, sizeof buf, "%-8s %s\n", f, str);

#if 0
	buf[0] = '\0';
	if (a != 0x0f)
	{
		snprintf(buf, sizeof buf, "%x", a);
		if (b != 0x0f)
		{
			snprintf(buf + 1, sizeof buf - 1, "%x", b);
			if (c != 0x0f)
				snprintf(buf + 2, sizeof buf - 2, "%x", c);
		}
	}
	snprintf(buf + strlen(buf), sizeof buf - strlen(buf), "  - %s\n", str);
#endif

	OUTF(buf);
}

static char *
PageMode(unsigned char mode)
{
	switch (mode)
	{
	case 0:
		return "------00 Page Mode: Normal paging";
	case 1:
		return "------01 Page Mode: Extended paging";
	case 2:
		return "------10 Page Mode: Paging reorganisation";
	case 3:
		return "------11 Page Mode: reserved / same as before";
	}

	return "------?? UNKNOWN\n";
}

static void
l2_RRpagingrequest1()
{
	if (data >= end)
		return;

	OUTF("%s\n", PageMode(data[0] & 0x3));

	/* FIXME complete here */

	data++;
	if (data >= end)
		return;

	l2_MobId();
	if (data >= end)
		return;	 /* sometimes it's end here */
	if (data[0] == 0x17)
	{
		data++;
		l2_MobId();
		return;
	}
	OUTF("ERR: wrong data\n");
}

static void
l2_ChannelNeeded(char *str, unsigned char ch)
{
	switch (ch)
	{
	case 0x00:
		OUTF("%s Channel Needed: Any channel\n", str);
		break;
	case 0x01:
		OUTF("%s Channel Needed: SDCCH\n", str);
		break;
	case 0x02:
		OUTF("%s Channel Needed: TCH/F (Full rate)\n", str);
		break;
	case 0x03:
		OUTF("%s Channel Needed: TCH/H or TCH/F (Dual rate)\n", str);
		break;
	}

}

static void
l2_RRpagingrequest2()
{
	if (data >= end)
		return;
	OUTF("%s\n", PageMode(data[0] & 0x03));

	l2_ChannelNeeded("--xx---- (first)", (data[0] >> 4) & 0x03);
	l2_ChannelNeeded("xx------ (second)", data[0] >> 6);

	data++;
	if (data + 3 >= end)
		RETTRUNK();
	OUTF("........ Mob. Ident 1 (P)TMSI: %02X%02X%02X%02X\n", data[0], data[1], data[2], data[3]);
	data += 4;
	if (data + 3 >= end)
		RETTRUNK();
	OUTF("........ Mob. Ident 2 (P)TMSI: %02X%02X%02X%02X\n", data[0], data[1], data[2], data[3]);
	data += 4;
	if (data >= end)
		RETTRUNK();

	if (data[0] == 0x17)
	{
		data++;
		l2_MobId();
		return;
	}

	OUTF("FIXME, unknown\n");
}

static void
l2_RRpagingrequest3()
{
	if (data >= end)
		RETTRUNK();
	OUTF("%s\n", PageMode(data[0] & 0x03));
	l2_ChannelNeeded("--xx---- (first)", (data[0] >> 4) & 0x03);
	l2_ChannelNeeded("xx------ (second)", data[0] >> 6);
	data++;

	int c = 0;
	while (c++ < 4)
	{
		if (data + 3 >= end)
			RETTRUNK();
		OUTF("........ Mob. Ident %u (P)TMSI: %02X%02X%02X%02X\n", c, data[0], data[1], data[2], data[3]);
		data += 4;
	}
}

static void
l2_MobId()
{
	const unsigned char *thisend = end;
	unsigned char len = data[0];
	char odd = 0;
	int bcd = 0;

	data++;
	if (data >= end)
		return;

	if ((data[0] >> 3) & 1)
		odd = 1;

	switch (data[0] & 0x07)
	{
	case 0:
		OUTF("-----000 Type of identity: No Identity\n");
		break;
	case 1:
		OUTF("-----001 Type of identity: IMSI\n");
		bcd = 1;
		break;
	case 2:
		OUTF("-----010 Type of identity: IMEI\n");
		bcd = 1;
		break;
	case 3:
		OUTF("-----011 Type of identity: IMEISV\n");
		bcd = 1;
		break;
	case 4:
		OUTF("-----100 Type of identity: TMSI/P-TMSI\n");
		break;
	default:
		OUTF("-----000 Type of identity: UNKNOWN\n");
		return;
	}
	if (len <= 0)
		return;

	/* Nokia Netmonitor never outputs the first value */
	//OUTF("%x", data[0] >> 4);
	unsigned char c;
	c = data[0] >> 4;
	len--;
	data++;
	if (len <= 0)
		return;

	OUTF("-------- ID(%d/%s): ", len, odd?"odd":"even");

	if (data + len < thisend)
		thisend = data + len;
	if (bcd)
	{
		OUT("%X", c);
		while (data < thisend)
		{
			if ((data + 1 == thisend) && (!odd))
				OUT("%X", data[0] & 0x0f);
			else
				OUT("%X%X", data[0] & 0x0f, (data[0] >> 4) & 0x0f);
			data++;
		}
	} else {
		while (data < thisend)
		{
			if ((data + 1 == thisend) && (odd))
				OUT("%X", (data[0] >> 4 ) & 0x0f);
			else
				OUT("%X%X", (data[0] >> 4) & 0x0f, data[0] & 0x0f);
			data++;
		}
	}
	OUT("\n");
}


static void CellSelectionParameters()
{
	if (data >= end)
		RETTRUNK();

	switch (data[0] >> 5)
	{
	case 0:
		OUTF("000----- Cell Reselect Hyst. :  0 db RXLEV\n");
		break;
	case 1:
		OUTF("001----- Cell Reselect Hyst. :  2 db RXLEV\n");
		break;
	case 2:
		OUTF("010----- Cell Reselect Hyst. :  4 db RXLEV\n");
		break;
	case 3:
		OUTF("011----- Cell Reselect Hyst. :  6 db RXLEV\n");
		break;
	case 4:
		OUTF("100----- Cell Reselect Hyst. :  8 db RXLEV\n");
		break;
	case 5:
		OUTF("101----- Cell Reselect Hyst. : 10 db RXLEV\n");
		break;
	case 6:
		OUTF("110----- Cell Reselect Hyst. : 12 db RXLEV\n");
		break;
	case 7:
		OUTF("111----- Cell Reselect Hyst. : 14 db RXLEV\n");
		break;
	}
	OUTF("---xxxxx Max Tx power level: %d\n", data[0] & 0x1f);
	data++;
	if (data >= end)
		RETTRUNK();

	if (data[0] >> 7)
		OUTF("1------- Additional cells in SysInfo 16,17\n");
	else
		OUTF("0------- No additional cells in SysInfo 7-8\n");
	if ((data[0] >> 6) & 1)
		OUTF("-1------ New establishm cause: supported\n");
	else
		OUTF("-0------ New establishm cause: not supported\n");
	OUTF("--xxxxxx RXLEV Access Min permitted = -110 + %ddB\n", data[0] & 0x3f);
	data++;
}

static void
l2_RRimmAssTBFDirEncHoChaC()
{
	unsigned char maio = (data[0] & 0x0f) << 4;

	data++;
	if (data >= end)
		RETTRUNK();
	maio |= (data[0] >> 6);
	OUTF("xxxxxxxx MAIO: %u\n", maio);
	OUTF("%s HSN: %u\n", BitRowFill(data[0], 0x3f), data[0] & 0x3f);
	data++;

	RequestReference();

	TimingAdvance();
	l2_MobileAllocation();

	if (data >= end)
		RETTRUNK();
	if (data[0] == 0x7c)
	{
		StartingTime();
		return;
	}
	OUTF("FIXME: implement me\n");
}

static void
RequestReference()
{
	if (data >= end)
		RETTRUNK();

	/* Request Reference */
	if ((data[0] >> 5) == 0)
		OUTF("000----- Establishing Cause : All other cases\n");
	else if ((data[0] >> 5) == 0x01)
		OUTF("001----- Establishing Cause : All other cases\n");
	else if ((data[0] >> 5) == 0x02)
		OUTF("010----- Establishing Cause : All other cases\n");
	else if ((data[0] >> 5) == 0x03)
		OUTF("011----- Establishing Cause : All other cases\n");
	else if ((data[0] >> 5) == 0x04)
		OUTF("100----- Establishing Cause: Answer to paging\n");
	else if ((data[0] >> 5) == 0x05)
		OUTF("101----- Establishing Cause: Emergency call\n");
	else if ((data[0] >> 5) == 0x07)
		OUTF("111----- Establishing Cause: Other services req. by user\n");
/* Random refernce must be 5 bit long ?! */
//	else if ((data[0] >> 4) == 0x05)
//		OUTF("0101---- Establishing Cause: Originating data call from dual rate mobile station\n");
//	else if ((data[0] >> 4) == 0x02)
//		OUTF("0010---- Establishing Cause: Answer to paging\n");
	else 
		OUTF("%s Establishing Cause: UNKNOWN [FIXME}\n", BitRowFill(data[0], 0xe0));

	OUTF("---xxxxx Random Reference : %d\n", data[0] & 0x1f);

	data++;
	if (data + 1>= end)
		RETTRUNK();

	OUTF("xxxxxxxx T1/T2/T3\n");
	data++;
	OUTF("xxxxxxxx T1/T2/T3\n");
	data++;
	/* END Request Reference */
}

static void
TimingAdvance()
{
	if (data >= end)
		RETTRUNK();
	OUTF("--xxxxxx Timing advance value: %d\n", data[0] & 0x3f);
	data++;
}

static void
StartingTime()
{
	if (data >= end)
		RETTRUNK();
	OUTF("01111100 Starting Time block\n");
	data++;
	if (data >= end)
		RETTRUNK();
	unsigned char t3;
	OUTF("%s T1 Frame Number: %u\n", BitRowFill(data[0], 0xf8), data[0] >> 3);
	t3 = (data[0] & 0x07) << 5;
	data++;
	if (data >= end)
		RETTRUNK();
	t3 |= (data[0] >> 5);
	OUTF("%s T2 Frame Number: %u\n", BitRowFill(data[0], 0x1f), data[0] & 0x1f);
	OUTF("........ T3 Frame Number: %u\n", t3);
}

/*
 * RRsystemInfo5
 */
static void
l2_NeighbourCellDescription()
{
	if (data >= end)
		RETTRUNK();
	l2_BcchAllocation();
}


static void
l2_ChannelRelease()
{
	if (data >= end)
		RETTRUNK();
	OUTF("%s\n", id_list_get(list_ChannelRelease, data[0]));
	data++;
}

static void
l2_MMcmServiceRequest()
{
	if (data >= end)
		RETTRUNK();

	OUTF("%s Ciphering key sequence: %u\n", BitRowFill(data[0], 0x70), (data[0] >> 4) & 0x07);
	OUTF("%s\n", id_list_get(list_RequestServiceType, data[0] & 0x0f));
	data++;
	MSClassMarkTwo();
	if (data >= end)
		RETTRUNK();

	if (data[0] == 0x20)
	{
		OUTF("FIXME: classmark3\n");
		return;
	}

	l2_MobId();
}

static void
MSClassMarkTwo()
{
	if (data >= end)
		RETTRUNK();

	OUTF("%s MS Classmark 2 length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	data++;
	if (data >= end)
		RETTRUNK();

	if (((data[0] >> 5) & 0x03) == 0)
		OUTF("-00----- Revision Level: Phase 1\n");
	else if (((data[0] >> 5) & 0x03) == 1)
		OUTF("-01----- Revision Level: Phase 2\n");
	else
		OUTF("-xx----- Revision Level: Unknown\n");
	if (((data[0] >> 4) & 1) == 0)
		OUTF("---0---- Controlled early classmark sending: Not implemented\n");
	else
		OUTF("---1---- Controlled early classmark sending: Implemented\n");
	if ((data[0] >> 3) & 1)
		OUTF("----1--- A5/1 not available\n");
	else
		OUTF("----0--- A5/1 available\n");
	
	OUTF("%s RF power class capability: Class %u\n", BitRowFill(data[0], 0x07), (data[0] & 0x07) + 1);

	data++;

	if ((data[0] >> 6) & 1)
		OUTF("-1------ Pseudo Sync Capability: present\n");
	else
		OUTF("-1------ Pseudo Sync Capability: not present\n");

	if (((data[0] >> 4) & 0x03) == 0)
		OUTF("--00---- SS Screening: Phase 1 default value\n");
	else if (((data[0] >> 4) & 0x03) == 1)
		OUTF("--01---- SS Screening: Phase 2 error handling\n");
	else
		OUTF("--xx---- SS Screening: UNKNOWN\n");

	if ((data[0] >> 3) & 1)
		OUTF("----1--- Mobile Terminated Point to Point SMS: supported\n");
	else
		OUTF("----0--- Mobile Terminated Point to Point SMS: not supported\n");

	if ((data[0] >> 2) & 1)
		OUTF("-----1-- VoiceBroadcastService: supported\n");
	else
		OUTF("-----0-- VoiceBroadcastService: not supported\n");

	if ((data[0] >> 1) & 1)
		OUTF("------1- VoiceGroupCallService: supported\n");
	else
		OUTF("------0- VoiceGroupCallService: not supported\n");

	if (data[0] & 1)
		OUTF("-------1 MS supports E-GSM or R-GSM: supported\n");
	else
		OUTF("-------0 MS supports E-GSM or R-GSM: not supported\n");

	data++;
	if (data >= end)
		RETTRUNK();


	if ((data[0] >> 7) & 1)
		OUTF("1------- CM3 option: supported\n");
	else
		OUTF("0------- CM3 option: not supported\n");

	if ((data[0] >> 5) & 1)
		OUTF("--1----- LocationServiceValueAdded Capability: supported\n");
	else
		OUTF("--0----- LocationServiceValueAdded Capability: not supported\n");

	if ((data[0] >> 3) & 1)
		OUTF("----1--- SoLSA Capability: supported\n");
	else
		OUTF("----0--- SoLSA Capability: not supported\n");

	if ((data[0] >> 1) & 1)
		OUTF("------1- A5/3 available\n");
	else
		OUTF("------0- A5/3 not available\n");
	
	if (data[0] & 1)
		OUTF("-------1 A5/2: available\n");
	else
		OUTF("-------0 A5/2: not available\n");
	data++;
}

static void
l2_RRciphModCmd()
{
	if (data >= end)
		RETTRUNK();
	if (((data[0] >> 1) & 0x07) == 0x07)
		OUTF("----111- Cipher: reserved [UNKNOWN]\n");
	else
		OUTF("%s Cipher: A5/%u\n", BitRowFill(data[0], 0x0e), ((data[0] >> 1) & 0x07) + 1);


	if (data[0] & 1)
		OUTF("-------1 Start ciphering\n");
	else
		OUTF("-------0 No ciphering\n");

	if ((data[0] >> 4) & 1)
		OUTF("---1---- Cipher Response: IMEISV shall be included\n");
	else
		OUTF("---0---- Cipher Response: IMEISV shall not be included\n");
	data++;
}

static void
l2_RRciphModCompl()
{
	if (data >= end)
		RETTRUNK();

	if (data[0] != 0x17)
		return;
	if (++data >= end)
		RETTRUNK();
	l2_MobId();
}

static void
l2_TmsiReallocCommand()
{
	if (data >= end)
		RETTRUNK();

	l2_MccMncLac();
	l2_MobId();
}

static void
l2_sms()
{
	if (data >= end)
		RETTRUNK();

	if ((data[0] == 0x04))
		OUTF("00000100 Type: CP-ACK\n");
	else if (data[0] == 0x10)
		OUTF("00010000 Type: CP-ERROR\n");
	else if (data[0] == 1) {
		OUTF("00000001 Type: CP-DATA\n");
		data++;
		cpData();
	} else
		OUTF("%s UNKNOWN\n", BitRowFill(data[0], 0xff));
}

static void
cpDataUp()
{
	if (data >= end)
		RETTRUNK();
	OUTF("%s Parameter %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();
	OUTF("%s Parameter %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();
	OUTF("%s Parameter %u\n", BitRowFill(data[0], 0xff), data[0]);
	data++;
	Address("SMSC");
	if (data >= end)
		RETTRUNK();

	/* FIXME: Be more detailed here about message flags */
	OUTF("%s Message Flags: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data + 1 >= end)
		RETTRUNK();
	int num = data[0] << 8;
	OUTF("%s Reference Number [continue]\n", BitRowFill(data[0], 0xff));
	data++;
	num |= data[0];
	OUTF("%s Reference Number: %u\n", BitRowFill(data[0], 0xff), num);
	data++;

	/* Destination address */
	Address("Destination");
	SmsProtocolDataValidity();
	if (data >= end)
		RETTRUNK();
}

static void
cpData()
{
	if (data >= end)
		RETTRUNK();
	OUTF("%s Length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	data++;
	if (data >= end)
		RETTRUNK();
	if ((data[0] & 1) == 0)
	{
		//OUTF("xxxxxxx0 cpDataUp FIXME\n");
		//data++;
		cpDataUp();
		return;
	}
	OUTF("%s Parameter\n", BitRowFill(data[0], 0xff));
	if (++data >= end)
		RETTRUNK();
	OUTF("%s Parameter\n", BitRowFill(data[0], 0xff));
	if (++data >= end)
		return; /* Can happen that msg terminated here... */

	Address("SMSC");
	if (data >= end)
		RETTRUNK();

	OUTF("%s TP-MTI, TP-MMS, TP-SRI, TP-UDIH, TP-RP: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();
	OUTF("%s Reference number: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if ((data[0]) == 0x44)
	{
		OUTF("FIXME: ems_type\n");
		return;
	}
	if (++data >= end)
		RETTRUNK();

	OUTF("%s Parameter\n", BitRowFill(data[0], 0xff));
	data++;

	Address("Destination");
	SmsProtocolDataValidity();

	int c = 0;
	while (c++ < 7)
	{
		OUTF("%s Parameter%u\n", BitRowFill(data[0], 0xff), c);
		if (++data >= end)
			RETTRUNK();
	}
	OUTF("FIXME %p < %p\n", data, end);
}

static void
Address(const char *str)
{
	int len;

	if (data >= end)
		RETTRUNK();
	len = data[0];
	OUTF("%s %s Address Length: %u\n", BitRowFill(data[0], 0xff), str, data[0]);
	data++;
	if (len <= 0)
		return;
	if (data >= end)
		RETTRUNK();
	if (data[0] >> 7)
		OUTF("1------- Extension\n");

	OUTF("%s\n", id_list_get(list_SMSCAddressType, (data[0] >> 4) & 0x07));
	OUTF("%s\n", id_list_get(list_SMSCAddressNumberingPlan, data[0] & 0x0f));
	len--;
	data++;
	if (len <= 0)
		return;
	const unsigned char *thisend = data + len;
	if (thisend > end)
		thisend = end;

	OUTF("-------- Number(%d): ", len);
	while (data < thisend)
	{
		if ((data[0] >> 4) == 0x0f)
			OUT("%X", data[0] & 0x0f);
		else
			OUT("%X%X", data[0] & 0x0f, data[0] >> 4);
		data++;
	}
	OUT("\n");
}


static void
l2_RRpagingresponse()
{
	if (data >= end)
		RETTRUNK();

	if ((data[0] & 0x07) == 0x07)
		OUTF("-----111 Cipher key sequence: Key not available!\n");
	else
		OUTF("%s Ciphering key sequence: %u\n", BitRowFill(data[0], 0x07), data[0] & 0x07);

	OUTF("%s Ciphering key sequence: %u\n", BitRowFill(data[0], 0x70), (data[0] >> 4) & 0x07);

	data++;
	MSClassMarkTwo();
	if (data >= end)
		RETTRUNK();
	l2_MobId();
}

static void
l2_RRassignCommand()
{
	ChannelDescriptionTwo();
	if (data >= end)
		RETTRUNK();
	
	OUTF("%s Training seq. code: %d\n", BitRowFill(data[0], 0xe0), data[0] >> 5);
	if (((data[0] >> 2) & 0x07) == 0x00)
		l2_SingleChannelAssCom();
	else if (((data[0] >> 4) & 1) == 0x01)
		l2_HoppingChannelAssCom();
	else
		OUTF("xxx0??xxx UNKNOWN %d\n", (data[0] >> 3) & 0x3);
}

static void
l2_RRassignComplete()
{
	if (data >= end)
		RETTRUNK();

	OUTF("%s\n", id_list_get(list_ChannelRelease, data[0]));
	data++;
}

static void
ChannelDescriptionTwo()
{
	if (data >= end)
		RETTRUNK();
	OUTF("%s Timeslot number: %d\n", BitRowFill(data[0], 0x07), data[0] & 0x07);
	OUTF("%s Channel Description: %s\n", BitRowFill(data[0], 0xf8), id_list_get(list_ChannelDescriptionTwo, data[0] >> 3));

	data++;
}

static void
l2_SingleChannelAssCom()
{
	int freq = (data[0] & 0x03) << 8;

	data++;
	if (data >= end)
		RETTRUNK();
	freq |= data[0];
	OUTF("........ Absolute RF channel number: %u\n", freq);
	if (++data >= end)
		RETTRUNK();
	OUTF("%s Power Level: %u\n", BitRowFill(data[0], 0x1f), data[0] & 0x1f);
	if (++data >= end)
		RETTRUNK();
	if (data[0] != 0x63)
		return;
	if (++data >= end)
		RETTRUNK();
	OUTF("%s\n", id_list_get(list_ChannelMode, data[0]));
}

static void
l2_HoppingChannelAssCom()
{
	OUTF("FIXME %s\n", __func__);
}

static void
CCalerting()
{
	if (data >= end)
		RETTRUNK();
	if (data[0] != 0x1e)
		return;

	data++;
	ProgressIndicator();
}

static void
ProgressIndicator()
{
	if (data >= end)
		RETTRUNK();

	OUTF("%s Length of IE Progress Indicator: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();
	OUTF("%s Coding: %s\n", BitRowFill(data[0], 0x60), id_list_get(list_CodingStandard, (data[0] >> 5) & 0x03));
	OUTF("%s Location: %s\n", BitRowFill(data[0], 0x0f), id_list_get(list_Location, data[0] & 0x0f));
	if (++data >= end)
		RETTRUNK();
	OUTF("%s\n", id_list_get(list_Progress, data[0] & 0x7f));
	data++;
}

static void
CCsetup()
{
	if (data >= end)
		RETTRUNK();

	if (data[0] == 0x04)
	{
		OUTF("00000100 Bearer Capability\n");
		data++;
		BearerCap();
	} else if (data[0] == 0x1e) {
		OUTF("00011110 Progress Indicator\n");
		ProgressIndicator();
		return;
	} else {
		OUTF("%s FIXME\n", BitRowFill(data[0], 0xff));
	}

	data++;
}

static void
Cause()
{
	if (data >= end)
		RETTRUNK();

	OUTF("%s Length of Cause: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();

	OUTF("%s Coding: %s\n", BitRowFill(data[0], 0x60), id_list_get(list_CodingStandard, (data[0] >> 5) & 0x03));
	OUTF("%s Location: %s\n", BitRowFill(data[0], 0x0f), id_list_get(list_Location, data[0] & 0x0f));
	if (++data >= end)
		RETTRUNK();

	OUTF("%s Cause: %s\n", BitRowFill(data[0], 0x7f), id_list_get(list_Cause, data[0] & 0x7f));
	data++;
}

static void
l2_RRclassmarkChange()
{
	if (data >= end)
		RETTRUNK();
	MSClassMarkTwo();
	if (data >= end)
		RETTRUNK();
	if (data[0] == 0x20)
	{
		OUTF("00100000 Class Mark 3 [FIXME]\n");
		data++;
	}
}

static void
SmsProtocolDataValidity()
{
	if (data >= end)
		RETTRUNK();
	OUTF("%s Protocol Identifier: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();
	OUTF("%s Data Coding Sheme: %u\n", BitRowFill(data[0], 0xff), data[0]);
	data++;
}

static void
BearerCap()
{
	int len;

	if (data >= end)
		RETTRUNK();

	len = data[0];
	OUTF("%s Length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (data + len > end)
		len = end - data;
	if (++data >= end)
		RETTRUNK();

	OUTF("%s Radio Channel: %s\n", BitRowFill(data[0], 0x60), id_list_get(list_RadioChannelReq, (data[0] >> 5) & 0x03));
	if ((data[0] >> 4) & 1)
		OUTF("---1---- Coding Standard: reserved\n");
	else
		OUTF("---0---- Coding Standard: GSM\n");
	if ((data[0] >> 3) & 1)
		OUTF("----1--- Transfer Mode: Packet\n");
	else
		OUTF("----0--- Transfer Mode: Circuit\n");

	OUTF("%s Transfer Capability: %s\n", BitRowFill(data[0], 0x07), id_list_get(list_TransferCap, data[0] & 0x07));
	//len--;
	OUTF("FIXME: Stuff missing here\n");
#if 0
	if (++data >= end)
		RETTRUNK();
	if ((data[0] >> 6) & 1)
		OUTF("-1------ Coding: octet 3 extended [FIXME]\n");
	else
		OUTF("-0------ Coding: octet 3 extended for inf. trans. cap\n");
	
	/* FIXME: Stuff missing here */
#endif
}

static void
AuthenticationRequest()
{
	char rand[16 * 2 + 1];

	if (data >= end)
		RETTRUNK();

	OUTF("%s Cipher Key Sequence Number: %u\n", BitRowFill(data[0], 0x07), data[0] & 0x07);
	data++;
	if (data + 16 > end)
		RETTRUNK();

	snprintf(rand, sizeof rand, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
	OUTF("-------- RAND: %s\n", rand);
	data += 16;
}

static void
AuthenticationResponse()
{
	char sres[4 * 2 + 1];

	if (data + 4 > end)
		RETTRUNK();
	snprintf(sres, sizeof sres, "%02x%02x%02x%02x", data[0], data[1], data[2], data[3]);
	OUTF("-------- SRES: %s\n", sres);
	data += 4;
}

