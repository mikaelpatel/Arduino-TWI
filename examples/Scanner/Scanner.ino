#include "TWI.h"
#include "Software/TWI.h"

Software::TWI<BOARD::D18, BOARD::D19> twi;

void setup()
{
  Serial.begin(57600);
  while (!Serial);
}

void loop()
{
  int i = 0;
  for (uint8_t addr = 3; addr < 128; addr++) {
    TWI::Device dev(twi, addr);
    dev.acquire();
    int res = dev.write(NULL);
    dev.release();
    if (res != 0) continue;
    Serial.print(i++);
    Serial.print(':');
    Serial.print(F(" 0x"));
    Serial.println(addr, HEX);
  }
  Serial.println();
  delay(5000);
}