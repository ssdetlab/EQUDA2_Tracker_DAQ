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
#include "mdatagenerator.h"
#include <stdio.h>
#include <stdlib.h>

MDataGenerator::MDataGenerator() {}

MDataGenerator::MDataGenerator(WishboneBus *wbbPtr, uint32_t baseAdd) : MWbbSlave(wbbPtr, baseAdd)
{
}

void MDataGenerator::setup(uint32_t evSize, uint32_t evDelay, bool on)
{
  wbb->addWrite(baseAddress + regModeOn, on ? MODEON_ON : 0);
  wbb->addWrite(baseAddress + regEventSize, evSize);
  wbb->addWrite(baseAddress + regEventDelay, evDelay);
  wbb->execute();
}

void MDataGenerator::getSetup(uint32_t *evSize, uint32_t *evDelay, bool *on)
{
  uint32_t onOff;

  wbb->addRead(baseAddress + regModeOn, &onOff);
  wbb->addRead(baseAddress + regEventSize, evSize);
  wbb->addRead(baseAddress + regEventDelay, evDelay);
  wbb->execute();
  *on = onOff & MODEON_ON;
}

void MDataGenerator::setOnOff(bool on)
{
  wbb->addWrite(baseAddress + regModeOn, on ? MODEON_ON : 0);
  wbb->execute();
}
