#ifndef TelescopeEvent_h
#define TelescopeEvent_h
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include <TObject.h>

class chip_event{
  public:
    // hit map of the chip in the event
    std::vector<std::tuple<int,int>> hits;

    // ALPIDE readout flags
    bool is_flushed_incomplete;
    bool is_strobe_extended;
    bool is_busy_transition;

    // chip identificators
    unsigned int chip_id;
    unsigned int channel;

    ClassDef(chip_event, 2);
};

class tlu_event{
  public:
    // fine trigger timestamp in 
    // 1.5 ns units showing
    // relative time of trigger
    // arrival 
    unsigned int fine_ts_0;
    unsigned int fine_ts_1;
    unsigned int fine_ts_2;
    unsigned int fine_ts_3;
    unsigned int fine_ts_4;
    unsigned int fine_ts_5;

    // number of triggers pre-veto (e.g. 
    // counts triggers under DUT-asserted 
    // busy)
    unsigned int particles;
    
    // event counnters in each trgger 
    // input (above threshold but not 
    // necessary unmasked)
    unsigned int scaler_0;
    unsigned int scaler_1;
    unsigned int scaler_2;
    unsigned int scaler_3;
    unsigned int scaler_4;
    unsigned int scaler_5;

    // shows trigger inputs
    // active in an event
    std::string trg_sign;

    // on of the following event types: 
    // 0000 trigger internal; 0001 trigger external
    // 0010 shutter falling;  0011 shutter rising
    // 0100 edge falling;     0101 edge rising
    // 0111 spill on;         0110 spill off
    std::string type;

    // event trigger id
    unsigned int trg_n;

    // event timestamp in ns
    unsigned int event_begin;
    unsigned int event_end;

    ClassDef(tlu_event, 1);
};

class stave_event{
  public:
    // chip event storage
    std::vector<chip_event> ch_ev_buffer;

    // stave identificator
    int stave_id;

    // MOSAIC readout flags    
    int  channel;
    bool end_of_run;
    bool overflow;

    ClassDef(stave_event, 1);
};

class detector_event{
  public:
    // stave event storage
    std::vector<stave_event> st_ev_buffer;
    
    // event time identificators  
    unsigned int trg_n;
    unsigned int event_begin;
    unsigned int event_end;

    ClassDef(detector_event, 2);
};

class detector_event_tlu{
  public:
    // stave event storage
    std::vector<stave_event> st_ev_buffer;

    // to sync with the tlu
    tlu_event tl_ev;

    // MOSAIC readout flags

    ClassDef(detector_event_tlu, 2);
};

#endif