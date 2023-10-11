#include "scope_control.h"
#include <serial/serial.h>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

scope_control::scope_control() {}

scope_control::~scope_control()
{
  if (link_init) {
    if (link->isOpen()) link->close();
    link_init = false;
    delete link;
  }
}

std::string scope_control::enumerate_ports()
{
  std::string                             port;
  char                                    text[200];
  std::vector<serial::PortInfo>           devices_found = serial::list_ports();
  std::vector<serial::PortInfo>::iterator iter          = devices_found.begin();

  if (devices_found.size() == 0) throw_ex("No USB devices could be enumerated.\n");
  while (iter != devices_found.end()) {
    serial::PortInfo device = *iter++;
    if (((device.hardware_id.find("0AAD") != std::string::npos) &&
         (device.hardware_id.find("013C") != std::string::npos)) ||
        ((device.hardware_id.find("0aad") != std::string::npos) &&
         (device.hardware_id.find("013c") != std::string::npos)))
      port = device.port;
    snprintf(text, sizeof(text), "(%s, %s, %s)\n", device.port.c_str(), device.description.c_str(),
             device.hardware_id.c_str());
    debug_print(text);
  }
  return port;
}

bool scope_control::open(std::string port, uint32_t baud, uint32_t timeoutms)
{
  if (link_init)
    if (link->isOpen()) link->close();
  try {
    link = new serial::Serial(port, baud, serial::Timeout::simpleTimeout(timeoutms));
  }
  catch (std::exception &e) {
    throw_ex(e.what());
    link_init = true;
    return false;
  }
  link_init = true;
  check_model();
  return true;
}

bool scope_control::open_auto(uint32_t timeoutms)
{
  char text[100];
  if (link_init)
    if (link->isOpen()) link->close();
  std::string port = scope_control::enumerate_ports();
  if (port == "") {
    throw_ex("Scope not detected.\n");
    return false;
  }
  else {
    snprintf(text, sizeof(text), "Opening %s.\n", port.c_str());
    debug_print(text);
    try {
      link = new serial::Serial(port, 9600, serial::Timeout::simpleTimeout(timeoutms));
    }
    catch (std::exception &e) {
      throw_ex(e.what());
      link_init = true;
      return false;
    }
    link_init = true;
    check_model();
    reset();
    return true;
  }
}

void scope_control::close()
{
  if (link_init)
    if (link->isOpen()) link->close();
}

bool scope_control::check_model()
{
  char text[100];
  scope_control::get_model();
  if (model.find("RTB2004") == std::string::npos) {
    snprintf(text, sizeof(text), "Untested model %s. power cycle scope before repeating test.\n",
             model.c_str());
    throw std::runtime_error(text);
    return false;
  }
  return true;
}

void scope_control::write(std::string data)
{
  char text[100];
  if (!link_init) {
    throw_ex("Link not initialized.\n");
    return;
  }
  if (!link->isOpen()) {
    throw_ex("Link not open.\n");
    return;
  }
  size_t bytes_wrote = link->write(data);
  snprintf(text, sizeof(text), "Wrote: %s\n", data.c_str());
  debug_print(text);
  if (bytes_wrote != data.size()) {
    snprintf(text, sizeof(text), "Did not send all bytes, sent %lud of %lud.\n", bytes_wrote,
             data.size());
    throw_ex(text);
    return;
  }
}

void scope_control::write_cmd(std::string data, bool wait)
{
  debug_print("Write cmd");
  scope_control::write("*CLS;*ESE 1\n");
  scope_control::write(data);
  for (int i = 0; i < 4; i++) {
    scope_control::write("*OPC;*ESR?\n");
    std::string val = scope_control::read();
    if ((std::stoi(val) & 1) && (std::stoi(val) != 1))
      printf("ESR bit 0 set but ESR = %i != 1", std::stoi(val));
    if (std::stoi(val) == 1)
      return;
    else
      scope_control::msleep(100);
  }
  if (!wait) return;
  data.resize(data.size() - 1); // remove line feed
  throw_ex(("Written task <" + data + "> did not complete in time.\n").c_str());
}

std::string scope_control::write_query(std::string data)
{
  scope_control::write("*CLS\n");
  scope_control::write(data);
  return scope_control::read();
}

std::string scope_control::read()
{
  char        text[100];
  std::string data;
  if (!link_init) {
    throw_ex("Link not initialized.\n");
    return data;
  }
  if (!link->isOpen()) {
    throw_ex("Link not open.\n");
    return data;
  }
  link->readline(data);
  snprintf(text, sizeof(text), "Read: %s\n", data.c_str());
  debug_print(text);
  if (data.length() == 0) {
    throw_ex("No data returned from read.\n");
    return data;
  }
  data.resize(data.size() - 1); /* Remove linefeed*/
  return data;
}

