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
 *		TPowerBoardConfig class implementation
 *
 *		ver.1.0		12/07/2017
 *
 *
 *  		HISTORY
 *
 *
 */
#include <algorithm>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "TPowerBoardConfig.h"

using namespace std;

/* -------------------------
        Constructor

        Parameter : AConfigFileName := Path and filename of a configuration ASCII file
  -------------------------- */
TPowerBoardConfig::TPowerBoardConfig(const char *AConfigFileName)
{
  fBoardType = boardMOSAIC;

  // Default values set
  fPBConfig.VBset          = DEF_BIASVOLTAGE;
  fPBConfig.CalBiasOffset  = DEF_BIASOFFSET;
  fPBConfig.CalBiasScale   = DEF_BIASSCALE;
  fPBConfig.CalIBiasOffset = DEF_IBIASOFFSET;
  for (int i = 0; i < MAX_MOULESPERMOSAIC; i++) {
    fPBConfig.Modul[i].AVset        = DEF_ANALOGVOLTAGE;
    fPBConfig.Modul[i].AIset        = DEF_ANALOGMAXCURRENT;
    fPBConfig.Modul[i].DVset        = DEF_DIGITALVOLTAGE;
    fPBConfig.Modul[i].DIset        = DEF_DIGITALMAXCURRENT;
    fPBConfig.Modul[i].BiasOn       = DEF_BIASCHANNELON;
    fPBConfig.Modul[i].BiasEnabled  = true;
    fPBConfig.Modul[i].CalAVScale   = DEF_AVSCALE;
    fPBConfig.Modul[i].CalDVScale   = DEF_DVSCALE;
    fPBConfig.Modul[i].CalAVOffset  = DEF_AVOFFSET;
    fPBConfig.Modul[i].CalDVOffset  = DEF_DVOFFSET;
    fPBConfig.Modul[i].CalDIOffset  = DEF_DIOFFSET;
    fPBConfig.Modul[i].CalAIOffset  = DEF_AIOFFSET;
    fPBConfig.Modul[i].CalDLineR    = DEF_CALDLINER;
    fPBConfig.Modul[i].CalALineR    = DEF_CALALINER;
    fPBConfig.Modul[i].CalGNDLineR  = DEF_CALGNDLINER;
    fPBConfig.Modul[i].CalAGNDLineR = DEF_CALAGNDLINER;
  }

  if (AConfigFileName) { // Read Configuration file
    try {
      if (AConfigFileName == NULL || strlen(AConfigFileName) == 0)
        throw std::invalid_argument("MOSAIC Config : invalid filename");
      fhConfigFile = fopen(AConfigFileName, "r"); // opens the file
    }
    catch (...) {
      throw std::invalid_argument("Power Board Config : file not exists !");
    }
    readConfiguration();
    fclose(fhConfigFile);
  }

  m_bottom = DEF_BOTTOM;
  m_board  = DEF_BOARD;
  InitParamMap();
}

/* -------------------------
        readConfiguration()
        Read the power board configuration from an ASCII file.

        Format : ASCII file with '\n' line termination

        # .....   := Comment line
        <PARAMETER_NAME>\t<VALUE>  := Single value parameter
        <PARAMETER_NAME>\t<VALUE_LIST> := Multi value parameters (for each board module)

        <VALUE_LIST> := <VALUE>[<DELIMITER><VALUE>..]
        <DELIMIDER> := [ ',' | ";" | ":" | "\t' | " "]

        List of Parameters:
        BIASVOLTAGE := single value, float. The voltage value of the Back Bias Output
        ANALOGVOLTAGE := multi value, float. The voltage of analogic source for the 8 modules
        ANALOGCURRENT := multi value, float. The maximum current of analogic source for the 8
  modules
        DIGITALVOLTAGE := multi value, float. The voltage of digital source for the 8 modules
        DIGITALCURRENT := multi value, float. The maximum current of digital source for the 8
  modules
        BIASON := multi value, bool ("TRUE" | "FALSE"). The switch ON/OFF of the Back Bias line for
  the 8 modules


  -------------------------- */
