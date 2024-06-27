#ifndef DummyDataCollector_h
#define DummyDataCollector_h

#include "eudaq/DataCollector.hh"

class DummyDataCollector : public eudaq::DataCollector {
    public:
        using eudaq::DataCollector::DataCollector;

        void DoStopRun() override {};
        void DoConfigure() override {};
        void DoReceive(eudaq::ConnectionSPC id, eudaq::EventSP ev) override{
            std::vector<std::uint8_t> block = ev->GetBlock(0);

            std::cout << "-------------------------------" << std::endl;
            for (std::uint8_t d : block) {
                std::cout << (unsigned int)d << " ";
            }
            std::cout << std::endl;
            std::cout << "-------------------------------" << std::endl;
        };
        void DoTerminate() override {};
        void DoReset()   override {};

        static const uint32_t m_id_factory = eudaq::cstr2hash("DummyDataCollector");
    private:
};

namespace {
    auto dummy0 = eudaq::Factory<eudaq::DataCollector>::
        Register<DummyDataCollector, const std::string&, const std::string&>
        (DummyDataCollector::m_id_factory);
}

#endif