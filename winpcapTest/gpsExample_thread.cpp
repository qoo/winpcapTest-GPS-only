/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact
Adept MobileRobots for information about a commercial version of ARIA at
robots@mobilerobots.com or
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/

/** @example gpsExample.cpp Example program to connect to a GPS and read and
* display data on the terminal.
*  Also tries to connect to a TCM compass through the computer serial port, and
*  use that to set the ArGPS compass data.
*/
/* Multithreading definition */
#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <thread>

/* GPS definition */
#include "Aria.h"
#include "ArGPS.h"
#include "ArGPSConnector.h"
#include "ArTrimbleGPS.h"
#include "ArTCMCompassDirect.h"

/* Write data, read system time */
#include <iostream>
#include <fstream>
#include <string> 
#include <chrono>

/* Multitheading */
#include <thread>

using namespace std::chrono;
using namespace std;


/* prototype of the GPS */
int GPS(int argc, char** argv);

/* Pcap definition */
#ifdef _MSC_VER
/*
* we do not want the warnings about the old deprecated and unsecure CRT functions
* since these examples can be compiled under *nix as well
*/
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <pcap.h>
/* prototype of the packet handler */
int Lidar(int argc, char** argv);


std::ofstream lidarInit;
/* Global variable, switch */
bool switch_gps = true;
void getTime(char* buf);

int main(int argc, char** argv)
{
	// predefine 
	//argv[1] = "Lidar_test.pcap";
	//argv[2] = "Gps_test.csv";
	argv[3] = "-gpsBaud";
	argv[4] = "38400";
	argv[5] = "-gpsPort";
	argv[6] = "COM1";
	argc = 7;
	//ArLog::log(ArLog::Terse, "Activate GPS device");

	/* Use timestamp for file names*/
	//string input = "";
	//while (true) {
	//	cout << "Please enter a port (ex: COM1): ";
	//	getline(cin, input);
	//	if (input.substr(0, 3) == "COM" && isdigit(input[3])  ) {
	//		break;
	//	}
	//	cout << "Invalid input, please try again" << endl;
	//}
	//
	//char *cinput = new char[input.length() + 1];
	//strcpy(cinput, input.c_str());
	//argv[6] = cinput;
	//cout << "Initiate gpsPort: " << argv[6] << endl;
	/* Use timestamp for file names*/
	char buf[26];
	getTime(buf);
	buf[24] = '_';
	buf[25] = '\0';
	int i = 0;
	while (buf[i] != '\0')
	{
		if (buf[i] == ':' || buf[i] == ' ')
		{
			buf[i] = '_';
		}		
		i = i + 1;
	}
	string str(buf);
	string str2(str);
	string str3(str);
	str += "Lidar_test.pcap";
	str2 += "GPS_test.csv";
	char *cstr = new char[str.length() + 1];
	strcpy(cstr, str.c_str());
	argv[1] = cstr;
	char *cstr2 = new char[str2.length() + 1];
	strcpy(cstr2, str2.c_str());
	argv[2] = cstr2;
	str3 += "Lidar_init.csv";
	char *cstr3 = new char[str3.length() + 1];
	strcpy(cstr3, str3.c_str());

	//std::ofstream lidarInit;
	lidarInit.open(cstr3);
	//lidarInit.close();
	// system("pause");

	/* mulithreads for GPS and Lidar*/
	// GPS(argc, argv);
	printf("Activate GPS device\n");
	thread t1(&GPS, argc, argv);
	// system("pause");
	printf("Activate Lidar device\n");
	// Lidar(argc, argv);
	thread t2(&Lidar, argc, argv);
	
	t1.join();
	t2.join();
	return 0;
}
void getTime(char* buf)
{
	struct tm newtime;
	__int64 ltime;
	//char buf[26];
	errno_t err;
	milliseconds ms = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch()
		);
	_time64(&ltime);
	// Obtain coordinated universal time:   
	err = _gmtime64_s(&newtime, &ltime);
	err = asctime_s(buf, 26, &newtime);
}
int Lidar(int argc, char** argv)
{
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
	//if (argc != 2)
	//{
	//	printf("usage: %s filename", argv[0]);
	//	return -1;
	//}

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
	//scanf("%d", &inum);
	inum = 1;

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
	long cntLidar = 0;
	while ((res = pcap_next_ex(adhandle, &header, &pkt_data)) >= 0) {

		if (res == 0)
			/* Timeout elapsed */
			continue;

		/* convert the timestamp to readable format */
		if (cntLidar < 10 || cntLidar % 100 == 0) {
			local_tv_sec = header->ts.tv_sec;
			ltime = localtime(&local_tv_sec);
			strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);
			printf("%s,%.6d len:%d\n", timestr, header->ts.tv_usec, header->len);
			if (cntLidar == 0) {
				/*system time*/
				milliseconds ms = duration_cast< milliseconds >(
					system_clock::now().time_since_epoch()
					);
				lidarInit << ms.count() << ",";
				lidarInit.close();
			}
		}
		cntLidar++;
		pcap_dump((unsigned char *)dumpfile, header, pkt_data);
	}

	if (res == -1) {
		printf("Error reading the packets: %s\n", pcap_geterr(adhandle));
		return -1;
	}


	pcap_close(adhandle);
	return 0;
}


