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
#include "mdatasave.h"
#include "mboard.h"
#include "mexception.h"
#include <sstream>
#include <stdio.h>

MDataSave::MDataSave()
{
  eventSize    = 0;
  saveFunction = NULL;
}

void MDataSave::flush() {}

// parse the data starting from begin of buffer
long MDataSave::parse(int numClosed)
{
  unsigned char *p = (unsigned char *)&dataBuffer[0];

  // check avalaible data size
  if (dataBufferUsed < numClosed * eventSize) {
    std::stringstream sstm;
    sstm << "Parser called with " << numClosed << " closed events of " << eventSize
         << " bytes but datasize is only " << dataBufferUsed << " bytes";
    throw MDataParserError(sstm.str());
  }

  // save the data
  for (int i = 0; i < numClosed; i++) {
    if (saveFunction != NULL) {
      saveFunction(NULL, 0, (char *)p, eventSize);
    }
    p += eventSize;
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
