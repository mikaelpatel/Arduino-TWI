#include "TWI.h"

// Configure: TWI bus manager
#define USE_SOFTWARE_TWI

// Configure: Hardware TWI bus clock frequency
#define FREQ 400000UL
// #define FREQ 100000UL

#if defined(USE_SOFTWARE_TWI)
#include "GPIO.h"
#include "Software/TWI.h"
#if defined(ARDUINO_attiny)
#include "Software/Serial.h"
Software::Serial<BOARD::D0> Serial;
Software::TWI<BOARD::D1, BOARD::D2> twi;
#else
Software::TWI<BOARD::D18, BOARD::D19> twi;
#endif
#else
#include "Hardware/TWI.h"
Hardware::TWI twi(FREQ);
#endif

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
