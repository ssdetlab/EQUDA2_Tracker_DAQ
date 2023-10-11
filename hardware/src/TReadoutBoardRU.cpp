#include <algorithm>
#include <iomanip>

#include "AlpideDecoder.h"
#include "TAlpide.h"
#include "TReadoutBoardRU.h"

const int     TReadoutBoardRU::VID              = 0x04B4;
const int     TReadoutBoardRU::PID              = 0x0008;
const int     TReadoutBoardRU::INTERFACE_NUMBER = 2;
const uint8_t TReadoutBoardRU::EP_CTL_OUT       = 3;
const uint8_t TReadoutBoardRU::EP_CTL_IN        = 3;
const uint8_t TReadoutBoardRU::EP_DATA0_IN      = 4;
const uint8_t TReadoutBoardRU::EP_DATA1_IN      = 5;

const size_t TReadoutBoardRU::EVENT_DATA_READ_CHUNK = 50 * 1024;
const size_t TReadoutBoardRU::USB_TIMEOUT           = 1;
const int    TReadoutBoardRU::MAX_RETRIES_READ      = 5;

const uint8_t TReadoutBoardRU::MODULE_MASTER  = 0;
const uint8_t TReadoutBoardRU::MODULE_STATUS  = 1;
const uint8_t TReadoutBoardRU::MODULE_VOLTAGE = 2;
const uint8_t TReadoutBoardRU::MODULE_DCTRL   = 3;
const uint8_t TReadoutBoardRU::MODULE_DATA0   = 4;

const uint8_t TReadoutBoardRU::MASTER_DP23_SOURCE = 10;

int roundUpToMultiple(int numToRound, int multiple)
{
  if (multiple == 0) return numToRound;

  int remainder = numToRound % multiple;
  if (remainder == 0) return numToRound;

  return numToRound + multiple - remainder;
}

TReadoutBoardRU::TReadoutBoardRU(TBoardConfigRU *config)
    : TReadoutBoard(config), m_buffer(), m_readBytes(0), m_logging(config->enableLogging()),
      m_enablePulse(false), m_enableTrigger(true), m_triggerDelay(0), m_pulseDelay(0)
{

  m_usb = std::make_shared<UsbDev>(TReadoutBoardRU::VID, TReadoutBoardRU::PID,
                                   TReadoutBoardRU::INTERFACE_NUMBER);

  dctrl = std::make_shared<TRuDctrlModule>(*this, TReadoutBoardRU::MODULE_DCTRL, m_logging);

  master = std::make_shared<TRuWishboneModule>(*this, TReadoutBoardRU::MODULE_MASTER, m_logging);
}

void TReadoutBoardRU::InitReceivers()
{
  std::cout << "....Setup Chip positions....\n";

  for (auto &chippos : fChipPositions) {
    bool hasTranceiver = (chippos.chipId % 8 == 0) && (chippos.enabled);
    std::cout << "chippos " << chippos.chipId << " " << hasTranceiver << std::endl;
    if (chippos.receiver >= 0 && hasTranceiver) {
      uint8_t moduleId = TReadoutBoardRU::MODULE_DATA0 + chippos.receiver;
      std::cout << "Module " << (int)moduleId << " set to chip " << chippos.chipId << "\n";
      transceiver_array[chippos.chipId] =
          std::make_shared<TRuTransceiverModule>(*this, moduleId, m_logging);
      transceiver_array[chippos.chipId]->DeactivateReadout();
    }
  }
}

void TReadoutBoardRU::registeredWrite(uint16_t module, uint16_t address, uint16_t data)
{
  uint8_t module_byte  = module & 0x7F;
  uint8_t address_byte = address & 0xFF;
  uint8_t data_low     = data >> 0 & 0xFF;
  uint8_t data_high    = data >> 8 & 0xFF;

  module_byte |= 0x80; // Write operation

  m_buffer.push_back(data_low);
  m_buffer.push_back(data_high);
  m_buffer.push_back(address_byte);
  m_buffer.push_back(module_byte);
}

