//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2020-12-23 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#include <SPI.h>
#include "FRAM_FM25040B_SPI.h"
#include <AskSinPP.h>
#include <MultiChannelDevice.h>
#include <Register.h>
#include <Button.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Fonts/FreeMono9pt7b.h>
#include "OAPC_I2C.h"


#define CC1101_CS           8 //PB0
#define CC1101_GDO0         6 //PE6 INT6
#define CONFIG_BUTTON_PIN   7 //PE7 INT7
#define MODE_BUTTON_PIN     5 //PE5
#define LED                22 //PD4
#define FRAM_CS            28 //PC0

#define TFT_CS             29 //PC1
#define TFT_RST            30 //PC2
#define TFT_DC             31 //PC3
#define TFT_LED            32 //PC4

#define TFT_BACKLIGHT_TIME 300
#define DISPLAY_BGCOLOR    ST77XX_BLACK
#define DISPLAY_ROTATE     3 // 0 = 0° , 1 = 90°, 2 = 180°, 3 = 270°
#define DISPLAY_LINES      6
#define MAX_TEXT_LENGTH    14

#define PEERS_PER_CHANNEL  4
#define MEASURE_INTERVAL   5

#define FRAM_VALUE_ADDRESS 0x01

using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
  {0xF3, 0x15, 0x01},          // Device ID
  "JPWMO00001",                // Device Serial
  {0xF3, 0x15},                // Device Model
  0x10,                        // Firmware Version
  0x53,                        // Device Type
  {0x01, 0x01}                 // Info Bytes
};

typedef LibSPI<CC1101_CS> SPIType;
typedef Radio<SPIType, CC1101_GDO0> RadioType;
typedef StatusLed<LED> LedType;
typedef AskSin<LedType, NoBattery, RadioType> Hal;
Hal hal;
FM25040B_SPI<FRAM_CS> fram;

class TFTDisplay {
  enum Colors { clWHITE, clRED, clORANGE, clYELLOW, clGREEN, clBLUE };

  class BacklightAlarm : public Alarm {
    TFTDisplay& tft;
    public:
      BacklightAlarm (TFTDisplay& t) : Alarm (0), tft(t) {}
      virtual ~BacklightAlarm () {}

      void trigger (__attribute__ ((unused)) AlarmClock& clock)  {
        tft.disableBacklight();
      }
    } tftBacklightTimer;

private:
  Adafruit_ST7735 tft;
  uint8_t         screenNum;
  bool            backlightOn;
public:
  TFTDisplay() : tftBacklightTimer(*this), tft(TFT_CS,  TFT_DC, TFT_RST), screenNum(0), backlightOn(false) {}
  ~TFTDisplay () {}

  void enableBacklight() {
    digitalWrite(TFT_LED, HIGH);
    backlightOn = true;
  }

  void disableBacklight() {
    digitalWrite(TFT_LED, LOW);
    backlightOn = false;
  }

  void drawLine(uint8_t rowNum, uint8_t colorNum, String text, bool center, bool clear) {
    if (clear) clearLine(rowNum);
    drawText(rowNum, colorNum, text, center);
  }

  void clearLine(uint8_t rowNum) {
    int8_t rowOffset = ((rowNum - 1) * ((tft.height() / DISPLAY_LINES) + 1)) + 1;
    tft.fillRect(0, rowOffset, tft.width(), (tft.height() / DISPLAY_LINES), DISPLAY_BGCOLOR);
  }

  void drawText(uint8_t rowNum, uint8_t colorNum, String text, bool center) {
    switch (colorNum) {
      case clWHITE:
        tft.setTextColor((DISPLAY_BGCOLOR == ST77XX_BLACK) ? 0xFFFF : 0x0000);
        break;
      case clRED:
        tft.setTextColor(0xF800);
        break;
      case clORANGE:
        tft.setTextColor(0xFBE0);
        break;
      case clYELLOW:
        tft.setTextColor(0xFFE0);
        break;
      case clGREEN:
        tft.setTextColor(0x07E0);
        break;
      case clBLUE:
        tft.setTextColor(0x001F);
        break;
      default:
        tft.setTextColor((DISPLAY_BGCOLOR == ST77XX_BLACK) ? 0xFFFF : 0x0000);
    }

    uint8_t leftOffset = 1;
    if (center == true)
      leftOffset = (((MAX_TEXT_LENGTH - text.length())) + 1) * 4.5;

    tft.setCursor(leftOffset, 12 + ((rowNum - 1) * 22));

    tft.print(text);
  }

