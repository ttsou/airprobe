#ifndef _PCAP_IF_H
#define _PCAP_IF_H

extern int open_pcap_file(char *fname);
int write_pcap_packet(int fd, int arfcn, int ts, int fn, char *data, unsigned int len);

#endif
