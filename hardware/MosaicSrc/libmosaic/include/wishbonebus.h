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

#ifndef WISHBONEBUS_H
#define WISHBONEBUS_H

#include <stdint.h>

class WishboneBus {
public:
  virtual ~WishboneBus() {}
  virtual void addWrite(uint32_t address, uint32_t data)              = 0;
  virtual void addWrite(int size, uint32_t address, uint32_t *data)   = 0;
  virtual void addNIWrite(int size, uint32_t address, uint32_t *data) = 0;
  virtual void addRead(int size, uint32_t address, uint32_t *data)    = 0;
  void         addRead(uint32_t address, uint32_t *data) { addRead(1, address, data); }
  virtual void addNIRead(int size, uint32_t address, uint32_t *data)                           = 0;
  virtual void addRMWbits(uint32_t address, uint32_t mask, uint32_t data, uint32_t *rData = 0) = 0;
  virtual void addRMWsum(uint32_t address, uint32_t data, uint32_t *rData = 0)                 = 0;
  virtual int  getBufferSize()                                                                 = 0;
  virtual void execute()                                                                       = 0;
};

#endif // WISHBONEBUS_H
