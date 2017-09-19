#include "stdafx.h"
#include <stdio.h>
#include <pcap.h>

#define LINE_LEN 16
/*test */
/* 4 bytes IP address */

/* Pcap definiton */
/* prototype of the packet handler */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);
int count = 0;
int main(int argc, char **argv)
{
	argv[1] = "test123.pcap";
	argc = 2;

	pcap_if_t *alldevs;
	pcap_if_t *d;
	int inum;
	int i = 0;
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_dumper_t *dumpfile;
	/* Parameter for ex */
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	time_t local_tv_sec;
	int res;
	struct tm *ltime;
	char timestr[16];

	/* Check command line */
	if (argc != 2)
	{
		printf("usage: %s filename", argv[0]);
		return -1;
	}

	/* Retrieve the device list on the local machine */
	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}

	/* Print the list */
	for (d = alldevs; d; d = d->next)
	{
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}

	if (i == 0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
		return -1;
	}

	printf("Enter the interface number (1-%d):", i);
	scanf_s("%d", &inum);

	if (inum < 1 || inum > i)
	{
		printf("\nInterface number out of range.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/* Jump to the selected adapter */
	for (d = alldevs, i = 0; i< inum - 1; d = d->next, i++);


	/* Open the adapter */
	if ((adhandle = pcap_open_live(d->name,	// name of the device
		65536,			// portion of the packet to capture. 
						// 65536 grants that the whole packet will be captured on all the MACs.
		1,				// promiscuous mode (nonzero means promiscuous)
		1000,			// read timeout
		errbuf			// error buffer
	)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/* Open the dump file */
	dumpfile = pcap_dump_open(adhandle, argv[1]);

	if (dumpfile == NULL)
	{
		fprintf(stderr, "\nError opening output file\n");
		return -1;
	}

	printf("\nlistening on %s... Press Ctrl+C to stop...\n", d->description);

	/* At this point, we no longer need the device list. Free it */
	pcap_freealldevs(alldevs);

	/* start the capture */
	// pcap_loop(adhandle, 0, packet_handler, (unsigned char *)dumpfile);
	/* Retrieve the packets */
	while ((res = pcap_next_ex(adhandle, &header, &pkt_data)) >= 0) {

		if (res == 0)
			/* Timeout elapsed */
			continue;

		/* convert the timestamp to readable format */
		//local_tv_sec = header->ts.tv_sec;
		//ltime = localtime(&local_tv_sec);
		//strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);
		//printf("%s,%.6d len:%d\n", timestr, header->ts.tv_usec, header->len);
		pcap_dump((unsigned char *)dumpfile, header, pkt_data);
	}

	if (res == -1) {
		printf("Error reading the packets: %s\n", pcap_geterr(adhandle));
		return -1;
	}


	pcap_close(adhandle);
	return 0;
}











/* Pcap method. Not used for this program */
/* Callback function invoked by libpcap for every incoming packet */
void packet_handler(u_char *dumpfile, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	count++;
	if (count == 10) {
		//GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
		system("pause");
		// pcap_breakloop(adhandle);
	}

	printf("%d, len:%d\n", count, header->len);
	/* save the packet on the dump file */
	pcap_dump(dumpfile, header, pkt_data);
}
