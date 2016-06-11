/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-11 16:18
 * Last Modified:  2016-06-12 02:32
 * Filename:       delay.cpp
 * Purpose:        HW
 */

#include "delay.h"
#include "tool.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void clientDelayAck( int &sockFd, int &currentSeqnum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr )
{
    cout << "Receive a file from " << serverIP << " : " << serverPort << endl;
    socklen_t serSize = sizeof( serverAddr );
    Packet pktTransRcv;
    int rwnd = FILEMAX, rcvIndex = 0;
    char fileBuf[FILEMAX];
    int pktCount = 1;
    while ( recvfrom( sockFd , &pktTransRcv, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, &serSize ) )
    {
        rcvPktNumMsg( pktTransRcv.tranSeqNum, pktTransRcv.ackNum );
        memcpy( &fileBuf[rcvIndex], pktTransRcv.appData, ( int )pktTransRcv.tranSeqNum > rwnd ? rwnd : pktTransRcv.tranSeqNum );
        rwnd -= pktTransRcv.tranSeqNum;
        rcvIndex += pktTransRcv.tranSeqNum;
        Packet dataAck( CLIENT_PORT, serverPort, ++currentSeqnum, pktTransRcv.seqNum + 1 );
        dataAck.tranAckNum = pktTransRcv.tranSeqNum < 512 ? pktTransRcv.tranSeqNum * 2 : pktTransRcv.tranSeqNum ;
        dataAck.rcvWin = rwnd;
        if ( pktCount % 2 == 0 )
        {
            sendto( sockFd, &dataAck, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
        }
        if ( rwnd <= 0 )
        {
            break;
        }
        pktCount++;
    }
    cout << "The file transmission finished" << endl;
    int output = creat("output",0666);
    write(output,fileBuf,sizeof(fileBuf));
    close(output);
}

void serverDelayAck( int &sockFd, int &currentSeqnum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktTransAck )
{
    socklen_t cliSize = sizeof( clientAddr );
    int cwnd = 1, sndIndex = 0, byesLeft = FILEMAX;
    char fileBuf[FILEMAX];
    string byteList = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for ( int i = 0; i < FILEMAX; i++ )
    {
        fileBuf[i] = byteList[rand() % 62];
    }

    int pktCount = 1;
    cout << "Start to send the file,the file size is 10240 bytes." << endl;
    cout << "*****Slow start*****" << endl;
    while ( byesLeft > 0 )
    {
        cout << "cwnd = " << cwnd << ", rwnd = " << pktTransAck.rcvWin << ", threshold = " << THRESHOLD << endl;
        cout << "       Send a packet at : " << cwnd << " byte " << endl;
        Packet dataSnd( SERVER_PORT, clientPort, ++currentSeqnum, pktTransAck.seqNum + 1 );
        dataSnd.tranSeqNum = cwnd;
        bzero( &dataSnd.appData, sizeof( dataSnd.appData ) );
        memcpy( dataSnd.appData, ( void * )&fileBuf[sndIndex], byesLeft < cwnd ? byesLeft : cwnd );
        sendto( sockFd, &dataSnd, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, cliSize );
        byesLeft -= cwnd;
        sndIndex += cwnd;
        if ( cwnd < 512 )
        {
            cwnd *= 2;
        }
        if ( pktCount % 2 == 0 )
        {
            recvfrom( sockFd , &pktTransAck, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, &cliSize );
            rcvPktNumMsg( pktTransAck.seqNum - 1, pktTransAck.tranAckNum );
        }
        pktCount++;
    }
    cout << "The file transmission finished" << endl;
}
