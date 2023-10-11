#include "MOSAICProducer.h"
#include <fstream>
#include "TPowerBoard.h"
#include "THIC.h"
#include <unordered_map>

MOSAICProducer::MOSAICProducer(const std::string & name, const std::string & runcontrol)
  :eudaq::Producer(name, runcontrol), running(false), configured(false){  
    
}

void MOSAICProducer::DoInitialise(){}

void MOSAICProducer::DoConfigure(){
    uint32_t b_us = 40;     // in 25ns units (40MHz)
    uint32_t b_ms = 40000;  // in 25ns units (40MHz)

    // in case it's a second CONFIG call
    if(this->configured) DoReset(); 
    
    // get root stave configuration
    auto conf = GetConfiguration();
    
    mosaic_id = conf->Get("MOSAIC_ID",    0);
    stats_dir = conf->Get("DATA_LOG_DIR", "");
    
    // set up ALICE-standardized config file
    std::vector<int> chip_ids;
    this->alice_conf = new TConfig(1, chip_ids, TBoardType::boardMOSAIC, TDeviceType::TYPE_IBHIC);
    
    // set up MOSAIC configuration class
    std::string m_ip_addr  = conf->Get("IP_ADDRESS",           "192.168.168.250");
    int  m_tcp_addr        = conf->Get("TCP_PORT_ADDR",        2000);
    int  m_nof_ctrl_ifc    = conf->Get("NOF_CTRL_IFC",         1);
    int  m_ctrl_ifc_ph     = conf->Get("CTRL_IFC_PHASE",       2);
    int  m_ctrl_af_thr     = conf->Get("CTRL_AF_THR",          1250000);
    int  m_ctrl_lat_mode   = conf->Get("CTRL_LATENCY_MODE",    0);
    int  m_ctrl_timeout    = conf->Get("CTRL_TIMEOUT",         0);
    int  m_data_link_pol   = conf->Get("DATA_LINK_POLARITY",   0);
    int  m_trig_del        = conf->Get("TRIG_DELAY",           20);
    int  m_pulse_del       = conf->Get("PULSE_DELAY",          10000);
    int  m_poll_timeout    = conf->Get("POLLING_DATA_TIMEOUT", 500);
    int  m_manchester_dis  = conf->Get("MANCHESTER_DISABLED",  0);
    int  m_trg_src         = conf->Get("EXT_TRG_SRC",          0);
    auto m_data_link_speed = static_cast<Mosaic::TReceiverSpeed>(conf->Get("DATA_LINK_SPEED", 0));
    
    this->mosaic_conf = new TBoardConfigMOSAIC();
    mosaic_conf->SetIPaddress(m_ip_addr.c_str());
    mosaic_conf->SetTCPport(m_tcp_addr);
    mosaic_conf->SetCtrlInterfaceNum(m_nof_ctrl_ifc);
    mosaic_conf->SetCtrlInterfacePhase(m_ctrl_ifc_ph);
    mosaic_conf->SetCtrlAFThreshold(m_ctrl_af_thr);
    mosaic_conf->SetCtrlLatMode(m_ctrl_lat_mode);
    mosaic_conf->SetCtrlTimeout(m_ctrl_timeout);
    mosaic_conf->SetInvertedData(m_data_link_pol);
    mosaic_conf->SetSpeedMode(m_data_link_speed);
    mosaic_conf->SetTriggerDelay(m_trig_del);
    mosaic_conf->SetPulseDelay(m_pulse_del);  
    mosaic_conf->SetPollingDataTimeout(m_poll_timeout);
    mosaic_conf->SetManchesterDisable(m_manchester_dis);
    
    mosaic = new TReadoutBoardMOSAIC(this->alice_conf, this->mosaic_conf);
    
    trg_src = trigExt;
    if(m_trg_src == 0){
        trg_src = trigInt;
    }
    
    // configure MOSAIC triggers
    mosaic->SetTriggerConfig(false, true, m_trig_del, m_pulse_del);
    mosaic->SetTriggerSource(trg_src);

    if(conf->Get("GENERATOR", 0)){
        IPbusUDP* ip_bus = new IPbusUDP(mosaic_conf->GetIPaddress(), mosaic_conf->GetTCPport());
        data_gen         = new MDataGenerator(ip_bus, WbbBaseAddress::dataGenerator);
        gen_ev_size = conf->Get("EV_SIZE",  512);
        gen_ev_del  = conf->Get("EV_DELAY", 100);
        n_gen_trig  = conf->Get("N_TRIG", 100);

        data_gen->setup(gen_ev_size, gen_ev_del, false);
    }

    data_log_path = conf->Get("DATA_LOG_DIR", "-1") + "run_" + std::to_string(MOSAICProducer::GetRunNumber()) + "_data_log.txt";

    configured = true;
}

void MOSAICProducer::RunLoop(){
    std::fstream fout;
    if(data_log_path != "-1"){
        fout.open(data_log_path, std::ios_base::app);
    }

    /* Data Tacking */
    unsigned char* buffer; // the buffer containing the event

    uint32_t b_us = 40;     // in 25ns units (40MHz)
    uint32_t b_ms = 40000;  // in 25ns units (40MHz)

    double delay = 0.99 * 1.0e5;

    buffer = (unsigned char*)malloc(200*1024); // allocates 200 kilobytes

    int return_code = 0;
    int timeout     = 1; // ten seconds

    mosaic->StartRun(); // Activate the data taking 

    data_gen->start();
    usleep(delay);
    data_gen->stop();

    int trig_counter = 0;
    while(running) {
        int nof_read_bytes = 0;
        return_code = mosaic->ReadEventData(nof_read_bytes, buffer);

        std::cout << "Number Of Read Bytes :" << nof_read_bytes << std::endl; // Consume the buffer ...
        if(fout.is_open()) fout << "Number Of Read Bytes :" << nof_read_bytes << '\n';

        std::cout << std::hex;
        for (int ii = 0; ii < nof_read_bytes; ii+=4) {
            unsigned int *aa = (unsigned int*)&buffer[ii];
            
            std::cout << *aa << "  ";
            if(fout.is_open()) fout << *aa << "  ";

            if(!((ii+4) % 64)){
                std::cout << std::endl;
                if(fout.is_open()) fout << "\n";
            }
        }
        std::cout << std::dec << std::endl;
    
        if(return_code > 0){ // we have some thing
            std::cout << "Dimension :" << nof_read_bytes << std::endl; // Consume the buffer ...
            if(fout.is_open()) fout << "Dimension :" << nof_read_bytes << "\n\n";
            sleep(0.5); // wait
        }
        else{ // read nothing, is finished ?
            if(timeout-- == 0) running = false;
            sleep(1);
        }
    
        trig_counter++;

        if(n_gen_trig != -1 && trig_counter >= n_gen_trig){ 
            running = false; 
        }
    }

    mosaic->StopRun(); // Stop run
    if(fout.is_open()) fout.close();
}

void MOSAICProducer::DoStartRun(){
    running = true;
}

void MOSAICProducer::DoStopRun(){
    running = false;
}

void MOSAICProducer::DoReset(){
    running = false;
}

void MOSAICProducer::DoTerminate(){
    running = false;
}