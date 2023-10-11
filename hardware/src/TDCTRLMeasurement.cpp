#include "TDCTRLMeasurement.h"
#include "AlpideConfig.h"
#include <exception>
#include <iostream>
#include <string.h>
#include <string>

TDctrlMeasurement::TDctrlMeasurement(TScanConfig *config, std::vector<TAlpide *> chips,
                                     std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                                     std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  CreateScanParameters();

  m_parameters->backBias = 0;

  // FIFO cell that is used for readback operation
  m_region = 13;
  m_offset = 5;

  strcpy(m_name, "Dctrl Measurement"); // Display name
  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = 1;

  // 2nd loop: loop over all chips
  m_start[1] = 0;
  m_step[1]  = 1;
  m_stop[1]  = m_chips.size();

  // innermost loop: loop over all possible driver settings
  m_start[0] = 0;
  m_step[0]  = 1;
  m_stop[0]  = 16;
}


THisto TDctrlMeasurement::CreateHisto()
{
  // save result in a 1-d histogram: x-axis 0..15 corresponds to dctrl setting
  // if second variable needed (rms?) expand to 2-d histo:
  // THisto histo("ErrorHisto", "ErrorHisto", 16, 0, 15, 2, 0, 1);
  THisto histo("ErrorHisto", "ErrorHisto", 16, 0, 15, 8, 0, 7);
  return histo;
}


void TDctrlMeasurement::InitScope()
{
  // scope.debug_en = true; // Enable to print all scope transactions
  if (!scope.open_auto()) { // Auto connects to scope
    throw runtime_error("scope.open_auto() failed!");
  }
  scope.get_errors(); // Check for scope errors

  // Setup channel specific
  for (int i = 1; i <= 4; i++) {
    scope.enable_ch(i);             // Enable channel
    scope.set_vscale_ch(i, 200e-3); // Set V/div
    scope.set_dc_coupling_ch(
        i, false); // Set AC coupling to avoid fiddling with reference level to get pulse on screen
  }

  // Timing
  scope.set_timescale(5e-9); // Set timescale

  // Trigger
  scope.set_trigger_ext();               // Set externally triggered
  scope.set_trigger_slope_rising(false); // Trigger on rising edge
  scope.set_ext_trigger_level(
      -0.3); // Set trigger level (negative as we are using a NIM signal with a terminator)
  scope.set_trigger_position(1.1e-6); // Move center of screen to a known good position for pulse

  // Measure
  scope.setup_measure(); // Enable peak, amplitude, risetime and falltime measurements
}


void TDctrlMeasurement::Init()
{
  CreateScanHisto();

  InitBase(false);

  InitScope();

  // initial chip configurations, modify if necessary
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
  }

  CorrectVoltageDrop();

  m_disableManchesterEncoding = -1;
  for (unsigned int i = 0; i < m_chips.size(); i++) {
    if (!m_chips.at(i)->GetConfig()->IsEnabled()) continue;
    if (m_disableManchesterEncoding == -1) {
      m_disableManchesterEncoding = (m_chips.at(i)->GetConfig()->GetDisableManchester()) ? 1 : 0;
    }
    m_chips.at(i)->GetConfig()->SetDisableManchester(false);
    AlpideConfig::ConfigureCMU(m_chips.at(i));
  }

  CorrectVoltageDrop();

  TScan::SaveStartConditions();
}


int TDctrlMeasurement::GetChipById(std::vector<TAlpide *> chips, int id)
{
  for (unsigned int i = 0; i < chips.size(); i++) {
    if (chips.at(i)->GetConfig()->GetChipId() == id) return i;
  }

  return -1;
}


// prepare step prepares the loop step
// loopIndex 0 (innermost): set the dctrl driver value of the test chip
// loopIndex 1: change the chip under test
void TDctrlMeasurement::PrepareStep(int loopIndex)
{
  switch (loopIndex) {
  case 0: // innermost loop
    if (m_testChip->GetConfig()->IsEnabled()) {
      m_testChip->GetConfig()->SetParamValue("DCTRLDRIVER", m_value[0]);
      AlpideConfig::ConfigureBuffers(m_testChip, m_testChip->GetConfig());
    }
    break;
  case 1: // 2nd loop
    m_testChip   = m_chips.at(m_value[1]);
    m_boardIndex = FindBoardIndex(m_testChip);
    sprintf(m_state, "Running %d", m_value[1]);
    break;
  case 2:
    break;
  default:
    break;
  }
}

void TDctrlMeasurement::WriteMem(TAlpide *chip, int ARegion, int AOffset, int AValue)
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
    std::cout << "Cannot write chip register." << std::endl;
    throw runtime_error("Cannot write chip register.");
  }
}

