#include "TChipConfig.h"
#include "TAlpide.h"
#include "THIC.h"
#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace ChipConfig;

TChipConfig::TChipConfig(TConfig *config, int chipId, const char *fName) {
    fConfig            = config;
    fChipId            = chipId;
    fEnabled           = true;
    fEnabledWithBB     = true;
    fEnduranceDisabled = false;
    fReceiver          = -1;
    fControlInterface  = -1;
    
    ClearNoisyPixels();
    // fill default values from header file
    fVCASN   = VCASN;
    fVCASN2  = VCASN2;
    fVCLIP   = VCLIP;
    fVRESETD = VRESETD;
    fITHR    = ITHR;
    fIBIAS   = IBIAS;
    fVRESETP = VRESETP;
    fVTEMP   = VTEMP;
    fIAUX2   = IAUX2;
    fIRESET  = IRESET;
    
    fReadoutMode         = READOUT_MODE;
    fEnableClustering    = ENABLE_CLUSTERING;
    fMatrixReadoutSpeed  = MATRIX_READOUT_SPEED;
    fSerialLinkSpeed     = SERIAL_LINK_SPEED;
    fEnableSkewingGlobal = ENABLE_SKEWING_GLOBAL;
    fEnableClockGating   = ENABLE_CLOCK_GATING;
    fEnableCMUReadout    = ENABLE_CMU_READOUT;
    
    fStrobeDuration = STROBE_DURATION;
    fStrobeGap      = STROBE_GAP;
    fStrobeDelay    = STROBE_DELAY;
    fTriggerDelay   = TRIGGER_DELAY;
    fPulseDuration  = PULSE_DURATION;
    
    fDclkReceiver  = DCLK_RECEIVER;
    fDclkDriver    = DCLK_DRIVER;
    fMclkReceiver  = MCLK_RECEIVER;
    fDctrlReceiver = DCTRL_RECEIVER;
    fDctrlDriver   = DCTRL_DRIVER;
    
    fPreviousId        = PREVIOUS_ID;
    fInitialToken      = INITIAL_TOKEN;
    fDisableManchester = DISABLE_MANCHESTER;
    fEnableDdr         = ENABLE_DDR;
    
    fPllPhase   = PLL_PHASE;
    fPllStages  = PLL_STAGES;
    fChargePump = CHARGE_PUMP;
    fDtuDriver  = DTU_DRIVER;
    fDtuPreemp  = DTU_PREEMP;

    fScanThrIthr = SCAN_THR_ITHR;
    fScanThrDv   = SCAN_THR_DV;
    fScanFhr     = SCAN_FHR;

    if (fName) {
        // read information from file
    }

    for (unsigned int ireg = 0; ireg < 32; ++ireg) {
        fDoubleColumnMask[ireg] = 0x0;
    }
    
    InitParamMap();
}

