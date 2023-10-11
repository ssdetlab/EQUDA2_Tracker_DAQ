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
#include "mruncontrol.h"
#include "mexception.h"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

MRunControl::MRunControl(WishboneBus *wbbPtr, uint32_t baseAdd) : MWbbSlave(wbbPtr, baseAdd) {}

void MRunControl::getErrors(uint32_t *errors, bool clear)
{
  wbb->addRead(baseAddress + regErrorState, errors);        // READ error register
  if (clear) wbb->addWrite(baseAddress + regErrorState, 0); // clear it
  wbb->execute();
}

void MRunControl::clearErrors()
{
  wbb->addWrite(baseAddress + regErrorState, 0);
  wbb->execute();
}

void MRunControl::setConfigReg(uint32_t d)
{
  wbb->addWrite(baseAddress + regConfig, d);
  wbb->execute();
}

void MRunControl::getConfigReg(uint32_t *d)
{
  wbb->addRead(baseAddress + regConfig, d);
  wbb->execute();
}

void MRunControl::rmwConfigReg(uint32_t mask, uint32_t data)
{
  wbb->addRMWbits(baseAddress + regConfig, mask, data);
  wbb->execute();
}

void MRunControl::setAFThreshold(uint32_t d)
{
  wbb->addWrite(baseAddress + regAlmostFullThreshold, d);
  wbb->execute();
}

void MRunControl::getAFThreshold(uint32_t *d)
{
  wbb->addRead(baseAddress + regAlmostFullThreshold, d);
  wbb->execute();
}

void MRunControl::setLatency(uint8_t mode, uint32_t timeout)
{
  uint32_t m = mode;

  wbb->addWrite(baseAddress + regLatency, ((m & 0x03) << 30) | (timeout & 0x00ffffff));
  wbb->execute();
}

void MRunControl::getLatency(uint8_t *mode, uint32_t *timeout)
{
  uint32_t d;

  wbb->addRead(baseAddress + regLatency, &d);
  wbb->execute();

  *mode    = d >> 30;
  *timeout = d & 0x00ffffff;
}

void MRunControl::getStatus(uint32_t *st)
{
  wbb->addRead(baseAddress + regStatus, st);
  wbb->execute();
}

void MRunControl::startRun()
{
  wbb->addWrite(baseAddress + regRunCtrl, RUN_CTRL_RUN);
  wbb->execute();
}

void MRunControl::stopRun()
{
  wbb->addWrite(baseAddress + regRunCtrl, 0);
  wbb->execute();
}

std::string MRunControl::dumpRegisters()
{
  if (!wbb) throw MIPBusUDPError("No IPBus configured");

  regAddress_e addrs[] = {regRunCtrl,
                          regErrorState,
                          regAlmostFullThreshold,
                          regLatency,
                          /* regTemperature, */ regStatus,
                          /* regReserved0, regReserved1,*/ regConfig};
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
      std::cerr << "MRunControl read error: address 0x" << std::hex << baseAddress + addrs[iAddr]
                << " (0x" << addrs[iAddr] << ")!" << std::dec << std::endl;
    };

    ss << "0x" << addrs[iAddr] << "\t0x" << result << std::endl;
  }

  ss << std::endl;

  return ss.str();
}
