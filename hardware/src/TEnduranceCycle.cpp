#include "TEnduranceCycle.h"
#include "AlpideConfig.h"
#include "TReadoutBoardMOSAIC.h"
#include <algorithm>
#include <string.h>
#include <string>

int OpenEnduranceRecoveryFile(const char *fName, std::vector<std::string> hicNames,
                              std::deque<std::map<std::string, THicCounter>> &counterVector)
{
  FILE *                             fp = fopen(fName, "r");
  char                               hicName[50];
  THicCounter                        counter;
  int                                trip;
  std::vector<std::string>::iterator stringIter;
  std::map<std::string, THicCounter> counterMap;

  if (!fp) {
    std::cout << "Error, file " << fName << " not found." << std::endl;
    return 0;
  }

  while (fscanf(fp, "%s %d %d %d %d %d %d %d %d %d %f %f %f %f %f %f", hicName, &trip,
                &counter.m_nWorkingChips, &counter.m_exceptions, &counter.m_fifoErrors,
                &counter.m_fifoErrors0, &counter.m_fifoErrors5, &counter.m_fifoErrorsa,
                &counter.m_fifoErrorsf, &counter.m_fifoExceptions, &counter.m_iddaClocked,
                &counter.m_idddClocked, &counter.m_iddaConfigured, &counter.m_idddConfigured,
                &counter.m_tempStart, &counter.m_tempEnd) == 16) {
    // check that hic name found is contained in hicNames, otherwise ignore entry
    counter.m_trip = (trip > 0);
    stringIter     = find(hicNames.begin(), hicNames.end(), string(hicName));
    if (stringIter == hicNames.end()) {
      std::cout << "Warning, found unknown HIC " << hicName << " in file, ignored" << std::endl;
      continue;
    }
    // check hic name already in map -> push map into vector and clear map
    // (assume only one entry per HIC and cycle)
    if (counterMap.find(string(hicName)) != counterMap.end()) {
      counterVector.push_back(counterMap);
      counterMap.clear();
    }
    // add counter to map
    counterMap.insert(std::pair<std::string, THicCounter>(string(hicName), counter));
  }
  // eof-> push last map into vector
  if (counterMap.size() > 0) counterVector.push_back(counterMap);
  fclose(fp);
  return counterVector.size();
}


TEnduranceCycle::TEnduranceCycle(TScanConfig *config, std::vector<TAlpide *> chips,
                                 std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                                 std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  CreateScanParameters();

  strcpy(m_name, "Endurance Cycle");

  m_parameters->backBias = 0;

  ((TCycleParameters *)m_parameters)->nTriggers = config->GetParamValue("ENDURANCETRIGGERS");
  ((TCycleParameters *)m_parameters)->upTime    = config->GetParamValue("ENDURANCEUPTIME");
  ((TCycleParameters *)m_parameters)->downTime  = config->GetParamValue("ENDURANCEDOWNTIME");
  ((TCycleParameters *)m_parameters)->nCycles   = config->GetParamValue("ENDURANCECYCLES");
  ((TCycleParameters *)m_parameters)->timeLimit = config->GetParamValue("ENDURANCETIMELIMIT");

  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = 1;

  m_start[1] = 0;
  m_step[1]  = 1;
  m_stop[1]  = 1;

  m_start[0] = 0;
  m_step[0]  = 1;
  m_stop[0]  = ((TCycleParameters *)m_parameters)->nCycles;

  CreateMeasurements();
  m_histo = 0;
}


