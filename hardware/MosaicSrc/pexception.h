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

#ifndef PEXCEPTION_H
#define PEXCEPTION_H

#include "mexception.h"
#include <string>

// class string;
using namespace std;

// Control interface errors
class PControlInterfaceError : public MException {
private:
  int m_controlInt;

public:
  explicit PControlInterfaceError(const string &__arg);
  void SetControlInterface(int controlInt) { m_controlInt = controlInt; };
  int  GetControlInterface() { return m_controlInt; };
};

// Receiver reset errors
class PReceiverResetError : public MException {
public:
  explicit PReceiverResetError(const string &__arg);
};

#endif // PEXCEPTION
