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
 * Written by Antonio Franco  <Anotnio.Franco@ba.infn.it>
 *
 *		TPowerBoard class implementation
 *
 *		ver.1.0		12/07/2017
 *
 *
 *  		HISTORY
 *
 *
 */
#include "TPowerBoard.h"
#include "TReadoutBoardMOSAIC.h"
#include <chrono>
#include <numeric>
#include <thread>
#include <unistd.h>

/* -------------------------
        Constructor

        Parameter Input : board, TReadoutBoardMOSAIC pointer to a valid MOSAIC board object
                                        config, TPowerBoardConfig pointer to a Power Board Config
  object
  -------------------------- */
TPowerBoard::TPowerBoard(TReadoutBoardMOSAIC *board, TPowerBoardConfig *config)
{
  fMOSAICPowerBoard = board->GetPowerBoardHandle();
  fPowerBoardConfig = config;
  realTimeRead      = false;
  Init();
}

/* -------------------------
        Constructor

        Parameter Input : board, TReadoutBoardMOSAIC pointer to a valid MOSAIC board object

        NOTE: this constructor allows to load a default configuration into the data set.
  -------------------------- */
TPowerBoard::TPowerBoard(TReadoutBoardMOSAIC *board)
{
  TPowerBoardConfig *theConfig = new TPowerBoardConfig(NULL);
  fMOSAICPowerBoard            = board->GetPowerBoardHandle();
  fPowerBoardConfig            = theConfig;
  realTimeRead                 = false;
  Init();
}

/* -------------------------
        Init()

        Initialize all the members with the configuration values, than
        try to access the power board. Switch off all channels, then store
        the settings and finally read the power board status.

  -------------------------- */
void TPowerBoard::Init()
{
  fPowerBoardConfig->ReadCalibrationFile();
  thePowerBoardState = new powerboard::pbstate;

  fPBoard.VBset = fPowerBoardConfig->GetBiasVoltage();
  for (int i = 0; i < MAX_MOULESPERMOSAIC; i++) {
    fPBoard.Modules[i].AIset  = fPowerBoardConfig->GetAnalogCurrent(i);
    fPBoard.Modules[i].AVset  = fPowerBoardConfig->GetAnalogVoltage(i);
    fPBoard.Modules[i].DIset  = fPowerBoardConfig->GetDigitalCurrent(i);
    fPBoard.Modules[i].DVset  = fPowerBoardConfig->GetDigitalVoltage(i);
    fPBoard.Modules[i].BiasOn = fPowerBoardConfig->GetBiasOn(i);
  }
  std::lock_guard<std::mutex> lock(mutex_pb);
  // first of all test the presence of the power board
  try {
    fMOSAICPowerBoard->isReady();
  }
  catch (...) {
    std::cerr << "No Power board found ! Abort." << std::endl;
    return;
  }

  fMOSAICPowerBoard->startADC();
  // Get the State
  try {
    fMOSAICPowerBoard->getState(thePowerBoardState, powerboard::getFlags::getAll);
  }
  catch (...) {
    std::cerr << "Error accessing the Power board found ! Abort." << std::endl;
    return;
  }

  // Switch off all channels before the setting
  fMOSAICPowerBoard->offAllVbias();
  fMOSAICPowerBoard->offAllVout();

  // Finally set up the PowerBoard
  fMOSAICPowerBoard->setVbias(fPBoard.VBset);
  for (int i = 0; i < MAX_MOULESPERMOSAIC; i++) {
    fMOSAICPowerBoard->setIth((unsigned char)(i * 2), fPBoard.Modules[i].AIset);
    fMOSAICPowerBoard->setIth((unsigned char)(i * 2 + 1), fPBoard.Modules[i].DIset);
    fMOSAICPowerBoard->setVout((unsigned char)(i * 2), fPBoard.Modules[i].AVset);
    fMOSAICPowerBoard->setVout((unsigned char)(i * 2 + 1), fPBoard.Modules[i].DVset);
    if (fPBoard.Modules[i].BiasOn)
      fMOSAICPowerBoard->onVbias((unsigned char)(i));
    else
      fMOSAICPowerBoard->offVbias((unsigned char)(i));
  }
  // and also store the values inside the PB memory
  fMOSAICPowerBoard->storeAllVout();

  // first read of monitor values
  fMOSAICPowerBoard->getState(thePowerBoardState, powerboard::getFlags::getAll);

  // ensure correct settings
  compareSettings(thePowerBoardState);

  return;
}

