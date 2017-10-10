#include "GPIO.h"
#include "TWI.h"
#include "Driver/DS2482.h"

// Configure: Software or Hardware TWI
// #define USE_SOFTWARE_TWI
#if defined(USE_SOFTWARE_TWI)
#include "Software/TWI.h"
#if defined(SAM)
Software::TWI<BOARD::D8,BOARD::D9> twi;
#else
Software::TWI<BOARD::D18,BOARD::D19> twi;
#endif
#else
#include "Hardware/TWI.h"
Hardware::TWI twi;
#endif

DS2482 owi(twi);

const size_t ROM_MAX = 8;
const uint8_t CHARBITS = 8;

// 1-Wire ROM and DS18B20 commands
enum {
  SEARCH_ROM = 0xF0,
  READ_ROM = 0x33,
  MATCH_ROM = 0x55,
  SKIP_ROM = 0xCC,
  ALARM_SEARCH = 0xEC,
  CONVERT_T = 0x44,
  READ_SCRATCHPAD = 0xBE
};

// DS18B20 scratchpad
struct scratchpad_t {
  int16_t temperature;
  int8_t high_trigger;
  int8_t low_trigger;
  uint8_t configuration;
  uint8_t reserved[3];
  uint8_t crc;
};

// 1-Wire CRC calculation
uint8_t one_wire_crc_update(uint8_t crc, uint8_t data)
{
  crc = crc ^ data;
  for (uint8_t i = 0; i < 8; i++) {
    if (crc & 0x01)
      crc = (crc >> 1) ^ 0x8C;
    else
      crc >>= 1;
  }
  return (crc);
}

// Check assertion to be true, otherwise print line and expression
#define ASSERT(expr)							\
  do {									\
    if (!(expr)) {							\
      Serial.print(__LINE__);						\
      Serial.println(F(":assert:" #expr));				\
      Serial.flush();							\
      exit(0);								\
    }									\
  } while (0)


// Print and evaluate expression
#define TRACE(expr)							\
  do {									\
    Serial.print(#expr "=");						\
    Serial.println(expr);						\
  } while (0)


void setup()
{
  Serial.begin(57600);
  while (!Serial);

  // Reset and configure the TWI 1-Wire Master
  ASSERT(owi.device_reset());
  ASSERT(owi.write_configuration());

  // Read and print registers
  uint8_t config;
  ASSERT(owi.set_read_pointer(owi.CONFIGURATION_REGISTER, config));
  TRACE(config);

  uint8_t data;
  ASSERT(owi.set_read_pointer(owi.READ_DATA_REGISTER, data));
  TRACE(data);

  uint8_t status;
  ASSERT(owi.set_read_pointer(owi.STATUS_REGISTER, status));
  TRACE(status);

#if defined(DS2482_800)
  uint8_t channel;
  ASSERT(owi.set_read_pointer(owi.CHANNEL_SELECTION_REGISTER, channel));
  TRACE(channel);
#endif

  // Read and print rom from device on 1-Wire bus
  uint8_t rom[ROM_MAX] = { 0 };
  uint8_t crc = 0;
  ASSERT(owi.one_wire_reset());
  ASSERT(owi.one_wire_write_byte(READ_ROM));
  Serial.print(F("read_rom="));
  for (size_t i = 0; i < sizeof(rom); i++) {
    owi.one_wire_read_byte(rom[i]);
    if (rom[i] < 0x10) Serial.print(0);
    Serial.print(rom[i], HEX);
    crc = one_wire_crc_update(crc, rom[i]);
  }
  Serial.println();
  ASSERT(crc == 0);

  // Search device on 1-Wire bus and print rom
  uint8_t bits = 0;
  uint8_t ix = 0;
  uint8_t value = 0;
  uint8_t dir = 0;
  int8_t res = 0;
  bool id;
  bool nid;
  crc = 0;
  ASSERT(owi.one_wire_reset());
  ASSERT(owi.one_wire_write_byte(SEARCH_ROM));
  Serial.print(F("search_rom="));
  do {
    res = owi.one_wire_triplet(dir);
    if (res < 0) break;
    id = (res & 1) != 0;
    nid = (res & 2) != 0;
    value = (value >> 1);
    if (dir) value |= 0x80;
    bits += 1;
    if (bits == CHARBITS) {
      rom[ix] = value;
      if (rom[ix] < 0x10) Serial.print(0);
      Serial.print(rom[ix], HEX);
      crc = one_wire_crc_update(crc, rom[ix]);
      ix += 1;
      bits = 0;
      value = 0;
    }
  } while (id != nid);
  Serial.println();
  ASSERT(crc == 0);
}

void loop()
{
  // Convert and read temperature
  ASSERT(owi.one_wire_reset());
  ASSERT(owi.one_wire_write_byte(SKIP_ROM));
  ASSERT(owi.one_wire_write_byte(CONVERT_T));
  delay(750);

  ASSERT(owi.one_wire_reset());
  ASSERT(owi.one_wire_write_byte(SKIP_ROM));
  ASSERT(owi.one_wire_write_byte(READ_SCRATCHPAD));

  // Print scatchpad and calculate check sum
  scratchpad_t scratchpad;
  uint8_t* p = (uint8_t*) &scratchpad;
  uint8_t crc = 0;
  Serial.print(F("read_scratchpad="));
  for (size_t i = 0; i < sizeof(scratchpad); i++) {
    ASSERT(owi.one_wire_read_byte(p[i]));
    if (i == sizeof(scratchpad) - 1) Serial.print(F(",crc="));
    if (p[i] < 0x10) Serial.print('0');
    Serial.print(p[i], HEX);
    crc = one_wire_crc_update(crc, p[i]);
  }
  ASSERT(crc == 0);

  // Print temperature (convert from fixed to floating point number)
  Serial.print(',');
  float temperature = scratchpad.temperature * 0.0625;
  TRACE(temperature);
  delay(2000);
}
