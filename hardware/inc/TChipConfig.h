#ifndef CHIPCONFIG_H
#define CHIPCONFIG_H

#include "AlpideDecoder.h"
#include "TConfig.h"
#include <map>
#include <string.h>
#include <string>

class TConfig;
class TAlpide;

namespace ChipConfig { // to avoid clashes with other configs (e.g. for STROBE_DELAY)
    // DAC registers
    const int IBIAS   = 64;
    const int ITHR    = 51;
    const int IDB     = 64;
    const int IRESET  = 50;
    const int IAUX2   = 0;
    
    const int VCASP   = 86;
    const int VCASN   = 57;
    const int VCASN2  = 62;
    const int VCLIP   = 0;
    const int VRESETP = 117;
    const int VRESETD = 147;
    const int VPULSEH = 170;
    const int VPULSEL = 170;
    const int VTEMP   = 0;
    
    // Mode control registers
    const int READOUT_MODE           = 0; 
    const int ENABLE_CLUSTERING      = 1;
    const int MATRIX_READOUT_SPEED   = 1;
    const int SERIAL_LINK_SPEED      = 3;
    const int ENABLE_SKEWING_GLOBAL  = 1;
    const int ENABLE_SKEWING_STARTRO = 1;
    const int ENABLE_CLOCK_GATING    = 0;
    const int ENABLE_CMU_READOUT     = 0;
    
    // FROMU configuration register 1
    const int MEB_MASK              = 0;
    const int INT_STROBE_GEN        = 0;
    const int BUSY_MON              = 1;
    const int TEST_PULSE_MODE       = 0;
    const int EN_TEST_STROBE        = 0;
    const int EN_ROTATE_PULSE_LINES = 0;
    const int TRIGGER_DELAY         = 0;
    
    // FROMU configuration register 2
    const int STROBE_DURATION = 80;   // 2 us
    
    // FROMU configuration register 3
    const int STROBE_GAP      = 4000; // .1 ms
    
    // FROMU pulsing register 1
    const int STROBE_DELAY    = 20;   // 500 ns
    
    // FROMU pulsing register 2
    const int PULSE_DURATION  = 500; // 12.5 us
    
    // CMU/DMU configuration register
    const int PREVIOUS_ID        = -1; // -1 & 0x0F = 0xF;
    const int INITIAL_TOKEN      = 1;
    const int DISABLE_MANCHESTER = 0;
    const int ENABLE_DDR         = 1;
    
    const int DCLK_RECEIVER  = 10;
    const int DCLK_DRIVER    = 10;
    const int MCLK_RECEIVER  = 10;
    const int DCTRL_RECEIVER = 10;
    const int DCTRL_DRIVER   = 10;
    
    const int PLL_PHASE   = 8;
    const int PLL_STAGES  = 0;
    const int CHARGE_PUMP = 8;
    const int DTU_DRIVER  = 8;
    const int DTU_PREEMP  = 0;

    // Scans
    const int SCAN_THR_ITHR = 0;
    const int SCAN_THR_DV   = 0;
    const int SCAN_FHR      = 0;

} // namespace ChipConfig

class TChipConfig {
    public:
        TChipConfig(TConfig *config, int chipId, const char *fName = 0);
        int  fEnduranceDisabled; // temporary fix to re-enabled chips that were disabled in end. test
        void InitParamMap();
    
        void SetChip(TAlpide *chip){ fChip = chip; };
    
        bool SetParamValue(std::string Name, std::string Value);
        bool SetParamValue(std::string Name, int Value);
        int  GetParamValue(std::string Name);
        bool IsParameter(std::string Name){ return (fSettings.count(Name) > 0); };
    
        int  GetChipId()  { return fChipId; };
        int  GetCtrInt()  { return fControlInterface; };
        int  GetDataLink(){ return fReceiver; };
    
        bool IsEnabled() const;
        bool IsEnabledNoBB() const { return fEnabled != 0; }
        bool IsEnabledWithBB() const;
    
        void SetEnable(bool Enabled)      { fEnabled = Enabled ? 1 : 0; };
        void SetEnableWithBB(bool Enabled){ fEnabledWithBB = Enabled ? 1 : 0; };
    
        int  GetModuleId(){ return (fChipId & 0x70) >> 4; };
        bool IsOBMaster() { return ((fChipId % 8 == 0) && (GetModuleId() > 0)); };
        bool HasEnabledSlave();
    
        bool GetReadoutMode()         { return (bool)fReadoutMode; };
        bool GetEnableClustering()    { return (bool)fEnableClustering; };
        int  GetMatrixReadoutSpeed()  { return fMatrixReadoutSpeed; };
        int  GetSerialLinkSpeed()     { return fSerialLinkSpeed; };
        bool GetEnableSkewingGlobal() { return (bool)fEnableSkewingGlobal; };
        bool GetEnableSkewingStartRO(){ return (bool)fEnableSkewingStartRO; };
        bool GetEnableClockGating()   { return (bool)fEnableClockGating; };
        bool GetEnableCMUReadout()    { return (bool)fEnableCMUReadout; };
    
