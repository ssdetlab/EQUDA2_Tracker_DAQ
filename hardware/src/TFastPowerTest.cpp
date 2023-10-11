#include "TFastPowerTest.h"
#include "AlpideConfig.h"
#include "TReadoutBoardMOSAIC.h"
#include <string.h>
#include <string>

TFastPowerTest::TFastPowerTest(TScanConfig *config, std::vector<TAlpide *> chips,
                               std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                               std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  CreateScanParameters();

  m_parameters->backBias = 0;
  m_ivcurve              = (m_config->GetParamValue("IVCURVE") != 0);

  strcpy(m_name, "Fast Power Test");
  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = 1;

  m_start[1] = 0;
  m_step[1]  = 1;
  m_stop[1]  = 1;

  m_start[0] = 0;
  m_step[0]  = 1;
  m_stop[0]  = m_hics.size();

  CreateMeasurements();
  m_histo = 0;
}

void TFastPowerTest::CreateMeasurements()
{
  // create map with measurement structure for each HIC
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    THicCurrents hicCurrents = {};
    hicCurrents.hicType      = m_hics.at(i)->GetHicType();
    hicCurrents.maxBias      = ((float)(m_config->GetParamValue("IVPOINTS")) - 1) / 10;
    m_hicCurrents.insert(
        std::pair<std::string, THicCurrents>(m_hics.at(i)->GetDbId(), hicCurrents));
  }
}

void TFastPowerTest::Init()
{
  strcpy(m_state, "Running");
  std::cout << std::endl
            << std::endl
            << ">>>>>>>> Starting scan " << GetName() << std::endl
            << std::endl;
  time_t     t   = time(0); // get time now
  struct tm *now = localtime(&t);

  sprintf(m_config->GetfNameSuffix(), "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100,
          now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

  std::cout << "Output file time suffix: " << m_config->GetfNameSuffix() << std::endl;
}

void TFastPowerTest::PrepareStep(int loopIndex)
{
  switch (loopIndex) {
  case 0: // innermost loop: change HIC
    m_testHic = m_hics.at(m_value[0]);
    break;
  case 1: // 2nd loop
    break;
  case 2: // outermost loop
    break;
  }
}

void TFastPowerTest::DoIVCurve(THicCurrents &result)
{
  for (int i = 0; i < m_config->GetParamValue("IVPOINTS"); i++) {
    float voltage = -((float)i) / 10.;
    m_testHic->GetPowerBoard()->SetBiasVoltage(voltage);
    sleep(1);
    result.ibias[i] = m_testHic->GetIBias() * 1000; // convert in mA
    // overcurrent protection; will be counted as trip
    if (result.ibias[i] > m_config->GetParamValue("MAXIBIAS")) {
      printf("MAXIBIAS exceeded at %g: %g > %d, switching off Vbias\n", voltage, result.ibias[i],
             m_config->GetParamValue("MAXIBIAS"));
      m_hicCurrents.find(m_testHic->GetDbId())->second.maxBias = voltage - .1;
      m_testHic->GetPowerBoard()->SetBiasVoltage(0.0);
      sleep(1);
      break;
    }
  }
}

void TFastPowerTest::Execute()
{
  std::vector<int> boardIndices = m_testHic->GetBoardIndices();

  std::map<std::string, THicCurrents>::iterator currentIt =
      m_hicCurrents.find(m_testHic->GetDbId());

  for (auto board : m_boards) {
    TReadoutBoardMOSAIC *mosaic = dynamic_cast<TReadoutBoardMOSAIC *>(board);
    if (mosaic) mosaic->enableControlInterfaces(false);
  }

  m_testHic->PowerOn();
  // m_testHic->GetPowerBoard()->CorrectVoltageDrop(m_testHic->GetPbMod());

  // measure -> switchon, no clock
  currentIt->second.idddSwitchon = m_testHic->GetIddd();
  currentIt->second.iddaSwitchon = m_testHic->GetIdda();

  // check if supply tripped
  if (!m_testHic->IsPowered()) {
    currentIt->second.trip = true;
  }
  else {
    currentIt->second.trip = false;
  }

  // switch on back bias only for module under test
  for (int i = 0; i < 8; i++) {
    if (i == m_testHic->GetBbChannel()) {
      m_testHic->GetPowerBoard()->SetBiasOn(i);
    }
    else {
      m_testHic->GetPowerBoard()->SetBiasOff(i);
    }
  }

  currentIt->second.ibias0 = m_testHic->GetIBias() * 1000;

  // measure IV curve or only bias current at 3 V
  if (m_ivcurve) {
    DoIVCurve(currentIt->second);
    currentIt->second.ibias3 = currentIt->second.ibias[30];
  }
  else {
    m_testHic->GetPowerBoard()->SetBiasVoltage(-3.0);
    sleep(1);
    currentIt->second.ibias3 = m_testHic->GetIBias() * 1000;
  }

  // check if supply tripped
  if (!currentIt->second.trip && !m_testHic->IsPowered()) {
    currentIt->second.tripBB = true;
  }
  else {
    currentIt->second.tripBB = false;
  }

  // check if back bias tripped
  if (m_testHic->GetPowerBoard()->GetBiasVoltage() > -1.0) {
    std::cout << "reading bias voltage of " << m_testHic->GetPowerBoard()->GetBiasVoltage()
              << std::endl;
    currentIt->second.tripBB = true;
  }
  else {
    currentIt->second.tripBB = false;
  }
  m_testHic->GetPowerBoard()->SetBiasVoltage(0.0);
  m_testHic->PowerOff();
}

void TFastPowerTest::Terminate()
{
  m_running = false;
  strcpy(m_state, "Done");
}
