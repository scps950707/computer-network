/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-11 00:06
 * Last Modified:  2016-06-11 03:19
 * Filename:       tool.cpp
 * Purpose:        homework
 */

#include <iostream>
#include <stdlib.h>
using namespace std;

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

void randFile( char *src, int size )
{
#ifdef __USESRAND__
    srand( time( NULL ) );
#endif
    string byteList = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for ( int i = 0; i < size; i++ )
    {
        src[i] = byteList[rand() % byteList.length()];
    }
}
