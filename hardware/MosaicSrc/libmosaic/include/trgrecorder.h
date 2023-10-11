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

#ifndef TRGRECORDER_H
#define TRGRECORDER_H

#include "mwbbslave.h"
#include <stdint.h>
#include <string>

class TrgRecorder : public MWbbSlave {
public:
  TrgRecorder(WishboneBus *wbbPtr, uint32_t baseAddress);
  void        addEnable(bool en);
  std::string dumpRegisters();

private: // WBB Slave registers map
  enum regAddress_e {
    regControl = 0 // Control register
  };

  enum controlBits_e { CONTROL_ENABLE = (1 << 0) };
};

#endif // TRGRECORDER_H
