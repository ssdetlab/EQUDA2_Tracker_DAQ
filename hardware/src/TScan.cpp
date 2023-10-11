#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string.h>
#include <string>
#include <typeinfo>

#include "AlpideConfig.h"
#include "Common.h"
#include "TBoardConfigMOSAIC.h"
#include "TDigitalScan.h"
#include "TPowerTest.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
// #include "TReadoutBoardRU.h"
#include "TScan.h"
// #include "version.h"

bool fScanAbort;
bool fScanAbortAll;
bool fTimeLimitReached;

TScan::TScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
             std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue,
             std::mutex *aMutex)
    : m_histo(nullptr), time_start(), time_end()
{
  m_config = config;
  m_chips  = chips;
  m_hics   = hics;
  m_boards = boards;

  m_firstEnabledChipId  = -1;
  m_firstEnabledBoard   = -1U;
  m_firstEnabledChannel = -1U;

  m_firstEnabledChipId_ref  = -1;
  m_firstEnabledBoard_ref   = -1U;
  m_firstEnabledChannel_ref = -1U;

  m_histoQue = histoQue;
  m_mutex    = aMutex;

  m_running     = false;
  fScanAbort    = false;
  fScanAbortAll = false;

  strcpy(m_state, "Waiting");
  CreateHicConditions();
}

void TScan::Init() { InitBase(true); }

void TScan::InitBase(bool saveStartConditions)
{
  fScanAbort        = false;
  fTimeLimitReached = false;

  strcpy(m_state, "Running");
  std::cout << std::endl
            << std::endl
            << ">>>>>>>> Starting scan " << GetName() << std::endl
            << std::endl;
  time_t     t   = time(0); // get time now
  struct tm *now = localtime(&t);
  time_start     = std::chrono::system_clock::now();

  sprintf(m_config->GetfNameSuffix(), "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100,
          now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

  std::cout << "Output file time suffix: " << m_config->GetfNameSuffix() << std::endl;

  // Power on HIC if not yet done (PowerOn() checks if already powered)
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    m_hics.at(ihic)->PowerOn();
    if (!m_hics.at(ihic)->GetPowerBoard()) continue;
  }

  CorrectVoltageDrop();

  if (saveStartConditions) {
    SaveStartConditions();
  }
}

void TScan::SaveStartConditions()
{
  usleep(1000); // let the system settle
  // std::cout << "Waiting for the measurement" << std::endl;
  // std::cout << "Waiting for the measurement" << std::endl;
  // sleep(1000);


  TReadoutBoardMOSAIC *mosaic = dynamic_cast<TReadoutBoardMOSAIC *>(m_boards.at(0));
  if (mosaic) {
    strcpy(m_conditions.m_fwVersion, mosaic->GetFwIdString());
  }
  // strcpy(m_conditions.m_swVersion, VERSION);
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    try {
      if (m_hics.at(ihic)->GetPowerBoard()) {
        m_conditions.m_tempPT100start[0] = m_hics.at(ihic)->GetPowerBoard()->GetStaveTemperature(0);
        m_conditions.m_tempPT100start[1] = m_hics.at(ihic)->GetPowerBoard()->GetStaveTemperature(1);
      }
      break;
    }
    catch (std::exception &e) {
      std::cout << "Init: Exception " << e.what()
                << " when reading the additional power board temperature sensors" << std::endl;
    }
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    try {
      m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_iddaStart =
          m_hics.at(ihic)->GetIdda();
      m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_idddStart =
          m_hics.at(ihic)->GetIddd();
      m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vddaStart =
          m_hics.at(ihic)->GetVdda();
      m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vdddStart =
          m_hics.at(ihic)->GetVddd();
      m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vddaSetStart =
          m_hics.at(ihic)->GetVddaSet();
      m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vdddSetStart =
          m_hics.at(ihic)->GetVdddSet();
    }
    catch (std::exception &e) {
      std::cout << "Init: Exception " << e.what()
                << " when reading power board voltages / currents for HIC " << ihic << std::endl;
    }

    if (dynamic_cast<TDigitalScan *>(this) ||
        (m_config->GetTestType() != TTestType::OBHalfStaveOLFAST &&
         m_config->GetTestType() != TTestType::OBHalfStaveMLFAST &&
         m_config->GetTestType() != TTestType::OBStaveOLFAST &&
         m_config->GetTestType() != TTestType::OBStaveMLFAST)) {
      try {
        m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_tempStart =
            m_hics.at(ihic)->GetTemperature(
                &(m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_chipTempsStart));
        m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vddaChipStart =
            m_hics.at(ihic)->GetAnalogueVoltage(
                &(m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())
                      ->m_chipAnalogueVoltagesStart));
        if (m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vddaChipStart < 1.6) {
          std::cerr << std::endl
                    << "W A R N I N G :"
                    << "Analogue voltage at HIC " << m_hics.at(ihic)->GetDbId()
                    << " below 1.6V! Please check your setup" << std::endl
                    << std::endl;
        }
        m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vdddChipStart =
            m_hics.at(ihic)->GetDigitalVoltage(
                &(m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())
                      ->m_chipDigitalVoltagesStart));
        if (m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vdddChipStart < 1.6) {
          std::cerr << std::endl
                    << "W A R N I N G :"
                    << "Digital voltage at HIC " << m_hics.at(ihic)->GetDbId()
                    << " below 1.6V! Please check your setup" << std::endl
                    << std::endl;
        }
      }
      catch (std::exception &e) {
        std::cout << "Init: Exception " << e.what() << " when reading chip temp / currents for HIC "
                  << ihic << std::endl;
      }
    }

    TErrorCounter errCount;
    errCount.nEnabled       = m_hics.at(ihic)->GetNEnabledChips();
    errCount.n8b10b         = 0;
    errCount.nCorruptEvent  = 0;
    errCount.nPrioEncoder   = 0;
    errCount.nTimeout       = 0;
    errCount.nOversizeEvent = 0;
    m_errorCounts.insert(
        std::pair<std::string, TErrorCounter>(m_hics.at(ihic)->GetDbId(), errCount));
  }

  if (dynamic_cast<TDigitalScan *>(this) ||
      (m_config->GetTestType() != TTestType::OBHalfStaveOLFAST &&
       m_config->GetTestType() != TTestType::OBHalfStaveMLFAST &&
       m_config->GetTestType() != TTestType::OBStaveOLFAST &&
       m_config->GetTestType() != TTestType::OBStaveMLFAST)) {
    for (const auto &rChip : m_chips) {
      if (rChip->GetConfig()->IsEnabled() || (rChip->GetConfig()->GetParamValue("PREVID") != -1)) {
        try {
          m_conditions.m_chipConfigStart.push_back(rChip->DumpRegisters());
        }
        catch (std::exception &e) {
          std::cout << "Init: exception " << e.what() << " when reading registers for chip "
                    << rChip->GetConfig()->GetChipId() << std::endl;
        }
      }
    }
  }

  for (const auto &rBoard : m_boards) {
    if (TReadoutBoardMOSAIC *rMOSAIC = dynamic_cast<TReadoutBoardMOSAIC *>(rBoard)) {
      m_conditions.m_boardConfigStart.push_back(rMOSAIC->GetRegisterDump());
    }
  }
  std::cout << "Saved the conditions" << std::endl;
}


