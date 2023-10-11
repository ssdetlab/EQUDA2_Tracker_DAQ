#ifndef TSCANCONFIG_H
#define TSCANCONFIG_H

#include <cmath>
#include <map>
#include <string.h>
#include <string>

enum TDeviceType {
  TYPE_CHIP,
  TYPE_TELESCOPE,
  TYPE_OBHIC,
  TYPE_IBHIC,
  TYPE_CHIP_MOSAIC,
  TYPE_HALFSTAVE,
  TYPE_HALFSTAVERU,
  TYPE_MLHALFSTAVE,
  TYPE_MLSTAVE,
  TYPE_IBHICRU,
  TYPE_ENDURANCE,
  TYPE_POWER,
  TYPE_UNKNOWN
};

typedef enum {
  OBQualification,
  OBEndurance,
  OBReception,
  OBPower,
  OBHalfStaveOL,
  OBHalfStaveML,
  IBQualification,
  IBEndurance,
  IBStave,
  IBStaveEndurance,
  IBDctrl,
  OBHalfStaveOLFAST,
  OBHalfStaveMLFAST,
  OBStaveOL,
  OBStaveML,
  StaveReceptionOL,
  StaveReceptionML,
  OBStaveOLFAST,
  OBStaveMLFAST,
  IBStaveLayerQualification,
  Unknown
} TTestType;

namespace ScanConfig {
  // 0.1: first, initial version of "new classification"
  // 0.2: lowered back bias limit to 15 mA, removed cut on 4 V current
  // 0.3: introduced cut on noisy pixels, set cut in FIFO errors to 0
  // 1.0: first version of new classification
  // 1.1: new classification, but treat bronze equal to gold, silver
  //      (no longer in partial b, no back bias b)
  //      allow more than two bad chips for partial cat B
  // 1.2: using Vclip=60DAC for all -3V scans
  // 1.3: move back to max. 2 bad chips for partial cat B (cf. plenary June 18, 2018)
  // 1.4: increased gravity of failed chi square cut in DCTRL Test (bronze -> red)
  // 1.5: reduced cycles in endurance test /10, reduced cut on failures 30 -> 3
  // 1.51: introduced FIFO test in endurance cycle (no cut yet)
  // 1.6: bug fix in the double column masking code
  // 1.7: only unmasking the double columns at the beginning of the mask scan (caused by MOSAIC
  // issues)
  // 1.8: shortened endurance test, added parameter for counting of exceptions
  // 1.9: checking DCTRL measurement values for plausibility and excluding them if necessary.
  // 2.0: introduced separate DCTRL cut for (half-)staves (lower slope and amplitude)
  // 2.1: Do not consider impedance test result any longer
  // 2.2: reduced number of endurance test cycles from 150 -> 130 to go from 3.5 to 3 days
  // 2.3: bug fix in the TPowerTest.cpp, correction in chip configuration
  //      (leads to higher IDDD configured)
  //      introduced silver cat. in fast power test for IDDD > 300 mA
  // 2.4: introduced 1V back bias scan in HIC Qualification test (fail -> not working)

  const float CLASSIFICATION_VERSION = 2.4;

  // 1.1 bugfixes (local bus for disabled chips, forcing back bias at 1V)
  // 1.2 voltage drop correction for power bus
  // 1.4 VRESETP fix
  const float NUMBEROFTHEBEAST = 1.4;

  const int AUTOREPEAT = 0; // automatically repeat scans without user prompt
  const int MAXREPEAT  = 5; // max number of automatic repetitions
  const int RECOVERY   = 0; // read recovery file for endurance test
  const int RSYNC      = 1;

  const int NINJ           = 50;     // number of injections in digital/threshold scans
  const int NTRIG          = 100000; // number of triggers for noise occupancy scans
  const int CHARGE_START   = 0;
  const int CHARGE_STOP    = 50;
  const int CHARGE_STEP    = 1;
  const int N_MASK_STAGES  = 512;
  const int PIX_PER_REGION = 32;
  const int NOISECUT_INV   = 100000; // inverse of pixel noise cut (e.g. 100000 = 1e-5)
  const int MAXTIMEOUT     = 100;
  const int MAXHITS        = 40000000; // max number of hits in hit vector; 40 Mio ~ 1GB

