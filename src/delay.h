/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-11 23:07
 * Last Modified:  2016-06-11 23:15
 * Filename:       delay.h
 * Purpose:        hw
 */

#ifndef __DELAY_H__
#define __DELAY_H__

#include <netinet/ip.h>
#include <iostream>
#include "segment.h"
using namespace std;

void clientDelayAck( int &sockFd, int &curPktSeqNum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr );
void serverDelayAck( int &sockFd, int &curPktSeqNum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktDataAck );

#endif
