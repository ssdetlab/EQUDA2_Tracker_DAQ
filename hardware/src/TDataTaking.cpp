#include <chrono>
#include <stdexcept>
#include <string.h>
#include <string>
#include <thread>

#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include "TDataTaking.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
// #include "TReadoutBoardRU.h"

TDataTaking::TDataTaking(TScanConfig *config, std::vector<TAlpide *> chips,
                         std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                         std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  m_nTriggers = m_config->GetParamValue("NTRIG");

  // divide triggers in trains
  CalculateTrains();
  m_start[0] = 0;
  m_step[0]  = 1;

  m_start[1] = 0;
  m_step[1]  = 1;
  m_stop[1]  = 1;

  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = 1;
}


void TDataTaking::CalculateTrains()
{
  if (m_nTriggers % kTrigPerTrain == 0) {
    m_nLast   = kTrigPerTrain;
    m_nTrains = m_nTriggers / kTrigPerTrain;
  }
  else {
    m_nLast   = m_nTriggers % kTrigPerTrain;
    m_nTrains = m_nTriggers / kTrigPerTrain + 1;
  }
  m_stop[0] = m_nTrains;
}


void TDataTaking::ConfigureFromu(TAlpide *chip)
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
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, m_pulseLength); // fromu pulsing 2: pulse length
}

void TDataTaking::ConfigureBoard(TReadoutBoard *board)
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
    board->SetTriggerConfig(m_pulse, true, board->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                            board->GetConfig()->GetParamValue("PULSEDELAY"));
    board->SetTriggerSource(trigInt);
  }
}

THisto TDataTaking::CreateHisto()
{
  THisto histo("HitmapHisto", "HitmapHisto", 1024, 0, 1023, 512, 0, 511);
  return histo;
}

void TDataTaking::Init()
{
  InitBase(false);

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

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode(Alpide::OPCODE_RORST);
    m_boards.at(i)->StartRun();
  }

  for (const auto &rBoard : m_boards) {
    if (TReadoutBoardMOSAIC *rMOSAIC = dynamic_cast<TReadoutBoardMOSAIC *>(rBoard)) {
      rMOSAIC->ResetAllReceivers();
    }
  }

  TScan::SaveStartConditions();
}

// check which HIC caused the timeout, i.e. did not send enough events
// called only in case a timeout occurs
void TDataTaking::FindTimeoutHics(int iboard, int *triggerCounts, int nTriggers)
{
  for (unsigned int iHic = 0; iHic < m_hics.size(); iHic++) {
    bool isOnBoard = false;
    int  nTrigs    = 0;
    for (unsigned int iRcv = 0; iRcv < MAX_MOSAICTRANRECV; iRcv++) {
      if (m_hics.at(iHic)->ContainsReceiver(iboard, iRcv)) {
        isOnBoard = true;
        nTrigs += triggerCounts[iRcv];
      }
    }
    // HIC is connected to this readout board AND did not send enough events
    if ((isOnBoard) && (nTrigs < nTriggers * (int)(m_hics.at(iHic)->GetNEnabledChips()))) {
      m_errorCounts.at(m_hics.at(iHic)->GetDbId()).nTimeout++;
    }
  }
}