void TScan::SetBackBias()
{
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TPowerBoard *pb = m_hics.at(ihic)->GetPowerBoard();
    if (!pb) continue;
    if (m_parameters->backBias == 0) {
      m_hics.at(ihic)->SwitchBias(false);
      pb->SetBiasVoltage(0);
      m_config->SetBackBiasActive(false);
    }
    else {
      if (m_parameters->backBias > 1.1) {
        m_hics.at(ihic)->SwitchBias(true);
      }
      else {
        // bias voltage <= 1V -> force back bias also for noBB modules
        m_hics.at(ihic)->SwitchBias(true, true);
      }
      pb->SetBiasVoltage((-1.) * m_parameters->backBias);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      // consider scans up to -1V as no back bias
      if (m_parameters->backBias > 1.1) {
        m_config->SetBackBiasActive(true);
      }
      else {
        m_config->SetBackBiasActive(false);
      }
    }
  }
}


void TScan::SwitchOffBackbias()
{
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (m_parameters->backBias != 0) {
      TPowerBoard *pb = m_hics.at(ihic)->GetPowerBoard();
      if (!pb) continue;
      m_hics.at(ihic)->SwitchBias(false);
      pb->SetBiasVoltage(0);
    }
  }
  m_config->SetBackBiasActive(false);
}


void TScan::ClearHistoQue()
{
  while (!(m_mutex->try_lock()))
    ;
  m_histoQue->clear();
  m_mutex->unlock();
}


// seems the board index is not accessible anywhere.
// for the time being do like this...
int TScan::FindBoardIndex(TAlpide *chip)
{
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    if (m_boards.at(i) == chip->GetReadoutBoard()) return i;
  }
  return -1;
}

std::string TScan::FindHIC(int boardIndex, int rcv)
{
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    if (m_hics.at(i)->ContainsReceiver(boardIndex, rcv)) {
      return m_hics.at(i)->GetDbId();
    }
  }
  return std::string("None");
}

