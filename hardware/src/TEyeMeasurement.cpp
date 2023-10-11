#include "TEyeMeasurement.h"
#include "AlpideConfig.h"
#include "TReadoutBoardMOSAIC.h"

#include <exception>
#include <iostream>
#include <string.h>
#include <string>


TEyeMeasurement::TEyeMeasurement(TScanConfig *config, std::vector<TAlpide *> chips,
                                 std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                                 std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  CreateScanParameters();

  m_parameters->backBias = 0;

  ((TEyeParameters *)m_parameters)->driverStrength = config->GetParamValue("EYEDRIVER");
  ((TEyeParameters *)m_parameters)->preemphasis    = config->GetParamValue("EYEPREEMP");

  SetName(); // Display name

  // TODO: Assign proper Mosaic

  // outer loop: loop over all chips
  // TODO: can this be done in parallel on all chips?
  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = m_chips.size();

  // loops over phase and amplitude
  m_start[1] = m_config->GetParamValue("EYEMINY");
  m_step[1]  = m_config->GetParamValue("EYESTEPY");
  m_stop[1]  = m_config->GetParamValue("EYEMAXY");

  // innermost loop
  m_start[0] = m_config->GetParamValue("EYEMINX");
  m_step[0]  = m_config->GetParamValue("EYESTEPX");
  m_stop[0]  = m_config->GetParamValue("EYEMAXX");

  // Other Parameters TODO:
  m_min_prescale = 0;
  m_max_prescale = 6; // max 0.1s at 1.2 Gbps

  // NOT supported for now (needs to change looping behaviour)
  m_max_zero_results = 0; // Max number of consecutive zero results
}


void TEyeMeasurement::SetName()
{
  sprintf(m_name, "EyeMeasurement D%d P%d", ((TEyeParameters *)m_parameters)->driverStrength,
          ((TEyeParameters *)m_parameters)->preemphasis);
}


bool TEyeMeasurement::SetParameters(TScanParameters *pars)
{
  TEyeParameters *ePars = dynamic_cast<TEyeParameters *>(pars);
  if (ePars) {
    std::cout << "TEyeMeasurement: Updating parameters" << std::endl;
    ((TEyeParameters *)m_parameters)->driverStrength = ePars->driverStrength;
    ((TEyeParameters *)m_parameters)->preemphasis    = ePars->preemphasis;
    SetName();
    return true;
  }
  else {
    std::cout << "TEyeMeasurement::SetParameters: Error, bad parameter type, doing nothing"
              << std::endl;
    return false;
  }
}


THisto TEyeMeasurement::CreateHisto()
{
  THisto histo("EyeDiagram", "EyeDiagram", 1 + (m_stop[0] - m_start[0]) / m_step[0], m_start[0],
               m_stop[0], 1 + (m_stop[1] - m_start[1]) / m_step[1], m_start[1], m_stop[1]);
  return histo;
}


void TEyeMeasurement::Init()
{
  CreateScanHisto();

  InitBase(false);
  TEyeParameters *params = (TEyeParameters *)m_parameters;

  for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
    if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
    int backupDriver = m_chips.at(ichip)->GetConfig()->GetParamValue("DTUDRIVER");
    int backupPreemp = m_chips.at(ichip)->GetConfig()->GetParamValue("DTUPREEMP");
    m_chips.at(ichip)->GetConfig()->SetParamValue("DTUDRIVER", params->driverStrength);
    m_chips.at(ichip)->GetConfig()->SetParamValue("DTUPREEMP", params->preemphasis);

    AlpideConfig::Init(m_chips.at(ichip));
    AlpideConfig::BaseConfig(m_chips.at(ichip));

    // Enable PRBS (1.2 Gbps)
    uint16_t value = 0;
    value |= 1;      // Test En = 1
    value |= 1 << 1; // Interal Pattern = 1 (Prbs Mode)
    value |= 1 << 5; // Bypass8b10b
    m_chips.at(ichip)->WriteRegister(Alpide::TRegister::REG_DTU_TEST1, value);

    // restore previous settings
    m_chips.at(ichip)->GetConfig()->SetParamValue("DTUDRIVER", backupDriver);
    m_chips.at(ichip)->GetConfig()->SetParamValue("DTUPREEMP", backupPreemp);
  }

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    TReadoutBoardMOSAIC *mosaic = (TReadoutBoardMOSAIC *)m_boards.at(i);
    mosaic->setSpeedMode(Mosaic::RCV_RATE_1200);
  }
  // Maximum prescale factor (Time spend per point)
  // 10 ~= log2(1.2Gbps)/(65536*20)

  m_running = true;

  TScan::SaveStartConditions();
}


// prepare step prepares the loop step
// loopIndex 0, 1 (innermost): set the phase and amplitude values;
// loopIndex 2: change the chip under test
void TEyeMeasurement::PrepareStep(int loopIndex)
{
  int receiverID = 0;
  switch (loopIndex) {
  case 0: // innermost loop
    // Reset the FSM
    // stop run resetting ES_CONTROL[0]
    break;
  case 1: // 2nd loop
    m_current_prescale = m_min_prescale;
    receiverID         = m_board->GetReceiver(m_testChip->GetConfig()->GetChipId());
    m_board->WriteTransceiverDRPField(receiverID, ES_CONTROL, ES_CONTROL_SIZE, ES_CONTROL_OFFSET,
                                      0x0, true);
    break;
  case 2:
    m_testChip   = m_chips.at(m_value[2]);
    m_boardIndex = FindBoardIndex(m_testChip);
    sprintf(m_state, "Running %d", m_value[2]);

    std::cout << "Testing chip : " << m_testChip->GetConfig()->GetChipId() << std::endl;
    m_board = dynamic_cast<TReadoutBoardMOSAIC *>(m_boards.at(m_boardIndex));
    if (!m_board) {
      std::cout << "Error: Wrong board";
      // TODO: Exit with error
    }
    break;
  default:
    break;
  }
}


