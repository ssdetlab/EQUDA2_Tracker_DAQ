#include "THIC.h"
#include "TReadoutBoardMOSAIC.h"

#include <cstring>
#include <thread>

THic::THic(const char *id, int modId, TPowerBoard *pb, int pbMod, int bbChannel)
{
  m_dbId.assign(id);

  m_powerBoard = pb;
  m_pbMod      = pbMod;
  if (bbChannel == -1)
    m_bbChannel = pbMod;
  else
    m_bbChannel = bbChannel;
  m_moduleId      = modId;
  m_class         = CLASS_UNTESTED;
  m_oldClass      = CLASS_UNTESTED;
  m_worstScanBB   = CLASS_UNTESTED;
  m_worstScanNoBB = CLASS_UNTESTED;
  m_noBB          = false;

  m_chips.clear();
}

int THic::AddChip(TAlpide *chip)
{
  m_chips.push_back(chip);
  return GetNChips();
}

bool THic::ContainsChip(int index)
{
  common::TChipIndex idx;
  idx.boardIndex   = (index >> 8) & 0xf;
  idx.dataReceiver = (index >> 4) & 0xf;
  idx.chipId       = index & 0xf;

  return ContainsChip(idx);
}

TAlpide *THic::GetChipById(int chipId)
{
  for (unsigned int i = 0; i < m_chips.size(); i++) {
    if ((m_chips.at(i)->GetConfig()->GetChipId() & 0xf) == (chipId & 0xf)) return m_chips.at(i);
  }
  return 0;
}

// IsEnabled: returns true if at least one chip on the HIC is enabled, false otherwise
bool THic::IsEnabled() { return (GetNEnabledChips() > 0); }

unsigned int THic::GetNEnabledChips(int boardIdx)
{
  unsigned int n = 0;
  for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
    if ((boardIdx == -1) || IsOnBoard(boardIdx, m_chips.at(ichip)->GetConfig()->GetChipId())) {
      if (m_chips.at(ichip)->GetConfig()->IsEnabled()) n++;
    }
  }
  return n;
}

unsigned int THic::GetNEnabledChipsNoBB(int boardIdx)
{
  unsigned int n = 0;
  for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
    if ((boardIdx == -1) || IsOnBoard(boardIdx, m_chips.at(ichip)->GetConfig()->GetChipId())) {
      if (m_chips.at(ichip)->GetConfig()->IsEnabledNoBB()) n++;
    }
  }
  return n;
}

unsigned int THic::GetNEnabledChipsWithBB(int boardIdx)
{
  unsigned int n = 0;
  for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
    if ((boardIdx == -1) || IsOnBoard(boardIdx, m_chips.at(ichip)->GetConfig()->GetChipId())) {
      if (m_chips.at(ichip)->GetConfig()->IsEnabledWithBB()) n++;
    }
  }
  return n;
}

void THic::Disable()
{
  for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
    m_chips.at(ichip)->SetEnable(false);
  }
}

void THic::SetNoBB()
{
  m_noBB = true;
  for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
    m_chips.at(ichip)->SetEnableWithBB(false);
    if (m_powerBoard) m_powerBoard->DisableBias(m_bbChannel);
  }
}

bool THic::IsPowered()
{
return true;
  if (IsPoweredAnalog() && !IsPoweredDigital()) {
    std::cout << "Warning: found module with only AVDD on -> powering off" << std::endl;
    if (m_powerBoard) m_powerBoard->SwitchModule(m_pbMod, false);
  }
  else if (IsPoweredDigital() && !IsPoweredAnalog()) {
    std::cout << "Warning: found module with only DVDD on -> powering off" << std::endl;
    if (m_powerBoard) m_powerBoard->SwitchModule(m_pbMod, false);
  }
  return ((IsPoweredAnalog()) && (IsPoweredDigital()));
}

bool THic::IsPoweredAnalog()
{
  if (m_powerBoard && m_powerBoard->IsAnalogChOn(m_pbMod)) return true;
  return false;
}

