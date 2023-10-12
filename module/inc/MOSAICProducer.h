#ifndef MOSAICProducer_h
#define MOSAICProducer_h

#include <iostream>
#include <fstream>
#include <algorithm>

#include "eudaq/Producer.hh"

#include "TReadoutBoardMOSAIC.h"

class MOSAICProducer : public eudaq::Producer{
  public:
    MOSAICProducer(const std::string & name, const std::string & runcontrol);
    void DoInitialise() override;
    void DoConfigure()  override;
    void DoStartRun()   override;
    void DoStopRun()    override;
    void DoTerminate()  override;
    void DoReset()      override;
    void RunLoop()      override;

    static const uint32_t id_factory = eudaq::cstr2hash("MOSAICProducer");
  
  private:
    // configurations
    TConfig*            alice_conf;
    TBoardConfigMOSAIC* mosaic_conf;

    // hardware classes
    TReadoutBoardMOSAIC* mosaic;

    MDataGenerator* data_gen;
    TTriggerSource trg_src;
    int gen_ev_size;
    int gen_ev_del;
    int n_gen_trig;
    std::string data_log_path;

    bool running;
    bool configured;

    int mosaic_id;
    
    std::string stats_dir;
};

namespace{
  auto dummy0 = eudaq::Factory<eudaq::Producer>::
    Register<MOSAICProducer, const std::string&, const std::string&>(MOSAICProducer::id_factory);
}

#endif