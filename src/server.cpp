/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:10
 * Last Modified:  2016-06-11 03:19
 * Filename:       server.cpp
 * Purpose:        homework
 */

#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "server.h"
#include "segment.h"
#include "tool.h"
using namespace std;

int main()
{

    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    int sockFd;
#ifdef __SEQSTATIC__
    int currentSeqnum = 3440;
#else
    int currentSeqnum = rand() % 10000 + 1;
#endif
    socklen_t cliSize = sizeof( clientAddr );

    bzero( &serverAddr, sizeof( serverAddr ) );
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    serverAddr.sin_port = htons( SERVER_PORT );

    if ( ( sockFd = socket( serverAddr.sin_family, SOCK_DGRAM, 0 ) ) < 0 )
    {
        perror( "socket" );
        exit( EXIT_FAILURE );
    }

    if ( bind( sockFd, ( struct sockaddr * )&serverAddr, sizeof( serverAddr ) ) < 0 )
    {
        perror( "bind" );
        exit( EXIT_FAILURE );
    }

    cout << "=====Parameter=====" << endl;
    cout << "The RTT delay = 200 ms" << endl;
    cout << "The threshold = 65535 bytes" << endl;
    cout << "The MSS = 512 bytes" << endl;
    cout << "The buffer size = 10240 bytes" << endl;
    cout << "Server's IP is 127.0.0.1 " << endl;
    cout << "Server is listening on port 10250" << endl;
    cout << "===============" << endl;
    cout << "Listening for client..." << endl;
    cout << "=====Start the three-way handshake=====" << endl;

    Packet pktRcv;
    while ( recvfrom( sockFd , &pktRcv, MSS, 0, ( struct sockaddr * )&clientAddr, &cliSize ) )
    {
        if ( pktRcv.SYN == true && pktRcv.ACK == false )
        {
            string ip = getIpStr( &clientAddr.sin_addr );
            rcvPktMsg( "SYN", ip, pktRcv.sourcePort );
            rcvPktNumMsg( pktRcv.seqNum, pktRcv.ackNum );
            Packet synack( SERVER_PORT, pktRcv.sourcePort, currentSeqnum, pktRcv.seqNum + 1 );
            synack.SYN = true;
            synack.ACK = true;
            sendPktMsg( "SYN/ACK", ip, synack.destPort );
            sendto( sockFd, &synack, MSS, 0, ( struct sockaddr * )&clientAddr, cliSize );
        }
        else if ( pktRcv.SYN == false && pktRcv.ACK == true )
        {
            string ip = getIpStr( &clientAddr.sin_addr );
            rcvPktMsg( "ACK", ip, pktRcv.sourcePort );
            rcvPktNumMsg( pktRcv.seqNum, pktRcv.ackNum );
            break;
        }
    }

    cout << "=====Complete the three-way handshake=====" << endl;

    return EXIT_SUCCESS;
}
