/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-12 02:47
 * Last Modified:  2016-06-12 21:52
 * Filename:       congenavoid.cpp
 * Purpose:        HW
 */

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include "tool.h"
#include "conavoid.h"

void clientConAvoid( int &sockFd, int &currentSeqnum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr )
{
    cout << "Receive a file from " << serverIP << " : " << serverPort << endl;
    socklen_t serSize = sizeof( serverAddr );
    Packet pktTransRcv;
    int rwnd = FILEMAX;
    int rcvIndex = 0;
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
    int cwnd = 1;
    int rwnd = pktTransAck.rcvWin;
    int sndIndex = 0;
    char fileBuf[FILEMAX];
    string byteList = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for ( int i = 0; i < FILEMAX; i++ )
    {
        fileBuf[i] = byteList[rand() % 62];
    }

    int pktCount = 0;
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
        if ( state == SLOWSTART )
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
            if ( pktCount % 2 == 1 )
            {
                recvfrom( sockFd , &pktTransAck, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, &cliSize );
                rcvPktNumMsg( pktTransAck.seqNum, pktTransAck.tranAckNum );
            }
            else
            {
                pktTransAck.seqNum++;
            }
            pktCount++;
            cwnd *= 2;
        }
        else if ( state == CONAVOID )
        {
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
            cout << "cwnd = " << cwnd << ", rwnd = " << rwnd << ", threshold = " << THRESHOLD << endl;
            for ( int i = 0; i < cnt; i++ )
            {
                cout << "\tSend a packet at : " << sndIndex + 1 << " byte " << endl;
                Packet dataSnd( SERVER_PORT, clientPort, ++currentSeqnum, pktTransAck.seqNum + 1 );
                dataSnd.tranSeqNum = sndIndex + 1;
                dataSnd.tranSize = siz;
                bzero( &dataSnd.appData, sizeof( dataSnd.appData ) );
                memcpy( dataSnd.appData, ( void * )&fileBuf[sndIndex], rwnd < siz ? rwnd : siz );
                sendto( sockFd, &dataSnd, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, cliSize );
                rwnd -= siz;
                sndIndex += siz;
                if ( rwnd < 0 )
                {
                    break;
                }
                if ( pktCount % 2 == 1 )
                {
                    recvfrom( sockFd , &pktTransAck, sizeof( Packet ), 0, ( struct sockaddr * )&clientAddr, &cliSize );
                    /* rcvPktNumMsg( pktTransAck.seqNum, pktTransAck.tranAckNum ); */
                    msgBuf.push_back( pair<uint32_t, uint32_t>( pktTransAck.seqNum, pktTransAck.tranAckNum ) );
                }
                else
                {
                    pktTransAck.seqNum++;
                }
                pktCount++;
            }
            for ( unsigned int i = 0; i < msgBuf.size(); i++ )
            {
                rcvPktNumMsg( msgBuf[i].first, msgBuf[i].second );
            }
            cwnd += MSS;
        }
        if ( rwnd < 0 )
        {
            break;
        }
    }
    cout << "The file transmission finished" << endl;
}
