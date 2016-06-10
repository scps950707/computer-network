/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:10
 * Last Modified:  2016-06-10 22:56
 * Filename:       server.cpp
 * Purpose:        homework
 */

#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "server.h"
#include "segment.h"
using namespace std;

int main()
{

    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    int sockFd;
    socklen_t cliSize = sizeof( clientAddr );

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
    cout << "=====Start the three-way handshake=====" << endl;

    Packet buf;
    while ( recvfrom( sockFd , &buf, MSS, 0, ( struct sockaddr * )&clientAddr, &cliSize ) > 0 )
    {
        if ( buf.SYN == true && buf.ACK == false )
        {
            char ip[INET_ADDRSTRLEN];
            inet_ntop( AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN );
            cout << "Receive a packet(SYN) from" <<  ip << " : " << buf.sourcePort << endl;
            cout << "    Receive a packet (seq_num = " << buf.seqNum << ", ack_num =" << buf.ackNum << ")" << endl;
        }
        else if ( buf.SYN == false && buf.ACK == true )
        {
            char ip[INET_ADDRSTRLEN];
            inet_ntop( AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN );
            cout << "Receive a packet(ACK) from" <<  ip << " : " << buf.sourcePort << endl;
            cout << "    Receive a packet (seq_num = " << buf.seqNum << ", ack_num =" << buf.ackNum << ")" << endl;
            cout << "=====Complete the three-way handshake=====" << endl;
        }
    }

    return 0;
}
