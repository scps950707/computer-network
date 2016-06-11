/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-12 02:48
 * Last Modified:  2016-06-12 02:48
 * Filename:       congenavoid.h
 * Purpose:        HW
 */

#ifndef __CONGENAVOID_H__
#define __CONGENAVOID_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <iostream>
#include "segment.h"
using namespace std;

void clientConAvoid( int &sockFd, int &currentSeqnum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr );
void serverConAvoid( int &sockFd, int &currentSeqnum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktTransAck );
enum {SLOWSTART,CONAVOID};

#endif