void TScan::Terminate()
{
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    try {
      if (m_hics.at(ihic)->GetPowerBoard()) {
        m_conditions.m_tempPT100end[0] = m_hics.at(ihic)->GetPowerBoard()->GetStaveTemperature(0);
        m_conditions.m_tempPT100end[1] = m_hics.at(ihic)->GetPowerBoard()->GetStaveTemperature(1);
      }
      break;
    }
    catch (std::exception &e) {
      std::cout << "Init: Exception " << e.what()
                << " when reading the additional power board temperature sensors" << std::endl;
    }
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    if ((m_config->GetTestType() != OBEndurance) && (typeid(*this) != typeid(TPowerTest))) {
      if (!m_hics.at(ihic)->IsPowered()) {
        throw std::runtime_error("TScan terminate: HIC powered off (Retry suggested)");
      }
      else if ((m_hics.at(ihic)->GetVddd() < 0.1) || (m_hics.at(ihic)->GetVdda() < 0.1)) {
        throw std::runtime_error("TScan terminate: voltage appears to be off (Retry suggested)");
      }
    }
    try {
      m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_iddaEnd =
          m_hics.at(ihic)->GetIdda();
      m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_idddEnd =
          m_hics.at(ihic)->GetIddd();
      m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vddaEnd =
          m_hics.at(ihic)->GetVdda();
      m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vdddEnd =
          m_hics.at(ihic)->GetVddd();
      m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vddaSetEnd =
          m_hics.at(ihic)->GetVddaSet();
      m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vdddSetEnd =
          m_hics.at(ihic)->GetVdddSet();
    }
    catch (std::exception &e) {
      std::cout << "Terminate: exception " << e.what()
                << " when reading power board voltages / currents for HIC " << ihic << std::endl;
    }

    if (m_config->GetTestType() != TTestType::OBHalfStaveOLFAST &&
        m_config->GetTestType() != TTestType::OBHalfStaveMLFAST &&
        m_config->GetTestType() != TTestType::OBStaveOLFAST &&
        m_config->GetTestType() != TTestType::OBStaveMLFAST) {
      try {
        m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_tempEnd =
            m_hics.at(ihic)->GetTemperature(
                &(m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_chipTempsEnd));
        m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vddaChipEnd =
            m_hics.at(ihic)->GetAnalogueVoltage(
                &(m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())
                      ->m_chipAnalogueVoltagesEnd));
        if (m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vddaChipEnd < 1.6) {
          std::cerr << std::endl
                    << "W A R N I N G :"
                    << "Analogue voltage at HIC " << m_hics.at(ihic)->GetDbId()
                    << " below 1.6V! Please check your setup" << std::endl
                    << std::endl;
        }
        m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())
            ->m_vdddChipEnd = m_hics.at(ihic)->GetDigitalVoltage(&(
            m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_chipDigitalVoltagesEnd));
        if (m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vdddChipEnd < 1.6) {
          std::cerr << std::endl
                    << "W A R N I N G :"
                    << "Digital voltage at HIC " << m_hics.at(ihic)->GetDbId()
                    << " below 1.6V! Please check your setup" << std::endl
                    << std::endl;
        }
      }
      catch (std::exception &e) {
        std::cout << "Terminate: exception " << e.what()
                  << " when reading chip temp / currents for HIC " << ihic << std::endl;
      }
    }
  }
  time_end      = std::chrono::system_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::minutes>(time_end - time_start);
  snprintf(m_state, sizeof(m_state), "Done (in %3d min)", int(duration.count()));

  if (m_config->GetTestType() != TTestType::OBHalfStaveOLFAST &&
      m_config->GetTestType() != TTestType::OBHalfStaveMLFAST &&
      m_config->GetTestType() != TTestType::OBStaveOLFAST &&
      m_config->GetTestType() != TTestType::OBStaveMLFAST) {
    for (const auto &rChip : m_chips) {
      if (rChip->GetConfig()->IsEnabled() || (rChip->GetConfig()->GetParamValue("PREVID") != -1)) {
        try {
          m_conditions.m_chipConfigEnd.push_back(rChip->DumpRegisters());
        }
        catch (std::exception &e) {
          std::cout << "Terminate: exception " << e.what() << " when reading registers for chip "
                    << rChip->GetConfig()->GetChipId() << std::endl;
        }
      }
    }
  }

  for (const auto &rBoard : m_boards) {
    if (TReadoutBoardMOSAIC *rMOSAIC = dynamic_cast<TReadoutBoardMOSAIC *>(rBoard)) {
      m_conditions.m_boardConfigEnd.push_back(rMOSAIC->GetRegisterDump());
    }
  }

  // reset voltage drop correction, reset chips, apply voltage drop correction to reset state
  CorrectVoltageDrop(true);

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
  }

  CorrectVoltageDrop();

  delete m_histo;
  m_histo = nullptr;
}

bool TScan::Loop(int loopIndex)
{
  if (fScanAbort) return false; // check for abort flags first
  if (fScanAbortAll) return false;
  if (fTimeLimitReached) return false;

  if ((m_step[loopIndex] > 0) && (m_value[loopIndex] < m_stop[loopIndex]))
    return true; // limit check for positive steps
  if ((m_step[loopIndex] < 0) && (m_value[loopIndex] > m_stop[loopIndex]))
    return true; // same for negative steps

  return false;
}

