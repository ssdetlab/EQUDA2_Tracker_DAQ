#ifndef TAPPLYMASK_H
#define TAPPLYMASK_H

#include <deque>
#include <mutex>
#include <vector>

#include "Common.h"
#include "THisto.h"
#include "TNoiseAnalysis.h"
#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

class TApplyMask : public TScanAnalysis {
private:
protected:
  TScanResultChip *GetChipResult() { return 0; };
  TScanResultHic * GetHicResult() { return 0; };
  void             CreateResult(){};
  void             AnalyseHisto(TScanHisto *histo) { (void)histo; };
  void             InitCounters(){};
  string           GetPreviousTestType() { return string(""); }; // no analysis result
  void             CalculatePrediction(std::string hicName) { (void)hicName; };

public:
  TApplyMask(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
             std::vector<THic *> hics, std::mutex *aMutex, TNoiseResult *aResult);
  void Initialize(){};
  void Finalize(){};
  void Run();
};

#endif
