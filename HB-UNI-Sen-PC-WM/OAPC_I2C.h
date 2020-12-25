//- -----------------------------------------------------------------------------------------------------------------------
// 2020-12-23 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef OAPC_I2C_H_
#define OAPC_I2C_H_

#include <Wire.h>
#include <Sensors.h>

#define I2C_CMD_GET_ANALOG_LOW_THRESHOLD     0x01
#define I2C_CMD_GET_ANALOG_HIGH_THRESHOLD    0x02
#define I2C_CMD_GET_PULSE_DELAY_MS           0x03
#define I2C_CMD_GET_CALIBRATIONMODE          0x04
#define I2C_CMD_GET_CURRENT_LDR_VALUE        0x20
#define I2C_CMD_GET_COUNT_W_RESET            0x30
#define I2C_CMD_GET_COUNT_WO_RESET           0x31
#define I2C_CMD_GET_VERSION                  0xF0
#define I2C_CMD_RESET_COUNT                  0xFC
#define I2C_CMD_RESET_EEPROM                 0xFD

#define I2C_CMD_SET_ANALOG_LOW_THRESHOLD     0x41
#define I2C_CMD_SET_ANALOG_HIGH_THRESHOLD    0x42
#define I2C_CMD_SET_PULSE_DELAY_MS           0x43
#define I2C_CMD_SET_CALIBRATIONMODE          0x44

namespace as {

template <uint8_t ADDRESS=0xF3>
class OAPC_I2C {
private:
  bool     _present;

  int readSingleByte(uint8_t adr) {
    int t = -1;
    Wire.beginTransmission(ADDRESS);
    Wire.write(adr);
    Wire.endTransmission();
    _delay_ms(2);
    Wire.requestFrom(ADDRESS, (uint8_t)1);
    if (Wire.available() == 1) {
      t = Wire.read();
    }
    return (t);
  }

  int readTwoBytes(uint8_t adr) {
    int t = -1;
    Wire.beginTransmission(ADDRESS);
    Wire.write(adr);
    Wire.endTransmission();
    _delay_ms(2);
    Wire.requestFrom(ADDRESS, (uint8_t)2);
    if (Wire.available() == 2) {
      t = (Wire.read() << 8);
      t += Wire.read();
    }
    return (t);
  }

  void writeSingleByte(uint8_t adr, uint8_t val) {
    Wire.beginTransmission(ADDRESS);
    Wire.write(adr);
    Wire.write(val & 0xFF);
    Wire.endTransmission();
  }

  void writeTwoBytes(uint8_t adr, uint16_t val) {
    Wire.beginTransmission(ADDRESS);
    Wire.write(adr);
    Wire.write(val >> 8 & 0xFF);
    Wire.write(val & 0xFF);
    Wire.endTransmission();
  }

public:
  OAPC_I2C () : _present(false) {}
  ~OAPC_I2C () {}

  bool init () {
    Wire.begin();
    uint8_t version = readSingleByte(I2C_CMD_GET_VERSION);
    DPRINT(F("OAPC Init: "));DHEXLN(version);
    _present = (version >= 0xA0);
    return _present;
  }

  uint16_t readCountValue(bool wReset) {
    if (_present == true) { return readTwoBytes(wReset ? I2C_CMD_GET_COUNT_W_RESET : I2C_CMD_GET_COUNT_WO_RESET); }
    return 0xFFFF;
  }

  bool resetCountValue() {
    return (_present == true) ? readSingleByte(I2C_CMD_RESET_COUNT) == 1 : false;
  }

  bool resetEEPROMSettings() {
    return (_present == true) ? readSingleByte(I2C_CMD_RESET_EEPROM) == 1 : false;
  }

  uint16_t readAnalogLowThreshold() {
    if (_present == true) { return readTwoBytes(I2C_CMD_GET_ANALOG_LOW_THRESHOLD);  }
    return 0xFFFF;
  }

  uint16_t readAnalogHighThreshold() {
    if (_present == true) { return readTwoBytes(I2C_CMD_GET_ANALOG_HIGH_THRESHOLD);  }
    return 0xFFFF;
  }

  uint16_t readPulseDelayMS() {
    if (_present == true) { return readTwoBytes(I2C_CMD_GET_PULSE_DELAY_MS); }
    return 0xFFFF;
  }

  uint16_t getCurrentLDRValue() {
    return (_present == true) ? readTwoBytes(I2C_CMD_GET_CURRENT_LDR_VALUE) : 0xFFFF;
  }

  bool setAnalogLowThreshold(uint16_t val) {
    if (_present == true) {
      writeTwoBytes(I2C_CMD_SET_ANALOG_LOW_THRESHOLD, val);
      readAnalogLowThreshold();
      return (val == readAnalogLowThreshold());
    }
    return false;
  }

  bool setAnalogHighThreshold(uint16_t val) {
    if (_present == true) {
      writeTwoBytes(I2C_CMD_SET_ANALOG_HIGH_THRESHOLD, val);
      readAnalogHighThreshold();
      return (val == readAnalogHighThreshold());
    }
    return false;
  }

  bool setPulseDelayMS(uint16_t val) {
    if (_present == true) {
      writeTwoBytes(I2C_CMD_SET_PULSE_DELAY_MS, val);
      readPulseDelayMS();
      return (val == readPulseDelayMS());
    }
    return false;
  }

  void calibrationMode(bool en) {
    if (_present == true) writeSingleByte(I2C_CMD_SET_CALIBRATIONMODE, en);
  }

  bool calibrationMode() {
    return (_present == true) ? readSingleByte(I2C_CMD_GET_CALIBRATIONMODE) == 1 : false;
  }

};

}

#endif /* OAPC_I2C_H_ */
