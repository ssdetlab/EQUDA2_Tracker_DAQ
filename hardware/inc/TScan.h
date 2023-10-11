#ifndef TSCAN_H
#define TSCAN_H

#include <deque>
#include <mutex>
#include <string>
#include <vector>

#include "AlpideDecoder.h"
#include "TAlpide.h"
#include "THIC.h"
#include "THisto.h"
#include "TReadoutBoard.h"
#include "TScanConfig.h"

const int MAXLOOPLEVEL = 3;
const int MAXBOARDS    = 2;

extern bool fScanAbort;        // fScanAbort stops current scan (set false in Init)
extern bool fScanAbortAll;     // fScanAbortAll stops all scans (set false in constructor)
extern bool fTimeLimitReached; // stops current scan but still lets the analysis finish; used for
                               // endurance test

class TReadoutTest;

typedef struct {
  int nEnabled;
  int n8b10b;
  int nCorruptEvent;
  int nPrioEncoder;
  int nTimeout;
  int nOversizeEvent;
} TErrorCounter;

typedef struct TScanParameters__ {
public:
  float backBias;
  virtual ~TScanParameters__(){};
} TScanParameters;

class TScanConditionsHic {
  friend class TScan;
  friend class TReadoutTest;

private:
  float                   m_tempStart;
  float                   m_tempEnd;
  float                   m_vddaChipStart;
  float                   m_vddaChipEnd;
  float                   m_vdddChipStart;
  float                   m_vdddChipEnd;
  float                   m_iddaStart;
  float                   m_iddaEnd;
  float                   m_idddStart;
  float                   m_idddEnd;
  float                   m_vddaStart;
  float                   m_vddaEnd;
  float                   m_vdddStart;
  float                   m_vdddEnd;
  float                   m_vddaSetStart;
  float                   m_vddaSetEnd;
  float                   m_vdddSetStart;
  float                   m_vdddSetEnd;
  std::map<int, float>    m_chipAnalogueVoltagesStart;
  std::map<int, float>    m_chipAnalogueVoltagesEnd;
  std::map<int, float>    m_chipDigitalVoltagesStart;
  std::map<int, float>    m_chipDigitalVoltagesEnd;
  std::map<int, float>    m_chipTempsStart;
  std::map<int, float>    m_chipTempsEnd;
  std::map<int, uint16_t> m_pllLockStart;
  std::map<int, uint16_t> m_pllLockEnd;

public:
  TScanConditionsHic(){};
};

class TScanConditions {
  friend class TScan;
  friend class TReadoutTest;

private:
  char                                        m_fwVersion[50];
  char                                        m_swVersion[50];
  std::vector<std::string>                    m_chipConfigStart;
  std::vector<std::string>                    m_chipConfigEnd;
  std::vector<std::string>                    m_boardConfigStart;
  std::vector<std::string>                    m_boardConfigEnd;
  std::map<std::string, TScanConditionsHic *> m_hicConditions;
  float                                       m_tempPT100start[2];
  float                                       m_tempPT100end[2];

public:
  TScanConditions(){};
  int AddHicConditions(std::string hicId, TScanConditionsHic *hicCond);
};

