/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-11 00:00
 * Last Modified:  2016-06-11 03:19
 * Filename:       tools.h
 * Purpose:        homework
 */

using namespace std;
#ifndef __TOOLS_H__
#define __TOOLS_H__

#define CHECKVAR(x) cout << "["<< __FILE__ << ":" << __LINE__ << "]\t[" << #x " = " << x << "]"
#include<iostream>
void sendPktMsg( string type, string ip, int port );
void rcvPktMsg( string type, string ip, int port );
void rcvPktNumMsg( int pktSeqNum, int pktAckNum );
void randFile( char *src, int size );

#endif
