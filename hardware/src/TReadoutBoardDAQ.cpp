#include <math.h>
#include <string.h>
#include <string>
#include <thread>
#include <unistd.h>

#include "TAlpide.h"
#include "TReadoutBoardDAQ.h"
#include "USB.h"

// constructor
TReadoutBoardDAQ::TReadoutBoardDAQ(libusb_device *ADevice, TBoardConfigDAQ *config)
    : TUSBBoard(ADevice), TReadoutBoard(config)
{
  fTrigCnt            = 0;
  fEvtCnt             = 0;
  fDiffTrigEvtCnt     = 0;
  fNTriggersTotal     = 0;
  fMaxDiffTrigEvtCnt  = MAX_DIFF_TRIG_EVT_CNT;
  fMaxEventBufferSize = MAX_EVT_BUFFSIZE;
  fMaxNTriggersTrain  = MAX_NTRIG_TRAIN;

  fIsTriggerThreadRunning  = false;
  fIsReadDataThreadRunning = false;

  fBoardConfigDAQ = config;

  // WriteDelays();

  // write default config to all registers
  WriteCMUModuleConfigRegisters();
  WriteReadoutModuleConfigRegisters();
  WriteADCModuleConfigRegisters();
  WriteTriggerModuleConfigRegisters();
  WriteResetModuleConfigRegisters();
  WriteSoftResetModuleConfigRegisters();

  // WriteRegister (0x200, 0x1801);
}

// destructor
TReadoutBoardDAQ::~TReadoutBoardDAQ()
{
  // join threads

  // fThreadTrigger.join();
  // fThreadReadData.join();
  // std::cout << "joined threads.." << std::endl;

  std::cout << "Powering off chip" << std::endl;
  PowerOff();
}

//---------------------------------------------------------
// general methods of TReadoutBoard
//---------------------------------------------------------

int TReadoutBoardDAQ::ReadRegister(uint16_t address, uint32_t &value)
{
  unsigned char data_buf[DAQBOARD_WORD_SIZE * 2];
  uint32_t      headerword = 0;
  int           err;

  err = SendWord(
      (uint32_t)address +
      (1 << (DAQBOARD_REG_ADDR_SIZE + DAQBOARD_MODULE_ADDR_SIZE))); // add 1 bit for read access
  if (err < 0) return -1;
  err = ReceiveData(ENDPOINT_READ_REG, data_buf, DAQBOARD_WORD_SIZE * 2);

  if (err < 0) return -1;

  value = 0;

  for (int i = 0; i < DAQBOARD_WORD_SIZE; i++) {
    headerword += (data_buf[i] << (8 * i));                 // bytes 0 ... 3 are header
    value += (data_buf[i + DAQBOARD_WORD_SIZE] << (8 * i)); // bytes 4 ... 7 are data
  }
  return 0;
}

int TReadoutBoardDAQ::WriteRegister(uint16_t address, uint32_t value)
{
  // std::cout << "[FPGA] ADDRESS: " << std::hex <<  address << " VALUE " << value << std::dec <<
  // std::endl;

  int err;
  err = SendWord((uint32_t)address);

  if (err < 0) return -1; // add exceptions

  err = SendWord(value);
  if (err < 0) return -1;
  err = ReadAcknowledge();
  if (err < 0) return -1;

  return 0;
}

int TReadoutBoardDAQ::WriteChipRegister(uint16_t address, uint16_t value, TAlpide *chipPtr)
{
  uint8_t chipId = chipPtr->GetConfig()->GetChipId();
  // int err;
  uint32_t address32  = (uint32_t)address;
  uint32_t chipId32   = (uint32_t)chipId;
  uint32_t newAddress = (address32 << 16) | (chipId32 << 8) | Alpide::OPCODE_WROP;

  // std::cout << "[CHIP] ADDRESS: " << std::hex <<  newAddress << " VALUE " << value << std::dec <<
  // std::endl;

  // err = WriteRegister (CMU_DATA + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), (uint32_t) value);
  // if(err < 0) return -1;
  // err = WriteRegister (CMU_INSTR + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), newAddress);
  // if(err < 0) return -1;

  uint32_t command[4];
  int      err;

  // std::cout << "[ CHIP ] ADDRESS: " << std::hex << address << " (" << newAddress << ") " << "
  // VALUE " << value << std::dec << std::endl;
  command[0] = CMU_DATA + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE);
  command[1] = value;
  command[2] = CMU_INSTR + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE);
  command[3] = newAddress;
  SendWord((uint32_t)command[0]);
  SendWord((uint32_t)command[1]);
  // err=ReadAck();
  // if(err==false) return -1;
  err = ReadAcknowledge();
  if (err < 0) return -1;
  SendWord((uint32_t)command[2]);
  SendWord((uint32_t)command[3]);
  // err=ReadAck();
  // if(err==false) return -1;
  err = ReadAcknowledge();
  if (err < 0) return -1;

  return 1;

  // return 0;
}

int TReadoutBoardDAQ::ReadChipRegister(uint16_t address, uint16_t &value, TAlpide *chipPtr)
{
  uint8_t  chipId = chipPtr->GetConfig()->GetChipId();
  int      err;
  uint32_t value32;
  uint32_t address32  = (uint32_t)address;
  uint32_t chipId32   = (uint32_t)chipId;
  uint32_t newAddress = (address32 << 16) | (chipId32 << 8) | Alpide::OPCODE_RDOP;

  err = WriteRegister(CMU_INSTR + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), newAddress);
  if (err < 0) return -1;
  err = ReadRegister(CMU_DATA + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), value32);
  if (err < 0) return -1;

  value = (value32 >> 8) & 0xffff;

  // std::cout << std::hex << value32 << std::dec << std::endl;

  uint8_t received_chipid = value32 & 0xff;
  if (received_chipid != chipId) {
    std::cout << "WARNING: received chipID (" << (int)received_chipid
              << ") does not match with configuration in DAQboard (" << (int)chipId << ")"
              << std::endl;
    return -1;
  }
  else {
    return 0;
  }
}

int TReadoutBoardDAQ::SendOpCode(Alpide::TOpCode OpCode)
{
  return WriteRegister(CMU_INSTR + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), (int)OpCode);
}

int TReadoutBoardDAQ::SendCommand(Alpide::TCommand Command, TAlpide *chipPtr)
{
  return WriteChipRegister(Alpide::REG_COMMAND, Command, chipPtr);
}

