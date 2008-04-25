/*
 * TODO: memcpy for concatendated sms is unchecked.
 */
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <arpa/inet.h>
//#include <netinet/in.h>
#include "id_list.h"
#include "gsm_desc.h"
#include "mcc_list.h"

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

extern struct _opt opt;

/* Prototype helper functions */
static unsigned int Bit_extract_val(unsigned char *data, int bit_pos, int numbits);


static void l2_rrm();
static void l2_sms();
static void l2_cc();
static void l2_RRsystemInfo1();
static void l2_MccMncLac();
static void l2_RRsystemInfo2();
static void l2_RRsystemInfo2bis();
static void l2_RRsystemInfo2ter();
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
static void l2_HoppingChannelC();
static void l2_SingleChannel();
static void l2_SingleChannelC();
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
static void l2_NonCallSS();
static void l2_FacilityRegister();
static void l2_FacilityInvoke();
static void l2_Facility();
static void l2_FacilityReturnResult();
static void l2_UssRequest();
static void l2_UssData();
static void l2_CCReleaseComplete();

static void l2_ChannelNeeded(char *str, unsigned char ch);
static int l2_MNCC(int mcc, unsigned char a, unsigned char b, unsigned char c);

static void maio();
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
static void TypeOfIdentity();
static void l2_NeighbourCellDescription();
static void CellIdentity();
static void MSClassMarkTwo();
static void MSClassMarkOne();
static void ClassMarkThree();
static void cpData();
static void Address(const char *str);
static void TPAddress(const char *str);
static void ChannelDescriptionTwo();
static void CCalerting();
static void CCsetup();
static void ProgressIndicator();
static void Cause();
static void SmsProtocolDataValidity();
static void BearerCap();
static void Number(int len);
static void BCDNumber();
static void AuthenticationRequest();
static void AuthenticationResponse();
static void sms_dcs();
static void sms_udh();
static void sms_default_alphabet();
static void SmscTimestamp();
static void simdatadownload();
static void LocationUpdateRequest();
static void MultiSupportTwo();
static void MeasurmentReport();
static int CellAllocation(unsigned char format, char *str);
static void ChannelMode();

static const unsigned char *start;
static const unsigned char *data;
static const unsigned char *end;

struct _nfo
{
	unsigned int flags;
	unsigned char seq_counter;
	unsigned char sapi;
	int direction;	/* 0 == downlink, 1 == uplink */
};
#define GSMSP_NFO_SMS			(0x01)
#define GSMSP_NFO_SEGMENTATION		(0x02)
#define GSMSP_NFO_UDHI			(0x04)
#define GSMSP_NFO_DEFAULTALPHABET	(0x08)
#define GSMSP_NFO_SIMDATADL		(0x10)	/* Sim Data Download */
#define GSMSP_NFO_SMSCON		(0x20)	/* Concatenated SMS */
#define GSMSP_NFO_LASTSMSCHUNK		(0x40)

static struct _nfo nfo;

#if 0
struct _sms
{
	unsigned char buf[248 + 3];
	unsigned char *ptr;
};
#endif

/* For concatenating SMS'es */
struct _sms_con
{
	unsigned char buf[8192];
	unsigned char *ptr;
};

struct _con
{
	unsigned char buf[248 + 3];
	unsigned char *ptr;
	int logicalchannel;
};
/* Is initialized to 0 (do not remove from .bss) */
static struct _sms_con sms_con;

struct _con con[8];
struct _con conuplink[8];
struct _con *conptr;



/*
 * B-format (and also A-Format)
 */
