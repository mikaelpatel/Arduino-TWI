#include "TWI.h"
#include "Hardware/TWI.h"
#include "Driver/Si70XX.h"

Hardware::TWI twi;
Si70XX sensor(twi);

void setup()
{
  Serial.begin(57600);

  uint8_t reg = 0;
  int res = sensor.read_user_register(reg);
  Serial.print(res);
  Serial.print(F(":user: "));
  Serial.println(reg, HEX);

  uint8_t snr[8];
  res = sensor.read_electronic_serial_number(snr);
  Serial.print(res);
  Serial.print(F(":snr:"));
  for (size_t i = 0; i < sizeof(snr); i++) {
    Serial.print(' ');
    Serial.print(snr[i], HEX);
  }
  Serial.println();

  uint8_t rev = 0;
  res = sensor.read_firmware_revision(rev);
  Serial.print(res);
  Serial.print(F(":rev:"));
  Serial.println(rev, HEX);
}

void loop()
{
  sensor.measure_humidity();
  delay(20);
  float humidity = sensor.read_humidity();
  float humidity_temperature = sensor.read_humidity_temperature();

  sensor.measure_temperature();
  delay(10);
  float temperature = sensor.read_temperature();

  Serial.print(humidity);
  Serial.print(F("% RH, "));
  Serial.print(humidity_temperature);
  Serial.print(F("° C, "));
  Serial.print(temperature);
  Serial.println(F("° C"));

  delay(1000);
}
