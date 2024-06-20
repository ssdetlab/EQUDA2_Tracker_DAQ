/*
 * Copyright (C) 2017
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * ====================================================
 *     __  __   __  _____  __   __
 *    / / /  | / / / ___/ /  | / / SEZIONE di BARI
 *   / / / | |/ / / /_   / | |/ /
 *  / / / /| / / / __/  / /| / /
 * /_/ /_/ |__/ /_/    /_/ |__/
 *
 * ====================================================
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2017.
 *
 */
#include "TAlpideDataParser.h"
#include "mboard.h"
#include "mexception.h"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>

TAlpideDataParser::TAlpideDataParser() {}

// fast parse of input frame
// return the size of the data for the event pointed by dBuffer.
long TAlpideDataParser::checkEvent(
    unsigned char *dBuffer, 
    unsigned char *evFlagsPtr) {
        unsigned char *p = dBuffer;
        unsigned char  h;
        int            d;

        for (int closed = 0; !closed;) {
            if (p - dBuffer > dataBufferUsed) {
                throw MDataParserError("Try to parse more bytes than received size");
            }
        
            h = *p++;
            if ((h >> DSHIFT_CHIP_EMPTY) == DCODE_CHIP_EMPTY) {
                closed = 1;
                p++;            
                // Check for the TLU payload
                if ((*p & 0xff) == DCODE_TLU_HEADER && (*(p + 3) == DCODE_TLU_TRAILER)) {
                    p += 5; ///< take MOSAIC trailer into account
                } 
            }
            else if ((h >> DSHIFT_CHIP_HEADER) == DCODE_CHIP_HEADER) {
                p++;
            }
            else if ((h >> DSHIFT_CHIP_TRAILER) == DCODE_CHIP_TRAILER) {
                closed = 1;
                *evFlagsPtr = (h & 0x0f);
            
                // Check for the TLU payload
                if ((*p & 0xff) == DCODE_TLU_HEADER && (*(p + 3) == DCODE_TLU_TRAILER)) {
                    p += 5; ///< take MOSAIC trailer into account
                }
                else {
                    p++; ///< take MOSAIC trailer into account
                }
            }
            else if ((h >> DSHIFT_REGION_HEADER) == DCODE_REGION_HEADER) {
            }
            else if ((h >> DSHIFT_DATA_SHORT) == DCODE_DATA_SHORT) {
                p++;
            }
            else if ((h >> DSHIFT_DATA_LONG) == DCODE_DATA_LONG) {
                p += 2;
            }
            else {
                d = h;
                cout << " Unknow data header: " << std::hex << d << endl;
            }
        }
        
        return (p - dBuffer);
}

// parse all data starting from begin of buffer
long TAlpideDataParser::parse(int numClosed) {
    unsigned char *dBuffer = (unsigned char *)&dataBuffer[0];
    unsigned char *p       = dBuffer;
    long           evSize;
    unsigned char  evFlags;
    
    while (numClosed) {
        evSize = checkEvent(p, &evFlags);
        p += evSize;
        numClosed--;
    }
    
    return (p - dBuffer);
}

//
// Read only one frame of data
// return the size of data frame
//        0 := No data
//       -1 := Event discharged
//
int TAlpideDataParser::ReadEventData(int &nBytes, unsigned char *buffer) {
    unsigned char *dBuffer = (unsigned char *)&dataBuffer[0];
    unsigned char *p       = dBuffer;
    long           evSize;
    long           retValue;
    unsigned char  evFlags;

    if (numClosedData == 0) return 0;
    evSize = checkEvent(p, &evFlags);
    
    if (evSize + MOSAIC_HEADER_SIZE < MAX_EVENT_SIZE) {
        // copy the block header to the user buffer
        memcpy(buffer, blockHeader, MOSAIC_HEADER_SIZE);
        // copy data to user buffer
        memcpy(buffer + MOSAIC_HEADER_SIZE, dBuffer, evSize);
        nBytes   = MOSAIC_HEADER_SIZE + evSize;
        retValue = evSize;
    }
    else {
        cerr << "One event exceeds the maximum buffer dimension (" << (MOSAIC_HEADER_SIZE + evSize)
            << " > " << MAX_EVENT_SIZE << ") Event discharged !" << endl;
        nBytes   = 0;
        retValue = -1;
    }
    
    // move unused bytes to the begin of buffer
    size_t bytesToMove = dataBufferUsed - evSize;
    if (bytesToMove > 0) memmove(&dataBuffer[0], &dataBuffer[evSize], bytesToMove);
    dataBufferUsed -= evSize;
    numClosedData--;
    return retValue;
}
