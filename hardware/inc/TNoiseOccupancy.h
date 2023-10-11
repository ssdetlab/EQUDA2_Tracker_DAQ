#ifndef TNOISEOCCUPANCY_H
#define TNOISEOCCUPANCY_H

#include <deque>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "AlpideDecoder.h"
#include "Common.h"
#include "TDataTaking.h"
#include "THisto.h"
#include "TScan.h"

// Test types containing noise occupancy
static const std::set<std::string> kNoiseTestTypes = {
    "OB HIC Qualification Test",   "IB HIC Qualification Test", "IB Stave Qualification Test",
    "OL HS Qualification Test",    "ML HS Qualification Test",  "OL Stave Qualification Test",
    "ML Stave Qualification Test", "OL Stave Reception Test",   "ML Stave Reception Test"};


typedef struct __TNoiseParameters : TDataTakingParameters {
  bool isMasked;
} TNoiseParameters;

class TNoiseOccupancy : public TDataTaking {
private:
  void ConfigureChip(TAlpide *chip);
  void ConfigureMask(TAlpide *chip, std::vector<TPixHit> *MaskedPixels);
  void SetName();

protected:
  THisto CreateHisto();
  void   CreateScanParameters() { m_parameters = new TNoiseParameters; };

public:
  TNoiseOccupancy(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                  std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                  std::mutex *aMutex);
  ~TNoiseOccupancy(){};
  void        Init();
  void        PrepareStep(int loopIndex) { (void)(&loopIndex); };
  bool        SetParameters(TScanParameters *pars);
  void        LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  static bool isPerformedDuring(string testType)
  {
    return kNoiseTestTypes.find(testType) != kNoiseTestTypes.end();
  };
};

#endif
