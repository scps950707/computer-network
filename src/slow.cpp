/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-11 16:18
 * Last Modified:  2016-06-13 23:03
 * Filename:       slow.cpp
 * Purpose:        HW
 */

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "slow.h"
#include "tool.h"

void clientSlowStart( int &sockFd, int &curPktSeqNum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr )
{
    cout << "Receive a file from " << serverIP << " : " << serverPort << endl;
    socklen_t serSize = sizeof( serverAddr );
    Packet pktDataRcv;
    char fileBuf[FILEMAX];
    while ( true )
    {
        recvfrom( sockFd , &pktDataRcv, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, &serSize );
        rcvPktNumMsg( pktDataRcv.dataPktSeqNum, pktDataRcv.pktAckNum );
        memcpy( &fileBuf[pktDataRcv.dataPktSeqNum - 1], pktDataRcv.appData, pktDataRcv.dataSize );
        if ( pktDataRcv.transEnd )
        {
            break;
        }
        Packet dataAck( CLIENT_PORT, serverPort, ++curPktSeqNum, pktDataRcv.pktSeqNum + 1 );
        dataAck.dataPktAckNum = pktDataRcv.dataPktSeqNum + pktDataRcv.dataSize;
        sendto( sockFd, &dataAck, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
    }
    cout << "The file transmission finished" << endl;
    int output = creat( "output", 0666 );
    write( output, fileBuf, sizeof( fileBuf ) );
    close( output );
}

void serverSlowStart( int &sockFd, int &curPktSeqNum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktDataAck )
{
    socklen_t cliSize = sizeof( clientAddr );
    int cwnd = 1;
    int preCwnd = 0;
    int rwnd = pktDataAck.rwnd;
    int bytesLeft = FILEMAX;
    int sndIndex = 0;
    char fileBuf[FILEMAX];
    randFile( fileBuf, FILEMAX );

    cout << "Start to send the file,the file size is 10240 bytes." << endl;
    cout << "*****Slow start*****" << endl;
    while ( true )
    {
        cout << "cwnd = " << cwnd << ", rwnd = " << rwnd - preCwnd << ", threshold = " << THRESHOLD << endl;
        cout << "\tSend a packet at : " << sndIndex + 1 << " byte " << endl;
        Packet pktDataSnd( SERVER_PORT, clientPort, ++curPktSeqNum, pktDataAck.pktSeqNum + 1 );
        pktDataSnd.dataPktSeqNum = sndIndex + 1;
        pktDataSnd.dataSize = bytesLeft < cwnd ? bytesLeft : cwnd;
        pktDataSnd.transEnd = bytesLeft < cwnd;
        bzero( &pktDataSnd.appData, sizeof( pktDataSnd.appData ) );
        memcpy( pktDataSnd.appData, ( void * )&fileBuf[sndIndex], pktDataSnd.dataSize );
        sendto( sockFd, &pktDataSnd, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, cliSize );
        bytesLeft -= pktDataSnd.dataSize;
        sndIndex += pktDataSnd.dataSize;
        if ( bytesLeft <= 0 )
        {
            break;
        }
        preCwnd = cwnd;
        if ( cwnd < 512 )
        {
            cwnd *= 2;
        }
        recvfrom( sockFd , &pktDataAck, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, &cliSize );
        rcvPktNumMsg( pktDataAck.pktSeqNum, pktDataAck.dataPktAckNum );
    }
    cout << "The file transmission finished" << endl;
}