  void init(uint8_t serial[11]) {
    tft.initR(INITR_BLACKTAB);
    pinMode(TFT_LED, OUTPUT);
    toggleBacklight();
    tft.fillScreen(DISPLAY_BGCOLOR);
    tft.setRotation(DISPLAY_ROTATE);
    tft.setFont(&FreeMono9pt7b);
    tft.setTextWrap(false);

    drawLine(1, clGREEN,  (char*)serial, true, false);
    drawLine(2, clWHITE,  F("--------------"), false, false);
    drawLine(3, clBLUE,   F("AskSinPP"), true, false);
    drawLine(4, clYELLOW, F("V " ASKSIN_PLUS_PLUS_VERSION), true, false);
    drawLine(5, clWHITE,  F("--------------"), false, false);
    drawLine(6, clORANGE, F("Starting..."), true, false);
    screenNum = 0;
  }

  void toggleBacklight() {
    if (backlightOn == false) {
      enableBacklight();
      sysclock.cancel(tftBacklightTimer);
      tftBacklightTimer.set(seconds2ticks(TFT_BACKLIGHT_TIME));
      sysclock.add(tftBacklightTimer);
    } else {
      sysclock.cancel(tftBacklightTimer);
      disableBacklight();
    }

  }

  void showCountValues(uint32_t value, uint32_t initValue) {
    if (backlightOn == true) {
      drawLine(3, clYELLOW, F("counted pulses"), false, screenNum != 1);
      drawLine(4, clWHITE, String(value), true, true);
      drawLine(5, clYELLOW, F("init value"), false, screenNum != 1);
      drawLine(6, clWHITE, String(initValue), true, true);
      screenNum = 1;
    }
  }

  void showLDRValue(uint16_t value) {
    if (backlightOn == true) {
      drawLine(3, clYELLOW, F("LDR Value:"), false, screenNum != 2);
      drawLine(4, clWHITE, String(value), true, true);
      clearLine(5);
      clearLine(6);
      screenNum = 2;
    }
  }

};
TFTDisplay Display;

DEFREGISTER(UReg0, MASTERID_REGS, 0x20, 0x21, DREG_CYCLICINFOMSGDIS)
class UList0 : public RegList0<UReg0> {
  public:
    UList0 (uint16_t addr) : RegList0<UReg0>(addr) {}
    virtual ~UList0 () {}

    bool txInterval (uint16_t value) const { return this->writeRegister(0x20, (value >> 8) & 0xff) && this->writeRegister(0x21, value & 0xff); }
    uint16_t txInterval () const { return (this->readRegister(0x20, 0) << 8) + this->readRegister(0x21, 0); }

    void defaults () {
      clear();
      fram.clear();
      fram.write4ByteValueToFRAM(0, FRAM_VALUE_ADDRESS);
      txInterval(180);
      cyclicInfoMsgDis(0);
    }
};

DEFREGISTER(UReg1, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14)
class UList1 : public RegList1<UReg1> {
  public:
    UList1 (uint16_t addr) : RegList1<UReg1>(addr) {}
    virtual ~UList1 () {}

    bool changeMode (uint8_t value) const { return this->writeRegister(0x08, value & 0xff); }
    uint8_t changeMode () const { return this->readRegister(0x08, 0); }

    bool literPerPulse (uint16_t value) const { return this->writeRegister(0x09, (value >> 8) & 0xff) && this->writeRegister(0x0a, value & 0xff); }
    uint16_t literPerPulse () const { return (this->readRegister(0x09, 0) << 8) + this->readRegister(0x0a, 0); }

    bool oapcAnalogLowThreshold (uint16_t value) const { return this->writeRegister(0x0b, (value >> 8) & 0xff) && this->writeRegister(0x0c, value & 0xff); }
    uint16_t oapcAnalogLowThreshold () const { return (this->readRegister(0x0b, 0) << 8) + this->readRegister(0x0c, 0); }

    bool oapcAnalogHighThreshold (uint16_t value) const { return this->writeRegister(0x0d, (value >> 8) & 0xff) && this->writeRegister(0x0e, value & 0xff); }
    uint16_t oapcAnalogHighThreshold () const { return (this->readRegister(0x0d, 0) << 8) + this->readRegister(0x0e, 0); }

    bool oapcPulseDelayMS (uint16_t value) const { return this->writeRegister(0x0f, (value >> 8) & 0xff) && this->writeRegister(0x10, value & 0xff); }
    uint16_t oapcPulseDelayMS () const { return (this->readRegister(0x0f, 0) << 8) + this->readRegister(0x10, 0); }

    bool initialCountValue (uint32_t value) const {
      return
          this->writeRegister(0x11, (value >> 24) & 0xff) &&
          this->writeRegister(0x12, (value >> 16) & 0xff) &&
          this->writeRegister(0x13, (value >> 8) & 0xff) &&
          this->writeRegister(0x14, (value) & 0xff)
          ;
    }

