#include "Scheduler.h"
#include "TWI.h"
#include "Software/TWI.h"
#include "Driver/DS1307.h"
#include "Driver/AT24CXX.h"

#if defined(ARDUINO_attiny)
#include "Software/Serial.h"
Software::Serial<BOARD::D0> Serial;
Software::TWI<BOARD::D1, BOARD::D2> twi;
#else
Software::TWI<BOARD::D18, BOARD::D19> twi;
#endif

AT24C32 eeprom(twi);
DS1307 rtc(twi);

const char* isotime(const struct tm* tm)
{
  static const char FORMAT[] PROGMEM = "%d-%02d-%02d %02d:%02d:%02d";
  static const size_t BUF_MAX = 20;
  static char buf[BUF_MAX];
  sprintf_P(buf, FORMAT,
	    tm->tm_year + 1900,
	    tm->tm_mon + 1,
	    tm->tm_mday,
	    tm->tm_hour,
	    tm->tm_min,
	    tm->tm_sec);
  return (buf);
}

namespace clock {

  void loop()
  {
    struct tm now;
    if (!rtc.get_time(now)) return;
    Serial.print(millis());
    Serial.print(F(":clock: "));
    Serial.println(isotime(&now));
    delay(1000);
  }

};

namespace logger {

  const size_t DATA_MAX = 32;
  int data[DATA_MAX];

  void setup()
  {
    for (size_t i = 0; i < DATA_MAX; i++) data[i] = i;
  }

  void loop()
  {
    for (size_t i = 0; i < DATA_MAX; i++) data[i] += 1;
    int res = eeprom.write(0, &data, sizeof(data));
    Serial.print(millis());
    Serial.print(F(":logger:write="));
    Serial.println(res);
    delay(5000);
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
  int data[logger::DATA_MAX];
  int res = eeprom.read(&data, 0, sizeof(data));
  Serial.print(millis());
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
  delay(700);
}