bool THic::IsPoweredDigital()
{
  if (m_powerBoard && m_powerBoard->IsDigitalChOn(m_pbMod)) return true;
  return false;
}

void THic::PowerOff()
{
  if (!IsPowered()) return;

  if (m_powerBoard) m_powerBoard->SwitchModule(m_pbMod, false);
  // Q: do we need to consider case where part of the channels is on?
}

float THic::GetIddd()
{
  if (m_powerBoard) {
    return m_powerBoard->GetDigitalCurrent(m_pbMod);
  }
  return 0;
}

float THic::GetIdda()
{
  if (m_powerBoard) {
    return m_powerBoard->GetAnalogCurrent(m_pbMod);
  }
  return 0;
}

float THic::GetIBias()
{
  if (m_powerBoard) {
    return m_powerBoard->GetBiasCurrent();
  }
  return 0;
}

float THic::GetVddd()
{
return 1.0;
  if (m_powerBoard) {
    return m_powerBoard->GetDigitalVoltage(m_pbMod);
  }
  return 0;
}

float THic::GetVdda()
{
return 1.0;
  if (m_powerBoard) {
    return m_powerBoard->GetAnalogVoltage(m_pbMod);
  }
  return 0;
}

float THic::GetVddaSet()
{
  if (m_powerBoard) {
    return m_powerBoard->GetAnalogSetVoltage(m_pbMod);
  }
  return 0;
}

float THic::GetVdddSet()
{
  if (m_powerBoard) {
    return m_powerBoard->GetDigitalSetVoltage(m_pbMod);
  }
  return 0;
}

float THic::GetVbias()
{
  if (m_powerBoard) {
    return m_powerBoard->GetBiasVoltage();
  }
  return 0;
}


// scales all voltages and current limits of the HIC by a given factor
// e.g. aFactor = 1.1 -> +10%
// method takes the value from the config and writes the scaled value to the board
// (config value is left unchanged)

// TODO: this should be more or less OK, but it neglects the offset in the power board
// calibration.
void THic::ScaleVoltage(float aFactor)
{
  if (!m_powerBoard) return;

  TPowerBoardConfig *pbConfig = m_powerBoard->GetConfigurationHandler();
  float              AVSet, AISet, DVSet, DISet;
  bool               BiasOn;

  pbConfig->GetModuleSetUp(m_pbMod, &AVSet, &AISet, &DVSet, &DISet, &BiasOn);

  m_powerBoard->SetModule(m_pbMod, AVSet * aFactor, AISet * aFactor, DVSet * aFactor,
                          DISet * aFactor, BiasOn);
}

void THic::SwitchBias(bool on, bool force)
{
  if (!m_powerBoard) return;
  if (on && m_noBB && !force) {
    std::cout << "Warning: HIC " << GetDbId()
              << " classified as no back bias, back bias not switched" << std::endl;
    return;
  }
  if (on) {
    m_powerBoard->SetBiasOn(m_bbChannel, force);
    std::cout << "Switched on BB channel: " << m_bbChannel << std::endl;
  }
  else {
    m_powerBoard->SetBiasOff(m_bbChannel);
    std::cout << "Switched off BB channel: " << m_bbChannel << std::endl;
  }
}


void THic::ReadChipRegister(Alpide::TRegister reg, std::map<int, uint16_t> &values)
{
  uint16_t value;
  values.clear();
  for (unsigned int i = 0; i < m_chips.size(); i++) {
    if (!m_chips.at(i)->GetConfig()->IsEnabled()) continue;
    m_chips.at(i)->ReadRegister(reg, value);
    values.insert(std::pair<int, uint16_t>(m_chips.at(i)->GetConfig()->GetChipId() & 0xf, value));
  }
}