int TReadoutBoardDAQ::SetTriggerConfig(bool enablePulse, bool enableTrigger, int triggerDelay,
                                       int pulseDelay)
{
  fBoardConfigDAQ->SetTriggerEnable(enableTrigger); // enableTrigger? DAQboard trigger disabled only
                                                    // if fBoardConfigDAQ.TriggerMode==0..
  fBoardConfigDAQ->SetPulseEnable(enablePulse);     // enablePulse on DAQboard??

  fBoardConfigDAQ->SetTriggerDelay((int32_t)triggerDelay);
  fBoardConfigDAQ->SetStrobeDelay(
      (int32_t)triggerDelay); // equivalent to trigger delay on DAQboard..
  WriteTriggerModuleConfigRegisters();

  fBoardConfigDAQ->SetPulseDelay(pulseDelay); // delay between pulse and strobe/trigger; if
                                              // fStrobePulseSeq is set correctly (to 2)
  WriteResetModuleConfigRegisters();

  return 0;
}

void TReadoutBoardDAQ::SetTriggerSource(TTriggerSource triggerSource)
{
  if (triggerSource == trigInt) {
    fBoardConfigDAQ->SetTriggerMode(1);
    WriteTriggerModuleConfigRegisters();
  }
  else if (triggerSource == trigExt) {
    fBoardConfigDAQ->SetTriggerMode(2);
    WriteTriggerModuleConfigRegisters();
  }
  else {
    std::cerr << "!!! Trigger source not known, doing nothing !!!" << std::endl;
  }
}