void TPowerBoardConfig::readConfiguration()
{
  char  buffer[4096];
  char  sName[4096];
  char  sParam[4096];
  char *tok, *ptr;
  int   p;

  if (!fgets(buffer, 4095, fhConfigFile)) {
    return;
  }
  while (!feof(fhConfigFile)) {
    if (strlen(buffer) > 0 && buffer[0] != '#') { // a good line
      if (fscanf(fhConfigFile, "%s\t%s", sName, sParam) == 2) {
        if (strcasecmp(sName, "BIASVOLTAGE") == 0) {
          fPBConfig.VBset = atof(sParam);
        }
        if (strcasecmp(sName, "ANALOGVOLTAGE") == 0) {
          p   = 0;
          ptr = sParam;
          while ((tok = strsep(&ptr, " ,:;\t")) != NULL)
            fPBConfig.Modul[p++].AVset = atof(tok);
          while (p < MAX_MOULESPERMOSAIC && p > 0) {
            fPBConfig.Modul[p].AVset = fPBConfig.Modul[p - 1].AVset;
            p++;
          }
        }
        if (strcasecmp(sName, "ANALOGCURRENT") == 0) {
          p   = 0;
          ptr = sParam;
          while ((tok = strsep(&ptr, " ,:;\t")) != NULL)
            fPBConfig.Modul[p++].AIset = atof(tok);
          while (p < MAX_MOULESPERMOSAIC && p > 0) {
            fPBConfig.Modul[p].AIset = fPBConfig.Modul[p - 1].AIset;
            p++;
          }
        }
        if (strcasecmp(sName, "DIGITALVOLTAGE") == 0) {
          p   = 0;
          ptr = sParam;
          while ((tok = strsep(&ptr, " ,:;\t")) != NULL)
            fPBConfig.Modul[p++].DVset = atof(tok);
          while (p < MAX_MOULESPERMOSAIC && p > 0) {
            fPBConfig.Modul[p].DVset = fPBConfig.Modul[p - 1].DVset;
            p++;
          }
        }
        if (strcasecmp(sName, "DIGITALCURRENT") == 0) {
          p   = 0;
          ptr = sParam;
          while ((tok = strsep(&ptr, " ,:;\t")) != NULL)
            fPBConfig.Modul[p++].DIset = atof(tok);
          while (p < MAX_MOULESPERMOSAIC && p > 0) {
            fPBConfig.Modul[p].DIset = fPBConfig.Modul[p - 1].DIset;
            p++;
          }
        }
        if (strcasecmp(sName, "BIASON") == 0) {
          p   = 0;
          ptr = sParam;
          while ((tok = strsep(&ptr, " ,:;\t")) != NULL)
            fPBConfig.Modul[p++].BiasOn = (strcasecmp(tok, "TRUE") == 0 ? true : false);
          while (p < MAX_MOULESPERMOSAIC && p > 0) {
            fPBConfig.Modul[p].BiasOn = fPBConfig.Modul[p - 1].BiasOn;
            p++;
          }
        }
      }
    }
    if (!fgets(buffer, 4095, fhConfigFile)) {
      return;
    }
  }
}

void TPowerBoardConfig::InitParamMap()
{
  fSettings["PBBOTTOM"] = &m_bottom;
  fSettings["PBBOARD"]  = &m_board;
}

bool TPowerBoardConfig::SetParamValue(std::string Name, std::string Value)
{
  if (fSettings.find(Name) != fSettings.end()) {
    *(fSettings.find(Name)->second) = std::stoi(Value);
    return true;
  }

  return false;
}

int TPowerBoardConfig::GetParamValue(std::string Name)
{
  if (fSettings.find(Name) != fSettings.end()) {
    return *(fSettings.find(Name)->second);
  }
  return -1;
}

