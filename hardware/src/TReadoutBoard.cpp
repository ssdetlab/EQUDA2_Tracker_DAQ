#include "TReadoutBoard.h"

TReadoutBoard::TReadoutBoard(TBoardConfig *config)
{
  fNChips = 0;
  // fChipPositions (0);
  fBoardConfig = config;
}

int TReadoutBoard::AddChip(uint8_t chipId, int controlInterface, int receiver, TAlpide *chipPtr)
{

  TChipPos newChip;
  newChip.chipId           = chipId;
  newChip.controlInterface = controlInterface;
  newChip.receiver         = receiver;
  newChip.enabled          = chipPtr->GetConfig()->IsEnabled();
  // true; // create chip positions by default enabled?
  newChip.alpidePtr = chipPtr;

  fChipPositions.push_back(newChip);
  fNChips++;
  return 0;
}

int TReadoutBoard::GetChipById(uint8_t chipId)
{
  for (unsigned int i = 0; i < fChipPositions.size(); ++i) {
    if (fChipPositions.at(i).chipId == chipId) return i;
  }
  return -1; // throw exception, non existing chip
}
int TReadoutBoard::GetChipById(TAlpide *chipPtr)
{
  for (unsigned int i = 0; i < fChipPositions.size(); ++i) {
    if (fChipPositions.at(i).alpidePtr == chipPtr) return i;
  }
  return -1; // throw exception, non existing chip
}

int TReadoutBoard::GetControlInterface(uint8_t chipId)
{
  int chip = GetChipById(chipId);
  if (chip > -1) return fChipPositions.at(chip).controlInterface;
  return -1;
}
int TReadoutBoard::GetControlInterface(TAlpide *chipPtr)
{
  int chip = GetChipById(chipPtr);
  if (chip > -1) return fChipPositions.at(chip).controlInterface;
  return -1;
}

int TReadoutBoard::GetReceiver(uint8_t chipId)
{
  int chip = GetChipById(chipId);
  if (chip > -1) return fChipPositions.at(chip).receiver;

  return -1;
}
int TReadoutBoard::GetReceiver(TAlpide *chipPtr)
{
  int chip = GetChipById(chipPtr);
  if (chip > -1) return fChipPositions.at(chip).receiver;

  return -1;
}

void TReadoutBoard::SetControlInterface(uint8_t chipId, int controlInterface)
{
  int chip = GetChipById(chipId);
  if (chip > -1) fChipPositions.at(chip).controlInterface = controlInterface;
}
void TReadoutBoard::SetControlInterface(TAlpide *chipPtr, int controlInterface)
{
  int chip = GetChipById(chipPtr);
  if (chip > -1) fChipPositions.at(chip).controlInterface = controlInterface;
}

void TReadoutBoard::SetReceiver(uint8_t chipId, int receiver)
{
  int chip = GetChipById(chipId);
  if (chip > -1) fChipPositions.at(chip).receiver = receiver;
}
void TReadoutBoard::SetReceiver(TAlpide *chipPtr, int receiver)
{
  int chip = GetChipById(chipPtr);
  if (chip > -1) fChipPositions.at(chip).receiver = receiver;
}

void TReadoutBoard::SetChipEnable(uint8_t chipId, bool Enable)
{
  int chip = GetChipById(chipId);
  if (chip > -1) fChipPositions.at(chip).enabled = Enable;
}
void TReadoutBoard::SetChipEnable(TAlpide *chipPtr, bool Enable)
{
  int chip = GetChipById(chipPtr);
  if (chip > -1) fChipPositions.at(chip).enabled = Enable;
}