// trigger function to be executed in thread fThreadTrigger
void TReadoutBoardDAQ::DAQTrigger()
{
  // std::cout << "-> in DAQTrigger function now" << std::endl;
  fMtx.lock();
  fIsTriggerThreadRunning = true;
  fMtx.unlock();

  fStatusTrigger              = 0;
  unsigned int evtbuffer_size = 0;

  if (fBoardConfig->GetTriggerEnable() && !fBoardConfigDAQ->GetPulseEnable()) { // TRIGGERING
    std::cout << "Number of triggers: " << fNTriggersTotal << std::endl;
    int nTriggerTrains = fNTriggersTotal / fMaxNTriggersTrain;
    std::cout << " --> " << nTriggerTrains << " trigger trains with " << fMaxNTriggersTrain
              << " triggers going to be launched" << std::endl;
    int nTriggersLeft = fNTriggersTotal % fMaxNTriggersTrain; // TODO: nicer solution?
    std::cout << " --> then " << nTriggersLeft << " triggers left to be launched" << std::endl;

    fBoardConfigDAQ->SetNTriggers(fMaxNTriggersTrain);
    WriteTriggerModuleConfigRegisters();

    for (int itrain = 0; itrain < nTriggerTrains; itrain++) {
      fMtx.lock();
      evtbuffer_size = fEventBuffer.size();
      fMtx.unlock();
      if (evtbuffer_size < fMaxEventBufferSize) {
        std::cout << "train " << itrain << std::endl;

        StartTrigger(); // start trigger train;
                        // sleep for enough time so that stoptrigger sent after last trigger..
        // int sleep_time = fBoardConfigDAQ->GetStrobeDelay()*0.25+375; // TODO: check why this is
        // not working.. but longer a wait time is needed..
        int sleep_time = fBoardConfigDAQ->GetStrobeDelay();
        usleep(sleep_time);
        StopTrigger();

        fTrigCnt += fMaxNTriggersTrain;
      }
      else {
        std::cout << "Maximum event buffer size reached before reaching nTriggers; stop here!"
                  << std::endl;
        std::cout << "    -> Number of triggers performed: " << fTrigCnt << std::endl;
        fStatusTrigger = -1;
        return;
      }
    }

    if (nTriggersLeft != 0) { // TODO: nicer solution?
      fMtx.lock();
      evtbuffer_size = fEventBuffer.size();
      fMtx.unlock();
      if (evtbuffer_size < fMaxEventBufferSize) {
        fBoardConfigDAQ->SetNTriggers(nTriggersLeft);
        WriteTriggerModuleConfigRegisters();

        StartTrigger(); // start trigger train;
                        // sleep for enough time so that stoptrigger sent after last trigger..
        // int sleep_time = fBoardConfigDAQ->GetStrobeDelay()*0.025+375; // TODO: check why this is
        // not working.. but longer a wait time is needed..
        int sleep_time = fBoardConfigDAQ->GetStrobeDelay();
        usleep(sleep_time);
        StopTrigger();

        fTrigCnt += nTriggersLeft;
      }
      else {
        std::cout << "Maximum event buffer size reached before reaching nTriggers; stop here!"
                  << std::endl;
        std::cout << "    -> Number of triggers performed: " << fTrigCnt << std::endl;
        fStatusTrigger = -1;
        return;
      }
    }
  }
  else if (!fBoardConfig->GetTriggerEnable() && fBoardConfigDAQ->GetPulseEnable()) { // just PULSING
    // fBoardConfigDAQ->SetNTriggers(100); // TODO: number of triggers to be launched with
    // StartTrigger command? if set trigger src to external not trigger sent at all?
    // WriteTriggerModuleConfigRegisters();

    // StartTrigger(); // TODO: needed so that DAQboard reads events? but does it also send trigger?
    for (fTrigCnt = 0; fTrigCnt < fNTriggersTotal; fTrigCnt++) {
      fMtx.lock();
      evtbuffer_size = fEventBuffer.size();
      fMtx.unlock();
      if (evtbuffer_size < fMaxEventBufferSize) {
        // SendOpCode (0x78); // send PULSE to chip
        StartTrigger();
        WriteRegister((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_PULSE,
                      13); // write anything to pulse register to trigger pulse
        StopTrigger();
        // std::cout << "Pulse " << fTrigCnt << " sent" << std::endl;
      }
      else {
        std::cout
            << "Maximum event buffer size reached before reaching nTriggers (Pulses); stop here!"
            << std::endl;
        std::cout << "    -> Number of pulses performed: " << fTrigCnt << std::endl;
        fStatusTrigger = -1;
        return;
      }
    }
    // StopTrigger();
  }
  else {
    std::cout << "Pulse and Trigger either both disabled or enabled, please select one of the two!"
              << std::endl;
    return;
  }

  // DEBUG read monitoring registers
  // ReadMonitorRegisters();

  // send EndOfRun command; resets counters in header
  // WriteRegister(0x201, 0x1);

  fMtx.lock();
  fIsTriggerThreadRunning = false;
  fMtx.unlock();
  fStatusTrigger = 1; // exited successfully
}

// readdata function to be executed in thread fThreadReadData
void TReadoutBoardDAQ::DAQReadData()
{
  fMtx.lock();
  // std::cout << " -> in DAQReadData now" << std::endl;
  fIsReadDataThreadRunning = true;
  fMtx.unlock();

  const int     max_length_buf = 1024 * 1000; // length needed at ITHR=10 ~5000!!!
  const int     length_buf     = 1024;        // length needed at ITHR=10 ~5000!!!
  unsigned char data_buf[max_length_buf];     // TODO large enough?
  int           evt_length = 0;
  int           tmp_error  = 0;

  std::vector<unsigned char> data_evt(max_length_buf);
  // std::copy(my_deque.begin(), my_deque.end(), std::back_inserter(my_vector));

  if (fBoardConfigDAQ->GetPktBasedROEnable() == false) { // event based
    // std::cout << " --> event based readout" << std::endl;
    while (fEvtCnt < fNTriggersTotal) { // no stop-trigger marker with event-based readout
      data_evt.clear();
      evt_length = ReceiveData(ENDPOINT_READ_DATA, data_buf, max_length_buf, &tmp_error);
      // std::cout << "Received " << *length << " bytes" << std::endl;

      if (tmp_error == -7) { // USB timeout
        std::cout << "timeout" << std::endl;
        fStatusReadData = -2;
        return; // TODO: this has to be handled better with
                // return -2;
      }
      else if (evt_length < 1) {
        std::cout << std::endl;
        std::cout << "ERROR, received data returned with " << evt_length << std::endl;
        std::cout << std::endl;
        fStatusReadData = -1;
        return;
      }
      else if (evt_length < (BoardDecoder::GetDAQEventHeaderLength(
                                 fFirmwareVersion, fBoardConfigDAQ->GetHeaderType()) +
                             BoardDecoder::GetDAQEventTrailerLength() + 4)) {
        std::cout << std::endl;
        std::cout << "WARNING, received too small event: " << evt_length
                  << " instead of expected >= "
                  << (BoardDecoder::GetDAQEventHeaderLength(fFirmwareVersion,
                                                            fBoardConfigDAQ->GetHeaderType()) +
                      BoardDecoder::GetDAQEventTrailerLength() + 4)
                  << std::endl;
        std::cout << std::endl;
      }
      else {
        for (int i = 0; i < evt_length; i++) {
          data_evt.push_back(data_buf[i]);
        }
        fMtx.lock();
        fEventBuffer.push_back(data_evt);
        fEvtCnt++;
        fMtx.unlock();

        // DEBUG output
        // fMtx.lock();
        // std::cout << "read evt " << fEvtCnt << std::endl;
        // std::cout << "\t data_evt size: " << data_evt.size() << std::endl;
        // std::cout << "\t EventBuffer length: " << fEventBuffer.size() << std::endl;
        // for (int iByte=0; iByte<data_evt.size(); ++iByte) {
        //  std::cout << std::hex << (int)data_evt[iByte] << std::dec;
        //}
        // std::cout << std::endl;
        // fMtx.unlock();
      }
    }
  }
  else if (fBoardConfigDAQ->GetPktBasedROEnable() == true) { // packet based
    // std::cout << " --> packet based readout" << std::endl;
    // each packet may contain more or less than one event. the following code split raw data into
    // events and writes it into fEventBuffer
    evt_length                               = 0; // no data read so far
    bool          foundMagicWord             = false;
    const int     nMagicWords                = 5;
    unsigned char magicWords[nMagicWords][4] = {
        {0xbf, 0xbf, 0xbf, 0xbf}, // pALPIDE-2/3 event trailer
        {0xaf, 0xaf, 0xaf, 0xaf}, // pALPIDE-2/3 event trailer for truncated event
        {0xfe, 0xeb, 0xfe, 0xeb}, // stop-trigger marker in the packet-based readout mode
        {0xef, 0xeb, 0xef,
         0xeb}, // stop-trigger marker in the packet-based readout mode (inconsistent timestamp and
                // data fifo)
        {0xfe, 0xab, 0xfe, 0xab}}; // pALPIDE-1 event trailer
    bool         timeout       = false;
    int          packet_length = 0;
    unsigned int length_tmp    = 0;

    while (fEvtCnt <= fNTriggersTotal ||
           fRawBuffer.size() !=
               0) { // at fEvtCnt==fNTriggersTotal it should find stop-trigger marker

      foundMagicWord = false;
      data_evt.clear();
      timeout    = false;
      length_tmp = 0;

      do {
        while (length_tmp + 4 <= fRawBuffer.size() &&
               !foundMagicWord) { // this is executed if fRawBuffer contains data, otherwise it
                                  // jumps to next if and reads data..

          // DEBUG OUTPUT
          // std::cout << "length_tmp: " << length_tmp << std::endl;
          // std::cout << std::hex << (int)fRawBuffer[length_tmp+0] << (int)fRawBuffer[length_tmp+1]
          // << (int)fRawBuffer[length_tmp+2] << (int)fRawBuffer[length_tmp+3] << std::dec;
          // std::cout << "\t";

          for (int iMagicWord = 0; iMagicWord < nMagicWords; ++iMagicWord) {
            if (magicWords[iMagicWord][0] == fRawBuffer[length_tmp + 0] &&
                magicWords[iMagicWord][1] == fRawBuffer[length_tmp + 1] &&
                magicWords[iMagicWord][2] == fRawBuffer[length_tmp + 2] &&
                magicWords[iMagicWord][3] == fRawBuffer[length_tmp + 3]) {

              // if found magicword write data/event to fEventBufffer
              foundMagicWord = true;
              switch (iMagicWord) {
              case 1:
                std::cerr << "Truncated pALPIDE-2/3 event found!" << std::endl;
                break;
              case 3:
                std::cout << "Inconsistent timestamp and data FIFO detected!" << std::endl;
                break;
              case 2:
                // DEBUG OUTPUT
                // for (int iByte=0; iByte<fRawBuffer.size(); ++iByte) {
                //  std::cout << std::hex << (int)fRawBuffer[iByte] << std::dec;
                //}
                // std::cout << std::endl;
                std::cout << "Stop-trigger marker received." << std::endl;
                data_evt.clear();
                // return -3;
                fStatusReadData = -3;
                return;
                break;
              }
            }
          }
          length_tmp += 4;
        }

        if (!timeout && !foundMagicWord) { // read new data packet here if not a magic word found or
                                           // timeout; this is performed until timeout or full event
                                           // (magicword) achieved.. or error occurs
          packet_length = ReceiveData(ENDPOINT_READ_DATA, data_buf, length_buf, &tmp_error);
          // std::cout << "packet: " << packet_length << std::endl;

          //          if (debug && debug_length) {
          //            *debug = new unsigned char[length];
          //            memcpy(*debug, data_buf, length);
          //            *debug_length = length;
          //          }
          //          if (error) {
          //            *error = tmp_error;
          //          }

          if (tmp_error == -7) { // USB timeout
            timeout = true;

            //#ifdef MYDEBUG
            //          for (int iByte=0; iByte<fRawBuffer.size(); ++iByte) {
            //            std::cout << std::hex << (int)fRawBuffer[iByte] << std::dec;
            //          }
            //          std::cout << std::endl;
            //#endif

            std::cout << "timeout" << std::endl;
            fStatusReadData = -2;
            return;
            // return -2;
          }

          if (packet_length < 1) {
            std::cout << "Error, receive data returned with " << packet_length << std::endl;
            fStatusReadData = -1;
            return;
            // return -1;
          }
          if (packet_length % 4 != 0) {
            std::cout << "Error, received data was not a multiple of 32 bit! Packet length: "
                      << packet_length << " byte" << std::endl;
            fStatusReadData = -1;
            return;
            // return -1;
          }

          for (int i = 0; i < packet_length; i++) {
            fRawBuffer.push_back(data_buf[i]);
          }

          //#if 0
          //        std::cout << "USB RAW (length " << length << "): ";
          //        for (int j=0; j<length; j++)
          //          printf("%02x ", fRawBuffer[j]);
          //        std::cout << std::endl;
          //#endif
        }
      } while (length_tmp < fRawBuffer.size() && !foundMagicWord);

      // arrive here only if
      if (!foundMagicWord) {
        // return -1; // did not achieve to read a full event
        fStatusReadData = -1;
        return;
      }

      evt_length = length_tmp;

      if (evt_length > max_length_buf) {
        evt_length = 0;
        std::cerr << "Event to large (" << evt_length << "Byte) to be read with a buffer of "
                  << max_length_buf << "Byte!" << std::endl;
        // return -1;
        fStatusReadData = -1;
        return;
      }
      else if (evt_length < (BoardDecoder::GetDAQEventHeaderLength(
                                 fFirmwareVersion, fBoardConfigDAQ->GetHeaderType()) +
                             BoardDecoder::GetDAQEventTrailerLength() + 4)) {
        std::cout << std::endl;
        std::cout << "WARNING, received too small event: " << evt_length << std::endl;
        std::cout << std::endl;
      }

      fEvtCnt++;
      // std::cout << "------------------------------------------------------" << std::endl;
      // std::cout << "\t evt: " << fEvtCnt << std::endl;
      // std::cout << "\t evt length: " << evt_length << std::endl;
      // std::cout << "\t RawBuffer length: " << fRawBuffer.size() << std::endl;
      for (int i = 0; i < evt_length; ++i) {
        data_evt.push_back(fRawBuffer.front());
        fRawBuffer.pop_front();
      }
      fMtx.lock();
      fEventBuffer.push_back(data_evt);
      fMtx.unlock();
      // std::cout << "\t data_evt size: " << data_evt.size() << std::endl;
      // std::cout << "\t EventBuffer length: " << fEventBuffer.size() << std::endl;
      // std::cout << "------------------------------------------------------" << std::endl;

      // DEBUG OUTPUT
      // for (int iByte=0; iByte<data_evt.size(); ++iByte) {
      //  std::cout << std::hex << (int)data_evt[iByte] << std::dec;
      //}
      // std::cout << std::endl;

    } // end while fEvtCnt<fNTriggersTotal
  }   // end if PktBasedROEnable

  // check if buffer is empty
  if (fRawBuffer.size() != 0) {
    std::cout << "WARNING: fRawBuffer not empty, but should be at this point!" << std::endl;
    for (unsigned int iByte = 0; iByte < fRawBuffer.size(); ++iByte) {
      std::cout << std::hex << (int)fRawBuffer[iByte] << std::dec;
    }
  }

  fMtx.lock();
  fIsReadDataThreadRunning = false;
  fMtx.unlock();

  // return;
  fStatusReadData = 1; // exited successfully
}

int TReadoutBoardDAQ::Trigger(
    int nTriggers) // open threads for triggering and reading/writing data to queue..
{
  // std::cout << "in Trigger function now" << std::endl;

  // int wait_counter = 0;
  // while ((fIsTriggerThreadRunning || fIsReadDataThreadRunning) && wait_counter<10) { // check if
  // some other threads still running
  //  std::cout << "Trigger or ReadData thread still active, please wait" << std::endl;
  //  usleep(100000);
  //  wait_counter++;
  //}

  fNTriggersTotal = nTriggers;

  fEventBuffer.clear();
  fRawBuffer.clear();
  fTrigCnt = 0;
  fEvtCnt  = 0;

  // launch trigger and readdata in threads:
  // std::cout << "starting threads.." << std::endl;
  if (nTriggers >= 0) {
    fThreadTrigger = std::thread(&TReadoutBoardDAQ::DAQTrigger, this);
  }
  else {
    fTrigCnt        = -nTriggers;
    fNTriggersTotal = -nTriggers;
  }
  // usleep(10000);
  // sleep(1);
  fThreadReadData = std::thread(&TReadoutBoardDAQ::DAQReadData, this);
  // fThreadTrigger  = std::thread ([this] { DAQTrigger(); });
  // fThreadReadData = std::thread ([this] { DAQReadData(); });

  if (nTriggers >= 0) fThreadTrigger.join();
  fThreadReadData.join();
  // std::cout << "joined threads.." << std::endl;

  return 0;
}

int TReadoutBoardDAQ::ReadEventData(
    int &NBytes, unsigned char *Buffer) // provide oldest event in queue and remove it from there
{
  // vector <unsigned char> evt_data = fEventBuffer.front();
  fMtx.lock();
  int n_evts_buf = fEventBuffer.size();
  fMtx.unlock();

  if (n_evts_buf == 0) {
    // std::cout << "No events left in fEventBuffer. Exit." << std::endl;
    return -1;
  }

  fMtx.lock();
  NBytes = fEventBuffer.front().size();
  for (int i = 0; i < NBytes; ++i) {
    Buffer[i] = fEventBuffer.front()[i];
    // std::cout << std::hex << (int)fEventBuffer.front()[i] << std::dec;
  }
  // std::cout << std::endl;
  fEventBuffer.pop_front(); // delete oldest event from deque
  fMtx.unlock();

  // for (int i=0; i<NBytes; ++i) {
  //  std::cout << std::hex << (int)(uint8_t)Buffer[i] << std::dec;
  //}
  // std::cout << std::endl;

  return 1;
}

//---------------------------------------------------------
// methods only for Cagliari DAQ board
//---------------------------------------------------------

// method to send 32 bit words to DAQ board
// FPGA internal registers have 12-bit addres field and 32-bit data payload
int TReadoutBoardDAQ::SendWord(uint32_t value)
{
  unsigned char data_buf[DAQBOARD_WORD_SIZE];

  for (int i = 0; i < DAQBOARD_WORD_SIZE; i++) {
    data_buf[i] = value & 0xff;
    value >>= 8;
  }

  if (SendData(ENDPOINT_WRITE_REG, data_buf, DAQBOARD_WORD_SIZE) != DAQBOARD_WORD_SIZE) return -1;
  return 0;
}

int TReadoutBoardDAQ::ReadAcknowledge()
{
  unsigned char data_buf[2 * DAQBOARD_WORD_SIZE];
  uint32_t      headerword = 0, dataword = 0;
  int           err;

  err = ReceiveData(ENDPOINT_READ_REG, data_buf, 2 * DAQBOARD_WORD_SIZE);

  if (err < 0) return -1;

  for (int i = 0; i < DAQBOARD_WORD_SIZE; i++) {
    headerword += (data_buf[i] << (8 * i));                    // bytes 0 ... 3 are header
    dataword += (data_buf[i + DAQBOARD_WORD_SIZE] << (8 * i)); // bytes 4 ... 7 are data
  }

  return 0;
}

int TReadoutBoardDAQ::CurrentToADC(int current)
{
  float Result = (float)current / 100. * 4096. / 3.3;
  // std::cout << "Current to ADC, Result = " << Result << std::endl;
  return (int)Result;
}

float TReadoutBoardDAQ::ADCToSupplyCurrent(int value)
{
  float Result = (float)value * 3.3 / 4096.; // reference voltage 3.3 V, full range 4096
  Result /= 0.1;                             // 0.1 Ohm resistor
  Result *= 10;                              // / 100 (gain) * 1000 (conversion to mA);
  return Result;
}

float TReadoutBoardDAQ::ADCToDacmonCurrent(int value)
{
  float Result = (float)value * (1e9 * 3.3); // reference voltage 3.3 V, conversion to nA
  Result /= (5100 * 4096 * 6);               // 5.1 kOhm res., ADC-range 4096, amplifier gain 6
  Result /= 10;                              // gain of monitoring buffer
  return Result;
}

float TReadoutBoardDAQ::ADCToTemperature(int AValue)
{
  float Temperature, R;
  float AVDD = 1.8;
  float R2   = 5100;
  float B    = 3900;
  float T0   = 273.15 + 25;
  float R0   = 10000;

  float Voltage = (float)AValue;
  Voltage *= 3.3;
  Voltage /= (1.8 * 4096);

  R           = (AVDD / Voltage) * R2 - R2; // Voltage divider between NTC and R2
  Temperature = B / (log(R / R0) + B / T0);

  return Temperature;
}

bool TReadoutBoardDAQ::PowerOn(int &AOverflow)
{

  // set current limits with voltages off
  WriteCurrentLimits(false, true);
  // switch on voltages
  WriteCurrentLimits(true, true);

  // do like in old software:
  // std::cout << "Chip voltages off, setting current limits" << std::endl;
  // WriteRegister(0x100, 0x1126ce8b);
  // WriteRegister(0x101, 0xe8b);
  // WriteRegister(0x501, 0x67666664);
  // usleep(50000);

  // std::cout << "Switching chip voltages on" << std::endl;
  // WriteRegister(0x100, 0x1326ce8b);

  sleep(1); // sleep after PowerOn

  return ReadLDOStatus(AOverflow);
}

void TReadoutBoardDAQ::PowerOff()
{
  // registers set in sequence similar to old software..

  fBoardConfigDAQ->SetDataPortSelect(0); // select no dataport
  WriteReadoutModuleConfigRegisters();

  fBoardConfigDAQ->SetDrstTime(0);         // TODO necessary?
  fBoardConfigDAQ->SetClockEnableTime(0);  // TODO necessary?
  fBoardConfigDAQ->SetSignalEnableTime(0); // TODO necessary?
  fBoardConfigDAQ->SetAutoShutdownTime(1); // TODO necessary?
  // WriteDelays();
  WriteResetModuleConfigRegisters();

  fBoardConfigDAQ->SetAutoShutdownEnable(1);
  fBoardConfigDAQ->SetLDOEnable(0);
  WriteADCModuleConfigRegisters();
}

void TReadoutBoardDAQ::ReadAllRegisters()
{
  for (int i_module = 0; i_module < 8; ++i_module) {
    for (int i_reg = 0; i_reg < 8; ++i_reg) {
      uint32_t value   = -1;
      uint16_t address = (i_module & 0xf) << 8 | (i_reg & 0xff);
      ReadRegister(address, value);
      std::cout << i_module << '\t' << i_reg << "\t0x" << std::hex << address << ":\t0x" << value
                << std::dec << std::endl;
    }
  }
}

void TReadoutBoardDAQ::DumpConfig(const char *fName, bool writeFile, char *config)
{
  config[0] = '\0';
  if (writeFile) {
    FILE *fp = fopen(fName, "w");
    fprintf(fp, "FIRMWARE  %i\n", ReadFirmwareVersion());
    fprintf(fp, "TRIGGERDELAY  %i\n",
            GetBoardConfig()->GetTriggerDelay()); // same as StrobeDelay on DAQboard
    fprintf(fp, "PULSEDELAY  %i\n", GetBoardConfig()->GetPulseDelay());
    fclose(fp);
  }

  sprintf(config, "FIRMWARE  0x%x\nTRIGGERDELAY  %i\nPULSEDELAY  %i\n", ReadFirmwareVersion(),
          GetBoardConfig()->GetTriggerDelay(), GetBoardConfig()->GetPulseDelay());
}

//---------------------------------------------------------
// methods related to data readout
//---------------------------------------------------------
int TReadoutBoardDAQ::GetEventBufferLength()
{
  int buffer_length = 0;
  fMtx.lock();
  buffer_length = fEventBuffer.size();
  fMtx.unlock();

  return buffer_length;
}

//---------------------------------------------------------
// methods module by module
//---------------------------------------------------------

// ADC Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteADCModuleConfigRegisters()
{
  int limitDigital  = CurrentToADC(fBoardConfigDAQ->GetCurrentLimitDigital());
  int limitIo       = CurrentToADC(fBoardConfigDAQ->GetCurrentLimitIo());
  int limitAnalogue = CurrentToADC(fBoardConfigDAQ->GetCurrentLimitAnalogue());

  // ADC config reg 0
  uint32_t config0 = 0;
  config0 |= (limitDigital & 0xfff);
  config0 |= ((limitIo & 0xfff) << 12);
  config0 |= ((fBoardConfigDAQ->GetAutoShutdownEnable() ? 1 : 0) << 24);
  config0 |= ((fBoardConfigDAQ->GetLDOEnable() ? 1 : 0) << 25);
  config0 |= ((fBoardConfigDAQ->GetADCEnable() ? 1 : 0) << 26);
  config0 |= ((fBoardConfigDAQ->GetADCSelfStop() ? 1 : 0) << 27);
  config0 |= ((fBoardConfigDAQ->GetDisableTstmpReset() ? 1 : 0) << 28);
  config0 |= ((fBoardConfigDAQ->GetPktBasedROEnableADC() ? 1 : 0) << 29);
  WriteRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG0, config0);

  // ADC config reg 1
  uint32_t config1 = 0;
  config1 |= (limitAnalogue & 0xfff);
  WriteRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG1, config1);

  // ADC config reg 2
  uint32_t config2 = 0;
  config2 |= (fBoardConfigDAQ->GetAutoShutOffDelay() & 0xfffff);
  config2 |= (fBoardConfigDAQ->GetADCDownSamplingFact() & 0xfff << 20);
  WriteRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG2, config2);
}