/* -------------------------
        GetModuleSetUp()
        Returns the settings for one specified module.

        Parameter In : mod, integer (0..7) The number of module to be read
        Paramer Out : AVSet, pointer to a float variable. Analogic source output voltage
                                AISet, pointer to a float variable. Analogic source output current
  limit
                                DVSet, pointer to a float variable. Digital source output voltage
                                DISet, pointer to a float variable. Digital source output current
  limit
                                isBiasOn, pointer to a bool variable. On/Off of the Back Bias
  channel

  -------------------------- */
void TPowerBoardConfig::GetModuleSetUp(int mod, float *AVSet, float *AISet, float *DVSet,
                                       float *DISet, bool *isBiasOn)
{
  *AVSet    = GetAnalogVoltage(mod);
  *AISet    = fPBConfig.Modul[mod].AIset;
  *DVSet    = GetDigitalVoltage(mod);
  *DISet    = fPBConfig.Modul[mod].DIset;
  *isBiasOn = fPBConfig.Modul[mod].BiasOn;
  return;
}

/* -------------------------
        ModuleSetUp()
        Setup for one specified module.

        Parameter In : mod, integer (0..7) The number of module to be read
                                AVSet, float. Analogic source output voltage
                                AISet, float. Analogic source output current limit
                                DVSet, float. Digital source output voltage
                                DISet, float. Digital source output current limit
                                isBiasOn, bool. On/Off of the Back Bias channel

  -------------------------- */
void TPowerBoardConfig::ModuleSetUp(int mod, float AVSet, float AISet, float DVSet, float DISet,
                                    bool isBiasOn)
{
  fPBConfig.Modul[mod].AVset  = AVSet;
  fPBConfig.Modul[mod].AIset  = AISet;
  fPBConfig.Modul[mod].DVset  = DVSet;
  fPBConfig.Modul[mod].DIset  = DISet;
  fPBConfig.Modul[mod].BiasOn = isBiasOn;
  return;
}

/* -------------------------
        GetAnalogVoltages()
        Returns the array of Analogic Voltage setting for all 8 modules, including calibration

        Parameter Output : pointer to a float array of 8 elements

  -------------------------- */
void TPowerBoardConfig::GetAnalogVoltages(float *AVSet)
{
  for (int i = 0; i < MAX_MOULESPERMOSAIC; i++) {
    *(AVSet++) = GetAnalogVoltage(i);
  }
  return;
}

float TPowerBoardConfig::GetAnalogVoltage(int mod)
{
  return (fPBConfig.Modul[mod].AVset * fPBConfig.Modul[mod].CalAVScale +
          fPBConfig.Modul[mod].CalAVOffset);
}

/* -------------------------
        GetDigitalVoltages()
        Returns the array of Digital Voltage setting for all 8 modules, including calibration

        Parameter Output : pointer to a float array of 8 elements

  -------------------------- */
void TPowerBoardConfig::GetDigitalVoltages(float *DVSet)
{
  for (int i = 0; i < MAX_MOULESPERMOSAIC; i++) {
    *(DVSet++) = GetDigitalVoltage(i);
  }
  return;
}

float TPowerBoardConfig::GetDigitalVoltage(int mod)
{
  return (fPBConfig.Modul[mod].DVset * fPBConfig.Modul[mod].CalDVScale +
          fPBConfig.Modul[mod].CalDVOffset);
}

/* -------------------------
        GetAnalogCurrents()
        Returns the array of Analogic Current limits setting for all 8 modules

        Parameter Output : pointer to a float array of 8 elements

  -------------------------- */
void TPowerBoardConfig::GetAnalogCurrents(float *AISet)
{
  for (int i = 0; i < MAX_MOULESPERMOSAIC; i++) {
    *(AISet++) = fPBConfig.Modul[i].AIset;
  }
  return;
}

/* -------------------------
        GetDigitalCurrents()
        Returns the array of Digital Current limits setting for all 8 modules

        Parameter Output : pointer to a float array of 8 elements

  -------------------------- */
void TPowerBoardConfig::GetDigitalCurrents(float *DISet)
{
  for (int i = 0; i < MAX_MOULESPERMOSAIC; i++) {
    *(DISet++) = fPBConfig.Modul[i].DIset;
  }
  return;
}

