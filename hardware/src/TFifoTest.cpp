#include "TFifoTest.h"
#include "AlpideConfig.h"
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string.h>
#include <string>

TFifoTest::TFifoTest(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                     std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue,
                     std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  CreateScanParameters();

  m_parameters->backBias = 0;

  float voltageScale  = config->GetVoltageScale();
  int   mlvdsStrength = config->GetMlvdsStrength();

  ((TFifoParameters *)m_parameters)->voltageScale  = voltageScale;
  ((TFifoParameters *)m_parameters)->mlvdsStrength = mlvdsStrength;

  SetName();

  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = m_chips.size();

  m_start[1] = 0;
  m_step[1]  = 1;
  m_stop[1]  = 32;

  m_start[0] = 0;
  m_step[0]  = 1;
  m_stop[0]  = 128;
}


void TFifoTest::SetName()
{
  float voltageScale  = ((TFifoParameters *)m_parameters)->voltageScale;
  int   mlvdsStrength = ((TFifoParameters *)m_parameters)->mlvdsStrength;
  if (voltageScale > 0.95 && voltageScale < 1.05) {
    if (mlvdsStrength == ChipConfig::DCTRL_DRIVER) {
      strcpy(m_name, "Fifo Scan");
    }
    else {
      sprintf(m_name, "FIFO Scan, Driver %d", mlvdsStrength);
    }
  }
  else if (voltageScale > 1.0 && voltageScale < 1.2) {
    strcpy(m_name, "Fifo Scan, V +10%");
  }
  else if (voltageScale > 0.8 && voltageScale < 1.0) {
    strcpy(m_name, "Fifo Scan, V -10%");
  }
  else {
    std::cout << "Warning: unforeseen voltage scale, using 1" << std::endl;
    voltageScale = 1.0;
    strcpy(m_name, "Fifo Scan");
  }
}


bool TFifoTest::SetParameters(TScanParameters *pars)
{
  TFifoParameters *fPars = dynamic_cast<TFifoParameters *>(pars);
  if (fPars) {
    std::cout << "TFifoScan: Updating parameters" << std::endl;
    ((TFifoParameters *)m_parameters)->voltageScale  = fPars->voltageScale;
    ((TFifoParameters *)m_parameters)->mlvdsStrength = fPars->mlvdsStrength;
    SetName();
    return true;
  }
  else {
    std::cout << "TFifoTest::SetParameters: Error, bad parameter type, doing nothing" << std::endl;
    return false;
  }
}


THisto TFifoTest::CreateHisto()
{
  // count errors in bins corresponding to pattern,
  // e.g. error in pattern 0xaaaa in region 16 -> Incr(16, 10)
  // exception counter: bin 32, 0
  THisto histo("ErrorHisto", "ErrorHisto", 33, 0, 32, 16, 0, 15);
  return histo;
}

void TFifoTest::Init()
{
  CreateScanHisto();

  InitBase(false);
  float voltageScale  = ((TFifoParameters *)m_parameters)->voltageScale;
  int   mlvdsStrength = ((TFifoParameters *)m_parameters)->mlvdsStrength;
  // scale voltage, send GRST, correct drop, configure chips, correct drop
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (voltageScale != 1.) {
      m_hics.at(ihic)->ScaleVoltage(voltageScale);
    }
  }

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
  }

  for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
    if (mlvdsStrength != ChipConfig::DCTRL_DRIVER) {
      m_chips.at(ichip)->GetConfig()->SetParamValue("DCTRLDRIVER", mlvdsStrength);
      AlpideConfig::ConfigureBuffers(m_chips.at(ichip), m_chips.at(ichip)->GetConfig());
    }
  }

//   CorrectVoltageDrop();

  for (unsigned int i = 0; i < m_chips.size(); i++) {
    if (!m_chips.at(i)->GetConfig()->IsEnabled()) continue;
    AlpideConfig::ConfigureCMU(m_chips.at(i));
    std::cout << "TFifoTest::Init() : " << i << std::endl;
  }

//   CorrectVoltageDrop();
//   for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
//     if (m_hics.at(ihic)->IsEnabled() && !m_hics.at(ihic)->IsPowered()) {
//       throw std::runtime_error("FIFO scan init: HIC powered off (Retry suggested)");
//     }
//     else if (m_hics.at(ihic)->IsEnabled() &&
//              ((m_hics.at(ihic)->GetVddd() < 0.1) || (m_hics.at(ihic)->GetVdda() < 0.1))) {
//       throw std::runtime_error("FIFO scan init: voltage appears to be off (Retry suggested)");
//     }
//   }

  TScan::SaveStartConditions();
}

int TFifoTest::GetChipById(std::vector<TAlpide *> chips, int id)
{
  for (unsigned int i = 0; i < chips.size(); i++) {
    if (chips.at(i)->GetConfig()->GetChipId() == id) return i;
  }

  return -1;
}

void TFifoTest::PrepareStep(int loopIndex)
{

  switch (loopIndex) {
  case 0: // innermost loop
    break;
  case 1: // 2nd loop
    break;
  case 2: // outermost loop: change chip
    m_testChip   = m_chips.at(m_value[2]);
    m_boardIndex = FindBoardIndex(m_testChip);
    sprintf(m_state, "Running %d", m_value[2]);
    break;
  default:
    break;
  }
}