bool TEnduranceCycle::SetParameters(TScanParameters *pars)
{
  TCycleParameters *cPars = dynamic_cast<TCycleParameters *>(pars);
  if (cPars) {
    std::cout << "TEnduranceCycle: Updating parameters" << std::endl;
    ((TCycleParameters *)m_parameters)->upTime    = cPars->upTime;
    ((TCycleParameters *)m_parameters)->downTime  = cPars->downTime;
    ((TCycleParameters *)m_parameters)->nTriggers = cPars->nTriggers;
    ((TCycleParameters *)m_parameters)->nCycles   = cPars->nCycles;
    ((TCycleParameters *)m_parameters)->timeLimit = cPars->timeLimit;
    m_stop[0]                                     = ((TCycleParameters *)m_parameters)->nCycles;
    return true;
  }
  else {
    std::cout << "TEnduranceCycle::SetParameters: Error, bad parameter type, doing nothing"
              << std::endl;
    return false;
  }
}


void TEnduranceCycle::CreateMeasurements()
{
  // create map with measurement structure for each HIC
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    THicCounter hicCounter;
    hicCounter.m_hicType = m_hics.at(i)->GetHicType();
    m_hicCounters.insert(std::pair<std::string, THicCounter>(m_hics.at(i)->GetDbId(), hicCounter));
  }
}

void TEnduranceCycle::ClearCounters()
{
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    m_hicCounters.at(m_hics.at(i)->GetDbId()).m_nWorkingChips  = 0;
    m_hicCounters.at(m_hics.at(i)->GetDbId()).m_fifoTests      = 0;
    m_hicCounters.at(m_hics.at(i)->GetDbId()).m_fifoErrors     = 0;
    m_hicCounters.at(m_hics.at(i)->GetDbId()).m_fifoErrors0    = 0;
    m_hicCounters.at(m_hics.at(i)->GetDbId()).m_fifoErrors5    = 0;
    m_hicCounters.at(m_hics.at(i)->GetDbId()).m_fifoErrorsa    = 0;
    m_hicCounters.at(m_hics.at(i)->GetDbId()).m_fifoErrorsf    = 0;
    m_hicCounters.at(m_hics.at(i)->GetDbId()).m_fifoExceptions = 0;
    m_hicCounters.at(m_hics.at(i)->GetDbId()).m_exceptions     = 0;
    m_hicCounters.at(m_hics.at(i)->GetDbId()).m_trip           = false;
  }
}


void TEnduranceCycle::ReadRecoveredCounters(
    std::deque<std::map<std::string, THicCounter>> &counterVector)
{
  // loop over cycles; stop if none left in deque or max cycles for this slice reached
  while ((counterVector.size() > 0) && (m_start[0] < m_stop[0])) {
    std::map<std::string, THicCounter>           hicCounters = counterVector.front();
    std::map<std::string, THicCounter>::iterator hicIt;
    // loop over HICs for this cycle
    for (hicIt = hicCounters.begin(); hicIt != hicCounters.end(); hicIt++) {
      try {
        m_hicCounters.at(hicIt->first).m_trip           = hicIt->second.m_trip;
        m_hicCounters.at(hicIt->first).m_iddaClocked    = hicIt->second.m_iddaClocked;
        m_hicCounters.at(hicIt->first).m_idddClocked    = hicIt->second.m_idddClocked;
        m_hicCounters.at(hicIt->first).m_iddaConfigured = hicIt->second.m_iddaConfigured;
        m_hicCounters.at(hicIt->first).m_idddConfigured = hicIt->second.m_idddConfigured;
        m_hicCounters.at(hicIt->first).m_tempStart      = hicIt->second.m_tempStart;
        m_hicCounters.at(hicIt->first).m_tempEnd        = hicIt->second.m_tempEnd;
        m_hicCounters.at(hicIt->first).m_nWorkingChips  = hicIt->second.m_nWorkingChips;
        m_hicCounters.at(hicIt->first).m_exceptions     = hicIt->second.m_exceptions;
        m_hicCounters.at(hicIt->first).m_fifoErrors     = hicIt->second.m_fifoErrors;
        m_hicCounters.at(hicIt->first).m_fifoErrors0    = hicIt->second.m_fifoErrors0;
        m_hicCounters.at(hicIt->first).m_fifoErrors5    = hicIt->second.m_fifoErrors5;
        m_hicCounters.at(hicIt->first).m_fifoErrorsa    = hicIt->second.m_fifoErrorsa;
        m_hicCounters.at(hicIt->first).m_fifoErrorsf    = hicIt->second.m_fifoErrorsf;
        m_hicCounters.at(hicIt->first).m_fifoExceptions = hicIt->second.m_fifoExceptions;
        m_hicCounters.at(hicIt->first).m_fifoTests      = hicIt->second.m_fifoTests;
      }
      catch (...) {
        std::cout << "Warning, found unknown HIC " << hicIt->first << " in deque, ignored"
                  << std::endl;
        continue;
      }
    }
    // push this cycle to vector and increment start point for "real" cycles

    WriteRecoveryFile();
    m_counterVector.push_back(m_hicCounters);
    m_start[0]++;
    // remove first entry from deque
    counterVector.pop_front();
  }
}


