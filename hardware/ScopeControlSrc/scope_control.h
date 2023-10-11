#pragma once
#include <serial/serial.h>
#include <string>

class scope_control {
public:
  scope_control();
  ~scope_control();
  bool        open(std::string port, uint32_t baud = 9600, uint32_t timeoutms = 1500);
  bool        open_auto(uint32_t timeoutms = 1500);
  void        close();
  std::string enumerate_ports();
  void        write(std::string data);
  void        write_cmd(std::string data, bool wait = true);
  std::string write_query(std::string data);
  std::string read();
  std::string get_model();
  std::string model;
  void        reset();
  void        cls();
  void        enable_ch(uint8_t ch);
  void        disable_ch(uint8_t ch);
  void        set_vscale_ch(uint8_t ch, double scale);
  void        set_dc_coupling_ch(uint8_t ch, bool dc_coupling);
  void        set_timescale(double time);
  void        start_quick_meas();
  void        stop_quick_meas();
  void        get_quick_meas();
  void        set_trigger_ext();
  void        set_trigger_slope_rising(bool rising);
  void        set_trigger_position(double time);
  void        set_ext_trigger_level(double level);
  void        set_math_diff(uint8_t ch_p, uint8_t ch_n);
  void        setup_measure();
  void        en_measure_math();
  void        en_measure_ch(uint8_t ch);
  void        get_meas();
  void        single_capture();
  void        wait_for_trigger(int timeout_sec);
  void        get_errors();
  bool        debug_en = false;
  // Returned by get quick measurments
  struct quick_measures {
    double peak; // Peak to peak
    double upe;  // Vp+
    double lpe;  // Vp-
    double cycr; // RMS cycl
    double cycm; // Mean cycl
    double per;  // Period
    double freq; // Frequency
    double rtim; // Risetime
    double ftim; // Falltime
  };
  struct measures {
    double peak; // Peak to peak
    double amp;  // Amplitude
    double rtim; // Risetime
    double ftim; // Falltime
  };
  measures       ch1;
  measures       ch2;
  measures       ch3;
  measures       ch4;
  measures       math;
  quick_measures quick_measures;

private:
  void            msleep(unsigned long milliseconds);
  serial::Serial *link;
  bool            link_init = 0;
  void            throw_ex(const char *);
  void            debug_print(const char *);
  bool            check_model();
  bool            eval_ch(uint8_t ch);
  void            en_measure(uint8_t ch);
  uint8_t         measure_ch = 0;
};
