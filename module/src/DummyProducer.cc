#include "DummyProducer.h"

DummyProducer::DummyProducer(
const std::string & name, 
const std::string & runcontrol)
    : eudaq::Producer(name, runcontrol), 
    running(false), configured(false) {};

void DummyProducer::DoInitialise() {};

void DummyProducer::DoConfigure()  {
    configured = true;
};

void DummyProducer::DoStartRun() {
    running = true;
}

void DummyProducer::DoStopRun() {
    running = false;
}

void DummyProducer::DoReset() {
    configured = false;
}

void DummyProducer::DoTerminate() {
    running = false;
}

void DummyProducer::RunLoop() {
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
