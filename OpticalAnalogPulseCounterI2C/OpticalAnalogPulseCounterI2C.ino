//- -----------------------------------------------------------------------------------------------------------------------
// 2020-12-23 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- --------------

#include <Wire.h>
#include <avr/eeprom.h>

#define I2C_ADDRESS                          0xF3
#define VERSION                              0xA0

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

#define LED_PIN                  LED_BUILTIN
#define ANALOG_INPUT_PIN         A0

bool oldPinState = HIGH;
bool LDRstate = LOW;

unsigned long pulseDiffMillis = 0;

uint16_t currentLDRValue = 0;
uint16_t countsSinceLastRequest = 0;

int  i2c_snd = 0;
bool requested = false;
bool calibrationMode = false;

uint8_t i2cRequestCommand;

struct settings_t {
  uint16_t analogHighThreshold;
  uint16_t analogLowThreshold;
  uint16_t pulseDelayMilliSeconds;
} settings;

void readSettings() {
  eeprom_read_block((void*)&settings, (void*)0, sizeof(settings));
  if ( (settings.analogHighThreshold == 0xffff) && (settings.analogLowThreshold == 0xffff) && (settings.pulseDelayMilliSeconds == 0xffff)) resetSettings();
}

void dumpSettings() {
  Serial.print(F("analogHighThreshold    "));Serial.println(settings.analogHighThreshold);
  Serial.print(F("analogLowThreshold     "));Serial.println(settings.analogLowThreshold);
  Serial.print(F("pulseDelayMilliSeconds "));Serial.println(settings.pulseDelayMilliSeconds);
}

void resetSettings() {
  Serial.println(F("reset_settings"));
  settings.pulseDelayMilliSeconds = 500;
  settings.analogHighThreshold    = 900;
  settings.analogLowThreshold     = 100;
  eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));
  //dumpSettings();
}

void onDataReceive(int numBytes) {
  uint8_t i2cReceiveBuffer[8];
  uint8_t count = 0;
  while (Wire.available() || count < numBytes ) {
    i2cReceiveBuffer[count] = Wire.read();
    count++;
  }

  if (count == 1) {
    i2cRequestCommand = i2cReceiveBuffer[0];
  } else {
    switch (i2cReceiveBuffer[0]) {
    case I2C_CMD_SET_ANALOG_LOW_THRESHOLD:
      settings.analogLowThreshold = (i2cReceiveBuffer[1] << 8);
      settings.analogLowThreshold += i2cReceiveBuffer[2];
      eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));
      dumpSettings();
      break;
    case I2C_CMD_SET_ANALOG_HIGH_THRESHOLD:
      settings.analogHighThreshold = (i2cReceiveBuffer[1] << 8);
      settings.analogHighThreshold += i2cReceiveBuffer[2];
      eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));
      dumpSettings();
      break;
    case I2C_CMD_SET_PULSE_DELAY_MS:
      settings.pulseDelayMilliSeconds = (i2cReceiveBuffer[1] << 8);
      settings.pulseDelayMilliSeconds += i2cReceiveBuffer[2];
      eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));
      dumpSettings();
      break;
    case I2C_CMD_SET_CALIBRATIONMODE:
      calibrationMode = (i2cReceiveBuffer[1] == 1);
      break;
    default:
      Serial.print("unknown command 0x");Serial.println(i2cRequestCommand, HEX);
      break;
    }
  }
}

void onDataRequest() {
  if (i2cRequestCommand > 0) {
    switch (i2cRequestCommand) {
    case I2C_CMD_GET_ANALOG_LOW_THRESHOLD:
      Wire.write(settings.analogLowThreshold >> 8 & 0xFF);
      Wire.write(settings.analogLowThreshold & 0xFF);
      break;
    case I2C_CMD_GET_ANALOG_HIGH_THRESHOLD:
      Wire.write(settings.analogHighThreshold >> 8 & 0xFF);
      Wire.write(settings.analogHighThreshold & 0xFF);
      break;
    case I2C_CMD_GET_PULSE_DELAY_MS:
      Wire.write(settings.pulseDelayMilliSeconds >> 8 & 0xFF);
      Wire.write(settings.pulseDelayMilliSeconds & 0xFF);
      break;
    case I2C_CMD_GET_CALIBRATIONMODE:
      Wire.write((uint8_t)calibrationMode);
      break;
    case I2C_CMD_GET_CURRENT_LDR_VALUE:
      Wire.write(currentLDRValue >> 8 & 0xFF);
      Wire.write(currentLDRValue & 0xFF);
      break;
    case I2C_CMD_GET_COUNT_WO_RESET:
      Wire.write(countsSinceLastRequest >> 8 & 0xFF);
      Wire.write(countsSinceLastRequest & 0xFF);
      break;
    case I2C_CMD_GET_COUNT_W_RESET:
      Wire.write(countsSinceLastRequest >> 8 & 0xFF);
      Wire.write(countsSinceLastRequest & 0xFF);
      countsSinceLastRequest = 0;
      break;
    case I2C_CMD_GET_VERSION:
      Wire.write((uint8_t)VERSION);
      break;
    case I2C_CMD_RESET_COUNT:
       countsSinceLastRequest = 0;
       Wire.write(1);
    break;
    case I2C_CMD_RESET_EEPROM:
       resetSettings();
       Wire.write(1);
    break;
    default:
      Serial.print("unknown command 0x");Serial.println(i2cRequestCommand, HEX);
      break;
    }
  }

  i2cRequestCommand = 0;
}

void setup() {
  Wire.begin(I2C_ADDRESS);

  Serial.begin(57600); Serial.print("\nOAPC_I2C Start at 0x");Serial.println(I2C_ADDRESS, HEX);

  pinMode(ANALOG_INPUT_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  readSettings();

  //dumpSettings();

  Wire.onReceive(onDataReceive);
  Wire.onRequest(onDataRequest);
}

void loop() {
  currentLDRValue = analogRead(ANALOG_INPUT_PIN);

  if (currentLDRValue > settings.analogHighThreshold) LDRstate = HIGH;
  if (currentLDRValue < settings.analogLowThreshold)  LDRstate = LOW;

  if (LDRstate == HIGH) {
    if (oldPinState == LOW) {
      if ((millis() - pulseDiffMillis > settings.pulseDelayMilliSeconds) ) {
        if (calibrationMode == false)
          countsSinceLastRequest++;
        else
          digitalWrite(LED_PIN, HIGH);
      }
      oldPinState = HIGH;
    }
  } else {
    if (oldPinState == HIGH) {
      digitalWrite(LED_PIN, LOW);
      oldPinState = LOW;
      pulseDiffMillis = millis();
    }
  }
}
