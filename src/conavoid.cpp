/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-12 02:47
 * Last Modified:  2016-06-12 02:47
 * Filename:       congenavoid.cpp
 * Purpose:        HW
 */

#include "delay.h"
#include "tool.h"
#include "conavoid.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void clientConAvoid( int &sockFd, int &currentSeqnum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr )
{
    cout << "Receive a file from " << serverIP << " : " << serverPort << endl;
    socklen_t serSize = sizeof( serverAddr );
    Packet pktTransRcv;
    int rwnd = FILEMAX, rcvIndex = 0;
    char fileBuf[FILEMAX];
    int pktCount = 1;
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
        dataAck.rcvWin = rwnd;
        if ( pktCount % 2 == 0 )
        {
            sendto( sockFd, &dataAck, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
        }
        pktCount++;
    }
    cout << "The file transmission finished" << endl;
    int output = creat( "output", 0666 );
    write( output, fileBuf, sizeof( fileBuf ) );
    close( output );
}

void serverConAvoid( int &sockFd, int &currentSeqnum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktTransAck )
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
    int state = SLOWSTART;
    cout << "Start to send the file,the file size is 10240 bytes." << endl;
    cout << "*****Slow start*****" << endl;
    while ( true )
    {
        if ( state == SLOWSTART && cwnd >= THRESHOLD )
        {
            cout << "**********Start Congestion Avoidance*********" << endl;
            state = CONAVOID;
        }
        cout << "cwnd = " << cwnd << ", rwnd = " << pktTransAck.rcvWin << ", threshold = " << THRESHOLD << endl;
        cout << "       Send a packet at : " << sndIndex + 1 << " byte " << endl;
        if ( state == SLOWSTART )
        {
            Packet dataSnd( SERVER_PORT, clientPort, ++currentSeqnum, pktTransAck.seqNum + 1 );
            dataSnd.tranSeqNum = sndIndex + 1;
            dataSnd.tranSize = cwnd;
            bzero( &dataSnd.appData, sizeof( dataSnd.appData ) );
            memcpy( dataSnd.appData, ( void * )&fileBuf[sndIndex], byesLeft < cwnd ? byesLeft : cwnd );
            sendto( sockFd, &dataSnd, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, cliSize );
        }
        else if ( state == CONAVOID )
        {
            int cnt, siz;
            if ( cwnd > MSS )
            {
                cnt = cwnd / MSS + ( ( cwnd % MSS == 0 ) ? 0 : 1 );
                siz = MSS;
            }
            else
            {
                cnt = 1;
                siz = cwnd;
            }
        }
        byesLeft -= cwnd;
        sndIndex += cwnd;
        if ( byesLeft < 0 )
        {
            break;
        }
        if ( state == SLOWSTART )
        {
            cwnd *= 2;
        }
        else if ( state == CONAVOID )
        {
            cwnd += MSS;
        }
        if ( pktCount % 2 == 0 )
        {
            recvfrom( sockFd , &pktTransAck, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, &cliSize );
            rcvPktNumMsg( pktTransAck.seqNum, pktTransAck.tranAckNum );
        }
        else
        {
            pktTransAck.seqNum++;
        }
        pktCount++;
    }
    cout << "The file transmission finished" << endl;
}
