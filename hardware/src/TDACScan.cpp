#include "TDACScan.h"
#include "AlpideConfig.h"
#include "Common.h"

TDACScan::TDACScan(TScanConfig *config, std::vector<TAlpide *> chips, std::vector<THic *> hics,
                   std::vector<TReadoutBoard *> boards, std::deque<TScanHisto> *histoQue,
                   std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  CreateScanParameters();

  m_parameters->backBias = 0;

  strcpy(m_name, "DAC Scan");

  m_start[0] = m_config->GetParamValue("DACSTART");
  m_stop[0]  = m_config->GetParamValue("DACSTOP");
  m_step[0]  = m_config->GetParamValue("DACSTEP");

  m_start[1] = Alpide::REG_VRESETP;
  m_step[1]  = 1;
  m_stop[1]  = Alpide::REG_ITHR;

  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = 1; // number of chips per hic?
}

THisto TDACScan::CreateHisto()
{
  // write currents/voltages for all DAC scans of 1 chip
  // x-axis: DAC register - 0x601
  // y-axis: DAC setting
  THisto histo("DAC measurement", "DAC measurement", m_stop[1] - m_start[1] + 1, 0,
               m_stop[1] - m_start[1], 256, 0, 255);
  return histo;
}

void TDACScan::ConfigureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);
  AlpideConfig::ConfigureCMU(chip);
}

void TDACScan::Init()
{
  CreateScanHisto();

  InitBase(false);
  m_running = true;
  CountEnabledChips();

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    std::cout << "Board " << i << ", found " << m_enabled[i] << " enabled chips" << std::endl;

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

  TScan::SaveStartConditions();
}

void TDACScan::PrepareStep(int loopIndex)
{
  switch (loopIndex) {
  case 0: // innermost loop: change DAC value
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      m_chips.at(ichip)->WriteRegister((Alpide::TRegister)m_value[1], m_value[0]);
    }
  default:
    break;
  }
}

void TDACScan::Execute()
{
  common::TChipIndex idx;
  float              average = 0;
  int                N       = m_config->GetParamValue("NDACSAMPLES");

  if (N == 0) {
    std::cout << "Warning, number of DAC samples 0; not doing anything...";
    return;
  }

  for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
    if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
    idx.boardIndex   = FindBoardIndex(m_chips.at(ichip));
    idx.chipId       = m_chips.at(ichip)->GetConfig()->GetChipId();
    idx.dataReceiver = m_chips.at(ichip)->GetConfig()->GetParamValue("RECEIVER");

    if (m_value[1] < Alpide::REG_IRESET) { // voltage DAC
      for (int i = 0; i < N; i++) {
        average += m_chips.at(ichip)->ReadDACVoltage((Alpide::TRegister)m_value[1]);
      }
    }
    else { // current DAC
      for (int i = 0; i < N; i++) {
        average += m_chips.at(ichip)->ReadDACCurrent((Alpide::TRegister)m_value[1]);
      }
    }
    average /= N;

    m_histo->Set(idx, m_value[1] - m_start[1], m_value[0], average);
  }
}

void TDACScan::LoopStart(int loopIndex)
{
  m_value[loopIndex] = m_start[loopIndex];
  // read current DAC value for first enabled chip;
  // this assumes that all chips have the same settings, which should be the case at this point
  if (loopIndex == 0) {
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      m_chips.at(ichip)->ReadRegister((Alpide::TRegister)m_value[1], m_restoreValue);
      break;
    }
  }
}

void TDACScan::LoopEnd(int loopIndex)
{
  switch (loopIndex) {
  case 0: // innermost loop: set DAC value back
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
      if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      m_chips.at(ichip)->WriteRegister((Alpide::TRegister)m_value[1], m_restoreValue);
    }
  case 1:
    while (!(m_mutex->try_lock()))
      ;
    m_histo->SetIndex(m_value[2]); // in case we add a loop on e.g. voltage
    m_histoQue->push_back(*m_histo);
    m_mutex->unlock();
    m_histo->Clear();
  default:
    break;
  }
}

void TDACScan::Terminate()
{
  TScan::Terminate();
  m_running = false;
}
