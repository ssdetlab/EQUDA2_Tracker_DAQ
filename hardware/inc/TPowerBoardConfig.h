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
 *		TPowerBoardConfig class header
 *
 *		ver.1.0		12/07/2017
 *
 *
 *  		HISTORY
 *
 *
 */
#ifndef POWERBOARDCONFIG_H
#define POWERBOARDCONFIG_H

#include "TBoardConfig.h"
#include <stdio.h>

// The maximum number of modules of the power board
// that the MOSAIC board can drive.
#define MAX_MOULESPERMOSAIC 8

// Default values table

#define DEF_BOTTOM 0 // default: top power unit
#define DEF_BOARD 0  // default: PB connected to first MOSAIC

// a) Default values used in constructor
#define DEF_BIASVOLTAGE 0.0
#define DEF_ANALOGVOLTAGE 1.80
#define DEF_ANALOGMAXCURRENT 1.00
#define DEF_DIGITALVOLTAGE 1.80
#define DEF_DIGITALMAXCURRENT 1.50
#define DEF_BIASCHANNELON false

// b) Setup-specific default values - to be refined
// (can be set by setter methods)
#define DEF_BIASVOLTAGE_OB 0.0
#define DEF_ANALOGVOLTAGE_OB 1.82
#define DEF_ANALOGMAXCURRENT_OB 0.5
#define DEF_DIGITALVOLTAGE_OB 1.82
#define DEF_DIGITALMAXCURRENT_OB 1.50
#define DEF_BIASCHANNELON_OB false

#define DEF_BIASVOLTAGE_IB 0.0
#define DEF_ANALOGVOLTAGE_IB 1.80
#define DEF_ANALOGMAXCURRENT_IB 2.0
#define DEF_DIGITALVOLTAGE_IB 1.80
#define DEF_DIGITALMAXCURRENT_IB 2.0
#define DEF_BIASCHANNELON_IB false

#define DEF_AVSCALE 1.0
#define DEF_DVSCALE 1.0
#define DEF_AVOFFSET 0.0
#define DEF_DVOFFSET 0.0
#define DEF_DIOFFSET 0.0
#define DEF_AIOFFSET 0.0
#define DEF_CALDLINER 0.0
#define DEF_CALALINER 0.0
#define DEF_CALGNDLINER 0.0
#define DEF_CALAGNDLINER -1.0
#define DEF_BIASOFFSET 0.0
#define DEF_BIASSCALE 1.0
#define DEF_IBIASOFFSET 0.0

// internal power board resistances between regulator and breakout board
// according to power board manual version 1.2 (14/07/2017)
// first index is the power unit (0 = bottom, 1 = top), second is module
const float RAnalog[2][8]  = {{0.035, 0.039, 0.047, 0.054, 0.033, 0.044, 0.051, 0.052},
                             {0.033, 0.038, 0.044, 0.056, 0.033, 0.044, 0.059, 0.052}};
const float RDigital[2][8] = {{0.034, 0.042, 0.043, 0.050, 0.036, 0.038, 0.044, 0.052},
                              {0.033, 0.040, 0.041, 0.049, 0.034, 0.037, 0.040, 0.056}};
// resistances of wire power bus
const float RWPBAnalog[7]  = {0, 0, 0, 0, 0, 0, 0};
const float RWPBDigital[7] = {0, 0, 0, 0, 0, 0, 0};
const float RWPBGround[7]  = {0, 0, 0, 0, 0, 0, 0};
// resistances of real power bus, outer layer and middle layer
// OL: values measured in Torino
const float RPBAnalog[7]    = {0.179, 0.383, 0.458, 0.476, 0.490, 0.512, 0.507};
const float RPBDigital[7]   = {0.074, 0.098, 0.107, 0.113, 0.123, 0.118, 0.121};
const float RPBGround[7]    = {0.007, 0.008, 0.010, 0.012, 0.014, 0.016, 0.018};
const float RPBAnalogML[4]  = {0.193, 0.291, 0.356, 0.388};
const float RPBDigitalML[4] = {0.094, 0.087, 0.092, 0.1};
const float RPBGroundML[4]  = {0.007, 0.008, 0.010, 0.012};

// Class definition
class TPowerBoardConfig {

  // structures a data types
  // the configuration data types contain set values and calibration constants
  // the set values correspond to the desired output value, i.e. the value before applying the
  // calibration
public:
  typedef struct Mod {
    bool  BiasOn;
    bool  BiasEnabled;
    float AVset;
    float AIset;
    float DVset;
    float DIset;
    float CalAVScale;
    float CalDVScale;
    float CalAVOffset;
    float CalDVOffset;
    float CalDIOffset;
    float CalAIOffset;
    float CalDLineR;
    float CalALineR;
    float CalGNDLineR;
    float CalAGNDLineR;
  } Mod_t;

  typedef struct PowBoard {
    Mod_t Modul[MAX_MOULESPERMOSAIC];
    float VBset;
    float CalBiasOffset;
    float CalBiasScale;
    float CalIBiasOffset;
  } PowBoard_t;