// any actions to be done at the ebd of the loop,
// most likely only the pushing of the histo at the end of teh outermost loop
void TEyeMeasurement::LoopEnd(int loopIndex)
{
  if (loopIndex == 2) {
    while (!(m_mutex->try_lock()))
      ;
    m_histoQue->push_back(*m_histo);
    m_mutex->unlock();
    m_histo->Clear();
  }
}


// Execute does the actual measurement
// in this case the measureemnt for one point of the eye diagram
// results are saved into m_histo as described below
void TEyeMeasurement::Execute()
{
  int hOffset = m_value[0];
  int vOffset = m_value[1];

  // std::cout << "In execute, x = " << hOffset << ", y = " << vOffset << std::endl;
  uint32_t errorCountReg;
  uint32_t sampleCountReg;
  uint16_t vertOffsetReg;

  // set ES_VERT_OFFSET	bit 7:sign. bits 6-0:offset
  if (vOffset < 0)
    vertOffsetReg = ((-vOffset) & 0x7f) | 0x80;
  else
    vertOffsetReg = vOffset & 0x7f;
  int receiverID = m_board->GetReceiver(m_testChip->GetConfig()->GetChipId());
  m_board->WriteTransceiverDRPField(receiverID, ES_VERT_OFFSET, ES_VERT_OFFSET_SIZE,
                                    ES_VERT_OFFSET_OFFSET, vertOffsetReg, false);

  // set ES_HORZ_OFFSET   [11:0]  bits10-0: Phase offset (2's complement)
  uint16_t horzOffsetReg = hOffset & 0x7ff;

  // bit 11:Phase unification(0:positive 1:negative)
  if (hOffset < 0) horzOffsetReg |= 0x800;
  m_board->WriteTransceiverDRP(receiverID, ES_HORZ_OFFSET, horzOffsetReg, false);

  for (bool goodMeasure = false; !goodMeasure;) {
    // std::cout << "in measuring loop " << std::endl;
    // setup ES_PRESCALE	[15:11]. Prescale = 2**(1+reg_value)
    m_board->WriteTransceiverDRPField(receiverID, ES_PRESCALE, ES_PRESCALE_SIZE, ES_PRESCALE_OFFSET,
                                      m_current_prescale, false);

    // set ES_CONTROL[0] to start the measure run
    // Configure and run measure
    m_board->WriteTransceiverDRPField(receiverID, ES_CONTROL, ES_CONTROL_SIZE, ES_CONTROL_OFFSET,
                                      0x1, true);

    // poll the es_control_status[0] for max 10s
    int i;
    for (i = 10000; i > 0; i--) {
      uint32_t val;
      usleep(1000);
      m_board->ReadTransceiverDRP(receiverID, ES_CONTROL_STATUS, &val);
      if (val & ES_CONTROL_STATUS_DONE) break;
    }
    if (i == 0) throw std::runtime_error("Timeout reading es_control_status");

    // stop run resetting ES_CONTROL[0]
    m_board->WriteTransceiverDRPField(receiverID, ES_CONTROL, ES_CONTROL_SIZE, ES_CONTROL_OFFSET,
                                      0x0);

    // read es_error_count and es_sample_count
    m_board->ReadTransceiverDRP(receiverID, ES_ERROR_COUNT, &errorCountReg, false);
    m_board->ReadTransceiverDRP(receiverID, ES_SAMPLE_COUNT, &sampleCountReg, true);

    if (errorCountReg == 0xffff && m_current_prescale == 0) {
      goodMeasure = true;
    }
    else if (sampleCountReg == 0xffff && errorCountReg > 0x7fff) {
      goodMeasure = true;
    }
    else if (sampleCountReg == 0xffff && m_current_prescale == m_max_prescale) {
      goodMeasure = true;
    }
    else if (errorCountReg == 0xffff && m_current_prescale > 0) {
      m_current_prescale--;
    }
    else if (errorCountReg <= 0x7fff) { // measure time too short
      if (m_current_prescale < m_max_prescale) {
        m_current_prescale++;
      }
    }
    else {
      goodMeasure = true;
    }
    // std::cout << "end of loop, current prescale " << m_current_prescale << std::endl;
  }

  double scanValue = ((double)errorCountReg / ((double)BUS_WIDTH * (double)sampleCountReg *
                                               (1UL << (m_current_prescale + 1))));

  // std::cout << "X " << hOffset << ", Y " << vOffset << ", value: " << scanValue << "\n";
  common::TChipIndex idx;
  idx.boardIndex   = m_boardIndex;
  idx.chipId       = m_testChip->GetConfig()->GetChipId();
  idx.dataReceiver = m_testChip->GetConfig()->GetParamValue("RECEIVER");
  // TODO: take into account step width (if != 1)
  m_histo->Set(idx, (hOffset - m_start[0]) / m_step[0], (vOffset - m_start[1]) / m_step[1],
               scanValue);
}


void TEyeMeasurement::Terminate()
{
  TScan::Terminate();

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    TReadoutBoardMOSAIC *mosaic = (TReadoutBoardMOSAIC *)m_boards.at(i);
    mosaic->setSpeedMode(((TBoardConfigMOSAIC *)m_board->GetConfig())->GetSpeedMode());
  }
  m_running = false;
}
