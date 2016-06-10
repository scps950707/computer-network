/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:09
 * Last Modified:  2016-06-11 00:18
 * Filename:       client.cpp
 * Purpose:        homework
 */

#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
#include "client.h"
#include "segment.h"
#include "tool.h"
using namespace std;

int main( int argc, char *argv[] )
{
    if ( argc != 3 )
    {
        cout << "./Prog [server IP] [PORT]";
    }
    srand( time( NULL ) );
    struct sockaddr_in clientAddr;
    struct sockaddr_in serverAddr;
    int sockFd;
    int port = atoi( argv[2] );
    int currentSeqnum = rand() % 10000 + 1;
    Packet pktSnd;
    Packet pktRcv;
    socklen_t serSize = sizeof( serverAddr );

    bzero( &serverAddr, sizeof( serverAddr ) );
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    clientAddr.sin_port = htons( CLIENT_PORT );

    bzero( &serverAddr, sizeof( serverAddr ) );
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons( port );
    if ( inet_pton( AF_INET, argv[1], &serverAddr.sin_addr ) <= 0 )
    {
        cout << "IP error " << endl;
        exit( EXIT_FAILURE );
    }

    if ( ( sockFd = socket( clientAddr.sin_family, SOCK_DGRAM, 0 ) ) < 0 )
    {
        perror ( "socket" );
        exit( EXIT_FAILURE );
    }

    if ( bind( sockFd, ( struct sockaddr * )&clientAddr, sizeof( clientAddr ) ) < 0 )
    {
        perror( "bind" );
        exit( EXIT_FAILURE );
    }

    cout << "=====Start the three-way handshake=====" << endl;

    pktSnd.SYN = true;
    pktSnd.destPort = SERVER_PORT;
    pktSnd.sourcePort = CLIENT_PORT;
    pktSnd.seqNum = currentSeqnum;
    pktSnd.ackNum = 0;
    sendto( sockFd, &pktSnd, MSS, 0, ( struct sockaddr * )&serverAddr, serSize );
    cout << "Send a packet(SYN) to " << argv[1] << " : " << argv[2] << endl;

    while ( recvfrom( sockFd , &pktRcv, MSS, 0, ( struct sockaddr * )&serverAddr, &serSize ) )
    {
        if ( pktRcv.SYN == true && pktRcv.ACK == true )
        {
            string ip = getIpStr( &serverAddr.sin_addr );
            cout << "Receive a packet(SYN/ACK) from " << ip  << " : " << pktRcv.sourcePort << endl;
            cout << "    Receive a packet (seq_num = " << pktRcv.seqNum << ", ack_num = " << pktRcv.ackNum << ")" << endl;
            Packet ack;
            ack.ACK = true;
            ack.destPort = SERVER_PORT;
            ack.sourcePort = CLIENT_PORT;
            ack.seqNum = ++currentSeqnum;
            ack.ackNum = pktRcv.seqNum + 1;
            cout << "Send a Packet(ACK) to " << ip << " : " << SERVER_PORT << endl;
            sendto( sockFd, &ack, MSS, 0, ( struct sockaddr * )&serverAddr, serSize );
            cout << "=====Complete the three-way handshake=====" << endl;
            break;
        }
    }

    return EXIT_SUCCESS;
}
