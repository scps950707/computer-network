/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-11 16:18
 * Last Modified:  2016-06-11 17:32
 * Filename:       slow.h
 * Purpose:        HW
 */

#ifndef __SLOW_H__
#define __SLOW_H__

#include <netinet/ip.h>
#include <iostream>
#include "segment.h"
using namespace std;

void clientSlowStart( int &sockFd, int &curPktSeqNum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr );
void serverSlowStart( int &sockFd, int &curPktSeqNum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktDataAck );

#endif
