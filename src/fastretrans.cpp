/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-12 22:39
 * Last Modified:  2017-06-11 21:03
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

void clientFastReTrans( int &sockFd, int &curPktSeqNum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr )
{
    cout << "Receive a file from " << serverIP << " : " << serverPort << endl;
    socklen_t serSize = sizeof( serverAddr );
    Packet pktDataRcv;
    char fileBuf[FILEMAX];
    uint32_t lastDataPktAckNum = 1;
    while ( true )
    {
        recvfrom( sockFd , &pktDataRcv, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, &serSize );
        rcvPktNumMsg( pktDataRcv.dataPktSeqNum, pktDataRcv.pktAckNum );
        memcpy( &fileBuf[pktDataRcv.dataPktSeqNum - 1], pktDataRcv.appData, pktDataRcv.dataSize );
        if ( pktDataRcv.transEnd )
        {
            break;
        }
        if ( lastDataPktAckNum != 0 && lastDataPktAckNum != pktDataRcv.dataPktSeqNum )
        {
            Packet dataAck( CLIENT_PORT, serverPort, curPktSeqNum, pktDataRcv.pktSeqNum + 1 );
            dataAck.dataPktAckNum = lastDataPktAckNum;
            sendto( sockFd, &dataAck, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
        }
        else
        {
            Packet dataAck( CLIENT_PORT, serverPort, ++curPktSeqNum, pktDataRcv.pktSeqNum + 1 );
            dataAck.dataPktAckNum = pktDataRcv.dataPktSeqNum + pktDataRcv.dataSize;
            sendto( sockFd, &dataAck, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
            lastDataPktAckNum = dataAck.dataPktAckNum;
        }
    }
    cout << "The file transmission finished" << endl;
    int output = creat( "output", 0666 );
    write( output, fileBuf, sizeof( fileBuf ) );
    close( output );
}

void serverFastReTrans( int &sockFd, int &curPktSeqNum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktDataAck )
{
    socklen_t cliSize = sizeof( clientAddr );
    int cwnd = 1;
    int preCwnd = 0;
    int rwnd = pktDataAck.rwnd;
    int bytesLeft = FILEMAX;
    int sndIndex = 0;
    char fileBuf[FILEMAX];
    randFile( fileBuf, FILEMAX );

    int dupAckCnt = 0;
    uint32_t lastDataPktAckNum = 0;
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
        vector<pair<uint32_t, uint32_t>> msgBuf;
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
            Packet pktDataSnd( SERVER_PORT, clientPort, ++curPktSeqNum, pktDataAck.pktSeqNum + 1 );
            pktDataSnd.dataPktSeqNum = sndIndex + 1;
            pktDataSnd.dataSize = bytesLeft < siz ? bytesLeft : siz;
            pktDataSnd.transEnd = bytesLeft < siz;
            bzero( &pktDataSnd.appData, sizeof( pktDataSnd.appData ) );
            memcpy( pktDataSnd.appData, ( void * )&fileBuf[sndIndex], pktDataSnd.dataSize );
            sendto( sockFd, &pktDataSnd, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, cliSize );
            bytesLeft -= siz;
            sndIndex += siz;
            if ( bytesLeft <= 0 )
            {
                break;
            }
            preCwnd = cwnd;
            lastDataPktAckNum = pktDataAck.dataPktAckNum;
            recvfrom( sockFd , &pktDataAck, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, &cliSize );
            /* rcvPktNumMsg( pktDataAck.pktSeqNum, pktDataAck.dataPktAckNum ); */
            msgBuf.push_back( pair<uint32_t, uint32_t>( pktDataAck.pktSeqNum, pktDataAck.dataPktAckNum ) );
            if ( lastDataPktAckNum == pktDataAck.dataPktAckNum )
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
                    bytesLeft = FILEMAX - ( lastDataPktAckNum - 1 );
                    sndIndex = lastDataPktAckNum - 1;
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