/* -------------------------
        GetBiasOnSets()
        Returns the array of Back Bias On/Off setting for all 8 modules

        Parameter Output : pointer to a bool array of 8 elements

  -------------------------- */
void TPowerBoardConfig::GetBiasOnSets(bool *BIASOn)
{
  for (int i = 0; i < MAX_MOULESPERMOSAIC; i++) {
    *(BIASOn++) = fPBConfig.Modul[i].BiasOn;
  }
  return;
}

// GetBiasVoltage: returns bias voltage including calibration
float TPowerBoardConfig::GetBiasVoltage()
{
  return (fPBConfig.VBset * fPBConfig.CalBiasScale + fPBConfig.CalBiasOffset);
}

void TPowerBoardConfig::GetVCalibration(int mod, float &AVScale, float &DVScale, float &AVOffset,
                                        float &DVOffset)
{
  AVScale  = fPBConfig.Modul[mod].CalAVScale;
  DVScale  = fPBConfig.Modul[mod].CalDVScale;
  AVOffset = fPBConfig.Modul[mod].CalAVOffset;
  DVOffset = fPBConfig.Modul[mod].CalDVOffset;
}

void TPowerBoardConfig::SetVCalibration(int mod, float AVScale, float DVScale, float AVOffset,
                                        float DVOffset)
{
  fPBConfig.Modul[mod].CalAVScale  = AVScale;
  fPBConfig.Modul[mod].CalDVScale  = DVScale;
  fPBConfig.Modul[mod].CalAVOffset = AVOffset;
  fPBConfig.Modul[mod].CalDVOffset = DVOffset;
}

void TPowerBoardConfig::GetICalibration(int mod, float &AIOffset, float &DIOffset)
{
  AIOffset = fPBConfig.Modul[mod].CalAIOffset;
  DIOffset = fPBConfig.Modul[mod].CalDIOffset;
}

void TPowerBoardConfig::SetICalibration(int mod, float AIOffset, float DIOffset)
{
  fPBConfig.Modul[mod].CalAIOffset = AIOffset;
  fPBConfig.Modul[mod].CalDIOffset = DIOffset;
}

void TPowerBoardConfig::SetVBiasCalibration(float AScale, float AOffset)
{
  fPBConfig.CalBiasOffset = AOffset;
  fPBConfig.CalBiasScale  = AScale;
}

void TPowerBoardConfig::SetIBiasCalibration(float AOffset) { fPBConfig.CalIBiasOffset = AOffset; }

void TPowerBoardConfig::GetVBiasCalibration(float &AScale, float &AOffset)
{
  AOffset = fPBConfig.CalBiasOffset;
  AScale  = fPBConfig.CalBiasScale;
}

void TPowerBoardConfig::GetIBiasCalibration(float &AOffset) { AOffset = fPBConfig.CalIBiasOffset; }

// sets the line resistances in the calibration part of the configuration
// expects the external resistances (breakout board -> module) and adds the internal ones

// TODO: substitute powerUnit by Top/Bottom variable
void TPowerBoardConfig::EnterMeasuredLineResistances(int mod, float ALineR, float DLineR,
                                                     float GNDLineR, float AGNDLineR)
{
  int powerUnit                     = ((m_bottom == 0) ? 1 : 0);
  fPBConfig.Modul[mod].CalDLineR    = DLineR + RDigital[powerUnit][mod];
  fPBConfig.Modul[mod].CalALineR    = ALineR + RAnalog[powerUnit][mod];
  fPBConfig.Modul[mod].CalGNDLineR  = GNDLineR;
  fPBConfig.Modul[mod].CalAGNDLineR = AGNDLineR;
}

void TPowerBoardConfig::GetWirePBResistances(int mod, float &ALineR, float &DLineR, float &GNDLineR,
                                             float &BBLineR)
{
  float x = mod + 1.;

  ALineR   = 7.9286 * x + 51.571;
  DLineR   = 8.25 * x + 45.714;
  GNDLineR = 4.5714 * x + 31.143;
  BBLineR  = 17.929 * x + 184.43;

  ALineR /= 1000.;
  DLineR /= 1000.;
  GNDLineR /= 1000.;
  BBLineR /= 1000.;
}