void TChipConfig::InitParamMap() {
    // general
    fSettings["CHIPID"]           = &fChipId;
    fSettings["RECEIVER"]         = &fReceiver;
    fSettings["CONTROLINTERFACE"] = &fControlInterface;
    fSettings["ENABLED"]          = &fEnabled;
    fSettings["ENABLEDBB"]        = &fEnabledWithBB;
    
    // DAC registers  
    fSettings["ITHR"]    = &fITHR;
    fSettings["IDB"]     = &fIDB;
    fSettings["VCASN"]   = &fVCASN;
    fSettings["VCASN2"]  = &fVCASN2;
    fSettings["VCLIP"]   = &fVCLIP;
    fSettings["VRESETD"] = &fVRESETD;
    fSettings["IBIAS"]   = &fIBIAS;
    fSettings["VCASP"]   = &fVCASP;
    fSettings["VPULSEL"] = &fVPULSEL;
    fSettings["VPULSEH"] = &fVPULSEH;
    fSettings["VRESETP"] = &fVRESETP;
    fSettings["VTEMP"]   = &fVTEMP;
    fSettings["IAUX2"]   = &fIAUX2;
    fSettings["IRESET"]  = &fIRESET;
    
    // FROMU configuration register 1
    fSettings["MEBMASK"]            = &fMEBMask;
    fSettings["INTSTROBEGEN"]       = &fInternalStrobeGen;
    fSettings["BUSYMON"]            = &fBusyMonitor;
    fSettings["TESTPULSEMODE"]      = &fTestPulseMode;
    fSettings["ENTESTSTROBE"]       = &fEnableTestStrobe;
    fSettings["ENROTATEPULSELINES"] = &fEnableRotatePulseLines;
    fSettings["TRIGGERDELAY"]       = &fTriggerDelay;
    
    // FROMU configuration register 2
    fSettings["STROBEDURATION"] = &fStrobeDuration;
    
    // FROMU configuration register 3
    fSettings["STROBEGAP"] = &fStrobeGap;
    
    // FROMU pulsing register 1
    fSettings["STROBEDELAYCHIP"] = &fStrobeDelay;
    
    // FROMU pulsing register 2
    fSettings["PULSEDURATION"] = &fPulseDuration;
    
    // mode control
    fSettings["READOUTMODE"]        = &fReadoutMode;
    fSettings["ENABLECLUSTERING"]   = &fEnableClustering;
    fSettings["MATRIXREADOUTSPEED"] = &fMatrixReadoutSpeed;
    fSettings["SERIALLINKSPEED"]    = &fSerialLinkSpeed;
    fSettings["ENABLESKEWGLOBAL"]   = &fEnableSkewingGlobal;
    fSettings["ENABLESKEWSTARTRO"]  = &fEnableSkewingStartRO;
    fSettings["ENABLECLKGATE"]      = &fEnableClockGating;
    fSettings["ENABLECMURO"]        = &fEnableCMUReadout;
    
    // CMU/DMU
    fSettings["PREVID"]             = &fPreviousId;
    fSettings["INITIALTOKEN"]       = &fInitialToken;
    fSettings["MANCHESTERDISABLED"] = &fDisableManchester;
    fSettings["ENABLEDDR"]          = &fEnableDdr;
    
    // misc
    fSettings["PLLPHASE"]    = &fPllPhase;
    fSettings["PLLSTAGES"]   = &fPllStages;
    fSettings["CHARGEPUMP"]  = &fChargePump;
    fSettings["DTUDRIVER"]   = &fDtuDriver;
    fSettings["DTUPREEMP"]   = &fDtuPreemp;
    fSettings["DCTRLDRIVER"] = &fDctrlDriver;
    
    // Scans
    fSettings["SCAN_THR_ITHR"] = &fScanThrIthr;
    fSettings["SCAN_THR_DV"]   = &fScanThrDv;
    fSettings["SCAN_FHR"]      = &fScanFhr;
}

bool TChipConfig::SetParamValue(std::string Name, std::string Value) {
    if (fSettings.find(Name) != fSettings.end()) {
        *(fSettings.find(Name)->second) = std::stoi(Value);
        return true;
    }

    return false;
}

bool TChipConfig::SetParamValue(std::string Name, int Value) {
    if (fSettings.find(Name) != fSettings.end()) {
        *(fSettings.find(Name)->second) = Value;
        return true;
    }

    return false;
}

int TChipConfig::GetParamValue(std::string Name) {
    if (fSettings.find(Name) != fSettings.end()) {
        return *(fSettings.find(Name)->second);
    }
    return -1;
}

bool TChipConfig::IsEnabled() const {
    return (bool)fEnabled;
}


bool TChipConfig::IsEnabledWithBB() const {
    THic *hic = fChip->GetHic();
    if (hic && !hic->BiasChannelEnabled()) {
        return false;
    }
    return (fEnabled != 0) && (fEnabledWithBB != 0);
}


bool TChipConfig::HasEnabledSlave() {
    if (!IsOBMaster()) { 
        return false;
    }
    for (unsigned int i = (unsigned int)fChipId + 1; i <= (unsigned int)fChipId + 6; i++) {
        if (fConfig->GetChipConfigById(i)->IsEnabled()) {
            return true;
        }
    }
    return false;
}

void TChipConfig::SetDoubleColumnMask(unsigned int dcol, bool mask /* = true*/) {
    unsigned int reg = (dcol >> 4) & 0x1f;
    unsigned int bit = (dcol & 0xf);
    fDoubleColumnMask[reg] &= ~(unsigned int)(0x1 << bit);
    if (mask) { 
        fDoubleColumnMask[reg] |= (0x1 << bit);
    }
}

unsigned int TChipConfig::GetDoubleColumnMask(unsigned int region) {
    return (region < 32) ? fDoubleColumnMask[region] : 0x0;
}
