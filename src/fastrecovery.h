/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-14 02:47
 * Last Modified:  2016-06-14 02:47
 * Filename:       fastrecovery.h
 * Purpose:        hw
 */

#ifndef __FASTRECOVERY_H__
#define __FASTRECOVERY_H__

#include <iostream>
#include <netinet/ip.h>
#include "segment.h"
using namespace std;

void clientFastRecovery( int &sockFd, int &currentSeqnum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr );
void serverFastRecovery( int &sockFd, int &currentSeqnum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktTransAck );

#endif
