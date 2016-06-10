/*
 * Author:         scps950707
 * Email:          scps950707@gmail.com
 * Created:        2016-06-10 16:57
 * Last Modified:  2016-06-10 18:26
 * Filename:       segment.h
 * Purpose:        homework
 */

#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#include "stdint.h"

typedef struct __attribute__( ( packed ) )
{
    uint16_t sourcePort = 0;
    uint16_t destPort = 0;
    uint32_t seqNum = 0;
    uint32_t ackNum = 0;
    bool URG = false;
    bool ACK = false;
    bool PSH = false;
    bool RST = false;
    bool SYN = false;
    bool FIN = false;
    uint16_t recvWindowl = 0;
    uint16_t checkSum = 0;
    uint16_t urgDataPtr = 0;
    uint32_t options = 0;
    char appData[484];
}
Packet;

#endif
