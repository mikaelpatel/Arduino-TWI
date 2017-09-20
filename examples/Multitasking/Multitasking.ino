#include "TWI.h"
#include "Driver/AT24CXX.h"
#include "RTC.h"
#include "Driver/DS1307.h"
#include "Scheduler.h"

#if defined(ARDUINO_attiny)
#error Multitasking: attiny boards not supported
#endif

#define USE_SOFTWARE_TWI
// #define USE_HARDWARE_TWI

#if defined(USE_SOFTWARE_TWI)
#include "GPIO.h"
#include "Software/TWI.h"
Software::TWI<BOARD::D6, BOARD::D7> twi;
#elif defined(USE_HARDWARE_TWI)
#include "Hardware/TWI.h"
Hardware::TWI twi;
#endif

AT24C32 eeprom(twi);
DS1307 rtc(twi);

namespace clock {

  void loop()
  {
    struct tm now;
    char buf[32];
    if (!rtc.get_time(now)) return;
    // Serial.print(millis() / 1000.0);
    // Serial.print(F(":clock: "));
    Serial.println();
    Serial.println(isotime_r(&now, buf));
    delay(1000);
  }

};

namespace logger {

  const size_t DATA_MAX = 32;
  int16_t data[DATA_MAX];

  void setup()
  {
    for (size_t i = 0; i < DATA_MAX; i++) data[i] = i;
  }

  void loop()
  {
    for (size_t i = 0; i < DATA_MAX; i++) data[i] += 1;
    int res = eeprom.write(0, &data, sizeof(data));
    Serial.print(millis() / 1000.0);
    Serial.print(F(":logger:write="));
    Serial.println(res);
    delay(5500);
  }

};

void setup()
{
  Serial.begin(57600);
  while (!Serial);

  Scheduler.start(logger::setup, logger::loop);
  Scheduler.startLoop(clock::loop);
}

void loop()
{
  static int latest = -1;
  int16_t data[logger::DATA_MAX];
  int res = eeprom.read(&data, 0, sizeof(data));
  Serial.print(millis() / 1000.0);
  Serial.print(F(":loop:read="));
  Serial.print(res);
  if (data[0] != latest) {
    latest = data[0];
    Serial.print(F(", data[]="));
    for (size_t i = 0; i < logger::DATA_MAX; i++) {
      Serial.print(data[i]);
      Serial.print(' ');
    }
  }
  Serial.println();
  delay(750);
}