void TDctrlMeasurement::ReadMem(TAlpide *chip, int ARegion, int AOffset, int &AValue,
                                bool &exception)
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
    // std::cout << "Exception " << e.what() << " when reading low value" << std::endl;
    return;
  }
  exception = false;
  if (err >= 0) {
    try {
      err = chip->ReadRegister(HighAdd, HighVal);
    }
    catch (std::exception &e) {
      // std::cout << "Exception " << e.what() << " when reading high value" << std::endl;
      exception = true;
      return;
    }
  }

  if (err < 0) {
    std::cout << "Cannot read chip register." << std::endl;
    throw runtime_error("Cannot read chip register.");
  }

  // Note to self: if you want to shorten the following lines,
  // remember that HighVal is 16 bit and (HighVal << 16) will yield 0
  // :-)
  AValue = (HighVal & 0xff);
  AValue <<= 16;
  AValue |= LowVal;
}


/// Old test measurement from FIFO scan, adapt if necessary
bool TDctrlMeasurement::TestPattern(int pattern, bool &exception)
{
  int readBack;
  WriteMem(m_testChip, m_region, m_offset, pattern);
  ReadMem(m_testChip, m_region, m_offset, readBack, exception);
  if (exception) return false;
  if (readBack != pattern) return false;
  return true;
}

void TDctrlMeasurement::LoopEnd(int loopIndex)
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
// this method has to implement the amplitude measurement for one single chip,
// for one driver setting
// results are saved into m_histo as described below
void TDctrlMeasurement::Execute()
{
  bool               exception;
  common::TChipIndex idx;
  idx.boardIndex   = m_boardIndex;
  idx.chipId       = m_testChip->GetConfig()->GetChipId();
  idx.dataReceiver = m_testChip->GetConfig()->GetParamValue("RECEIVER");

  // skip disabled chips and OB slaves
  if ((m_testChip->GetConfig()->IsEnabled()) &&
      (m_testChip->GetConfig()->GetParamValue("LINKSPEED") != -1)) {

    scope.single_capture();           // Stop on first trigger
    TestPattern(0xFFFFFF, exception); // Generate data on bus
    /*if (exception) { // Should these be ignored?
      std::cout << "Fifo scan failed" << std::endl;
      throw runtime_erro("Fifo scan failed");
    }*/
    scope.wait_for_trigger(10); // Check and wait until triggered

    // Do the measurement here, value has to be saved in the histogram
    // with THisto::Set, idx indicates the chip, e.g.
    // m_histo->Set(idx, m_value[1], measured amplitude)
    // to enter the measured amplitude for the current chip and the current
    // driver setting
    if ((m_testChip->GetConfig()->GetCtrInt() == 0) && (FindBoardIndex(m_testChip) == 0)) {
      scope.en_measure_ch(3); // Set measurement to read from scope channel 3
      scope.get_meas();       // Retrieve measuremts
      m_histo->Set(idx, m_value[0], peak_p, scope.ch3.peak); // Update plots
      m_histo->Set(idx, m_value[0], amp_p, scope.ch3.amp);
      m_histo->Set(idx, m_value[0], rtim_p, scope.ch3.rtim);
      m_histo->Set(idx, m_value[0], ftim_p, scope.ch3.ftim);
      scope.en_measure_ch(4);
      scope.get_meas();
      m_histo->Set(idx, m_value[0], peak_n, scope.ch4.peak);
      m_histo->Set(idx, m_value[0], amp_n, scope.ch4.amp);
      m_histo->Set(idx, m_value[0], rtim_n, scope.ch4.rtim);
      m_histo->Set(idx, m_value[0], ftim_n, scope.ch4.ftim);
    }
    else {
      scope.en_measure_ch(1);
      scope.get_meas();
      m_histo->Set(idx, m_value[0], peak_p, scope.ch1.peak);
      m_histo->Set(idx, m_value[0], amp_p, scope.ch1.amp);
      m_histo->Set(idx, m_value[0], rtim_p, scope.ch1.rtim);
      m_histo->Set(idx, m_value[0], ftim_p, scope.ch1.ftim);
      scope.en_measure_ch(2);
      scope.get_meas();
      m_histo->Set(idx, m_value[0], peak_n, scope.ch2.peak);
      m_histo->Set(idx, m_value[0], amp_n, scope.ch2.amp);
      m_histo->Set(idx, m_value[0], rtim_n, scope.ch2.rtim);
      m_histo->Set(idx, m_value[0], ftim_n, scope.ch2.ftim);
    }

    // here only to avoid error "idx set but not used"
    // remove in implementation
    // m_histo->Set(idx, m_value[1], 0);
  }
}

void TDctrlMeasurement::Terminate()
{
  if (m_disableManchesterEncoding == 1) {
    for (unsigned int i = 0; i < m_chips.size(); i++) {
      if (!m_chips.at(i)->GetConfig()->IsEnabled()) continue;
      m_chips.at(i)->GetConfig()->SetDisableManchester(true);
      AlpideConfig::ConfigureCMU(m_chips.at(i));
    }
  }

  TScan::Terminate();
  scope.close();

  m_running = false;
}