std::string scope_control::get_model()
{
  model = scope_control::write_query("*IDN?\n");
  return model;
}

void scope_control::reset()
{
  scope_control::write("*CLS;*RST;*OPC?\n");
  msleep(3000);
  for (int i = 0; i < 4; i++) {
    std::string val = scope_control::read();
    if (std::stoi(val) == 1)
      return;
    else
      scope_control::msleep(1000);
  }
  throw_ex("Reset task did not complete in time.\n");
}

void scope_control::cls() { scope_control::write("*CLS\n"); }

bool scope_control::eval_ch(uint8_t ch)
{
  char text[100];
  if ((ch > 4) || (ch < 1)) {
    snprintf(text, sizeof(text), "Incorrect channel specified %d.\n", ch);
    throw_ex(text);
    return false;
  }
  else
    return true;
}

void scope_control::enable_ch(uint8_t ch)
{
  char cmd[20];
  if (!eval_ch(ch)) return;
  snprintf(cmd, sizeof(cmd), "CHAN%1d:STAT ON\n", ch);
  scope_control::write_cmd(cmd);
}

void scope_control::disable_ch(uint8_t ch)
{
  char cmd[20];
  if (!eval_ch(ch)) return;
  snprintf(cmd, sizeof(cmd), "CHAN%d:STAT OFF\n", ch);
  scope_control::write_cmd(cmd);
}

void scope_control::set_vscale_ch(uint8_t ch, double scale)
{
  char cmd[30];
  if (!eval_ch(ch)) return;
  if (scale > 10) {
    throw_ex("Voltage scale too large.\n");
    return;
  }
  if (scale < 1e-3) {
    throw_ex("Voltage scale too small.\n");
    return;
  }
  snprintf(cmd, sizeof(cmd), "CHAN%d:SCAL %e\n", ch, scale);
  scope_control::write_cmd(cmd);
}

void scope_control::set_dc_coupling_ch(uint8_t ch, bool dc_coupling)
{
  char cmd[30];
  if (!eval_ch(ch)) return;
  if (dc_coupling) {
    snprintf(cmd, sizeof(cmd), "CHAN%d:COUP DCL\n", ch);
  }
  else {
    snprintf(cmd, sizeof(cmd), "CHAN%d:COUP ACL\n", ch);
  }
  scope_control::write_cmd(cmd);
}

void scope_control::set_timescale(double time)
{
  char cmd[30];
  if (time > 50) {
    throw_ex("Timescale too large.\n");
    return;
  }
  if (time < 1e-9) {
    throw_ex("Timescale too small.\n");
    return;
  }
  snprintf(cmd, sizeof(cmd), "TIM:SCAL %e\n", time);
  scope_control::write_cmd(cmd);
}

void scope_control::set_trigger_ext()
{
  scope_control::write_cmd("TRIG:A:MODE NORM\n"); // Normal trigger mode
  scope_control::write_cmd("TRIG:A:SOUR EXT\n");  // External trigger
}

void scope_control::set_trigger_slope_rising(bool rising)
{
  if (rising) {
    scope_control::write_cmd("TRIG:A:EDGE:SLOP POS\n"); // Rising edge
  }
  else {
    scope_control::write_cmd("TRIG:A:EDGE:SLOP NEG\n"); // Falling edge
  }
}

void scope_control::set_trigger_position(double time)
{
  char cmd[30];
  snprintf(cmd, sizeof(cmd), "TIM:POS %e\n", time);
  scope_control::write_cmd(cmd);
}

void scope_control::set_ext_trigger_level(double level)
{
  char cmd[30];
  snprintf(cmd, sizeof(cmd), "TRIG:A:LEV5 %e\n", level); // Trigger level
  scope_control::write_cmd(cmd);
}

void scope_control::single_capture()
{
  debug_print("Single capture");
  scope_control::write_cmd("SING\n", false); // Single capture
}

void scope_control::set_math_diff(uint8_t ch_p, uint8_t ch_n)
{
  char cmd[30];
  scope_control::write_cmd("CALC:QMATH:STAT ON\n");           // Enable math
  snprintf(cmd, sizeof(cmd), "CALC:QMAT:SOUR1 CH%d\n", ch_p); // Set ch1 source
  scope_control::write_cmd(cmd);
  snprintf(cmd, sizeof(cmd), "CALC:QMAT:SOUR1 CH%d\n", ch_n); // Set ch2 source
  scope_control::write_cmd(cmd);
  scope_control::write_cmd("CALC:QMAT:OPER SUB\n"); // Sub ch2 from ch1
}

void scope_control::en_measure_math() { scope_control::en_measure(5); }

