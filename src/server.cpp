/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:10
 * Last Modified:  2016-06-11 15:11
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
    while ( recvfrom( sockFd , &pktThreeShakeRcv, sizeof(Packet), 0, ( struct sockaddr * )&clientAddr, &cliSize ) )
    {
        if ( pktThreeShakeRcv.SYN == true && pktThreeShakeRcv.ACK == false )
        {
            clientIP = getIpStr( &clientAddr.sin_addr );
            clientPort = pktThreeShakeRcv.sourcePort;
            rcvPktMsg( "SYN", clientIP, clientPort );
            rcvPktNumMsg( pktThreeShakeRcv.seqNum, pktThreeShakeRcv.ackNum );
            Packet synack( SERVER_PORT, clientPort, ++currentSeqnum, pktThreeShakeRcv.seqNum + 1 );
            synack.SYN = true;
            synack.ACK = true;
            sendPktMsg( "SYN/ACK", clientIP, clientPort );
            sendto( sockFd, &synack, sizeof(Packet), 0, ( struct sockaddr * )&clientAddr, cliSize );
        }
        else if ( pktThreeShakeRcv.SYN == false && pktThreeShakeRcv.ACK == true )
        {
            rcvPktMsg( "ACK", clientIP, clientPort );
            rcvPktNumMsg( pktThreeShakeRcv.seqNum, pktThreeShakeRcv.ackNum );
            break;
        }
    }
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

    Packet fin( SERVER_PORT, clientPort, ++currentSeqnum, curRcvSeqnum + 1 );
    fin.FIN = true;
    sendPktMsg( "FIN", clientIP, clientPort );
    sendto( sockFd, &fin, sizeof(Packet), 0, ( struct sockaddr * )&clientAddr, cliSize );

    Packet pktFourShake;
    while ( recvfrom( sockFd , &pktFourShake, sizeof(Packet), 0, ( struct sockaddr * )&clientAddr, &cliSize ) )
    {
        if ( pktFourShake.FIN == true )
        {
            rcvPktMsg( "FIN", clientIP, clientPort );
            rcvPktNumMsg( pktFourShake.seqNum, pktFourShake.ackNum );
            Packet ack( SERVER_PORT, clientPort, ++currentSeqnum, pktFourShake.seqNum + 1 );
            ack.ACK = true;
            sendPktMsg( "ACK", clientIP, clientPort );
            sendto( sockFd, &ack, sizeof(Packet), 0, ( struct sockaddr * )&clientAddr, cliSize );
            break;
        }
        if ( pktFourShake.ACK == true )
        {
            rcvPktMsg( "ACK", clientIP, clientPort );
            rcvPktNumMsg( pktFourShake.seqNum, pktFourShake.ackNum );
        }
    }
    curRcvSeqnum = pktFourShake.seqNum;

    cout << "=====Complete the four-way handshake=====" << endl;

    return EXIT_SUCCESS;
}
