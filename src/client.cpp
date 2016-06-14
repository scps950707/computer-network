/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:09
 * Last Modified:  2016-06-12 02:32
 * Filename:       client.cpp
 * Purpose:        homework
 */

#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
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
        cout << "./Prog [server IP] [PORT]";
    }
    struct sockaddr_in clientAddr;
    struct sockaddr_in serverAddr;
    int sockFd;
#ifdef __SEQSTATIC__
    int currentSeqnum = 9230;
#else
    int currentSeqnum = rand() % 10000 + 1;
#endif
    string serverIP( argv[1] );
    uint16_t serverPort = atoi( argv[2] );

    map<string, string> table;
    createNatTable( table );
    string defaultIP = "192.168.0.";
    defaultIP += ( char )( rand() % 4 + 2 + '0' ) ;
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
    if ( inet_pton( AF_INET, argv[1], &serverAddr.sin_addr ) <= 0 )
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

    ClientThreeWayHandShake( sockFd, currentSeqnum, serverIP, serverPort, serverAddr );

#ifdef __SLOW__
    clientSlowStart( sockFd, currentSeqnum, serverIP, serverPort, serverAddr );
#elif __DELAY__
    clientDelayAck( sockFd, currentSeqnum, serverIP, serverPort, serverAddr );
#elif __CONAVOID__
    clientConAvoid( sockFd, currentSeqnum, serverIP, serverPort, serverAddr );
#elif __FASTRE__
    clientFastReTrans( sockFd, currentSeqnum, serverIP, serverPort, serverAddr );
#elif __FASTCOV__
    clientFastRecovery( sockFd, currentSeqnum, serverIP, serverPort, serverAddr );
#elif __SACK__
    clientSack( sockFd, currentSeqnum, serverIP, serverPort, serverAddr );
#endif

    ClientFourWayHandShake( sockFd, currentSeqnum, serverIP, serverPort, serverAddr );

    close ( sockFd );

    return EXIT_SUCCESS;
}
