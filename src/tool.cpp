/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-11 00:06
 * Last Modified:  2016-06-11 00:18
 * Filename:       tool.cpp
 * Purpose:        homework
 */

#include<iostream>
#include <arpa/inet.h>
#include "tool.h"
using namespace std;

string getIpStr( const void *src )
{
    char ip[INET_ADDRSTRLEN];
    inet_ntop( AF_INET, src, ip, INET_ADDRSTRLEN );
    return string( ip );
}
