#include "TWI.h"
#include "Software/TWI.h"
#include "DS1307.h"

Software::TWI<BOARD::D18, BOARD::D19> twi;
DS1307 rtc(twi);

void setup()
{
  Serial.begin(57600);
  while (!Serial);

  // Set real-time clock and enable square-wave signal
  struct tm now(THURSDAY, 2018, SEPTEMBER, 13, 13, 35, 0);
  rtc.set_time(now);
  rtc.enable();
}

void loop()
{
  // Read real-time clock and print time every second
  struct tm now;
  if (!rtc.get_time(now)) return;
  Serial.println(isotime(&now));
  delay(1000);
}

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
