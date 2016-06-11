/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:09
 * Last Modified:  2016-06-11 22:35
 * Filename:       client.cpp
 * Purpose:        homework
 */

#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <map>
#include "client.h"
#include "segment.h"
#include "tool.h"
#include "para.h"
#include "shake.h"
#include "slow.h"
#include "nat.h"
using namespace std;

int main( int argc, char *argv[] )
{
    if ( argc != 3 )
    {
        cout << "./Prog [server IP] [PORT]";
    }
    srand( time( NULL ) );
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
    string defaultIP = "192.168.0.2";
    bzero( &clientAddr, sizeof( clientAddr ) );
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr(defaultIP.c_str());
    clientAddr.sin_port = htons( CLIENT_PORT );

    cout << "client IP:" <<  getIpStr( &clientAddr.sin_addr ) << endl;
    cout << "translate IP to " << table[defaultIP] << endl;
    clientAddr.sin_addr.s_addr = inet_addr(table[defaultIP].c_str());

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

    clientSlowStart( sockFd, currentSeqnum, serverIP, serverPort, serverAddr );

    ClientFourWayHandShake( sockFd, currentSeqnum, serverIP, serverPort, serverAddr );

    return EXIT_SUCCESS;
}
