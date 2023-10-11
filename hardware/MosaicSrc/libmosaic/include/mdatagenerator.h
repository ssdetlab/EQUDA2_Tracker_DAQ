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

#ifndef MDATAGENERATOR_H
#define MDATAGENERATOR_H

#include "mwbbslave.h"
#include <stdint.h>

class MDataGenerator : public MWbbSlave {
public:
  MDataGenerator();
  MDataGenerator(WishboneBus *wbbPtr, uint32_t baseAddress);
  void setup(uint32_t evSize, uint32_t evDelay, bool on = true);
  void getSetup(uint32_t *evSize, uint32_t *evDelay, bool *on);
  void setOnOff(bool on);
  void start() { return setOnOff(true); }
  void stop() { return setOnOff(false); }

private: // WBB Slave registers map
  enum regAddress_e {
    regModeOn     = 0, // Run control register
    regEventSize  = 1, // Event size
    regEventDelay = 2  // delay between events
  };

  enum modeOn_e {
    MODEON_ON   = (1 << 0),
    MODEON_MODE = (1 << 1) // NOT implemented
  };
};

#endif // MDATAGENERATOR_H
