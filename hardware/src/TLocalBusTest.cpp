#include "TLocalBusTest.h"
#include "AlpideConfig.h"
#include "AlpideDebug.h"
#include <iostream>
#include <string>

TLocalBusTest::TLocalBusTest(TScanConfig *config, std::vector<TAlpide *> chips,
                             std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                             std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  CreateScanParameters();

  m_parameters->backBias = 0;

  strcpy(m_name, "Local Bus Test");
  FindDaisyChains(chips);
  m_start[2] = 0;
  m_stop[2]  = m_daisyChains.size();
  m_step[2]  = 1;

  // stop values here have to be set on the fly as they depend on the number of enabled chips
  m_start[1] = 0;
  m_step[1]  = 1;

  m_start[0] = 0;
  m_step[0]  = 1;
}

THisto TLocalBusTest::CreateHisto()
{
  // count errors in bins corresponding to pattern,
  // e.g. error in pattern 0xa in communication with chip 3 -> Incr(3, 10)
  // for the time being: count errors both for read and write chip in the
  // same way
  // 17th and 18th bin are used for busy checks
  // (busy false error-> Incr(3,16), busy true error->Incr(3,17))
  THisto histo("ErrorHisto", "ErrorHisto", 15, 0, 14, 18, 0, 17);
  return histo;
}

void TLocalBusTest::Init()
{
  CreateScanHisto();

  InitBase(false);
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
  }
  for (unsigned int i = 0; i < m_chips.size(); i++) {
    if (!m_chips.at(i)->GetConfig()->IsEnabled()) continue;
    m_chips.at(i)->GetConfig()->SetInitialToken(false);
    m_chips.at(i)->GetConfig()->SetEnableDdr(false);
    AlpideConfig::ConfigureCMU(m_chips.at(i));
  }

  CorrectVoltageDrop();

  TScan::SaveStartConditions();
}

int TLocalBusTest::GetChipById(std::vector<TAlpide *> chips, int id)
{
  for (unsigned int i = 0; i < chips.size(); i++) {
    if (chips.at(i)->GetConfig()->GetChipId() == id) return i;
  }

  return -1;
}

void TLocalBusTest::FindDaisyChains(std::vector<TAlpide *> chips)
{
  int                    totalChips = 0;
  int                    iChip      = 0;
  int                    maxChip    = -1;
  std::vector<TAlpide *> daisyChain;

  int totEnabled = 0;
  for (auto chip : m_chips)
    if (chip->GetConfig()->IsEnabled()) totEnabled++;
  // cout << "Enabled chips: " << totEnabled << endl;

  while ((totalChips < totEnabled) && (maxChip < (int)chips.size() - 1)) {
    // find next enabled chip and put into new daisy chain vector
    for (iChip = maxChip + 1;
         (iChip < (int)chips.size()) && (!(chips.at(iChip)->GetConfig()->IsEnabled())); iChip++) {
      totalChips++;
    }

    daisyChain.push_back(chips.at(iChip));
    totalChips++;

    // go back through daisy chain until next chip would be iChip
    int chipId = chips.at(iChip)->GetConfig()->GetChipId();
    int iiChip = iChip;

    while (chips.at(iiChip)->GetConfig()->GetPreviousId() != chipId) {
      int previousId = chips.at(iiChip)->GetConfig()->GetPreviousId();

      iiChip = GetChipById(chips, previousId);
      if (iiChip < 0) {
        std::cout << "Something went wrong, Did not find chip Id" << previousId << std::endl;
        exit(1);
      }
      if (iiChip > maxChip) maxChip = iiChip;
      totalChips++;
      daisyChain.push_back(chips.at(iiChip));
    }

    m_daisyChains.push_back(daisyChain);
    daisyChain.clear();
  }

  for (unsigned int i = 0; i < m_daisyChains.size(); i++) {
    std::cout << "Daisy chain no " << i << ": " << std::endl;
    for (unsigned int ii = 0; ii < m_daisyChains.at(i).size(); ii++) {
      std::cout << "  chip id " << m_daisyChains.at(i).at(ii)->GetConfig()->GetChipId()
                << std::endl;
    }
  }
}

void TLocalBusTest::PrepareStep(int loopIndex)
{

  switch (loopIndex) {
  case 0: // innermost loop: change read chip
    m_readChip = m_daisyChains.at(m_value[2]).at(m_value[0]);
    break;
  case 1: // 2nd loop: change write chip
    m_writeChip  = m_daisyChains.at(m_value[2]).at(m_value[1]);
    m_boardIndex = FindBoardIndex(m_writeChip);
    // give token to write chip, take away in loop end

    m_writeChip->ModifyRegisterBits(Alpide::REG_CMUDMU_CONFIG, 4, 1, 1);
    for (const auto &rBoard : m_boards)
      rBoard->SendOpCode(Alpide::OPCODE_RORST);
    break;
  case 2: // outermost loop: change daisy chain
    m_stop[0] = m_daisyChains.at(m_value[2]).size();
    m_stop[1] = m_daisyChains.at(m_value[2]).size();
    break;
  default:
    break;
  }
}

