#include "AlpideDebug.h"
#include "TAlpide.h"
#include <iostream>

// TODO: decode remaining debug streams

bool AlpideDebug::ReadStream(TAlpide *chip, TRegister reg, uint16_t *stream, int len,
                             uint16_t Header)
{
  for (int i = 0; i < len; i++) {
    chip->ReadRegister(reg, stream[i]);
  }
  if (stream[0] != Header) {
    std::cout << "Wrong header value 0x" << std::hex << stream[0] << std::dec << std::endl;
    return false;
  }
  return true;
}

bool AlpideDebug::GetBMUDebugStream(TAlpide *chip, TBMUDebugStream &stream)
{
  uint16_t streamData[2];

  if (!ReadStream(chip, REG_BMU_DEBUG, streamData, 2, 0xDEBB)) return false;

  stream.BusyRequestSM   = (streamData[1] >> 14) & 0x3;
  stream.BusyGeneratorSM = (streamData[1] >> 13) & 0x1;
  stream.BusyInState     = ((streamData[1] & 0x1000) != 0);
  stream.SEUErrorOR      = ((streamData[1] & 0x800) != 0);
  return true;
}

bool AlpideDebug::GetDMUDebugStream(TAlpide *chip, TDMUDebugStream &stream)
{
  uint16_t streamData[4];

  if (!ReadStream(chip, REG_DMU_DEBUG, streamData, 4, 0xDEBD)) return false;

  for (int i = 0; i < 2; i++) {
    stream.DataFIFOReadPointer[i]  = (streamData[1] >> (6 + (i * 4))) & 0x3;
    stream.DataFIFOWritePointer[i] = (streamData[1] >> (4 + (i * 4))) & 0x3;
  }
  stream.BusyFIFOReadPointer       = (streamData[1] >> 2) & 0x3;
  stream.BusyFIFOWritePointer      = (streamData[1]) & 0x3;
  stream.LocalBusValue             = (streamData[2] >> 8) & 0xff;
  stream.LocalBusOENGenSM          = (streamData[2] >> 7) & 0x1;
  stream.MuxFIFOCtrlSM             = (streamData[2] >> 4) & 0x7;
  stream.CtrlWordDecoderLocalBusSM = (streamData[2] >> 2) & 0x3;
  stream.TokenGrantorSM            = streamData[2] & 0x3;
  stream.SendCommaSM               = (streamData[3] >> 14) & 0x3;
  stream.DataPackingSM             = (streamData[3] >> 11) & 0x7;
  stream.SEUErrorOR                = ((streamData[3] & 0x400) != 0);
  stream.BusyMismatchError         = ((streamData[3] & 0x200) != 0);
  stream.BusyFIFOError             = ((streamData[3] & 0x100) != 0);
  stream.DataFIFOError             = ((streamData[3] & 0x80) != 0);

  return true;
}

bool AlpideDebug::GetTRUDebugStream(TAlpide *chip, TTRUDebugStream &stream)
{
  uint16_t streamData[5];

  if (!ReadStream(chip, REG_TRU_DEBUG, streamData, 5, 0xDEB7)) return false;

  return true;
}

bool AlpideDebug::GetRRUDebugSteam(TAlpide *chip, TRRUDebugStream &stream)
{
  uint16_t streamData[65];

  if (!ReadStream(chip, REG_RRU_DEBUG, streamData, 65, 0xDEB8)) return false;

  return true;
}

bool AlpideDebug::GetFromuDebugStream(TAlpide *chip, TFromuDebugStream &stream)
{
  uint16_t streamData[8];

  if (!ReadStream(chip, REG_FROMU_DEBUG, streamData, 8, 0xDEBF)) return false;

  stream.TriggerCounter  = streamData[1];
  stream.StrobeCounter   = streamData[2];
  stream.FrameCounter    = streamData[3];
  stream.ReadoutCounter  = streamData[4];
  stream.BunchCounter    = (streamData[5] >> 4) & 0xfff;
  stream.StrobeManagerSM = (streamData[5] >> 1) & 0x7;
  stream.FlushValue      = streamData[5] & 0x1;
  stream.WriterSM        = (streamData[6] >> 11) & 0x1f;
  stream.ReaderSM        = (streamData[6] >> 6) & 0x1f;
  stream.PRSTSM          = streamData[6] & 0x3f;
  stream.StrobeValue     = (streamData[7] >> 13) & 0x7;
  stream.MemselValue     = (streamData[7] >> 10) & 0x7;
  stream.BusyManagerSM   = (streamData[7] >> 8) & 0x3;
  stream.EventInMEB      = (streamData[7] >> 5) & 0x7;
  return true;
}

bool AlpideDebug::GetADCDebugStream(TAlpide *chip, TADCDebugStream &stream)
{
  uint16_t streamData[4];

  if (!ReadStream(chip, REG_BMU_DEBUG, streamData, 4, 0xDEBA)) return false;

  return true;
}
