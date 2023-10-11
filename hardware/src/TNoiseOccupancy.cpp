#include <string.h>
#include <string>

#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "TNoiseOccupancy.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
// #include "TReadoutBoardRU.h"

TNoiseOccupancy::TNoiseOccupancy(TScanConfig *config, std::vector<TAlpide *> chips,
                                 std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                                 std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TDataTaking(config, chips, hics, boards, histoQue, aMutex)
{
  CreateScanParameters();

  m_parameters->backBias = m_config->GetBackBias();
  m_pulse                = false;
  m_pulseLength          = 0;

  ((TNoiseParameters *)m_parameters)->nTriggers = config->GetParamValue("NTRIG");
  ;
  ((TNoiseParameters *)m_parameters)->isMasked = config->GetIsMasked();

  SetName();
}


void TNoiseOccupancy::SetName()
{
  if (((TNoiseParameters *)m_parameters)->isMasked) {
    sprintf(m_name, "Noise Occupancy %.1f V, masked", m_parameters->backBias);
  }
  else {
    sprintf(m_name, "Noise Occupancy %.1f V", m_parameters->backBias);
  }
}


bool TNoiseOccupancy::SetParameters(TScanParameters *pars)
{
  TNoiseParameters *nPars = dynamic_cast<TNoiseParameters *>(pars);
  if (nPars) {
    std::cout << "TNoiseOccupancy: Updating parameters" << std::endl;
    ((TNoiseParameters *)m_parameters)->backBias  = nPars->backBias;
    ((TNoiseParameters *)m_parameters)->nTriggers = nPars->nTriggers;
    ((TNoiseParameters *)m_parameters)->isMasked  = nPars->isMasked;
    CalculateTrains();
    SetName();
    return true;
  }
  else {
    std::cout << "TNoiseOccupancy::SetParameters: Error, bad parameter type, doing nothing"
              << std::endl;
    return false;
  }
}


// TODO: save number of masked pixels (return value of ApplyMask)
void TNoiseOccupancy::ConfigureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);
  ConfigureFromu(chip);
  ConfigureMask(chip, 0);
  AlpideConfig::ApplyMask(chip, true);
  AlpideConfig::ConfigureCMU(chip);
}

void TNoiseOccupancy::ConfigureMask(TAlpide *chip, std::vector<TPixHit> *MaskedPixels)
{
  AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_MASK, false);
  AlpideConfig::WritePixRegAll(chip, Alpide::PIXREG_SELECT, false);
  AlpideConfig::EnableDoubleColumns(chip);
}

THisto TNoiseOccupancy::CreateHisto()
{
  THisto histo("HitmapHisto", "HitmapHisto", 1024, 0, 1023, 512, 0, 511);
  return histo;
}

void TNoiseOccupancy::Init()
{
  // update mask information
  ((TNoiseParameters *)m_parameters)->isMasked = m_config->GetIsMasked();
  SetName();

  TDataTaking::Init();
  m_running = true;
}
