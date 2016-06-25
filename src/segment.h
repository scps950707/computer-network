/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:57
 * Last Modified:  2016-06-12 21:52
 * Filename:       segment.h
 * Purpose:        homework
 */

#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#include <stdint.h>
#include "para.h"

typedef struct tmp
{
    uint16_t sourcePort;
    uint16_t destPort;
    uint32_t pktSeqNum;
    uint32_t pktAckNum;
    uint32_t dataPktSeqNum;
    uint32_t dataPktAckNum;
    bool URG;
    bool ACK;
    bool PSH;
    bool RST;
    bool SYN;
    bool FIN;
    bool transEnd;
    uint16_t rwnd;
    uint16_t checkSum;
    uint16_t urgDataPtr;
    uint16_t dataSize;
    uint32_t options;
    uint32_t sackBuffer[6];
    int sackSize;
    char appData[MSS];
    tmp()
    {
        sourcePort = 0;
        destPort = 0;
        pktSeqNum = 0;
        pktAckNum = 0;
        URG = false;
        ACK = false;
        PSH = false;
        RST = false;
        SYN = false;
        FIN = false;
        transEnd = false;
        rwnd = 0;
        checkSum = 0;
        urgDataPtr = 0;
        options = 0;
        dataSize = 0;
    }
    tmp( uint16_t sourcePort, uint16_t destPort, uint32_t pktSeqNum, uint32_t pktAckNum )
        : sourcePort( sourcePort ), destPort( destPort ), pktSeqNum( pktSeqNum ), pktAckNum( pktAckNum )
    {
        URG = false;
        ACK = false;
        PSH = false;
        RST = false;
        SYN = false;
        FIN = false;
        transEnd = false;
        dataPktSeqNum = 0;
        dataPktAckNum = 0;
        rwnd = 0;
        checkSum = 0;
        urgDataPtr = 0;
        options = 0;
        dataSize = 0;
    }
}
Packet;

#endif