void TReadoutBoardDAQ::WriteCurrentLimits(bool ALDOEnable, bool AAutoshutdown)
{
  // int limitDigital = CurrentToADC (fBoardConfigDAQ->GetCurrentLimitDigital());
  // int limitIo      = CurrentToADC (fBoardConfigDAQ->GetCurrentLimitIo());
  // int limitAnalog  = CurrentToADC (fBoardConfigDAQ->GetCurrentLimitAnalogue());

  fBoardConfigDAQ->SetAutoShutdownEnable(AAutoshutdown); // keep track of settings in BoardConfig..
  fBoardConfigDAQ->SetLDOEnable(ALDOEnable);             // keep track of settings in BoardConfig..

  // uint32_t config0 = (((int) limitDigital) & 0xfff) | ((((int) limitIo) & 0xfff) << 12);
  // config0 |= ((AAutoshutdown?1:0) << 24);
  // config0 |= ((ALDOEnable       ?1:0) << 25);
  // WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG0, config0);
  // uint32_t config1 = ((int) limitAnalog) & 0xfff;
  // WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG1, config1);

  WriteADCModuleConfigRegisters();
}

bool TReadoutBoardDAQ::ReadLDOStatus(int &AOverflow)
{
  uint32_t ReadValue;
  bool     reg0, reg1, reg2;
  int      err;

  err = ReadRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA0, ReadValue);
  if (err == -1) std::cout << "Failed reading from DAQ board" << std::endl;
  reg0 = ((ReadValue & 0x1000000) != 0); // LDO off if bit==0, on if bit==1
  err  = ReadRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA1, ReadValue);
  if (err == -1) std::cout << "Failed reading from DAQ board" << std::endl;
  reg1 = ((ReadValue & 0x1000000) != 0);
  err  = ReadRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA2, ReadValue);
  if (err == -1) std::cout << "Failed reading from DAQ board" << std::endl;
  reg2 = ((ReadValue & 0x1000000) != 0);

  err       = ReadRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_OVERFLOW, ReadValue);
  AOverflow = (int)ReadValue;

  if (!(reg0 & reg1 & reg2))
    std::cout << "GetLDOStatus, LDO status = " << reg0 << ", " << reg1 << ", " << reg2 << std::endl;

  return (reg0 & reg1 & reg2);
}

