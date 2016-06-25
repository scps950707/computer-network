/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-11 15:44
 * Last Modified:  2016-06-11 17:40
 * Filename:       shake.cpp
 * Purpose:        homework
 */


#include <arpa/inet.h>
#include "shake.h"
#include "tool.h"

void ServerThreeWayHandShake( int &sockFd, int &curPktSeqNum, string &clientIP, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktThreeShakeRcv )
{
    socklen_t cliSize = sizeof( clientAddr );
    while ( recvfrom( sockFd , &pktThreeShakeRcv, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, &cliSize ) )
    {
        if ( pktThreeShakeRcv.SYN == true && pktThreeShakeRcv.ACK == false )
        {
            cout << "=====Start the three-way handshake=====" << endl;
            clientIP = string ( inet_ntoa( clientAddr.sin_addr ) );
            clientPort = pktThreeShakeRcv.sourcePort;
            rcvPktMsg( "SYN", clientIP, clientPort );
            rcvPktNumMsg( pktThreeShakeRcv.pktSeqNum, pktThreeShakeRcv.pktAckNum );
            Packet synack( SERVER_PORT, clientPort, ++curPktSeqNum, pktThreeShakeRcv.pktSeqNum + 1 );
            synack.SYN = true;
            synack.ACK = true;
            sendPktMsg( "SYN/ACK", clientIP, clientPort );
            sendto( sockFd, &synack, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, cliSize );
        }
        else if ( pktThreeShakeRcv.SYN == false && pktThreeShakeRcv.ACK == true )
        {
            rcvPktMsg( "ACK", clientIP, clientPort );
            rcvPktNumMsg( pktThreeShakeRcv.pktSeqNum, pktThreeShakeRcv.pktAckNum );
            break;
        }
    }
    cout << "=====Complete the three-way handshake=====" << endl;
}

void ServerFourWayHandShake( int &sockFd, int &curPktSeqNum, int &curRcvpktSeqNum, string &clientIP, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktFourShake )
{
    cout << "=====Start the four-way handshake=====" << endl;
    socklen_t cliSize = sizeof( clientAddr );
    Packet fin( SERVER_PORT, clientPort, ++curPktSeqNum, curRcvpktSeqNum + 1 );
    fin.FIN = true;
    sendPktMsg( "FIN", clientIP, clientPort );
    sendto( sockFd, &fin, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, cliSize );

    while ( recvfrom( sockFd , &pktFourShake, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, &cliSize ) )
    {
        if ( pktFourShake.FIN == true )
        {
            rcvPktMsg( "FIN", clientIP, clientPort );
            rcvPktNumMsg( pktFourShake.pktSeqNum, pktFourShake.pktAckNum );
            Packet ack( SERVER_PORT, clientPort, ++curPktSeqNum, pktFourShake.pktSeqNum + 1 );
            ack.ACK = true;
            sendPktMsg( "ACK", clientIP, clientPort );
            sendto( sockFd, &ack, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, cliSize );
            break;
        }
        if ( pktFourShake.ACK == true )
        {
            rcvPktMsg( "ACK", clientIP, clientPort );
            rcvPktNumMsg( pktFourShake.pktSeqNum, pktFourShake.pktAckNum );
        }
    }
    cout << "=====Complete the four-way handshake=====" << endl;
}

void ClientThreeWayHandShake( int &sockFd, int &curPktSeqNum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr )
{
    cout << "=====Start the three-way handshake=====" << endl;
    socklen_t serSize = sizeof( serverAddr );
    Packet pktThreeShakeSyn( CLIENT_PORT, serverPort, curPktSeqNum, 0 );
    pktThreeShakeSyn.SYN = true;
    sendPktMsg( "SYN", serverIP, serverPort );
    sendto( sockFd, &pktThreeShakeSyn, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );

    Packet pktThreeShake;
    while ( recvfrom( sockFd , &pktThreeShake, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, &serSize ) )
    {
        if ( pktThreeShake.SYN == true && pktThreeShake.ACK == true )
        {
            rcvPktMsg( "SYN/ACK", serverIP, pktThreeShake.sourcePort );
            rcvPktNumMsg( pktThreeShake.pktSeqNum, pktThreeShake.pktAckNum );
            Packet ack( CLIENT_PORT, serverPort, ++curPktSeqNum, pktThreeShake.pktSeqNum + 1 );
            ack.ACK = true;
            ack.rwnd = FILEMAX;
            sendPktMsg( "ACK", serverIP, serverPort );
            sendto( sockFd, &ack, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
            break;
        }
    }
    cout << "=====Complete the three-way handshake=====" << endl;
}

void ClientFourWayHandShake( int &sockFd, int &curPktSeqNum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr )
{
    socklen_t serSize = sizeof( serverAddr );
    Packet pktFourShake;
    while ( recvfrom( sockFd , &pktFourShake, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, &serSize ) )
    {
        if ( pktFourShake.FIN == true )
        {
            cout << "=====Start the four-way handshake=====" << endl;
            rcvPktMsg( "FIN", serverIP, pktFourShake.sourcePort );
            rcvPktNumMsg( pktFourShake.pktSeqNum, pktFourShake.pktAckNum );
            Packet ack( CLIENT_PORT, serverPort, ++curPktSeqNum, pktFourShake.pktSeqNum + 1 );
            ack.ACK = true;
            sendPktMsg( "ACK", serverIP, serverPort );
            sendto( sockFd, &ack, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
            Packet fin( CLIENT_PORT, serverPort, curPktSeqNum, pktFourShake.pktSeqNum + 1 );
            fin.FIN = true;
            sendPktMsg( "FIN", serverIP, serverPort );
            sendto( sockFd, &fin, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
        }
        if ( pktFourShake.ACK == true )
        {
            rcvPktMsg( "ACK", serverIP, pktFourShake.sourcePort );
            rcvPktNumMsg( pktFourShake.pktSeqNum, pktFourShake.pktAckNum );
            break;
        }
    }
    cout << "=====Complete the four-way handshake=====" << endl;
}
