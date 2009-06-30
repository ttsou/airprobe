/* PCAP support for gsm-tvoid
 * (C) 2008 by Harald Welte <laforge@gnumonks.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pcap.h>
#include <errno.h>
#include <time.h>

#include "out_pcap.h"
#include "gsmtap.h"

#ifndef LINKTYPE_GSMTAP
#define LINKTYPE_GSMTAP		2342
#endif

#define TCPDUMP_MAGIC   0xa1b2c3d4

struct pcap_timeval {
	int32_t tv_sec;
	int32_t tv_usec;
};
	
struct pcap_sf_pkthdr {
	struct pcap_timeval ts;		/* time stamp */
	u_int32_t caplen;		/* lenght of portion present */
	u_int32_t len;			/* length of this packet */
};

static int write_pcap_file_header(int fd)
{
	struct pcap_file_header pfh;

	pfh.magic = TCPDUMP_MAGIC;
	pfh.version_major = PCAP_VERSION_MAJOR;
	pfh.version_minor = PCAP_VERSION_MINOR;
	pfh.thiszone = timezone;
	pfh.sigfigs = 0;
	pfh.snaplen = 1024; /* FIXME */ 
	pfh.linktype = LINKTYPE_GSMTAP;

	if (write(fd, &pfh, sizeof(pfh)) < sizeof(pfh))
		return -1;

	return 0;
}

/* open pcap file and write header */
int open_pcap_file(char *fname)
{
	int fd;
	int rc;

 	fd = open(fname, O_WRONLY|O_CREAT|O_TRUNC, 0660);
	if (fd < 0)
		return fd;

	rc = write_pcap_file_header(fd);
	if (rc < 0) {
		close(fd);
		fd = -EIO;
	}

	return fd;
}

int write_pcap_packet(int fd, int arfcn, int ts, int fn,
		      int burst, int burst_type,
		      const unsigned char *data, unsigned int len)
{
	unsigned char buf[8192];
	struct pcap_sf_pkthdr *ph;
	struct gsmtap_hdr *gh;
	struct timeval tv;
	int rc;

	if (fd < 0)
		return -EINVAL;

	ph = (struct pcap_sf_pkthdr *) &buf[0];
	gh = (struct gsmtap_hdr *) &buf[sizeof(struct pcap_sf_pkthdr)];

	gettimeofday(&tv, NULL);

	ph->ts.tv_sec = tv.tv_sec;
	ph->ts.tv_usec = tv.tv_usec;
	ph->caplen = ph->len = len + sizeof(struct gsmtap_hdr);

	gh->version = GSMTAP_VERSION;
	gh->hdr_len = sizeof(struct gsmtap_hdr)>>2;
	if (burst)
		gh->type = GSMTAP_TYPE_UM_BURST;
	else
		gh->type = GSMTAP_TYPE_UM;
	gh->timeslot = ts;
	gh->arfcn = htons(arfcn);
	/* we don't support signal/noise yet */
	gh->noise_db = gh->signal_db = 0;
	gh->frame_number = htonl(fn);
	gh->burst_type = burst_type & 0xff;

	memcpy(buf + sizeof(*ph) + sizeof(*gh), data, len);
	
	rc = write(fd, buf, sizeof(*ph) + sizeof(*gh) + len);

	//fsync(fd);

	return rc;
}
