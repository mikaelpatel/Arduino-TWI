#include "TWI.h"

// #define USE_SOFTWARE_TWI
#define USE_HARDWARE_TWI

#if defined(USE_SOFTWARE_TWI)
#include "GPIO.h"
#include "Software/TWI.h"
#if defined(ARDUINO_attiny)
#include "Software/Serial.h"
Software::Serial<BOARD::D0> Serial;
Software::TWI<BOARD::D1, BOARD::D2> twi;
#else
Software::TWI<BOARD::D6, BOARD::D7> twi;
#endif
#elif defined(USE_HARDWARE_TWI)
#include "Hardware/TWI.h"
Hardware::TWI twi;
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
