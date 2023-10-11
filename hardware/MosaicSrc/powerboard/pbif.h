/*
 * Copyright (C) 2015
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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2015.
 *
 */

#ifndef PBIF_H
#define PBIF_H

#include "i2cbus.h"
#include "ipbusudp.h"
#include "mwbb.h"
#include "powerboard.h"
#include <stdint.h>

#define DEFAULT_PACKET_SIZE 1400
#define DEFAULT_UDP_PORT 2000

class PBif {
public:
  PBif();
  PBif(char *IPaddr, int UDPport = DEFAULT_UDP_PORT);
  void setIPaddress(const char *IPaddr, int UDPport = DEFAULT_UDP_PORT);
  ~PBif();

private:
  void init();

private:
  // extend WBB address definitions in mwbb.h
  enum baseAddress_e { add_i2cMaster = (5 << 24), add_i2cAux = (29 << 24) };

  std::string IPaddress;
  IPbusUDP *  mIPbus;
  I2Cbus *    i2cBus;
  I2Cbus *    i2cBusAux;

public:
  powerboard *pb;
};

#endif // PBIF_H
