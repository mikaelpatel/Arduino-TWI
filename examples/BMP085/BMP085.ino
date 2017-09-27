#include "TWI.h"
#include "Driver/BMP085.h"

// Configure: TWI bus manager (software or hardware)
#define USE_SOFTWARE_TWI

#if defined(USE_SOFTWARE_TWI)
#include "GPIO.h"
#include "Software/TWI.h"
Software::TWI<BOARD::D18, BOARD::D19> twi;
#else
// Configure: Hardware TWI bus clock frequency (100 or 400 kHz)
#define FREQ 100000UL
// #define FREQ 400000UL
#include "Hardware/TWI.h"
Hardware::TWI twi(FREQ);
#endif

BMP085 bmp(twi);

void setup()
{
  Serial.begin(57600);
  while (!Serial);

  // Start the digital pressure sensor
  while (!bmp.begin(BMP085::ULTRA_LOW_POWER)) {
    Serial.println(F("bmp.begin:error"));
    delay(5000);
  }
}

void loop()
{
  // Print start time and sampled values
  uint32_t start = millis();
  Serial.print(start / 1000.0);
  Serial.print(':');

  // Sample, calculate and print temperature and pressure
  if (bmp.sample()) {
    Serial.print(bmp.temperature() / 10.0);
    Serial.print(F(" C, "));
    Serial.print(bmp.pressure() / 100.0);
    Serial.println(F(" hPa"));
  }
  else {
    Serial.println(F("bmp.sample:error"));
  }

  // Periodic execute every two seconds
  delay(2000 - (millis() - start));
}