/* -------------------------
        compareSettings()

        Compares the setup read from the power board firmware
        with that we store in class member

        Return : false is there is a mismatch

  -------------------------- */
bool TPowerBoard::compareSettings(powerboard::pbstate_t *aState)
{
  bool match = true;
  int  i;
  for (i = 0; i < MAX_MOULESPERMOSAIC; i++) {
    if (std::abs(aState->Vout[i * 2] - fPBoard.Modules[i].AVset) > 0.005) {
      match = false;
      std::cout << "Power board : Module =" << i
                << " the Analog V Set is different ! Board:" << aState->Vout[i * 2]
                << " Config:" << fPBoard.Modules[i].AVset << std::endl;
    }
    if (std::abs(aState->Vout[i * 2 + 1] - fPBoard.Modules[i].DVset) > 0.005) {
      match = false;
      std::cout << "Power board : Module =" << i
                << " the Digital V Set is different ! Board:" << aState->Vout[i * 2 + 1]
                << " Config:" << fPBoard.Modules[i].DVset << std::endl;
    }
  }
  return (match);
}

/* -------------------------
        readMonitor()

        Retrieve all monitored power board parameters from the hardware, and stores
        the values inside the class data member.
        If the last read time stamp is less then MINIMUM_TIMELAPSEFORPOLLING seconds
        the physical access will be skipped and the actual values are considered valid.

        Return : true if a physical access is done in order to monitor the power board

  -------------------------- */
bool TPowerBoard::readMonitor()
{
  int i;

  // test if the values are considered valid
  if (realTimeRead) {
    time_t now;
    time(&now);
    if (now <= fPBoard.TimeStamp + MINIMUM_TIMELAPSEFORPOLLING) return (false);
  }

  // Read the board
  std::lock_guard<std::mutex> lock(mutex_pb);
  fMOSAICPowerBoard->getState(thePowerBoardState, powerboard::getFlags::GetMonitor);

  // Set the data members
  time(&fPBoard.TimeStamp); // mark the time stamp
  fPBoard.VBmon         = thePowerBoardState->Vbias;
  fPBoard.IBmon         = thePowerBoardState->Ibias;
  fPBoard.Temp          = thePowerBoardState->T;
  fPBoard.TempStaves[0] = thePowerBoardState->Tstaves[0];
  fPBoard.TempStaves[1] = thePowerBoardState->Tstaves[1];

  for (i = 0; i < MAX_MOULESPERMOSAIC; i++) { // for each module
    fPBoard.Modules[i].AVmon         = thePowerBoardState->Vmon[i * 2];
    fPBoard.Modules[i].DVmon         = thePowerBoardState->Vmon[i * 2 + 1];
    fPBoard.Modules[i].AVsetReadback = thePowerBoardState->Vout[i * 2];
    fPBoard.Modules[i].DVsetReadback = thePowerBoardState->Vout[i * 2 + 1];
    fPBoard.Modules[i].AImon         = thePowerBoardState->Imon[i * 2];
    fPBoard.Modules[i].DImon         = thePowerBoardState->Imon[i * 2 + 1];
    fPBoard.Modules[i].BiasOn        = thePowerBoardState->biasOn & (0x01 << i);
    fPBoard.Modules[i].AchOn         = thePowerBoardState->chOn & (0x0001 << (i * 2));
    fPBoard.Modules[i].DchOn         = thePowerBoardState->chOn & (0x0001 << (i * 2 + 1));
  }

  return (true);
}

void TPowerBoard::GetPowerBoardState(powerboard::pbstate *state)
{
  // Read the board
  std::lock_guard<std::mutex> lock(mutex_pb);
  fMOSAICPowerBoard->getState(state, powerboard::getFlags::GetMonitor);
}

