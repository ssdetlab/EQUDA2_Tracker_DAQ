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

#ifndef MTRIGGERCONTROL_H
#define MTRIGGERCONTROL_H

#include "mwbbslave.h"
#include <stdint.h>
#include <string>

class MTriggerControl : public MWbbSlave {
public:
  MTriggerControl(WishboneBus *wbbPtr, uint32_t baseAddress);
  void        addEnableExtTrigger(bool en, bool levelSensitive = 0);
  void        getTriggerCounter(uint32_t *counter);
  std::string dumpRegisters();

private: // WBB Slave registers map
  enum regAddress_e {
    regCfg            = 0, // enable external trigger
    regTriggerCounter = 1, // Trigger counter - Read only. Reset by RUN signal
    regTimeL          = 2, // TIMER from first trigger bits 31:0 - Read only. Reset by RUN signal
    regTimeH          = 3, // TIMER from first trigger bits 63:32 - Read only. Reset by RUN signal
  };

  enum cfgBits_e { EN_EXT_TRIGGER = (1 << 0), EXT_TRG_LEVEL = (1 << 1) };
};

#endif // MTRIGGERCONTROL_H