void TEnduranceCycle::Init()
{
  InitBase(false);
  time(&m_startTime);
  // configure readout boards
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    ConfigureBoard(m_boards.at(i));
  }
  // disable all receivers
  for (unsigned int i = 0; i < m_chips.size(); i++) {
    if (!(m_chips.at(i)->GetConfig()->IsEnabled())) continue;
    m_chips.at(i)->GetReadoutBoard()->SetChipEnable(m_chips.at(i), false);
  }
  TScan::SaveStartConditions();
}


void TEnduranceCycle::PrepareStep(int loopIndex)
{
  switch (loopIndex) {
  case 0: // innermost loop: cycling
    std::cout << "Starting cycle " << m_value[0] << std::endl;
    sprintf(m_state, "Running %d", m_value[0]);
    break;
  default:
    break;
  }
}

// Try to communicate with all chips, disable chips that are not answering
void TEnduranceCycle::CountWorkingChips()
{
  uint16_t WriteValue = 10;
  uint16_t Value;

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
  }

  for (unsigned int i = 0; i < m_chips.size(); i++) {
    m_chips.at(i)->WriteRegister(0x60d, WriteValue);
    try {
      m_chips.at(i)->ReadRegister(0x60d, Value);
      if (WriteValue == Value) {
        THic *hic = m_chips.at(i)->GetHic();
        m_hicCounters.at(hic->GetDbId()).m_nWorkingChips++;
        m_chips.at(i)->SetEnable(true);
      }
      else {
        m_chips.at(i)->SetEnable(false);
        m_chips.at(i)->GetConfig()->fEnduranceDisabled = true;
      }
    }
    catch (exception &e) {
      m_chips.at(i)->SetEnable(false);
      m_chips.at(i)->GetConfig()->fEnduranceDisabled = true;
    }
  }
}

void TEnduranceCycle::ConfigureBoard(TReadoutBoard *board)
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
    board->SetTriggerConfig(false, true, board->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                            board->GetConfig()->GetParamValue("PULSEDELAY"));
    board->SetTriggerSource(trigInt);
  }
}

void TEnduranceCycle::ConfigureFromu(TAlpide *chip)
{
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1, 0x0); // digital pulsing
  chip->WriteRegister(
      Alpide::REG_FROMU_CONFIG2,
      chip->GetConfig()->GetParamValue("STROBEDURATION")); // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1,
                      chip->GetConfig()->GetParamValue("STROBEDELAYCHIP")); // fromu pulsing 1:
                                                                            // delay pulse - strobe
                                                                            // (not used here, since
                                                                            // using external
                                                                            // strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, 0); // fromu pulsing 2: pulse length
}

void TEnduranceCycle::ConfigureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);
  ConfigureFromu(chip);
  ConfigureMask(chip);
  AlpideConfig::ConfigureCMU(chip);
}

void TEnduranceCycle::ConfigureMask(TAlpide *chip)
{
  AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_MASK, true);
  AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_SELECT, false);

  // TODO: decide on correct masking; for the time being: enable row 0
  AlpideConfig::WritePixRegRow(chip, Alpide::PIXREG_MASK, false, 0);
  AlpideConfig::WritePixRegRow(chip, Alpide::PIXREG_SELECT, true, 0);
}

