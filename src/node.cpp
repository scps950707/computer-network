/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2017-06-11 15:43
 * Last Modified:  2017-06-11 21:10
 * Filename:       node.cpp
 */


#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "client.h"
#include "tool.h"
#include "shake.h"
#include "slow.h"
#include "nat.h"
#include "delay.h"
#include "conavoid.h"
#include "fastretrans.h"
#include "fastrecovery.h"
#include "sack.h"
using namespace std;

int main( int argc, char *argv[] )
{
#ifdef __USESRAND__
    srand( time( NULL ) );
#endif
    if ( argc != 3 )
    {
        cout << "./node [IP] [PORT]";
    }

    pid_t pid = fork();

    if ( pid == 0 ) //child be server
    {
        struct sockaddr_in serverAddr;
        struct sockaddr_in clientAddr;
        int sockFd;
#ifdef __SEQSTATIC__
        int curPktSeqNum = 3439;
#else
        int curPktSeqNum = rand() % 10000 + 1;
#endif
        int curRcvpktSeqNum;
        string clientIP;
        uint16_t clientPort;

        bzero( &serverAddr, sizeof( serverAddr ) );
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr =  inet_addr( argv[1] );
        serverAddr.sin_port = htons( atoi( argv[2] ) );

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
        cout << "Server's IP is " << string( inet_ntoa( serverAddr.sin_addr ) ) << endl;
        cout << "Server is listening on port " << string( argv[2] ) << endl;
        cout << "===============" << endl;
        cout << "Listening for client..." << endl;

        Packet pktThreeShakeRcv;
        ServerThreeWayHandShake( sockFd, curPktSeqNum, clientIP, clientPort, clientAddr, pktThreeShakeRcv );
        curRcvpktSeqNum = pktThreeShakeRcv.pktSeqNum;

        Packet pktDataAck;
        pktDataAck.pktSeqNum = curRcvpktSeqNum;
        pktDataAck.rwnd = pktThreeShakeRcv.rwnd;
#ifdef __SLOW__
        serverSlowStart( sockFd, curPktSeqNum, clientPort, clientAddr, pktDataAck );
#elif __DELAY__
        serverDelayAck( sockFd, curPktSeqNum, clientPort, clientAddr, pktDataAck );
#elif __CONAVOID__
        serverConAvoid( sockFd, curPktSeqNum, clientPort, clientAddr, pktDataAck );
#elif __FASTRE__
        serverFastReTrans( sockFd, curPktSeqNum, clientPort, clientAddr, pktDataAck );
#elif __FASTCOV__
        serverFastRecovery( sockFd, curPktSeqNum, clientPort, clientAddr, pktDataAck );
#elif __SACK__
        serverSack( sockFd, curPktSeqNum, clientPort, clientAddr, pktDataAck );
#endif
        curRcvpktSeqNum = pktDataAck.pktSeqNum;

        Packet pktFourShake;
        ServerFourWayHandShake( sockFd, curPktSeqNum, curRcvpktSeqNum, clientIP, clientPort, clientAddr, pktFourShake );
        curRcvpktSeqNum = pktFourShake.pktSeqNum;

        close ( sockFd );
    }
    else//parent be client
    {
        int status = 0;
        cout << "[ClientFunction]Please input node [IP] [PORT] you want to connect" << endl;
        fd_set rfds;
        struct timeval tv;
        int retval;
        FD_ZERO( &rfds );
        FD_SET( 0, &rfds );
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        retval = select( 1, &rfds, NULL, NULL, &tv );
        if ( retval == -1 )
        {
            perror( "select()" );
        }
        else if ( retval )
        {
            kill( pid, SIGTERM ); // kill child process server
            struct sockaddr_in clientAddr;
            struct sockaddr_in serverAddr;
            int sockFd;
#ifdef __SEQSTATIC__
            int curPktSeqNum = 9230;
#else
            int curPktSeqNum = rand() % 10000 + 1;
#endif
            char ip[20], port[10], line[30];
            read( 0, line, 30 );
            sscanf( line, "%s %s", ip, port );
            string serverIP( ip );
            uint16_t serverPort = atoi( port );

            map<string, string> table;
            createNatTable( table );
            string defaultIP = "192.168.0.";
            defaultIP += ( char )( rand() % 4 + 2 + '0' );
            bzero( &clientAddr, sizeof( clientAddr ) );
            clientAddr.sin_family = AF_INET;
            clientAddr.sin_addr.s_addr = inet_addr( defaultIP.c_str() );
            clientAddr.sin_port = htons( CLIENT_PORT );

            cout << "client IP:" << string( inet_ntoa( clientAddr.sin_addr ) ) << endl;
            cout << "translate IP to " << table[defaultIP] << endl;
            clientAddr.sin_addr.s_addr = inet_addr( table[defaultIP].c_str() );

            bzero( &serverAddr, sizeof( serverAddr ) );
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons( serverPort );
            if ( inet_pton( AF_INET, serverIP.c_str(), &serverAddr.sin_addr ) <= 0 )
            {
                cout << "IP error " << endl;
                exit( EXIT_FAILURE );
            }

            if ( ( sockFd = socket( clientAddr.sin_family, SOCK_DGRAM, 0 ) ) < 0 )
            {
                perror ( "socket" );
                exit( EXIT_FAILURE );
            }

            if ( bind( sockFd, ( struct sockaddr * )&clientAddr, sizeof( clientAddr ) ) < 0 )
            {
                perror( "bind" );
                exit( EXIT_FAILURE );
            }

            ClientThreeWayHandShake( sockFd, curPktSeqNum, serverIP, serverPort, serverAddr );

#ifdef __SLOW__
            clientSlowStart( sockFd, curPktSeqNum, serverIP, serverPort, serverAddr );
#elif __DELAY__
            clientDelayAck( sockFd, curPktSeqNum, serverIP, serverPort, serverAddr );
#elif __CONAVOID__
            clientConAvoid( sockFd, curPktSeqNum, serverIP, serverPort, serverAddr );
#elif __FASTRE__
            clientFastReTrans( sockFd, curPktSeqNum, serverIP, serverPort, serverAddr );
#elif __FASTCOV__
            clientFastRecovery( sockFd, curPktSeqNum, serverIP, serverPort, serverAddr );
#elif __SACK__
            clientSack( sockFd, curPktSeqNum, serverIP, serverPort, serverAddr );
#endif

            ClientFourWayHandShake( sockFd, curPktSeqNum, serverIP, serverPort, serverAddr );

            close ( sockFd );
        }
        else
        {
            waitpid( -1, &status, 0 );
        }
    }
    return EXIT_SUCCESS;
}