  const int ITHR_START  = 30;
  const int ITHR_STOP   = 100;
  const int ITHR_STEP   = 1;
  const int VCASN_START = 30;
  const int VCASN_STOP  = 70;
  const int VCASN_STEP  = 1;
  const int SCAN_STEP   = 256; // Grab every Xth row (for tuneITHR/VCASN scan only).
  // Speeds up scan; changing this has little effect on result accuracy.
  const int TUNING_MAXROW  = 512;
  const int LOCALBUSCUTRED = 1;

  const int DAC_START         = 0;
  const int DAC_STOP          = 255;
  const int DAC_STEP          = 8;
  const int NDACSAMPLES       = 10;
  const int STATUS            = 0;
  const int TEST_WITHOUT_COMP = 0;
  // settings for readout test
  const int READOUTSPEED     = 1200;
  const int READOUTOCC       = 32;
  const int READOUTDRIVER    = 10;
  const int READOUTPREEMP    = 10;
  const int READOUTROW       = 0;
  const int READOUTPLLSTAGES = -1; // -1 meaning using the standard setting from the chip config

  // current limits for powering test in mA
  const int POWER_CUT_MINIDDA_OB     = 20;
  const int POWER_CUT_MINIDDD_OB     = 50;
  const int POWER_CUT_MAXIDDA_OB     = 250;  // for fast power test
  const int POWER_CUT_MAXIDDD_OB     = 1000; // for fast power test
  const int POWER_CUT_MAXIDDD_SILVER = 300;  // for fast power test
  const int POWER_CUT_MINIDDD_IB     = 50;
  const int POWER_CUT_MINIDDA_IB     = 20;

  const int POWER_CUT_MINIDDA_CLOCKED_OB = 120;
  const int POWER_CUT_MINIDDD_CLOCKED_OB = 500;
  const int POWER_CUT_MAXIDDA_CLOCKED_OB = 250;
  const int POWER_CUT_MAXIDDD_CLOCKED_OB = 850;

  const int POWER_CUT_MINIDDA_CLOCKED_IB = 70;
  const int POWER_CUT_MINIDDD_CLOCKED_IB = 300;
  const int POWER_CUT_MAXIDDA_CLOCKED_IB = 180;
  const int POWER_CUT_MAXIDDD_CLOCKED_IB = 550;

  const int POWER_CUT_MAXBIAS_3V_IB = 10;
  const int POWER_CUT_MAXBIAS_3V_OB = 10;

  const int POWER_MAXFACTOR_4V_IB = 3;
  const int POWER_MAXFACTOR_4V_OB = 3;

  // cuts for readout test
  const int READOUT_MAXTIMEOUT     = 0;
  const int READOUT_MAXCORRUPT     = 0;
  const int READOUT_MAX8b10b_GREEN = 0;

  // cuts for fifo test
  const int FIFO_CUT_MAXEXCEPTION  = 0; // max number of exceptions
  const int FIFO_CUT_MAXERR_ORANGE = 0; // max number of errors per pattern and hic
  const int FIFO_CUT_MAXERR_GREEN  = 0; // max number of errors per pattern and hic
  const int FIFO_CUT_MAXFAULTY     = 1; // max number of chips with errors

  // cuts for digital scan
  const int DIGITAL_MAXTIMEOUT_GREEN      = 0;
  const int DIGITAL_MAXCORRUPT_GREEN      = 0;
  const int DIGITAL_MAXTIMEOUT_ORANGE     = 0;
  const int DIGITAL_MAXCORRUPT_ORANGE     = 0;
  const int DIGITAL_MAXBAD_CHIP_OB        = 1024; // max number of bad pixels: 1 dcol
  const int DIGITAL_MAXBAD_CHIP_IB        = 524;  // 1 per mille
  const int DIGITAL_MAXDEAD_CHIP_GREEN    = 10;
  const int DIGITAL_MAXDEAD_CHIP_ORANGE   = 5120;
  const int DIGITAL_MAXDEAD_HIC_GREEN_OB  = 140;
  const int DIGITAL_MAXDEAD_HIC_GREEN_IB  = 90;
  const int DIGITAL_MAXDEAD_HIC_ORANGE_OB = 71680;
  const int DIGITAL_MAXDEAD_HIC_ORANGE_IB = 46080;

