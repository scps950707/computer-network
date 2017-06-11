/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-12 02:47
 * Last Modified:  2017-06-11 21:03
 * Filename:       congenavoid.cpp
 * Purpose:        HW
 */

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include "tool.h"
#include "conavoid.h"

void clientConAvoid( int &sockFd, int &curPktSeqNum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr )
{
    cout << "Receive a file from " << serverIP << " : " << serverPort << endl;
    socklen_t serSize = sizeof( serverAddr );
    Packet pktDataRcv;
    char fileBuf[FILEMAX];
    int pktCount = 1;
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

void serverConAvoid( int &sockFd, int &curPktSeqNum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktDataAck )
{
    socklen_t cliSize = sizeof( clientAddr );
    int cwnd = 1;
    int preCwnd = 0;
    int rwnd = pktDataAck.rwnd;
    int bytesLeft = FILEMAX;
    int sndIndex = 0;
    char fileBuf[FILEMAX];
    randFile( fileBuf, FILEMAX );

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
        cout << "cwnd = " << cwnd << ", rwnd = " << rwnd - preCwnd << ", threshold = " << THRESHOLD << endl;
        for ( int i = 0; i < cnt; i++ )
        {
            cout << "\tSend a packet at : " << sndIndex + 1 << " byte " << endl;
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
            if ( pktCount % 2 == 0 )
            {
                recvfrom( sockFd , &pktDataAck, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, &cliSize );
                /* rcvPktNumMsg( pktDataAck.pktSeqNum, pktDataAck.dataPktAckNum ); */
                msgBuf.push_back( pair<uint32_t, uint32_t>( pktDataAck.pktSeqNum, pktDataAck.dataPktAckNum ) );
            }
            else
            {
                pktDataAck.pktSeqNum++;
            }
            pktCount++;
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