void TReadoutBoardRU::registeredRead(uint16_t module, uint16_t address)
{
  uint8_t module_byte  = module & 0x7F;
  uint8_t address_byte = address & 0xFF;

  m_buffer.push_back(0);
  m_buffer.push_back(0);
  m_buffer.push_back(address_byte);
  m_buffer.push_back(module_byte);

  m_readBytes += 4;
}

bool TReadoutBoardRU::flush()
{
  size_t toWrite = m_buffer.size();
  size_t written = m_usb->writeData(EP_CTL_OUT, m_buffer, USB_TIMEOUT);
  // std::cout << "Bytes written: " << written << "( ";
  // for (auto b : m_buffer)
  //    std::cout << "0x" <<std::hex << (int)b << ",";
  // std::cout << ")\n";
  m_buffer.clear();

  return toWrite == written;
}

void TReadoutBoardRU::readFromPort(uint8_t port, size_t size, UsbDev::DataBuffer &buffer)
{
  buffer.resize(roundUpToMultiple(size, 1024));
  m_usb->readData(port, buffer, USB_TIMEOUT); // returns bytesRead : size_t

  // std::cout << "Port " << (int) port << ": Bytes Read: " << bytesRead << "(
  // ";
  // for (auto b : buffer)
  //    std::cout << "0x" <<std::hex << (int)b << ",";
  // std::cout << ")\n";
}

std::vector<TReadoutBoardRU::ReadResult> TReadoutBoardRU::readResults()
{
  // Read from Control Port
  UsbDev::DataBuffer buffer_all, buffer_single;
  int                retries   = 0;
  size_t             data_left = m_readBytes;
  while (buffer_all.size() < m_readBytes && retries < MAX_RETRIES_READ) {

    readFromPort(EP_CTL_IN, data_left, buffer_single);
    buffer_all.insert(buffer_all.end(), buffer_single.begin(), buffer_single.end());

    if (buffer_all.size() < m_readBytes) {
      data_left = m_readBytes - buffer_all.size();
      ++retries;
    }
  }
  if (buffer_all.size() < m_readBytes) {
    if (m_logging)
      std::cout << "TReadoutBoardRU: could not read all data from Control Port. "
                   "Packet dropped";
  }
  m_readBytes = 0;

  // Process read data
  std::vector<TReadoutBoardRU::ReadResult> results;
  for (size_t i = 0; i < buffer_all.size(); i += 4) {
    uint16_t data       = buffer_all[i] | (buffer_all[i + 1] << 8);
    uint16_t address    = buffer_all[i + 2] | (buffer_all[i + 3] << 8);
    bool     read_error = (address & 0x8000) > 0;
    address &= 0x7FFF;
    if (read_error && m_logging) {
      std::cout << "TReadoutBoardRU: Wishbone error while reading: Address " << address << "\n";
    }
    // std::cout << "Result received: Address: " << std::hex << (int)address <<
    // ", data: " << std::hex
    //          << data << "read error: " << read_error << "\n";
    results.emplace_back(address, data, read_error);
  }
  return results;
}

int TReadoutBoardRU::ReadRegister(uint16_t Address, uint32_t &Value)
{
  uint16_t module      = Address >> 8 & 0xFF;
  uint16_t sub_address = Address & 0xFF;
  registeredRead(module, sub_address);
  flush();
  auto results = readResults();
  if (results.size() != 1) {
    if (m_logging)
      std::cout << "TReadoutBoardRU: Expected 1 result, got " << results.size() << "\n";
    return -1;
  }
  else {
    Value = results[0].data;
  }
  return 0;
}
int TReadoutBoardRU::WriteRegister(uint16_t Address, uint32_t Value)
{
  uint16_t module      = Address >> 8 & 0xFF;
  uint16_t sub_address = Address & 0xFF;
  uint16_t data        = Value & 0xFFFF;
  registeredWrite(module, sub_address, data);
  flush();

  return 0;
}

int TReadoutBoardRU::WriteChipRegister(uint16_t Address, uint16_t Value, TAlpide *chipPtr)
{
  uint8_t chipId = chipPtr->GetConfig()->GetChipId();
  dctrl->WriteChipRegister(Address, Value, chipId);
  return 0;
}

