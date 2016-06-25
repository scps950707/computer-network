/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-14 21:25
 * Last Modified:  2016-06-14 21:25
 * Filename:       sack.h
 * Purpose:        hw
 */

#ifndef __SACK_H__
#define __SACK_H__

#include <iostream>
#include <netinet/ip.h>
#include "segment.h"
using namespace std;

void clientSack( int &sockFd, int &curPktSeqNum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr );
void serverSack( int &sockFd, int &curPktSeqNum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktDataAck );

#endif