void TReadoutBoardDAQ::DecodeOverflow(int AOverflow)
{
  if (AOverflow & 0x1) {
    std::cout << "Overflow in digital current" << std::endl;
  }
  if (AOverflow & 0x2) {
    std::cout << "Overflow in digital I/O current" << std::endl;
  }
  if (AOverflow & 0x4) {
    std::cout << "Overflow in analogue current" << std::endl;
  }
}

float TReadoutBoardDAQ::ReadAnalogI()
{
  uint32_t ReadValue;
  ReadRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA2, ReadValue);
  int Value = (ReadValue >> 12) & 0xfff;

  return ADCToSupplyCurrent(Value);
}

float TReadoutBoardDAQ::ReadDigitalI()
{
  uint32_t ReadValue;
  ReadRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA1, ReadValue);
  int Value = (ReadValue >> 12) & 0xfff;

  return ADCToSupplyCurrent(Value);
}

float TReadoutBoardDAQ::ReadIoI()
{
  uint32_t ReadValue;
  ReadRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA2, ReadValue);
  int Value = (ReadValue)&0xfff;

  return ADCToSupplyCurrent(Value);
}

float TReadoutBoardDAQ::ReadMonV()
{
  uint32_t ReadValue;
  ReadRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA0, ReadValue);
  int Value = (ReadValue >> 12) & 0xfff;

  float Voltage = (float)Value;
  Voltage *= 3.3;
  Voltage /= (1.8 * 4096);
  return Voltage;
}

