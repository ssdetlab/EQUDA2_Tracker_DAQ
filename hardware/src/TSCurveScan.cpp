#include "TSCurveScan.h"
#include "AlpideConfig.h"
#include "SetupHelpers.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include <chrono>
#include <string.h>
#include <string>
#include <thread>
#include <unistd.h>

TSCurveScan::TSCurveScan(TScanConfig *config, std::vector<TAlpide *> chips,
                         std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                         std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TMaskScan(config, chips, hics, boards, histoQue, aMutex), m_hitsets(nullptr)
{
  CreateScanParameters();

  m_parameters->backBias                       = m_config->GetBackBias();
  ((TSCurveParameters *)m_parameters)->nominal = (m_config->GetParamValue("NOMINAL") == 1);
}


bool TSCurveScan::SetParameters(TScanParameters *pars)
{
  TSCurveParameters *sPars = dynamic_cast<TSCurveParameters *>(pars);
  if (sPars) {
    std::cout << "TSCurveScan: Updating parameters" << std::endl;
    ((TSCurveParameters *)m_parameters)->nominal  = sPars->nominal;
    ((TSCurveParameters *)m_parameters)->VPULSEH  = sPars->VPULSEH;
    ((TSCurveParameters *)m_parameters)->VPULSEL  = sPars->VPULSEL;
    ((TSCurveParameters *)m_parameters)->TARGET   = sPars->TARGET;
    ((TSCurveParameters *)m_parameters)->backBias = sPars->backBias;
    SetName();
    return true;
  }
  else {
    std::cout << "TSCurveScan::SetParameters: Error, bad parameter type, doing nothing"
              << std::endl;
    return false;
  }
}


TThresholdScan::TThresholdScan(TScanConfig *config, std::vector<TAlpide *> chips,
                               std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                               std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TSCurveScan(config, chips, hics, boards, histoQue, aMutex)
{
  m_start[0] = m_config->GetChargeStart();
  m_stop[0]  = m_config->GetChargeStop();
  m_step[0]  = m_config->GetChargeStep();

  m_start[1] = 0;
  m_step[1]  = 1;
  m_stop[1]  = m_config->GetNMaskStages();

  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = 1;

  ((TSCurveParameters *)m_parameters)->VPULSEH = 170;
  m_nTriggers                                  = m_config->GetParamValue("NINJ");

  SetName();
}


void TThresholdScan::SetName()
{
  sprintf(m_name, "Threshold Scan %.1f V", ((TSCurveParameters *)m_parameters)->backBias);
}


TtuneVCASNScan::TtuneVCASNScan(TScanConfig *config, std::vector<TAlpide *> chips,
                               std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                               std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TSCurveScan(config, chips, hics, boards, histoQue, aMutex)
{
  m_start[0] = m_config->GetVcasnStart();
  m_stop[0]  = m_config->GetVcasnStop();
  m_step[0]  = m_config->GetVcasnStep();

  m_start[1] = 0;
  m_step[1]  = m_config->GetScanStep();
  m_stop[1]  = m_config->GetParamValue("TUNINGMAXROW");

  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = 1;

  ((TSCurveParameters *)m_parameters)->VPULSEH = 170;
  ((TSCurveParameters *)m_parameters)->TARGET  = m_config->GetParamValue("TARGETTHRESH");
  if (((TSCurveParameters *)m_parameters)->TARGET % 10) {
    ((TSCurveParameters *)m_parameters)->TARGET -=
        (((TSCurveParameters *)m_parameters)->TARGET % 10);
    ((TSCurveParameters *)m_parameters)->TARGET += 10;
    std::cout << "Warning: threshold target not multiple of 10, rounding up to "
              << ((TSCurveParameters *)m_parameters)->TARGET;
  }
  ((TSCurveParameters *)m_parameters)->VPULSEL = ((TSCurveParameters *)m_parameters)->VPULSEH -
                                                 ((TSCurveParameters *)m_parameters)->TARGET / 10;
  m_nTriggers = m_config->GetParamValue("NINJ");

  SetName();
}


void TtuneVCASNScan::SetName()
{
  sprintf(m_name, "Tune VCASN Scan %.1f V", ((TSCurveParameters *)m_parameters)->backBias);
}


TtuneITHRScan::TtuneITHRScan(TScanConfig *config, std::vector<TAlpide *> chips,
                             std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                             std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TSCurveScan(config, chips, hics, boards, histoQue, aMutex)
{
  m_start[0] = m_config->GetIthrStart();
  m_stop[0]  = m_config->GetIthrStop();
  m_step[0]  = m_config->GetIthrStep();

  m_start[1] = 0;
  m_step[1]  = m_config->GetScanStep();
  m_stop[1]  = m_config->GetParamValue("TUNINGMAXROW");

  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = 1;

  ((TSCurveParameters *)m_parameters)->VPULSEH = 170;
  ((TSCurveParameters *)m_parameters)->TARGET  = m_config->GetParamValue("TARGETTHRESH");
  if (((TSCurveParameters *)m_parameters)->TARGET % 10) {
    ((TSCurveParameters *)m_parameters)->TARGET -=
        (((TSCurveParameters *)m_parameters)->TARGET % 10);
    ((TSCurveParameters *)m_parameters)->TARGET += 10;
    std::cout << "Warning: threshold target not multiple of 10, rounding up to "
              << ((TSCurveParameters *)m_parameters)->TARGET;
  }
  ((TSCurveParameters *)m_parameters)->VPULSEL = ((TSCurveParameters *)m_parameters)->VPULSEH -
                                                 ((TSCurveParameters *)m_parameters)->TARGET / 10;
  m_nTriggers = m_config->GetParamValue("NINJ");
  SetName();
}


void TtuneITHRScan::SetName()
{
  sprintf(m_name, "Tune ITHR Scan %.1f V", ((TSCurveParameters *)m_parameters)->backBias);
}


void TSCurveScan::RestoreNominalSettings()
{
  if (((TSCurveParameters *)m_parameters)->backBias == 0.0) {
    for (unsigned int i = 0; i < m_chips.size(); i++) {
      m_chips.at(i)->GetConfig()->SetParamValue("ITHR", 50);
      m_chips.at(i)->GetConfig()->SetParamValue("VCASN", 50);
      m_chips.at(i)->GetConfig()->SetParamValue("VCASN2", 57);
      m_chips.at(i)->GetConfig()->SetParamValue("VCLIP", 0);
    }
  }
  else if ((((TSCurveParameters *)m_parameters)->backBias > 0.99) &&
           (((TSCurveParameters *)m_parameters)->backBias < 1.01)) {
    for (unsigned int i = 0; i < m_chips.size(); i++) {
      m_chips.at(i)->GetConfig()->SetParamValue("ITHR", 50);
      m_chips.at(i)->GetConfig()->SetParamValue("VCASN", 70);
      m_chips.at(i)->GetConfig()->SetParamValue("VCASN2", 82);
      m_chips.at(i)->GetConfig()->SetParamValue("VCLIP", 20);
    }
  }
  else if ((((TSCurveParameters *)m_parameters)->backBias > 2.99) &&
           (((TSCurveParameters *)m_parameters)->backBias < 3.01)) {
    for (unsigned int i = 0; i < m_chips.size(); i++) {
      m_chips.at(i)->GetConfig()->SetParamValue("ITHR", 50);
      m_chips.at(i)->GetConfig()->SetParamValue("VCASN", 105);
      m_chips.at(i)->GetConfig()->SetParamValue("VCASN2", 117);
      m_chips.at(i)->GetConfig()->SetParamValue("VCLIP", 60);
    }
  }
}

void TSCurveScan::ConfigureBoard(TReadoutBoard *board)
{
  if (board->GetConfig()->GetBoardType() == boardDAQ) {
    // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns * strobe
    // delay
    // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
    board->SetTriggerConfig(true, false, 0,
                            2 * board->GetConfig()->GetParamValue("STROBEDELAYBOARD"));
    board->SetTriggerSource(trigExt);
  }
  else {
    board->SetTriggerConfig(true, true, board->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                            board->GetConfig()->GetParamValue("PULSEDELAY"));
    board->SetTriggerSource(trigInt);
  }
}

void TSCurveScan::ConfigureFromu(TAlpide *chip)
{
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1, 0x20); // analogue pulsing
  chip->WriteRegister(
      Alpide::REG_FROMU_CONFIG2,
      chip->GetConfig()->GetParamValue("STROBEDURATION")); // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1,
                      chip->GetConfig()->GetParamValue("STROBEDELAYCHIP")); // fromu pulsing 1:
                                                                            // delay pulse - strobe
                                                                            // (not used here, since
                                                                            // using external
                                                                            // strobe)
  chip->WriteRegister(
      Alpide::REG_FROMU_PULSING2,
      chip->GetConfig()->GetParamValue("PULSEDURATION")); // fromu pulsing 2: pulse length
}

void TThresholdScan::ConfigureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);

  ConfigureFromu(chip);

  AlpideConfig::ConfigureCMU(chip);
}

