#include <string.h>
#include <string>

#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
// #include "TReadoutBoardRU.h"
#include "TReadoutTest.h"

TReadoutTest::TReadoutTest(TScanConfig *config, std::vector<TAlpide *> chips,
                           std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                           std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TDataTaking(config, chips, hics, boards, histoQue, aMutex)
{
  CreateScanParameters();

  m_parameters->backBias = 0;
  // trigger frequency, number of triggers have to be set in scan config
  // before creating readout test object
  m_pulse       = true;
  m_pulseLength = chips.at(0)->GetConfig()->GetParamValue("PULSEDURATION");

  ((TReadoutParameters *)m_parameters)->nTriggers = config->GetParamValue("NTRIG");
  ;
  ((TReadoutParameters *)m_parameters)->row            = config->GetParamValue("READOUTROW");
  ((TReadoutParameters *)m_parameters)->linkSpeed      = config->GetParamValue("READOUTSPEED");
  ((TReadoutParameters *)m_parameters)->occupancy      = config->GetParamValue("READOUTOCC");
  ((TReadoutParameters *)m_parameters)->driverStrength = config->GetParamValue("READOUTDRIVER");
  ((TReadoutParameters *)m_parameters)->preemp         = config->GetParamValue("READOUTPREEMP");
  ((TReadoutParameters *)m_parameters)->pllStages      = config->GetParamValue("READOUTPLLSTAGES");
  ((TReadoutParameters *)m_parameters)->voltageScale   = config->GetVoltageScale();

  SetName();
}


void TReadoutTest::SetName()
{
  if (((TReadoutParameters *)m_parameters)->pllStages != -1) {
    sprintf(m_name, "ReadoutTest %.1f %d", ((TReadoutParameters *)m_parameters)->voltageScale,
            ((TReadoutParameters *)m_parameters)->pllStages);
  }
  else {
    sprintf(m_name, "ReadoutTest %d %d %d", ((TReadoutParameters *)m_parameters)->linkSpeed,
            ((TReadoutParameters *)m_parameters)->driverStrength,
            ((TReadoutParameters *)m_parameters)->preemp);
  }
}


bool TReadoutTest::SetParameters(TScanParameters *pars)
{
  TReadoutParameters *rPars = dynamic_cast<TReadoutParameters *>(pars);
  if (rPars) {
    std::cout << "TReadoutTest: Updating parameters" << std::endl;

    ((TReadoutParameters *)m_parameters)->nTriggers      = rPars->nTriggers;
    ((TReadoutParameters *)m_parameters)->row            = rPars->row;
    ((TReadoutParameters *)m_parameters)->linkSpeed      = rPars->linkSpeed;
    ((TReadoutParameters *)m_parameters)->occupancy      = rPars->occupancy;
    ((TReadoutParameters *)m_parameters)->driverStrength = rPars->driverStrength;
    ((TReadoutParameters *)m_parameters)->preemp         = rPars->preemp;
    ((TReadoutParameters *)m_parameters)->pllStages      = rPars->pllStages;
    ((TReadoutParameters *)m_parameters)->voltageScale   = rPars->voltageScale;

    SetName();
    return true;
  }
  else {
    std::cout << "TReadoutTest::SetParameters: Error, bad parameter type, doing nothing"
              << std::endl;
    return false;
  }
}


// TODO: save number of masked pixels (return value of ApplyMask)
void TReadoutTest::ConfigureChip(TAlpide *chip)
{
  TReadoutParameters *params = (TReadoutParameters *)m_parameters;
  // store driver settings and set temporary ones, used in this scan
  // (used by BaseConfig -> BaseConfigPLL
  int backupDriver = chip->GetConfig()->GetParamValue("DTUDRIVER");
  int backupPreemp = chip->GetConfig()->GetParamValue("DTUPREEMP");
  int backupSpeed  = chip->GetConfig()->GetParamValue("LINKSPEED");
  int backupStages = chip->GetConfig()->GetParamValue("PLLSTAGES");
  chip->GetConfig()->SetParamValue("DTUDRIVER", params->driverStrength);
  chip->GetConfig()->SetParamValue("DTUPREEMP", params->preemp);
  chip->GetConfig()->SetParamValue("LINKSPEED", params->linkSpeed);
  if (params->pllStages >= 0) {
    chip->GetConfig()->SetParamValue("PLLSTAGES", params->pllStages);
  }
  AlpideConfig::BaseConfig(chip);
  ConfigureFromu(chip);
  ConfigureMask(chip, 0);
  AlpideConfig::ApplyMask(chip, false);
  AlpideConfig::ConfigureCMU(chip);
  // restore previous settings
  chip->GetConfig()->SetParamValue("DTUDRIVER", backupDriver);
  chip->GetConfig()->SetParamValue("DTUPREEMP", backupPreemp);
  chip->GetConfig()->SetParamValue("LINKSPEED", backupSpeed);
  if (params->pllStages >= 0) {
    chip->GetConfig()->SetParamValue("PLLSTAGES", backupStages);
  }
}

// TODO: Add masking / selecting of given occupancy
void TReadoutTest::ConfigureMask(TAlpide *chip, std::vector<TPixHit> *MaskedPixels)
{
  AlpideConfig::ConfigureMaskStage(chip, ((TReadoutParameters *)m_parameters)->occupancy,
                                   ((TReadoutParameters *)m_parameters)->row, true, true);
}

THisto TReadoutTest::CreateHisto()
{
  THisto histo("HitmapHisto", "HitmapHisto", 1024, 0, 1023, 512, 0, 511);
  return histo;
}

void TReadoutTest::Init()
{
  int   linkSpeed    = ((TReadoutParameters *)m_parameters)->linkSpeed;
  float voltageScale = ((TReadoutParameters *)m_parameters)->voltageScale;
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    TReadoutBoardMOSAIC *mosaic = (TReadoutBoardMOSAIC *)m_boards.at(i);
    if (mosaic) {
      if (linkSpeed == 400)
        mosaic->setSpeedMode(Mosaic::RCV_RATE_400);
      else if (linkSpeed == 600)
        mosaic->setSpeedMode(Mosaic::RCV_RATE_600);
      else if (linkSpeed == 1200)
        mosaic->setSpeedMode(Mosaic::RCV_RATE_1200);
    }
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (voltageScale != 1.) {
      m_hics.at(ihic)->ScaleVoltage(voltageScale);
    }
  }

  TDataTaking::Init();

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hics.at(ihic)->ReadChipRegister(
        Alpide::REG_PLL_LOCK1,
        m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_pllLockStart);
  }

  m_running = true;
}