int TReadoutBoardRU::ReadChipRegister(uint16_t Address, uint16_t &Value, TAlpide *chipPtr)
{
  uint8_t chipId = chipPtr->GetConfig()->GetChipId();
  // set control
  dctrl->SetConnector(GetControlInterface(chipId), false);

  return dctrl->ReadChipRegister(Address, Value, chipId);
}
int TReadoutBoardRU::SendOpCode(Alpide::TOpCode OpCode)
{
  dctrl->SendOpCode(OpCode);
  return 0;
}

int TReadoutBoardRU::SendOpCode(Alpide::TOpCode OpCode, TAlpide *chipPtr)
{
  return SendOpCode(OpCode);
}

int TReadoutBoardRU::SendCommand(Alpide::TCommand Command, TAlpide *chipPtr)
{
  return WriteChipRegister(Alpide::REG_COMMAND, Command, chipPtr);
}

int TReadoutBoardRU::SetTriggerConfig(bool enablePulse, bool enableTrigger, int triggerDelay,
                                      int pulseDelay)
{
  m_enablePulse   = enablePulse;
  m_enableTrigger = enableTrigger;
  m_triggerDelay  = triggerDelay;
  m_pulseDelay    = pulseDelay;
  return 0;
}

void TReadoutBoardRU::SetTriggerSource(TTriggerSource triggerSource) {}

int TReadoutBoardRU::Trigger(int nTriggers)
{
  for (int i = 0; i < nTriggers; ++i) {
    if (m_enablePulse) dctrl->SendOpCode(Alpide::OPCODE_PULSE, false);
    if (m_triggerDelay > 0) dctrl->Wait(4 * m_triggerDelay, false);
    if (m_enableTrigger) dctrl->SendOpCode(Alpide::OPCODE_TRIGGER1, false);
    if (m_pulseDelay > 0) dctrl->Wait(4 * m_pulseDelay, false);
  }
  flush();
  return 0;
}

void TReadoutBoardRU::fetchEventData()
{
  static UsbDev::DataBuffer buffer;
  std::map<uint8_t, size_t> chipByteCounters;
  for (int i = 0; i < 6; ++i) {
    this->setDataportSource(i, i);
    for (auto port : {EP_DATA0_IN, EP_DATA1_IN}) {
      // read chunk from USB
      readFromPort(port, TReadoutBoardRU::EVENT_DATA_READ_CHUNK, buffer);
      if (!buffer.empty()) {
        // std::cout << "===== HS Data ======\n";
        // std::cout << std::hex;
        // for (int i = 0; i < buffer.size(); ++i) {
        //     std::cout << std::setfill('0') << std::setw(2)
        //               << (int)buffer[i] << " ";
        //     if (i % 40 == 39)
        //         std::cout << "\n";
        // }
        // std::cout << std::dec;
        // std::cout << "\n========== /HS Data ==========\n";
      }

      // Filter, remove status, remove padded bytes, split to dataport
      for (size_t i = 0; i < buffer.size(); i += 4) {
        uint8_t status = buffer[i + 3];
        uint8_t index  = status >> 5;
        uint8_t chip   = m_config->getTransceiverChip(port, index);

        for (int j = 0; j < 3; j++) {
          bool isPadded = ((status >> (2 + j)) & 1) == 1;
          if (!isPadded) {
            m_readoutBuffers[chip].push_back(buffer[i + j]);
            chipByteCounters[chip]++;
          }
        }
      }
    }
  }

  // Read out events
  for (auto &buffer : m_readoutBuffers) {
    auto  chipId     = buffer.first;
    auto &data       = buffer.second;
    int   eventStart = 0;
    int   eventEnd   = 0;
    bool  isError;
    bool  hasEvent = AlpideDecoder::ExtractNextEvent(data.data(), data.size(), eventStart, eventEnd,
                                                    isError, false);
    if (isError and m_logging) {
      std::cout << "Event decoding error on chip 0x" << std::hex << (int)chipId
                << ". Event size: " << std::dec << eventEnd << "\n";
    }
    if (hasEvent) {
      // std::cout << "===== Event Data ======\n";
      // std::cout << std::hex;
      // for (int i = eventStart; i < eventEnd; ++i) {
      //     std::cout << std::setfill('0') << std::setw(2)
      //               << (int)data[i] << " ";
      //     if (i % 20 == 19)
      //         std::cout << "\n";
      // }
      // std::cout << std::dec;
      // std::cout << "\n========== /Event Data ==========\n";

      // Extract event bytes and store in event list
      std::vector<uint8_t> eventData(begin(data) + eventStart, begin(data) + eventEnd);
      eventData.push_back((uint8_t)chipId);
      m_events.emplace_back(eventData);

      // event is extracted, remove data from buffer
      data.erase(begin(data), begin(data) + eventEnd);
    }
  }
}

