#pragma once

#include <string>
#include <vector>

#include "StaveProducer.h"
#include "eudaq/Configuration.hh"

#include "TF1.h"

class StaveFakeHitRateScanner {
    public:
        StaveFakeHitRateScanner(
            std::shared_ptr<StaveController> stavePtr)
            : stave(std::move(stavePtr)) {};

        void fhr_scan(
            std::string path,
            int n_it);

        std::shared_ptr<StaveController> stave;

    private:
        std::map<int, std::vector<std::pair<int,int>>> hit_map;

        void prepare_scan();
};