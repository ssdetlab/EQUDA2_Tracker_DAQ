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

#ifndef MSERVICE_H
#define MSERVICE_H

#include "mexception.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>

#define FIRMWARE_PORT 65000
#define RCV_LONG_TIMEOUT 2000 // timeout in ms for the first rx datagrams
#define RCV_SHORT_TIMEOUT 100 // timeout in ms for rx datagrams

class MService {
public:
  typedef struct fw_info {
    int           ver_maj;
    int           ver_min;
    unsigned char flash_id[3];
    unsigned char flash_status_register;
    char          sw_identity[33];
    char          fw_identity[33];
  } fw_info_t;

public:
  MService();
  MService(const char *brdName, int port = FIRMWARE_PORT);
  ~MService();
  void setIPaddress(const char *brdName, int port = FIRMWARE_PORT);
  void readFWinfo(fw_info_t *info);

private:
  int  sockRead(unsigned char *rxBuffer, int bufSize);
  void sockWrite(unsigned char *txBuffer, int txSize);

private:
  // int port;
  int                sockfd;
  struct sockaddr_in sockAddress;
  int                rcvTimoutTime;
  uint8_t            seqNumber;
};

class MSrvcError : public MException {
public:
  explicit MSrvcError(const string &__arg);
};

#endif // MSERVICE_H
