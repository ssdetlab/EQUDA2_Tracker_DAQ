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

#ifndef MEXCEPTION_H
#define MEXCEPTION_H

#include <exception>
#include <string>

// class string;
using namespace std;

class MException : public exception {
public:
  /** Takes a character string describing the error.  */
  explicit MException();
  explicit MException(const string &__arg);
  ~MException() throw();

  /** Returns a C-style character string describing the general cause of
   *  the current error (the same string passed to the ctor).  */
  virtual const char *what() const throw();

protected:
  string msg;
};

// IPBus  error
class MIPBusError : public MException {
public:
  explicit MIPBusError(const string &__arg, const string &__address = "");
};

// IPBus error - Remote Bus Write error
class MIPBusErrorWrite : public MException {
public:
  explicit MIPBusErrorWrite(const string &__arg, const string &__address = "");
};

// IPBus error - Remote Bus Read error
class MIPBusErrorReadTimeout : public MException {
public:
  explicit MIPBusErrorReadTimeout(const string &__arg, const string &__address = "");
};

// IPBus over UDP error
class MIPBusUDPError : public MException {
public:
  explicit MIPBusUDPError(const string &__arg);
};

// IPBus over UDP Timeout
class MIPBusUDPTimeout : public MException {
public:
  explicit MIPBusUDPTimeout();
};

// Data connection over TCP error
class MDataConnectError : public MException {
public:
  explicit MDataConnectError(const string &__arg);
};

// Data receive over TCP
class MDataReceiveError : public MException {
public:
  explicit MDataReceiveError(const string &__arg);
};

// Data parser
class MDataParserError : public MException {
public:
  explicit MDataParserError(const string &__arg);
};

class MBoardInitError : public MException {
public:
  explicit MBoardInitError(const string &__arg);
};

#endif // MEXCEPTION
