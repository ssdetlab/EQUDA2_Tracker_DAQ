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

#ifndef POWERBOARD_H
#define POWERBOARD_H

#include "ADC128D818.h"
#include "ad5254.h"
#include "ad7997.h"
#include "i2cbus.h"
#include "ltc2635.h"
#include "max31865.h"
#include "max5419.h"
#include "pcf8574.h"
#include "sc18is602.h"
#include <stdint.h>

class powerboard {
public:
  typedef struct pbstate {
    float    Vmon[16];
    float    Imon[16];
    uint16_t chOn;
    float    Vout[16];
    float    Vbias;
    float    Ibias;
    uint8_t  biasOn;
    float    T;
    float    Tstaves[2];
  } pbstate_t;

  enum getFlags { GetMonitor = 0x01, GetSettings = 0x02, WaitTconv = 0x04, getAll = 0x07 };

public:
  powerboard(I2Cbus *busMaster, I2Cbus *busAux);
  ~powerboard();
  bool isReady();
  void setIth(uint8_t ch, float value);
  void setVbias(float value);
  void onVbias(uint8_t ch);
  void offVbias(uint8_t ch);
  void onAllVbias();
  void offAllVbias();
  void setVout(uint8_t ch, float value);
  void storeVout(uint8_t ch);
  void storeAllVout();
  void restoreAllVout();
  void onVout(uint8_t ch);
  void offVout(uint8_t ch);
  void onAllVout();
  void offAllVout();
  void startADC();
  void getState(pbstate_t *state, getFlags flags = getAll);

protected:
  I2Cbus *i2cBus;
  I2Cbus *i2cBusAux;

private:
  LTC2635 *   dacThreshold[4];
  AD5254 *    rdacVadj[4];
  PCF8574 *   regCtrl[2];
  PCF8574 *   regCtrlBias;
  ADC128D818 *adcMon[5];
  MAX5419 *   rdacVbias;
  SC18IS602 * spiBridge;
  MAX31865 *  temperatureDetector;
  MAX31865 *  temperatureDetectorStave[2];

private:
  // I2C device addresses
  enum {
    I2Caddress_dacThreshold_1_4   = 0x52, // LTC2635 U291	0x52
    I2Caddress_dacThreshold_5_8   = 0x60, // LTC2635 U287 0x60
    I2Caddress_dacThreshold_9_12  = 0x70, // LTC2635 U288 0x70
    I2Caddress_dacThreshold_13_16 = 0x72, // LTC2635 U289 0x72
    I2Caddress_rdacVadj_1_4       = 0x2c, // AD5254  U275 0x2c
    I2Caddress_rdacVadj_5_8       = 0x2d, // AD5254  U276 0x2d
    I2Caddress_rdacVadj_9_12      = 0x2e, // AD5254  U272 0x2e
    I2Caddress_rdacVadj_13_16     = 0x2f, // AD5254  U274 0x2f
    I2Caddress_rdacVbias          = 0x29, // MAX5419 U253 0x29
    AUX_I2Caddress_regCtrl_1_8    = 0x38, // PCF8574A U267 0x38 (AUX)
    AUX_I2Caddress_regCtrl_9_16   = 0x39, // PCF8574A U268 0x39 (AUX)
    I2Caddress_regCtrl_Bias       = 0x38, // PCF8574A U265 0x38
    I2Caddress_adcMon_1_4         = 0x1d, // ADC128D818 U279 0x1d
    I2Caddress_adcMon_5_8         = 0x1f, // ADC128D818 U281 0x1f
    I2Caddress_adcMon_9_12        = 0x35, // ADC128D818 U280 0x35
    I2Caddress_adcMon_13_16       = 0x37, // ADC128D818 U283 0x37
    I2Caddress_adcMon_Bias        = 0x1e, // ADC128D818 U282 0x1e
    I2Caddress_spiBridge          = 0x28  // SC18IS602  U195 0x28
  };
};

#endif // POWERBOARD_H
