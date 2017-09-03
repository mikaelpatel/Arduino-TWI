#include "TWI.h"
#include "Software/TWI.h"
#include "DS1307.h"

Software::TWI<BOARD::D18, BOARD::D19> twi;
DS1307 rtc(twi);

void setup()
{
  Serial.begin(57600);
  while (!Serial);

  time_t now = { 0x00, 0x0, 0x0, 0x10, 0x10, 0x10, 0x10 };
  rtc.set_time(now);

  rtc.enable();
}

void loop()
{
  time_t now;
  if (!rtc.get_time(now)) return;

  Serial.print(F("20"));
  Serial.print(now.year, HEX);
  Serial.print('-');
  Serial.print(now.month, HEX);
  Serial.print('-');
  Serial.print(now.date, HEX);
  Serial.print(' ');
  Serial.print(now.hours, HEX);
  Serial.print(':');
  Serial.print(now.minutes, HEX);
  Serial.print(':');
  Serial.print(now.seconds, HEX);
  Serial.println();

  delay(1000);
}
