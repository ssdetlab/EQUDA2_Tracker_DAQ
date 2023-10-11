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
#include "powerboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

powerboard::powerboard(I2Cbus *busMaster, I2Cbus *busAux)
{
  i2cBus    = busMaster;
  i2cBusAux = busAux;

  dacThreshold[0] = new LTC2635(i2cBus, I2Caddress_dacThreshold_1_4);
  dacThreshold[1] = new LTC2635(i2cBus, I2Caddress_dacThreshold_5_8);
  dacThreshold[2] = new LTC2635(i2cBus, I2Caddress_dacThreshold_9_12);
  dacThreshold[3] = new LTC2635(i2cBus, I2Caddress_dacThreshold_13_16);

  rdacVadj[0] = new AD5254(i2cBus, I2Caddress_rdacVadj_1_4);
  rdacVadj[1] = new AD5254(i2cBus, I2Caddress_rdacVadj_5_8);
  rdacVadj[2] = new AD5254(i2cBus, I2Caddress_rdacVadj_9_12);
  rdacVadj[3] = new AD5254(i2cBus, I2Caddress_rdacVadj_13_16);

  rdacVbias = new MAX5419(i2cBus, I2Caddress_rdacVbias);

  regCtrl[0] = new PCF8574(i2cBusAux, AUX_I2Caddress_regCtrl_1_8);
  regCtrl[1] = new PCF8574(i2cBusAux, AUX_I2Caddress_regCtrl_9_16);

  regCtrlBias = new PCF8574(i2cBus, I2Caddress_regCtrl_Bias);

  adcMon[0] = new ADC128D818(i2cBus, I2Caddress_adcMon_1_4);
  adcMon[1] = new ADC128D818(i2cBus, I2Caddress_adcMon_5_8);
  adcMon[2] = new ADC128D818(i2cBus, I2Caddress_adcMon_9_12);
  adcMon[3] = new ADC128D818(i2cBus, I2Caddress_adcMon_13_16);
  adcMon[4] = new ADC128D818(i2cBus, I2Caddress_adcMon_Bias);

  spiBridge           = new SC18IS602(i2cBus, I2Caddress_spiBridge);
  temperatureDetector = new MAX31865(spiBridge, 0);

#ifdef PB_NEW
  for (unsigned int i = 0; i < 2; ++i)
    temperatureDetectorStave[i] = new MAX31865(spiBridge, i + 1);
#else
  for (unsigned int i = 0; i < 2; ++i)
    temperatureDetectorStave[i] = 0x0;
#endif
}

powerboard::~powerboard()
{
  // delete objects in creation reverse order
  delete temperatureDetector;
#ifdef PB_NEW
  for (unsigned int i = 0; i < 0; ++i) {
    delete temperatureDetectorStave[i];
  }
#endif
  delete spiBridge;

  for (int i = 0; i < 5; i++)
    delete adcMon[i];

  delete regCtrlBias;
  delete regCtrl[0];
  delete regCtrl[1];
  delete rdacVbias;

  for (int i = 0; i < 4; i++)
    delete rdacVadj[i];

  for (int i = 0; i < 4; i++)
    delete dacThreshold[i];
}

bool powerboard::isReady()
{
  uint8_t tmp;
  adcMon[0]->getConfiguration(&tmp);
  // if board is offline, an exception is generated

  return true;
}

/*
        Set the current threshold of a single channel
*/
void powerboard::setIth(uint8_t ch, float value)
{
  LTC2635 *dac;
  uint16_t data;

  // select the DAC
  dac = dacThreshold[ch / 4];

  // evaluate the dac value from current value
  data = 410 + (3685.0 / 3.0) * value;

  if (data > 0x0fff) data = 0x0fff;

  dac->WriteUpdateReg(ch & 0x03, data);
}

/*
        Set the substrate bias
*/
void powerboard::setVbias(float value)
{
  uint16_t data;

  if (value > 0) return;

  data = value * (-125.0 / 5.0);

  if (data > 255) data = 255;

  rdacVbias->setRDAC(data);
}

void powerboard::offVbias(uint8_t ch)
{
  uint8_t tmp = regCtrlBias->read();
  regCtrlBias->write(tmp | (1 << ch));
}

void powerboard::onVbias(uint8_t ch)
{
  uint8_t tmp = regCtrlBias->read();
  regCtrlBias->write(tmp & ~(1 << ch));
}

void powerboard::offAllVbias() { regCtrlBias->write(0xff); }

void powerboard::onAllVbias() { regCtrlBias->write(0x00); }

void powerboard::setVout(uint8_t ch, float value)
{
  int     data;
  AD5254 *rdac;

  // select the RDAC
  rdac = rdacVadj[ch / 4];

  data = (value / 0.00486) - 306;
  if (data < 0) data = 0;
  if (data > 255) data = 255;

  rdac->setRDAC(ch & 0x03, data);
}

