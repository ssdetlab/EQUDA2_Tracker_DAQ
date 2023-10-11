/*
 * Copyright (C) 2014
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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2014.
 *
 */
#include "genconsumer.h"
#include "mboard.h"
#include "mexception.h"
#include <sstream>
#include <stdio.h>

GenConsumer::GenConsumer()
{
  eventSize    = 0;
  expectedData = 0;
}

void GenConsumer::flush() { expectedData = 0; }

// parse the data starting from begin of buffer
long GenConsumer::parse(int numClosed)
{
  uint32_t       d;
  unsigned char *p = (unsigned char *)&dataBuffer[0];

  // printf("Called GenConsumer::parse ne:%d from buffer at 0x%08x\n", numClosed, (unsigned long)
  // p);

  // check avalaible data size
  if (dataBufferUsed < numClosed * eventSize) {
    std::stringstream sstm;
    sstm << "Parser called with " << numClosed << " closed events of " << eventSize
         << " bytes but datasize is only " << dataBufferUsed << " bytes";
    throw MDataParserError(sstm.str());
  }

  // check data
  for (int i = 0; i < numClosed; i++) {
    for (int j = 0; j < eventSize; j += 4) {
      d = MBoard::buf2ui(p);
      if (d != expectedData) {
        //				printf("ERROR: GenConsumer::parse found:0x%08x exp:0x%08x at
        // 0x%08lx\n", d, expectedData, (unsigned long) p);
        //				printf("ERROR: GenConsumer::parse numClosed:%d eventSize:%d
        // j:%d\n", numClosed, eventSize, j);
        throw MDataParserError("Data mismatch");
      }
      p += 4;
      expectedData++;
    }
  }

  if ((dataBufferUsed - numClosed * eventSize) > eventSize) {
    std::stringstream sstm;
    sstm << "after parsing buffer content lenght (" << (dataBufferUsed - numClosed * eventSize)
         << ") is greater then eventSize(" << eventSize << ")";
    throw MDataParserError(sstm.str());
    // printf("ERROR: GenConsumer::parse after parsing buffer content lenght (%ld) is greater then
    // eventSize(%ld)\n",
    //					dataBufferUsed - numClosed*eventSize, eventSize);
  }
  return numClosed * eventSize;
}
