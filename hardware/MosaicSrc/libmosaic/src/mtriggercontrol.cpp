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
#include "mtriggercontrol.h"
#include "mexception.h"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

MTriggerControl::MTriggerControl(WishboneBus *wbbPtr, uint32_t baseAdd) : MWbbSlave(wbbPtr, baseAdd)
{
}

void MTriggerControl::addEnableExtTrigger(bool en, bool levelSensitive)
{
  uint32_t tmp;

  tmp = en ? EN_EXT_TRIGGER : 0;
  tmp |= levelSensitive ? EXT_TRG_LEVEL : 0;

  wbb->addWrite(baseAddress + regCfg, tmp);
}

void MTriggerControl::getTriggerCounter(uint32_t *counter)
{
  wbb->addRead(baseAddress + regTriggerCounter, counter);
  wbb->execute();
}

std::string MTriggerControl::dumpRegisters()
{
  if (!wbb) throw MIPBusUDPError("No IPBus configured");

  regAddress_e addrs[] = {regCfg, regTriggerCounter, regTimeL, regTimeH};
  uint32_t     nAddrs  = sizeof(addrs) / sizeof(regAddress_e);

  std::stringstream ss;
  ss << std::hex;

  for (uint32_t iAddr = 0; iAddr < nAddrs; ++iAddr) {
    uint32_t result = 0xDEADBEEF;
    try {
      wbb->addRead(baseAddress + addrs[iAddr], &result);
      execute();
    }
    catch (...) {
      std::cerr << "MTriggerControl read error: address 0x" << std::hex
                << baseAddress + addrs[iAddr] << " (0x" << addrs[iAddr] << ")!" << std::dec
                << std::endl;
    };

    ss << "0x" << addrs[iAddr] << "\t0x" << result << std::endl;
  }

  ss << std::endl;

  return ss.str();
}