void powerboard::storeVout(uint8_t ch)
{
  AD5254 *rdac;

  // select the RDAC
  rdac = rdacVadj[ch / 4];
  rdac->storeRDAC(ch & 0x03);
}

void powerboard::storeAllVout()
{
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      rdacVadj[j]->storeRDAC(i);
    }
  }
  rdacVbias->storeRDAC();
}

void powerboard::restoreAllVout()
{
  for (int j = 0; j < 4; j++)
    rdacVadj[j]->restoreAll();
  rdacVbias->restoreRDAC();
}

// turn on selected channel
void powerboard::onVout(uint8_t ch)
{
  PCF8574 *regCtrlPtr = regCtrl[ch / 8];
  ch %= 8;

  uint8_t tmp = regCtrlPtr->read();
  regCtrlPtr->write(tmp | (1 << ch));
}

// turn off selected channel
void powerboard::offVout(uint8_t ch)
{
  PCF8574 *regCtrlPtr = regCtrl[ch / 8];
  ch %= 8;

  uint8_t tmp = regCtrlPtr->read();
  regCtrlPtr->write(tmp & ~(1 << ch));
}

// turn on all channels
void powerboard::onAllVout()
{
  regCtrl[0]->write(0xff);
  regCtrl[1]->write(0xff);
}

// turn off all channels
void powerboard::offAllVout()
{
  regCtrl[0]->write(0x00);
  regCtrl[1]->write(0x00);
}

// polinomial interpolation of RTD to Temperature function for PT100 sensor
static float RTD2T(float r)
{
  const float R0 = 100.0; // PT100 ressitance at 0 C

  const float a = 2.3E-3;
  const float b = 2.5579;
  const float c = 1E-3;

  r -= R0;
  return a + r * (b + r * c);
}

/*
        Start ADC and temperature converter
*/
void powerboard::startADC()
{
  temperatureDetector->configure();

#ifdef PB_NEW
  for (unsigned int i = 0; i < 2; ++i)
    temperatureDetectorStave[i]->configure();
#endif

  for (int i = 0; i < 5; i++)
    adcMon[i]->setConfiguration();
}

/*
        Read board state from all chips
*/
void powerboard::getState(pbstate_t *state, getFlags flags)
{
  // Latch register
  state->chOn = regCtrl[0]->read();
  state->chOn |= regCtrl[1]->read() << 8;
  state->biasOn = ~regCtrlBias->read();

  if (flags & GetSettings) {
    // Voltage adjust
    for (int i = 0; i < 16; i++) {
      int     rdata;
      AD5254 *rdac = rdacVadj[i / 4];

      rdata          = rdac->getRDAC(i & 0x03);
      state->Vout[i] = (float)(rdata + 306) * 0.00486;
    }
  }

  if (flags & GetMonitor) {
    uint16_t  adcData[40];
    uint16_t *dataPtr = adcData;
    float     data;

    for (int i = 0; i < 5; i++)
      adcMon[i]->convert(adcData + i * 8);

    for (int i = 0; i < 16; i++) {
      // Voltage
      data = ((*dataPtr++) >> 4) & 0xfff;
#ifdef PB_NEW
      state->Vmon[i] = data * (3.072 / 4096.0);
#else
      state->Vmon[i]    = data * (2.56 / 4096.0);
#endif

      // Current
      data           = ((*dataPtr++) >> 4) & 0xfff;
      state->Imon[i] = ((data * (2.56 / 4096.0) - 0.25) * 1.337) + 0.013;
      if (state->Imon[i] < 0) state->Imon[i] = 0;
    }

    data         = ((*dataPtr++) >> 4) & 0xfff;
    state->Ibias = data * (2.56 / 4096.0);
#ifdef PB_NEW
    state->Ibias *= 4. / 40.;
#endif
#ifdef PB_MODIFIED
    state->Ibias *= 4. / 39.;

#endif
    dataPtr++;
    data         = ((*dataPtr++) >> 4) & 0xfff;
    state->Vbias = data * (-5.12 / 4096.0);
#if defined(PB_NEW) || defined(PB_MODIFIED)
    state->Ibias = state->Ibias < -state->Vbias / 100. ? 0. : state->Vbias / 100.;
#endif

    float rtd = (temperatureDetector->getRTD() >> 1) * (400.0 / 32768.0);
    state->T  = RTD2T(rtd);

    for (unsigned i = 0; i < 2; ++i) {
#ifdef PB_NEW
      uint16_t temp = temperatureDetectorStave[i]->getRTD();
      if (temp != 65535U && temp != 0U) { // something connected
        rtd               = (temp >> 1) * (400.0 / 32768.0);
        state->Tstaves[i] = RTD2T(rtd);
      }
      else { // nothing connected
        state->Tstaves[i] = -273.15;
      }
#else
      state->Tstaves[i] = -273.15;
#endif
    }
  }
}