  const int DIGITAL_MAXBAD_CHIP_GOLD   = 50;
  const int DIGITAL_MAXBAD_CHIP_SILVER = 2100;
  const int DIGITAL_MAXBAD_CHIP_BRONZE = 5243;

  const int DIGITAL_MAXBAD_HIC_OB = 7340; // 1 per mille
  const int DIGITAL_MAXBAD_HIC_IB = 4700; // 1 per mille

  const int DIGITAL_MAXNOMASK_HIC_OB           = 7340; // 1 per mille?
  const int DIGITAL_MAXNOMASK_HIC_IB           = 4700; // 1 per mille?
  const int DIGITAL_MAXNOMASKSTUCK_HIC_OB      = 0;
  const int DIGITAL_MAXNOMASKSTUCK_HIC_IB      = 0;
  const int DIGITAL_MAXNOMASKSTUCK_CHIP_GOLD   = 0;
  const int DIGITAL_MAXNOMASKSTUCK_CHIP_SILVER = 2;
  const int DIGITAL_MAXNOMASKSTUCK_CHIP_BRONZE = 5;

  // cuts for threshold scan
  const int THRESH_MAXTIMEOUT_GREEN        = 0;
  const int THRESH_MAXCORRUPT_GREEN        = 0;
  const int THRESH_MAXTIMEOUT_ORANGE       = 1;
  const int THRESH_MAXCORRUPT_ORANGE       = 1;
  const int THRESH_MAXBAD_CHIP_OB          = 1024; // max number of bad pixels: 1 dcol
  const int THRESH_MAXBAD_CHIP_IB          = 524;  // 1 per mille
  const int THRESH_MAXBAD_HIC_OB           = 7340; // 1 per mille
  const int THRESH_MAXBAD_HIC_IB           = 4700; // 1 per mille
  const int THRESH_MAXNOISE_OB             = 10;   // max noise of a single chip
  const int THRESH_MAXNOISE_IB             = 10;
  const int THRESH_MAXDEAD_HIC_GREEN_OB    = 140;
  const int THRESH_MAXDEAD_HIC_GREEN_IB    = 90;
  const int THRESH_MAXDEAD_HIC_ORANGE_OB   = 71680;
  const int THRESH_MAXDEAD_HIC_ORANGE_IB   = 46080;
  const int THRESH_MAXDEAD_CHIP_GOLD       = 50;
  const int THRESH_MAXDEAD_CHIP_SILVER     = 2100;
  const int THRESH_MAXDEAD_CHIP_BRONZE     = 5243;
  const int THRESH_MAXNOTHRESH_CHIP_GOLD   = 5243;
  const int THRESH_MAXNOTHRESH_CHIP_SILVER = 26214;
  const int THRESH_MAXNOTHRESH_CHIP_BRONZE = 52429;

  const int MAXNOISY_CHIP_GOLD   = 50;
  const int MAXNOISY_CHIP_SILVER = 2100;
  const int MAXNOISY_CHIP_BRONZE = 5243;

