#include "TScanConfig.h"
// #include "DBHelpers.h"
#include "TChipConfig.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

using namespace ScanConfig;

TScanConfig::TScanConfig()
{
  m_retest.clear();
  // dummy values for first tests
  m_classVersion = CLASSIFICATION_VERSION;

  // m_db = 0;

  m_autorepeat = AUTOREPEAT;
  m_maxrepeat  = MAXREPEAT;
  m_recovery   = RECOVERY;
  m_rsync      = RSYNC;
  m_allowOld   = 0;

  m_nInj       = NINJ;
  m_nTrig      = NTRIG;
  m_maxTimeout = MAXTIMEOUT;
  m_maxHits    = MAXHITS;

  m_chargeStart      = CHARGE_START;
  m_chargeStop       = CHARGE_STOP;
  m_chargeStep       = CHARGE_STEP;
  m_nMaskStages      = N_MASK_STAGES;
  m_pixPerRegion     = PIX_PER_REGION;
  m_noiseCutInv      = NOISECUT_INV;
  m_vcasnStart       = VCASN_START;
  m_vcasnStop        = VCASN_STOP;
  m_vcasnStep        = VCASN_STEP;
  m_ithrStart        = ITHR_START;
  m_ithrStop         = ITHR_STOP;
  m_ithrStep         = ITHR_STEP;
  m_dacStart         = DAC_START;
  m_dacStop          = DAC_STOP;
  m_dacStep          = DAC_STEP;
  m_nDacSamples      = NDACSAMPLES;
  m_scanStep         = SCAN_STEP;
  m_tuningMaxrow     = TUNING_MAXROW;
  m_speedy           = SPEEDY;
  m_rawData          = RAWDATA;
  m_ivCurve          = IVCURVE;
  m_ivPoints         = IVPOINTS;
  m_maxIbias         = MAXIBIAS;
  m_localBusCutRed   = LOCALBUSCUTRED;
  m_calVpulsel       = CAL_VPULSEL;
  m_targetThresh     = TARGET_THRESHOLD;
  m_voltageScale     = VOLTAGE_SCALE;
  m_backBias         = BACKBIAS;
  m_backBias_active  = false;
  m_nominal          = NOMINAL;
  m_isMasked         = false;
  m_mlvdsStrength    = ChipConfig::DCTRL_DRIVER;
  m_testWithoutComp  = TEST_WITHOUT_COMP;
  m_status           = STATUS;
  m_readoutSpeed     = READOUTSPEED;
  m_readoutOcc       = READOUTOCC;
  m_readoutDriver    = READOUTDRIVER;
  m_readoutPreemp    = READOUTPREEMP;
  m_readoutRow       = READOUTROW;
  m_readoutPllStages = READOUTPLLSTAGES;

  m_powerCutMinIdda_OB        = POWER_CUT_MINIDDA_OB;
  m_powerCutMinIddd_OB        = POWER_CUT_MINIDDD_OB;
  m_powerCutMaxIdda_OB        = POWER_CUT_MAXIDDA_OB;
  m_powerCutMaxIddd_OB        = POWER_CUT_MAXIDDD_OB;
  m_powerCutMaxIdddSilver_OB  = POWER_CUT_MAXIDDD_SILVER;
  m_powerCutMinIddaClocked_OB = POWER_CUT_MINIDDA_CLOCKED_OB;
  m_powerCutMinIdddClocked_OB = POWER_CUT_MINIDDD_CLOCKED_OB;
  m_powerCutMaxIddaClocked_OB = POWER_CUT_MAXIDDA_CLOCKED_OB;
  m_powerCutMaxIdddClocked_OB = POWER_CUT_MAXIDDD_CLOCKED_OB;
  m_powerCutMinIdda_IB        = POWER_CUT_MINIDDD_IB;
  m_powerCutMinIddd_IB        = POWER_CUT_MINIDDA_IB;
  m_powerCutMinIddaClocked_IB = POWER_CUT_MINIDDA_CLOCKED_IB;
  m_powerCutMinIdddClocked_IB = POWER_CUT_MINIDDD_CLOCKED_IB;
  m_powerCutMaxIddaClocked_IB = POWER_CUT_MAXIDDA_CLOCKED_IB;
  m_powerCutMaxIdddClocked_IB = POWER_CUT_MAXIDDD_CLOCKED_IB;
  m_powerCutMaxBias3V_IB      = POWER_CUT_MAXBIAS_3V_IB;
  m_powerCutMaxBias3V_OB      = POWER_CUT_MAXBIAS_3V_OB;
  m_powerMaxFactor4V_IB       = POWER_MAXFACTOR_4V_IB;
  m_powerMaxFactor4V_OB       = POWER_MAXFACTOR_4V_OB;
  m_readoutMaxTimeout         = READOUT_MAXTIMEOUT;
  m_readoutMaxCorrupt         = READOUT_MAXCORRUPT;
  m_readoutMax8b10bGreen      = READOUT_MAX8b10b_GREEN;

  m_fifoCutMaxErrGreen  = FIFO_CUT_MAXERR_GREEN;
  m_fifoCutMaxErrOrange = FIFO_CUT_MAXERR_ORANGE;
  m_fifoCutMaxFaulty    = FIFO_CUT_MAXFAULTY;
  m_fifoCutMaxException = FIFO_CUT_MAXEXCEPTION;

  m_digitalMaxTimeoutGreen       = DIGITAL_MAXTIMEOUT_GREEN;
  m_digitalMaxTimeoutOrange      = DIGITAL_MAXTIMEOUT_ORANGE;
  m_digitalMaxCorruptGreen       = DIGITAL_MAXCORRUPT_GREEN;
  m_digitalMaxCorruptOrange      = DIGITAL_MAXCORRUPT_ORANGE;
  m_digitalMaxBadPerChipOB       = DIGITAL_MAXBAD_CHIP_OB;
  m_digitalMaxBadPerChipIB       = DIGITAL_MAXBAD_CHIP_IB;
  m_digitalMaxBadPerHicOB        = DIGITAL_MAXBAD_HIC_OB;
  m_digitalMaxBadPerHicIB        = DIGITAL_MAXBAD_HIC_IB;
  m_digitalMaxDeadPerChipGreen   = DIGITAL_MAXDEAD_CHIP_GREEN;
  m_digitalMaxDeadPerChipOrange  = DIGITAL_MAXDEAD_CHIP_ORANGE;
  m_digitalMaxDeadPerHicGreenOB  = DIGITAL_MAXDEAD_HIC_GREEN_OB;
  m_digitalMaxDeadPerHicGreenIB  = DIGITAL_MAXDEAD_HIC_GREEN_IB;
  m_digitalMaxDeadPerHicOrangeOB = DIGITAL_MAXDEAD_HIC_ORANGE_OB;
  m_digitalMaxDeadPerHicOrangeIB = DIGITAL_MAXDEAD_HIC_ORANGE_IB;
  m_digitalMaxBadChipGold        = DIGITAL_MAXBAD_CHIP_GOLD;
  m_digitalMaxBadChipSilver      = DIGITAL_MAXBAD_CHIP_SILVER;
  m_digitalMaxBadChipBronze      = DIGITAL_MAXBAD_CHIP_BRONZE;

  m_digitalMaxNoMaskHicIB           = DIGITAL_MAXNOMASK_HIC_IB;
  m_digitalMaxNoMaskHicOB           = DIGITAL_MAXNOMASK_HIC_OB;
  m_digitalMaxNoMaskStuckHicIB      = DIGITAL_MAXNOMASKSTUCK_HIC_IB;
  m_digitalMaxNoMaskStuckHicOB      = DIGITAL_MAXNOMASKSTUCK_HIC_OB;
  m_digitalMaxNoMaskStuckChipGold   = DIGITAL_MAXNOMASKSTUCK_CHIP_GOLD;
  m_digitalMaxNoMaskStuckChipSilver = DIGITAL_MAXNOMASKSTUCK_CHIP_SILVER;
  m_digitalMaxNoMaskStuckChipBronze = DIGITAL_MAXNOMASKSTUCK_CHIP_BRONZE;

  m_threshMaxTimeoutGreen       = THRESH_MAXTIMEOUT_GREEN;
  m_threshMaxTimeoutOrange      = THRESH_MAXTIMEOUT_ORANGE;
  m_threshMaxCorruptGreen       = THRESH_MAXCORRUPT_GREEN;
  m_threshMaxCorruptOrange      = THRESH_MAXCORRUPT_ORANGE;
  m_threshMaxDeadPerHicGreenOB  = THRESH_MAXDEAD_HIC_GREEN_OB;
  m_threshMaxDeadPerHicGreenIB  = THRESH_MAXDEAD_HIC_GREEN_IB;
  m_threshMaxDeadPerHicOrangeOB = THRESH_MAXDEAD_HIC_ORANGE_OB;
  m_threshMaxDeadPerHicOrangeIB = THRESH_MAXDEAD_HIC_ORANGE_IB;
  m_threshMaxBadPerChipOB       = THRESH_MAXBAD_CHIP_OB;
  m_threshMaxBadPerChipIB       = THRESH_MAXBAD_CHIP_IB;
  m_threshMaxBadPerHicOB        = THRESH_MAXBAD_HIC_OB;
  m_threshMaxBadPerHicIB        = THRESH_MAXBAD_HIC_IB;
  m_threshMaxDeadChipGold       = THRESH_MAXDEAD_CHIP_GOLD;
  m_threshMaxDeadChipSilver     = THRESH_MAXDEAD_CHIP_SILVER;
  m_threshMaxDeadChipBronze     = THRESH_MAXDEAD_CHIP_BRONZE;
  m_threshMaxNoThreshChipGold   = THRESH_MAXNOTHRESH_CHIP_GOLD;
  m_threshMaxNoThreshChipSilver = THRESH_MAXNOTHRESH_CHIP_SILVER;
  m_threshMaxNoThreshChipBronze = THRESH_MAXNOTHRESH_CHIP_BRONZE;
  m_threshMaxNoiseOB            = THRESH_MAXNOISE_OB;
  m_threshMaxNoiseIB            = THRESH_MAXNOISE_IB;

  m_maxNoisyChipGold   = MAXNOISY_CHIP_GOLD;
  m_maxNoisyChipSilver = MAXNOISY_CHIP_SILVER;
  m_maxNoisyChipBronze = MAXNOISY_CHIP_BRONZE;

  m_testDctrl            = TEST_DCTRL;
  m_dctrlMinAmpOB        = DCTRL_MINAMP_OB;
  m_dctrlMinSlopeOB      = DCTRL_MINSLOPE_OB;
  m_dctrlMinAmpOBStave   = DCTRL_MINAMP_OBSTAVE;
  m_dctrlMinSlopeOBStave = DCTRL_MINSLOPE_OBSTAVE;
  m_dctrlMaxRiseGreenOB  = DCTRL_MAXRISE_GREEN_OB;
  m_dctrlMaxFallGreenOB  = DCTRL_MAXFALL_GREEN_OB;
  m_dctrlMinAmpIB        = DCTRL_MINAMP_IB;
  m_dctrlMinSlopeIB      = DCTRL_MINSLOPE_IB;
  m_dctrlMaxRiseGreenIB  = DCTRL_MAXRISE_GREEN_IB;
  m_dctrlMaxFallGreenIB  = DCTRL_MAXFALL_GREEN_IB;
  m_dctrlMaxChisqSilver  = DCTRL_MAXCHISQ_SILVER;

  m_enduranceSlices            = ENDURANCE_SLICES;
  m_enduranceCycles            = ENDURANCE_CYCLES;
  m_enduranceTriggers          = ENDURANCE_TRIGGERS;
  m_enduranceUptime            = ENDURANCE_UPTIME;
  m_enduranceDowntime          = ENDURANCE_DOWNTIME;
  m_enduranceLimit             = ENDURANCE_LIMIT;
  m_enduranceMaxtripsGreen     = ENDURANCE_MAXTRIPS_GREEN;
  m_enduranceMaxtripsOrange    = ENDURANCE_MAXTRIPS_ORANGE;
  m_enduranceMinchipsGreen     = ENDURANCE_MINCHIPS_GREEN;
  m_enduranceMaxfailuresOrange = ENDURANCE_MAXFAILURES_ORANGE;

  m_eyeDriver = EYE_DRIVER;
  m_eyePreemp = EYE_PREEMP;
  m_eyeMaxX   = EYE_MAX_X;
  m_eyeMinX   = EYE_MIN_X;
  m_eyeStepX  = EYE_STEP_X;
  m_eyeMaxY   = EYE_MAX_Y;
  m_eyeMinY   = EYE_MIN_Y;
  m_eyeStepY  = EYE_STEP_Y;

  m_useDataPath   = false;
  m_halfstavecomp = HALFSTAVE_COMPONENT;
  InitParamMap();

  time_t     t   = time(0); // get time now
  struct tm *now = localtime(&t);

  sprintf(m_startTime, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1,
          now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
}

void TScanConfig::InitParamMap()
{
  fSettings["AUTOREPEAT"] = &m_autorepeat;
  fSettings["MAXREPEAT"]  = &m_maxrepeat;
  fSettings["RECOVERY"]   = &m_recovery;
  fSettings["RSYNC"]      = &m_rsync;
  fSettings["ALLOWOLD"]   = &m_allowOld;

  fSettings["NINJ"]         = &m_nInj;
  fSettings["NTRIG"]        = &m_nTrig;
  fSettings["MAXTIMEOUT"]   = &m_maxTimeout;
  fSettings["MAXHITS"]      = &m_maxHits;
  fSettings["CHARGESTART"]  = &m_chargeStart;
  fSettings["CHARGESTOP"]   = &m_chargeStop;
  fSettings["CHARGESTEP"]   = &m_chargeStep;
  fSettings["NMASKSTAGES"]  = &m_nMaskStages;
  fSettings["PIXPERREGION"] = &m_pixPerRegion;
  fSettings["NOISECUT_INV"] = &m_noiseCutInv;

  fSettings["STATUS"]                 = &m_status;
  fSettings["TESTWITHOUTCOMP"]        = &m_testWithoutComp;
  fSettings["VCASN_START"]            = &m_vcasnStart;
  fSettings["VCASN_STOP"]             = &m_vcasnStop;
  fSettings["VCASN_STEP"]             = &m_vcasnStep;
  fSettings["ITHR_START"]             = &m_ithrStart;
  fSettings["ITHR_STOP"]              = &m_ithrStop;
  fSettings["ITHR_STEP"]              = &m_ithrStep;
  fSettings["DACSTART"]               = &m_ithrStart;
  fSettings["DACSTOP"]                = &m_ithrStop;
  fSettings["DACSTEP"]                = &m_ithrStep;
  fSettings["NDACSAMPLES"]            = &m_nDacSamples;
  fSettings["SCAN_STEP"]              = &m_scanStep;
  fSettings["TUNINGMAXROW"]           = &m_tuningMaxrow;
  fSettings["SPEEDY"]                 = &m_speedy;
  fSettings["RAWDATA"]                = &m_rawData;
  fSettings["IVCURVE"]                = &m_ivCurve;
  fSettings["IVPOINTS"]               = &m_ivPoints;
  fSettings["MAXIBIAS"]               = &m_maxIbias;
  fSettings["MINIDDA_OB"]             = &m_powerCutMinIdda_OB;
  fSettings["MINIDDD_OB"]             = &m_powerCutMinIddd_OB;
  fSettings["MAXIDDA_OB"]             = &m_powerCutMaxIdda_OB;
  fSettings["MAXIDDD_OB"]             = &m_powerCutMaxIddd_OB;
  fSettings["MAXIDDDSILVEROB"]        = &m_powerCutMaxIdddSilver_OB;
  fSettings["MINIDDA_CLOCKED_OB"]     = &m_powerCutMinIddaClocked_OB;
  fSettings["MINIDDD_CLOCKED_OB"]     = &m_powerCutMinIdddClocked_OB;
  fSettings["MAXIDDA_CLOCKED_OB"]     = &m_powerCutMaxIddaClocked_OB;
  fSettings["MAXIDDD_CLOCKED_OB"]     = &m_powerCutMaxIdddClocked_OB;
  fSettings["MINIDDA_IB"]             = &m_powerCutMinIdda_IB;
  fSettings["MINIDDD_IB"]             = &m_powerCutMinIddd_IB;
  fSettings["MINIDDA_CLOCKED_IB"]     = &m_powerCutMinIddaClocked_IB;
  fSettings["MINIDDD_CLOCKED_IB"]     = &m_powerCutMinIdddClocked_IB;
  fSettings["MAXIDDA_CLOCKED_IB"]     = &m_powerCutMaxIddaClocked_IB;
  fSettings["MAXIDDD_CLOCKED_IB"]     = &m_powerCutMaxIdddClocked_IB;
  fSettings["MAXBIAS_3V_IB"]          = &m_powerCutMaxBias3V_IB;
  fSettings["MAXBIAS_3V_OB"]          = &m_powerCutMaxBias3V_OB;
  fSettings["MAXFACTOR_4V_IB"]        = &m_powerMaxFactor4V_IB;
  fSettings["MAXFACTOR_4V_OB"]        = &m_powerMaxFactor4V_OB;
  fSettings["READOUT_MAXTIMEOUT"]     = &m_readoutMaxTimeout;
  fSettings["READOUT_MAXCORRUPT"]     = &m_readoutMaxCorrupt;
  fSettings["READOUT_MAX8b10b_GREEN"] = &m_readoutMax8b10bGreen;

  fSettings["FIFO_MAXERR_GREEN"]   = &m_fifoCutMaxErrGreen;
  fSettings["FIFO_MAXERR_ORANGE"]  = &m_fifoCutMaxErrOrange;
  fSettings["FIFO_MAXFAULTYCHIPS"] = &m_fifoCutMaxFaulty;
  fSettings["FIFO_MAXEXCEPTION"]   = &m_fifoCutMaxException;

  fSettings["DIGITAL_MAXTIMEOUT_GREEN"]           = &m_digitalMaxTimeoutGreen;
  fSettings["DIGITAL_MAXTIMEOUT_ORANGE"]          = &m_digitalMaxTimeoutOrange;
  fSettings["DIGITAL_MAXCORRUPT_GREEN"]           = &m_digitalMaxCorruptGreen;
  fSettings["DIGITAL_MAXCORRUPT_ORANGE"]          = &m_digitalMaxCorruptOrange;
  fSettings["DIGITAL_MAXBAD_CHIP_OB"]             = &m_digitalMaxBadPerChipOB;
  fSettings["DIGITAL_MAXBAD_CHIP_IB"]             = &m_digitalMaxBadPerChipIB;
  fSettings["DIGITAL_MAXBAD_HIC_OB"]              = &m_digitalMaxBadPerHicOB;
  fSettings["DIGITAL_MAXBAD_HIC_IB"]              = &m_digitalMaxBadPerHicIB;
  fSettings["DIGITAL_MAXDEAD_CHIP_GREEN"]         = &m_digitalMaxDeadPerChipGreen;
  fSettings["DIGITAL_MAXDEAD_CHIP_ORANGE"]        = &m_digitalMaxDeadPerChipOrange;
  fSettings["DIGITAL_MAXDEAD_HIC_GREEN_OB"]       = &m_digitalMaxDeadPerHicGreenOB;
  fSettings["DIGITAL_MAXDEAD_HIC_GREEN_IB"]       = &m_digitalMaxDeadPerHicGreenIB;
  fSettings["DIGITAL_MAXDEAD_HIC_ORANGE_OB"]      = &m_digitalMaxDeadPerHicOrangeOB;
  fSettings["DIGITAL_MAXDEAD_HIC_ORANGE_IB"]      = &m_digitalMaxDeadPerHicOrangeIB;
  fSettings["DIGITAL_MAXBAD_CHIP_GOLD"]           = &m_digitalMaxBadChipGold;
  fSettings["DIGITAL_MAXBAD_CHIP_SILVER"]         = &m_digitalMaxBadChipSilver;
  fSettings["DIGITAL_MAXBAD_CHIP_BRONZE"]         = &m_digitalMaxBadChipBronze;
  fSettings["DIGITAL_MAXNOMASK_HIC_OB"]           = &m_digitalMaxNoMaskHicOB;
  fSettings["DIGITAL_MAXNOMASK_HIC_IB"]           = &m_digitalMaxNoMaskHicIB;
  fSettings["DIGITAL_MAXNOMASKSTUCK_HIC_OB"]      = &m_digitalMaxNoMaskStuckHicOB;
  fSettings["DIGITAL_MAXNOMASKSTUCK_HIC_IB"]      = &m_digitalMaxNoMaskStuckHicIB;
  fSettings["DIGITAL_MAXNOMASKSTUCK_CHIP_GOLD"]   = &m_digitalMaxNoMaskStuckChipGold;
  fSettings["DIGITAL_MAXNOMASKSTUCK_CHIP_SILVER"] = &m_digitalMaxNoMaskStuckChipSilver;
  fSettings["DIGITAL_MAXNOMASKSTUCK_CHIP_BRONZE"] = &m_digitalMaxNoMaskStuckChipBronze;

  fSettings["THRESH_MAXTIMEOUT_GREEN"]        = &m_threshMaxTimeoutGreen;
  fSettings["THRESH_MAXTIMEOUT_ORANGE"]       = &m_threshMaxTimeoutOrange;
  fSettings["THRESH_MAXCORRUPT_GREEN"]        = &m_threshMaxCorruptGreen;
  fSettings["THRESH_MAXCORRUPT_ORANGE"]       = &m_threshMaxCorruptOrange;
  fSettings["THRESH_MAXBAD_CHIP_OB"]          = &m_threshMaxBadPerChipOB;
  fSettings["THRESH_MAXBAD_CHIP_IB"]          = &m_threshMaxBadPerChipIB;
  fSettings["THRESH_MAXBAD_HIC_OB"]           = &m_threshMaxBadPerHicOB;
  fSettings["THRESH_MAXBAD_HIC_IB"]           = &m_threshMaxBadPerHicIB;
  fSettings["THRESH_MAXDEAD_HIC_GREEN_OB"]    = &m_threshMaxDeadPerHicGreenOB;
  fSettings["THRESH_MAXDEAD_HIC_GREEN_IB"]    = &m_threshMaxDeadPerHicGreenIB;
  fSettings["THRESH_MAXDEAD_HIC_ORANGE_OB"]   = &m_threshMaxDeadPerHicOrangeOB;
  fSettings["THRESH_MAXDEAD_HIC_ORANGE_IB"]   = &m_threshMaxDeadPerHicOrangeIB;
  fSettings["THRESH_MAXDEAD_CHIP_GOLD"]       = &m_threshMaxDeadChipGold;
  fSettings["THRESH_MAXDEAD_CHIP_SILVER"]     = &m_threshMaxDeadChipSilver;
  fSettings["THRESH_MAXDEAD_CHIP_BRONZE"]     = &m_threshMaxDeadChipBronze;
  fSettings["THRESH_MAXNOTHRESH_CHIP_GOLD"]   = &m_threshMaxNoThreshChipGold;
  fSettings["THRESH_MAXNOTHRESH_CHIP_SILVER"] = &m_threshMaxNoThreshChipSilver;
  fSettings["THRESH_MAXNOTHRESH_CHIP_BRONZE"] = &m_threshMaxNoThreshChipBronze;
  fSettings["THRESH_MAXNOISE_OB"]             = &m_threshMaxNoiseOB;
  fSettings["THRESH_MAXNOISE_IB"]             = &m_threshMaxNoiseIB;
  fSettings["MAXNOISY_CHIP_GOLD"]             = &m_maxNoisyChipGold;
  fSettings["MAXNOISY_CHIP_SILVER"]           = &m_maxNoisyChipSilver;
  fSettings["MAXNOISY_CHIP_BRONZE"]           = &m_maxNoisyChipBronze;

  fSettings["TESTDCTRL"]            = &m_testDctrl;
  fSettings["DCTRLMINAMPIB"]        = &m_dctrlMinAmpIB;
  fSettings["DCTRLMINSLOPEIB"]      = &m_dctrlMinSlopeIB;
  fSettings["DCTRLMAXRISEGREENIB"]  = &m_dctrlMaxRiseGreenIB;
  fSettings["DCTRLMAXFALLGREENIB"]  = &m_dctrlMaxFallGreenIB;
  fSettings["DCTRLMINAMPOB"]        = &m_dctrlMinAmpOB;
  fSettings["DCTRLMINSLOPEOB"]      = &m_dctrlMinSlopeOB;
  fSettings["DCTRLMINAMPOBSTAVE"]   = &m_dctrlMinAmpOBStave;
  fSettings["DCTRLMINSLOPEOBSTAVE"] = &m_dctrlMinSlopeOBStave;
  fSettings["DCTRLMAXRISEGREENOB"]  = &m_dctrlMaxRiseGreenOB;
  fSettings["DCTRLMAXFALLGREENOB"]  = &m_dctrlMaxFallGreenOB;
  fSettings["DCTRLMAXCHISQSILVER"]  = &m_dctrlMaxChisqSilver;
  fSettings["CAL_VPULSEL"]          = &m_calVpulsel;
  fSettings["TARGETTHRESH"]         = &m_targetThresh;
  fSettings["NOMINAL"]              = &m_nominal;
  fSettings["HALFSTAVECOMP"]        = &m_halfstavecomp;

  fSettings["ENDURANCESLICES"]    = &m_enduranceSlices;
  fSettings["ENDURANCECYCLES"]    = &m_enduranceCycles;
  fSettings["ENDURANCETRIGGERS"]  = &m_enduranceTriggers;
  fSettings["ENDURANCEUPTIME"]    = &m_enduranceUptime;
  fSettings["ENDURANCEDOWNTIME"]  = &m_enduranceDowntime;
  fSettings["ENDURANCETIMELIMIT"] = &m_enduranceLimit;


  fSettings["ENDURANCEMAXTRIPSGREEN"]     = &m_enduranceMaxtripsGreen;
  fSettings["ENDURANCEMAXTRIPSORANGE"]    = &m_enduranceMaxtripsOrange;
  fSettings["ENDURANCEMINCHIPSGREEN"]     = &m_enduranceMinchipsGreen;
  fSettings["ENDURANCEMAXFAILURESORANGE"] = &m_enduranceMaxfailuresOrange;

  fSettings["READOUTSPEED"]     = &m_readoutSpeed;
  fSettings["READOUTOCC"]       = &m_readoutOcc;
  fSettings["READOUTDRIVER"]    = &m_readoutDriver;
  fSettings["READOUTPREEMP"]    = &m_readoutPreemp;
  fSettings["READOUTROW"]       = &m_readoutRow;
  fSettings["READOUTPLLSTAGES"] = &m_readoutPllStages;

  fSettings["EYEDRIVER"] = &m_eyeDriver;
  fSettings["EYEPREEMP"] = &m_eyePreemp;
  fSettings["EYEMINX"]   = &m_eyeMinX;
  fSettings["EYEMAXX"]   = &m_eyeMaxX;
  fSettings["EYESTEPX"]  = &m_eyeStepX;
  fSettings["EYEMINY"]   = &m_eyeMinY;
  fSettings["EYEMAXY"]   = &m_eyeMaxY;
  fSettings["EYESTEPY"]  = &m_eyeStepY;
}

bool TScanConfig::SetParamValue(std::string Name, std::string Value)
{
  if (fSettings.find(Name) != fSettings.end()) {
    *(fSettings.find(Name)->second) = std::stoi(Value);
    return true;
  }

  return false;
}

bool TScanConfig::SetParamValue(std::string Name, int Value)
{
  if (fSettings.find(Name) != fSettings.end()) {
    *(fSettings.find(Name)->second) = Value;
    return true;
  }

  return false;
}

int TScanConfig::GetParamValue(std::string Name)
{

  if (fSettings.find(Name) != fSettings.end()) {
    return *(fSettings.find(Name)->second);
  }
  return -1;
}

void TScanConfig::SetRetestNumber(std::string hicName, int aRetest)
{
  if (m_retest.find(hicName) != m_retest.end()) {
    std::cout << "Warning (TScanConfig::SetRetestNumber): hic " << hicName
              << " occurred more than once, ignored!" << std::endl;
    return;
  }
  m_retest.insert(std::pair<std::string, int>(hicName, aRetest));
}

int TScanConfig::GetRetestNumber(std::string hicName)
{
  if (m_retest.find(hicName) != m_retest.end()) {
    return m_retest.find(hicName)->second;
  }
  std::cout << "Warning (TScanConfig::GetRetestNumber): hic " << hicName
            << " not found, returning 0" << std::endl;
  return 0;
}

std::string TScanConfig::GetDataPath(std::string HicName)
{
  std::string result = GetTestDir() + HicName;
  if (const char *dataDir = std::getenv("ALPIDE_TEST_DATA"))
    result.insert(0, std::string(dataDir) + "/");
  else
    result.insert(0, std::string("Data/"));
  if (GetRetestNumber(HicName) > 0) {
    result.append("_Retest_");
    result.append(std::to_string(GetRetestNumber(HicName)));
  }
  return result;
}

std::string TScanConfig::GetRemoteHicPath(std::string HicName)
{
  std::string result = HicName;
  if (GetRetestNumber(HicName) > 0) {
    result.append("_Retest_");
    result.append(std::to_string(GetRetestNumber(HicName)));
  }
  return result;
}


std::string TScanConfig::GetTestDir() { return ""; }


bool TScanConfig::IsHalfStave()
{
  return ((m_testType == OBHalfStaveOL) || (m_testType == OBHalfStaveML) ||
          (m_testType == OBHalfStaveOLFAST) || (m_testType == OBHalfStaveMLFAST) ||
          (m_testType == OBStaveOL) || (m_testType == OBStaveML) || (m_testType == OBStaveOLFAST) ||
          (m_testType == OBStaveMLFAST) || (m_testType == StaveReceptionOL) ||
          (m_testType == StaveReceptionML));
}
