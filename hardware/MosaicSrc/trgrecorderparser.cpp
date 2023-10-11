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
#include "trgrecorderparser.h"
#include "mboard.h"
#include "mexception.h"
#include <iostream>
#include <sstream>
#include <stdio.h>

#define PLATFORM_IS_LITTLE_ENDIAN

TrgRecorderParser::TrgRecorderParser() { verbose = false; }

void TrgRecorderParser::flush() {}

uint32_t TrgRecorderParser::buf2uint32(unsigned char *buf)
{
#ifdef PLATFORM_IS_LITTLE_ENDIAN
  return (*(uint32_t *)buf) & 0xffffffff;
#else
  uint32_t d;

  d = *buf++;
  d |= (*buf++) << 8;
  d |= (*buf++) << 16;
  d |= (*buf++) << 24;

  return d;
#endif
}

uint64_t TrgRecorderParser::buf2uint64(unsigned char *buf)
{
#ifdef PLATFORM_IS_LITTLE_ENDIAN
  return (*(uint64_t *)buf) & 0xffffffffffffffff;
#else
  uint64_t d;

  d = *buf++;
  d |= (*buf++) << 8;
  d |= (*buf++) << 16;
  d |= (*buf++) << 24;
  d |= (*buf++) << 32;
  d |= (*buf++) << 40;
  d |= (*buf++) << 48;
  d |= (*buf++) << 56;

  return d;
#endif
}

// parse the data starting from begin of buffer
long TrgRecorderParser::parse(int numClosed)
{
  unsigned char *dBuffer = (unsigned char *)&dataBuffer[0];
  unsigned char *p       = dBuffer;
  long           evSize  = TRIGGERDATA_SIZE;
  uint32_t       trgNum;
  uint64_t       trgTime;

  // check avalaible data size
  if (dataBufferUsed < numClosed * evSize) {
    std::stringstream sstm;
    sstm << "Parser called with " << numClosed << " closed events of " << evSize
         << " bytes but datasize is only " << dataBufferUsed << " bytes";
    throw MDataParserError(sstm.str());
  }

  while (numClosed) {
    trgNum  = buf2uint32(p);
    trgTime = buf2uint64(p + 4);

    if (verbose) printf("Trigger %d @ %llu\n", trgNum, static_cast<unsigned long long>(trgTime));

    p += evSize;
    numClosed--;
  }

  return (p - dBuffer);
}


//
// Read only one trigger time stamp
// return the size of data frame
int TrgRecorderParser::ReadTriggerInfo(uint32_t &trgNum, uint64_t &trgTime)
{
  unsigned char *dBuffer = (unsigned char *)&dataBuffer[0];
  unsigned char *p       = dBuffer;
  long           evSize  = TRIGGERDATA_SIZE;

  if (numClosedData == 0) return 0;

  // check avalaible data size
  if (dataBufferUsed < numClosedData * evSize) {
    std::stringstream sstm;
    sstm << "Parser called with " << numClosedData << " closed events of " << evSize
         << " bytes but datasize is only " << dataBufferUsed << " bytes";
    throw MDataParserError(sstm.str());
  }

  trgNum  = buf2uint32(p);
  trgTime = buf2uint64(p + 4);

  // move unused bytes to the begin of buffer
  size_t bytesToMove = dataBufferUsed - evSize;
  if (bytesToMove > 0) memmove(&dataBuffer[0], &dataBuffer[evSize], bytesToMove);
  dataBufferUsed -= evSize;
  numClosedData--;

  if (verbose) printf("Trigger %d @ %llu\n", trgNum, static_cast<unsigned long long>(trgTime));

  return 1;
}
