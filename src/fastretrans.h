/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-12 22:39
 * Last Modified:  2016-06-12 22:39
 * Filename:       fastretrans.h
 * Purpose:        hw
 */

#ifndef __FASTRETRANS_H__
#define __FASTRETRANS_H__

#include <iostream>
#include <netinet/ip.h>
#include "segment.h"
using namespace std;

void clientFastReTrans( int &sockFd, int &currentSeqnum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr );
void serverFastReTrans( int &sockFd, int &currentSeqnum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktTransAck );

#endif
