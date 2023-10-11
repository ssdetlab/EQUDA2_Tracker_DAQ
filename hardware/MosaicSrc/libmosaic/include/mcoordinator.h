/*
 * Copyright (C) 2018
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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2018.
 *
 */

#ifndef MCOORDINATOR_H
#define MCOORDINATOR_H

#include "mwbbslave.h"
#include <stdint.h>
#include <string>

class MCoordinator : public MWbbSlave {
public:
  typedef enum mode_e { Alone = 0, Master = 1, Slave = 2 } mode_t;

public:
  MCoordinator(WishboneBus *wbbPtr, uint32_t baseAddress);
  void        addEnableExtClock(bool en);
  void        addSetMode(mode_t mode);
  void        setMode(mode_t mode);
  std::string dumpRegisters();

private: // WBB Slave registers map
  enum regAddress_e {
    regCfg = 0, // enable external trigger
  };

  enum cfgBits_e { ALONE = (1 << 0), MASTER = (1 << 1), EXT_CLK = (1 << 2) };
};

#endif // MCOORDINATOR_H