void TReadoutTest::WritePLLReg(const char *fName, THic *aHic)
{
  FILE *fp = fopen(fName, "a");

  std::map<int, uint16_t>::iterator it;

  for (it = m_conditions.m_hicConditions.at(aHic->GetDbId())->m_pllLockStart.begin();
       it != m_conditions.m_hicConditions.at(aHic->GetDbId())->m_pllLockStart.end(); it++) {
    fprintf(fp, "  PLL Lock Register (start) on chip %d: 0x%x\n", it->first, it->second);
  }
  fputs("\n", fp);
  for (it = m_conditions.m_hicConditions.at(aHic->GetDbId())->m_pllLockEnd.begin();
       it != m_conditions.m_hicConditions.at(aHic->GetDbId())->m_pllLockEnd.end(); it++) {
    fprintf(fp, "  PLL Lock Register (end) on chip %d: 0x%x\n", it->first, it->second);
  }
  fputs("\n", fp);
  fclose(fp);
}


void TReadoutTest::Terminate()
{
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hics.at(ihic)->ReadChipRegister(
        Alpide::REG_PLL_LOCK1,
        m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_pllLockEnd);
  }

  TDataTaking::Terminate();

  // restore old voltage
  int voltageScale = ((TReadoutParameters *)m_parameters)->voltageScale;
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (voltageScale != 1.) {
      m_hics.at(ihic)->ScaleVoltage(1.);
    }
  }

  // reset link speed in mosaic; chip configs have been reset already in ConfigureChip()
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    TReadoutBoardMOSAIC *mosaic = (TReadoutBoardMOSAIC *)m_boards.at(i);
    if (mosaic) {
      int linkSpeed = m_chips.at(0)->GetConfig()->GetParamValue("LINKSPEED");
      if (linkSpeed == 400)
        mosaic->setSpeedMode(Mosaic::RCV_RATE_400);
      else if (linkSpeed == 600)
        mosaic->setSpeedMode(Mosaic::RCV_RATE_600);
      else if (linkSpeed == 1200)
        mosaic->setSpeedMode(Mosaic::RCV_RATE_1200);
    }
  }
}
