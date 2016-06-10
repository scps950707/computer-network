/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-11 00:00
 * Last Modified:  2016-06-11 02:48
 * Filename:       tools.h
 * Purpose:        homework
 */

using namespace std;
#ifndef __TOOLS_H__
#define __TOOLS_H__

#include<iostream>
string getIpStr( const void *src );
void sendPktMsg( string type, string ip, int port );

#endif