  const int TEST_DCTRL = 1;
  // slope and amplitude cut values take into account losses on
  //  - FPCs and long firefly cable for OB HICs
  //  - long firefly cable for OB Staves
  const int   DCTRL_MINAMP_IB        = 150; // in mV
  const int   DCTRL_MINSLOPE_IB      = 10;  // in mV / DAC
  const int   DCTRL_MAXRISE_GREEN_IB = 10;  // in ns
  const int   DCTRL_MAXFALL_GREEN_IB = 10;
  const int   DCTRL_MINAMP_OB        = 300; // in mV
  const int   DCTRL_MINAMP_OBSTAVE   = 225; // in mV
  const int   DCTRL_MINSLOPE_OB      = 20;  // in mV / DAC
  const int   DCTRL_MINSLOPE_OBSTAVE = 15;  // in mV / DAC
  const int   DCTRL_MAXRISE_GREEN_OB = 10;  // in ns
  const int   DCTRL_MAXFALL_GREEN_OB = 10;
  const int   DCTRL_MAXCHISQ_SILVER  = 5; // 100 * max. chisq 5 -> 0.05
  const int   HALFSTAVE_COMPONENT    = 0;
  const int   SPEEDY                 = 1; // Use slow fit if 0, differentiate->mean if 1.
  const int   RAWDATA                = 0;
  const int   CAL_VPULSEL            = 160; // VPULSEH assumed 170.  Used for ITHR and VCASN scans.
  const int   TARGET_THRESHOLD       = 100;
  const int   IVCURVE                = 1; // Do I-V-curve on back bias
  const int   IVPOINTS      = 41; // number of 100 mV-points for back bias IV curve (max. 50 = 5V)
  const int   MAXIBIAS      = 15; // current limit for I-V-curve in mA;
  const float VOLTAGE_SCALE = 1.0;
  const float BACKBIAS      = 0;
  const int   NOMINAL       = 1;
  const int   ENDURANCE_SLICES = 10; // number of cycle slices
  const int   ENDURANCE_CYCLES = 13; // total number of cycles per slice
  const int   ENDURANCE_UPTIME =
      1600; // up and down wait time in seconds per cycle (originally: 60 +120)
  const int ENDURANCE_DOWNTIME           = 200;
  const int ENDURANCE_TRIGGERS           = 100000;
  const int ENDURANCE_LIMIT              = 8; // time limit in hours per slice
  const int ENDURANCE_MAXTRIPS_GREEN     = 0;
  const int ENDURANCE_MAXTRIPS_ORANGE    = 3; // approx. 1 per 1000 cycles
  const int ENDURANCE_MINCHIPS_GREEN     = 14;
  const int ENDURANCE_MAXFAILURES_ORANGE = 3; // approx. 1 per 100 cycles

  // MAX - MIN should be divisible by STEP
  // last point should be <= 128 for y and 127 for x (upper limit is excluded)
  const int EYE_DRIVER = 10;
  const int EYE_PREEMP = 10;
  const int EYE_MIN_X  = -128;
  const int EYE_MAX_X  = 132;
  const int EYE_STEP_X = 4;
  const int EYE_MIN_Y  = -127;
  const int EYE_MAX_Y  = 129;
  const int EYE_STEP_Y = 4;

} // namespace ScanConfig

