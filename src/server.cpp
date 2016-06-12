/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:10
 * Last Modified:  2016-06-12 02:32
 * Filename:       server.cpp
 * Purpose:        homework
 */

#include <string.h>
#include <arpa/inet.h>
#include "server.h"
#include "shake.h"
#include "slow.h"
#include "delay.h"
#include "conavoid.h"
using namespace std;

int main()
{
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    int sockFd;
#ifdef __SEQSTATIC__
    int currentSeqnum = 3439;
#else
    int currentSeqnum = rand() % 10000 + 1;
#endif
    int curRcvSeqnum;
    string clientIP;
    uint16_t clientPort;

    bzero( &serverAddr, sizeof( serverAddr ) );
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    serverAddr.sin_port = htons( SERVER_PORT );

    if ( ( sockFd = socket( serverAddr.sin_family, SOCK_DGRAM, 0 ) ) < 0 )
    {
        perror( "socket" );
        exit( EXIT_FAILURE );
    }

    if ( bind( sockFd, ( struct sockaddr * )&serverAddr, sizeof( serverAddr ) ) < 0 )
    {
        perror( "bind" );
        exit( EXIT_FAILURE );
    }

    cout << "=====Parameter=====" << endl;
    cout << "The RTT delay = 200 ms" << endl;
    cout << "The threshold = 65535 bytes" << endl;
    cout << "The MSS = 512 bytes" << endl;
    cout << "The buffer size = 10240 bytes" << endl;
    cout << "Server's IP is 127.0.0.1 " << endl;
    cout << "Server is listening on port 10250" << endl;
    cout << "===============" << endl;
    cout << "Listening for client..." << endl;

    Packet pktThreeShakeRcv;
    ServerThreeWayHandShake( sockFd, currentSeqnum, clientIP, clientPort, clientAddr, pktThreeShakeRcv );
    curRcvSeqnum = pktThreeShakeRcv.seqNum;

    Packet pktTransAck;
    pktTransAck.seqNum = curRcvSeqnum;
    pktTransAck.rcvWin = pktThreeShakeRcv.rcvWin;
    serverConAvoid( sockFd, currentSeqnum, clientPort, clientAddr, pktTransAck );
    curRcvSeqnum = pktTransAck.seqNum;

    Packet pktFourShake;
    ServerFourWayHandShake( sockFd, currentSeqnum, curRcvSeqnum, clientIP, clientPort, clientAddr, pktFourShake );
    curRcvSeqnum = pktFourShake.seqNum;

    return EXIT_SUCCESS;
}