void TEnduranceCycle::Execute()
{
  // 1) Power on all HICs, check for trips, measure currents
  std::cout << "  Powering on" << std::endl;
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    m_hics.at(ihic)->PowerOn();
    sleep(1);
    m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_idddClocked = m_hics.at(ihic)->GetIddd();
    m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_iddaClocked = m_hics.at(ihic)->GetIdda();
    m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_trip        = !(m_hics.at(ihic)->IsPowered());
  }

  // 2) enable all chips, check control interfaces -> number of working chips
  //    measure initial temperature (here and not earlier to avoid non-working chips
  CountWorkingChips();
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    try {
      m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_tempStart = m_hics.at(ihic)->GetTemperature();
    }
    catch (...) {
      m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_tempStart = 0;
      m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_exceptions++;
      std::cout << "Exception in HIC " << m_hics.at(ihic)->GetDbId() << " while reading temp"
                << std::endl;
    }
  }

  // 3) configure chips, measure currents
  for (unsigned int i = 0; i < m_boards.size(); i++) {
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
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_RORST);
    m_boards.at(i)->StartRun();
  }
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod(),
                                                         TPowerBoardConfig::none);
    m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_idddConfigured = m_hics.at(ihic)->GetIddd();
    m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_iddaConfigured = m_hics.at(ihic)->GetIdda();
  }

  // 4) trigger
  std::cout << "  Triggering" << std::endl;
  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    m_boards.at(iboard)->Trigger(((TCycleParameters *)m_parameters)->nTriggers);
  }

  // 5) wait, measure final temperature & power off, sleep again
  //    while waiting: perform Fifo scan
  std::cout << "  Waiting and scanning FIFOs" << std::endl;
  time_t start;

  time(&start);
  int upTime = ((TCycleParameters *)m_parameters)->upTime;

  for (int ioffset = 0; (ioffset < 128) && (difftime(time(NULL), start) < upTime); ioffset++) {
    for (int ireg = 0; ireg < 32; ireg++) {
      for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
        THic *hic = m_chips.at(ichip)->GetHic();
        TestPattern(m_chips.at(ichip), 0x0000, ireg, ioffset, m_hicCounters.at(hic->GetDbId()));
        TestPattern(m_chips.at(ichip), 0x5555, ireg, ioffset, m_hicCounters.at(hic->GetDbId()));
        TestPattern(m_chips.at(ichip), 0xaaaa, ireg, ioffset, m_hicCounters.at(hic->GetDbId()));
        TestPattern(m_chips.at(ichip), 0xffff, ireg, ioffset, m_hicCounters.at(hic->GetDbId()));
        m_hicCounters.at(hic->GetDbId()).m_fifoTests++;
      }
    }
  }
  std::cout << "FIFO scan done after " << difftime(time(NULL), start) << " s." << std::endl;

  while (difftime(time(NULL), start) < upTime)
    sleep(1);

  std::cout << "  Powering off" << std::endl;
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    try {
      m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_tempEnd = m_hics.at(ihic)->GetTemperature();
    }
    catch (...) {
      m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_tempEnd = 0;
      m_hicCounters.at(m_hics.at(ihic)->GetDbId()).m_exceptions++;
      std::cout << "Exception in HIC " << m_hics.at(ihic)->GetDbId() << " while reading temp"
                << std::endl;
    }
    m_hics.at(ihic)->PowerOff();
  }
  WriteRecoveryFile();
  std::cout << "  Waiting" << std::endl;
  sleep(((TCycleParameters *)m_parameters)->downTime);
}