float TReadoutBoardDAQ::ReadMonI()
{
  uint32_t ReadValue;
  ReadRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA1, ReadValue);
  int Value = (ReadValue)&0xfff;

  return ADCToDacmonCurrent(Value);
}

float TReadoutBoardDAQ::ReadTemperature()
{
  uint32_t ReadValue;
  ReadRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA0, ReadValue);
  // printf("NTC ADC: 0x%08X\n",Reading);
  int Value = (ReadValue)&0xfff;

  return ADCToTemperature(Value);
}

bool TReadoutBoardDAQ::ReadMonitorRegisters()
{
  ReadMonitorReadoutRegister();
  ReadMonitorTriggerRegister();
  return true; // added Caterina
}

bool TReadoutBoardDAQ::ReadMonitorReadoutRegister()
{
  uint32_t value;
  int      addr;
  addr = READOUT_MONITOR1 + (MODULE_READOUT << DAQBOARD_REG_ADDR_SIZE);
  ReadRegister(addr, value);
  std::cout << "READOUT_MONITOR1 (0x" << std::hex << addr << "): 0x" << value << std::endl;
  std::cout << " READOUT SM 2: 0x" << (value & 0x7) << std::endl;
  std::cout << " EOT SM: 0x" << ((value >> 3) & 0x7) << std::endl;
  std::cout << " EOT COUNTER: 0x" << ((value >> 6) & 0xff) << std::endl;
  std::cout << " TIMESTAMP FIFO EMPTY: " << ((value >> 14) & 0x1) << std::endl;
  std::cout << " PACKET BASED FLAG: " << ((value >> 15) & 0x1) << std::endl;
  std::cout << " FIFO 33 BIT EMPTY: " << ((value >> 16) & 0x1) << std::endl;
  std::cout << " FIFO 32 BIT ALMOST FULL: " << ((value >> 17) & 0x1) << std::endl;
  std::cout << " SM READOUT 1: 0x" << ((value >> 18) & 0x7) << std::endl;
  std::cout << " FIFO 9 BIT EMPTY: " << ((value >> 21) & 0x1) << std::endl;
  std::cout << " SM READOUT 3: " << ((value >> 22) & 0x1) << std::endl;
  std::cout << " FIFO 33 BIT FULL: " << ((value >> 23) & 0x1) << std::endl;
  std::cout << " SM CTRL WORD DECODER: 0x" << ((value >> 24) & 0x7) << std::endl;
  std::cout << " FIFO 32 BIT EMPTY: " << ((value >> 27) & 0x1) << std::dec << std::endl;
  return true;
}

