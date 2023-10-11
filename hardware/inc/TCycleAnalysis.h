#ifndef TCYCLEANALYSIS_H
#define TCYCLEANALYSIS_H

#include "TScan.h"
#include "TScanAnalysis.h"
#include "TScanConfig.h"

class TCycleResultChip : public TScanResultChip {
public:
  TCycleResultChip() : TScanResultChip(){};
  void  WriteToFile(FILE *fp) { (void)fp; };
  float GetVariable(TResultVariable var);
};

class TCycleResultHic : public TScanResultHic {
  friend class TCycleAnalysis;

private:
  float m_weight;
  int   m_nTrips;
  int   m_minWorkingChips;
  int   m_nChipFailures;
  int   m_nExceptions;
  int   m_nFifoTests;
  int   m_nFifoExceptions;
  int   m_nFifoErrors;
  int   m_nFifoErrors0;
  int   m_nFifoErrors5;
  int   m_nFifoErrorsa;
  int   m_nFifoErrorsf;
  float m_avDeltaT;
  float m_maxDeltaT;
  float m_avIdda;
  float m_maxIdda;
  float m_minIdda;
  float m_avIddd;
  float m_maxIddd;
  float m_minIddd;
  char  m_cycleFile[300];
  void  SetCycleFile(const char *fName) { strncpy(m_cycleFile, fName, sizeof(m_cycleFile)); };

protected:
public:
  TCycleResultHic() : TScanResultHic(){};
  void WriteToFile(FILE *fp);
  // void WriteToDB(AlpideDB *db, ActivityDB::activity &activity);
  void Add(TCycleResultHic &aResult);
};

class TCycleResult : public TScanResult {
  friend class TCycleAnalysis;

private:
  int m_nCycles;

protected:
public:
  TCycleResult() : TScanResult(){};
  void WriteToFileGlobal(FILE *fp);
};

class TCycleAnalysis : public TScanAnalysis {
private:
protected:
  TScanResultChip *GetChipResult()
  {
    TCycleResultChip *Result = new TCycleResultChip();
    return Result;
  };
  TScanResultHic *GetHicResult()
  {
    TCycleResultHic *Result = new TCycleResultHic();
    return Result;
  };
  void               CreateResult(){};
  void               InitCounters();
  void               WriteResult();
  void               AnalyseHisto(TScanHisto *histo) { (void)histo; };
  string             GetPreviousTestType() { return string(""); }; // done only once
  void               CalculatePrediction(std::string hicName) { (void)hicName; };
  THicClassification GetClassificationOB(TCycleResultHic *result);

public:
  TCycleAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig,
                 std::vector<THic *> hics, std::mutex *aMutex, TCycleResult *aResult = 0);
  void Initialize()
  {
    CreateHicResults();
    InitCounters();
  }; // initcounters normally executed in TScanAnalysis::Run
  void               Run(){};
  void               Finalize();
  THicClassification ReClassify(TCycleResultHic *result);
};

#endif
