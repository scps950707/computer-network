/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:09
 * Last Modified:  2016-06-11 17:00
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
#include <stdint.h>
#include "client.h"
#include "segment.h"
#include "tool.h"
#include "para.h"
#include "shake.h"
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
#ifdef __SEQSTATIC__
    int currentSeqnum = 9230;
#else
    int currentSeqnum = rand() % 10000 + 1;
#endif
    socklen_t serSize = sizeof( serverAddr );
    string serverIP( argv[1] );
    uint16_t serverPort = atoi( argv[2] );

    bzero( &clientAddr, sizeof( clientAddr ) );
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

    ClientThreeWayHandShake( sockFd, currentSeqnum, serverIP, serverPort, serverAddr );

    cout << "=====Complete the three-way handshake=====" << endl;

    cout << "Receive a file from " << serverIP << ":" << serverPort << endl;

    Packet pktTransRcv;
    int rwnd = 10240;
    while ( recvfrom( sockFd , &pktTransRcv, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, &serSize ) )
    {
        rcvPktNumMsg( pktTransRcv.tranSeqNum, pktTransRcv.ackNum );
        rwnd -= pktTransRcv.tranSeqNum;
        Packet dataAck( CLIENT_PORT, serverPort, ++currentSeqnum, pktTransRcv.seqNum + 1 );
        dataAck.tranAckNum = pktTransRcv.tranSeqNum < 512 ? pktTransRcv.tranSeqNum * 2 : pktTransRcv.tranSeqNum ;
        dataAck.rcvWin = rwnd;
        sendto( sockFd, &dataAck, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
        if ( rwnd <= 0 )
        {
            break;
        }
    }


    cout << "=====Start the four-way handshake=====" << endl;

    ClientFourWayHandShake( sockFd, currentSeqnum, serverIP, serverPort, serverAddr );

    cout << "=====Complete the four-way handshake=====" << endl;

    return EXIT_SUCCESS;
}