float TPowerBoard::GetAnalogCurrent(int module)
{
  float AIOffset, DIOffset;
  float Current = 0;
  int   N       = 20;
  for (int i = 0; i < N; i++) {
    readMonitor();
    Current += fPBoard.Modules[module].AImon;
  }
  Current /= N;
  fPowerBoardConfig->GetICalibration(module, AIOffset, DIOffset);
  return (Current - AIOffset);
}

float TPowerBoard::GetDigitalCurrent(int module)
{
  float AIOffset, DIOffset;
  float Current = 0;
  int   N       = 20;
  for (int i = 0; i < N; i++) {
    readMonitor();
    Current += fPBoard.Modules[module].DImon;
  }
  Current /= N;
  fPowerBoardConfig->GetICalibration(module, AIOffset, DIOffset);
  return (Current - DIOffset);
}

float TPowerBoard::GetBiasCurrent()
{
  float Offset;
  float Current = 0;
  int   N       = 20;
  for (int i = 0; i < N; i++) {
    readMonitor();
    Current += fPBoard.IBmon;
  }
  Current /= N;
  fPowerBoardConfig->GetIBiasCalibration(Offset);

  return (Current - Offset);
}

// Calibrate the bias voltage
// set calibration constants back to 1 / 0
// set two different voltages and measure the output voltage for each setting
// determine new calibration constants
void TPowerBoard::CalibrateVoltage(int module)
{
  // two set points that fall on a full bin
  // set the lower voltage second to not risk applying 2.2 V to a HIC
  float set2 = 1.58;  //-0.4;
  float set1 = 2.187; // -4;
  float analog1, analog2, digital1, digital2;
  float manalog, mdigital, banalog, bdigital;

  // set calibration back to slope 1 / intercept 0
  fPowerBoardConfig->SetVCalibration(module, 1, 1, 0, 0);

  // set and measure first point
  SetAnalogVoltage(module, set1);
  SetDigitalVoltage(module, set1);
  SwitchModule(module, true);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  analog1  = GetAnalogVoltage(module);
  digital1 = GetDigitalVoltage(module);

  // set and measure second point
  SetAnalogVoltage(module, set2);
  SetDigitalVoltage(module, set2);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  analog2  = GetAnalogVoltage(module);
  digital2 = GetDigitalVoltage(module);

  // calculate slope and intercept for calibration Vout -> Vset
  manalog  = (set1 - set2) / (analog1 - analog2);
  mdigital = (set1 - set2) / (digital1 - digital2);
  banalog  = set1 - manalog * analog1;
  bdigital = set1 - mdigital * digital1;

  // set new calibration values and switch off module
  fPowerBoardConfig->SetVCalibration(module, manalog, mdigital, banalog, bdigital);

  SwitchModule(module, false);
}

// Calibrate the current offset for a given module
// switch of the channel
// measure the current
void TPowerBoard::CalibrateCurrent(int module)
{
  float aOffset, dOffset;
  fPowerBoardConfig->SetICalibration(module, 0, 0);

  SwitchModule(module, false);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  aOffset = GetAnalogCurrent(module);
  dOffset = GetDigitalCurrent(module);

  fPowerBoardConfig->SetICalibration(module, aOffset, dOffset);
}

void TPowerBoard::CalibrateBiasCurrent()
{
  float Offset;
  fPowerBoardConfig->SetIBiasCalibration(0);

  SetBiasVoltage(0);
  // this is in principle unneccessary as the calibration should be done without HIC attached
  SwitchOFF();
  for (int imod = 0; imod < MAX_MOULESPERMOSAIC; imod++) {
    SetBiasOff(imod);
  }

  Offset = GetBiasCurrent();
  fPowerBoardConfig->SetIBiasCalibration(Offset);
}

