/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-11 15:51
 * Last Modified:  2016-06-11 16:24
 * Filename:       shake.h
 * Purpose:        homework
 */


#ifndef __SHAKE_H__
#define __SHAKE_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <iostream>
#include <stdint.h>
#include "tool.h"
#include "segment.h"
using namespace std;

void ServerThreeWayHandShake( int &sockFd, int &currentSeqnum, string &clientIP, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktThreeShakeRcv );
void ServerFourWayHandShake( int &sockFd, int &currentSeqnum, int &curRcvSeqnum, string &clientIP, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktFourShake );
void ClientThreeWayHandShake(int &sockFd, int &currentSeqnum,string &serverIP,uint16_t &serverPort,sockaddr_in &serverAddr);
void ClientFourWayHandShake(int &sockFd, int &currentSeqnum,string &serverIP,uint16_t &serverPort,sockaddr_in &serverAddr);

#endif
