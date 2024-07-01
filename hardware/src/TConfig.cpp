#include "TConfig.h"
#include "TBoardConfigMOSAIC.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

// construct Config from config file
TConfig::TConfig(const char *fName) {
    fDeviceType    = TYPE_UNKNOWN; // will be overwritten in read config file
    fUsePowerBoard = false;
    ReadConfigFile(fName);
}

// construct Config in the application using only number of boards and number of chips / vector of
// chip Ids
// for the time being use one common config for all board types (change this?)
// this constructor does not set the device type correctly
// (not clear right now, which setup this constructor will be used for)
TConfig::TConfig(
    int nBoards, 
    std::vector<int> chipIds, 
    TBoardType boardType, 
    TDeviceType dev_type) {
        fDeviceType = dev_type;
        Init(nBoards, chipIds, boardType);
}

// construct a config for a single chip setup (one board and one chip only)
TConfig::TConfig(int chipId, TBoardType boardType) {
    fUsePowerBoard = false;
    Init(chipId, boardType);
}

void TConfig::Init(int nBoards, std::vector<int> chipIds, TBoardType boardType) {
    for (int iboard = 0; iboard < nBoards; iboard++) {
        fBoardConfigs.push_back(new TBoardConfigMOSAIC());
    }
    for (unsigned int ichip = 0; ichip < chipIds.size(); ichip++) {
        fChipConfigs.push_back(new TChipConfig(this, chipIds.at(ichip)));
    }
}

void TConfig::Init(int chipId, TBoardType boardType) {
    fDeviceType = TYPE_CHIP_MOSAIC;
    fBoardConfigs.push_back(new TBoardConfigMOSAIC());
    
    fChipConfigs.push_back(new TChipConfig(this, chipId));
}

// getter functions for chip, board and hic config
TChipConfig *TConfig::GetChipConfigById(int chipId) {
    for (unsigned int i = 0; i < fChipConfigs.size(); i++) {
        if (fChipConfigs.at(i)->GetChipId() == chipId) {
            return fChipConfigs.at(i);
        }
    }
    // throw exception here.
    std::cout << "Chip id " << chipId << " not found" << std::endl;
    return 0;
}

TChipConfig *TConfig::GetChipConfig(unsigned int iChip) {
    if (iChip < fChipConfigs.size()) {
        return fChipConfigs.at(iChip);
    }
    else {
        return 0;
    }
}

TBoardConfig *TConfig::GetBoardConfig(unsigned int iBoard) {
    if (iBoard < fBoardConfigs.size()) {
        return fBoardConfigs.at(iBoard);
    }
    else { // throw exception
        return 0;
    }
}

TDeviceType TConfig::ReadDeviceType(std::string deviceName) {
    TDeviceType type = TYPE_UNKNOWN;
    
    if (deviceName.find(" ") != std::string::npos) {
        deviceName.erase(deviceName.find(" "));
    }
    if (deviceName.find("\t") != std::string::npos) {
        deviceName.erase(deviceName.find("\t"));
    }
    if (deviceName.compare("CHIP") == 0) {
        type = TYPE_CHIP;
    }
    else if (deviceName.compare("TELESCOPE") == 0) {
        type = TYPE_TELESCOPE;
    }
    else if (deviceName.compare("OBHIC") == 0) {
        type = TYPE_OBHIC;
    }
    else if (deviceName.compare("OBHIC_PB") == 0) {
        SetUsePowerBoard(true);
        type = TYPE_OBHIC;
    }
    else if (deviceName.compare("IBHIC") == 0) {
        type = TYPE_IBHIC;
    }
    else if (deviceName.compare("IBHIC_PB") == 0) {
        SetUsePowerBoard(true);
        type = TYPE_IBHIC;
    }
    else if (deviceName.compare("CHIPMOSAIC") == 0) {
        type = TYPE_CHIP_MOSAIC;
    }
    else if (deviceName.compare("HALFSTAVE") == 0) {
        type = TYPE_HALFSTAVE;
    }
    else if (deviceName.compare("HALFSTAVE_PB") == 0) {
        SetUsePowerBoard(true);
        type = TYPE_HALFSTAVE;
    }
    else if (deviceName.compare("HALFSTAVERU") == 0) {
        type = TYPE_HALFSTAVERU;
    }
    else if (deviceName.compare("MLHALFSTAVE") == 0) {
        type = TYPE_MLHALFSTAVE;
    }
    else if (deviceName.compare("MLHALFSTAVE_PB") == 0) {
        SetUsePowerBoard(true);
        type = TYPE_MLHALFSTAVE;
    }
    else if (deviceName.compare("MLSTAVE") == 0) {
        type = TYPE_MLSTAVE;
    }
    else if (deviceName.compare("MLSTAVE_PB") == 0) {
        SetUsePowerBoard(true);
        type = TYPE_MLSTAVE;
    }
    else if (deviceName.compare("ENDURANCETEST") == 0) {
        type = TYPE_ENDURANCE;
    }
    else if (deviceName.compare("ENDURANCETEST_PB") == 0) {
        SetUsePowerBoard(true);
        type = TYPE_ENDURANCE;
    }
    else if (deviceName.compare("IBHICRU") == 0) {
        type = TYPE_IBHICRU;
    }
    else if (deviceName.compare("POWER") == 0) {
        SetUsePowerBoard(true);
        type = TYPE_POWER;
    }
    else {
        std::cout << "Error, unknown setup type found: " << deviceName << std::endl;
        exit(EXIT_FAILURE);
    }
    return type;
}

