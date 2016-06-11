/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-11 22:06
 * Last Modified:  2016-06-11 22:35
 * Filename:       nat.cpp
 * Purpose:        hw
 */

#include "nat.h"
#include <iostream>
void createNatTable( map<string, string> &table )
{
    table.insert( pair<string, string>( "192.168.0.2", "192.168.0.1" ) );
    table.insert( pair<string, string>( "192.168.0.3", "192.168.0.1" ) );
    table.insert( pair<string, string>( "192.168.0.4", "192.168.0.1" ) );
    table.insert( pair<string, string>( "192.168.0.5", "192.168.0.1" ) );
    cout << "------------NAT translation table-----------" << endl;
    cout << "WAN side addr      |     LAN side addr" << endl;
    for ( map<string, string>::iterator it = table.begin(); it != table.end(); it++ )
    {
        cout << it->second << " :10250    " << it->first << " :10250    " << endl;
    }
}
