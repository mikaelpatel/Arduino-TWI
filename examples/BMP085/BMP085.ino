#include "TWI.h"
#include "Driver/BMP085.h"
#include "assert.h"

// Configure: TWI bus manager (software or hardware)
// #define USE_SOFTWARE_TWI

#if defined(USE_SOFTWARE_TWI)
#include "GPIO.h"
#include "Software/TWI.h"
Software::TWI<BOARD::D18, BOARD::D19> twi;
#else
// Configure: Hardware TWI bus clock frequency (100 or 400 kHz)
#include "Hardware/TWI.h"
Hardware::TWI twi(100000UL);
// Hardware::TWI twi(400000UL);
#endif

BMP085 bmp(twi);

void setup()
{
  Serial.begin(57600);
  while (!Serial);

  // Start the digital pressure sensor
  ASSERT(bmp.begin(BMP085::ULTRA_LOW_POWER));
}

void loop()
{
  // Print start time and sampled values
  uint32_t start = millis();
  Serial.print(start / 1000.0);
  Serial.print(':');

  // Sample, calculate and print temperature and pressure
  ASSERT(bmp.sample());
  Serial.print(bmp.temperature() / 10.0);
  Serial.print(F(" C, "));
  Serial.print(bmp.pressure() / 100.0);
  Serial.println(F(" hPa"));

  // Periodic execute every two seconds
  delay(2000 - (millis() - start));
}