  enum pb_t { none = 0, mockup, realML, realOL };

  // members
private:
  void GetLineResistances(int mod, float &ALineR, float &DLineR, float &GNDLineR, float &AGNDLineR);

  FILE *                       fhConfigFile; // the file handle of the Configuration File
  PowBoard_t                   fPBConfig;
  TBoardType                   fBoardType;
  int                          m_bottom;
  int                          m_board;
  std::map<std::string, int *> fSettings;

  // methods
public:
  TPowerBoardConfig(const char *AConfigFileName);

  // Info

  // Getters
  float GetBiasVoltage();

  // GetAnalogVoltage and GetDigitalVoltage return the voltages
  // taking into account the channel calibrations
  float GetAnalogVoltage(int mod);
  float GetAnalogCurrent(int mod) { return (fPBConfig.Modul[mod].AIset); };
  float GetDigitalVoltage(int mod);
  float GetDigitalCurrent(int mod) { return (fPBConfig.Modul[mod].DIset); };
  bool  GetBiasOn(int mod) { return (fPBConfig.Modul[mod].BiasOn); };

  float GetAVDDUncalibrated(int mod) { return (fPBConfig.Modul[mod].AVset); };
  float GetDVDDUncalibrated(int mod) { return (fPBConfig.Modul[mod].DVset); };

  void GetModuleSetUp(int mod, float *AVSet, float *AISet, float *DVSet, float *DISet,
                      bool *isBiasOn);
  void GetAnalogVoltages(float *AVSet);
  void GetDigitalVoltages(float *DVSet);
  void GetAnalogCurrents(float *AISet);
  void GetDigitalCurrents(float *DISet);
  void GetBiasOnSets(bool *BIASOn);
  void GetVCalibration(int mod, float &AVScale, float &DVScale, float &AVOffset, float &DVOffset);
  void SetVCalibration(int mod, float AVScale, float DVScale, float AVOffset, float DVOffset);
  void GetICalibration(int mod, float &AIOffset, float &DIOffset);
  void SetICalibration(int mod, float AIOffset, float DIOffset);
  void SetVBiasCalibration(float AScale, float AOffset);
  void SetIBiasCalibration(float AOffset);
  void GetVBiasCalibration(float &AScale, float &AOffset);
  void GetIBiasCalibration(float &AOffset);
  void SetLineResistances(int mod, float ALineR, float DLineR, float GNDLineR,
                          float AGNDLineR = -1.);
  void EnterMeasuredLineResistances(int mod, float ALineR, float DLineR, float GNDLineR,
                                    float AGNDLineR = -1.);
  void GetWirePBResistances(int mod, float &ALineR, float &DLineR, float &GNDLineR, float &BBLineR);
  void GetResistances(int mod, float &ALineR, float &DLineR, float &GNDLineR, float &AGNDLineR,
                      pb_t pb);
  bool IsCalibrated(int mod);
  bool BiasEnabled(int mod) { return fPBConfig.Modul[mod].BiasEnabled; };
  void DisableBias(int mod) { fPBConfig.Modul[mod].BiasEnabled = false; };
  void WriteCalibrationFile();
  void ReadCalibrationFile();
  int  CheckFileFormat(std::string fName);
  // Setters
  void SetBiasVoltage(float val) { fPBConfig.VBset = val; };

  void ModuleSetUp(int mod, float AVSet, float AISet, float DVSet, float DISet, bool isBiasOn);
  void SetAnalogVoltage(int mod, float val) { fPBConfig.Modul[mod].AVset = val; };
  void SetAnalogCurrent(int mod, float val) { fPBConfig.Modul[mod].AIset = val; };
  void SetDigitalVoltage(int mod, float val) { fPBConfig.Modul[mod].DVset = val; };
  void SetDigitalCurrent(int mod, float val) { fPBConfig.Modul[mod].DIset = val; };
  void SetBiasOn(int mod, bool val) { fPBConfig.Modul[mod].BiasOn = val; };

  void SetDefaultsOB(int mod);
  void SetDefaultsIB(int mod);
  // Utilities
  bool ReadFromFile(char *AFileName);
  bool WriteToFile(char *AFileName);
  bool DumpConfig() { return false; }; // TODO: not yet implemented
  bool GetIsBottom() { return (m_bottom == 1); };
  void SetIsBottom(bool bottom) { m_bottom = bottom ? 1 : 0; };
  int  GetBoard() { return m_board; }; // which MOSAIC is the PB attached to?
  void SetBoard(int board) { m_board = board; };
  void InitParamMap();
  bool SetParamValue(std::string Name, std::string Value);
  int  GetParamValue(std::string Name);
  bool IsParameter(std::string Name) { return (fSettings.count(Name) > 0); };

private:
  void readConfiguration();
};

#endif /* BOARDCONFIGMOSAIC_H */

// ---------------- eof -------------------------