    uint32_t initialCountValue () const {
      return
          ((uint32_t)(this->readRegister(0x11, 0)) << 24) +
          ((uint32_t)(this->readRegister(0x12, 0)) << 16) +
          ((uint32_t)(this->readRegister(0x13, 0)) << 8) +
          ((uint32_t)(this->readRegister(0x14, 0)))
          ;
    }

   void defaults () {
      clear();
      changeMode(RISING);
      literPerPulse(1);
      oapcAnalogLowThreshold(300);
      oapcAnalogHighThreshold(900);
      oapcPulseDelayMS(500);
      initialCountValue(0);
    }
};

class MeasureEventMsg : public Message {
  public:
    void init(uint8_t msgcnt, uint32_t value) {
      Message::init(0x0d, msgcnt, 0x53, BCAST, (value >> 24)  & 0xff, (value >> 16) & 0xff);
      pload[0] = (value >> 8) & 0xff;
      pload[1] = (value) & 0xff;
    }
};

class MeasureChannel : public Channel<Hal, UList1, EmptyList, List4, PEERS_PER_CHANNEL, UList0>, public Alarm {
    bool            calibrationMode;
    bool            oapcError;
    bool            framError;
    uint8_t         dismissCount;
    uint16_t        trg_count;
    uint16_t        literPerPulse;
    uint32_t        value;
    uint32_t        lastValue;
    uint32_t        initValue;
    MeasureEventMsg msg;
    OAPC_I2C<>      oapc;

  public:
    MeasureChannel () : Channel(), Alarm(0), calibrationMode(false), oapcError(false), framError(false), dismissCount(0), trg_count(0), literPerPulse(0), value(0), lastValue(0), initValue(0) {}
    virtual ~MeasureChannel () {}

    void setCountValue(uint32_t val) {
      value = val;
      if (val == 0xFFFFFFFF) framError = true;
    }

    void toggleCalibrationMode() {
      if (calibrationMode == false) {
        calibrationMode = true;
        oapc.calibrationMode(true);
        sysclock.cancel(*this);
        set(millis2ticks(500));
        sysclock.add(*this);
      } else {
        calibrationMode = false;
        oapc.calibrationMode(false);
      }
    }

    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
      if (calibrationMode == false) {
        set(seconds2ticks(MEASURE_INTERVAL));
        trg_count++;

        uint16_t oapcCountValue = oapc.readCountValue(true);  // read values (with reset to zero)
        //DPRINT("oapcCountValue = "); DDECLN(oapcCountValue);

        if (oapcCountValue < 0xFFFF) {
          value += (literPerPulse * oapcCountValue);
          fram.write4ByteValueToFRAM(value, FRAM_VALUE_ADDRESS);
        } else {
          oapcError = true;
          this->changed(true);
        }

        Display.showCountValues(value, initValue);

        if ( trg_count >= max(10, device().getList0().txInterval()) / MEASURE_INTERVAL ) {

          uint32_t valueToSend = value + initValue;

          uint8_t cyclicInfoMsgDis = device().getList0().cyclicInfoMsgDis();

          if (valueToSend == lastValue) dismissCount++;

          if (valueToSend != lastValue || dismissCount > cyclicInfoMsgDis) {
            msg.init(device().nextcount(), valueToSend);
            device().broadcastEvent(msg);
            dismissCount = 0;
          }

          trg_count = 0;
          lastValue = valueToSend;
        }

        DPRINT("value=");DDEC(value);DPRINT(", initValue = ");DDECLN(initValue);
      } else {
        set(millis2ticks(500));
        uint16_t ldrValue = oapc.getCurrentLDRValue();
        Display.showLDRValue(ldrValue);
        DPRINT("ldrValue = ");DDECLN(ldrValue);
      }

