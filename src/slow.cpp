/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-11 16:18
 * Last Modified:  2016-06-12 20:42
 * Filename:       slow.cpp
 * Purpose:        HW
 */

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "slow.h"
#include "tool.h"

void clientSlowStart( int &sockFd, int &currentSeqnum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr )
{
    cout << "Receive a file from " << serverIP << " : " << serverPort << endl;
    socklen_t serSize = sizeof( serverAddr );
    Packet pktTransRcv;
    int rwnd = FILEMAX;
    int rcvIndex = 0;
    char fileBuf[FILEMAX];
    while ( true )
    {
        recvfrom( sockFd , &pktTransRcv, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, &serSize );
        rcvPktNumMsg( pktTransRcv.tranSeqNum, pktTransRcv.ackNum );
        memcpy( &fileBuf[rcvIndex], pktTransRcv.appData, pktTransRcv.tranSize > rwnd ? rwnd : pktTransRcv.tranSize );
        rwnd -= pktTransRcv.tranSize;
        rcvIndex += pktTransRcv.tranSize;
        if ( rwnd < 0 )
        {
            break;
        }
        Packet dataAck( CLIENT_PORT, serverPort, ++currentSeqnum, pktTransRcv.seqNum + 1 );
        dataAck.tranAckNum = rcvIndex + 1;
        sendto( sockFd, &dataAck, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
    }
    cout << "The file transmission finished" << endl;
    int output = creat( "output", 0666 );
    write( output, fileBuf, sizeof( fileBuf ) );
    close( output );
}

void serverSlowStart( int &sockFd, int &currentSeqnum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktTransAck )
{
    socklen_t cliSize = sizeof( clientAddr );
    int cwnd = 1;
    int rwnd = pktTransAck.rwnd;
    int sndIndex = 0;
    char fileBuf[FILEMAX];
    string byteList = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for ( int i = 0; i < FILEMAX; i++ )
    {
        fileBuf[i] = byteList[rand() % 62];
    }

    cout << "Start to send the file,the file size is 10240 bytes." << endl;
    cout << "*****Slow start*****" << endl;
    while ( true )
    {
        cout << "cwnd = " << cwnd << ", rwnd = " << rwnd << ", threshold = " << THRESHOLD << endl;
        cout << "\tSend a packet at : " << sndIndex + 1 << " byte " << endl;
        Packet dataSnd( SERVER_PORT, clientPort, ++currentSeqnum, pktTransAck.seqNum + 1 );
        dataSnd.tranSeqNum = sndIndex + 1;
        dataSnd.tranSize = cwnd;
        bzero( &dataSnd.appData, sizeof( dataSnd.appData ) );
        memcpy( dataSnd.appData, ( void * )&fileBuf[sndIndex], rwnd < cwnd ? rwnd : cwnd );
        sendto( sockFd, &dataSnd, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, cliSize );
        rwnd -= cwnd;
        sndIndex += cwnd;
        if ( rwnd < 0 )
        {
            break;
        }
        if ( cwnd < 512 )
        {
            cwnd *= 2;
        }
        recvfrom( sockFd , &pktTransAck, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, &cliSize );
        rcvPktNumMsg( pktTransAck.seqNum, pktTransAck.tranAckNum );
    }
    cout << "The file transmission finished" << endl;
}