void TFifoTest::WriteMem(TAlpide *chip, int ARegion, int AOffset, int AValue)
{
  if ((ARegion > 31) || (AOffset > 127)) {
    std::cout << "WriteMem: invalid parameters" << std::endl;
    return;
  }
  uint16_t LowAdd  = Alpide::REG_RRU_MEB_LSB_BASE | (ARegion << 11) | AOffset;
  uint16_t HighAdd = Alpide::REG_RRU_MEB_MSB_BASE | (ARegion << 11) | AOffset;

  uint16_t LowVal  = AValue & 0xffff;
  uint16_t HighVal = (AValue >> 16) & 0xff;

  int err = chip->WriteRegister(LowAdd, LowVal);
  if (err >= 0) err = chip->WriteRegister(HighAdd, HighVal);

  if (err < 0) {
    std::cout << "Cannot write chip register. Exiting ... " << std::endl;
    exit(1);
  }
}

void TFifoTest::ReadMem(TAlpide *chip, int ARegion, int AOffset, int &AValue, bool &exception)
{
  if ((ARegion > 31) || (AOffset > 127)) {
    std::cout << "ReadMem: invalid parameters" << std::endl;
    return;
  }
  uint16_t LowAdd  = Alpide::REG_RRU_MEB_LSB_BASE | (ARegion << 11) | AOffset;
  uint16_t HighAdd = Alpide::REG_RRU_MEB_MSB_BASE | (ARegion << 11) | AOffset;

  uint16_t LowVal, HighVal;
  int      err;

  try {
    err = chip->ReadRegister(LowAdd, LowVal);
  }
  catch (std::exception &e) {
    exception = true;
    std::cout << "Exception " << e.what() << " when reading low value for chip ID "
              << chip->GetConfig()->GetChipId() << std::endl;
    return;
  }
  exception = false;
  if (err >= 0) {
    try {
      err = chip->ReadRegister(HighAdd, HighVal);
    }
    catch (std::exception &e) {
      std::cout << "Exception " << e.what() << " when reading high value for chip ID "
                << chip->GetConfig()->GetChipId() << std::endl;
      exception = true;
      return;
    }
  }

  if (err < 0) {
    std::cout << "Cannot read chip register. Exiting ... " << std::endl;
    exit(1);
  }

  // Note to self: if you want to shorten the following lines,
  // remember that HighVal is 16 bit and (HighVal << 16) will yield 0
  // :-)
  AValue = (HighVal & 0xff);
  AValue <<= 16;
  AValue |= LowVal;
}

bool TFifoTest::TestPattern(int pattern, bool &exception)
{
  int readBack;
  WriteMem(m_testChip, m_value[1], m_value[0], pattern);
  ReadMem(m_testChip, m_value[1], m_value[0], readBack, exception);
  if (exception) return false;
  if (readBack != pattern) return false;
  return true;
}

void TFifoTest::LoopEnd(int loopIndex)
{
  if (loopIndex == 2) {
    while (!(m_mutex->try_lock()))
      ;
    m_histoQue->push_back(*m_histo);
    m_mutex->unlock();
    m_histo->Clear();
  }
}

void TFifoTest::Execute()
{
  common::TChipIndex idx;
  bool               exception, result;
  idx.boardIndex   = m_boardIndex;
  idx.chipId       = m_testChip->GetConfig()->GetChipId();
  idx.dataReceiver = m_testChip->GetConfig()->GetParamValue("RECEIVER");

  // Test readback of four different patterns
  // if exception in readback increment bin (32, 0)
  // otherwise in case of false readback increment corresponding bin
  if (m_testChip->GetConfig()->IsEnabled()) {
    result = TestPattern(0xffff, exception);
    if (exception)
      m_histo->Incr(idx, 32, 0);
    else if (!result)
      m_histo->Incr(idx, m_value[1], 0xf);
    result = TestPattern(0x0, exception);
    if (exception)
      m_histo->Incr(idx, 32, 0);
    else if (!result)
      m_histo->Incr(idx, m_value[1], 0x0);
    result = TestPattern(0xaaaa, exception);
    if (exception)
      m_histo->Incr(idx, 32, 0);
    else if (!result)
      m_histo->Incr(idx, m_value[1], 0xa);
    result = TestPattern(0x5555, exception);
    if (exception)
      m_histo->Incr(idx, 32, 0);
    else if (!result)
      m_histo->Incr(idx, m_value[1], 0x5);
  }
}

void TFifoTest::Terminate()
{
  TScan::Terminate();
  float voltageScale  = ((TFifoParameters *)m_parameters)->voltageScale;
  int   mlvdsStrength = ((TFifoParameters *)m_parameters)->mlvdsStrength;


  // restore old voltage
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (m_hics.at(ihic)->IsEnabled() && !m_hics.at(ihic)->IsPowered()) {
      throw std::runtime_error("FIFO scan terminate: HIC powered off (Retry suggested)");
    }
    else if (m_hics.at(ihic)->IsEnabled() &&
             ((m_hics.at(ihic)->GetVddd() < 0.1) || (m_hics.at(ihic)->GetVdda() < 0.1))) {
      throw std::runtime_error("FIFO scan terminate: voltage appears to be off (Retry suggested)");
    }

    if (voltageScale != 1.) {
      m_hics.at(ihic)->ScaleVoltage(1.);
    }
  }
  // restore old driver setting
  for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
    if (mlvdsStrength != ChipConfig::DCTRL_DRIVER) {
      m_chips.at(ichip)->GetConfig()->SetParamValue("DCTRLDRIVER", ChipConfig::DCTRL_DRIVER);
      AlpideConfig::ConfigureBuffers(m_chips.at(ichip), m_chips.at(ichip)->GetConfig());
    }
  }
  m_running = false;
}