      sysclock.add(*this);
    }

    void configChanged() {
      uint16_t oapcAnalogLowThreshold  = oapc.readAnalogLowThreshold();
      uint16_t oapcAnalogHighThreshold = oapc.readAnalogHighThreshold();
      uint16_t oapcPulseDelayMS        = oapc.readPulseDelayMS();
      uint8_t  oapcChangeMode          = oapc.readChangeMode();

      uint16_t list1AnalogLowThreshold  = this->getList1().oapcAnalogLowThreshold();
      uint16_t list1AnalogHighThreshold = this->getList1().oapcAnalogHighThreshold();
      uint16_t list1PulseDelayMS        = this->getList1().oapcPulseDelayMS();
      uint8_t  list1ChangeMode          = this->getList1().changeMode();

      DPRINT("oapcAnalogLowThreshold   = "); DDECLN(oapcAnalogLowThreshold);
      DPRINT("oapcAnalogHighThreshold  = "); DDECLN(oapcAnalogHighThreshold);
      DPRINT("oapcPulseDelayMS         = "); DDECLN(oapcPulseDelayMS);
      DPRINT("oapcChangeMode           = "); DDECLN(oapcChangeMode);
      DPRINT("list1AnalogLowThreshold  = ");DDECLN(list1AnalogLowThreshold);
      DPRINT("list1AnalogHighThreshold = ");DDECLN(list1AnalogHighThreshold);
      DPRINT("list1PulseDelayMS        = ");DDECLN(list1PulseDelayMS);
      DPRINT("list1ChangeMode          = ");DDECLN(list1ChangeMode);


      if (oapcAnalogLowThreshold != list1AnalogLowThreshold) {
        bool ok = oapc.setAnalogLowThreshold(list1AnalogLowThreshold);
        DPRINT("set oapcAnalogLowThreshold to ");DDEC(list1AnalogLowThreshold); DPRINT(" is ");DDECLN(ok);
      }

      if (oapcAnalogHighThreshold != list1AnalogHighThreshold) {
        bool ok = oapc.setAnalogHighThreshold(list1AnalogHighThreshold);
        DPRINT("set oapcAnalogHighThreshold to ");DDEC(list1AnalogHighThreshold); DPRINT(" is ");DDECLN(ok);
      }

      if (oapcPulseDelayMS != list1PulseDelayMS) {
        bool ok = oapc.setPulseDelayMS(list1PulseDelayMS);
        DPRINT("set oapcPulseDelayMS to ");DDEC(list1PulseDelayMS); DPRINT(" is ");DDECLN(ok);
      }

      if (oapcChangeMode != list1ChangeMode) {
        bool ok = oapc.setChangeMode(list1ChangeMode);
        DPRINT("set oapcChangeMode to ");DDEC(list1ChangeMode); DPRINT(" is ");DDECLN(ok);
      }

      initValue = this->getList1().initialCountValue();
      DPRINT("initial value            = ");DDECLN(initValue);

      literPerPulse = this->getList1().literPerPulse();
      DPRINT("liter per pulse          = ");DDECLN(literPerPulse);
    }

    void setup(Device<Hal, UList0>* dev, uint8_t number, uint16_t addr) {
      Channel::setup(dev, number, addr);
      sysclock.add(*this);
      if (oapc.init() == false) oapcError = true;
    }

    uint8_t status () const { return 0; }

    uint8_t flags () const {
      uint8_t flg = 0x00;
      if (oapcError == true) flg |= 0x01 << 1;
      if (framError == true) flg |= 0x01 << 2;
      return flg;
    }
};

class UType : public MultiChannelDevice<Hal, MeasureChannel, 1, UList0> {
  public:
    typedef MultiChannelDevice<Hal, MeasureChannel, 1, UList0> TSDevice;
    UType(const DeviceInfo& info, uint16_t addr) : TSDevice(info, addr) {}
    virtual ~UType () {}

    void init(Hal& hal) {
      TSDevice::init(hal);
    }

    virtual void configChanged () {
      TSDevice::configChanged();
      DPRINT(F("*txInterval: ")); DDECLN(this->getList0().txInterval());
      DPRINT(F("*cyclicDisM: ")); DDECLN(this->getList0().cyclicInfoMsgDis());
    }
};

UType sdev(devinfo, 0x20);
ConfigButton<UType> cfgBtn(sdev);

class ModeBtn : public Button  {
  TFTDisplay& disp;
  UType& dev;
public:
  typedef Button ButtonType;
  ModeBtn (TFTDisplay& d, UType& i) : disp(d), dev(i) {
    this->setLongPressTime(seconds2ticks(2));
  }
  virtual ~ModeBtn () {}

  virtual void state (uint8_t s) {
    ButtonType::state(s);
    if( s == ButtonType::released ) {
      disp.toggleBacklight();
    }
    else if( s == ButtonType::longpressed ) {
      dev.channel(1).toggleCalibrationMode();
    }
  }
};

ModeBtn modeBtn(Display, sdev);

void setup() {
  DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);

  uint8_t framInitResult = fram.init();
  DPRINT("FRAM Init: ");DHEXLN(framInitResult);

  sdev.init(hal);

  uint8_t serial[11];
  sdev.getDeviceSerial(serial);
  serial[10] = 0;
  Display.init(serial);

  uint32_t savedCountValue = framInitResult == 0x09 ? fram.read4ByteValueFromFRAM(0x1) : 0xFFFFFFFF;
  DPRINT("FRAM savedCountValue = ");DDECLN(savedCountValue);
  sdev.channel(1).setCountValue(savedCountValue);

  buttonISR(modeBtn, MODE_BUTTON_PIN);
  buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
  sdev.initDone();

  sdev.channel(1).changed(true);

}

void loop() {
  hal.runready();
  sdev.pollRadio();
}
