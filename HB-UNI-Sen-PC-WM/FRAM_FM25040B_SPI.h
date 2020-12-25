//- -----------------------------------------------------------------------------------------------------------------------
// 2020-12-23 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------


#ifndef FRAM_FM25040B_SPI_H_
#define FRAM_FM25040B_SPI_H_

#include <Adafruit_SPIDevice.h>
#include <Arduino.h>
#include <SPI.h>

template <uint8_t CS>
class FM25040B_SPI {
private:

  Adafruit_SPIDevice fram;

  typedef enum opcodes_e {
    OPCODE_WREN  = 0b0110,    /* Write Enable Latch */
    OPCODE_WRDI  = 0b0100,    /* Reset Write Enable Latch */
    OPCODE_RDSR  = 0b0101,    /* Read Status Register */
    OPCODE_WRSR  = 0b0001,    /* Write Status Register */
    OPCODE_READ  = 0b0011,    /* Read Memory */
    OPCODE_WRITE = 0b0010,    /* Write Memory */
  } opcodes_t;

  void setStatusRegister(uint8_t value) {
    uint8_t cmd[2];
    cmd[0] = OPCODE_WRSR;
    cmd[1] = value;
    fram.write(cmd, 2);
  }

  uint8_t getStatusRegister() {
    uint8_t cmd, val;
    cmd = OPCODE_RDSR;
    fram.write_then_read(&cmd, 1, &val, 1);
    return val;
  }

  void writeEnable(bool enable) {
    uint8_t cmd =enable ? OPCODE_WREN : OPCODE_WRDI;
    fram.write(&cmd, 1);
  }

  void write8(uint8_t addr, uint8_t value) {
    //Serial.print("write8 to address ");Serial.print(addr,DEC); Serial.print(", val ");Serial.println(value,DEC);
    uint8_t buffer[10];
    uint8_t i = 0;

    buffer[i++] = OPCODE_WRITE;
    buffer[i++] = addr & 0xFF;
    buffer[i++] = value;

    fram.write(buffer, i);
  }

  uint8_t read8(uint8_t addr) {
    uint8_t buffer[10], val;
    uint8_t i = 0;

    buffer[i++] = OPCODE_READ;
    buffer[i++] = addr & 0xFF;

    fram.write_then_read(buffer, i, &val, 1);

    return val;
  }

public:
  FM25040B_SPI() : fram(CS, 1000000, SPI_BITORDER_MSBFIRST, SPI_MODE0, &SPI)  {}
  ~  FM25040B_SPI() {}

  uint8_t init() {
    if (!fram.begin()) {
      return false;
    }

    return readByteFromFRAM(0x00);
  }

  void write4ByteValueToFRAM(uint32_t val, uint8_t  addr) {
    writeByteToFRAM(val >> 24 & 0xFF, addr);
    writeByteToFRAM(val >> 16 & 0xFF, addr+1);
    writeByteToFRAM(val >> 8  & 0xFF, addr+2);
    writeByteToFRAM(val       & 0xFF, addr+3);
  }

  uint32_t read4ByteValueFromFRAM(uint8_t addr) {
    uint32_t value = (uint32_t)read8(addr++) << 24;
    value += (uint32_t)read8(addr++) << 16;
    value += (uint32_t)read8(addr++) << 8;
    value += (uint32_t)read8(addr);
    return value;
  }

  uint8_t readByteFromFRAM(uint8_t addr) {
    return read8(addr);
  }

  void writeByteToFRAM(uint8_t val, uint8_t  addr) {
    writeEnable(true);
    write8(addr, val);
    writeEnable(false);
  }

  void clear() {
    Serial.println("FRAM clear");
    for (uint8_t i = 0; i < 255; i++) {
      writeByteToFRAM(0xFF, i);
    }
    writeByteToFRAM(0x09, 0x00);
  }

  void dump() {
    for (uint8_t a = 0; a < 255; a++) {
      uint8_t value = readByteFromFRAM(a);
      if ((a % 32) == 0) {
        Serial.print("\n 0x"); Serial.print(a, HEX); Serial.print(": ");
      }
      Serial.print("0x");
      if (value < 0x1)
        Serial.print('0');
      Serial.print(value, HEX); Serial.print(" ");
    }
  }

};




#endif /* FRAM_FM25040B_SPI_H_ */