void TtuneVCASNScan::ConfigureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);

  ConfigureFromu(chip);

  AlpideConfig::ConfigureCMU(chip);

  chip->WriteRegister(Alpide::REG_VPULSEL, ((TSCurveParameters *)m_parameters)->VPULSEL);
}

void TtuneITHRScan::ConfigureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);

  ConfigureFromu(chip);

  AlpideConfig::ConfigureCMU(chip);

  chip->WriteRegister(Alpide::REG_VPULSEL, ((TSCurveParameters *)m_parameters)->VPULSEL);
}

THisto TSCurveScan::CreateHisto()
{
  THisto histo("ThresholdHisto", "ThresholdHisto", 1024, 0, 1023,
               1 + (m_stop[0] - m_start[0]) / m_step[0], m_start[0], m_stop[0]);
  std::cout << "CREATING: " << (m_stop[0] - m_start[0]) / m_step[0] << ", " << m_start[0] << ", "
            << m_stop[0] << std::endl;
  return histo;
}

void TSCurveScan::Init()
{
  m_hitsets = new TRingBuffer<THitSet>;

  InitBase(false);

  if (((TSCurveParameters *)m_parameters)->nominal) RestoreNominalSettings();

  m_running = true;

  SetBackBias();

  CreateScanHisto();

  CountEnabledChips();
  MakeDaisyChain(nullptr, &m_boards, nullptr, &m_chips);

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    std::cout << "Board " << i << ", found " << m_enabled[i] << " enabled chips" << std::endl;
    ConfigureBoard(m_boards.at(i));

    m_boards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_PRST);
  }

  for (unsigned int i = 0; i < m_chips.size(); i++) {
    if (!(m_chips.at(i)->GetConfig()->IsEnabled())) {
      TChipConfig *config = m_chips.at(i)->GetConfig();
      if ((config->GetPreviousId() != -1) || !config->GetEnableDdr()) {
        uint16_t cmuconfig = 0;
        cmuconfig |= (config->GetPreviousId() & 0xf);
        cmuconfig |= 0 << 4; // no initial token
        cmuconfig |= (config->GetDisableManchester() ? 1 : 0) << 5;
        cmuconfig |= (config->GetEnableDdr() ? 1 : 0) << 6;
        printf("setting non-default CMU parameters for disabled chip %i: 0x%02x\n", i, cmuconfig);
        m_chips.at(i)->WriteRegister(Alpide::REG_CMUDMU_CONFIG, cmuconfig);
      }
      continue;
    }
    ConfigureChip(m_chips.at(i));
  }

  CorrectVoltageDrop();

  for (const auto &rBoard : m_boards) {
    rBoard->SendOpCode(Alpide::OPCODE_RORST);
    rBoard->StartRun();
  }

  for (const auto &rBoard : m_boards) {
    if (TReadoutBoardMOSAIC *rMOSAIC = dynamic_cast<TReadoutBoardMOSAIC *>(rBoard)) {
      rMOSAIC->ResetAllReceivers();
    }
  }

  TScan::SaveStartConditions();

  m_thread = new std::thread(&TSCurveScan::Histo, this);
}

