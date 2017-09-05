#include "TWI.h"
#include "GPIO.h"
#include "Software/TWI.h"
#include "AT24CXX.h"

Software::TWI<BOARD::D18, BOARD::D19> twi;
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