bool TReadoutBoardDAQ::ReadMonitorTriggerRegister()
{
  uint32_t value;
  int      addr;
  addr = TRIG_MONITOR1 + (MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE);
  WriteRegister(addr, 0xaaa);
  ReadRegister(addr, value);
  std::cout << "TRIG_MONITOR1 (0x" << std::hex << addr << "): 0x" << value << std::endl;
  std::cout << " TRIGGER SM: 0x" << (value & 0x7) << std::endl;
  std::cout << " BUSY SM: 0x" << ((value >> 3) & 0x3) << std::endl;
  std::cout << " STOP TRIGGER COUNTER: 0x" << ((value >> 5) & 0xff) << std::endl;
  std::cout << " START TRIGGER COUNTER: 0x" << ((value >> 13) & 0xff) << std::endl;
  std::cout << " BUSY IN: " << ((value >> 21) & 0x1) << std::endl;
  std::cout << " BUSY PALPIDE: " << ((value >> 22) & 0x1) << std::endl;
  std::cout << " CONTROL WORD BUSY: " << ((value >> 23) & 0x1) << std::endl;
  std::cout << " BUSY EVENT BUILDER: " << ((value >> 24) & 0x1) << std::endl;
  std::cout << " BUSY OVERRIDE FLAG: " << ((value >> 25) & 0x1) << std::endl;
  std::cout << " BUSY : " << ((value >> 26) & 0x1) << std::dec << std::endl;
  return true;
}

// READOUT Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteReadoutModuleConfigRegisters()
{

  // Event builder config reg 0
  uint32_t config = 0;
  config |= (fBoardConfigDAQ->GetMaxDiffTriggers() & 0xf);
  config |= ((fBoardConfigDAQ->GetSamplingEdgeSelect() ? 1 : 0) << 4);
  config |= ((fBoardConfigDAQ->GetPktBasedROEnable() ? 1 : 0) << 5);
  config |= ((fBoardConfigDAQ->GetDDREnable() ? 1 : 0) << 6);
  config |= ((fBoardConfigDAQ->GetDataPortSelect() & 0x3) << 7);
  config |= ((fBoardConfigDAQ->GetFPGAEmulationMode() & 0x3) << 9);
  config |= ((fBoardConfigDAQ->GetHeaderType() ? 1 : 0) << 11);
  config |= ((fBoardConfigDAQ->GetParamValue("BOARDVERSION") & 0x1) << 12);

  // std::cout << "FPGAEmulationMode: " << fBoardConfigDAQ->GetFPGAEmulationMode() << std::endl;

  WriteRegister((MODULE_READOUT << DAQBOARD_REG_ADDR_SIZE) + READOUT_EVENTBUILDER_CONFIG, config);
}

bool TReadoutBoardDAQ::ResyncSerialPort()
{
  return WriteRegister((MODULE_READOUT << DAQBOARD_REG_ADDR_SIZE) + READOUT_RESYNC, 0x0);
}

bool TReadoutBoardDAQ::WriteSlaveDataEmulatorReg(uint32_t AWord)
{
  AWord &= 0xffffffff;
  return WriteRegister((MODULE_READOUT << DAQBOARD_REG_ADDR_SIZE) + READOUT_SLAVE_DATA_EMULATOR,
                       AWord);
}