float THic::GetTemperature(std::map<int, float> *chipValues)
{
  float result = 0;
  int   nChips = 0;

  if (chipValues) chipValues->clear();
  for (unsigned int i = 0; i < m_chips.size(); i++) {
    if (!m_chips.at(i)->GetConfig()->IsEnabled()) continue;
    float temp = m_chips.at(i)->ReadTemperature();
    result += temp;
    nChips++;
    if (chipValues) {
      chipValues->insert(
          std::pair<int, float>(m_chips.at(i)->GetConfig()->GetChipId() & 0xf, temp));
    }
  }

  if (nChips > 0)
    return result / (float)nChips;
  else
    return 0.;
}

float THic::GetAnalogueVoltage(std::map<int, float> *chipValues)
{
  return GetSupplyVoltage(true, chipValues);
}

float THic::GetDigitalVoltage(std::map<int, float> *chipValues)
{
  return GetSupplyVoltage(false, chipValues);
}

float THic::GetSupplyVoltage(bool analogueNotDigital /* = true */, std::map<int, float> *chipValues)
{
  float result = 0;
  int   nChips = 0;

  if (chipValues) chipValues->clear();
  for (unsigned int i = 0; i < m_chips.size(); i++) {
    if (!m_chips.at(i)->GetConfig()->IsEnabled()) continue;
    float voltage = (analogueNotDigital) ? m_chips.at(i)->ReadAnalogueVoltage()
                                         : m_chips.at(i)->ReadDigitalVoltage();
    result += voltage;
    if (chipValues) {
      chipValues->insert(
          std::pair<int, float>(m_chips.at(i)->GetConfig()->GetChipId() & 0xf, voltage));
    }
    nChips++;
  }

  if (nChips > 0.)
    return result / (float)nChips;
  else
    return 0.;
}

void THic::AddClassification(THicClassification aClass, bool backBias)
{
  // Power test results
  if ((aClass == CLASS_GOLD_NOBB) || (aClass == CLASS_SILVER_NOBB) ||
      (aClass == CLASS_BRONZE_NOBB)) {
    m_worstScanBB = CLASS_RED;
    if ((aClass == CLASS_GOLD_NOBB) && (m_worstScanNoBB < CLASS_GOLD)) m_worstScanNoBB = CLASS_GOLD;
    if ((aClass == CLASS_SILVER_NOBB) && (m_worstScanNoBB < CLASS_SILVER))
      m_worstScanNoBB = CLASS_SILVER;
    if ((aClass == CLASS_BRONZE_NOBB) && (m_worstScanNoBB < CLASS_BRONZE))
      m_worstScanNoBB = CLASS_BRONZE;
  }
  // Result of all other tests
  else if (backBias && (aClass > m_worstScanBB)) {
    m_worstScanBB = aClass;
  }
  else if (!backBias && (aClass > m_worstScanNoBB)) {
    m_worstScanNoBB = aClass;
  }
}

THicClassification THic::GetClassification()
{
  // set ABORTED to RED
  if (m_worstScanNoBB == CLASS_ABORTED) m_worstScanNoBB = CLASS_RED;
  if (m_worstScanBB == CLASS_ABORTED) m_worstScanBB = CLASS_RED;

  // Class RED: more than 2 non-working chips or worst no BB scan RED
  if (m_chips.size() - GetNEnabledChips() > 2) return CLASS_RED;

  if (m_worstScanNoBB == CLASS_RED) return CLASS_RED;

  // Class No back bias and No back bias, cat B
  if ((m_worstScanBB == CLASS_RED) || (GetNEnabledChipsWithBB() < GetNEnabledChipsNoBB())) {
    if (m_chips.size() > GetNEnabledChips())
      return Worst(m_oldClass, CLASS_NOBBB);
    else if (m_worstScanNoBB <= CLASS_BRONZE)
      return Worst(m_oldClass, CLASS_NOBB);
    else {
      std::cout << "Warning: unconsidered case 1 in HIC classification" << std::endl;
      return CLASS_UNTESTED;
    }
  }

  // class Partial and Partial, cat B
  if (m_chips.size() - GetNEnabledChips() > 1) {
    if (Worst(m_worstScanBB, m_worstScanNoBB) <= CLASS_BRONZE)
      return Worst(m_oldClass, CLASS_PARTIALB);
    else {
      std::cout << "Warning: unconsidered case 2 in HIC classification" << std::endl;
      return CLASS_UNTESTED;
    }
  }

  if (m_chips.size() - GetNEnabledChips() == 1) {
    if (Worst(m_worstScanBB, m_worstScanNoBB) <= CLASS_BRONZE)
      return Worst(m_oldClass, CLASS_PARTIAL);
    else {
      std::cout << "Warning: unconsidered case 3 in HIC classification" << std::endl;
      return CLASS_UNTESTED;
    }
  }

  // "trivial" classes
  return Worst(m_oldClass, Worst(m_worstScanBB, m_worstScanNoBB));
}