class TScanConfig {
private:
  std::map<std::string, int *> fSettings;
  std::map<std::string, int>   m_retest;
  float                        m_classVersion;
  int                          m_nInj;
  int                          m_nTrig;
  int                          m_maxTimeout;
  int                          m_maxHits;
  int                          m_chargeStart;
  int                          m_chargeStop;
  int                          m_chargeStep;
  int                          m_dacStart;
  int                          m_dacStop;
  int                          m_dacStep;
  int                          m_nDacSamples;
  int                          m_nMaskStages;
  int                          m_pixPerRegion;
  int                          m_noiseCutInv;
  char                         m_fNameSuffix[80];
  char                         m_startTime[80];
  int                          m_testWithoutComp;
  int                          m_status;
  int                          m_halfstavecomp;
  int                          m_allowOld;
  // NEW--added for additional scans
  int       m_autorepeat;
  int       m_maxrepeat;
  int       m_recovery;
  int       m_rsync;
  int       m_ithrStart; // usually 30
  int       m_ithrStop;  // usually 100
  int       m_ithrStep;
  int       m_vcasnStart; // usually 40
  int       m_vcasnStop;  // usually 60
  int       m_vcasnStep;
  int       m_scanStep; // 16
  int       m_tuningMaxrow;
  int       m_speedy;
  int       m_rawData;
  int       m_ivCurve;
  int       m_ivPoints;
  int       m_maxIbias;
  int       m_localBusCutRed;
  int       m_powerCutMinIdda_OB;
  int       m_powerCutMinIddd_OB;
  int       m_powerCutMaxIdda_OB;
  int       m_powerCutMaxIddd_OB;
  int       m_powerCutMaxIdddSilver_OB;
  int       m_powerCutMinIddaClocked_OB;
  int       m_powerCutMinIdddClocked_OB;
  int       m_powerCutMaxIddaClocked_OB;
  int       m_powerCutMaxIdddClocked_OB;
  int       m_powerCutMinIdda_IB;
  int       m_powerCutMinIddd_IB;
  int       m_powerCutMinIddaClocked_IB;
  int       m_powerCutMinIdddClocked_IB;
  int       m_powerCutMaxIddaClocked_IB;
  int       m_powerCutMaxIdddClocked_IB;
  int       m_powerCutMaxBias3V_IB;
  int       m_powerCutMaxBias3V_OB;
  int       m_powerMaxFactor4V_IB;
  int       m_powerMaxFactor4V_OB;
  int       m_readoutMaxTimeout;
  int       m_readoutMaxCorrupt;
  int       m_readoutMax8b10bGreen;
  int       m_fifoCutMaxException;
  int       m_fifoCutMaxErrGreen;
  int       m_fifoCutMaxErrOrange;
  int       m_fifoCutMaxFaulty;
  int       m_digitalMaxTimeoutOrange;
  int       m_digitalMaxTimeoutGreen;
  int       m_digitalMaxCorruptOrange;
  int       m_digitalMaxCorruptGreen;
  int       m_digitalMaxBadPerChipOB;
  int       m_digitalMaxBadPerChipIB;
  int       m_digitalMaxBadPerHicOB;
  int       m_digitalMaxBadPerHicIB;
  int       m_digitalMaxBadChipGold;
  int       m_digitalMaxBadChipSilver;
  int       m_digitalMaxBadChipBronze;
  int       m_digitalMaxDeadPerChipGreen;
  int       m_digitalMaxDeadPerChipOrange;
  int       m_digitalMaxDeadPerHicGreenOB;
  int       m_digitalMaxDeadPerHicGreenIB;
  int       m_digitalMaxDeadPerHicOrangeOB;
  int       m_digitalMaxDeadPerHicOrangeIB;
  int       m_digitalMaxNoMaskHicIB;
  int       m_digitalMaxNoMaskHicOB;
  int       m_digitalMaxNoMaskStuckHicIB;
  int       m_digitalMaxNoMaskStuckHicOB;
  int       m_digitalMaxNoMaskStuckChipGold;
  int       m_digitalMaxNoMaskStuckChipSilver;
  int       m_digitalMaxNoMaskStuckChipBronze;
  int       m_threshMaxTimeoutOrange;
  int       m_threshMaxTimeoutGreen;
  int       m_threshMaxCorruptOrange;
  int       m_threshMaxCorruptGreen;
  int       m_threshMaxBadPerChipOB;
  int       m_threshMaxBadPerChipIB;
  int       m_threshMaxBadPerHicOB;
  int       m_threshMaxBadPerHicIB;
  int       m_threshMaxDeadPerHicGreenOB;
  int       m_threshMaxDeadPerHicGreenIB;
  int       m_threshMaxDeadPerHicOrangeOB;
  int       m_threshMaxDeadPerHicOrangeIB;
  int       m_threshMaxDeadChipGold;
  int       m_threshMaxDeadChipSilver;
  int       m_threshMaxDeadChipBronze;
  int       m_threshMaxNoThreshChipGold;
  int       m_threshMaxNoThreshChipSilver;
  int       m_threshMaxNoThreshChipBronze;
  int       m_threshMaxNoiseIB;
  int       m_threshMaxNoiseOB;
  int       m_maxNoisyChipGold;
  int       m_maxNoisyChipSilver;
  int       m_maxNoisyChipBronze;
  int       m_testDctrl;
  int       m_dctrlMinAmpOB;
  int       m_dctrlMinSlopeOB;
  int       m_dctrlMinAmpOBStave;
  int       m_dctrlMinSlopeOBStave;
  int       m_dctrlMaxRiseGreenOB;
  int       m_dctrlMaxFallGreenOB;
  int       m_dctrlMinAmpIB;
  int       m_dctrlMinSlopeIB;
  int       m_dctrlMaxRiseGreenIB;
  int       m_dctrlMaxFallGreenIB;
  int       m_dctrlMaxChisqSilver;
  int       m_calVpulsel;
  int       m_targetThresh;
  int       m_nominal;
  bool      m_isMasked;
  float     m_voltageScale;
  int       m_mlvdsStrength;
  float     m_backBias;
  bool      m_backBias_active;
  bool      m_useDataPath; // for compatibility with standalone scans, set true for GUI
  int       m_enduranceSlices;
  int       m_enduranceCycles;
  int       m_enduranceUptime;
  int       m_enduranceDowntime;
  int       m_enduranceTriggers;
  int       m_enduranceLimit;
  int       m_enduranceMaxtripsGreen;
  int       m_enduranceMaxtripsOrange;
  int       m_enduranceMinchipsGreen;
  int       m_enduranceMaxfailuresOrange;
  int       m_readoutSpeed;
  int       m_readoutOcc;
  int       m_readoutDriver;
  int       m_readoutPreemp;
  int       m_readoutRow;
  int       m_readoutPllStages;
  int       m_eyeDriver;
  int       m_eyePreemp;
  int       m_eyeMaxX;
  int       m_eyeMinX;
  int       m_eyeStepX;
  int       m_eyeMaxY;
  int       m_eyeMinY;
  int       m_eyeStepY;
  TTestType m_testType;

