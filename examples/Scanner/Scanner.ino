#include "TWI.h"

// Configure: TWI bus manager (software or hardware)
// #define USE_SOFTWARE_TWI

#if defined(USE_SOFTWARE_TWI)
#include "GPIO.h"
#include "Software/TWI.h"
#if defined(ARDUINO_attiny)
#include "Software/Serial.h"
Software::Serial<BOARD::D0> Serial;
Software::TWI<BOARD::D1, BOARD::D2> twi;
#else
#if defined(SAM)
Software::TWI<BOARD::D8, BOARD::D9> twi;
#else
Software::TWI<BOARD::D18, BOARD::D19> twi;
#endif
#endif

#else
// Configure: Hardware TWI bus clock frequency (100 or 400 kHz)
#include "Hardware/TWI.h"
Hardware::TWI twi(100000UL);
// Hardware::TWI twi(400000UL);
#endif

void setup()
{
  Serial.begin(57600);
  while (!Serial);
}

void loop()
{
  // Scan twi device addresses. Print address of all devices
  // that respond to a write request
  int i = 0;
  for (uint8_t addr = 3; addr < 128; addr++) {
    TWI::Device dev(twi, addr);
    dev.acquire();
    int res = dev.write(NULL);
    dev.release();
    if (res < 0) continue;
    Serial.print(i++);
    Serial.print(F(":addr=0x"));
    if (addr < 0x10) Serial.print(0);
    Serial.println(addr, HEX);
  }
  Serial.println();
  delay(2000);
}