// sets the line resistances in the calibration part of the configuration
// WITHOUT adding the internal resistances (used when read from file)
void TPowerBoardConfig::SetLineResistances(int mod, float ALineR, float DLineR, float GNDLineR,
                                           float AGNDLineR)
{
  fPBConfig.Modul[mod].CalDLineR    = DLineR;
  fPBConfig.Modul[mod].CalALineR    = ALineR;
  fPBConfig.Modul[mod].CalGNDLineR  = GNDLineR;
  fPBConfig.Modul[mod].CalAGNDLineR = AGNDLineR;
}

// GetLineResistances returns the total value of the line resistance (internal + external)
void TPowerBoardConfig::GetLineResistances(int mod, float &ALineR, float &DLineR, float &GNDLineR,
                                           float &AGNDLineR)
{
  DLineR    = fPBConfig.Modul[mod].CalDLineR;
  ALineR    = fPBConfig.Modul[mod].CalALineR;
  GNDLineR  = fPBConfig.Modul[mod].CalGNDLineR;
  AGNDLineR = fPBConfig.Modul[mod].CalAGNDLineR;
}

void TPowerBoardConfig::GetResistances(int mod, float &ALineR, float &DLineR, float &GNDLineR,
                                       float &AGNDLineR, pb_t pb)
{

  GetLineResistances(mod, ALineR, DLineR, GNDLineR, AGNDLineR);

  if (pb == TPowerBoardConfig::realML) {
    DLineR += RPBDigitalML[mod];
    ALineR += RPBAnalogML[mod];
    GNDLineR += RPBGroundML[mod];
  }
  else if (pb == TPowerBoardConfig::realOL) {
    DLineR += RPBDigital[mod];
    ALineR += RPBAnalog[mod];
    GNDLineR += RPBGround[mod];
  }
  else if (pb == TPowerBoardConfig::mockup) {
    float Ra, Rd, Rgnd, Rbb;
    GetWirePBResistances(mod, Ra, Rd, Rgnd, Rbb);
    DLineR += Rd;
    ALineR += Ra;
    GNDLineR += Rgnd;
  }
  else if (pb != TPowerBoardConfig::none) {
    std::cout << "WARNING: Unsupported power bus" << std::endl;
  }
}

// TODO: change filename to PBCalibTop.cfg, PBCalibBottom.cfg
// requires: correct setting of PBBOTTOM in config file
void TPowerBoardConfig::WriteCalibrationFile()
{
  float ALineR, DLineR, GNDLineR, AGNDLineR, AIOffset, DIOffset, AVScale, DVScale, AVOffset,
      DVOffset;
  float VBScale, VBOffset, IBOffset;

  std::string filename = m_bottom ? "PBBottomCalib.cfg" : "PBTopCalib.cfg";
  if (const char *cfgDir = std::getenv("ALPIDE_TEST_CONFIG"))
    filename.insert(0, std::string(cfgDir) + "/");
  FILE *fp = fopen(filename.c_str(), "w");

  for (int imod = 0; imod < MAX_MOULESPERMOSAIC; imod++) {
    GetLineResistances(imod, ALineR, DLineR, GNDLineR, AGNDLineR);
    GetICalibration(imod, AIOffset, DIOffset);
    GetVCalibration(imod, AVScale, DVScale, AVOffset, DVOffset);
    if (AGNDLineR > 0) {
      fprintf(fp, "%d %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f\n", imod, ALineR, DLineR,
              GNDLineR, AGNDLineR, AIOffset, DIOffset, AVScale, AVOffset, DVScale, DVOffset);
    }
    else {
      fprintf(fp, "%d %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f\n", imod, ALineR, DLineR,
              GNDLineR, AIOffset, DIOffset, AVScale, AVOffset, DVScale, DVOffset);
    }
  }
  GetVBiasCalibration(VBScale, VBOffset);
  GetIBiasCalibration(IBOffset);
  fprintf(fp, "%.3f %.3f %.3f\n", IBOffset, VBScale, VBOffset);
  fclose(fp);
}


