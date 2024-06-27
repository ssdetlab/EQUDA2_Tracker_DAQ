#ifndef DummyProducer_h
#define DummyProducer_h

#include "eudaq/Producer.hh"
#include "unistd.h"

class DummyProducer : public eudaq::Producer {
    public:
        DummyProducer(
        const std::string & name, 
        const std::string & runcontrol);

        void DoInitialise() override;
        void DoConfigure()  override;
        void DoStartRun();
        void DoStopRun();
        void DoReset();
        void DoTerminate();
        
        void RunLoop() override;

    static const uint32_t id_factory = eudaq::cstr2hash("DummyProducer");
  
    private:
        bool running;
        bool configured;
};

namespace{
    auto dummy0 = eudaq::Factory<eudaq::Producer>::
        Register<DummyProducer, 
            const std::string&, 
            const std::string&>(DummyProducer::id_factory);
}

#endif
