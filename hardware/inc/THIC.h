#ifndef HIC_H
#define HIC_H

#include "Common.h"
#include "TAlpide.h"
#include "TPowerBoard.h"
#include "TScanAnalysis.h"
#include <string>
#include <vector>

typedef enum { HIC_IB, HIC_OB } THicType;

class THic {
private:
protected:
  std::vector<TAlpide *> m_chips;
  TPowerBoard *          m_powerBoard;
  int                    m_moduleId;  // module ID as used in chip IDs
  int                    m_pbMod;     // module number inside power board
  int                    m_bbChannel; // channel for back bias (for stave != pbMod)
  // unique identifiers
  int                m_hicNumber; // TODO: find out name and format ...
  std::string        m_dbId;      // ... in db: int? string?
  THicClassification m_oldClass;
  THicClassification m_class;
  THicClassification m_worstScanBB;
  THicClassification m_worstScanNoBB;
  bool               m_noBB;
  virtual bool       IsOnBoard(int boardIdx, int chipId) = 0;

public:
  THic(const char *dbId, int modId, TPowerBoard *pb, int pbMod, int bbChannel = -1);
  virtual ~THic(){};
  int          GetNumber() { return m_hicNumber; };
  bool         IsPowered();
  bool         IsPoweredAnalog();
  bool         IsPoweredDigital();
  bool         IsEnabled();
  void         Disable();
  unsigned int GetNEnabledChips(int boardIdx = -1);
  unsigned int GetNEnabledChipsNoBB(int boardIdx = -1);
  unsigned int GetNEnabledChipsWithBB(int boardIdx = -1);
  virtual void PowerOn() = 0;
  void         PowerOff();
  float        GetIddd();
  float        GetIdda();
  float        GetIBias();
  float        GetVddd();
  float        GetVdda();
  float        GetVdddSet();
  float        GetVddaSet();
  float        GetVbias();
  float        GetTemperature(std::map<int, float> *chipValues = 0);
  void         ReadChipRegister(Alpide::TRegister reg, std::map<int, uint16_t> &values);
  void         ScaleVoltage(float aFactor);
  std::string  GetDbId() { return m_dbId; };
  int          GetModId() { return m_moduleId; };
  unsigned int GetNChips() { return m_chips.size(); };
  int          AddChip(TAlpide *chip);
  virtual bool ContainsChip(common::TChipIndex idx) = 0;
  bool         ContainsChip(int index);
  virtual bool ContainsReceiver(int boardIndex, int rcv) = 0;
  virtual int  GetReceiver(int boardIndex, int chipId)   = 0;
  virtual common::TChipIndex GetChipIndex(int i)         = 0;
  virtual std::vector<int>   GetBoardIndices()           = 0;
  virtual THicType           GetHicType()                = 0;
  std::vector<TAlpide *>     GetChips() { return m_chips; };
  TAlpide *                  GetChipById(int chipId);
  TPowerBoard *              GetPowerBoard() { return m_powerBoard; };
  int                        GetPbMod() { return m_pbMod; };
  int                        GetBbChannel() { return m_bbChannel; };
  void                       SwitchBias(bool on, bool force = false);
  float                      GetAnalogueVoltage(std::map<int, float> *chipValues = 0);
  float                      GetDigitalVoltage(std::map<int, float> *chipValues = 0);
  float GetSupplyVoltage(bool analogueNotDigital = true, std::map<int, float> *chipValues = 0);
  void  AddClassification(THicClassification aClass, bool backBias);
  THicClassification GetClassification();
  THicClassification GetOldClassification() { return m_oldClass; };
  void               SetOldClassification(THicClassification aOldClass) { m_oldClass = aOldClass; };
  void               SetNoBB();
  bool               GetNoBB() { return m_noBB; }
  // check if the back bias channel on the pb is enabled
  // this can be different from ~noBB in cases where two ore more HICs are connected to one bb
  // channel
  bool BiasChannelEnabled()
  {
    if (m_powerBoard) return m_powerBoard->BiasEnabled(m_bbChannel);
    return true;
  }
};

class THicOB : public THic {
private:
  int  m_boardidx0;  // readout board index for master 0
  int  m_boardidx8;  // readout board index for master 8
  int  m_rcv0;       // receiver for master 0
  int  m_rcv8;       // receiver for master 8
  int  m_ctrl0;      // control interface for master 0
  int  m_ctrl8;      // control interface for master 8
  int  m_position;   // position on half-stave
  bool m_powercombo; // powering with FB+PB+BB combo
protected:
  bool IsOnBoard(int boardIdx, int chipId);

public:
  THicOB(const char *dbId, int modId, TPowerBoard *pb, int pbMod, int bbChannel = -1,
         bool useCombo = 1);
  virtual ~THicOB(){};
  common::TChipIndex GetChipIndex(int i);
  THicType           GetHicType() { return HIC_OB; };
  std::vector<int>   GetBoardIndices();
  bool               ContainsChip(common::TChipIndex idx);
  bool               ContainsReceiver(int boardIndex, int rcv);
  bool               IsPowerCombo() { return m_powercombo; }
  virtual int        GetReceiver(int boardIndex, int chipId);
  void               SetPosition(int aPos) { m_position = aPos; };
  int                GetPosition() { return m_position; };
  void               ConfigureMaster(int Master, int board, int rcv, int ctrl);
  void               PowerOn();
};

class THicIB : public THic {
private:
  int m_boardidx;
  int m_rcv[9];
  int m_ctrl; // control interface
protected:
  bool IsOnBoard(int boardIdx, int chipId);

public:
  THicIB(const char *dbId, int modId, TPowerBoard *pb, int pbMod, int bbChannel = -1);
  virtual ~THicIB(){};
  common::TChipIndex GetChipIndex(int i);
  THicType           GetHicType() { return HIC_IB; };
  std::vector<int>   GetBoardIndices();
  bool               ContainsChip(common::TChipIndex idx);
  bool               ContainsReceiver(int boardIndex, int rcv);
  virtual int        GetReceiver(int boardIndex, int chipId);
  void               ConfigureInterface(int board, int *rcv, int ctrl);
  void               PowerOn();
};

#endif
