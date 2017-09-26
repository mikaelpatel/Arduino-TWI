#include "TWI.h"
#include "Driver/PCF8574.h"

// Configure: TWI bus manager
// #define USE_SOFTWARE_TWI

// Configure: Hardware TWI bus clock frequency
// #define FREQ 800000UL
#define FREQ 400000UL
// #define FREQ 100000UL

#if defined(USE_SOFTWARE_TWI)
#include "GPIO.h"
#include "Software/TWI.h"
Software::TWI<BOARD::D18, BOARD::D19> twi;
#else
#include "Hardware/TWI.h"
Hardware::TWI twi(FREQ);
#endif

PCF8574 port(twi, 0);
PCF8574::GPIO<0> pin(port);

void setup()
{
  Serial.begin(57600);
  while (!Serial);

  pin.output();
}

void loop()
{
  static bool state = false;

  pin = state;
  Serial.print(F("pin = "));
  Serial.println(pin);

  Serial.print(F("port.read() = "));
  Serial.println(port.read(), BIN);
  delay(10);

  for (int i = 0; i < 16; i++)
    pin = (state = !state);
  delay(10);

  uint8_t buf[] = { 0, 1, 0, 1, 0, 1, 0, 1 };
  port.write(buf, sizeof(buf));
  delay(1000);
}
