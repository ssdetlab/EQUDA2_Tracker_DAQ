#ifndef CONFIG_H
#define CONFIG_H

#include "TBoardConfig.h"
#include "TChipConfig.h"
#include "THicConfig.h"
#include "TPowerBoardConfig.h"
#include "TScanConfig.h"
#include <string>
#include <vector>

const int DEFAULT_MODULE_ID = 1;

class TChipConfig;

class TConfig {
public:
    TConfig(const char *fName);
    TConfig(int nBoards, std::vector<int> chipIds, TBoardType boardType, TDeviceType dev_type);
    TConfig(int chipId, TBoardType boardType = boardDAQ);

    TDeviceType        GetDeviceType(){ return fDeviceType; };
    unsigned int       GetNChips()    { return fChipConfigs.size(); };
    unsigned int       GetNBoards()   { return fBoardConfigs.size(); };
    unsigned int       GetNHics()     { return fHicConfigs.size(); };

    bool               GetUsePowerBoard()          { return false; };
    void               SetUsePowerBoard(bool UsePB){ fUsePowerBoard = UsePB; };

    TChipConfig*       GetChipConfig(unsigned int iChip);
    TChipConfig*       GetChipConfigById(int chipId);
    TBoardConfig*      GetBoardConfig(unsigned int iBoard);
    TPowerBoardConfig* GetPBConfig(unsigned int iBoard);
    THicConfig*        GetHicConfig(unsigned int iHic);
    THicConfig*        GetHicConfigById(int modId);
    
    TScanConfig*       GetScanConfig(){ return fScanConfig; };
    void               WriteToFile(const char *fName);

    std::string GetSoftwareVersion();

  private:
    std::vector<TBoardConfig*>       fBoardConfigs;
    std::vector<TChipConfig*>        fChipConfigs;
    std::vector<THicConfig*>         fHicConfigs;
    TDeviceType                      fDeviceType;
    
    bool                             fUsePowerBoard;
    std::vector<TPowerBoardConfig*>  fPBConfigs;
    TScanConfig*                     fScanConfig;

    void        ReadConfigFile(const char *fName);
    void        Init(int nBoards, std::vector<int> chipIds, TBoardType boardType = boardMOSAIC);
    void        Init(int chipId, TBoardType boardType = boardDAQ);
    void        ParseLine(std::string Line, std::string &Param, std::string &Value, int *Chip);
    void        DecodeLine(std::string Line);
    void        SetDeviceType(TDeviceType AType, int NChips);
    TDeviceType ReadDeviceType(std::string deviceName);
};

#endif