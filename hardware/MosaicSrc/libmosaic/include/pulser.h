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

#ifndef PULSER_H
#define PULSER_H

#include "mwbbslave.h"
#include <stdint.h>
#include <string>

class Pulser : public MWbbSlave {
public:
  Pulser();
  Pulser(WishboneBus *wbbPtr, uint32_t baseAddress);
  ~Pulser();
  void        setBusAddress(WishboneBus *wbbPtr, uint32_t baseAddress);
  void        setConfig(uint32_t triggerDelay, uint32_t pulseDelay,
                        uint32_t opMode = OPMODE_ENPLS_BIT | OPMODE_ENTRG_BIT);
  void        getConfig(uint32_t *triggerDelay, uint32_t *pulseDelay, uint32_t *opMode);
  void        run(uint32_t numPulses);
  void        getStatus(uint32_t *numPulses);
  std::string dumpRegisters();

private: // WBB Slave registers map
  enum regAddress_e {
    regOpMode       = 0,
    regTriggerDelay = 1,
    regPulseDelay   = 2,
    regNumPulses    = 3,
    regStatus       = 7
  };

public:
  enum readFlagsBits_e {
    OPMODE_ENPLS_BIT    = (1 << 0),
    OPMODE_ENTRG_BIT    = (1 << 1),
    OPMODE_ENEXTTRG_BIT = (1 << 2)
  };
};

#endif // PULSER_H