int TPowerBoardConfig::CheckFileFormat(string fName)
{
  FILE *fp = fopen(fName.c_str(), "r");
  if (fp) {
    char line[100];
    if (fgets(line, 100, fp) == NULL) {
      std::cout << "Warning: unable to determine calibration file format, using default"
                << std::endl;
      fclose(fp);
      return 9;
    }
    string test    = string(line);
    int    nBlanks = (int)std::count(test.begin(), test.end(), ' ');
    fclose(fp);
    return nBlanks;
  }
  else {
    return 9;
  }
}


void TPowerBoardConfig::ReadCalibrationFile()
{
  float ALineR, DLineR, AGNDLineR = -1, GNDLineR, AIOffset, DIOffset, AVScale, DVScale, AVOffset,
                        DVOffset;
  float VBScale, VBOffset, IBOffset;
  int   mod;
  int   nLines = 0;

  std::string filename = m_bottom ? "PBBottomCalib.cfg" : "PBTopCalib.cfg";
  if (const char *cfgDir = std::getenv("ALPIDE_TEST_CONFIG"))
    filename.insert(0, std::string(cfgDir) + "/");
  int   nPars = CheckFileFormat(filename) + 1;
  FILE *fp    = fopen(filename.c_str(), "r");

  if (!fp) {
    std::cout << "No calibration file found" << std::endl;
    return;
  }
  for (int imod = 0; imod < MAX_MOULESPERMOSAIC; imod++) {
    if (((nPars == 10) &&
         (fscanf(fp, "%d %f %f %f %f %f %f %f %f %f\n", &mod, &ALineR, &DLineR, &GNDLineR,
                 &AIOffset, &DIOffset, &AVScale, &AVOffset, &DVScale, &DVOffset) == 10)) ||
        ((nPars == 11) && (fscanf(fp, "%d %f %f %f %f %f %f %f %f %f %f\n", &mod, &ALineR, &DLineR,
                                  &GNDLineR, &AGNDLineR, &AIOffset, &DIOffset, &AVScale, &AVOffset,
                                  &DVScale, &DVOffset) == 11))) {
      SetLineResistances(mod, ALineR, DLineR, GNDLineR, AGNDLineR);
      SetICalibration(mod, AIOffset, DIOffset);
      SetVCalibration(mod, AVScale, DVScale, AVOffset, DVOffset);
      nLines++;
    }
    else {
      std::cout << "WARNING: calibration file corrupt, read " << nLines << " lines instead of "
                << MAX_MOULESPERMOSAIC << std::endl;
    }
  }
  if (fscanf(fp, "%f %f %f\n", &IBOffset, &VBScale, &VBOffset) == 3) {
    SetVBiasCalibration(VBScale, VBOffset);
    SetIBiasCalibration(IBOffset);
  }
  else {
    std::cout << "WARNING: back bias calibration not found, using default" << std::endl;
  }
  fclose(fp);
}

bool TPowerBoardConfig::IsCalibrated(int mod)
{
  float DLineR, ALineR, GNDLineR, AGNDLineR;
  GetLineResistances(mod, ALineR, DLineR, GNDLineR, AGNDLineR);
  if (DLineR + ALineR + GNDLineR == 0) return false;
  return true;
}

/* -------------------------
        ReadFromFile()
        Read a complete configuration from a file and stores it into the class members

        Parameter input : char pointer to a string that specify path and filename of the
  Configuration file

        Return : false in case of error

  -------------------------- */
bool TPowerBoardConfig::ReadFromFile(char *AFileName)
{
  if (AFileName != NULL && strlen(AFileName) != 0) {
    fhConfigFile = fopen(AFileName, "r"); // opens the file
    if (fhConfigFile != NULL) {
      readConfiguration();
      fclose(fhConfigFile);
      return (true);
    }
    else {
      cerr << "Error opening the Config file for read !" << endl;
    }
  }
  else {
    cerr << "Bad Config file name for read !" << endl;
  }
  return (false);
}

