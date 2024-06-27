#ifndef StaveProducer_h
#define StaveProducer_h

#include <iostream>
#include <fstream>
#include <algorithm>

#include "eudaq/Producer.hh"

#include "TReadoutBoardMOSAIC.h"
#include "TAlpide.h"
#include "AlpideConfig.h"

class TAlpide;

struct StaveController {
    StaveController() = default;

    // configurations
    TConfig* alice_conf;
    TBoardConfigMOSAIC* mosaic_conf;
    std::map<std::uint8_t,TChipConfig*> chip_confs;

    // hardware classes
    TReadoutBoardMOSAIC* mosaic;
    std::map<std::uint8_t,TAlpide*> chips;

    std::uint8_t stave_id = 0;
    int RCVMAP[9] = {3, 5, 7, 8, 6, 4, 2, 1, 0};

    std::string stats_dir = "";

    // configuration methods
    void setup_mosaic(std::string conf_dir);
    void setup_chips(std::string conf_dir); 
    void setup_chip_conf(
        eudaq::Configuration c_conf, 
        std::uint8_t chip_id);

    // bad pixels masking
    void config_bad_pixels(
        std::string path);
};

class StaveProducer : public eudaq::Producer {
    public:
        StaveProducer(const std::string & name, 
            const std::string & runcontrol);
        void DoInitialise() override;
        void DoConfigure()  override;
        void DoStartRun()   override;
        void DoStopRun()    override;
        void DoTerminate()  override;
        void DoReset()      override;
        void RunLoop()      override;

    static const uint32_t id_factory = eudaq::cstr2hash("StaveProducer");
  
    private:
        bool running;
        bool configured;

        std::shared_ptr<StaveController> control;
};

namespace{
    auto dummy0 = eudaq::Factory<eudaq::Producer>::
        Register<StaveProducer, 
            const std::string&, 
            const std::string&>(StaveProducer::id_factory);
}

#endif