void TThresholdScan::PrepareStep(int loopIndex)
{
  switch (loopIndex) {
  case 0: // innermost loop: change VPULSEL
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      m_chips.at(ichip)->WriteRegister(Alpide::REG_VPULSEL,
                                       ((TSCurveParameters *)m_parameters)->VPULSEH - m_value[0]);
    }
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      uint16_t reg = 0x0;
      m_chips.at(ichip)->ReadRegister(Alpide::REG_VPULSEL, reg);
      ++reg; // trick the compiler
      break;
    }
    break;
  case 1: // 2nd loop: mask staging
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      ConfigureMaskStage(m_chips.at(ichip), m_value[1]);
    }
    sprintf(m_state, "Running %d", m_value[1]);
    break;
  default:
    break;
  }
}

// Need different registers for different classes...
void TtuneVCASNScan::PrepareStep(int loopIndex)
{
  switch (loopIndex) {
  case 0: // innermost loop: change VCASN
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      m_chips.at(ichip)->WriteRegister(Alpide::REG_VCASN, m_value[0]);
      m_chips.at(ichip)->WriteRegister(Alpide::REG_VCASN2, m_value[0] + 12);
    }
    break;
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      uint16_t reg = 0x0;
      m_chips.at(ichip)->ReadRegister(Alpide::REG_VCASN2, reg);
      ++reg; // trick the compiler
      break;
    }
  case 1: // 2nd loop: mask staging
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      ConfigureMaskStage(m_chips.at(ichip), m_value[1]);
    }
    break;
  default:
    break;
  }
}

