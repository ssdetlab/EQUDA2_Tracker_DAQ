#ifndef ALPIDE_H
#define ALPIDE_H

#include "TConfig.h"
#include <string>
#include <unistd.h>

namespace Alpide {
  typedef enum {
    REG_COMMAND         = 0x0,
    REG_MODECONTROL     = 0x1,
    REG_REGDISABLE_LOW  = 0x2,
    REG_REGDISABLE_HIGH = 0x3,
    REG_FROMU_CONFIG1   = 0x4,
    REG_FROMU_CONFIG2   = 0x5,
    REG_FROMU_CONFIG3   = 0x6,
    REG_FROMU_PULSING1  = 0x7,
    REG_FROMU_PULSING2  = 0x8,
    REG_FROMU_STATUS1   = 0x9,
    REG_FROMU_STATUS2   = 0xa,
    REG_FROMU_STATUS3   = 0xb,
    REG_FROMU_STATUS4   = 0xc,
    REG_FROMU_STATUS5   = 0xd,
    REG_CLKIO_DACS      = 0xe,
    REG_CMUIO_DACS      = 0xf,
    REG_CMUDMU_CONFIG   = 0x10,
    REG_CMUDMU_STATUS   = 0x11,
    REG_CMUFIFO_LOW     = 0x12,
    REG_CMUFIFO_HIGH    = 0x13,
    REG_DTU_CONFIG      = 0x14,
    REG_DTU_DACS        = 0x15,
    REG_PLL_LOCK1       = 0x16,
    REG_PLL_LOCK2       = 0x17,
    REG_DTU_TEST1       = 0x18,
    REG_DTU_TEST2       = 0x19,
    REG_DTU_TEST3       = 0x1a,
    REG_BUSY_MINWIDTH   = 0x1b,
    REG_PIXELCONFIG     = 0x500,
    REG_ANALOGMON       = 0x600,
    
    // DAC registers
    REG_VRESETP         = 0x601,
    REG_VRESETD         = 0x602,
    REG_VCASP           = 0x603,
    REG_VCASN           = 0x604,
    REG_VPULSEH         = 0x605,
    REG_VPULSEL         = 0x606,
    REG_VCASN2          = 0x607,
    REG_VCLIP           = 0x608,
    REG_VTEMP           = 0x609,
    REG_IAUX2           = 0x60a,
    REG_IRESET          = 0x60b,
    REG_IDB             = 0x60c,
    REG_IBIAS           = 0x60d,
    REG_ITHR            = 0x60e,
    
    REG_BYPASS_BUFFER   = 0x60f,
    REG_ADC_CONTROL     = 0x610,
    REG_ADC_DAC_INPUT   = 0x611,
    REG_ADC_CALIB       = 0x612,
    REG_ADC_AVSS        = 0x613,
    REG_ADC_DVSS        = 0x614,
    REG_ADC_AVDD        = 0x615,
    REG_ADC_DVDD        = 0x616,
    REG_ADC_VCASN       = 0x617,
    REG_ADC_VCASP       = 0x618,
    REG_ADC_VPULSEH     = 0x619,
    REG_ADC_VPULSEL     = 0x61a,
    REG_ADC_VRESETP     = 0x61b,
    REG_ADC_VRESETD     = 0x61c,
    REG_ADC_VCASN2      = 0x61d,
    REG_ADC_VCLIP       = 0x61e,
    REG_ADC_VTEMP       = 0x61f,
    REG_ADC_ITHR        = 0x620,
    REG_ADC_IREF        = 0x621,
    REG_ADC_IDB         = 0x622,
    REG_ADC_IBIAS       = 0x623,
    REG_ADC_IAUX2       = 0x624,
    REG_ADC_IRESET      = 0x625,
    REG_ADC_BG2V        = 0x626,
    REG_ADC_T2V         = 0x627,
    REG_SEU_ERROR_COUNT = 0x700,
    REG_TEST_CONTROL    = 0x701,
    REG_BMU_DEBUG       = 0x702,
    REG_DMU_DEBUG       = 0x703,
    REG_TRU_DEBUG       = 0x704,
    REG_RRU_DEBUG       = 0x705,
    REG_FROMU_DEBUG     = 0x706,
    REG_ADC_DEBUG       = 0x707,
    // region register base addresses (addres for region 0),
    // to be ORed with 0x0800 - 0xf800 for regions 1 - 31 or
    // with 0x80 for region broadcast
    REG_RRU_MEB_LSB_BASE   = 0x100, // to be ORed with 0x00 - 0x7f for the different RAM words
    REG_RRU_MEB_MSB_BASE   = 0x200, // same here
    REG_DCOL_DISABLE_BASE  = 0x300,
    REG_REGION_STATUS_BASE = 0x301,
    REG_COLSEL1_BASE       = 0x401,
    REG_COLSEL2_BASE       = 0x402,
    REG_ROWSEL_BASE        = 0x404,
    REG_PULSESEL_BASE      = 0x408
  } TRegister;