void TLocalBusTest::LoopEnd(int loopIndex)
{
  if (loopIndex == 2) {
    while (!(m_mutex->try_lock()))
      ;
    m_histoQue->push_back(*m_histo);
    m_mutex->unlock();
    m_histo->Clear();
  }
}

void TLocalBusTest::Next(int loopIndex)
{
  if (loopIndex == 1) {
    m_writeChip->ModifyRegisterBits(Alpide::REG_CMUDMU_CONFIG, 4, 1, 0);
  }
  TScan::Next(loopIndex);
}

bool TLocalBusTest::TestPattern(int pattern)
{
  TDMUDebugStream debugStream;
  int             readId  = m_readChip->GetConfig()->GetChipId();
  int             writeId = m_writeChip->GetConfig()->GetChipId();
  uint16_t        Value   = 1 | ((pattern & 0xf) << 1);

  common::TChipIndex read_idx, write_idx;

  read_idx.boardIndex   = m_boardIndex;
  read_idx.chipId       = readId;
  read_idx.dataReceiver = m_readChip->GetConfig()->GetParamValue("RECEIVER");

  write_idx.boardIndex   = m_boardIndex;
  write_idx.chipId       = writeId;
  write_idx.dataReceiver = m_writeChip->GetConfig()->GetParamValue("RECEIVER");

  m_writeChip->WriteRegister(REG_TEST_CONTROL, Value);
  m_writeChip->GetReadoutBoard()->SendOpCode(Alpide::OPCODE_DEBUG);

  if (!AlpideDebug::GetDMUDebugStream(m_readChip, debugStream)) {
    std::cout << "Problem reading debug stream" << std::endl;
  }

  if ((debugStream.LocalBusValue & 0xf) != (pattern & 0xf)) {
    std::cout << "ERROR (BUS VALUE): written 0x" << std::hex << pattern << std::dec << " to chip "
              << writeId << ", read " << std::hex << debugStream.LocalBusValue << std::dec
              << " from chip " << readId << std::endl;
    m_histo->Incr(read_idx, writeId & 0xf, pattern);
    m_histo->Incr(write_idx, readId & 0xf, pattern);
    return false;
  }
  return true;
}

bool TLocalBusTest::TestBusy(bool busy)
{
  TBMUDebugStream bmuDebugStream;
  int             readId  = m_readChip->GetConfig()->GetChipId();
  int             writeId = m_writeChip->GetConfig()->GetChipId();

  common::TChipIndex read_idx, write_idx;

  read_idx.boardIndex   = m_boardIndex;
  read_idx.chipId       = readId;
  read_idx.dataReceiver = m_readChip->GetConfig()->GetParamValue("RECEIVER");

  write_idx.boardIndex   = m_boardIndex;
  write_idx.chipId       = writeId;
  write_idx.dataReceiver = m_writeChip->GetConfig()->GetParamValue("RECEIVER");

  m_writeChip->WriteRegister(REG_TEST_CONTROL, busy ? 0x200 : 0x000);
  m_writeChip->GetReadoutBoard()->SendOpCode(Alpide::OPCODE_DEBUG);

  if (!AlpideDebug::GetBMUDebugStream(m_readChip, bmuDebugStream)) {
    std::cout << "Problem reading debug stream" << std::endl;
  }
  int busyRead = (int)bmuDebugStream.BusyInState;

  if (((bool)busyRead) == busy) { // busy logic is active low!
    std::cout << "ERROR (BUSY): written " << std::hex << (int)busy << std::dec << " to chip "
              << writeId << ", read " << std::hex << (int)(!busyRead) << std::dec << " from chip "
              << readId << std::endl;
    m_histo->Incr(read_idx, writeId & 0xf, 16 + (busy ? 1 : 0));
    m_histo->Incr(write_idx, readId & 0xf, 16 + (busy ? 1 : 0));
    return false;
  }
  return true;
}

void TLocalBusTest::Execute()
{
  TestPattern(0xf);
  TestPattern(0x0);
  TestPattern(0xa);
  TestPattern(0x5);

  TestBusy(true);
  TestBusy(false);
}

void TLocalBusTest::Terminate()
{
  TScan::Terminate();

  for (unsigned int i = 0; i < m_hics.size(); i++) {
    m_hics.at(i)->PowerOff();
  }
  sleep(1);
  m_running = false;
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    m_hics.at(i)->PowerOff();
  }
  sleep(1);
}