void TDataTaking::ReadEventData(std::vector<TPixHit> *Hits, int iboard, int nTriggers)
{
  unsigned char buffer[MAX_EVENT_SIZE];
  int           n_bytes_data, n_bytes_header, n_bytes_trailer;
  int           itrg = 0, trials = 0;
  TBoardHeader  boardInfo;
  int           nTrigPerHic[MAX_MOSAICTRANRECV];

  for (unsigned int i = 0; i < MAX_MOSAICTRANRECV; i++) {
    nTrigPerHic[i] = 0;
  }

  while (itrg < nTriggers * m_enabled[iboard]) {
    if (m_boards.at(iboard)->ReadEventData(n_bytes_data, buffer) <=
        0) { // no event available in buffer yet, wait a bit
      usleep(100);
      trials++;
      if (trials == 3) {
        std::cout << "Board " << iboard << ": reached 3 timeouts, giving up on this event"
                  << std::endl;
        std::cout << "  Events per receiver (receiver order): ";
        for (unsigned int i = 0; i < MAX_MOSAICTRANRECV; i++) {
          std::cout << nTrigPerHic[i] << " ";
        }
        std::cout << std::endl;
        std::cout << "  Trigger counters per chip (chip order): ";
        for (unsigned int i = 0; i < m_chips.size(); i++) {
          if (m_chips.at(i)->GetConfig()->IsEnabled()) {
            uint16_t value;
            m_chips.at(i)->ReadRegister(Alpide::REG_FROMU_STATUS1, value);
            std::cout << m_chips.at(i)->GetConfig()->GetChipId() << ": " << value << " ";
          }
          else {
            std::cout << m_chips.at(i)->GetConfig()->GetChipId() << ": disabled ";
          }
        }
        std::cout << std::endl;
        itrg = nTriggers * m_enabled[iboard];
        FindTimeoutHics(iboard, nTrigPerHic, nTriggers);
        m_errorCount.nTimeout++;
        if (m_errorCount.nTimeout > m_config->GetParamValue("MAXTIMEOUT")) {
          throw std::runtime_error("Maximum number of timouts reached. Aborting scan.");
        }
        trials = 0;
      }
      continue;
    }
    else {
      BoardDecoder::DecodeEvent(m_boards.at(iboard)->GetConfig()->GetBoardType(), buffer,
                                n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
      // decode Chip event
      if (boardInfo.decoder10b8bError) {
        m_errorCount.n8b10b++;
        if (FindHIC(iboard, boardInfo.channel).compare("None") != 0) {
          m_errorCounts.at(FindHIC(iboard, boardInfo.channel)).n8b10b++;
        }
      }
      if (boardInfo.eventOverSizeError) {
        std::cout << "Found oversized event, truncated in MOSAIC" << std::endl;
        m_errorCount.nOversizeEvent++;
      }
      int n_bytes_chipevent = n_bytes_data - n_bytes_header; //-n_bytes_trailer;
      if (boardInfo.eoeCount < 2) n_bytes_chipevent -= n_bytes_trailer;
      bool dataIntegrity = false;
      try {
        int flags;
        dataIntegrity = AlpideDecoder::DecodeEvent(
            buffer + n_bytes_header, n_bytes_chipevent, Hits, iboard, boardInfo.channel,
            m_errorCount.nPrioEncoder, flags, m_config->GetParamValue("MAXHITS"), &m_stuck);
      }
      catch (const std::runtime_error &e) {
        std::cout << "Exception " << e.what() << " after " << itrg << " Triggers (this point)"
                  << std::endl;
        DumpHitInformation(Hits);
        throw e;
      }

      if (!dataIntegrity) {
        std::cout << "Found bad event, length = " << n_bytes_chipevent << std::endl;
        m_errorCount.nCorruptEvent++;
        if (FindHIC(iboard, boardInfo.channel).compare("None") != 0) {
          m_errorCounts.at(FindHIC(iboard, boardInfo.channel)).nCorruptEvent++;
        }
      }
      nTrigPerHic[boardInfo.channel]++;
      itrg++;
    }
  }
}

void TDataTaking::FillHistos(std::vector<TPixHit> *Hits, int board)
{
  common::TChipIndex idx;
  idx.boardIndex = board;

  for (unsigned int i = 0; i < Hits->size(); i++) {
    idx.dataReceiver = Hits->at(i).channel;
    idx.chipId       = Hits->at(i).chipId;

    int col = Hits->at(i).region * 32 + Hits->at(i).dcol * 2;
    int leftRight =
        ((((Hits->at(i).address % 4) == 1) || ((Hits->at(i).address % 4) == 2)) ? 1 : 0);
    col += leftRight;
    m_histo->Incr(idx, col, Hits->at(i).address / 2);
  }
}

void TDataTaking::LoopEnd(int loopIndex)
{
  if (loopIndex == 0) {
    while (!(m_mutex->try_lock()))
      ;
    m_histoQue->push_back(*m_histo);
    m_mutex->unlock();
    m_histo->Clear();
  }
}

void TDataTaking::Execute()
{
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

  // always send kTrigPerTrain triggers, except for last train (in case total nTrig is no multiple
  // of kTrigPerTrain)
  int nTriggers = (m_value[0] == m_stop[0] - 1) ? m_nLast : kTrigPerTrain;

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    m_boards.at(iboard)->Trigger(nTriggers);
  }
  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard++) {
    Hits->clear();
    ReadEventData(Hits, iboard, nTriggers);
    FillHistos(Hits, iboard);
  }
  delete Hits;
}

void TDataTaking::Terminate()
{
  TScan::Terminate();
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
}
