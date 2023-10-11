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
#include "pbif.h"
#include "mexception.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

PBif::PBif() { init(); }

PBif::PBif(char *IPaddr, int port)
{
  init();
  setIPaddress(IPaddr, port);
}

void PBif::setIPaddress(const char *IPaddr, int port)
{
  IPaddress = IPaddr;
  mIPbus->setIPaddress(IPaddr, port);
}

void PBif::init()
{
  //
  // Mosaic board IPBus
  //
  mIPbus = new IPbusUDP();

  // I2C master (WBB slave) and connected peripherals
  i2cBus    = new I2Cbus(mIPbus, add_i2cMaster);
  i2cBusAux = new I2Cbus(mIPbus, add_i2cAux);
  pb        = new powerboard(i2cBus, i2cBusAux);
}

PBif::~PBif()
{
  // delete objects in creation reverse order
  delete i2cBusAux;
  delete i2cBus;
  delete mIPbus;
}