void TEnduranceCycle::WriteRecoveryFile()
{
  char fName[200];
  sprintf(fName, "CycleRecoveryFile_%s.dat", m_config->GetStartTime());

  FILE *fp = fopen(fName, "a");

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (!m_hics.at(ihic)->IsEnabled()) continue;
    THicCounter counter = m_hicCounters.at(m_hics.at(ihic)->GetDbId());
    fprintf(fp, "%s %d %d %d %d %d %d %d %d %d %.3f %.3f %.3f %.3f %.1f %.1f\n",
            m_hics.at(ihic)->GetDbId().c_str(), counter.m_trip ? 1 : 0, counter.m_nWorkingChips,
            counter.m_exceptions, counter.m_fifoErrors, counter.m_fifoErrors0,
            counter.m_fifoErrors5, counter.m_fifoErrorsa, counter.m_fifoErrorsf,
            counter.m_fifoExceptions, counter.m_iddaClocked, counter.m_idddClocked,
            counter.m_iddaConfigured, counter.m_idddConfigured, counter.m_tempStart,
            counter.m_tempEnd);
  }

  fclose(fp);
}


void TEnduranceCycle::Next(int loopIndex)
{
  time_t timeNow;
  if (loopIndex == 0) {
    m_counterVector.push_back(m_hicCounters);
    ClearCounters();

    time(&timeNow);
    if (difftime(timeNow, m_startTime) > ((TCycleParameters *)m_parameters)->timeLimit * 3600) {
      fTimeLimitReached = true;
    }
    // temporary fix: re-enable chips that were disabled in endurance test
    for (unsigned int i = 0; i < m_chips.size(); i++) {
      if (m_chips.at(i)->GetConfig()->fEnduranceDisabled) {
        m_chips.at(i)->SetEnable(true);
        m_chips.at(i)->GetConfig()->fEnduranceDisabled = false;
      }
    }
  }
  TScan::Next(loopIndex);
}

void TEnduranceCycle::Terminate()
{
  TScan::Terminate();
  m_running = false;
  // re-enable receivers in readout board for all enabled chips
  for (unsigned int i = 0; i < m_chips.size(); i++) {
    m_chips.at(i)->GetReadoutBoard()->SetChipEnable(m_chips.at(i),
                                                    m_chips.at(i)->GetConfig()->IsEnabled());
  }
}


void TEnduranceCycle::WriteMem(TAlpide *chip, int ARegion, int AOffset, int AValue)
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


void TEnduranceCycle::ReadMem(TAlpide *chip, int ARegion, int AOffset, int &AValue)
{
  if ((ARegion > 31) || (AOffset > 127)) {
    std::cout << "ReadMem: invalid parameters" << std::endl;
    return;
  }
  uint16_t LowAdd  = Alpide::REG_RRU_MEB_LSB_BASE | (ARegion << 11) | AOffset;
  uint16_t HighAdd = Alpide::REG_RRU_MEB_MSB_BASE | (ARegion << 11) | AOffset;

  uint16_t LowVal, HighVal;
  int      err;

  err = chip->ReadRegister(LowAdd, LowVal);
  if (err >= 0) {
    err = chip->ReadRegister(HighAdd, HighVal);
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


bool TEnduranceCycle::TestPattern(TAlpide *chip, int pattern, int region, int offset,
                                  THicCounter &hicCounter)
{
  int readBack;
  try {
    WriteMem(chip, region, offset, pattern);
    ReadMem(chip, region, offset, readBack);
  }
  catch (...) {
    hicCounter.m_fifoExceptions++;
    return false;
  }
  if (readBack != pattern) {
    if (hicCounter.m_fifoErrors < 12) {
      std::cout << "Fifo test cycle " << m_value[0] << ", wrote pattern 0x" << std::hex << pattern
                << ", read 0x" << readBack << std::dec << std::endl;
    }
    hicCounter.m_fifoErrors++;
    if (pattern == 0x0)
      hicCounter.m_fifoErrors0++;
    else if (pattern == 0x5555)
      hicCounter.m_fifoErrors5++;
    else if (pattern == 0xaaaa)
      hicCounter.m_fifoErrorsa++;
    else if (pattern == 0xffff)
      hicCounter.m_fifoErrorsf++;
    return false;
  }
  return true;
}
