#include "stdafx.h"
#include <stdio.h>
#include <pcap.h>

#define LINE_LEN 16
/*test */
/* 4 bytes IP address */
typedef struct ip_address {
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
}ip_address;

/* IPv4 header */
typedef struct ip_header {
	u_char  ver_ihl;        // Version (4 bits) + Internet header length (4 bits)
	u_char  tos;            // Type of service 
	u_short tlen;           // Total length 
	u_short identification; // Identification
	u_short flags_fo;       // Flags (3 bits) + Fragment offset (13 bits)
	u_char  ttl;            // Time to live
	u_char  proto;          // Protocol
	u_short crc;            // Header checksum
	ip_address  saddr;      // Source address
	ip_address  daddr;      // Destination address
	u_int   op_pad;         // Option + Padding
}ip_header;

/* UDP header*/
typedef struct udp_header {
	u_short sport;          // Source port
	u_short dport;          // Destination port
	u_short len;            // Datagram length
	u_short crc;            // Checksum
}udp_header;

int main(int argc, char **argv)
{
	pcap_t *fp;
	char errbuf[PCAP_ERRBUF_SIZE];
	char source[PCAP_BUF_SIZE];
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	u_int i = 0;
	int res;

	/*add test ip*/
	ip_header *ih;
	udp_header *uh;
	u_int ip_len;
	u_short sport, dport;

	if (argc != 2)
	{
		printf("usage: %s filename", argv[0]);
		return -1;
	}

	/* Create the source string according to the new WinPcap syntax */
	if (pcap_createsrcstr(source,         // variable that will keep the source string
		PCAP_SRC_FILE,  // we want to open a file
		NULL,           // remote host
		NULL,           // port on the remote host
		argv[1],        // name of the file we want to open
		errbuf          // error buffer
	) != 0)
	{
		fprintf(stderr, "\nError creating a source string\n");
		return -1;
	}

	/* Open the capture file */
	if ((fp = pcap_open(source,         // name of the device
		65536,          // portion of the packet to capture
						// 65536 guarantees that the whole packet will be captured on all the link layers
		PCAP_OPENFLAG_PROMISCUOUS,     // promiscuous mode
		1000,              // read timeout
		NULL,              // authentication on the remote machine
		errbuf         // error buffer
	)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the file %s.\n", source);
		return -1;
	}

	/* Retrieve the packets from the file */
	int cnt = 0;

	while (cnt < 1 && (res = pcap_next_ex(fp, &header, &pkt_data)) >= 0)
	{
		/* print pkt timestamp and pkt len */
		printf("%ld:%ld (%ld)\n", header->ts.tv_sec, header->ts.tv_usec, header->len);

		/* Port, ip.  port:sport,dport*/
		/* retireve the position of the ip header */
		ih = (ip_header *)(pkt_data +
			14); //length of ethernet header

				 /* retireve the position of the udp header */
		ip_len = (ih->ver_ihl & 0xf) * 4;
		uh = (udp_header *)((u_char*)ih + ip_len);

		/* convert from network byte order to host byte order */
		sport = ntohs(uh->sport);
		dport = ntohs(uh->dport);
		printf("%d.%d.%d.%d.%d -> %d.%d.%d.%d.%d\n",
			ih->saddr.byte1,
			ih->saddr.byte2,
			ih->saddr.byte3,
			ih->saddr.byte4,
			sport,
			ih->daddr.byte1,
			ih->daddr.byte2,
			ih->daddr.byte3,
			ih->daddr.byte4,
			dport);
		if (sport == 2368) {
			printf("LiDAR!\n");
		}
		/* Print the packet */
		for (i = 1; (i < header->caplen + 1); i++)
		{
			printf("%.2x ", pkt_data[i - 1]);
			if ((i % LINE_LEN) == 0) printf("\n");
		}
		/* Start: data */

		/* End*/
		printf("\n\n");
		cnt++;
	}
	system("pause"); // <----------------------------------
	pkt_data[42];
	if (res == -1)
	{
		printf("Error reading the packets: %s\n", pcap_geterr(fp));
	}

	return 0;
}
