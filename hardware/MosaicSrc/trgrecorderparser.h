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

#ifndef TRGRECORDERPARSER_H
#define TRGRECORDERPARSER_H

#include "mdatareceiver.h"
#include <stdint.h>

#define TRIGGERDATA_SIZE 12 // 4 bytes: Trigger number. 8 bytes: Time stamp

class TrgRecorderParser : public MDataReceiver {
public:
  TrgRecorderParser();
  void flush();
  int  ReadTriggerInfo(uint32_t &trgNum, uint64_t &trgTime);
  bool hasData() { return (numClosedData != 0); }

protected:
  long parse(int numClosed);

private:
  bool verbose;

private:
  uint32_t buf2uint32(unsigned char *buf);
  uint64_t buf2uint64(unsigned char *buf);

public:
  void setVerbose(bool v) { verbose = v; }
};

#endif // TRGRECORDERPARSER_H
