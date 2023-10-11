#ifndef BOARDCONFIGMOSAIC_H
#define BOARDCONFIGMOSAIC_H

#include "Mosaic.h"
#include "TBoardConfig.h"

#include <stdio.h>

#define MAX_MOSAICCTRLINT 12
#define MAX_MOSAICTRANRECV 10
#define MOSAIC_HEADER_LENGTH 64

class TBoardConfigMOSAIC : public TBoardConfig {
  public:
    TBoardConfigMOSAIC(const char *fName = 0, int boardIndex = 0);

    char*                  GetIPaddress()         { return IPAddress; }
    uint16_t               GetCtrlInterfaceNum()  { return (uint16_t)NumberOfControlInterfaces; }
    uint16_t               GetTCPport()           { return (uint16_t)TCPPort; }
    uint16_t               GetCtrlInterfacePhase(){ return (uint16_t)ControlInterfacePhase; }
    uint32_t               GetCtrlAFThreshold()   { return (uint32_t)RunCtrlAFThreshold; }
    uint16_t               GetCtrlLatMode()       { return (uint16_t)RunCtrlLatMode; }
    uint32_t               GetCtrlTimeout()       { return (uint32_t)RunCtrlTimeout; }
    uint32_t               GetPollingDataTimeout(){ return (uint32_t)pollDataTimeout; }
    uint32_t               GetManchesterDisable() { return (uint32_t)ManchesterDisable; }
    uint32_t               GetMasterSlaveMode()   { return (uint32_t)MasterSlave; }
    bool                   IsInverted()           { return (bool)Inverted; }
    Mosaic::TReceiverSpeed GetSpeedMode();

    void SetIPaddress(const char *AIPaddress);
    void SetTCPport(uint16_t APort){ TCPPort = (int)APort; }
    void SetCtrlInterfaceNum(uint16_t ACtrlInterfaceNumber){
      NumberOfControlInterfaces = (int)ACtrlInterfaceNumber;
    }
    void SetCtrlInterfacePhase(uint16_t ACtrlInterfacePhase){
      ControlInterfacePhase = (int)ACtrlInterfacePhase;
    }
    void SetCtrlAFThreshold(uint32_t ACtrlAFThreshold)   { RunCtrlAFThreshold = (int)ACtrlAFThreshold; }
    void SetCtrlLatMode(uint16_t ARunCtrlLatencyMode)    { RunCtrlLatMode = (int)ARunCtrlLatencyMode; }
    void SetCtrlTimeout(uint32_t ARunCtrlTimeout)        { RunCtrlTimeout = (int)ARunCtrlTimeout; }
    void SetInvertedData(bool AIsInverted)               { Inverted = (int)AIsInverted; };
    void SetPollingDataTimeout(uint32_t APollDataTimeout){ pollDataTimeout = (int)APollDataTimeout; }
    void SetManchesterDisable(uint32_t AIsManchesterDisabled){
      ManchesterDisable = (int)AIsManchesterDisabled;
    }
    void SetSpeedMode(Mosaic::TReceiverSpeed ASpeedMode);

  private:
    FILE *fhConfigFile; // the file handle of the Configuration File

    const int    DEF_TCPPORT           = 2000;
    const int    DEF_CTRLINTPHASE      = 2;
    const int    DEF_CTRLAFTHR         = 1250000;
    const int    DEF_CTRLLATMODE       = 0;
    const int    DEF_CTRLTIMEOUT       = 0;
    const int    DEF_POLLDATATIMEOUT   = 500;
    const int    DEF_POLARITYINVERSION = 0;
    const int    DEF_SPEEDMODE         = 0;
    static char* DEF_IP_ADDRESS;
    const int    DEF_MANCHESTERDISABLE = 0;
    const int    DEF_MASTERSLAVE       = 0;

  protected:
    void InitParamMap();

    int NumberOfControlInterfaces;
    int TCPPort;
    int ControlInterfacePhase;
    int RunCtrlAFThreshold;
    int RunCtrlLatMode;
    int RunCtrlTimeout;
    int pollDataTimeout;
    int Inverted;
    int SpeedMode;
    int ManchesterDisable; //     0; 0: enable manchester encoding; 1: disable
    int MasterSlave;       //     0: off, 1; activated

    char IPAddress[30];
};

#endif