#ifndef DummyDataCollector_h
#define DummyDataCollector_h

#include "eudaq/DataCollector.hh"

class DummyDataCollector : public eudaq::DataCollector {
    public:
        using eudaq::DataCollector::DataCollector;

        void DoStopRun() override {};
        void DoConfigure() override {};
        void DoReceive(eudaq::ConnectionSPC id, eudaq::EventSP ev) override;
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