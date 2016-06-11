/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:10
 * Last Modified:  2016-06-11 16:23
 * Filename:       server.cpp
 * Purpose:        homework
 */

#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdint.h>
#include "server.h"
#include "segment.h"
#include "tool.h"
#include "para.h"
#include "shake.h"
using namespace std;

int main()
{
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    int sockFd;
#ifdef __SEQSTATIC__
    int currentSeqnum = 3439;
#else
    int currentSeqnum = rand() % 10000 + 1;
#endif
    int curRcvSeqnum;
    socklen_t cliSize = sizeof( clientAddr );
    string clientIP;
    uint16_t clientPort;

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

    Packet pktThreeShakeRcv;
    ServerThreeWayHandShake( sockFd, currentSeqnum, clientIP, clientPort, clientAddr, pktThreeShakeRcv );
    curRcvSeqnum = pktThreeShakeRcv.seqNum;

    cout << "=====Complete the three-way handshake=====" << endl;

    int cwnd = 1, sndIndex = 0, byesLeft = FILEMAX;
    char fileBuf[FILEMAX];
    string byteList = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for ( int i = 0; i < FILEMAX; i++ )
    {
        fileBuf[i] = byteList[rand() % 62];
    }

    cout << "Start to send the file,the file size is 10240 bytes." << endl;
    cout << "*****Slow start*****" << endl;
    Packet pktTransAck;
    pktTransAck.seqNum = curRcvSeqnum;
    while ( byesLeft > 0 )
    {
        cout << "cwnd = " << cwnd << ", rwnd = " << pktTransAck.rcvWin << ", threshold = " << THRESHOLD << endl;
        cout << "       Send a packet at : " << cwnd << " byte " << endl;
        Packet dataSnd( SERVER_PORT, clientPort, ++currentSeqnum, pktTransAck.seqNum + 1 );
        bzero( &dataSnd.appData, sizeof( dataSnd.appData ) );
        memcpy( dataSnd.appData, ( void * )&fileBuf[sndIndex], byesLeft < cwnd ? byesLeft : cwnd );
        sendto( sockFd, &dataSnd, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, cliSize );
        byesLeft -= cwnd;
        sndIndex += cwnd;
        if ( cwnd < 512 )
        {
            cwnd *= 2;
        }
        recvfrom( sockFd , &pktTransAck, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, &cliSize );
#ifdef __TRANSEQ__
        rcvPktNumMsg( pktTransAck.seqNum, pktTransAck.ackNum );
#else
        rcvPktNumMsg( pktTransAck.seqNum, cwnd );
#endif
    }
    curRcvSeqnum = pktTransAck.seqNum;


    cout << "=====Start the four-way handshake=====" << endl;

    Packet pktFourShake;
    ServerFourWayHandShake( sockFd, currentSeqnum, curRcvSeqnum, clientIP, clientPort, clientAddr, pktFourShake );
    curRcvSeqnum = pktFourShake.seqNum;

    cout << "=====Complete the four-way handshake=====" << endl;

    return EXIT_SUCCESS;
}
