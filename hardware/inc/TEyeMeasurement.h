#ifndef TEYEMEASUREMENT_H
#define TEYEMEASUREMENT_H

#include <mutex>

#include "Common.h"
#include "THisto.h"
#include "TScan.h"

class TReadoutBoardMosaic;

typedef struct __TEyeParameters : TScanParameters {
  int driverStrength;
  int preemphasis;
} TEyeParameters;

class TEyeMeasurement : public TScan {
private:
  TAlpide *m_testChip;
  int      m_boardIndex;

  // parameters
  int m_max_prescale;
  int m_min_prescale;
  int m_max_zero_results;

  // internal
  int                  m_current_prescale;
  TReadoutBoardMOSAIC *m_board;
  void                 SetName();

protected:
  static constexpr int MAX_HORZ_OFFSET = 128;
  static constexpr int MIN_HORZ_OFFSET = -128;
  static constexpr int MAX_VERT_OFFSET = 127;
  static constexpr int MIN_VERT_OFFSET = -127;
  static constexpr int BUS_WIDTH       = 20;

  static constexpr int ES_QUALIFIER_0               = 0x2c;
  static constexpr int ES_QUALIFIER_1               = 0x2d;
  static constexpr int ES_QUALIFIER_2               = 0x2e;
  static constexpr int ES_QUALIFIER_3               = 0x2f;
  static constexpr int ES_QUALIFIER_4               = 0x30;
  static constexpr int ES_QUAL_MASK_0               = 0x31;
  static constexpr int ES_QUAL_MASK_1               = 0x32;
  static constexpr int ES_QUAL_MASK_2               = 0x33;
  static constexpr int ES_QUAL_MASK_3               = 0x34;
  static constexpr int ES_QUAL_MASK_4               = 0x35;
  static constexpr int ES_SDATA_MASK_0              = 0x36;
  static constexpr int ES_SDATA_MASK_1              = 0x37;
  static constexpr int ES_SDATA_MASK_2              = 0x38;
  static constexpr int ES_SDATA_MASK_3              = 0x39;
  static constexpr int ES_SDATA_MASK_4              = 0x3a;
  static constexpr int ES_PRESCALE                  = 0x3b; // bits [15:11]
  static constexpr int ES_PRESCALE_SIZE             = 5;
  static constexpr int ES_PRESCALE_OFFSET           = 11;
  static constexpr int ES_VERT_OFFSET               = 0x3b; // bits [7:0]
  static constexpr int ES_VERT_OFFSET_SIZE          = 8;
  static constexpr int ES_VERT_OFFSET_OFFSET        = 0;
  static constexpr int ES_HORZ_OFFSET               = 0x3c; // bits [11:0]
  static constexpr int ES_ERRDET_EN                 = 0x3d; // bit 9
  static constexpr int ES_ERRDET_EN_SIZE            = 1;
  static constexpr int ES_ERRDET_EN_OFFSET          = 9;
  static constexpr int ES_EYE_SCAN_EN               = 0x3d; // bit 8
  static constexpr int ES_EYE_SCAN_EN_SIZE          = 1;
  static constexpr int ES_EYE_SCAN_EN_OFFSET        = 8;
  static constexpr int ES_CONTROL                   = 0x3d; // bits [5:0]
  static constexpr int ES_CONTROL_SIZE              = 6;
  static constexpr int ES_CONTROL_OFFSET            = 0;
  static constexpr int USE_PCS_CLK_PHASE_SEL        = 0x91;
  static constexpr int USE_PCS_CLK_PHASE_SEL_SIZE   = 1;
  static constexpr int USE_PCS_CLK_PHASE_SEL_OFFSET = 14;

  // Read only registers
  static constexpr int ES_ERROR_COUNT         = 0x151;
  static constexpr int ES_SAMPLE_COUNT        = 0x152;
  static constexpr int ES_CONTROL_STATUS      = 0x153;
  static constexpr int ES_CONTROL_STATUS_DONE = 0x0001;
  static constexpr int ES_CONTROL_STATUS_FSM  = 0x000e;
  static constexpr int ES_RDATA_4             = 0x154;
  static constexpr int ES_RDATA_3             = 0x155;
  static constexpr int ES_RDATA_2             = 0x156;
  static constexpr int ES_RDATA_1             = 0x157;
  static constexpr int ES_RDATA_0             = 0x158;
  static constexpr int ES_SDATA_4             = 0x159;
  static constexpr int ES_SDATA_3             = 0x15a;
  static constexpr int ES_SDATA_2             = 0x15b;
  static constexpr int ES_SDATA_1             = 0x15c;
  static constexpr int ES_SDATA_0             = 0x15d;

  THisto CreateHisto();
  void   CreateScanParameters() { m_parameters = new TEyeParameters; };

public:
  TEyeMeasurement(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                  std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                  std::mutex *aMutex);
  ~TEyeMeasurement(){};

  void Init();
  void Execute();
  void Terminate();
  void LoopEnd(int loopIndex);
  void LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  void PrepareStep(int loopIndex);
  bool SetParameters(TScanParameters *pars);
};


#endif