void TScan::Next(int loopIndex) { m_value[loopIndex] += m_step[loopIndex]; }

void TScan::CountEnabledChips()
{

  // std::cout << "in count enabled chips, boards_size = " << m_boards.size() << ", chips_size = "
  // << m_chips.size() << std::endl;
  for (int i = 0; i < MAXBOARDS; i++) {
    m_enabled[i] = 0;
  }
  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if ((m_chips.at(ichip)->GetConfig()->IsEnabled()) &&
          (m_chips.at(ichip)->GetReadoutBoard() == m_boards.at(iboard))) {
        m_enabled[iboard]++;
      }
    }
  }
}

void TScan::CreateScanHisto()
{
  common::TChipIndex id;
  m_histo = new TScanHisto();

  THisto histo = CreateHisto();

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if ((m_chips.at(ichip)->GetConfig()->IsEnabled()) &&
          (m_chips.at(ichip)->GetReadoutBoard() == m_boards.at(iboard))) {
        id.boardIndex   = iboard;
        id.dataReceiver = m_chips.at(ichip)->GetConfig()->GetParamValue("RECEIVER");
        id.chipId       = m_chips.at(ichip)->GetConfig()->GetChipId();

        m_histo->AddHisto(id, histo);
      }
    }
  }
  std::cout << "CreateHisto: generated map with " << m_histo->GetSize() << " elements" << std::endl;
  m_histo->GetChipList(m_chipList);
}

void TScan::ActivateTimestampLog()
{
  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    TReadoutBoardMOSAIC *b = dynamic_cast<TReadoutBoardMOSAIC *>(m_boards.at(iboard));
    if (b) b->setReadTriggerInfo(true);
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if ((m_chips.at(ichip)->GetConfig()->IsEnabled()) &&
          (m_chips.at(ichip)->GetReadoutBoard() == m_boards.at(iboard))) {
        if (iboard == 0) {
          if (m_firstEnabledChipId < 0) {
            m_firstEnabledChipId = m_chips.at(ichip)->GetConfig()->GetChipId() & 0xf;
            m_firstEnabledBoard  = iboard;
            m_firstEnabledChannel =
                m_chips.at(ichip)->GetHic()->GetReceiver(iboard, m_firstEnabledChipId);
            std::cout << "Chip ID used for timestamp logging: "
                      << m_chips.at(ichip)->GetConfig()->GetChipId() << " (" << m_firstEnabledChipId
                      << ")" << std::endl;
            std::cout << m_firstEnabledChipId << '\t' << m_firstEnabledBoard << '\t'
                      << m_firstEnabledChannel << std::endl;
          }
        }
        else if (m_firstEnabledChipId_ref < 0) {
          m_firstEnabledChipId_ref = m_chips.at(ichip)->GetConfig()->GetChipId() & 0xf;
          m_firstEnabledBoard_ref  = iboard;
          m_firstEnabledChannel_ref =
              m_chips.at(ichip)->GetHic()->GetReceiver(iboard, m_firstEnabledChipId_ref);
          std::cout << "Chip ID used as timestamp logging reference: "
                    << m_chips.at(ichip)->GetConfig()->GetChipId() << " ("
                    << m_firstEnabledChipId_ref << ")" << std::endl;
          std::cout << m_firstEnabledChipId_ref << '\t' << m_firstEnabledBoard_ref << '\t'
                    << m_firstEnabledChannel_ref << std::endl;
        }
        if (m_firstEnabledChipId > 0 && m_firstEnabledChipId_ref > 0) return;
      }
    }
  }
}