/* -------------------------
        WriteToFile()
        Write a complete configuration from the class members to a configuration file

        Parameter input : char pointer to a string that specify path and filename of the
  Configuration file

        Return : false in case of error

  -------------------------- */
bool TPowerBoardConfig::WriteToFile(char *AFileName)
{
  int i;
  if (AFileName != NULL && strlen(AFileName) != 0) {
    fhConfigFile = fopen(AFileName, "w"); // opens the file
    if (fhConfigFile != NULL) {
      fprintf(fhConfigFile, "# ALPIDE POWER BOARD CONFIGURATION  - v0.1\n");
      fprintf(fhConfigFile, "BIASVOLTAGE\t%f\n", fPBConfig.VBset);
      fprintf(fhConfigFile, "# PARAM.\tM0\tM1\tM2\tM3\tM4\tM5\tM6\tM7");
      fprintf(fhConfigFile, "\nANALOGVOLTAGE");
      for (i = 0; i < MAX_MOULESPERMOSAIC; i++)
        fprintf(fhConfigFile, "\t%f", fPBConfig.Modul[i].AVset);
      fprintf(fhConfigFile, "\nANALOGCURRENT");
      for (i = 0; i < MAX_MOULESPERMOSAIC; i++)
        fprintf(fhConfigFile, "\t%f", fPBConfig.Modul[i].AIset);
      fprintf(fhConfigFile, "\nDIGITALVOLTAGE");
      for (i = 0; i < MAX_MOULESPERMOSAIC; i++)
        fprintf(fhConfigFile, "\t%f", fPBConfig.Modul[i].DVset);
      fprintf(fhConfigFile, "\nDIGITALCURRENT");
      for (i = 0; i < MAX_MOULESPERMOSAIC; i++)
        fprintf(fhConfigFile, "\t%f", fPBConfig.Modul[i].DIset);
      fprintf(fhConfigFile, "\nBIASON");
      for (i = 0; i < MAX_MOULESPERMOSAIC; i++)
        fprintf(fhConfigFile, "\t%s", (fPBConfig.Modul[i].BiasOn ? "TRUE" : "FALSE"));
      fprintf(fhConfigFile, "\n# --- eof ----\n");
      fclose(fhConfigFile);
      return (true);
    }
    else {
      cerr << "Error opening the Config file for write !" << endl;
    }
  }
  else {
    cerr << "Bad Config file name for write !" << endl;
  }
  return (false);
}

void TPowerBoardConfig::SetDefaultsOB(int mod)
{
  fPBConfig.Modul[mod].AVset  = DEF_ANALOGVOLTAGE_OB;
  fPBConfig.Modul[mod].AIset  = DEF_ANALOGMAXCURRENT_OB;
  fPBConfig.Modul[mod].DVset  = DEF_DIGITALVOLTAGE_OB;
  fPBConfig.Modul[mod].DIset  = DEF_DIGITALMAXCURRENT_OB;
  fPBConfig.Modul[mod].BiasOn = DEF_BIASCHANNELON_OB;

  fPBConfig.VBset = DEF_BIASVOLTAGE_OB;
}

void TPowerBoardConfig::SetDefaultsIB(int mod)
{
  fPBConfig.Modul[mod].AVset  = DEF_ANALOGVOLTAGE_IB;
  fPBConfig.Modul[mod].AIset  = DEF_ANALOGMAXCURRENT_IB;
  fPBConfig.Modul[mod].DVset  = DEF_DIGITALVOLTAGE_IB;
  fPBConfig.Modul[mod].DIset  = DEF_DIGITALMAXCURRENT_IB;
  fPBConfig.Modul[mod].BiasOn = DEF_BIASCHANNELON_IB;

  fPBConfig.VBset = DEF_BIASVOLTAGE_IB;
}

// ------------------  eof -----------------------
