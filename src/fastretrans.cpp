/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-12 22:39
 * Last Modified:  2016-06-15 03:47
 * Filename:       fastretrans.cpp
 * Purpose:        hw
 */

#include <string.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include "fastretrans.h"
#include "segment.h"
#include "tool.h"
#include "para.h"
#define PKTLOSSNUM 2048

void clientFastReTrans( int &sockFd, int &currentSeqnum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr )
{
    cout << "Receive a file from " << serverIP << " : " << serverPort << endl;
    socklen_t serSize = sizeof( serverAddr );
    Packet pktTransRcv;
    char fileBuf[FILEMAX];
    uint32_t lastAckNum = 1;
    while ( true )
    {
        recvfrom( sockFd , &pktTransRcv, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, &serSize );
        rcvPktNumMsg( pktTransRcv.tranSeqNum, pktTransRcv.ackNum );
        memcpy( &fileBuf[pktTransRcv.tranSeqNum - 1], pktTransRcv.appData, pktTransRcv.tranSize );
        if ( pktTransRcv.transEnd )
        {
            break;
        }
        if ( lastAckNum != 0 && lastAckNum != pktTransRcv.tranSeqNum )
        {
            Packet dataAck( CLIENT_PORT, serverPort, currentSeqnum, pktTransRcv.seqNum + 1 );
            dataAck.tranAckNum = lastAckNum;
            sendto( sockFd, &dataAck, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
        }
        else
        {
            Packet dataAck( CLIENT_PORT, serverPort, ++currentSeqnum, pktTransRcv.seqNum + 1 );
            dataAck.tranAckNum = pktTransRcv.tranSeqNum + pktTransRcv.tranSize;
            sendto( sockFd, &dataAck, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
            lastAckNum = dataAck.tranAckNum;
        }
    }
    cout << "The file transmission finished" << endl;
    int output = creat( "output", 0666 );
    write( output, fileBuf, sizeof( fileBuf ) );
    close( output );
}

void serverFastReTrans( int &sockFd, int &currentSeqnum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktTransAck )
{
    socklen_t cliSize = sizeof( clientAddr );
    int cwnd = 1;
    int preCwnd = 0;
    int rwnd = pktTransAck.rwnd;
    int bytesLeft = FILEMAX;
    int sndIndex = 0;
    char fileBuf[FILEMAX];
    randFile( fileBuf, FILEMAX );

    int dupAckCnt = 0;
    uint32_t lastAckNum = 0;
    bool simulated = false;
    int threshold = THRESHOLD;

    int state = SLOWSTART;
    cout << "Start to send the file,the file size is 10240 bytes." << endl;
    cout << "*****Slow start*****" << endl;
    bool jumpout = false;
    while ( true )
    {
        if ( ( state == SLOWSTART || state == FASTRECOVERY ) && cwnd >= threshold )
        {
            cout << "**********Start Congestion Avoidance*********" << endl;
            state = CONAVOID;
        }
        vector< pair<uint32_t, uint32_t> > msgBuf;
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
        cout << "cwnd = " << cwnd << ", rwnd = " << rwnd - preCwnd << ", threshold = " << threshold << endl;
        for ( int i = 0; i < cnt; i++ )
        {
            cout << "\tSend a packet at : " << sndIndex + 1 << " byte " << endl;
            if ( sndIndex + 1 == PKTLOSSNUM && !simulated )
            {
                cout << "*****Data loss at byte : " << PKTLOSSNUM << endl;
                bytesLeft -= siz;
                sndIndex += siz;
                continue;
            }
            Packet dataSnd( SERVER_PORT, clientPort, ++currentSeqnum, pktTransAck.seqNum + 1 );
            dataSnd.tranSeqNum = sndIndex + 1;
            dataSnd.tranSize = bytesLeft < siz ? bytesLeft : siz;
            dataSnd.transEnd = bytesLeft < siz;
            bzero( &dataSnd.appData, sizeof( dataSnd.appData ) );
            memcpy( dataSnd.appData, ( void * )&fileBuf[sndIndex], dataSnd.tranSize );
            sendto( sockFd, &dataSnd, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, cliSize );
            bytesLeft -= siz;
            sndIndex += siz;
            if ( bytesLeft <= 0 )
            {
                break;
            }
            preCwnd = cwnd;
            lastAckNum = pktTransAck.tranAckNum;
            recvfrom( sockFd , &pktTransAck, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, &cliSize );
            /* rcvPktNumMsg( pktTransAck.seqNum, pktTransAck.tranAckNum ); */
            msgBuf.push_back( pair<uint32_t, uint32_t>( pktTransAck.seqNum, pktTransAck.tranAckNum ) );
            if ( lastAckNum == pktTransAck.tranAckNum )
            {
                if ( ++dupAckCnt == 3 )
                {
                    for ( unsigned int i = 0; i < msgBuf.size(); i++ )
                    {
                        rcvPktNumMsg( msgBuf[i].first, msgBuf[i].second );
                    }
                    cout << endl;
                    cout << "Receive three Dup Acks" << endl;
                    cout << "******FAST RETRANSMIT*****" << endl;
                    cout << endl;
                    threshold = cwnd / 2;
                    cwnd = 1;
                    preCwnd = 0;
                    bytesLeft = FILEMAX - ( lastAckNum - 1 );
                    sndIndex = lastAckNum - 1;
                    jumpout = true;
                    simulated = true;
                    break;
                }
            }
            else
            {
                jumpout = false;
            }
        }
        if ( jumpout )
        {
            continue;
        }
        for ( unsigned int i = 0; i < msgBuf.size(); i++ )
        {
            rcvPktNumMsg( msgBuf[i].first, msgBuf[i].second );
        }
        if ( state == SLOWSTART )
        {
            cwnd *= 2;
        }
        else if ( state == CONAVOID )
        {
            cwnd += MSS;
        }
        if ( bytesLeft <= 0 )
        {
            break;
        }
    }
    cout << "The file transmission finished" << endl;
}
