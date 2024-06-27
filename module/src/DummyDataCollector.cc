#include "DummyDataCollector.h"

void DummyDataCollector::DoReceive(eudaq::ConnectionSPC id, eudaq::EventSP ev) {
    std::vector<std::uint8_t> block = ev->GetBlock(0);

    std::cout << "-------------------------------" << std::endl;
    for (std::uint8_t d : block) {
        std::cout << (unsigned int)d << " ";
    }
    std::cout << std::endl;
    std::cout << "-------------------------------" << std::endl;
};