class TScan {
private:
protected:
  TScanConfig *                        m_config;
  TScanParameters *                    m_parameters;
  char                                 m_name[40];
  char                                 m_state[40];
  std::vector<TAlpide *>               m_chips;
  std::vector<THic *>                  m_hics;
  std::vector<TReadoutBoard *>         m_boards;
  std::vector<common::TChipIndex>      m_chipList;
  std::vector<uint64_t>                m_eventIds;
  std::vector<uint64_t>                m_timestamps;
  std::vector<uint32_t>                m_bunchCounters;
  std::vector<uint64_t>                m_eventIds_ref;
  std::vector<uint64_t>                m_timestamps_ref;
  std::vector<uint32_t>                m_bunchCounters_ref;
  int                                  m_firstEnabledChipId;
  int                                  m_firstEnabledBoard;
  int                                  m_firstEnabledChannel;
  int                                  m_firstEnabledChipId_ref;
  int                                  m_firstEnabledBoard_ref;
  int                                  m_firstEnabledChannel_ref;
  TScanHisto *                         m_histo;
  std::deque<TScanHisto> *             m_histoQue;
  std::mutex *                         m_mutex;
  bool                                 m_running;
  TScanConditions                      m_conditions;
  std::map<std::string, TErrorCounter> m_errorCounts;
  int                                  m_start[MAXLOOPLEVEL];
  int                                  m_stop[MAXLOOPLEVEL];
  int                                  m_step[MAXLOOPLEVEL];
  int                                  m_value[MAXLOOPLEVEL];
  int m_enabled[MAXBOARDS]; // number of enabled chips per readout board
  std::chrono::time_point<std::chrono::system_clock> time_start, time_end;

  void           SetBackBias();
  void           SwitchOffBackbias();
  void           CountEnabledChips();
  int            FindBoardIndex(TAlpide *chip);
  std::string    FindHIC(int boardIndex, int rcv);
  virtual THisto CreateHisto() = 0;
  //  virtual void CreateScanParameters() = 0;
  void DumpHitInformation(std::vector<TPixHit> *Hits);

  TPowerBoardConfig::pb_t GetPBtype(THic *hic) const;
  void                    CorrectVoltageDrop(bool reset = false);

public:
  TScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
        std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue, std::mutex *aMutex);
  virtual ~TScan() { delete m_histo; }

  virtual void Init();
  void         InitBase(bool saveStartConditions);
  virtual void Terminate();
  virtual void LoopStart(int loopIndex)   = 0;
  virtual void LoopEnd(int loopIndex)     = 0;
  virtual void PrepareStep(int loopIndex) = 0;
  virtual void Execute()                  = 0;
  void         SaveStartConditions();
  void         ClearHistoQue();
  bool         Loop(int loopIndex);
  virtual void Next(int loopIndex);
  void         CreateScanHisto();
  bool         IsRunning() { return m_running; };
  //  TScanHisto       GetTScanHisto     () {return *m_histo;};
  const char *     GetName() { return m_name; };
  const char *     GetState() { return m_state; };
  TScanConditions *GetConditions() { return &m_conditions; };
  TScanParameters *GetParameters() { return m_parameters; };
  float            GetBackbias() { return m_parameters->backBias; };
  virtual bool     SetParameters(TScanParameters *pars)
  {
    (void)pars;
    return false;
  };
  TErrorCounter                   GetErrorCount(std::string hicId);
  void                            CreateHicConditions();
  void                            WriteConditions(const char *fName, THic *aHic);
  void                            WriteChipRegisters(const char *fName);
  void                            WriteBoardRegisters(const char *fName);
  void                            ActivateTimestampLog();
  void                            WriteTimestampLog(const char *fName);
  bool                            HasBackBias() { return (m_parameters->backBias > 1.1); };
  float                           GetBackBias() { return m_parameters->backBias; };
  std::vector<common::TChipIndex> GetChipList() { return m_chipList; };
};

class TMaskScan : public TScan {
private:
protected:
  int                  m_pixPerStage;
  int                  m_nTriggers;
  int                  m_row;
  std::vector<TPixHit> m_stuck;
  TErrorCounter        m_errorCount;
  virtual void         ConfigureMaskStage(TAlpide *chip, int istage);
  void                 FindTimeoutHics(int iboard, int *triggerCounts);
  void                 ReadEventData(std::vector<TPixHit> *Hits, int iboard);

public:
  TMaskScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
            std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue,
            std::mutex *aMutex);
  ~TMaskScan(){};
  std::vector<TPixHit> GetStuckPixels() { return m_stuck; };
  TErrorCounter        GetErrorCount() { return m_errorCount; };
};

#endif
