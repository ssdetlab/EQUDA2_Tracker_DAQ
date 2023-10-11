#ifndef TFIFOANALYSIS_H
#define TFIFOANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

typedef struct {
  int boardIndex;
  int receiver;
  int chipId;
  int err0;
  int err5;
  int erra;
  int errf;
  int exc;
} TFifoCounter;

class TFifoResultChip : public TScanResultChip {
  friend class TFifoAnalysis;

private:
  int m_err0;
  int m_err5;
  int m_erra;
  int m_errf;
  int m_exc;

public:
  TFifoResultChip() : TScanResultChip(){};
  void  WriteToFile(FILE *fp);
  float GetVariable(TResultVariable var);
};

class TFifoResultHic : public TScanResultHic {
  friend class TFifoAnalysis;

private:
  int  m_err0;
  int  m_err5;
  int  m_erra;
  int  m_errf;
  bool m_upper;
  bool m_lower;
  bool m_nominal;
  int  m_driver;
  int  m_exc;
  int  m_nFaultyChips;
  void GetParameterSuffix(std::string &suffix, std::string &file_suffix);

public:
  TFifoResultHic() : TScanResultHic(){};
  void WriteToFile(FILE *fp);
  // void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
};

class TFifoResult : public TScanResult {
  friend class TFifoAnalysis;

public:
  TFifoResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp) { (void)fp; };
};

class TFifoAnalysis : public TScanAnalysis {
private:
  std::vector<TFifoCounter> m_counters;
  void                      InitCounters();
  void                      WriteResult();
  void                      FillVariableList();
  THicClassification        GetClassification(TFifoResultHic *result);

protected:
  TScanResultChip *GetChipResult()
  {
    TFifoResultChip *Result = new TFifoResultChip();
    return Result;
  };
  TScanResultHic *GetHicResult()
  {
    TFifoResultHic *Result = new TFifoResultHic();
    return Result;
  };
  void   CreateResult(){};
  void   AnalyseHisto(TScanHisto *histo);
  string GetPreviousTestType();
  void   CalculatePrediction(std::string hicName) { (void)hicName; };

public:
  TFifoAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                std::vector<THic *> hics, std::mutex *aMutex, TFifoResult *aResult = 0);
  void Initialize();
  void Finalize();

  std::string GetName() { return "FIFO analysis"; }
};

#endif