void TScan::WriteTimestampLog(const char *fName)
{
  if (m_eventIds.size() == 0) return;
  if (m_eventIds_ref.size() == 0) return;

  std::ofstream output(fName, std::fstream::out | std::fstream::app);

  output << "### Timestamp Log" << std::endl;
  uint32_t lastBC     = 0;
  uint32_t lastBC_ref = 0;

  uint32_t histo[257] = {0};
  for (unsigned int iEvent = 0; iEvent < m_eventIds.size() && iEvent < m_eventIds_ref.size();
       iEvent++) {
    unsigned int diff = (lastBC < m_bunchCounters[iEvent]) ? (m_bunchCounters[iEvent] - lastBC)
                                                           : 256 - lastBC + m_bunchCounters[iEvent];
    unsigned int diff_ref = (lastBC_ref < m_bunchCounters_ref[iEvent])
                                ? (m_bunchCounters_ref[iEvent] - lastBC_ref)
                                : 256 - lastBC_ref + m_bunchCounters_ref[iEvent];
    if (diff > 0 && diff < 257) {
      ++histo[diff];
    }
    else {
      std::cerr << "WriteTimestampLog: Index out of range: " << diff << std::endl;
    }
    output << iEvent << '\t' << m_eventIds[iEvent] << '\t' << m_timestamps[iEvent] << '\t'
           << m_bunchCounters[iEvent] << '\t' << m_bunchCounters[iEvent] * 200 << '\t' << diff * 200
           << '\t' << m_eventIds_ref[iEvent] << '\t' << m_timestamps_ref[iEvent] << '\t'
           << m_bunchCounters_ref[iEvent] << '\t' << m_bunchCounters_ref[iEvent] * 200 << '\t'
           << diff_ref * 200 << '\t' << (diff - diff_ref) * 200 << std::endl;
    lastBC     = m_bunchCounters[iEvent];
    lastBC_ref = m_bunchCounters_ref[iEvent];
  }

  output << std::endl << std::endl;
  output << " ## Timestamp histogram" << std::endl;
  for (unsigned int i = 0; i < 257; ++i) {
    output << i << "\t" << i * 200 << "\t" << histo[i] << "\t"
           << (double)histo[i] / (double)m_eventIds.size() << std::endl;
  }

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    output << " ## BOARD TIMESTAMPS ##" << std::endl;
    TReadoutBoardMOSAIC *b = dynamic_cast<TReadoutBoardMOSAIC *>(m_boards.at(iboard));
    if (b) {
      std::vector<uint32_t> *triggerIDs = b->getTriggerNums();
      std::vector<uint64_t> *timestamps = b->getTriggerTimes();
      for (unsigned long i = 0; i < triggerIDs->size(); ++i) {
        output << iboard << "\t" << i << "\t" << triggerIDs->at(i) << "\t" << timestamps->at(i)
               << std::endl;
      }
    }
  }
  output << std::endl;
  output.close();
}

TErrorCounter TScan::GetErrorCount(std::string hicId)
{
  auto hicCount = m_errorCounts.find(hicId);

  if (hicCount != m_errorCounts.end()) {
    return hicCount->second;
  }
  else {
    std::cout << "WARNING (TScan::GetErrorCount), hic not found, returning empty counter"
              << std::endl;
    TErrorCounter result;
    return result;
  }
}

void TScan::DumpHitInformation(std::vector<TPixHit> *Hits)
{
  unsigned int linkInfo[MAXBOARDS][MAX_MOSAICTRANRECV];

  for (unsigned int iboard = 0; iboard < MAXBOARDS; iboard++) {
    for (unsigned int ich = 0; ich < MAX_MOSAICTRANRECV; ich++) {
      linkInfo[iboard][ich] = 0;
    }
  }

  std::cout << "Dumping hit data to file EventData.dat" << std::endl;
  FILE *fp = fopen("EventData.dat", "w");

  for (unsigned int i = 0; i < Hits->size(); i++) {
    fprintf(fp, "%d %d %d %d %d %d\n", Hits->at(i).boardIndex, Hits->at(i).channel,
            Hits->at(i).chipId, Hits->at(i).region, Hits->at(i).dcol, Hits->at(i).address);
    linkInfo[Hits->at(i).boardIndex][Hits->at(i).channel]++;
  }
  fclose(fp);

  std::cout << std::endl << "Link statistics:" << std::endl;
  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    for (unsigned int ich = 0; ich < MAX_MOSAICTRANRECV; ich++) {
      std::cout << "  Board " << iboard << ", channel " << ich
                << ", hits: " << linkInfo[iboard][ich] << std::endl;
    }
  }

  std::cout << std::endl << "Board configuration:" << std::endl;
  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    if (TReadoutBoardMOSAIC *mosaic = dynamic_cast<TReadoutBoardMOSAIC *>(m_boards.at(0))) {
      std::cout << mosaic->GetRegisterDump() << std::endl;
    }
  }


  std::cout << std::endl;
}


TMaskScan::TMaskScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                     std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue,
                     std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  m_pixPerStage = m_config->GetParamValue("PIXPERREGION");
  m_stuck.clear();
  m_errorCount = {};
  FILE *fp     = fopen("DebugData.dat", "w");
  fclose(fp);
}

// check which HIC caused the timeout, i.e. did not send enough events
// called only in case a timeout occurs
void TMaskScan::FindTimeoutHics(int iboard, int *triggerCounts)
{
  for (unsigned int iHic = 0; iHic < m_hics.size(); iHic++) {
    if (!m_hics.at(iHic)->IsEnabled()) continue;
    bool isOnBoard = false;
    int  nTrigs    = 0;
    for (unsigned int iRcv = 0; iRcv < MAX_MOSAICTRANRECV; iRcv++) {
      if (m_hics.at(iHic)->ContainsReceiver(iboard, iRcv)) {
        isOnBoard = true;
        nTrigs += triggerCounts[iRcv];
      }
    }

    // HIC is connected to this readout board AND did not send enough events
    if ((isOnBoard) && (nTrigs < m_nTriggers * (int)(m_hics.at(iHic)->GetNEnabledChips(iboard)))) {
      std::cout << "identified timeout on HIC " << m_hics.at(iHic)->GetDbId() << std::endl;
      m_errorCounts.at(m_hics.at(iHic)->GetDbId()).nTimeout++;
    }
  }
}

