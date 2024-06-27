#ifndef DummyProducer_h
#define DummyProducer_h

#include "eudaq/Producer.hh"
#include "unistd.h"

class DummyProducer : public eudaq::Producer {
    public:
        DummyProducer(
        const std::string & name, 
        const std::string & runcontrol)
            : eudaq::Producer(name, runcontrol), 
            running(false), configured(false) {};

        void DoInitialise() override {};
        void DoConfigure()  override {
            configured = true;
        };
        
        void DoStartRun() {
            running = true;
        }
        
        void DoStopRun() {
            running = false;
        }
        
        void DoReset() {
            configured = false;
        }
        
        void DoTerminate() {
            running = false;
        }
        
        void RunLoop() override {
            int k = 0;
            while (running) {
                sleep(1);
                auto ev = eudaq::Event::MakeUnique("DummyRaw");
                ev->SetTag("Test", "BUBUBABA");

                std::vector<std::uint8_t> block{1,2,3,4,5,6};

                ev->AddBlock(0, block);
                
                // This is a dummy trigger ID
                // The true one comes from the TLU
                ev->SetTriggerN(k);
                k++;
                SendEvent(std::move(ev)); 
            }
        };

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
