#pragma once

#include <string>
#include <vector>

#include "StaveProducer.h"
#include "eudaq/Configuration.hh"

#include "TF1.h"

class StaveThresholdScanner {
    public:
        StaveThresholdScanner(
            std::shared_ptr<StaveController> stavePtr)
            : stave(std::move(stavePtr)) {};

        std::map<int,int> 
        ithr_scan(
            std::string path,
            int dv_thr, 
            int n_it,
            int ithr_min,
            int ithr_max);

        std::map<int,int> 
        dv_scan(
            std::string path, 
            int n_it,
            int dv_min,
            int dv_max);

        std::pair<double,double> 
        fit_thr_curve(
            std::vector<std::pair<int,double>> reg_to_hits, 
            TF1& fit_func,
            std::string path);

        std::shared_ptr<StaveController> stave;

    private:
        std::map<int,std::vector<
            std::pair<int,double>>> ithr_map;

        std::map<int,std::vector<
            std::pair<int,double>>> dv_map;

        void prepare_scan(std::map<int,std::vector<
            std::pair<int,double>>>& data_to_init);

        std::pair<int,int> 
        precheck_range(Alpide::TRegister reg, int min, int max);

        void iter_reg(
            Alpide::TRegister reg, 
            std::vector<int> range,
            int n_it,
            std::map<int,std::vector<
                std::pair<int,double>>>& data);
};