void scope_control::en_measure_ch(uint8_t ch)
{
  if (!eval_ch(ch))
    return;
  else
    scope_control::en_measure(ch);
}

void scope_control::setup_measure()
{
  scope_control::write_cmd("REFL:REL:MODE TWEN\n");
  scope_control::write_cmd("MEAS1:MAIN PEAK\n"); // Peak to peak
  scope_control::write_cmd("MEAS2:MAIN AMPL\n"); // Amplitude
  scope_control::write_cmd("MEAS3:MAIN RTIM\n"); // Risetime
  scope_control::write_cmd("MEAS4:MAIN FTIM\n"); // Falltime

  scope_control::write_cmd("MEAS1:ENAB ON\n");
  scope_control::write_cmd("MEAS2:ENAB ON\n");
  scope_control::write_cmd("MEAS3:ENAB ON\n");
  scope_control::write_cmd("MEAS4:ENAB ON\n");
}

void scope_control::en_measure(uint8_t ch)
{
  if (ch == 5) { // Set math as source
    scope_control::write_cmd("MEAS1:SOUR MA1\n");
    scope_control::write_cmd("MEAS2:SOUR MA1\n");
    scope_control::write_cmd("MEAS3:SOUR MA1\n");
    scope_control::write_cmd("MEAS4:SOUR MA1\n");
    measure_ch = 5;
  }
  else {
    char cmd[30];
    snprintf(cmd, sizeof(cmd), "MEAS1:SOUR CH%d\n", ch);
    scope_control::write_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "MEAS2:SOUR CH%d\n", ch);
    scope_control::write_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "MEAS3:SOUR CH%d\n", ch);
    scope_control::write_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "MEAS4:SOUR CH%d\n", ch);
    scope_control::write_cmd(cmd);
    measure_ch = ch;
  }
}

void scope_control::get_meas()
{
  scope_control::measures result;
  std::string             value;
  value       = scope_control::write_query("MEAS1:RES?\n");
  result.peak = std::stod(value);
  value       = scope_control::write_query("MEAS2:RES?\n");
  result.amp  = std::stod(value);
  value       = scope_control::write_query("MEAS3:RES?\n");
  result.rtim = std::stod(value);
  value       = scope_control::write_query("MEAS4:RES?\n");
  result.ftim = std::stod(value);
  switch (measure_ch) {
  case 1:
    ch1 = result;
    break;
  case 2:
    ch2 = result;
    break;
  case 3:
    ch3 = result;
    break;
  case 4:
    ch4 = result;
    break;
  case 5:
    math = result;
    break;
  }
}

void scope_control::start_quick_meas() { scope_control::write_cmd("MEAS:AON\n"); }

void scope_control::stop_quick_meas() { scope_control::write_cmd("MEAS:AOFF\n"); }

void scope_control::get_errors() { scope_control::write_query("SYST:ERR:ALL?\n"); }

void scope_control::wait_for_trigger(int timeout_sec)
{
  int count = 0;
  while (scope_control::write_query("ACQ:STAT?\n") != "COMP") // Wait until complete
  {
    if (count == timeout_sec * 100) throw_ex("Timeout reached while waiting for trigger");
    msleep(10);
    count++;
  }
}

void scope_control::get_quick_meas()
{
  std::string value;
  value               = scope_control::write_query("MEAS:ARES?\n");
  int pos             = value.find(',', 0);
  quick_measures.peak = std::stod(value.substr(0, pos));
  pos                 = value.find(',', pos);
  quick_measures.upe  = std::stod(value.substr(0, pos));
  pos                 = value.find(',', pos);
  quick_measures.lpe  = std::stod(value.substr(0, pos));
  pos                 = value.find(',', pos);
  quick_measures.cycr = std::stod(value.substr(0, pos));
  pos                 = value.find(',', pos);
  quick_measures.cycm = std::stod(value.substr(0, pos));
  pos                 = value.find(',', pos);
  quick_measures.per  = std::stod(value.substr(0, pos));
  pos                 = value.find(',', pos);
  quick_measures.freq = std::stod(value.substr(0, pos));
  pos                 = value.find(',', pos);
  quick_measures.rtim = std::stod(value.substr(0, pos));
  pos                 = value.find(',', pos);
  quick_measures.ftim = std::stod(value.substr(0, pos));
}

void scope_control::throw_ex(const char *text)
{
  printf("%s", text);
  // Throw an exception
}

void scope_control::debug_print(const char *text)
{
  if (debug_en) printf("%s", text);
}

void scope_control::msleep(unsigned long milliseconds)
{
#ifdef _WIN32
  Sleep(milliseconds);
#else
  usleep(milliseconds * 1000);
#endif
}
