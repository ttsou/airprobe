#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <pcap.h>

#include "gsmstack.h"
#include "gsmtap.h"

/* source_pcap_burst.c: read a PCAP file with GSMTAP_UM_BURST packets
 * and feed them into the stack.
 *
 * (C) 2008 by Harald Welte <laforge@gnumonks.org>
 *
 * TODO:
 *	fix PCAP endianness
 */ 


static struct gsm_rf_chan *rfchans[NR_ARFCN];

static int read_pcap_hdr(int fd)
{
	int rc;
	struct pcap_file_header pfh;

	rc = read(fd, &pfh, sizeof(pfh));
	if (rc < sizeof(pfh))
		return -EIO;

	if (pfh.magic != TCPDUMP_MAGIC)
		return -EINVAL;

	if (pfh.linktype != LINKTYPE_GSMTAP)
		return -EINVAL;

	return 0;
}

static int send_burst(struct gsmtap_hdr *gh, int burst_len,
		      struct timeval *tv)
{
	unsigned char *data = (unsigned char *)gh + sizeof(*gh);
	struct gsm_burst burst;
	struct gsm_rf_chan *rfchan;
	struct gsm_phys_chan *pchan;
	unsigned int arfcn;

	arfcn = ntohs(gh->arfcn);

	/* Create new RF channel structure if we've not seen this ARFCN
	 * before */
	if (!rfchans[arfcn]) {
		rfchans[arfcn] = gsm_init_rfchan(arfcn);
		if (!rfchans[arfcn]) {
			fprintf(stderr, "cannot init rfchan ARFCN=%u\n",
				arfcn);
			return -EINVAL;
		}
	}

	rfchan = rfchans[arfcn];
	pchan = &rfchan->phys_chan[gh->timeslot];

	memset(&burst, 0, sizeof(burst));
	burst.phys_chan = pchan;
	burst.burst_type = gh->burst_type;
	burst.rx_frame_nr = ntohl(gh->frame_number);
	memcpy(&burst.rx_time, tv, sizeof(burst.rx_time));
	memcpy(burst.decoded, data, burst_len);

	return gsm_rx_burst(&burst, 0);
}

/* fills 'buf' with gsmtap_hdr and payload, returns length of payload */
static int read_one_pkt(int fd, unsigned char *buf, int bufsize,
			struct timeval *tv)
{
	struct pcap_sf_pkthdr pkthdr;
	struct gsmtap_hdr *gh = (struct gsmtap_hdr *) buf;
	int len, burstlen;

	len = read(fd, &pkthdr, sizeof(pkthdr));
	if (len < sizeof(pkthdr))
		return -1;

	/* FIXME: ntohl on caplen and len? */

	if (pkthdr.caplen < sizeof(*gh))
		return -2;

	if (pkthdr.len > pkthdr.caplen)
		return -3;

	if (bufsize < pkthdr.len)
		return -4;

	if (tv) {
		tv->tv_sec = pkthdr.ts.tv_sec;
		tv->tv_usec = pkthdr.ts.tv_usec;
	}

	len = read(fd, buf, pkthdr.caplen);
	if (len < pkthdr.caplen)
		return -5;

	if (gh->version != GSMTAP_VERSION ||
	    gh->type != GSMTAP_TYPE_UM_BURST)
		return -6;

	return pkthdr.caplen - sizeof(*gh);
}

int main(int argc, char **argv)
{
	char *fname;
	int fd, rc;

	if (argc != 2) {
		fprintf(stderr, "please specify pcap filename\n");
		exit(2);
	}

	fname = argv[1];

	fd = open(fname, O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(1);
	}

	rc = read_pcap_hdr(fd);
	if (rc < 0) {
		perror("pcap_hdr");
		exit(1);
	}

	while (1) {
		unsigned char buf[1024];
		struct timeval tv;
		int burst_len;
		burst_len = read_one_pkt(fd, buf, sizeof(buf), &tv);
		if (burst_len < 0) {
			fprintf(stderr, "read_one_pkt() = %d\n", burst_len);
			exit(3);
		}
		rc = send_burst((struct gsmtap_hdr *)buf, burst_len, &tv);
	}
}
