#ifndef TEYEANALYSIS_H
#define TEYEANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

class TVirtualPad;
class TH2;

class TEyeResultChip : public TScanResultChip {
  friend class TEyeAnalysis;

private:
public:
  TEyeResultChip() : TScanResultChip(){};
  void  WriteToFile(FILE *fp) { (void)fp; }; // TODO
  float GetVariable(TResultVariable var)
  {
    (void)var;
    return 0;
  }; // TODO
};


class TEyeResultHic : public TScanResultHic {
  friend class TEyeAnalysis;

private:
public:
  TEyeResultHic() : TScanResultHic(){};
  TScanParameters *GetScanParameters() const { return m_scanParameters; }
  void             WriteToFile(FILE *fp) { (void)fp; }; // TODO
  // void             WriteToDB(AlpideDB *db, ActivityDB::activity &activity)
  // {
    // (void)db;
    // (void)activity;
  // }; // TODO
};


class TEyeResult : public TScanResult {
  friend class TEyeAnalysis;

public:
  TEyeResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp) { (void)fp; }; // TODO
};


class TEyeAnalysis : public TScanAnalysis {
private:
  void FillVariableList(){};

protected:
  TScanResultChip *GetChipResult()
  {
    TEyeResultChip *Result = new TEyeResultChip();
    return Result;
  };
  TScanResultHic *GetHicResult()
  {
    TEyeResultHic *Result = new TEyeResultHic();
    return Result;
  };
  void   CreateResult(){};
  void   AnalyseHisto(TScanHisto *histo);              // TODO
  string GetPreviousTestType() { return string(""); }; // TODO
  void   InitCounters(){};                             // TODO
  void   CalculatePrediction(std::string hicName) { (void)hicName; };

public:
  TEyeAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
               std::vector<THic *> hics, std::mutex *aMutex, TEyeResult *aResult = 0);
  void Initialize(); // TODO
  void Finalize(){}; // TODO

  static void PlotHisto(TVirtualPad &p, TH2 &h, const std::string &filename = "");
};


#endif
