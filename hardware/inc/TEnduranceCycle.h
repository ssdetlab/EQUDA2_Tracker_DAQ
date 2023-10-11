#ifndef TENDURANCECYCLE_H
#define TENDURANCECYCLE_H

#include <map>
#include <mutex>
#include <string>
#include <time.h>

#include "Common.h"
#include "THIC.h"
#include "THisto.h"
#include "TScan.h"


typedef struct {
  THicType m_hicType;
  bool     m_trip;
  float    m_iddaClocked;
  float    m_idddClocked;
  float    m_iddaConfigured;
  float    m_idddConfigured;
  float    m_tempStart;
  float    m_tempEnd;
  int      m_fifoErrors;
  int      m_fifoErrors0;
  int      m_fifoErrors5;
  int      m_fifoErrorsa;
  int      m_fifoErrorsf;
  int      m_fifoExceptions;
  int      m_fifoTests;
  int      m_exceptions;
  int      m_nWorkingChips;
} THicCounter;

typedef struct __TCycleParameters : TScanParameters {
  int upTime;
  int downTime;
  int nTriggers;
  int nCycles;
  int timeLimit;
} TCycleParameters;

int OpenEnduranceRecoveryFile(const char *fName, std::vector<std::string> hicNames,
                              std::deque<std::map<std::string, THicCounter>> &counterVector);

class TEnduranceCycle : public TScan {
private:
  time_t m_startTime;
  void   CreateMeasurements();
  void   ClearCounters();
  THisto CreateHisto()
  {
    THisto histo;
    return histo;
  };
  void ConfigureBoard(TReadoutBoard *board);
  void ConfigureFromu(TAlpide *chip);
  void ConfigureChip(TAlpide *chip);
  void ConfigureMask(TAlpide *chip);
  void CountWorkingChips();
  void WriteRecoveryFile();
  bool TestPattern(TAlpide *chip, int pattern, int region, int offset, THicCounter &hicCounter);
  void ReadMem(TAlpide *chip, int ARegion, int AOffset, int &AValue);
  void WriteMem(TAlpide *chip, int ARegion, int AOffset, int AValue);
  std::map<std::string, THicCounter>              m_hicCounters;
  std::vector<std::map<std::string, THicCounter>> m_counterVector;

protected:
  void CreateScanParameters() { m_parameters = new TCycleParameters; };

public:
  TEnduranceCycle(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                  std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                  std::mutex *aMutex);
  ~TEnduranceCycle(){};

  void Init();
  void Execute();
  void Terminate();
  void Next(int loopIndex);
  void LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  void LoopEnd(int loopIndex) { (void)loopIndex; };
  void PrepareStep(int loopIndex);
  bool SetParameters(TScanParameters *pars);
  void ReadRecoveredCounters(std::deque<std::map<std::string, THicCounter>> &counterVector);
  std::vector<std::map<std::string, THicCounter>> GetCounters() { return m_counterVector; };
};

#endif