void TConfig::SetDeviceType(TDeviceType AType, int NChips) {
    std::vector<int> chipIds;
    
    fDeviceType = AType;
    if (AType == TYPE_CHIP) {
        Init(16, boardDAQ);
    }
    else if (AType == TYPE_CHIP_MOSAIC) {
        Init(0, boardMOSAIC);
    }
    else if (AType == TYPE_TELESCOPE) {
        for (int i = 0; i < NChips; i++) {
            chipIds.push_back(16);
        }
        Init(NChips, chipIds, boardDAQ);
    }
    else if (AType == TYPE_OBHIC) {
        for (int i = 0; i < 15; i++) {
            if (i == 7) {
                continue;
            }
            int ModuleId = (NChips <= 0 ? DEFAULT_MODULE_ID : NChips) & 0x07;
            chipIds.push_back(i + (ModuleId << 4));
        }
        Init(1, chipIds, boardMOSAIC);
    }
    else if (AType == TYPE_ENDURANCE) {
        for (int mod = 0; mod < 10; mod++) {
            for (int i = 0; i < 15; i++) {
                if (i == 7) {
                    continue;
                }
                int ModuleId = (NChips <= 0 ? DEFAULT_MODULE_ID : NChips) & 0x07;
                chipIds.push_back(i + (ModuleId << 4));
            }
        }
        Init(2, chipIds, boardMOSAIC);
    }
    else if (AType == TYPE_IBHIC) {
        for (int i = 8; i >= 0; i--) {
            chipIds.push_back(i);
        }
        Init(1, chipIds, boardMOSAIC);
    }
    else if (AType == TYPE_IBHICRU) {
        for (int i = 8; i >= 0; i--) {
            chipIds.push_back(i);
        }
        Init(1, chipIds, boardRU);
    }
    else if ((AType == TYPE_HALFSTAVE) || (AType == TYPE_HALFSTAVERU) ||
            (AType == TYPE_MLHALFSTAVE) || (AType == TYPE_MLSTAVE)) {
        // in case of half stave NChips contains number of modules
        for (int imod = 1; imod <= NChips; imod++) {
            int modId = (imod & 0x7);
            for (int i = 0; i < 15; i++) {
                if (i == 7) {
                    continue;
                }
                chipIds.push_back(i + (modId << 4));
            }
        }
        if ((AType == TYPE_HALFSTAVE) || (AType == TYPE_MLHALFSTAVE)) {
            Init(2, chipIds, boardMOSAIC);
        }
        else if (AType == TYPE_MLSTAVE) {
            Init(1, chipIds, boardMOSAIC);
        }
        else {
            Init(1, chipIds, boardRU);
        }
    }
    else if (AType == TYPE_POWER) {
        chipIds.clear();
        Init(1, chipIds, boardMOSAIC);
    }
}

void TConfig::ReadConfigFile(const char *fName) {
    std::string line, param, value;
    bool        Initialised = false;
    int         NChips      = 0;
    int         NModules    = 0;
    int         ModuleId    = DEFAULT_MODULE_ID;
    int         Chip;
    TDeviceType type = TYPE_UNKNOWN;

    std::string filename = fName;
    if (const char *configDir = std::getenv("ALPIDE_TEST_CONFIG")) {
            filename.insert(0, std::string(configDir) + "/");
    }

    std::cout << "looking for config file:" << filename << std::endl;
    std::ifstream infile(filename);

    if (!infile.good()) {
        std::cout << "WARNING: Config file " << filename << " not found, using default configuration."
                << std::endl;
        return;
    }

    // first look for the type of setup in order to initialise config structure
    while ((!Initialised) && std::getline(infile, line)) {
        // remove leading tabs or blanks
        size_t p = line.find_first_not_of(" \t");
        line.erase(0, p);
    
        if ((line.size() == 0) || (line[0] == '#')) continue;
        if (line.find("#") != std::string::npos) line.erase(line.find("#"));
        ParseLine(line, param, value, &Chip);
        if (param.compare("NCHIPS") == 0) {
            NChips = stoi(value);
        }
        std::cout << "NCHIPS: " << NChips << std::endl;
        if (param.compare("NMODULES") == 0) {
            NModules = stoi(value);
        }
        std::cout << "NMODULES: " << NModules << std::endl;
        if (param.compare("MODULE") == 0) {
            ModuleId = stoi(value);
        }
        std::cout << "MODULE: " << ModuleId << std::endl;
        if (param.compare("DEVICE") == 0) {
            type = ReadDeviceType(value);
        }
        std::cout << "DEVICE: " << type << std::endl;
        if ((type != TYPE_UNKNOWN) && ((type != TYPE_TELESCOPE) || (NChips > 0)) &&
            (((type != TYPE_HALFSTAVE) && (type != TYPE_HALFSTAVERU) && (type != TYPE_MLHALFSTAVE) &&
            (type != TYPE_MLSTAVE)) ||
            (NModules > 0))) { // type and nchips has been found (nchips not needed for type chip)
                // SetDeviceType calls the appropriate init method, which in turn calls
                // the constructors for board and chip configs
                if ((type == TYPE_OBHIC) || (type == TYPE_ENDURANCE)) {
                    SetDeviceType(type, ModuleId);
                }
                else if ((type == TYPE_HALFSTAVE) || (type == TYPE_HALFSTAVERU) ||
                    (type == TYPE_MLHALFSTAVE) || (type == TYPE_MLSTAVE)) {
                        SetDeviceType(type, NModules);
                }
                else {
                    SetDeviceType(type, NChips);
                }
                Initialised = true;
        }
    }
    
    // now read the rest
    while (std::getline(infile, line)) {
        // remove leading blanks or white spaces
        size_t p = line.find_first_not_of(" \t");
        line.erase(0, p);
    
        DecodeLine(line);
    }
}

