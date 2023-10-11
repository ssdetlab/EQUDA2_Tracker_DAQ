#ifndef TSCURVESCAN_H
#define TSCURVESCAN_H

#include <atomic>
#include <deque>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include "AlpideDecoder.h"
#include "Common.h"
#include "THisto.h"
#include "TScan.h"

// Test types containing threshold scan
static const std::set<std::string> kThresholdTestTypes = {
    "OB HIC Qualification Test",   "IB HIC Qualification Test", "IB Stave Qualification Test",
    "OL HS Qualification Test",    "ML HS Qualification Test",  "OL Stave Qualification Test",
    "ML Stave Qualification Test", "OL Stave Reception Test",   "ML Stave Reception Test"};


typedef struct __TSCurveParameters : TScanParameters {
  bool nominal;
  int  VPULSEH;
  int  VPULSEL;
  int  TARGET;
} TSCurveParameters;

class TSCurveScan : public TMaskScan {
protected:
  void CreateScanParameters() { m_parameters = new TSCurveParameters; };
  class THitSet {
  public:
    std::vector<TPixHit> hits{512};
    int                  board;
    int                  val;
  };

  template <typename T, int depth = 8> class TRingBuffer {
  private:
    T                data[depth];
    std::atomic<int> pos_read;
    std::atomic<int> pos_write;

  public:
    TRingBuffer() : pos_read(0), pos_write(0) {}
    TRingBuffer(const TRingBuffer &) = delete;

    bool IsEmpty() { return ((pos_read - pos_write) % depth) == 0; }

    T &      Write() { return data[pos_write % depth]; }
    const T &Read() const { return data[pos_read % depth]; }

    void Pop()
    {
      if (((pos_read - pos_write) % depth) != 0) pos_read = ++pos_read % depth;
    }

    void Push()
    {
      while (((pos_write - pos_read + 1) % depth) == 0) {
        usleep(10);
      }
      // std::this_thread::sleep_for(std::chrono::microseconds(10));
      pos_write = ++pos_write % depth;
    }
  };

  void         ConfigureFromu(TAlpide *chip);
  virtual void ConfigureChip(TAlpide *chip) = 0;
  void         ConfigureBoard(TReadoutBoard *board);
  void         RestoreNominalSettings();
  void         FillHistos(const THitSet &hs);
  // THisto CreateHisto    ();
  virtual void SetName() = 0;

  void         Histo();
  bool         m_stopped = false;
  std::thread *m_thread;

  TRingBuffer<THitSet> *m_hitsets;

public:
  TSCurveScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
              std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
              std::mutex *aMutex);
  virtual ~TSCurveScan() { delete m_hitsets; };

  THisto       CreateHisto(); // public in TScan, so...
  void         Init();
  virtual void PrepareStep(int loopIndex) = 0;
  void         LoopEnd(int loopIndex);
  void         LoopStart(int loopIndex) { m_value[loopIndex] = m_start[loopIndex]; };
  void         Execute();
  void         Terminate();
  bool         GetNominal() { return ((TSCurveParameters *)m_parameters)->nominal; };
  bool         SetParameters(TScanParameters *pars);
};

class TThresholdScan : public TSCurveScan {
  // Conducts a regular threshold scan
protected:
  void SetName();

public:
  TThresholdScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                 std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                 std::mutex *aMutex); //:
  void ConfigureChip(TAlpide *chip);
  void PrepareStep(int loopIndex);
  ~TThresholdScan(){};
  static bool isPerformedDuring(string testType)
  {
    return kThresholdTestTypes.find(testType) != kThresholdTestTypes.end();
  };
};

class TtuneVCASNScan : public TSCurveScan {
  // NOTE:  may need new destructor?
  // Conducts a threshold scan changing VCASN
protected:
  void SetName();

public:
  TtuneVCASNScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                 std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                 std::mutex *aMutex); //:
  void ConfigureChip(TAlpide *chip);
  void PrepareStep(int loopIndex);
  ~TtuneVCASNScan(){};
};

class TtuneITHRScan : public TSCurveScan {
  // Conducts a threshold scan changing ITHR (note:  needs data from VCASNscan first)
protected:
  void SetName();

public:
  TtuneITHRScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoque,
                std::mutex *aMutex); //:
  void ConfigureChip(TAlpide *chip);
  void PrepareStep(int loopIndex);
  ~TtuneITHRScan(){};
};

#endif