void TPowerBoard::CalibrateBiasVoltage()
{
  // two set points that fall on a full bin
  // set the lower voltage second to not risk applying 2.2 V to a HIC
  float set2 = -0.4; // 1.58;
  float set1 = -4;   // 2.187;
  float measured1, measured2;
  float m, b;

  // set calibration back to slope 1 / intercept 0
  fPowerBoardConfig->SetVBiasCalibration(1, 0);

  // set and measure first point
  SetBiasVoltage(set1);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  measured1 = GetBiasVoltage();

  // set and measure second point
  SetBiasVoltage(set2);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  measured2 = GetBiasVoltage();

  // calculate slope and intercept for calibration Vout -> Vset
  m = (set1 - set2) / (measured1 - measured2);
  b = set1 - m * measured1;

  // set new calibration values and switch off module
  fPowerBoardConfig->SetVBiasCalibration(m, b);
}

// correct the output voltage for the (calculated) voltage drop
// if reset is true, the correction is set to 0
void TPowerBoard::CorrectVoltageDrop(TPowerBoardConfig::pb_t pb, bool reset, int nch)
{
  // Measure the channel currents
  // Calculate voltage drop
  // Correct voltage drop for slope of voltage characteristics
  // add corrected voltage drop to channel set voltage

  printf("\nPerforming voltage drop correction\n");

  std::vector<float> IDDA(nch);
  std::vector<float> IDDD(nch);
  std::vector<float> RGnd(nch);
  std::vector<float> VdropPart(nch);
  std::vector<float> VdropGnd(nch);
  std::vector<float> VdropAGnd(nch);
  std::vector<float> dVAnalog(nch);
  std::vector<float> dVDigital(nch);

  float RAnalog, RDigital, RGround, RAGround;
  float AVScale, DVScale, AVOffset, DVOffset;
  if (reset) {
    dVAnalog.assign(nch, 0);
    dVDigital.assign(nch, 0);
  }
  else {
    // store all values of analog and digital currents to vectors
    for (int i = 0; i < nch; i++) {
      IDDA[i] = GetAnalogCurrent(i);
      IDDD[i] = GetDigitalCurrent(i);
      fPowerBoardConfig->GetResistances(i, RAnalog, RDigital, RGround, RAGround, pb);

      printf("channel %d: I_a = %.3f A, I_d = %.3f A\n", i, IDDA[i], IDDD[i]);
      printf("     PB %d: R_a = %.3f \u2126, R_d = %.3f \u2126, R_gnd = %.3f \u2126\n", pb, RAnalog,
             RDigital, RGround);
      RGnd[i]     = RGround;
      VdropGnd[i] = (IDDA[i] + IDDD[i]) * RGnd[i];
    }

    if ((pb == TPowerBoardConfig::realML) || (pb == TPowerBoardConfig::realOL)) {
      float Itot = 0.;
      for (int ihic = nch - 1; ihic >= 0; --ihic) {
        Itot += IDDD[ihic];
        Itot += IDDA[ihic];
        float res       = RGnd[ihic] - (ihic > 0 ? RGnd[ihic - 1] : 0.);
        VdropPart[ihic] = res * Itot;
        printf("channel %d: VdropPart = %.3f \u2126 * %.3f A = %.3f V\n", ihic, res, Itot,
               VdropPart[ihic]);
      }
    }
    for (int module = 0; module < nch; ++module) {
      if ((pb == TPowerBoardConfig::realML) || (pb == TPowerBoardConfig::realOL))
        VdropGnd[module] = std::accumulate(VdropPart.begin(), VdropPart.begin() + module + 1, 0.);
      // printf("channel %d: VdropGnd = %.3f V\n", module, VdropGnd[module]);

      fPowerBoardConfig->GetResistances(module, RAnalog, RDigital, RGround, RAGround, pb);
      fPowerBoardConfig->GetVCalibration(module, AVScale, DVScale, AVOffset, DVOffset);

      dVAnalog[module]  = IDDA[module] * RAnalog + VdropGnd[module];
      dVDigital[module] = IDDD[module] * RDigital + VdropGnd[module];
      dVAnalog[module] *= AVScale;
      dVDigital[module] *= DVScale;
      printf("channel %d: \u0394V_a = (%.3f A * %.3f \u2126 + %.3f V) * %.3f = %.3f,\n           "
             "\u0394V_d = (%.3f A * %.3f \u2126 + %.3f V) * %.3f = %.3f\n",
             module, IDDA[module], RAnalog, VdropGnd[module], AVScale, dVAnalog[module],
             IDDD[module], RDigital, VdropGnd[module], DVScale, dVDigital[module]);
    }
  }

  for (int module = 0; module < nch; ++module) {
    if (fPBoard.Modules[module].AVset + dVAnalog[module] > SAFE_OUTPUT) {
      std::cout
          << "ERROR (CorrectVoltageDrop): Asking for set voltage AVDD above safe limit; using "
             "safe max value, difference = "
          << fPBoard.Modules[module].AVset + dVAnalog[module] - SAFE_OUTPUT << " V." << std::endl;
      dVAnalog[module] = SAFE_OUTPUT - fPBoard.Modules[module].AVset;
    }
    if (fPBoard.Modules[module].DVset + dVDigital[module] > SAFE_OUTPUT) {
      std::cout
          << "ERROR (CorrectVoltageDrop): Asking for set voltage DVDD above safe limit; using "
             "safe max value, difference = "
          << fPBoard.Modules[module].DVset + dVDigital[module] - SAFE_OUTPUT << " V." << std::endl;
      dVDigital[module] = SAFE_OUTPUT - fPBoard.Modules[module].DVset;
    }

    // fPBoard contains the voltages corrected with the channel calibration
    printf("channel %i: V_a = %.3f V + %.3f V, V_d = %.3f V + %.3f V\n", module,
           fPBoard.Modules[module].AVset, dVAnalog[module], fPBoard.Modules[module].DVset,
           dVDigital[module]);
    std::lock_guard<std::mutex> lock(mutex_pb);
    fMOSAICPowerBoard->setVout((unsigned char)(module * 2),
                               fPBoard.Modules[module].AVset + dVAnalog[module]);
    fMOSAICPowerBoard->setVout((unsigned char)(module * 2 + 1),
                               fPBoard.Modules[module].DVset + dVDigital[module]);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void TPowerBoard::CorrectVoltageDrop(int module, TPowerBoardConfig::pb_t pb, bool reset)
{
  // Measure the channel currents
  // Calculate voltage drop
  // Correct voltage drop for slope of voltage characteristics
  // add corrected voltage drop to channel set voltage

  printf("\nPerforming voltage drop correction (module-wise)\n");

  std::vector<float> IDDA(MAX_MOULESPERMOSAIC);
  std::vector<float> IDDD(MAX_MOULESPERMOSAIC);
  std::vector<float> RGnd(MAX_MOULESPERMOSAIC);
  std::vector<float> RAGnd(MAX_MOULESPERMOSAIC);
  std::vector<float> VdropPart(MAX_MOULESPERMOSAIC);

  float RAnalog, RDigital, RGround, RAGround;
  float dVAnalog, dVDigital, VdropGnd, VdropAGnd;
  float AVScale, DVScale, AVOffset, DVOffset;
  if (reset) {
    dVAnalog  = 0;
    dVDigital = 0;
  }
  else {
    // store all values of analog and digital currents to vectors
    for (int i = 0; i < MAX_MOULESPERMOSAIC; i++) {
      if ((pb != TPowerBoardConfig::realML) && (pb != TPowerBoardConfig::realOL) && (i != module))
        continue;
      IDDA[i] = GetAnalogCurrent(i);
      IDDD[i] = GetDigitalCurrent(i);
      fPowerBoardConfig->GetResistances(i, RAnalog, RDigital, RGround, RAGround, pb);

      printf("channel %d: I_a = %.3f A, I_d = %.3f A\n", i, IDDA[i], IDDD[i]);
      printf("     PB %d: R_a = %.3f \u2126, R_d = %.3f \u2126, R_gnd = %.3f \u2126, R_Agnd = "
             "%.3f \u2126\n",
             pb, RAnalog, RDigital, RGround, RAGround);
      RGnd[i]  = RGround;
      RAGnd[i] = RAGround;
    }
    // include possibility of split GND (for IB HICs only)
    if (RAGnd[module] > 0) {
      VdropGnd  = IDDD[module] * RGnd[module];
      VdropAGnd = IDDA[module] * RAGnd[module];
    }
    else {
      VdropGnd  = (IDDA[module] + IDDD[module]) * RGnd[module];
      VdropAGnd = -1;
    }

    if ((pb == TPowerBoardConfig::realML) || (pb == TPowerBoardConfig::realOL)) {
      float Itot = 0.;
      for (int ihic = MAX_MOULESPERMOSAIC - 1; ihic >= 0; --ihic) {
        Itot += IDDD[ihic];
        Itot += IDDA[ihic];
        float res       = RGnd[ihic] - (ihic > 0 ? RGnd[ihic - 1] : 0.);
        VdropPart[ihic] = res * Itot;
        printf("channel %d: VdropPart = %.3f \u2126 * %.3f A = %.3f V\n", ihic, res, Itot,
               VdropPart[ihic]);
      }
      VdropGnd = std::accumulate(VdropPart.begin(), VdropPart.begin() + module + 1, 0.);
    }

    fPowerBoardConfig->GetResistances(module, RAnalog, RDigital, RGround, RAGround, pb);
    fPowerBoardConfig->GetVCalibration(module, AVScale, DVScale, AVOffset, DVOffset);

    if (VdropAGnd > 0) {
      dVAnalog = IDDA[module] * RAnalog + VdropAGnd;
    }
    else {
      dVAnalog = IDDA[module] * RAnalog + VdropGnd;
    }
    dVDigital = IDDD[module] * RDigital + VdropGnd;
    dVAnalog *= AVScale;
    dVDigital *= DVScale;
    if (VdropAGnd > 0) {
      printf("channel %d: \u0394V_a = (%.3f A * %.3f \u2126 + %.3f V) * %.3f = %.3f,\n           "
             "\u0394V_d = (%.3f A * %.3f \u2126 + %.3f V) * %.3f = %.3f\n",
             module, IDDA[module], RAnalog, VdropAGnd, AVScale, dVAnalog, IDDD[module], RDigital,
             VdropGnd, DVScale, dVDigital);
    }
    else {
      printf("channel %d: \u0394V_a = (%.3f A * %.3f \u2126 + %.3f V) * %.3f = %.3f,\n           "
             "\u0394V_d = (%.3f A * %.3f \u2126 + %.3f V) * %.3f = %.3f\n",
             module, IDDA[module], RAnalog, VdropGnd, AVScale, dVAnalog, IDDD[module], RDigital,
             VdropGnd, DVScale, dVDigital);
    }
  }


  if (fPBoard.Modules[module].AVset + dVAnalog > SAFE_OUTPUT) {
    std::cout << "ERROR (CorrectVoltageDrop): Asking for set voltage AVDD above safe limit; using "
                 "safe max value, difference = "
              << fPBoard.Modules[module].AVset + dVAnalog - SAFE_OUTPUT << " V." << std::endl;
    dVAnalog = SAFE_OUTPUT - fPBoard.Modules[module].AVset;
  }
  if (fPBoard.Modules[module].DVset + dVDigital > SAFE_OUTPUT) {
    std::cout << "ERROR (CorrectVoltageDrop): Asking for set voltage DVDD above safe limit; using "
                 "safe max value, difference = "
              << fPBoard.Modules[module].DVset + dVDigital - SAFE_OUTPUT << " V." << std::endl;
    dVDigital = SAFE_OUTPUT - fPBoard.Modules[module].DVset;
  }

  // fPBoard contains the voltages corrected with the channel calibration
  printf("channel %i: V_a = %.3f V + %.3f V, V_d = %.3f V + %.3f V\n", module,
         fPBoard.Modules[module].AVset, dVAnalog, fPBoard.Modules[module].DVset, dVDigital);
  std::lock_guard<std::mutex> lock(mutex_pb);
  fMOSAICPowerBoard->setVout((unsigned char)(module * 2), fPBoard.Modules[module].AVset + dVAnalog);
  fMOSAICPowerBoard->setVout((unsigned char)(module * 2 + 1),
                             fPBoard.Modules[module].DVset + dVDigital);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

/* -------------------------
        SetModule()

        Sets all the parameters for one module

        Parameter Input : module, integer (0..7) the Number of the power board module
                                        AV, float. Analogic channel voltage output
                                        AI, float. Analogic channel current limit
                                        DV, float. Digital channel voltage output
                                        DI, float. Digital channel current limit
                                        BiasOn bool. Set up of the Back Bias channel

  -------------------------- */
void TPowerBoard::SetModule(int module, float AV, float AI, float DV, float DI, bool BiasOn)
{
  fPBoard.Modules[module].AVset  = AV;
  fPBoard.Modules[module].DVset  = DV;
  fPBoard.Modules[module].AIset  = AI;
  fPBoard.Modules[module].DIset  = DI;
  fPBoard.Modules[module].BiasOn = BiasOn;

  std::lock_guard<std::mutex> lock(mutex_pb);
  fMOSAICPowerBoard->setVout((unsigned char)(module * 2), AV);
  fMOSAICPowerBoard->setVout((unsigned char)(module * 2 + 1), DV);
  fMOSAICPowerBoard->setIth((unsigned char)(module * 2), AI);
  fMOSAICPowerBoard->setIth((unsigned char)(module * 2 + 1), DI);
  if (BiasOn)
    fMOSAICPowerBoard->onVbias(module);
  else
    fMOSAICPowerBoard->offVbias(module);
  return;
}

/* -------------------------
        SwitchModule()

        Switch On/Off the Analogic and Digital channel of the specified module

        Parameter Input : module, integer (0..7) the Number of the power board module
                                        value, bool. The switch value ON := true, OFF := false

  -------------------------- */
void TPowerBoard::SwitchModule(int module, bool value)
{
  fPBoard.Modules[module].AchOn = value;
  fPBoard.Modules[module].DchOn = value;
  std::lock_guard<std::mutex> lock(mutex_pb);
  if (value) {
    fMOSAICPowerBoard->onVout(module * 2);
    fMOSAICPowerBoard->onVout(module * 2 + 1);
  }
  else {
    fMOSAICPowerBoard->offVout(module * 2);
    fMOSAICPowerBoard->offVout(module * 2 + 1);
  }

  return;
}

/* -------------------------
        GetModule()

        Gets all the parameters for one module

        Parameter Input : module, integer (0..7) the Number of the power board module

        Parameter Output : AV, pointer to float. Analogic channel voltage output
                                        AI, pointer to float. Analogic channel current limit
                                        DV, pointer to float. Digital channel voltage output
                                        DI, pointer to float. Digital channel current limit
                                        BiasOn pointer to bool. Set up of the Back Bias channel
                                        AChOn, pointer to bool. State of Analogic channel, true :=
  ON, false := OFF
                                        DChOn, pointer to bool. State of Digital channel, true :=
  ON, false := OFF
  -------------------------- */
void TPowerBoard::GetModule(int module, float *AV, float *AI, float *DV, float *DI, bool *BiasOn,
                            bool *AChOn, bool *DChOn)
{
  readMonitor();
  *AV     = fPBoard.Modules[module].AVmon;
  *AI     = GetAnalogCurrent(module);
  *DV     = fPBoard.Modules[module].DVmon;
  *DI     = GetDigitalCurrent(module);
  *BiasOn = fPBoard.Modules[module].BiasOn;
  *AChOn  = fPBoard.Modules[module].AchOn;
  *DChOn  = fPBoard.Modules[module].DchOn;
  return;
}

/* -------------------------
        IsOk()

        Returns if the power board is connected and operable

        Return : true if the power board is OK
  -------------------------- */
bool TPowerBoard::IsOK()
{
  std::lock_guard<std::mutex> lock(mutex_pb);
  // first of all test the presence of the power board
  try {
    fMOSAICPowerBoard->isReady();
  }
  catch (...) {
    std::cerr << "No Power board found ! Abort." << std::endl;
    return (false);
  }
  // Now read the state
  try {
    fMOSAICPowerBoard->getState(thePowerBoardState, powerboard::getFlags::GetMonitor);
  }
  catch (...) {
    std::cerr << "Error accessing the Power board found ! Abort." << std::endl;
    return (false);
  }
  return (true);
}

// ------------------ eof ---------------------------