void TConfig::ParseLine(std::string Line, std::string &Param, std::string &Value, int *Chip) {
    std::string tmp;
    std::string sep;
    
    // find the seperating tab or blank, whichever is first
    if (Line.find("\t") != std::string::npos && Line.find("\t") < Line.find(" ")) {
        sep = "\t";
    }
    else if (Line.find(" ") != std::string::npos && Line.find("\t") > Line.find(" ")) {
        sep = " ";
    }
    else {
        return;
    }
    
    Value = Line.substr(Line.find(sep) + 1);
    // remove tabs or blanks from the value
    std::string::iterator end_pos = std::remove(Value.begin(), Value.end(), ' ');
    Value.erase(end_pos, Value.end());
    end_pos = std::remove(Value.begin(), Value.end(), '\t');
    Value.erase(end_pos, Value.end());
    
    if (Line.find("_") == std::string::npos || Line.find("_") > Line.find(sep)) {
        *Chip = -1;
        Param = Line.substr(0, Line.find(sep));
    }
    else {
        tmp   = Line.substr(0, Line.find(sep));
        Param = tmp.substr(0, Line.find("_"));
        *Chip = stoi(tmp.substr(Line.find("_") + 1));
    }
}

void TConfig::DecodeLine(std::string Line) {
    int         Index, ChipStart, ChipStop, BoardStart, BoardStop, HicStart, HicStop;
    std::string Param, Value;
    if ((Line.size() == 0) || (Line[0] == '#')) { // empty Line or comment
        return;
    }

    ParseLine(Line, Param, Value, &Index);
    
    if (Index == -1) {
        ChipStart  = 0;
        ChipStop   = fChipConfigs.size();
        BoardStart = 0;
        BoardStop  = fBoardConfigs.size();
        HicStart   = 0;
    }
    else {
        ChipStart  = (Index < (int)fChipConfigs.size()) ? Index : -1;
        ChipStop   = (Index < (int)fChipConfigs.size()) ? Index + 1 : -1;
        BoardStart = (Index < (int)fBoardConfigs.size()) ? Index : -1;
        BoardStop  = (Index < (int)fBoardConfigs.size()) ? Index + 1 : -1;
    }

    // Todo: correctly handle the number of readout boards
    // currently only one is written
    // Note: having a config file with parameters for the mosaic board, but a setup with a DAQ board
    // (or vice versa) will issue unknown-parameter warnings...
    if (ChipStart >= 0 && ChipStop > 0 && fChipConfigs.at(ChipStart)->IsParameter(Param)) {
        for (int i = ChipStart; i < ChipStop; i++) {
            fChipConfigs.at(i)->SetParamValue(Param, Value);
        }
    }
    else if (BoardStart >= 0 && BoardStop > 0 && fBoardConfigs.at(BoardStart)->IsParameter(Param)) {
        for (int i = BoardStart; i < BoardStop; i++) {
            fBoardConfigs.at(i)->SetParamValue(Param, Value);
        }
    }
    else if (BoardStart >= 0 && Param.compare("ADDRESS") == 0) {
        for (int i = BoardStart; i < BoardStop; i++) {
            if (fBoardConfigs.at(BoardStart)->GetBoardType() == boardMOSAIC) {
                ((TBoardConfigMOSAIC *)fBoardConfigs.at(i))->SetIPaddress(Value.c_str());
            }
        }
    }
    else {
        std::cout << "Warning: Unknown parameter " << Param << std::endl;
    }
}

// write config to file, has to call same function for all sub-configs (chips and boards)
void TConfig::WriteToFile(const char *fName) {}

// std::string TConfig::GetSoftwareVersion() { return VERSION; }