  TDeviceType m_deviceType;

protected:
public:
  TScanConfig();
  ~TScanConfig(){};
  void        InitParamMap();
  bool        SetParamValue(std::string Name, std::string Value);
  bool        SetParamValue(std::string Name, int Value);
  int         GetParamValue(std::string Name);
  std::string GetDataPath(std::string HicName);
  std::string GetTestDir();
  std::string GetRemoteHicPath(std::string HicName);
  bool        IsParameter(std::string Name) { return (fSettings.count(Name) > 0); };

  float GetClassificationVersion() { return m_classVersion; };

  int   GetRetestNumber(std::string hicName);
  int   GetNInj() { return m_nInj; };
  int   GetChargeStart() { return m_chargeStart; };
  int   GetChargeStep() { return m_chargeStep; };
  int   GetChargeStop() { return m_chargeStop; };
  int   GetNMaskStages() { return m_nMaskStages; };
  char *GetfNameSuffix() { return m_fNameSuffix; };
  char *GetStartTime() { return m_startTime; };
  int   GetIthrStart() { return m_ithrStart; };
  int   GetIthrStop() { return m_ithrStop; };
  int   GetIthrStep() { return m_ithrStep; };
  int   GetVcasnStart() { return m_vcasnStart; };
  int   GetVcasnStop() { return m_vcasnStop; };
  int   GetVcasnStep() { return m_vcasnStep; };
  int   GetScanStep() { return m_scanStep; };
  int   GetSpeedy() { return m_speedy; };
  int   GetLocalBusCutRed() { return m_localBusCutRed; };
  int   GetCalVpulsel() { return m_calVpulsel; };
  float GetVoltageScale() { return m_voltageScale; };
  int   GetMlvdsStrength() { return m_mlvdsStrength; };
  bool  GetIsMasked() { return m_isMasked; };
  float GetBackBias() { return m_backBias; };
  bool  GetUseDataPath() { return m_useDataPath; };
  void  SetRetestNumber(std::string hicName, int aRetest);
  void  SetfNameSuffix(const char *aSuffix) { strcpy(m_fNameSuffix, aSuffix); };
  void  SetVoltageScale(float aScale) { m_voltageScale = aScale; };
  void  SetMlvdsStrength(int aStrength) { m_mlvdsStrength = aStrength; };
  void  SetBackBias(float aVoltage) { m_backBias = fabs(aVoltage); };
  void  SetBackBiasActive(bool act = true) { m_backBias_active = act; }
  bool  IsBackBiasActive() const { return m_backBias_active; }
  bool  IsHalfStave();
  void  SetVcasnRange(int start, int stop)
  {
    m_vcasnStart = start;
    m_vcasnStop  = stop;
  };
  void      SetIsMasked(bool masked) { m_isMasked = masked; };
  void      SetUseDataPath(bool usePath) { m_useDataPath = usePath; };
  void      SetTestType(TTestType type) { m_testType = type; };
  TTestType GetTestType() { return m_testType; };

  void        SetDeviceType(TDeviceType type) { m_deviceType = type; }
  TDeviceType GetDeviceType() { return m_deviceType; }
};

#endif
