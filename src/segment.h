/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:57
 * Last Modified:  2016-06-11 01:11
 * Filename:       segment.h
 * Purpose:        homework
 */

#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#include "stdint.h"

typedef struct tmp
{
    uint16_t sourcePort;
    uint16_t destPort;
    uint32_t seqNum;
    uint32_t ackNum;
    bool URG;
    bool ACK;
    bool PSH;
    bool RST;
    bool SYN;
    bool FIN;
    uint16_t recvWindowl;
    uint16_t checkSum;
    uint16_t urgDataPtr;
    uint32_t options;
    char appData[484];
    tmp()
    {
        sourcePort = 0;
        destPort = 0;
        seqNum = 0;
        ackNum = 0;
        URG = false;
        ACK = false;
        PSH = false;
        RST = false;
        SYN = false;
        FIN = false;
        recvWindowl = 0;
        checkSum = 0;
        urgDataPtr = 0;
        options = 0;
    }
    tmp( uint16_t sourcePort, uint16_t destPort, uint32_t seqNum, uint32_t ackNum )
        : sourcePort( sourcePort ), destPort( destPort ), seqNum( seqNum ), ackNum( ackNum )
    {
        URG = false;
        ACK = false;
        PSH = false;
        RST = false;
        SYN = false;
        FIN = false;
        recvWindowl = 0;
        checkSum = 0;
        urgDataPtr = 0;
        options = 0;
    }
} __attribute__( ( packed ) )
Packet;

#endif