THicIB::THicIB(const char *dbId, int modId, TPowerBoard *pb, int pbMod, int bbChannel)
    : THic(dbId, modId, pb, pbMod, bbChannel)
{
  m_ctrl = -1; // FIXME: init m_ctrl to avoid not used warning/error (clang)
}

void THicIB::ConfigureInterface(int board, int *rcv, int ctrl)
{
  m_boardidx = board;
  m_ctrl     = ctrl;
  for (int i = 0; i < 9; i++) {
    m_rcv[i] = rcv[i];
  }
}

common::TChipIndex THicIB::GetChipIndex(int i)
{
  common::TChipIndex idx;
  if (i > (int)m_chips.size()) {
    std::cout << "Error (THicIB::GetChipIndex): trying to access bad chip" << std::endl;
    return idx;
  }
  TAlpide *chip = m_chips.at(i);

  idx.chipId       = chip->GetConfig()->GetChipId() & 0xf;
  idx.boardIndex   = m_boardidx;
  idx.dataReceiver = m_rcv[idx.chipId];

  return idx;
}

std::vector<int> THicIB::GetBoardIndices()
{
  std::vector<int> Indices;
  Indices.push_back(m_boardidx);

  return Indices;
}

bool THicIB::ContainsChip(common::TChipIndex idx)
{
  // probably the check on board id is enough...
  if (((int)idx.boardIndex == m_boardidx) && (idx.chipId >= 0) && (idx.chipId < 10)) return true;
  return false;
}

bool THicIB::IsOnBoard(int boardIdx, int chipId)
{
  if (boardIdx == m_boardidx) return true;
  return false;
}


bool THicIB::ContainsReceiver(int boardIndex, int rcv)
{
  if (boardIndex != m_boardidx) return false;
  for (int i = 0; i < 9; i++) {
    if (rcv == m_rcv[i]) return true;
  }
  return false;
}

int THicIB::GetReceiver(int boardIndex, int chipId)
{
  if (boardIndex != m_boardidx) return -1;
  return m_rcv[chipId & 0xf];
}

void THicIB::PowerOn()
{
  TReadoutBoardMOSAIC *mosaic = 0;

  if (IsPowered()) return;
  mosaic = (TReadoutBoardMOSAIC *)m_chips.at(0)->GetReadoutBoard();

  mosaic->enableClockOutput(m_ctrl, false);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  if (m_powerBoard) {
    m_powerBoard->SwitchAnalogOn(m_pbMod);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    m_powerBoard->SwitchDigitalOn(m_pbMod);
  }
  // if (m_powerBoard) m_powerBoard->SwitchModule(m_pbMod, true);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  mosaic->enableClockOutput(m_ctrl, true);
}

THicOB::THicOB(const char *dbId, int modId, TPowerBoard *pb, int pbMod, int bbChannel,
               bool useCombo)
    : THic(dbId, modId, pb, pbMod, bbChannel)
{
  m_position   = 0;
  m_powercombo = useCombo;
}