        int GetTriggerDelay()  { return fTriggerDelay; };
        int GetStrobeDuration(){ return fStrobeDuration; };
        int GetStrobeDelay()   { return fStrobeDelay; };
        int GetPulseDuration() { return fPulseDuration; };
    
        int GetDclkReceiver() { return fDclkReceiver; };
        int GetDclkDriver()   { return fDclkDriver; };
        int GetMclkReceiver() { return fMclkReceiver; };
        int GetDctrlReceiver(){ return fDctrlReceiver; };
        int GetDctrlDriver()  { return fDctrlDriver; };
    
        int  GetPreviousId()       { return fPreviousId; };
        bool GetInitialToken()     { return (bool)fInitialToken; };
        bool GetDisableManchester(){ return (bool)fDisableManchester; };
        bool GetEnableDdr()        { return (bool)fEnableDdr; };
    
        void SetPreviousId(int APreviousId)     { fPreviousId = APreviousId; };
        void SetInitialToken(bool AInitialToken){ fInitialToken = (AInitialToken) ? 1 : 0; };
        void SetEnableDdr(bool AEnableDdr)      { fEnableDdr = (AEnableDdr) ? 1 : 0; };
        void SetDisableManchester(bool ADisableManchester){
        fDisableManchester = (ADisableManchester) ? 1 : 0;
        };
    
        void                 SetMaskFile(const char *fName)            { strcpy(fMaskFile, fName); };
        void                 SetNoisyPixels(std::vector<TPixHit> noisy){ m_noisyPixels = noisy; };
        void                 ClearNoisyPixels()                        { m_noisyPixels.clear(); };
        std::vector<TPixHit> GetNoisyPixels()                          { return m_noisyPixels; };
    
        void         SetDoubleColumnMask(unsigned int dcol, bool mask = true);
        unsigned int GetDoubleColumnMask(unsigned int region);
    
        bool ScanThresholdIthr() { return (bool)fScanThrIthr; };
        bool ScanThresholdDv() { return (bool)fScanThrDv; };
        bool ScanFakeHitRate() { return (bool)fScanFhr; };

    private:
        std::map<std::string, int *> fSettings;
        TConfig *                    fConfig;
        TAlpide *                    fChip;
        
        // genera;
        int fChipId;
        int fEnabled;       // variable to exclude (non-working) chip from tests, default true
        int fEnabledWithBB; // variable to exclude chips from tests when BB on, default true
        int fReceiver;
        int fControlInterface;
        
        // DACs registers
        int fITHR;
        int fIDB;
        int fVCASN;
        int fVCASN2;
        int fVCLIP;
        int fVRESETD;
        int fVCASP;
        int fVPULSEL;
        int fVPULSEH;
        int fIBIAS;
        int fVRESETP;
        int fVTEMP;
        int fIAUX2;
        int fIRESET;
    
        // Control register settings
        int fReadoutMode; // 0 = triggered, 1 = continuous (influences busy handling)
        int fEnableClustering;
        int fMatrixReadoutSpeed;
        int fSerialLinkSpeed;
        int fEnableSkewingGlobal;
        int fEnableSkewingStartRO;
        int fEnableClockGating;
        int fEnableCMUReadout;
    
        // FRROMU configuration register 1
        int fMEBMask;
        int fInternalStrobeGen;
        int fBusyMonitor;
        int fTestPulseMode;
        int fEnableTestStrobe;
        int fEnableRotatePulseLines;
        int fTriggerDelay; 
    
        // Fromu settings
        int fStrobeDuration;
        int fStrobeDelay;  // delay from pulse to strobe if generated internally
        int fStrobeGap;    // gap between subsequent strobes in sequencer mode
        int fPulseDuration;
        // Buffer current settings
        int fDclkReceiver;
        int fDclkDriver;
        int fMclkReceiver;
        int fDctrlReceiver;
        int fDctrlDriver;
        // CMU / DMU settings
        int fPreviousId;
        int fInitialToken;
        int fDisableManchester;
        int fEnableDdr;
        // DTU settings
        int fPllPhase;
        int fPllStages;
        int fChargePump;
        int fDtuDriver;
        int fDtuPreemp;
        // Mask file
        char                 fMaskFile[200];
        std::vector<TPixHit> m_noisyPixels;
        unsigned int         fDoubleColumnMask[32];
    
        // Scans
        int fScanThrIthr;
        int fScanThrDv;
        int fScanFhr;
};

#endif