// TRIGGER Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteTriggerModuleConfigRegisters()
{
  //  busy config reg
  uint32_t config0 = 0;
  config0 |= fBoardConfigDAQ->GetBusyDuration();
  WriteRegister((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_BUSY_DURATION, config0);

  // trigger conif reg
  uint32_t config1 = 0;
  config1 |= (fBoardConfigDAQ->GetNTriggers() & 0xffff);
  config1 |= ((fBoardConfigDAQ->GetTriggerMode() & 0x7) << 16);
  config1 |= ((fBoardConfigDAQ->GetStrobeDuration() & 0xff) << 19);
  config1 |= ((fBoardConfigDAQ->GetBusyConfig() & 0x7) << 27);
  // std::cout << "config1: " << std::hex << config1 << std::dec << std::endl;
  WriteRegister((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_TRIGGER_CONFIG, config1);

  //  strobe delay config reg
  // std::cout << fBoardConfigDAQ->GetStrobeDelay() << std::endl;
  uint32_t config2 = 0;
  config2 |= fBoardConfigDAQ->GetStrobeDelay();
  // std::cout << "config2: " << std::hex << config2 << std::dec << std::endl;
  WriteRegister((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_DELAY,
                config2); // returns err: int
  // std::cout << err << std::endl;
  //  busy override config reg
  uint32_t config3 = 0;
  config3 |= (fBoardConfigDAQ->GetBusyOverride() ? 1 : 0);
  // std::cout << "config3: " << std::hex << config3 << std::dec << std::endl;
  WriteRegister((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_BUSY_OVERRIDE, config3);
}

bool TReadoutBoardDAQ::StartTrigger()
{
  return WriteRegister((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_START, 13);
}

bool TReadoutBoardDAQ::StopTrigger()
{
  return WriteRegister((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_STOP, 13);
}

bool TReadoutBoardDAQ::WriteBusyOverrideReg(bool ABusyOverride)
{
  fBoardConfigDAQ->SetBusyOverride(ABusyOverride);
  bool err;
  err =
      WriteRegister((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_BUSY_OVERRIDE, ABusyOverride);
  if (!err) return false;

  return err;
}

// CMU Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteCMUModuleConfigRegisters()
{

  //  CMU config reg
  uint32_t config = 0;
  config |= (fBoardConfigDAQ->GetManchesterDisable() ? 1 : 0);
  config |= ((fBoardConfigDAQ->GetSamplingEdgeSelectCMU() ? 1 : 0) << 1);
  config |= ((fBoardConfigDAQ->GetInvertCMUBus() ? 1 : 0) << 2);
  config |= ((fBoardConfigDAQ->GetChipMaster() ? 1 : 0) << 3);
  WriteRegister((MODULE_CMU << DAQBOARD_REG_ADDR_SIZE) + CMU_CONFIG, config);
}

// RESET Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteResetModuleConfigRegisters()
{
  //  PULSE DRST PRST duration reg
  uint32_t config0 = 0;
  config0 |= (fBoardConfigDAQ->GetPRSTDuration() & 0xff);
  config0 |= ((fBoardConfigDAQ->GetDRSTDuration() & 0xff) << 8);
  config0 |= ((fBoardConfigDAQ->GetPULSEDuration() & 0xffff) << 16);
  WriteRegister((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_DURATION, config0);

  // power up sequencer delay register
  uint32_t config1 = ((fBoardConfigDAQ->GetDrstTime() & 0xff) << 24) |
                     ((fBoardConfigDAQ->GetSignalEnableTime() & 0xff) << 16) |
                     ((fBoardConfigDAQ->GetClockEnableTime() & 0xff) << 8) |
                     (fBoardConfigDAQ->GetAutoShutdownTime() & 0xff);
  WriteRegister((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_DELAYS, config1);

  // PULSE STROBE delay sequence reg
  uint32_t config2 = 0;
  // std::cout << "PulseDelay: " << fBoardConfigDAQ->GetPulseDelay() << std::endl;
  config2 |= (fBoardConfigDAQ->GetPulseDelay() & 0xffff);
  config2 |= ((fBoardConfigDAQ->GetStrobePulseSeq() & 0x3) << 16);
  WriteRegister((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_PULSE_DELAY, config2);

  // Power On Reset disable reg
  uint32_t config3 = 0;
  config3 |= (fBoardConfigDAQ->GetPORDisable() ? 1 : 0);
  WriteRegister((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_POR_DISABLE, config3);
}

void TReadoutBoardDAQ::WriteDelays()
{
  uint32_t delays = ((fBoardConfigDAQ->GetDrstTime() & 0xff) << 24) |
                    ((fBoardConfigDAQ->GetSignalEnableTime() & 0xff) << 16) |
                    ((fBoardConfigDAQ->GetClockEnableTime() & 0xff) << 8) |
                    (fBoardConfigDAQ->GetAutoShutdownTime() & 0xff);
  WriteRegister((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_DELAYS, delays);
}

// ID Module
//----------------------------------------------------------------------------

int TReadoutBoardDAQ::ReadBoardAddress()
{
  uint32_t ReadValue;
  ReadRegister((MODULE_ID << DAQBOARD_REG_ADDR_SIZE) + ID_ADDRESS, ReadValue);
  int BoardAddress = (~ReadValue) & 0xf;

  return BoardAddress;
}

bool TReadoutBoardDAQ::CheckBoardAddress()
{
  int addr      = ReadBoardAddress();
  int conf_addr = fBoardConfigDAQ->GetBoardAddress();
  if (addr < 0) return false;         // read failure
  if (addr == conf_addr) return true; // matching address
  if (conf_addr == -1) return true;   // accept any board address
  // std::cout << "Address mismatch in configuration (0x"
  //          << std::hex << conf_addr << std::dec
  //          << ") and on the board (0x"
  //          << std::hex << addr << std::dec
  //          << ")" << std::endl;
  return false;
}

uint32_t TReadoutBoardDAQ::ReadFirmwareVersion()
{
  ReadRegister((MODULE_ID << DAQBOARD_REG_ADDR_SIZE) + ID_FIRMWARE, fFirmwareVersion);

  return fFirmwareVersion;
}

uint32_t TReadoutBoardDAQ::ReadFirmwareDate()
{
  if (fFirmwareVersion == 0) {
    ReadRegister((MODULE_ID << DAQBOARD_REG_ADDR_SIZE) + ID_FIRMWARE, fFirmwareVersion);
  }

  return (fFirmwareVersion & 0xffffff);
}

int TReadoutBoardDAQ::ReadFirmwareChipVersion()
{
  if (fFirmwareVersion == 0) {
    ReadRegister((MODULE_ID << DAQBOARD_REG_ADDR_SIZE) + ID_FIRMWARE, fFirmwareVersion);
  }
  return ((fFirmwareVersion & 0xf0000000) >> 28);
}

// SOFTRESET Module
//----------------------------------------------------------------------------
void TReadoutBoardDAQ::WriteSoftResetModuleConfigRegisters()
{
  //  PULSE DRST PRST duration reg
  uint32_t config = 0;
  config |= (fBoardConfigDAQ->GetSoftResetDuration() & 0xff);
  WriteRegister((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_DURATION, config);
}

bool TReadoutBoardDAQ::ResetBoardFPGA(int ADuration)
{
  fBoardConfigDAQ->SetSoftResetDuration(
      ADuration); // keep track of latest config in TBoardConfigDAQ
  bool err;
  err = WriteRegister((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_DURATION, ADuration);
  if (!err) return false;
  return WriteRegister((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_FPGA_RESET, 13);
}

bool TReadoutBoardDAQ::ResetBoardFX3(int ADuration)
{
  fBoardConfigDAQ->SetSoftResetDuration(
      ADuration); // keep track of latest config in TBoardConfigDAQ
  bool err;
  err = WriteRegister((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_DURATION, ADuration);
  if (!err) return false;
  return WriteRegister((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_FX3_RESET, 13);
}
