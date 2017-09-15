#include "TWI.h"
#include "Driver/PCF8574.h"

#define USE_SOFTWARE_TWI
// #define USE_HARDWARE_TWI

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