  typedef enum {
    OPCODE_TRIGGER1 = 0xb1,
    OPCODE_TRIGGER2 = 0x55,
    OPCODE_TRIGGER3 = 0xc9,
    OPCODE_TRIGGER4 = 0x2d,
    OPCODE_GRST     = 0xd2,
    OPCODE_PRST     = 0xe4,
    OPCODE_PULSE    = 0x78,
    OPCODE_BCRST    = 0x36,
    OPCODE_DEBUG    = 0xaa,
    OPCODE_RORST    = 0x63,
    OPCODE_WROP     = 0x9c,
    OPCODE_RDOP     = 0x4e,
  } TOpCode;

  typedef enum {
    COMMAND_CMU_CLEARERR = 0xff00,
    COMMAND_FIFOTEST     = 0xff01,
    COMMAND_LOADOBDEFCFG = 0xff02,
    COMMAND_XOFF         = 0xff10,
    COMMAND_XON          = 0xff11,
    COMMAND_ADCMEASURE   = 0xff20
  } TCommand;

  typedef enum { PIXREG_MASK = 0x0, PIXREG_SELECT = 0x1 } TPixReg;

  typedef enum { PT_DIGITAL = 0, PT_ANALOGUE = 1 } TPulseType;

  typedef enum { MODE_CONFIG = 0, MODE_TRIGGERED = 1, MODE_CONTINUOUS = 2 } TChipMode;

  typedef enum { IREF_025uA = 0, IREF_075uA = 1, IREF_100uA = 2, IREF_125uA = 3 } TDACMonIref;

  typedef enum {
    MODE_MANUAL      = 0,
    MODE_CALIBRATE   = 1,
    MODO_AUTO        = 2,
    MODE_SUPERMANUAL = 3
  } TADCMode;

  typedef enum {
    INP_AVSS         = 0,
    INP_DVSS         = 1,
    INP_AVDD         = 2,
    INP_DVDD         = 3,
    INP_VBGthVolScal = 4,
    INP_DACMONV      = 5,
    INP_DACMONI      = 6,
    INP_Bandgap      = 7,
    INP_Temperature  = 8
  } TADCInput;

  typedef enum { COMP_180uA = 0, COMP_190uA = 1, COMP_296uA = 2, COMP_410uA = 3 } TADCComparator;

  typedef enum { RAMP_500ms = 0, RAMP_1us = 1, RAMP_2us = 2, RAMP_4us = 3 } TADCRampSpeed;
} // namespace Alpide

class TReadoutBoard;
class THic;

class TAlpide{
  public:
    TAlpide(TChipConfig *config);
    
    TChipConfig*  GetConfig() { return fConfig; };
    
    void           SetReadoutBoard(TReadoutBoard *readoutBoard){ fReadoutBoard = readoutBoard; };
    TReadoutBoard* GetReadoutBoard()                           { return fReadoutBoard; };
    
    THic* GetHic()         { return fHic; };
    void  SetHic(THic *hic){ fHic = hic; };

    int ReadRegister(Alpide::TRegister address, uint16_t &value);
    int WriteRegister(Alpide::TRegister address, uint16_t value, bool verify = false);
    int ReadRegister(uint16_t address, uint16_t &value);
    int WriteRegister(uint16_t address, uint16_t value, bool verify = false);
    int ModifyRegisterBits(Alpide::TRegister address, uint8_t lowBit, uint8_t nBits, uint16_t value,
                           bool verify = false);
    int ModifyRegisterBits(uint16_t address, uint8_t lowBit, uint8_t nBits, uint16_t value,
                           bool verify = false);

    void SetEnable(bool Enable);
    void SetEnableWithBB(bool Enable);

    void DumpConfig(const char *fName, bool writeFile = true, char *Config = 0);

    std::string DumpRegisters();

    int      GetADCOffset(){ return fADCOffset; };
    int      CalibrateADC();
    void     SetTheDacMonitor(Alpide::TRegister ADac, Alpide::TDACMonIref IRef = Alpide::IREF_100uA);
    uint16_t SetTheADCCtrlRegister(Alpide::TADCMode Mode, Alpide::TADCInput SelectInput,
                                   Alpide::TADCComparator ComparatorCurrent,
                                   Alpide::TADCRampSpeed  RampSpeed);
    float    ReadTemperature();
    float    ReadAnalogueVoltage();
    float    ReadDigitalVoltage();
    float    ReadDACVoltage(Alpide::TRegister ADac);
    float    ReadDACCurrent(Alpide::TRegister ADac);

  private:
    TChipConfig*   fConfig;
    int            fChipId;
    TReadoutBoard* fReadoutBoard;
    THic*          fHic;

    // ADC calibration parameters
    int  fADCOffset;
    bool fADCHalfLSB;
    bool fADCSign;
};

#endif