void TMaskScan::ConfigureMaskStage(TAlpide *chip, int istage)
{
  m_row = AlpideConfig::ConfigureMaskStage(chip, m_pixPerStage, istage);
}

void TMaskScan::ReadEventData(std::vector<TPixHit> *Hits, int iboard)
{
  unsigned char buffer[MAX_EVENT_SIZE];
  int           n_bytes_data, n_bytes_header, n_bytes_trailer;
  int           itrg = 0, trials = 0;
  int           nBad = 0;
  TBoardHeader  boardInfo;
  int           nTrigPerHic[MAX_MOSAICTRANRECV];

  for (unsigned int i = 0; i < MAX_MOSAICTRANRECV; i++) {
    nTrigPerHic[i] = 0;
  }

  while (itrg < m_nTriggers * m_enabled[iboard]) {
    if (m_boards.at(iboard)->ReadEventData(n_bytes_data, buffer) <=
        0) { // no event available in buffer yet, wait a bit
      usleep(1000);
      trials++;
      if (trials == 3) {
        std::cout << "Board " << iboard << ": reached 3 timeouts, giving up on this event"
                  << std::endl;
        itrg = m_nTriggers * m_enabled[iboard];
        FindTimeoutHics(iboard, nTrigPerHic);
        m_errorCount.nTimeout++;
        if (m_errorCount.nTimeout > m_config->GetParamValue("MAXTIMEOUT")) {
          throw std::runtime_error("Maximum number of timouts reached. Aborting scan.");
        }
        trials = 0;
      }
      continue;
    }
    else {
      BoardDecoder::DecodeEvent(m_boards.at(iboard)->GetConfig()->GetBoardType(), buffer,
                                n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
      // decode Chip event
      if (boardInfo.decoder10b8bError) {
        m_errorCount.n8b10b++;
        if (FindHIC(iboard, boardInfo.channel).compare("None") != 0) {
          std::cout << "WARNING: 8b10b error in board " << iboard << " channel "
                    << boardInfo.channel << std::endl;
          m_errorCounts.at(FindHIC(iboard, boardInfo.channel)).n8b10b++;
        }
      }
      if (boardInfo.eventOverSizeError) {
        std::cout << "Found oversized event, truncated in MOSAIC" << std::endl;
        m_errorCount.nOversizeEvent++;
        FILE *fTruncated = fopen("TruncatedEvent.dat", "w");
        for (int iByte = 0; iByte < n_bytes_data + 1; ++iByte) {
          fprintf(fTruncated, "%02x ", (int)buffer[iByte]);
        }
        fclose(fTruncated);
      }
      int n_bytes_chipevent = n_bytes_data - n_bytes_header; //-n_bytes_trailer;
      if (boardInfo.eoeCount < 2) n_bytes_chipevent -= n_bytes_trailer;
      unsigned int bunchCounter  = -1U;
      int          chipId        = -1U;
      bool         dataIntegrity = false;
      try {
        int flags;
        dataIntegrity = AlpideDecoder::DecodeEvent(
            buffer + n_bytes_header, n_bytes_chipevent, Hits, iboard, boardInfo.channel,
            m_errorCounts.at(FindHIC(iboard, boardInfo.channel)).nPrioEncoder, flags,
            m_config->GetParamValue("MAXHITS"), &m_stuck, &chipId, &bunchCounter);
      }
      catch (const std::runtime_error &e) {
        std::cout << "Exception " << e.what() << " after " << itrg << " Triggers (this point)"
                  << std::endl;
        DumpHitInformation(Hits);
        throw e;
      }

      if (!dataIntegrity) {
        std::cout << "Found bad event, length = " << n_bytes_chipevent << std::endl;
        m_errorCount.nCorruptEvent++;
        if (FindHIC(iboard, boardInfo.channel).compare("None") != 0) {
          m_errorCounts.at(FindHIC(iboard, boardInfo.channel)).nCorruptEvent++;
        }
        if (nBad > 10) continue;
        FILE *fDebug = fopen("DebugData.dat", "a");
        fprintf(fDebug, "Bad event:\n");
        for (int iByte = 0; iByte < n_bytes_data + 1; ++iByte) {
          fprintf(fDebug, "%02x ", (int)buffer[iByte]);
        }
        fprintf(fDebug, "\nFull Event:\n");
        for (unsigned int ibyte = 0; ibyte < fDebugBuffer.size(); ibyte++) {
          fprintf(fDebug, "%02x ", (int)fDebugBuffer.at(ibyte));
        }
        fprintf(fDebug, "\n\n");
        fclose(fDebug);
      }
      if (((chipId & 0xf) == m_firstEnabledChipId) &&
          (boardInfo.channel == m_firstEnabledChannel) && (iboard == m_firstEnabledBoard)) {
        m_eventIds.push_back(boardInfo.eventId);
        m_timestamps.push_back(boardInfo.timestamp);
        m_bunchCounters.push_back(bunchCounter);
      }
      if (((chipId & 0xf) == m_firstEnabledChipId_ref) &&
          (boardInfo.channel == m_firstEnabledChannel_ref) && (iboard == m_firstEnabledBoard_ref)) {
        m_eventIds_ref.push_back(boardInfo.eventId);
        m_timestamps_ref.push_back(boardInfo.timestamp);
        m_bunchCounters_ref.push_back(bunchCounter);
      }
      nTrigPerHic[boardInfo.channel]++;
      itrg++;
    }
  }
}

void TScan::CreateHicConditions()
{
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    m_conditions.AddHicConditions(m_hics.at(i)->GetDbId(), new TScanConditionsHic());
  }
}

