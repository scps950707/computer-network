/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:09
 * Last Modified:  2016-06-11 03:12
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
    int serverPort = atoi( argv[2] );
    string serverIP(argv[1]);
#ifdef __SEQSTATIC__
    int currentSeqnum = 9230;
#else
    int currentSeqnum = rand() % 10000 + 1;
#endif
    socklen_t serSize = sizeof( serverAddr );

    bzero( &serverAddr, sizeof( serverAddr ) );
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    clientAddr.sin_port = htons( CLIENT_PORT );

    bzero( &serverAddr, sizeof( serverAddr ) );
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons( serverPort );
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

    Packet pktSnd( CLIENT_PORT, serverPort, currentSeqnum, 0 );
    pktSnd.SYN = true;
    sendPktMsg("SYN",serverIP,serverPort);
    sendto( sockFd, &pktSnd, MSS, 0, ( struct sockaddr * )&serverAddr, serSize );

    Packet pktRcv;
    while ( recvfrom( sockFd , &pktRcv, MSS, 0, ( struct sockaddr * )&serverAddr, &serSize ) )
    {
        if ( pktRcv.SYN == true && pktRcv.ACK == true )
        {
            rcvPktMsg("SYN/ACK",serverIP,pktRcv.sourcePort);
            cout << "    Receive a packet (seq_num = " << pktRcv.seqNum << ", ack_num = " << pktRcv.ackNum << ")" << endl;
            Packet ack( CLIENT_PORT, serverPort, ++currentSeqnum, pktRcv.seqNum + 1 );
            ack.ACK = true;
            sendPktMsg("ACK",serverIP,serverPort);
            sendto( sockFd, &ack, MSS, 0, ( struct sockaddr * )&serverAddr, serSize );
            break;
        }
    }

    cout << "=====Complete the three-way handshake=====" << endl;

    return EXIT_SUCCESS;
}
