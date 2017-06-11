/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-14 21:25
 * Last Modified:  2017-06-11 21:03
 * Filename:       sack.cpp
 * Purpose:        hw
 */

#include <string.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <iomanip>
#include "sack.h"
#include "segment.h"
#include "tool.h"
#include "para.h"
#define PKTLOSSNUM 5120
#define PKTLOSSNUM_2 6144
#define PKTLOSSNUM_3 7168

void printSack( uint32_t *sackBuffer )
{
    for ( int i = 0; i < SACKSIZE; i++ )
    {
        if ( sackBuffer[i] != 0 )
        {
            cout << sackBuffer[i] << "\t";
        }
    }
    cout << endl;
}

void removeleft( uint32_t *sackBuffer, int &sackCnt )
{
    for ( int i = 0; i < SACKSIZE - 2; i++ )
    {
        sackBuffer[i] = sackBuffer[i + 2];
    }
    sackBuffer[SACKSIZE - 2] = 0;
    sackBuffer[SACKSIZE - 1] = 0;
    sackCnt -= 2;
}

void clientSack( int &sockFd, int &curPktSeqNum, string &serverIP, uint16_t &serverPort, sockaddr_in &serverAddr )
{
    cout << "Receive a file from " << serverIP << " : " << serverPort << endl;
    socklen_t serSize = sizeof( serverAddr );
    Packet pktDataRcv;
    char fileBuf[FILEMAX];
    uint32_t lastDataPktAckNum = 1;
    uint32_t sackBuffer[SACKSIZE];
    bzero( sackBuffer, sizeof( sackBuffer ) );
    int sackCnt = 0;
    cout << "ACK\t1 Left 1 Right 2 Left 2 Right 3 Left 3 Right" << endl;
    while ( true )
    {
        recvfrom( sockFd , &pktDataRcv, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, &serSize );
        /* rcvPktNumMsg( pktDataRcv.dataPktSeqNum, pktDataRcv.pktAckNum ); */
        memcpy( &fileBuf[pktDataRcv.dataPktSeqNum - 1], pktDataRcv.appData, pktDataRcv.dataSize );
        if ( pktDataRcv.transEnd )
        {
            break;
        }
        if ( pktDataRcv.dataPktSeqNum != lastDataPktAckNum )
        {
            sackBuffer[sackCnt++] = pktDataRcv.dataPktSeqNum;
            sackBuffer[sackCnt++] = pktDataRcv.dataPktSeqNum + pktDataRcv.dataSize;
        }
        if ( lastDataPktAckNum != 0 && lastDataPktAckNum != pktDataRcv.dataPktSeqNum )
        {
            Packet dataAck( CLIENT_PORT, serverPort, curPktSeqNum, pktDataRcv.pktSeqNum + 1 );
            dataAck.dataPktAckNum = lastDataPktAckNum;
            memcpy( dataAck.sackBuffer, sackBuffer, sizeof( sackBuffer ) );
            sendto( sockFd, &dataAck, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
            cout << left << setw( 5 ) << dataAck.dataPktAckNum << "\t";
        }
        else
        {
            Packet dataAck( CLIENT_PORT, serverPort, ++curPktSeqNum, pktDataRcv.pktSeqNum + 1 );
            if ( sackCnt != 0 )
            {
                dataAck.dataPktAckNum = sackBuffer[1];
            }
            else
            {
                dataAck.dataPktAckNum = pktDataRcv.dataPktSeqNum + pktDataRcv.dataSize;
            }
            memcpy( dataAck.sackBuffer, sackBuffer, sizeof( sackBuffer ) );
            sendto( sockFd, &dataAck, sizeof( Packet ), 0, ( struct sockaddr * )&serverAddr, serSize );
            lastDataPktAckNum = dataAck.dataPktAckNum;
            cout << left <<  setw( 5 ) << dataAck.dataPktAckNum << "\t";
            if ( dataAck.dataPktAckNum == sackBuffer[1] )
            {
                removeleft( sackBuffer, sackCnt );
            }
        }
        printSack( sackBuffer );
    }
    cout << "The file transmission finished" << endl;
    int output = creat( "output", 0666 );
    write( output, fileBuf, sizeof( fileBuf ) );
    close( output );
}

void serverSack( int &sockFd, int &curPktSeqNum, uint16_t &clientPort, sockaddr_in &clientAddr, Packet &pktDataAck )
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
            bool skip = false;
            if ( simulated )
            {
                for ( int i = 0; i < SACKSIZE; i += 2 )
                {
                    if ( pktDataAck.sackBuffer[i] == ( uint32_t )( sndIndex + 1 ) )
                    {
                        skip = true;
                        break;
                    }
                }
            }
            if ( skip )
            {
                bytesLeft -= siz;
                sndIndex += siz;
                i--;
                continue;
            }
            cout << "\tSend a packet at : " << sndIndex + 1 << " byte " << endl;
            if ( ( sndIndex + 1 == PKTLOSSNUM || sndIndex + 1 == PKTLOSSNUM_2 || sndIndex + 1 == PKTLOSSNUM_3 )  && !simulated )
            {
                cout << "*****Data loss at byte : " << sndIndex + 1 << endl;
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
                    cout << "******FAST RECOVERY*****" << endl;
                    cout << endl;
                    threshold = cwnd / 2;
                    cwnd /= 2;
                    preCwnd = 0;
                    bytesLeft = FILEMAX - ( lastDataPktAckNum - 1 );
                    sndIndex = lastDataPktAckNum - 1;
                    jumpout = true;
                    simulated = true;
                    state = FASTRECOVERY;
                    break;
                }
            }
            else
            {
                jumpout = false;
            }
        }
        if ( state == SLOWSTART )
        {
            cwnd *= 2;
        }
        else if ( state == CONAVOID )
        {
            cwnd += MSS;
        }
        else if ( state == FASTRECOVERY )
        {
            cwnd += MSS;
        }
        if ( jumpout )
        {
            continue;
        }
        for ( unsigned int i = 0; i < msgBuf.size(); i++ )
        {
            rcvPktNumMsg( msgBuf[i].first, msgBuf[i].second );
        }
        if ( bytesLeft <= 0 )
        {
            break;
        }
    }
    cout << "The file transmission finished" << endl;
}
