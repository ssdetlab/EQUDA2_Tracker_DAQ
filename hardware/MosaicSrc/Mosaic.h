/*
 * Copyright (C) 2017
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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2017.
 *
 */

#ifndef MOSAIC_H
#define MOSAIC_H

#include "TAlpideDataParser.h"
#include "alpidercv.h"
#include "controlinterface.h"
#include "i2cbus.h"
#include "i2csyspll.h"
#include "ipbusudp.h"
#include "mboard.h"
#include "mcoordinator.h"
#include "mdatagenerator.h"
#include "mdatareceiver.h"
#include "mruncontrol.h"
#include "mtriggercontrol.h"
#include "mwbb.h"
#include "pulser.h"
#include "trgrecorder.h"

namespace Mosaic {
  typedef enum rcvRate_e { // Receiver data rate (in Mbps)
    RCV_RATE_400,
    RCV_RATE_600,
    RCV_RATE_1200
  } TReceiverSpeed;
}

#endif // MOSAIC_H
