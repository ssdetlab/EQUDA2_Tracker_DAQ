#ifndef TLOCALBUSANALYSIS_H
#define TLOCALBUSANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "Common.h"
#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

class TLocalBusResultChip : public TScanResultChip {
  friend class TLocalBusAnalysis;

private:
  int m_err0;
  int m_err5;
  int m_erra;
  int m_errf;
  int m_errBusyOn;
  int m_errBusyOff;

public:
  TLocalBusResultChip() : TScanResultChip(){};
  void  WriteToFile(FILE *fp);
  float GetVariable(TResultVariable var);
};

class TLocalBusResultHic : public TScanResultHic {
  friend class TLocalBusAnalysis;

private:
  int m_err0;
  int m_err5;
  int m_erra;
  int m_errf;
  int m_errBusyOn;
  int m_errBusyOff;

public:
  TLocalBusResultHic() : TScanResultHic(){};
  void WriteToFile(FILE *fp);
};

class TLocalBusResult : public TScanResult {
  friend class TLocalBusAnalysis;

private:
public:
  TLocalBusResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp) { (void)fp; };
};

class TLocalBusAnalysis : public TScanAnalysis {
private:
  void InitCounters();
  void WriteResult();
  void FillVariableList();

protected:
  TScanResultChip *GetChipResult()
  {
    TLocalBusResultChip *Result = new TLocalBusResultChip();
    return Result;
  };
  TScanResultHic *GetHicResult()
  {
    TLocalBusResultHic *Result = new TLocalBusResultHic();
    return Result;
  };
  void   CreateResult(){};
  void   AnalyseHisto(TScanHisto *histo);
  string GetPreviousTestType() { return string(""); }; // done only once
  void   CalculatePrediction(std::string hicName) { (void)hicName; };

public:
  TLocalBusAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                    std::vector<THic *> hics, std::mutex *aMutex, TLocalBusResult *aResult = 0);
  void Initialize();
  void Finalize();
};

#endif