void TtuneITHRScan::PrepareStep(int loopIndex)
{
  switch (loopIndex) {
  case 0: // innermost loop: change ITHR
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      m_chips.at(ichip)->WriteRegister(Alpide::REG_ITHR, m_value[0]);
    }
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      uint16_t reg = 0x0;
      m_chips.at(ichip)->ReadRegister(Alpide::REG_ITHR, reg);
      ++reg; // trick the compiler
      break;
    }
    break;
  case 1: // 2nd loop: mask staging
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      ConfigureMaskStage(m_chips.at(ichip), m_value[1]);
    }
    break;
  default:
    break;
  }
}

void TSCurveScan::Execute()
{
  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++)
    m_boards.at(iboard)->Trigger(m_nTriggers);

  usleep(1000);

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    THitSet &hs = m_hitsets->Write();
    hs.board    = iboard;
    hs.val      = m_value[0] - m_start[0]; // m_value is too large (>20) often!!
    hs.hits.clear();
    ReadEventData(&hs.hits, iboard);
    m_hitsets->Push();
  }
}

void TSCurveScan::Histo()
{
  while (!m_stopped) {
    if (m_hitsets->IsEmpty()) {
      usleep(100);
      continue;
    }
    // printf("processing input\n");
    FillHistos(m_hitsets->Read());
    m_hitsets->Pop();
  }
}

void TSCurveScan::FillHistos(const THitSet &hs)
{
  common::TChipIndex idx;
  idx.boardIndex          = hs.board;
  const unsigned int size = hs.hits.size();
  for (unsigned int i = 0; i < size; i++) {
    const TPixHit &hit = hs.hits[i];
    if (hit.address / 2 != m_row)
      continue; // todo: keep track of spurious hits, i.e. hits in non-injected rows
    // !! This will not work when allowing several chips with the same Id
    idx.dataReceiver = hit.channel;
    idx.chipId       = hit.chipId;

    int col = hit.region * 32 + hit.dcol * 2;
    col += ((hit.address + 1) >> 1) & 1;
    // TODO: Catch this case earlier (do not fill hit vector for corrupt events
    try {
      m_histo->Incr(idx, col, hs.val);
    }
    catch (...) {
      std::cout << "Caught exception in TSCurveScan::FillHistos, trying to fill histo for chipID "
                << idx.chipId << ", receiver " << idx.dataReceiver << std::endl;
    }
  }
}

void TSCurveScan::LoopEnd(int loopIndex)
{

  if (loopIndex == 0) {
    while (!m_hitsets->IsEmpty())
      usleep(10);
    m_histo->SetIndex(m_row);
    // std::cout << "SCAN: Writing histo with row " << m_histo->GetIndex() << std::endl;

    // wait for FillHisto to finish
    // (i.e. empty queue, reading is finished when we reach this point)

    m_mutex->lock();
    m_histoQue->push_back(*m_histo);
    m_mutex->unlock();
    m_histo->Clear();
  }
}

void TSCurveScan::Terminate()
{
  m_stopped = true;
  m_thread->join();

  TScan::Terminate();
  // write Data;
  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC *>(m_boards.at(iboard));
    if (myMOSAIC) {
      myMOSAIC->StopRun();
      // delete myMOSAIC;
    }
    TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ *>(m_boards.at(iboard));
    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      // delete myDAQBoard;
    }
  }

  SwitchOffBackbias();

  m_running = false;
  // YCM: Print error summary
  std::cout << "Total number of 8b10b decoder errors: " << m_errorCount.n8b10b << std::endl;
  std::cout << "Number of corrupt events:             " << m_errorCount.nCorruptEvent << std::endl;
  std::cout << "Number of skipped points:             " << m_errorCount.nTimeout << std::endl;
  std::cout << "Priority encoder errors:              " << m_errorCount.nPrioEncoder << std::endl;
  std::cout << std::endl;

  delete m_hitsets;
  m_hitsets = nullptr;
}
