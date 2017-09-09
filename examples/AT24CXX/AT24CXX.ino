#include "TWI.h"
#include "Driver/AT24CXX.h"

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
Software::TWI<BOARD::D18, BOARD::D19> twi;
#endif
#elif defined(USE_HARDWARE_TWI)
#include "Hardware/TWI.h"
Hardware::TWI twi;
#endif

AT24C32 eeprom(twi);

// Data vector (64 bytes). Force multi-page access
const uint16_t V_EEPROM = eeprom.PAGE_MAX / 2;
int v[32];

void setup()
{
  Serial.begin(57600);
  while (!Serial);

  // Initiate vector
  for (size_t i = 0; i < sizeof(v) / sizeof(v[0]); i++) {
    v[i] = i;
  }
}

void loop()
{
  // Print contents of vector and increment elements
  for (size_t i = 0; i < sizeof(v) / sizeof(v[0]); i++) {
    Serial.print(v[i]);
    Serial.print(' ');
    v[i] += 1;
  }
  Serial.println();
  delay(5000);

  // Write vector to eeprom
  Serial.print(F("write: "));
  Serial.println(eeprom.write(V_EEPROM, &v, sizeof(v)));

  // Clear vector to verify read
  for (size_t i = 0; i < sizeof(v) / sizeof(v[0]); i++) {
    v[i] = 0;
  }

  // Read vector from eeprom
  Serial.print(F("read: "));
  Serial.println(eeprom.read(&v, V_EEPROM, sizeof(v)));
}