void
l2_data_out_B(int fn, const unsigned char *input_data, int len, int logicalchannel, int direction)
{
	const unsigned char *from;
	int val;

	data = input_data;
	start = data;
	end = data + len;

	HEXDUMPF(data, 23 /*len*/, "Format B DATA (%s)\n", direction?"up":"down");

	/* Do not analyze dummy octets */
	if (memcmp(data + 3, "\x2b\x2b\x2b\x2b\x2b\x2b\x2b\x2b\x2b\x2b\x2b\x2b\x2b\x2b\x2b\x2b\x2b\x2b\x2b\x2b", 20) == 0)
		return;

	//printf("Logical channel: %d\n", logicalchannel);

	/* Need at least 3 octets */
	if (data + 2 >= end)
		RETTRUNK();

	memset(&nfo, 0, sizeof nfo);
	nfo.direction = direction;
	dcch_address();
	data++;
	dcch_control();
	data++;
	/* FIXME: Why is extended length always set to 1? */
	OUTF("%s EL, Extended Length: %s\n", BitRow(data[0], 0), (data[0] & 1)?"y":"n");
	OUTF("%s M, segmentation: %c\n", BitRow(data[0], 1), ((data[0] >> 1) & 1)?'Y':'N');

	/* Initialization. have to do this only once but there
	 * is no better place to do it atm
	 */
	if (conptr->ptr == NULL)
		conptr->ptr = conptr->buf;

	if ((data[0] >> 1) & 1)
	{
		/* Segmentation: y */
		if ((logicalchannel != conptr->logicalchannel) && (conptr->ptr > conptr->buf))
		{
			/* Currently we only support 1 segmented stream
 			 * at the same time per direction.
 			 * If downlink sends two segments on two different
 			 * logical channels then we can not handle this..
 			 */
			OUTF("WARN: Two segmented streams at the same time..\n");
			/* Reset */
			conptr->ptr = conptr->buf;
		}
		nfo.flags |= GSMSP_NFO_SEGMENTATION;
		conptr->logicalchannel = logicalchannel;
	}

	val = data[0] >> 2;
	OUTF("%s Length: %u\n", BitRowFill(data[0], 0xfc), val);
	if (data + val < end)
	{
		end = data + val + 1;
	}

	data++;
	if (data >= end)
		return;

	/* Chunk of a fragmented. */
	/* All SMS type messages go into the same buffer.
	 * Other segmented messages are currently not supported.
	 */
	//if (nfo.flags & GSMSP_NFO_SMS)
	if ((logicalchannel == conptr->logicalchannel) && ((conptr->ptr > conptr->buf) || (nfo.flags & GSMSP_NFO_SEGMENTATION)))
	{
		from = data;

		if (conptr->ptr + (end - data) < conptr->buf + sizeof conptr->buf)
		{
			memcpy(conptr->ptr, data, end - data);
			conptr->ptr += (end - data);
		} else {
			OUTF("ERROR, buffer to small!!!\n");
			OUTF("buf: %p, ptr %p, filled: %d len %d\n", conptr->buf, conptr->ptr, conptr->ptr - conptr->buf, end - from);
		}
	}

	if (conptr->ptr > conptr->buf)
	{
		if (nfo.flags & GSMSP_NFO_SEGMENTATION)
		{
			OUTF("-------- [SEGMENTED MESSAGE. MORE DATA FOLLOWS...]\n");
			/* More fragments follow. No need to decode yet */
			return;
		} else if (logicalchannel == conptr->logicalchannel) 
			OUTF("-------- [SEGMENTED MESSAGE. LAST...]\n");
	}

	/* Here: segmentation == No */
	/* See if we get an SMS message and if this was the last fragment */
	/* Currently only 1 logical channel can contain segmented data.
 	 * If there are two channels that both send segmented data
 	 * then it's gettin muddled up.
 	 */
	if ((logicalchannel == conptr->logicalchannel) && (conptr->ptr > conptr->buf))
	{
		start = conptr->buf;
		data = conptr->buf;
		end = conptr->ptr;

		if (nfo.flags & GSMSP_NFO_SMS)
			HEXDUMPF(data, end - data, "Format SMS data\n");
		else
			HEXDUMPF(data, end - data, "Format Bbis (RR, MM or CC)\n");

		l2_Bbis();
		conptr->ptr = conptr->buf;
		return;
	}
		
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
			nfo.seq_counter = ((data[0] >> 1) & 0x07);
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
	case 0x2c: /* 001-11-- According to J.Goeller this is SABM */
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
		OUTF("011-00-- UA frame (Unnumbered acknowledgement)\n");
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
		nfo.sapi = (data[0] >> 2) & 0x07;
		if (nfo.direction == 1)
		{
			conptr = &conuplink[nfo.sapi];
		} else {
			conptr = &con[nfo.sapi];
		}

		switch ((data[0] >> 2) & 0x07)
		{
		case 0x03:
			nfo.flags |= GSMSP_NFO_SMS;
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
			nfo.flags |= GSMSP_NFO_SMS;
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

	memset(&nfo, 0, sizeof nfo);
	if (len <= 0)
		return;

	data = input_data;
	start = data;

	/* 2008-01-05: Motorola output has Length field wrongly set.. */
	if (opt.flags & FL_MOTOROLA)
		i = data[0];
	else
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
l2_Bbis()	/* GSM 04.07 11.2.3.2.1 */
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
		OUTF("----1011 Non-call related SS message\n");
		data++;
		l2_NonCallSS();
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
	if (data < end)
		OUTF("XXXXXXXX UNKNOWN DATA (%d bytes)\n", end - data);
	if (data > end)
	{
		OUTF("INTERNAL ERROR. Processed to many data\n");
		return;
	}

	while (1)
	{
		if (end >= start + 23)
			break;
		if (*end == 0x2b)
			break;
		end++;
	}
	if (end > data)
	{
		OUTF("YYYYYYYY REST OCTETS (%d)\n", end - data);
	}
}

/*
 *  Extract up to 24 bit from a data field and return as integer.
 */
static unsigned int
Bit_extract_val(unsigned char *data, int bit_pos, int numbits)
{
	unsigned int ofs;
	unsigned int val = 0;
	unsigned int bit_ofs;
	char len;
	char overlap;
	int i;

	ofs = bit_pos / 8;
	bit_ofs = bit_pos % 8;

	len = ((7 - bit_ofs) + numbits + 7) / 8;
	//OUTF("len = %u\n", len);

	i = 0;
	while (i < len)
	{
		val = val << 8;
		val = val | data[i + ofs];
		i++;
	}
	overlap = (numbits + (7 - bit_ofs)) % 8;
	//OUTF("overlap = %u bit\n", overlap);
	if (overlap != 0)
	{
		val = val >> (8 - overlap);
	}
	val = val & (((unsigned int)1 << numbits) - 1);

	return val;
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
		OUTF("--000100 Message Type: Location Updating Reject\n");
		break;
	case 8:
		OUTF("--001000 MM Location Update Request\n"); /* FIXME */
		data++;
		LocationUpdateRequest();
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
	case 0x18:
		OUTF("--011000 MMIdentityRequest\n");
		data++;
		TypeOfIdentity();
		break;
	case 0x19:
		OUTF("--011001 MMidentityResponse\n");
		data++;
		l2_MobId();
		break;
	case 0x1a:
		OUTF("--011010 TMSI Realloc Command\n");
		data++;
		l2_TmsiReallocCommand();
		break;
	case 0x21:
		OUTF("--100001 CM Service Accept\n");
		data++;
		break;
	case 0x24:	/* --100100 */
		OUTF("--100100 MMcmServiceRequest\n");
		data++;
		l2_MMcmServiceRequest();
		/* in multisupport2 and others! */
		break;
	default:
		OUTF("UNKNOWN\n");
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
	case 0x02:
		OUTF("00000010 System Information Type 2bis\n");
		data++;
		l2_RRsystemInfo2bis();
		break;
	case 0x03:
		OUTF("00000011 System Information Type 2ter\n");
		data++;
		l2_RRsystemInfo2ter();
		break;
	case 0x05:
		OUTF("00000101 System Information Type 5bis\n");
		data++;
		l2_BcchAllocation();
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
		OUTF("00010101 RR Measurement Report C\n");
		data++;
		MeasurmentReport();
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
		OUTF("--0----- Downlink assign to MS: No meaning\n");
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
	l2_SingleChannelC();
	RequestReference();
	TimingAdvance();
	l2_MobileAllocation();
	if (data >= end)
		return;
}

static void
l2_SingleChannelC()
{
	int freq;
	if (data + 1 >= end)
		RETTRUNK();
	OUTF("%s Training seq. code: %d\n", BitRowFill(data[0], 0xe0), data[0] >> 5);
	OUTF("---0---- Single channel\n");
	freq = (data[0] & 0x03) << 8;
	data++;
	freq |= data[0];
	OUTF("........ Absolute RF channel number: %u\n", freq);
	data++;
}

static void
l2_HoppingChannel()
{
	OUTF("%s Training seq. code : %d\n", BitRowFill(data[0], 0xe0), data[0] >> 5);

	maio();
	RequestReference();
	TimingAdvance();
	l2_MobileAllocation();
	if (data >= end)
		return;	/* finished. not truncated! */

	OUTF("FIXME, more data left here???\n");
}

static void
maio()
{
	unsigned char maio = 0;

	OUTF("---1---- HoppingChannel\n");
	maio = (data[0] & 0x0f) << 2;
	data++;
	if (data >= end)
		RETTRUNK();
	maio |= (data[0] >> 6);
	OUTF("........ Mobile Allocation Index Offset (MAIO) %d\n", maio);
	OUTF("%s Hopping Seq. Number: %d\n", BitRowFill(data[0], 0x3f), data[0] & 0x3f);

	data++;
}

static void
l2_HoppingChannelC()
{
	OUTF("FIXME-2\n");
}

static void
l2_MobileAllocation()
{
	int pos;
	const unsigned char *thisend;
	int len;

	if (data >= end)
		RETTRUNK();
	OUTF("%s Length of Mobile Allocation: %d\n", BitRowFill(data[0], 0xff), data[0]);

	len = data[0];

	thisend = data + len + 1;
	if (thisend > end)
	{
		OUTF("xxxxxxxx ERROR: Packet to short or length to long\n");
		thisend = end;
	}

	data++;
	/* If mobile allocation has length 0 */
	if (data >= thisend)
		return;

	/* This is the index into the list of arfcn's */
	pos = 7;
	while (data < thisend)
	{
		while (pos >= 0)
		{
			if ((data[0] >> pos) & 1)
			{
				OUTF("%s Mobile Allocation ARFCN #%d\n", BitRow(data[0], pos), 8 * len - (7 - pos));

			}
			pos--;
		}
		pos = 7;
		len--;
		data++;
	}
}

/*
 * From RRsystemInfo2
 */
static void
l2_BcchAllocation()
{
	OUTF("---x---- BCCH alloc. seq. num: %d\n", (data[0] >> 4) & 1);
	CellAllocation(data[0], "BCCH Allocation    : ARFCN");
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
	if (data == end)
		return;
	/* FIXME: rest octets?? */
	OUTF("FIXME: implenet\n");
}

static void
CellAllocationBitmapZero(char *str)
{
	int ca;

	if ((data[0] >> 3) & 1)
		OUTF("----1--- %s 124\n", str);
	if ((data[0] >> 2) & 1)
		OUTF("-----1-- %s 123\n", str);
	if ((data[0] >> 1) & 1)
		OUTF("------1- %s 122\n", str);
	if (data[0] & 1)
		OUTF("-------1 %s 121\n", str);

	ca = 120;
	while (ca > 0)
	{
		data++;
		if (data >= end)
			return;
		if ((data[0] >> 7) & 1)
			OUTF("1------- %s %d\n", str, ca);
		if ((data[0] >> 6) & 1)
			OUTF("-1------ %s %d\n", str, ca - 1);
		if ((data[0] >> 5) & 1)
			OUTF("--1----- %s %d\n", str, ca - 2);
		if ((data[0] >> 4) & 1)
			OUTF("---1---- %s %d\n", str, ca - 3);
		if ((data[0] >> 3) & 1)
			OUTF("----1--- %s %d\n", str, ca - 4);
		if ((data[0] >> 2) & 1)
			OUTF("-----1-- %s %d\n", str, ca - 5);
		if ((data[0] >> 1) & 1)
			OUTF("------1- %s %d\n", str, ca - 6);
		if (data[0] & 1)
			OUTF("-------1 %s %d\n", str, ca - 7);

		ca -= 8;
	}
	data++;
}

/*
 * Return number of bits required to store val.
 */
static int
num_bits(unsigned int val)
{
	int i = 32;

	while (1)
	{
		i--;
		if (i <= 0)
			break;
		if ((((unsigned int)1) << i) <= val)
			break;
	}

	return i + 1;
}

static int
Bitmap256_extract_frequency(unsigned short *w, int index)
{
	int j;
	int n = w[index];

	j = 1 << (num_bits(index) - 1);
	//OUTF("j = %d\n", j);
	while (index > 1)
	{
		if (2 * index < 3 * j)
		{
			index = index - j / 2;
			n = (n + w[index] - 256 / j - 1) % (512 / j - 1) + 1;
		} else {
			index = index - j;
			n = (n + w[index] - 1) % (512 / j - 1) + 1;
		}
		j = j / 2;
	}
	//OUTF("freq %d\n", (w[0] + n) % 1024);

	return (w[0] + n) % 1024;
}


static void
CellAllocationBitmap256(char *str)
{
	int arfcn = 0;
	int i;
	unsigned short w[30];
	int pos, len, border;
	int ii;

	memset(w, 0, sizeof w);

	arfcn = (data[0] & 1) << 9;
	data++;
	arfcn |= (data[0] << 1);
	data++;
	arfcn |= (data[0] >> 7);
	w[0] = arfcn;

#if 0
	w[0] = 0xff;
	w[1] = 0x00;
	w[2] = 0xff;
	w[3] = 0x00;
	OUTF("XXX 6,8 254 %u\n", Bit_extract_val(w, 6, 8));
	OUTF("XXX 7,7 127 %u\n", Bit_extract_val(w, 7, 7));
	OUTF("XXX 7,8 255 %u\n", Bit_extract_val(w, 7, 8));
	OUTF("XXX 6,7 127 %u\n", Bit_extract_val(w, 6, 7));
	OUTF("XXX 0,8 128 %u\n", Bit_extract_val(w, 0, 8));
	OUTF("XXX 7,16 65280 %u\n", Bit_extract_val(w, 7, 16));
	OUTF("XXX 7,1 1 %u\n", Bit_extract_val(w, 7, 1));
	OUTF("XXX 8,1 0 %u\n", Bit_extract_val(w, 8, 1));
#endif

	pos = 6;
	len = 8;
	border = 2;
	for (i = 1; i <= 29; i++)
	{
		if (i == border)
		{
			border = border * 2;
			len--;
			if (len <= 0)
			{
				i--;
				break;
			}
		}
		w[i] = Bit_extract_val((unsigned char *)data, pos, len);
		if (w[i] == 0)
			break;
		pos += len;
	}
	//OUTF("%d entries\n", i);

	OUTF("xxxxxxxx %s %d (original)\n", str, w[0]);
	for (ii = 1; ii <= i; ii++)
	{
		OUTF("xxxxxxxx %s %d\n", str, Bitmap256_extract_frequency(w, ii));
	}
	data += (pos + 7) / 8;

}

static void
CellAllocationBitmapVariable(char *str)
{
	int arfcn = 0;
	int i;
	const unsigned char *thisend;

	arfcn = (data[0] & 1) << 9;
	data++;
	arfcn |= (data[0] << 1);
	data++;
	arfcn |= (data[0] >> 7);

	OUTF("........ %s %d (original)\n", str, arfcn);

	for (i = 1; i < 8; i++)
	{
		if (BIT(data[0], 7-i))
			OUTF("%s %s %d\n", BitRow(data[0], 7-i), str, (arfcn + i) % 1024);
	}
	data++;

	thisend = data + 13;
	while (data < thisend)
	{
		for (i = 0; i < 8; i++)
		{
			if (BIT(data[0], 7 - i))
				OUTF("%s %s %d\n", BitRow(data[0], 7-i), str, (arfcn + (104 + i)) % 1024);
		}
		data++;
	}
}

/*
 * Return 0 on success (e.g. caller continues processing).
 * Cell Channel Description (for example SystemInformationType 1)
 */
static int
CellAllocation(unsigned char format, char *str)
{
	const unsigned char *orig_data = data;

	if ((format >> 6) == 0x00)
	{
		OUTF("00------ Bitmap 0 format\n");
		CellAllocationBitmapZero(str);
		data = orig_data + 16;
		return 0;
	}
	if (((format >> 3) & 1) == 0x00)
	{
		OUTF("-------- Bitmap format: 1024 Range (FIXME)\n");
		data = orig_data + 16;
		return -1;
	}

	switch ((format >> 1) & 0x07)
	{
	case 0x04:
		OUTF("10--100- Bitmap format: 512 Range (FIXME)\n");
		data = orig_data + 16;
		return -1;
	case 0x05:
		OUTF("10--101- Bitmap format: 256 Range\n");
		CellAllocationBitmap256(str);
		data = orig_data + 16;
		return 0;
	case 0x06:
		OUTF("10--110- Bitmap format: 128 Range (FIXME)\n");
		data = orig_data + 16;
		return -1;
	case 0x07:
		OUTF("10--111- Bitmap format: Variable Range\n");
		CellAllocationBitmapVariable(str);
		data = orig_data + 16;
		return 0;
	}

	return -1;
}

static void
l2_RRsystemInfo1()
{

	if (data + 1 >= end)
		return;

	if (CellAllocation(data[0], "Cell Allocation     : ARFCN") != 0)
		return;

	l2_RachControlParameters();
	if (data >= end)
		return;
	OUTF("FIXME: NCH Position\n");
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

/*
 * GSM 04.08-9.1.32
 */
static void
l2_RRsystemInfo2()
{
	if (data >= end)
		RETTRUNK();

	/* Neighbour Cell Description. 16 octets */
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

static void
l2_RRsystemInfo2bis()
{
	if (data >= end)
		RETTRUNK();

	/* Extended BCCH Frequency List. 16 octets */
	/* Neighbour Cell Description. 10.5.2.22 */
	l2_BcchAllocation();

	/* 3 octets */
	l2_RachControlParameters();
}

static void
l2_RRsystemInfo2ter()
{
	if (data >= end)
		RETTRUNK();

	/* Neighbour Cell Description 2 */
	l2_BcchAllocation();
}


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
	if (data + 1 >= end)
		RETTRUNK();

	if (data[0] != 0x64)
	{
		OUTF("UNKNWON\n");
		return;
	}
	OUTF("01100100 Channel Description\n");
	data++;
	ChannelDescriptionTwo();
	if (data >= end)
		RETTRUNK();
	//OUTF("%s Training sequence code: %d\n", BitRowFill(data[0], 0xe0), data[0] >> 5);
	if (((data[0] >> 3) & 0x1) == 0)
	{
		l2_SingleChannelC();
	} else {
		l2_HoppingChannelC();
	}

	if (data >= end)
		return;

	if (data[0] != 0x72)
	{
		OUTF("UNKNOWN\n");
		return;
	}
	OUTF("01110010 CBCH Mobile Allocation\n");
	data++;
	if (data >= end)
		RETTRUNK();
	l2_MobileAllocation();

//	OUTF("FIXME\n");
}

/*
 * Output MCC, MNC and LAC. consume 5 bytes.
 */
static void
l2_MccMncLac()
{
	int mcc;

	if (data + 2 >= end)
		return;
	unsigned short lac;
	
	mcc = l2_MNCC(0, data[0] & 0x0f, (data[0] >> 4) & 0x0f, data[1] & 0x0f);
	data++;
	l2_MNCC(mcc, data[1] & 0x0f, (data[1] >> 4) & 0x0f, (data[0] >> 4) & 0x0f);
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
	data++;
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

/*
 * If mcc is 0 then output MCC and return MCC as value.
 * Otherwise output MNC.
 */
static int
l2_MNCC(int mcc, unsigned char a, unsigned char b, unsigned char c)
{
	char buf[128];
	char f[12];
	const char *country;
	const char *mnc;

	snprintf(f, sizeof f, "%x%x%x", a, b, c);
	/* Nokia netmonitor shows NC's like '30F' and '10F' */
	if (mcc == 0)
	{
		/* Find out MCC */
		mcc = atoi(f);
		country = mcc_get(mcc);
		if (country == NULL)
			country = "UNKNOWN";
		snprintf(buf, sizeof buf, "%-8s %s (%s)\n", f, "Mobile Country Code", country);
	} else {
		mnc = mnc_get(mcc, atoi(f));
		snprintf(buf, sizeof buf, "%-8s %s (%s)\n", f, "Mobile Network Code", mnc);
	}

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

	return mcc;
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
	OUTF("xxxxxxxx Mobile Allocation Index Offset (MAIO): %u\n", maio);
	OUTF("%s Hopping Sequence Number: %u\n", BitRowFill(data[0], 0x3f), data[0] & 0x3f);
	data++;

	RequestReference();

	TimingAdvance();
	l2_MobileAllocation();

	if (data >= end)
		return;
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
MSClassMarkOne()
{
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
}

static void
MSClassMarkTwo()
{
	if (data >= end)
		RETTRUNK();

	OUTF("%s MS Classmark 2 length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	data++;
	MSClassMarkOne();

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
		cpData();
	} else
		OUTF("%s UNKNOWN\n", BitRowFill(data[0], 0xff));
	data++;
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

/* From Network to MS */
/* Called when a full 140 byte SMS is received */
static void
cpData()
{
	int n_symbols;
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
	OUTF("%s reserved\n", BitRowFill(data[0], 0xf8));
	OUTF("%s Message Type Indicator(MTI): %s\n", BitRowFill(data[0], 0x07), id_list_get(list_MTI, data[0] & 0x07));
	if (++data >= end)
		RETTRUNK();
	OUTF("%s Message Reference: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		return; /* Can happen that msg terminated here... */

	/* RP-address */
	Address("SMSC");
	if (data >= end)
		RETTRUNK();

#if 0
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
#endif

	TPAddress("Destination");
	OUTF("%s TP-MTI: %s\n", BitRowFill(data[0], 0x03), id_list_get(list_TP_MTI, data[0] & 0x03));

	/* SMS-DELIVER and SMS-STATUS-REPORT.
	 * This may be different for other SMS-types! (FIXME) */
	if ((data[0] >> 2) & 1)
		OUTF("-----1-- More Messages (TP-MMS): No\n");
	else
		OUTF("-----0-- More Messages (TP-MMS): Yes\n");
	OUTF("%s Status Report Indication (TP-SRI)\n", BitRowFill(data[0], 0x20));
	if ((data[0] >> 6) & 1)
	{
		OUTF("-1------ User Data Header Indicator (TP-UDHI): Yes\n");
		nfo.flags |= GSMSP_NFO_UDHI;
	} else {
		OUTF("-0------ User Data Header Indicator (TP-UDHI): No\n");
	}
	OUTF("%s Reply Path (TP-RP)\n", BitRowFill(data[0], 0x80));
	/* SMS-DELIVER and SMS-STATUS-REPORT end */
	data++;

	/* FIXME: Why is TPAddress differently encoded as SMSC address?
	 * Am i doing something wrong or are the GSM people nuts?
	 */
	TPAddress("Originating (TP-OA)");

	SmsProtocolDataValidity();

	SmscTimestamp();
	if (data >= end)
		RETTRUNK();
	n_symbols = data[0];
	OUTF("%s User Data Length (TP-UDL): %u symbols\n", BitRowFill(data[0], 0xff), data[0]);
 
	data++;
#if 0
	if ((data[0] >> 6) & 1)
	{
		OUTF("-1------ TP User Data Header Indicator (TP-UDHI): yes\n");
		data++;
		OUTF("-------- FIXME: Decode header\n");
		data += (data[0] + 1);
		/* FIXME: Skip over fill bits as well. GSM 03.40 9.2.3.24 */
	} else {
		OUTF("-0------ TP User Data Header Indicator (TP-UDHI): no\n");
		//data++; /* Contains directly the data!!! */
	}
#endif
	
	if (sms_con.ptr == NULL)
		sms_con.ptr = sms_con.buf;

	if (nfo.flags & GSMSP_NFO_UDHI)
		sms_udh();
	if ((nfo.flags & GSMSP_NFO_SMSCON) && (!(nfo.flags & GSMSP_NFO_LASTSMSCHUNK)))
	{
		HEXDUMPF(data, end - data, "TP-UD\n");
		/* If this is a concatendated SMS then wait for rest of data */
		return;
	}
	if (sms_con.ptr > sms_con.buf)
	{
		/* was a concatenated sms. output concatendated TP-UD */
		data = sms_con.buf;
		start = data;
		end = sms_con.ptr;
	}

	if (nfo.flags & GSMSP_NFO_DEFAULTALPHABET)
	{
		if (((end - data) * 8 ) / 7 < n_symbols)
			n_symbols = ((end - data) * 8) / 7;
		sms_default_alphabet(n_symbols);
	} else if (nfo.flags & GSMSP_NFO_SIMDATADL) {
		HEXDUMPF(data, end - data, "Format Sim Data Download\n");
		simdatadownload();
	} else {
		HEXDUMPF(data, end - data, "TP-UD\n");
	}
	/* Reset concatendating sms counter again */
	if ((sms_con.ptr > sms_con.buf) && (nfo.flags & GSMSP_NFO_LASTSMSCHUNK))
	{
		memset(&sms_con, 0, sizeof sms_con);
	}
}

static unsigned char sms_default_alpha[] = ""
"@_$__________\r__"
"________________"
" !\"#_%&`()*+,-./"
"0123456789:;<=>?"
"_ABCDEFGHIJKLMNO"
"PQRSTUVWXYZ_____"
"_abcdefghijklmno"
"pqrstuvwxyz_____";

/*
 * Len is the number of symbols (not number of bytes)
 */
static void
sms_default_alphabet(int len)
{
	unsigned char buf[512];
	unsigned char *ptr = buf;
	unsigned char *buf_end = buf + sizeof buf;

	while (ptr < buf_end)
	{
		*ptr++ = sms_default_alpha[data[0] & 0x7f];
		if (--len <= 0) break;
		*ptr++ = sms_default_alpha[((data[0] >> 7) & 0x01) | ((data[1] & 0x3f) << 1)];
		if (--len <= 0) break;
		*ptr++ = sms_default_alpha[((data[1] >> 6) & 0x03) | ((data[2] & 0x1f) << 2)];
		if (--len <= 0) break;
		*ptr++ = sms_default_alpha[((data[2] >> 5) & 0x07) | ((data[3] & 0x0f) << 3)];
		if (--len <= 0) break;
		*ptr++ = sms_default_alpha[((data[3] >> 4) & 0x0f) | ((data[4] & 0x07) << 4)];
		if (--len <= 0) break;
		*ptr++ = sms_default_alpha[((data[4] >> 3) & 0x1f) | ((data[5] & 0x03) << 5)];
		if (--len <= 0) break;
		*ptr++ = sms_default_alpha[((data[5] >> 2) & 0x3f) | ((data[6] & 0x01) << 6)];
		if (--len <= 0) break;
		*ptr++ = sms_default_alpha[((data[6] >> 1) & 0x7f)];
		if (--len <= 0) break;
		data += 7;
	}
	data = end;
	*ptr = '\0';
	OUTF("-------- Content: %s\n", buf);
}

/* User Data Header (GSM 03.40:9.2.3.24) */
static void
sms_udh()
{
	int len_udh;
	int len;
	int seq_total;
	const unsigned char *end_udh;
	const unsigned char *next;

	if (data >= end)
		RETTRUNK();
	len_udh = data[0];
	end_udh = data + data[0] + 1;
	OUTF("%s User Data Header Length: %u octets\n", BitRowFill(data[0], 0xff), data[0]);
	data++;

	if (end_udh > end)
		end_udh = end;

	while (data < end_udh)
	{
		if ((data + 1 >= end_udh) || (data + data[1] + 2 > end_udh))
		{
			OUTF("ERR: Short data\n");
			break;
		}
		next = data + data[1] + 2;
		len = data[1];

		if (data[0] == 0) {
			OUTF("00000000 Concatenated short messages, 8-bit reference number\n");
			data++;
			OUTF("--------  Length: %u\n", data[0]);
			data++;
			OUTF("--------  Message Reference Number: %.2X\n", data[0]);
			data++;
			OUTF("--------  Number of Segments: %.2X\n", data[0]);
			seq_total = data[0];
			data++;
			OUTF("--------  Seq Number: %.2X\n", data[0]);
			//OUTF("DEBUG: %u += memcpy(%p, %p, %u)\n", sms_con.ptr - sms_con.buf, sms_con.ptr, end_udh, end-end_udh);
			memcpy(sms_con.ptr, end_udh, end - end_udh);
			sms_con.ptr += (end - end_udh);

			nfo.flags |= GSMSP_NFO_SMSCON;
			if (data[0] == seq_total)
				nfo.flags |= GSMSP_NFO_LASTSMSCHUNK;
			data++;
		} else if (data[0] == 1)
			OUTF("00000001 Special SMS Message Indication\n");
		else if (data[0] == 4) {
			OUTF("00000100 Application port address, 8 bit\n");
			OUTF("--------  Desitnation Port: %u (0x%.2X)\n", data[1], data[1]);
			OUTF("--------  Source Port: %u (0x%.2X)\n", data[2], data[2]);
			data += 3;
		} else if (data[0] == 5) {
			unsigned short dport, sport;
			dport = (data[2] << 8) + data[3];
			sport = (data[4] << 8) + data[5];
			OUTF("00000101 Application port address, 16 bit\n");
			data++;
			OUTF("--------  Length: %u\n", data[0]);
			data++;
			/* 0 - 15999 allocated by IANA
			 * 16000 - 16999 Available for allocation by application
			 * 17000 - 65535 reserved
			 */
			OUTF("--------  Destination Port: %u\n", dport);
			data += 2;
			OUTF("--------  Source Port: %u\n", sport);
			data += 2;
		} else if (data[0] == 6)
			OUTF("00000110 SMSC Control Parameters\n");
		else if (data[0] == 7)
			OUTF("00000111 UDH Source Indicator\n");
		else if (data[0] == 8) {
			OUTF("00001000 Concatenated short messages, 16-bit reference number\n");
			data++;
			OUTF("--------  Message Reference Number: %.4X\n", (data[0]<<8) + data[1]);
			data += 2;
			OUTF("--------  Number of Segments: %.2X\n", data[0]);
			seq_total = data[0];
			data++;
			OUTF("--------  Seq Number: %.2X\n", data[0]);
			memcpy(sms_con.ptr, end_udh, end - end_udh);
			sms_con.ptr += (end - end_udh);

			nfo.flags |= GSMSP_NFO_SMSCON;
			if (data[0] == seq_total)
				nfo.flags |= GSMSP_NFO_LASTSMSCHUNK;
			data++;
		} else if (data[0] == 9)
			OUTF("00001001 Wireless Control Message Protocol\n");
		else if ((data[0] >= 0x70) && (data[0] <= 0x7f))
			OUTF("%s SIM Toolkit Security Header\n", BitRowFill(data[0], 0xff));
		else if ((data[0] >= 0x80) && (data[0] <= 0x9f))
			OUTF("%s SME to SME specific use\n", BitRowFill(data[0], 0xff));
		else if ((data[0] >= 0xc0) && (data[0] <= 0xdf))
			OUTF("%s SC specific use\n", BitRowFill(data[0], 0xff));
		else
			OUTF("%s reserved\n", BitRowFill(data[0], 0xff));

		data = next;
	}

}

static void
TPAddress(const char *str)
{
	int len;

	if (data >= end)
		RETTRUNK();

	len = data[0];
	OUTF("%s %s Address Length: %u\n", BitRowFill(data[0], 0xff), str, data[0]);
	data++;
	if (len <= 0)
	{
		data++;
		return;
	}
	if (data >= end)
		RETTRUNK();
	if (data[0] >> 7)
		OUTF("1------- Extension\n");

	OUTF("%s\n", id_list_get(list_SMSCAddressType, (data[0] >> 4) & 0x07));
	OUTF("%s\n", id_list_get(list_SMSCAddressNumberingPlan, data[0] & 0x0f));
	data++;
	Number(len);
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
	{
		data++;
		return;
	}
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

/*
 * 04.08-9.1.2
 */
static void
l2_RRassignCommand()
{
	ChannelDescriptionTwo();
	if (data >= end)
		RETTRUNK();
	
	OUTF("%s Training seq. code: %d\n", BitRowFill(data[0], 0xe0), data[0] >> 5);

	/* Power Command 10.5.2.28 */
	/* Frequency List 10.5.2.13 */
	/* Cell Channel Description 10.5.2.1b */
	/* Multislot allocation... */
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
PowerLevel()
{
	OUTF("%s Power Level: %u\n", BitRowFill(data[0], 0x1f), data[0] & 0x1f);
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
	PowerLevel();
	if (data[0] != 0x63)
		return;
	if (++data >= end)
		RETTRUNK();
	ChannelMode();
}

static void
FrequencyList()
{
	/* Should be 16 */
	OUTF("%s Length: %d\n", BitRowFill(data[0], 0xff), data[0]);
	data++;
	CellAllocation(data[0], "Cell Allocation    : ARFCN");
}

static void
ChannelMode()
{
	OUTF("%s\n", id_list_get(list_ChannelMode, data[0]));
	data++;
}

static void
l2_HoppingChannelAssCom()
{
	maio();
	PowerLevel();
	while (data < end)
	{
		if (data[0] == 0x05)
		{
			data++;
			FrequencyList();
		} else if (data[0] == 0x63) {
			data++;
			ChannelMode();
		} else {
			OUTF("UNKNOWN. FIXME\n");
			break;
		}

	}
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

	while (data < end)
	{
		if (data[0] == 0x04)
		{
			OUTF("00000100 Bearer Capability\n");
			data++;
			BearerCap();
		} else if (data[0] == 0x1e) {
			OUTF("00011110 Progress Indicator\n");
			ProgressIndicator();
			return;
		} else if (data[0] == 0x5e) {
			OUTF("01011110 Called Party BCD Number\n");
			data++;
			BCDNumber();
		} else if (data[0] == 0xa1) {
			OUTF("10100001 CLIR supression\n");
			data++;
		} else if (data[0] == 0xa2) {
			OUTF("10100010 CLIR invocation\n");
			data++;
		} else {
			OUTF("%s FIXME\n", BitRowFill(data[0], 0xff));
			break;
		}
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
		OUTF("00100000 Class Mark 3\n");
		data++;
		ClassMarkThree();
	}
}

static void
MultiSupportTwo()
{
	int c = 0;
	if (data + 1 >= end)
		RETTRUNK();
	OUTF("0110---- P-GSM, E-GSM, R-GSM supported, DSC 1800 not supported\n");

	for (c = 3; c >= 0; c--)
	{
		if ((data[0] >> c) & 0x1)
			OUTF("%s A5/%d available\n", BitRowFill(data[0], 1<<c), 4+c);
		else
			OUTF("%s A5/%d not available\n", BitRowFill(data[0], 1<<c), 4+c);
	}
	data++;
	OUTF("%s Associated Radio capability 1 Power Class: %d\n", BitRowFill(data[0], 0xf), data[0] & 0xf);
	OUTF("%s Associated Radio capability 2 Power Class: %d\n", BitRowFill(data[0], 0xf0), data[0] >> 4);
	data++;
}

static void
ClassMarkThree()
{
	unsigned char c;

	if (data + 1 >= end)
		RETTRUNK();
	OUTF("%s Length: %d\n", BitRowFill(data[0], 0xff), data[0]);
	data++;

	c = data[0] >> 4;
	if ((c == 0x5) || (c == 0x6))
		MultiSupportTwo();
	else
		OUTF("FIXME\n");
}

static void
SmsProtocolDataValidity()
{
	unsigned char b7,b6,b5,b4;

	if (data >= end)
		RETTRUNK();
	OUTF("%s Protocol Identifier: 0x%.2X\n", BitRowFill(data[0], 0xff), data[0]);

	b7 = (data[0] >> 7) & 0x1;
	b6 = (data[0] >> 6) & 0x1;
	b5 = (data[0] >> 5) & 0x1;
	b4 = (data[0] >> 4) & 0x1;

	if ( !b7 && !b6 && b5) {
		OUTF("001----- Telematic Interworking\n");
	} 
	if ( !b7 && b6 ) {
		switch ( (data[0] & 0x3f ) ) {
			case 0x1f:
			OUTF("01011111  Return Call Message\n");
			break;
			case 0x3d:
			OUTF("01111101  ME Data Download\n");
			break;
			case 0x3e:
			OUTF("01111110  ME De-personalization SMS\n");
			break;
			case 0x3f:
			OUTF("01111111  SIM Data download\n");
			break;
			default:break;
		}
		
	} 

	if (data[0] == 0x00)
		OUTF("00000000  normal\n");

	if (data[0] == 0x40)
		OUTF("00101000  SMS PING\n");

	if (data[0] == 0x7f) {
		nfo.flags |= GSMSP_NFO_SIMDATADL;
	//	printf("set GSMSP_NFO_SIMDATADL=1\n");
	} else
		nfo.flags &= ~GSMSP_NFO_SIMDATADL;

	data++;
	sms_dcs();
}

static void
BearerCap()
{
	int len;
	char extension = 0;

	if (data >= end)
		RETTRUNK();

	len = data[0];
	OUTF("%s Length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (data + len > end)
		len = end - data;
	if (++data >= end)
		RETTRUNK();

	if ((data[0] >> 7) & 0x1)
	{
		extension = 1;
		OUTF("1------- Extension: yes\n");
	} else {
		OUTF("0------- Extension: no\n");
	//	extenstion = 0;
	}

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
	data++;

	if (extension)
	{
		OUTF("FIXME: Stuff missing here\n");
		/* FIXME: can be followed by antoher estension etc!*/
	}

	if ((data[0] >> 7) & 0x1)
	{
		extension = 1;
		OUTF("1------- Extension: yes\n");
	} else {
		OUTF("0------- Extension: no\n");
	//	extenstion = 0;
	}

	if ((data[0] >> 6) & 0x01)
		OUTF("-1------ Compression: yes\n");
	else
		OUTF("-0------ Compression: no\n");

	OUTF("%s Duplex Mode: %s\n", BitRowFill(data[0], 0x8), id_list_get(list_Duplex, data[0] & 0x8));
	if ((data[0] >> 1) & 0x1)
		OUTF("------1- Rate Request: Data 4.8 kb/s, full rate, n. transp. 6kb req\n");
	data++;

	if ((data[0] >> 7) & 0x1)
	{
		extension = 1;
		OUTF("1------- Extension: yes\n");
	} else {
		OUTF("0------- Extension: no\n");
	//	extenstion = 0;
	}
	OUTF("%s Rate Adaptation: %s\n", BitRowFill(data[0], 0x18), id_list_get(list_Rate, data[0] & 0x18));
	OUTF("%s Signalling Access Protocol: %s\n", BitRowFill(data[0], 0x7), id_list_get(list_Signalling, data[0] & 0x7));	
	data++;
	if ((data[0] & 0x1) == 1)
		OUTF("-------1 Asynchronous\n");
	else
		OUTF("-------0 Synchronous\n");
	data++;
	/* FIXME: some octets might continue here, depending
	 * on extension */
	OUTF("FIXME: some data might be in extentions\n");
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
LocationUpdateRequest()
{
	if (data >= end)
		RETTRUNK();
	OUTF("%s Cipher Key Sequence Number: %d\n", BitRowFill(data[0], 0x70), (data[0] >> 4) & 0x7);
	if (data[0] & 0x8)
		OUTF("----1--- Follow-on request pending\n");
	else
		OUTF("----0--- No follow-on request pending\n");
	OUTF("%s Location Update: %s\n", BitRowFill(data[0], 0x3), id_list_get(list_TypeOfLocationUpdate, data[0] & 0x03));
	data++;
	l2_MccMncLac();
	MSClassMarkOne();
	l2_MobId();
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


static void
sms_dcs()
{
	if (data >= end)
		RETTRUNK();

	if (data[0] == 0)
	{
		OUTF("00000000 Default Data Coding Scheme (default Alphabet)\n");
		nfo.flags |= GSMSP_NFO_DEFAULTALPHABET;
		data++;
		return;
	}

	if (((data[0] >> 6) & 0x03) == 0)
	{
		OUTF("00------ General Data Coding\n");
		if ((data[0] >> 5) & 1)
			OUTF("--1----- Compressed (TS 03.42)\n");
		else
			OUTF("--0----- Not compressed\n");
		if ((data[0] >> 4) & 1)
			OUTF("---1---- Message class meaning: yes\n");
		else
			OUTF("---0---- Message class meaning: no\n");


		OUTF("%s Coding: %s\n", BitRowFill(data[0], 0x0c), id_list_get(list_alphabet, (data[0] >> 5) & 0x03));
		/* If bit 4 is set then bit 0..1 have a Message Class meaning */
		if ((data[0] >> 4) & 1)
		{
			OUTF("%s Message Class: %s\n", BitRowFill(data[0], 0x03), id_list_get(list_MessageClassMeaning, data[0] & 0x03));
		} else {

			OUTF("%s Message Class: No meaning (see bit4)\n", BitRowFill(data[0], 0x03));
		}
	} else if ((((data[0] >> 6) & 0x03) == 0x03) && (((data[0] >> 4) & 0x03) != 0x03)) {
		if (((data[0] >> 4) & 0x0f) == 0x0c) {
			OUTF("1100---- Message Waiting Indicator Group: Discard Message\n");
		} else if (((data[0] >> 4) & 0x0f) == 0x0d) {
			OUTF("1101---- Message Waiting Indicator Group: Store Message\n");
		} else if (((data[0] >> 4) & 0x0f) == 0x0e) {
			OUTF("1110---- Message Waiting Indicator Group: Store Message\n");
		}
		OUTF("FIXME GSM 03.38\n");
	} else if (((data[0] >> 4) & 0x0f) == 0x0f) {
		OUTF("1111---- Data Coding/Message Class\n");
		OUTF("%s reserved\n", BitRowFill(data[0], 0x08));
		if ((data[0] >> 2) & 1)
			OUTF("-----1-- Message coding: 8 bit\n");
		else {
			OUTF("-----0-- Message coding: default alphabet\n");
			nfo.flags |= GSMSP_NFO_DEFAULTALPHABET;
		}
		OUTF("%s Message Class: %s\n", BitRowFill(data[0], 0x03), id_list_get(list_MessageClassMeaning, data[0] & 0x03));
	} else {
		OUTF("%s reserved\n", BitRowFill(data[0], 0xff));
	}

	data++;
}

#define FlipBCD(val)	((val & 0x0f) << 4) | (val >> 4)

static void
SmscTimestamp()
{
	if (data + 6 >= end)
		RETTRUNK();

	OUTF("-------- SMSC Timestamp: %02x-%02x-%02x %02x:%02x:%02x (TZ %02x)\n", FlipBCD(data[0]), FlipBCD(data[1]), FlipBCD(data[2]), FlipBCD(data[3]), FlipBCD(data[4]), FlipBCD(data[5]), FlipBCD(data[6]));

	data += 7;
}


static void
simdatadownload()
{
	unsigned char chl; /* command header length */
	int	i;

	OUTF("-------- Length of CPL: 0x%.4X (%u)\n", (data[0]<<8)+data[1],(data[0]<<8)+data[1]);
	data+=2;
	chl = data[0];
	OUTF("-------- Command Header Length: 0x%.2X (%u)\n",chl,chl);
	data++;
	// SPI
	OUTF("%s SPI : 0x%.2X\n",BitRowFill(data[0], 0xff), data[0]);
	switch (data[0] & 0x3) {
		case 0x00:
			OUTF("------00  No RC, CC or DS\n");
			break;
		case 0x01:
			OUTF("------01  Redudancy Check\n");
			break;
		case 0x02:
			OUTF("------10  Cryptographic Checksum\n");
			break;
		case 0x03:
			OUTF("------11  Digital Signature\n");
			break;
		default: break;
	}
	if ( ((data[0]>>2)& 0x01) )
		OUTF("-----1--  Ciphering\n");
	else
		OUTF("-----0--  No Ciphering\n");
	
	switch ( (data[0]>>3) & 0x03 ) {
		case 0x00:
			OUTF("---00---  No counter available\n");
			break;
		case 0x01:
			OUTF("---01---  Counter available; no replay or sequence checking\n");
			break;
		case 0x02:
			OUTF("---10---  Process if counter is higher\n");
			break;
		case 0x03:
			OUTF("---11---  Process if counter is 1 higher\n");
			break;
		default:break;
	}
	data++;
	OUTF("%s  PoR : 0x%.2X\n", BitRowFill(data[0], 0xff), data[0]); //FIXME:implement me
	switch (data[0] & 0x03) {
		case 0x00:
			OUTF("------00   No PoR-reply to sending entity\n");
			break;
		case 0x01:
			OUTF("------01   PoR required to be send to sending entity\n");
			break;
		case 0x02:
			OUTF("------10   PoR required only on error\n");
			break;
		case 0x03:
			OUTF("--------   reserved\n");
			break;
		default:break;
	}
	switch ( (data[0]>>2) & 0x03) {
		case 0x00:
			OUTF("----00--   No RC/CC/DS applied to PoR\n");
			break;
		case 0x01:
			OUTF("----01--   PoR with simple RC\n");
			break;
		case 0x02:
			OUTF("----10--   PoR with CC\n");
			break;
		case 0x03:
			OUTF("----11--   PoR with DS\n");
			break;
		default:break;
	}
	if ( (data[0]>>4) & 0x01 ) 
			OUTF("---1----   PoR via SMS-SUBMIT\n");
		else
			OUTF("---0----   PoR via SMS-Deliver-Report\n");


	data++;

	// KIc
	OUTF("%s  KIc: 0x%.2X\n",BitRowFill(data[0], 0xff), data[0]);

	switch(data[0] & 0x03) {
		case 0x00:
		OUTF("------00   Algorithm known implicitly\n");
		break;
		case 0x01:
		OUTF("------01   DES\n");
		break;
		case 0x02:
		OUTF("------10   Reserved.\n");
		break;
		case 0x03:
		OUTF("------11   properietary Implementation\n");
		break;
		default:break;
	}
	
	switch ( (data[0]>>2) & 0x03) {
		case 0x00:
		OUTF("----00--   DES-CBC\n");
		break;
		case 0x01:
		OUTF("----01--   3DES-outer-CBC 2 different Keys\n");
		break;
		case 0x02:
		OUTF("----10--   3DES-outer-CBC 3 different Keys\n");
		break;
		case 0x03:
		OUTF("----11--   DES-ECB\n");
		break;
		default:break;
	}
	OUTF("XXX-----   Key Number: %.2X\n", data[0]>>4 );
	data++;
	
	// KId
        OUTF("%s  KId: 0x%.2X\n",BitRowFill(data[0], 0xff), data[0]);

        switch(data[0] & 0x03) {
                case 0x00:
                OUTF("------00   Algorithm known implicitly\n");
                break;
                case 0x01:
                OUTF("------01   DES\n");
                break;
                case 0x02:
                OUTF("------10   Reserved.\n");
                break;
                case 0x03:
                OUTF("------11   properietary Implementation\n");
                break;
                default:break;
        }

        switch ( (data[0]>>2) & 0x03) {
                case 0x00:
                OUTF("----00--   DES-CBC\n");
                break;
                case 0x01:
                OUTF("----01--   3DES-outer-CBC 2 different Keys\n");
                break;
                case 0x02:
                OUTF("----10--   3DES-outer-CBC 3 different Keys\n");
                break;
                case 0x03:
                OUTF("----11--   DES-ECB\n");
                break;
                default:break;
        }
        OUTF("XXX-----   Key Number: %.2X\n", data[0]>>5 );
        data++;

	OUTF("--------  Toolkit Application Reference (TAR): 0x%.2X 0x%.2X 0x%.2X\n", data[0],data[1],data[2]);
	data+=3;
	OUTF("--------  Counter (CNTR): 0x%.2X 0x%.2X 0x%.2X 0x%.2X 0x%.2X\n",data[0], data[1], data[2], data[3], data[4]);
	data+=5;

	// print the remaining bytes of the header, may be redudancy check eg
	for (i=0; i<chl-12;i++) {
		OUTF("%s  %.2X\n",BitRowFill(data[0], 0xff), data[0]);
		data++;
	}

}
	

static void
TypeOfIdentity()
{
	if (data >= end)
		RETTRUNK();

	OUTF("%s Type of Identity: %s\n", BitRowFill(data[0], 0x07), id_list_get(list_TypeOfIdentity, data[0] & 0x07));
	data++;
}

static void
l2_NonCallSS()
{
	if (data >= end)
		RETTRUNK();

	OUTF("%s SendSequenceNumber: %u\n", BitRowFill(data[0], 0xc0), data[0] >> 6);
	if ((data[0] & 0x3f) == 0x3b)
	{
		OUTF("--111011 Facility Register\n");
		data++;
		l2_FacilityRegister();
	} else if ((data[0] & 0x3f) == 0x2a) {
		OUTF("--101010 CCReleaseComplete\n");
		data++;
		l2_CCReleaseComplete();
	} else {
		OUTF("%s UNKNOWN\n", BitRowFill(data[0], 0x3f));
		data++;
	}
}

static void
l2_FacilityRegister()
{
	if (data >= end)
		RETTRUNK();

	if (data[0] != 0x1c)
	{
		OUTF("%s UNKNOWN\n", BitRowFill(data[0], 0xff));
		return;
	}
	OUTF("00011100 Information Element: Facility\n");
	data++;
	l2_Facility();
}

static void
l2_Facility()
{
	if (data >= end)
		RETTRUNK();
	OUTF("%s Length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();
	switch (data[0])
	{
	case 0xa1: /* 10100001 */
		OUTF("10100001  Invoke\n");
		data++;
		l2_FacilityInvoke();
		break;
	case 0xa2: /* 10100010 */
		OUTF("10100010  ReturnResult\n");
		data++;
		l2_FacilityReturnResult();
		break;
	default:
		OUTF("%s  UNKNOWN\n", BitRowFill(data[0], 0xff));
		break;
	}
}

static void
l2_FacilityHeader()
{
	if (data >= end)
		RETTRUNK();
	OUTF("%s  Length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();
	if (data[0] != 0x02)
	{
		OUTF("%s  Type: UNKNOWN\n", BitRowFill(data[0], 0xff));
		return;
	}
	OUTF("00000010  Type: Integer\n");
	if (++data >= end)
		RETTRUNK();
	OUTF("%s  Length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();
	OUTF("%s  Invoke ID: %u\n", BitRowFill(data[0], 0xff), data[0]);
	data++;
}

static void
l2_FacilityInvoke()
{
	l2_FacilityHeader();
	if (data >= end)
		RETTRUNK();
	if (data[0] != 0x02)
	{
		OUTF("%s   Type: UNKNOWN\n", BitRowFill(data[0], 0xff));
		return;
	}
	OUTF("00000010   Type: Integer\n");
	if (++data >= end)
		RETTRUNK();
	OUTF("%s   Length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();

	switch (data[0])
	{
	case 0x3b:
		OUTF("00111011   Unstructured SS Request\n");
		data++;
		l2_UssRequest();
		break;
	case 0x13:
		OUTF("00010011   Unsturcutred SS Data\n");
		data++;
		l2_UssData();
		break;
	default:
		OUTF("%s UNKNOWN\n", BitRowFill(data[0], 0xff));
		break;
	}
}

static void
l2_UssRequest()
{
	if (data >= end)
		RETTRUNK();

	if (data[0] != 0x30)
	{
		OUTF("%s Sequence: UNKNOWN\n", BitRowFill(data[0], 0xff));
		return;
	}
	OUTF("00110000    Sequence: ussd-Arg\n");
	if (++data >= end)
		RETTRUNK();
	OUTF("%s    Length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();
	if (data[0] != 0x04)
	{
		OUTF("%s    Octet String: UNKNOWN\n", BitRowFill(data[0], 0xff));
		return;
	}
	OUTF("00000100    Octet String\n");
	if (++data >= end)
		RETTRUNK();
	OUTF("%s    Length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();

	OUTF("%s    Coding Sheme Number: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();
	if (data[0] != 0x04)
	{
		OUTF("%s     Octet String: UNKNOWN\n", BitRowFill(data[0], 0xff));
		return;
	}
	OUTF("00000100     Octet String\n");
	if (++data >= end)
		RETTRUNK();
	OUTF("%s     Length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();
}

static void
l2_UssData()
{
	char buf[32];
	int len;

	if (data >= end)
		RETTRUNK();
	if (data[0] != 0x16)
	{
		OUTF("%s UNKNWON\n", BitRowFill(data[0], 0xff));
		return;
	}
	if (++data >= end)
		RETTRUNK();
	len = data[0];
	OUTF("%s   Length: %d\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();
	if (data + len > end)
		RETTRUNK();
	snprintf(buf, sizeof buf, "%s", data);
	buf[len] = '\0';
	OUTF("--------   String: %s\n", buf);
	data += len;
}

static void
l2_CCReleaseComplete()
{
	if (data >= end)
		RETTRUNK();
	if (data[0] == 0x08)
	{
		OUTF("00001000 Cause [FIXME]\n");
		data++;
		return;
	} else if (data[0] == 0x1c) { /* 00011100 */
		OUTF("00011100 Facility\n");
		data++;
		l2_Facility();
	} else {
		OUTF("%s UNKNOWN\n", BitRowFill(data[0], 0xff));
		data++;
		return;
	}
}

static void
l2_FacilityReturnResult()
{
	l2_FacilityHeader();
	if (data >= end)
		RETTRUNK();
	if (data[0] != 0x30) /* 00110000 */
		goto err;
	OUTF("00110000 Sequence: Result Info\n");
	if (++data >= end)
		RETTRUNK();
	OUTF("%s  Length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	if (++data >= end)
		RETTRUNK();
	if (data[0] != 2)
		goto err;
	OUTF("00000010   Integer\n");
	if (++data >= end)
		RETTRUNK();
	if (data[0] != 1)
		goto err;
	OUTF("00000001   Length: 1\n");
	if (++data >= end)
		RETTRUNK();
	switch (data[0])
	{
	case 0x13:
		OUTF("00010011   Unsturcutred SS Data\n");
		data++;
		l2_UssData();
		break;
	default:
		goto err;
	}

	return;
err:
	OUTF("%s UNKNOWN\n", BitRowFill(data[0], 0xff));
	data++;
}

static void
MeasurmentReport()
{
	int c, max;

	if (data + 1 >= end)
		RETTRUNK();

	if ((data[0] >> 7) & 0x1)
		OUTF("1------- BA used: yes\n");
	else
		OUTF("0------- BA used: no\n");

	if ((data[0] >> 6) & 0x1)
		OUTF("-1------ Discontinous Transmission: yes\n");
	else
		OUTF("-0------ Discontinous Transmission: no\n");
	OUTF("%s RxLev Full Serving Cell: %d dB\n", BitRowFill(data[0], 0x3f), -110 + (data[0] & 0x3f));
	data++;
	if ((data[0] >> 7) & 0x1)
		OUTF("1------- BA used: yes\n");
	else
		OUTF("0------- BA used: no\n");
	if ((data[0] >> 6) & 0x1)
		OUTF("-1------ MEAS Valid: no\n");
	else
		OUTF("-0------ MEAS Valid: yes\n");
	OUTF("%s RxLev Sub Serving Cell: %d dB\n", BitRowFill(data[0], 0x3f), -110 + (data[0] & 0x3f));
	data++;
	
	OUTF("%s Rx Quality Full Serving Cell: %s\n", BitRowFill(data[0], 0x70), id_list_get(list_RxQual, (data[0] >> 3) & 0x07));
	OUTF("%s Rx Quality Sub Serving Cell: %s\n", BitRowFill(data[0], 0x7), id_list_get(list_RxQual, data[0] & 0x07));
	data++;
	if (data >= end)
		RETTRUNK();
	max = data[0] & 0x7;
	OUTF("%s Number of neighbouring cell measurements: %d\n", BitRowFill(data[0], 0x7), data[0] & 0x7);
	c = 1;
	data++;
	while (c <= max)
	{
		if (data + 2 >= end)
			RETTRUNK();
		OUTF("%s RxLev Neighbour Cell %d: %d dB\n", BitRowFill(data[0], 0x3f), c, -100 + (data[0] & 0x3f));
		data++;
		OUTF("%s Bcch Freq MCell %d: %d\n", BitRowFill(data[0], 0x1f), c, data[0] & 0x1f);
		data++;
		OUTF("%s BTS Identity Code: %d\n", BitRowFill(data[0], 0x3f), data[0] & 0x3f);
		data++;
		c++;
	}
	/* FIXME: I think something iswrong here */
	OUTF("FIXME\n");
}

static void
Number(int len)
{
	OUTF("-------- Number(%u): ", len);
	while (len > 0)
	{
		if ((data[0] >> 4) == 0x0f)
			OUT("%X", data[0] & 0x0f);
		else
			OUT("%X%X", data[0] & 0x0f, data[0] >> 4);
		len -= 2;
		data++;
	}
	OUT("\n");
}

static void
BCDNumber()
{
	int len;

	if (data >= end)
		RETTRUNK();

	len = data[0];
	OUTF("%s Length: %u\n", BitRowFill(data[0], 0xff), data[0]);
	data++;
	OUTF("%s Type of number: %s\n", BitRowFill(data[0], 0x70), id_list_get(list_TypeNumber, (data[0] & 0x70) >> 4));
	data++;
	len--;
	Number(len * 2);
}

