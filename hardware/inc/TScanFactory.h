#ifndef TSCANFACTORY_H
#define TSCANFACTORY_H

#include <deque>
#include <vector>

#include "TScan.h"

typedef enum {
  STPower,
  STFifo,
  STLocalBus,
  STDigital,
  STDigitalWF,
  STThreshold,
  STVCASN,
  STITHR,
  STApplyITHR,
  STApplyVCASN,
  STApplyMask,
  STClearMask,
  STNoise,
  STReadout,
  STEndurance,
  STFastPowerTest,
  STDctrl,
  STEyeScan
} TScanType;

class TAlpide;
class THic;
class TScan;
class TScanAnalysis;
class TScanConfig;
class TScanResult;
class TReadoutBoard;

class TScanFactory {
public:
  struct TScanObjects {
    TScan *        scan;
    TScanResult *  result;
    TScanAnalysis *analysis;
    bool           hasButton;
  };

  static TScanObjects CreateScanObjects(TScanType scanType, TScanConfig *config,
                                        std::vector<TAlpide *> chips, std::vector<THic *> hics,
                                        std::vector<TReadoutBoard *> boards,
                                        std::deque<TScanHisto> *histoQue, std::mutex *mutex,
                                        TScanResult *result);
};
#endif
