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

#ifndef MRUNCONTROL_H
#define MRUNCONTROL_H

#include "mwbbslave.h"
#include <stdint.h>
#include <string>

class MRunControl : public MWbbSlave {
public:
  MRunControl(WishboneBus *wbbPtr, uint32_t baseAddress);
  void        getErrors(uint32_t *errors, bool clear = true);
  void        clearErrors();
  void        setConfigReg(uint32_t d);
  void        getConfigReg(uint32_t *d);
  void        rmwConfigReg(uint32_t mask, uint32_t data);
  void        setAFThreshold(uint32_t d);
  void        getAFThreshold(uint32_t *d);
  void        setLatency(uint8_t mode, uint32_t d);
  void        getLatency(uint8_t *mode, uint32_t *d);
  void        getStatus(uint32_t *st);
  void        startRun();
  void        stopRun();
  std::string dumpRegisters();

private: // WBB Slave registers map
  enum regAddress_e {
    regRunCtrl             = 0, // Run control register
    regErrorState          = 1, // Error state register
    regAlmostFullThreshold = 2, // Threshold of almost full flag for the ddr3 memory buffer
    regLatency             = 3, // Data Latency control register
    // FPGA and Temprature and reserved location
    regTemperature = 4, // NOT IMPLEMENTED
    regStatus      = 5, // Board status flags
    regReserved0   = 6,
    regReserved1   = 7,
    regConfig      = 8 // Configuration register
  };

  enum runCtrlBits_e { RUN_CTRL_RUN = (1 << 0), RUN_CTRL_PAUSE = (1 << 1) };

public:
  enum latencyMode_e { latencyModeEoe = 0, latencyModeTimeout = 1, latencyModeMemory = 2 };
};

#endif // MRUNCONTROL_H
