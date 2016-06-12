/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-11 00:06
 * Last Modified:  2016-06-11 03:19
 * Filename:       tool.cpp
 * Purpose:        homework
 */

#include <iostream>
#include <arpa/inet.h>
using namespace std;

string getIpStr( const void *src )
{
    char ip[INET_ADDRSTRLEN];
    inet_ntop( AF_INET, src, ip, INET_ADDRSTRLEN );
    return string( ip );
}

void sendPktMsg( string type, string ip, int port )
{
    cout << "Send a packet(" << type << ") to " << ip << " : " << port << endl;
}

void rcvPktMsg( string type, string ip, int port )
{
    cout << "Receive a packet(" << type << ") from " <<  ip << " : " << port << endl;
}

void rcvPktNumMsg( int seqNum, int ackNum )
{
    cout << "\tReceive a packet (seq_num = " << seqNum << ", ack_num = " << ackNum << ")" << endl;
}