int GPS(int argc, char** argv) 
{
	ArLog::log(ArLog::Normal, "Program start....");
	Aria::init();
	ArArgumentParser parser(&argc, argv);
	parser.loadDefaultArguments();
	ArRobot robot;
	ArRobotConnector robotConnector(&parser, &robot);
	ArGPSConnector gpsConnector(&parser);
	std::ofstream myfile;
	//myfile.open(argv[2]);
	// Connect to the robot, get some initial data from it such as type and name,
	// and then load parameter files for this robot.
	//if(!robotconnector.connectrobot())
	//{
	//  arlog::log(arlog::terse, "gpsexample: warning: could not connect to robot.  will not be able to switch gps power on, or load gps options from this robot's parameter file.");
	//}

	if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
	{
		Aria::logOptions();
		ArLog::log(ArLog::Terse, "gpsExample options:\n  -printTable   Print data to standard output in regular columns rather than a refreshing terminal display, and print more digits of precision");
		Aria::exit(1);
	}

	//ArLog::log(ArLog::Normal, "gpsExample: Connected to robot.");

	//robot.runAsync(true);

	//// check command line arguments for -printTable
	bool printTable = parser.checkArgument("printTable");

	// On the Seekur, power to the GPS receiver is switched on by this command.
	// (A third argument of 0 would turn it off). On other robots this command is
	// ignored.
	robot.com2Bytes(116, 6, 1);
	// Try connecting to a GPS. We pass the robot pointetr to the connector so it
	// can check the robot parameters for this robot type for default values for
	// GPS device connection information (receiver type, serial port, etc.)
	ArLog::log(ArLog::Normal, "gpsExample: Connecting to GPS, it may take a few seconds...");
	ArGPS *gps = gpsConnector.createGPS(&robot);
	
	ArLog::log(ArLog::Terse, "save data as example.csv");
	myfile.open(argv[2]);
	myfile.precision(15);
	gps->printDataLabelsHeader();
	//gps->printData();
	//(gps->printData());
	//gps->haveLatitude();
	//gps->writeDataLabelsHeader();
	//printf("Pos:% 2.6f % 2.6f", gps->getLatitude(), gps->getLongitude());;
	myfile << "Latitude,Longitude,GPS time(hhmmss), System time(ms),Altitude (m),Speed (m/s),NumSatellites,AvgSNR (dB)\n";


	if (!gps || !gps->connect())
	{
		ArLog::log(ArLog::Terse, "gpsExample: Error connecting to GPS device.  Try -gpsType, -gpsPort, and/or -gpsBaud command-line arguments. Use -help for help.");
		system("pause");
		return -1;
	}

	if (gpsConnector.getGPSType() == ArGPSConnector::Simulator)
	{
		ArLog::log(ArLog::Normal, "gpsExample: GPS data is from simulator.");
		/*
		If connected to MobileSim, and aa map is loaded into MobileSim that contains an OriginLatLonAlt line,
		then MobileSim will provides simulated GPS data based on the robot's
		true position in the simulator.  But you can also manually set "dummy"
		positions like this instead, or to simulate GPS without connecting
		to MobileSim:
		*/
		//ArLog::log(ArLog::Normal, "gpsExample: GPS is a simulator. Setting dummy position.");
		//(dynamic_cast<ArSimulatedGPS*>(gps))->setDummyPosition(42.80709, -71.579047, 100);
	}



	ArLog::log(ArLog::Normal, "gpsExample: Reading data...");
	ArTime lastReadTime;
	if (printTable)
	{
		ArLog::log(ArLog::Normal, "printTable is true...");
		//gps->printDataLabelsHeader();
	}

	while (true)
	{
		int r = gps->read();
		if (r & ArGPS::ReadError)
		{
			ArLog::log(ArLog::Terse, "gpsExample: Warning: error reading GPS data.");
			ArUtil::sleep(1000);
			continue;
		}


		if (r & ArGPS::ReadUpdated)
		{
			if (printTable)
			{
				gps->printData(false);
				printf("\n");
			}
			else
			{
				gps->printData();
				printf("\r");
				/* Save data to csv via following order.
				"Latitude,Longitude,System time,Altitude (m),Speed (m/s),NumSatellites,AvgSNR (dB)\n";*/
				if (!gps->havePosition())
				{
					myfile << ",";
					myfile << ",";
				}
				else
				{
					myfile << gps->getLatitude() << ",";
					myfile << gps->getLongitude() << ",";
				}
				/* time */
				myfile << gps->getGPSPositionTimestamp().getSec() << ",";
				/*system time*/
				milliseconds ms = duration_cast< milliseconds >(
					system_clock::now().time_since_epoch()
					);
				myfile << ms.count() << ",";

				if (!gps->haveAltitude())
				{
					myfile << ",";
				}
				else
				{
					myfile << gps->getAltitude() << ",";
				}
				if (gps->haveSpeed())
				{
					myfile << gps->getSpeed() << ",";
				}
				else
				{
					myfile << ",";
				}


				myfile << gps->getNumSatellitesTracked() << ",";

				if (gps->haveSNR())
				{
					myfile << gps->getMeanSNR() << ",";

				}
				else
				{
					myfile << ",";

				}
				myfile << "\n";
			}
			ArUtil::sleep(500);
			lastReadTime.setToNow();
			continue;
		}
		else {
			if (lastReadTime.secSince() >= 5) {
				ArLog::log(ArLog::Terse, "gpsExample: Warning: haven't recieved any data from GPS for more than 5 seconds!");
			}
			ArUtil::sleep(1000);
			continue;
		}

	}
	myfile.close();
	return 0;
};