void TScan::WriteConditions(const char *fName, THic *aHic)
{
  FILE *fp = fopen(fName, "a");

  fprintf(fp, "Firmware version: %s\n", m_conditions.m_fwVersion);
  fprintf(fp, "Software version: %s\n\n", m_conditions.m_swVersion);

  fprintf(fp, "On-Stave PT100 #0 (start): %0.2f degrees Celsius\n",
          m_conditions.m_tempPT100start[0]);
  fprintf(fp, "On-Stave PT100 #1 (start): %0.2f degrees Celsius\n",
          m_conditions.m_tempPT100start[1]);
  fprintf(fp, "On-Stave PT100 #0 (end):   %0.2f degrees Celsius\n", m_conditions.m_tempPT100end[0]);
  fprintf(fp, "On-Stave PT100 #1 (end):   %0.2f degrees Celsius\n", m_conditions.m_tempPT100end[1]);
  fputs("# -273.15 degrees Celsius => sensor not connected\n", fp);


  fprintf(fp, "VDDD (start): %.3f V\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vdddStart);
  fprintf(fp, "VDDD (end):   %.3f V\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vdddEnd);
  fprintf(fp, "VDDA (start): %.3f V\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vddaStart);
  fprintf(fp, "VDDA (end):   %.3f V\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vddaEnd);

  fprintf(fp, "VDDD set (start): %.3f V\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vdddSetStart);
  fprintf(fp, "VDDD set (end):   %.3f V\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vdddSetEnd);
  fprintf(fp, "VDDA set (start): %.3f V\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vddaSetStart);
  fprintf(fp, "VDDA set (end):   %.3f V\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vddaSetEnd);

  fprintf(fp, "IDDD (start): %.3f A\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_idddStart);
  fprintf(fp, "IDDD (end):   %.3f A\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_idddEnd);
  fprintf(fp, "IDDA (start): %.3f A\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_iddaStart);
  fprintf(fp, "IDDA (end):   %.3f A\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_iddaEnd);

  fprintf(fp, "Analogue Supply Voltage (on-chip, start): %.3f V\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vddaChipStart);
  fprintf(fp, "Analogue Supply Voltage (on-chip, end):   %.3f V\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vddaChipEnd);
  fprintf(fp, "Digital Supply Voltage (on-chip, saturating at 1.72V, start): %.3f V\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vdddChipStart);
  fprintf(fp, "Digital Supply Voltage (on-chip, saturating at 1.72V, end):   %.3fV \n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vdddChipEnd);
  fprintf(fp, "Temp (on-chip, start): %.1f degrees Celsius\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_tempStart);
  fprintf(fp, "Temp (on-chip, end):   %.1f degrees Celsius\n",
          m_conditions.m_hicConditions.at(aHic->GetDbId())->m_tempEnd);

  fprintf(fp, "\nSingle chip values:\n\n");

  std::map<int, float>::iterator it;

  for (it = m_conditions.m_hicConditions.at(aHic->GetDbId())->m_chipAnalogueVoltagesStart.begin();
       it != m_conditions.m_hicConditions.at(aHic->GetDbId())->m_chipAnalogueVoltagesStart.end();
       it++) {
    fprintf(fp, "  Analogue voltage (start) on chip %d: %.3f V\n", it->first, it->second);
  }

  fputs("\n", fp);

  for (it = m_conditions.m_hicConditions.at(aHic->GetDbId())->m_chipDigitalVoltagesStart.begin();
       it != m_conditions.m_hicConditions.at(aHic->GetDbId())->m_chipDigitalVoltagesStart.end();
       it++) {
    fprintf(fp, "  Digital voltage (start) on chip %d: %.3f V\n", it->first, it->second);
  }

  fputs("\n", fp);

  for (it = m_conditions.m_hicConditions.at(aHic->GetDbId())->m_chipAnalogueVoltagesEnd.begin();
       it != m_conditions.m_hicConditions.at(aHic->GetDbId())->m_chipAnalogueVoltagesEnd.end();
       it++) {
    fprintf(fp, "  Analogue voltage (end) on chip %d: %.3f V\n", it->first, it->second);
  }

  fputs("\n", fp);

  for (it = m_conditions.m_hicConditions.at(aHic->GetDbId())->m_chipDigitalVoltagesEnd.begin();
       it != m_conditions.m_hicConditions.at(aHic->GetDbId())->m_chipDigitalVoltagesEnd.end();
       it++) {
    fprintf(fp, "  Digital voltage (end) on chip %d: %.3f V\n", it->first, it->second);
  }

  fputs("\n", fp);

  for (it = m_conditions.m_hicConditions.at(aHic->GetDbId())->m_chipTempsStart.begin();
       it != m_conditions.m_hicConditions.at(aHic->GetDbId())->m_chipTempsStart.end(); it++) {
    fprintf(fp, "  Temperature (start) on chip %d: %.3f degrees Celsius\n", it->first, it->second);
  }

  fputs("\n", fp);

  for (it = m_conditions.m_hicConditions.at(aHic->GetDbId())->m_chipTempsEnd.begin();
       it != m_conditions.m_hicConditions.at(aHic->GetDbId())->m_chipTempsEnd.end(); it++) {
    fprintf(fp, "  Temperature (end) on chip %d: %.3f degrees Celsius\n", it->first, it->second);
  }

  fputs("\n", fp);

  fclose(fp);
}

void TScan::WriteChipRegisters(const char *fName)
{
  FILE *fp = fopen(fName, "a");

  fputs("\n", fp);

  fputs("== Chip registers (start)\n", fp);
  for (const auto &str : m_conditions.m_chipConfigStart)
    fprintf(fp, "%s", str.c_str());
  fputs("== Chip registers (end)\n", fp);
  for (const auto &str : m_conditions.m_chipConfigEnd)
    fprintf(fp, "%s", str.c_str());
  fputs("==\n", fp);

  fclose(fp);
}

void TScan::WriteBoardRegisters(const char *fName)
{
  FILE *fp = fopen(fName, "a");

  fputs("\n", fp);

  fputs("== Board registers (start)\n", fp);
  for (const auto &str : m_conditions.m_boardConfigStart)
    fprintf(fp, "%s", str.c_str());
  fputs("== Board registers (end)\n", fp);
  for (const auto &str : m_conditions.m_boardConfigEnd)
    fprintf(fp, "%s", str.c_str());
  fputs("==\n", fp);

  fclose(fp);
}

TPowerBoardConfig::pb_t TScan::GetPBtype(THic *hic) const
{
  TPowerBoardConfig::pb_t pb = TPowerBoardConfig::none;

  const int nchips = m_chips.size();
  if (nchips > 14) {
    THicOB *obhic = dynamic_cast<THicOB *>(hic);
    // printf("power combo: %s\n", obhic->IsPowerCombo() ? "true" : "false");
    if (!obhic->IsPowerCombo())
      pb = TPowerBoardConfig::mockup;
    else if (nchips == 56)
      pb = TPowerBoardConfig::realML;
    else
      pb = TPowerBoardConfig::realOL;
  }

  // printf("pb = %d (%d chips)\n", pb, nchips);
  return pb;
}

void TScan::CorrectVoltageDrop(bool reset)
{
  const auto devType = m_config->GetDeviceType();

  if ((devType == TYPE_HALFSTAVE) || (devType == TYPE_HALFSTAVERU) ||
      (devType == TYPE_MLHALFSTAVE) || (devType == TYPE_MLSTAVE)) {
    if (TPowerBoard *pb = !m_hics.empty() ? m_hics.front()->GetPowerBoard() : nullptr) {
      pb->CorrectVoltageDrop(GetPBtype(m_hics.front()), reset, m_hics.size());
      pb->CorrectVoltageDrop(GetPBtype(m_hics.front()), reset, m_hics.size());
    }
  }
  else {
    for (auto hic : m_hics) {
      if (!hic->IsEnabled() || !hic->GetPowerBoard()) continue;
      hic->GetPowerBoard()->CorrectVoltageDrop(hic->GetPbMod(), GetPBtype(hic), reset);
    }
  }
}

int TScanConditions::AddHicConditions(std::string hicId, TScanConditionsHic *hicCond)
{
  m_hicConditions.insert(std::pair<std::string, TScanConditionsHic *>(hicId, hicCond));

  return m_hicConditions.size();
}
