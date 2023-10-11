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

#include "mexception.h"

MException::MException() {}

MException::MException(const string &arg) { msg = arg; }

MException::~MException() throw() {}

MIPBusUDPError::MIPBusUDPError(const string &arg) { msg = "IPBusUDP Error: " + arg; }

// IPBus over UDP Timeout
MIPBusUDPTimeout::MIPBusUDPTimeout() {}

// IPBus error
MIPBusError::MIPBusError(const string &arg, const string &address)
{
  if (address.length() > 0) {
    msg = "IPBus Error (address " + address + "): " + arg;
  }
  else {
    msg = "IPBus Error: " + arg;
  }
}

// IPBus error - Remote Bus Write error
MIPBusErrorWrite::MIPBusErrorWrite(const string &arg, const string &address)
{
  if (address.length() > 0) {
    msg = "IPBus Error (address " + address + "): " + arg;
  }
  else {
    msg = "IPBus Error: " + arg;
  }
}

// IPBus error - Remote Bus Read error
MIPBusErrorReadTimeout::MIPBusErrorReadTimeout(const string &arg, const string &address)
{
  if (address.length() > 0) {
    msg = "IPBus Error (address " + address + "): " + arg;
  }
  else {
    msg = "IPBus Error: " + arg;
  }
}

// Data connection over TCP error
MDataConnectError::MDataConnectError(const string &arg)
{
  msg = "TCP Data connection Error: " + arg;
}

// Data receive over TCP
MDataReceiveError::MDataReceiveError(const string &arg) { msg = "TCP Data receive Error: " + arg; }

// Data parser
MDataParserError::MDataParserError(const string &arg) { msg = "TCP Data parser Error: " + arg; }

// Board initialization
MBoardInitError::MBoardInitError(const string &arg) { msg = "Board initialization Error: " + arg; }

const char *MException::what() const throw() { return msg.c_str(); }