common::TChipIndex THicOB::GetChipIndex(int i)
{
  common::TChipIndex idx;
  if (i > (int)m_chips.size()) {
    std::cout << "Error (THicOB::GetChipIndex): trying to access bad chip" << std::endl;
    return idx;
  }

  TAlpide *chip = m_chips.at(i);

  idx.chipId = chip->GetConfig()->GetChipId() & 0xf;
  if (idx.chipId < 7) {
    idx.boardIndex   = m_boardidx0;
    idx.dataReceiver = m_rcv0;
  }
  else {
    idx.boardIndex   = m_boardidx8;
    idx.dataReceiver = m_rcv8;
  }

  return idx;
}

std::vector<int> THicOB::GetBoardIndices()
{
  std::vector<int> Indices;
  Indices.push_back(m_boardidx0);
  Indices.push_back(m_boardidx8);

  return Indices;
}

void THicOB::ConfigureMaster(int Master, int board, int rcv, int ctrl)
{
  if (Master == 0) {
    m_boardidx0 = board;
    m_rcv0      = rcv;
    m_ctrl0     = ctrl;
  }
  else if (Master == 8) {
    m_boardidx8 = board;
    m_rcv8      = rcv;
    m_ctrl8     = ctrl;
  }
  else {
    std::cout << "Warning: bad master id, doing nothing" << std::endl;
  }
}

bool THicOB::ContainsChip(common::TChipIndex idx)
{
  if (idx.chipId < 7) {
    if (((int)idx.boardIndex == m_boardidx0) && ((int)idx.dataReceiver == m_rcv0)) return true;
  }
  else {
    if (((int)idx.boardIndex == m_boardidx8) && ((int)idx.dataReceiver == m_rcv8)) return true;
  }

  return false;
}

bool THicOB::IsOnBoard(int boardIdx, int chipId)
{
  if ((chipId & 0x8) && (boardIdx == m_boardidx8)) return true;
  if (((chipId & 0x8) == 0) && (boardIdx == m_boardidx0)) return true;
  return false;
}


bool THicOB::ContainsReceiver(int boardIndex, int rcv)
{
  if ((boardIndex == m_boardidx0) && (rcv == m_rcv0)) return true;
  if ((boardIndex == m_boardidx8) && (rcv == m_rcv8)) return true;

  return false;
}

int THicOB::GetReceiver(int boardIndex, int chipId)
{
  if (boardIndex == m_boardidx0 && (chipId & 0xf) < 8) return m_rcv0;
  if (boardIndex == m_boardidx8 && (chipId & 0xf) >= 8) return m_rcv8;

  return -1;
}

void THicOB::PowerOn()
{
  TReadoutBoardMOSAIC *mosaic = 0, *mosaic2 = 0;
  bool                 chips = (m_chips.size() > 0); // consider 0 chips in case of fast power test

  if (IsPowered()) return;
  if (chips) {
    mosaic = (TReadoutBoardMOSAIC *)m_chips.at(0)->GetReadoutBoard();

    // OB-HS -> 2 different MOSAICs
    // all other HIC types (IB and OB HIC alone) have the same MOSAIC on chip 0 and 7
    if (m_chips.at(7)->GetReadoutBoard() != m_chips.at(0)->GetReadoutBoard()) {
      mosaic2 = (TReadoutBoardMOSAIC *)m_chips.at(7)->GetReadoutBoard();
    }

    mosaic->enableClockOutput(m_ctrl0, false);
    if (mosaic2)
      mosaic2->enableClockOutput(m_ctrl8, false);
    else
      mosaic->enableClockOutput(m_ctrl8, false);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  if (m_powerBoard) {
    m_powerBoard->SwitchAnalogOn(m_pbMod);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    m_powerBoard->SwitchDigitalOn(m_pbMod);
  }
  // if (m_powerBoard) m_powerBoard->SwitchModule(m_pbMod, true);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  if (chips) {
    mosaic->enableClockOutput(m_ctrl0, true);
    if (mosaic2)
      mosaic2->enableClockOutput(m_ctrl8, true);
    else
      mosaic->enableClockOutput(m_ctrl8, true);
  }
}
