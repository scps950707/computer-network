/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:09
 * Last Modified:  2016-06-10 22:58
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
#include "client.h"
#include "segment.h"
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
    int port = atoi( argv[2] );
    socklen_t serSize = sizeof( serverAddr );

    bzero( &serverAddr, sizeof( serverAddr ) );
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    clientAddr.sin_port = htons( CLIENT_PORT );

    bzero( &serverAddr, sizeof( serverAddr ) );
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons( port );
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


    Packet buf;
    buf.SYN = true;
    buf.destPort = SERVER_PORT;
    buf.sourcePort = port;
    buf.seqNum = rand() % 10000 + 1;
    sendto( sockFd, &buf, MSS, 0, ( struct sockaddr * )&serverAddr, serSize );
    return EXIT_SUCCESS;
}