int TReadoutBoardRU::ReadEventData(int &NBytes, unsigned char *Buffer)
{

  if (m_events.empty()) fetchEventData();

  if (!m_events.empty()) {
    auto event = m_events.front();
    m_events.pop_front();

    std::copy(begin(event), end(event), Buffer);
    NBytes = event.size();
  }
  else {
    return -1;
  }

  return 0;
}

int TReadoutBoardRU::Initialize()
{
  dctrl->SetConnector(m_config->getConnector());

  for (auto &transceiver : transceiver_array) {
    transceiver.second->Initialize(m_config->getReadoutSpeed(), m_config->getInvertPolarity());
  }
  return 0;
}

void TReadoutBoardRU::StartRun()
{
  for (size_t i = 0; i < fChipPositions.size(); ++i) {

    if (fChipPositions.at(i).chipId & 0x7) continue;
    if (fChipPositions.at(i).receiver < 0) continue;
    if (!fChipPositions.at(i).enabled) continue;
    std::cout << "initialising transceiver for chip id " << std::dec << fChipPositions.at(i).chipId
              << std::endl;
    TBoardConfigRU *config = (TBoardConfigRU *)GetConfig();
    auto tr = transceiver_array[fChipPositions.at(i).chipId]; // TODO: Mapping between transceiver
                                                              // and chipid
    tr->Initialize(config->getReadoutSpeed(), config->getInvertPolarity());
    std::cout << "Done" << std::endl;
    bool alignedBefore = tr->IsAligned();
    tr->ActivateReadout();
    if (tr->IsAligned()) {
      std::cout << "Transceiver " << std::dec << fChipPositions.at(i).receiver
                << " is aligned (before: " << alignedBefore << " )\n";
    }
    else {
      std::cout << "Transceiver " << std::dec << fChipPositions.at(i).receiver
                << " is NOT aligned \n";
      tr->DeactivateReadout();
    }
    tr->ResetCounters();
  }

  std::cout << "Clean ports\n";
  UsbDev::DataBuffer buf;
  setDataportSource(0, 0);
  readFromPort(TReadoutBoardRU::EP_DATA0_IN, 1024 * 10000, buf);
  readFromPort(TReadoutBoardRU::EP_DATA1_IN, 1024 * 10000, buf);
  std::cout << "Clean ports done\n";
}

void TReadoutBoardRU::checkGitHash()
{
  registeredRead(TReadoutBoardRU::MODULE_STATUS, 0);
  registeredRead(TReadoutBoardRU::MODULE_STATUS, 1);

  flush();
  auto results = readResults();
  if (results.size() != 2) {
    if (m_logging)
      std::cout << "TReadoutBoardRU: Expected 2 results, got " << results.size() << "\n";
  }
  else {
    uint16_t lsb = results[0].data;
    uint16_t msb = results[1].data;
    std::cout << "Git hash: " << std::hex << msb << lsb << "\n";
  }
}

void TReadoutBoardRU::setDataportSource(uint8_t DP2Source, uint8_t DP3Source)
{
  uint16_t setting = ((DP3Source & 0xF) << 8) | (DP2Source & 0xF);
  master->Write(MASTER_DP23_SOURCE, setting, true);
}
