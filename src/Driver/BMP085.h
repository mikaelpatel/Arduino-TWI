/**
 * @file BMP085.h
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2017, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef BMP085_H
#define BMP085_H

#include "TWI.h"

/**
 * TWI Device Driver for the Bosch BMP085 Digital Pressure Sensor.
 *
 * @section Circuit
 * The GY-80 10DOF module with pull-up resistors (4K7) for TWI signals
 * and internal 3V3 voltage converter.
 * @code
 *                           GY-80
 *                       +------------+
 * (VCC)---------------1-|VCC         |
 *                     2-|3V3         |
 * (GND)---------------3-|GND         |
 * (A5/SCL)------------4-|SCL         |
 * (A4/SDA)------------5-|SDA         |
 *                     6-|M-DRDY      |
 *                     7-|A-INT1      |
 *                     8-|T-INT1      |
 *                     9-|P-XCLR      |
 * (Dn/EXTn)----------10-|P-EOC       |
 *                       +------------+
 * @endcode
 *
 * @section References
 * 1. http://media.digikey.com/pdf/Data%20Sheets/Bosch/BMP085.pdf
 * BST-BMP085-DS000-03, Rev. 1.0, 01 July 2008.
 */
class BMP085 : protected TWI::Device {
public:
  /**
   * Oversampling modes (table, pp. 10).
   */
  enum Mode {
    ULTRA_LOW_POWER = 0,
    STANDARD = 1,
    HIGH_RESOLUTION = 2,
    ULTRA_HIGH_RESOLUTION = 3
  } __attribute__((packed));

  /**
   * Construct BMP085 driver with I2C address(0x77) and default
   * ULTRA_LOW_POWER mode.
   */
  BMP085(TWI& twi) :
    TWI::Device(twi, 0x77),
    m_mode(ULTRA_LOW_POWER),
    m_cmd(0),
    m_start(0),
    B5(0),
    m_pressure(0)
  {}

  /**
   * Initiate device driver. Load calibration coefficients from device.
   * Return true(1) if successful otherwise false(0).
   * @param[in] mode oversampling (Default ULTRA_LOW_POWER).
   * @return bool.
   */
  bool begin(Mode mode = ULTRA_LOW_POWER)
  {
    // Set the operation mode
    m_mode = mode;

    // Read coefficients from the device
    if (!acquire()) return (false);
    uint8_t reg = COEFF_REG;
    write(&reg, sizeof(reg));
    int res = read(&m_param, sizeof(m_param));
    if (!release()) return (false);
    if (res != sizeof(m_param)) return (false);

    // Adjust coefficients to little endian
    uint16_t* p = (uint16_t*) &m_param;
    for (size_t i = 0; i < sizeof(param_t) / sizeof(uint16_t); i++, p++)
      *p = bswap16(*p);

    return (true);
  }

  /**
   * Issue a sample raw temperature sensor request. Return true(1) if
   * successful otherwise false.
   * @return bool.
   */
  bool sample_temperature_request()
  {
    // Check that a conversion request is not in process
    if (m_cmd != 0) return (false);

    // Start a temperature measurement and wait
    m_cmd = TEMP_CONV_CMD;
    uint8_t req[2] = { CMD_REG, m_cmd };
    if (!acquire()) return (false);
    write(req, sizeof(req));
    if (!release()) return (false);

    // Set start time for completion
    m_start = millis();
    return (true);
  }

  /**
   * Read the raw temperature sensor. Will wait for the conversion to
   * complete. Return true(1) if successful otherwise false(0).
   * @return bool.
   */
  bool read_temperature()
  {
    // Check that a temperature conversion request was issued
    if (m_cmd != TEMP_CONV_CMD) return (false);
    m_cmd = 0;

    // Check if we need to wait for the conversion to complete
    uint16_t run = millis() - m_start;
    if (run < TEMP_CONV_MS) delay(TEMP_CONV_MS - run);

    // Read the raw temperature sensor data
    uint8_t reg = RES_REG;
    int16_t UT;
    if (!acquire()) return (false);
    write(&reg, sizeof(reg));
    read(&UT, sizeof(UT));
    if (!release()) return (false);

    // Adjust for little-endian
    UT = bswap16(UT);

    // Temperature calculation
    int32_t X1 = ((((int32_t) UT) - m_param.ac6) * m_param.ac5) >> 15;
    int32_t X2 = (((int32_t) m_param.mc) << 11) / (X1 + m_param.md);
    B5 = X1 + X2;
    return (true);
  }

  /**
   * Sample the raw temperature sensor and read. Return true(1) if
   * successful otherwise false(0).
   * @return bool.
   */
  bool sample_temperature()
  {
    return (sample_temperature_request() && read_temperature());
  }

  /**
   * Issue a sample request of the raw pressure sensor. Return true(1)
   * if successful otherwise false(0).
   * @return bool.
   */
  bool sample_pressure_request()
  {
    // Check that a conversion request is not in process
    if (m_cmd != 0) return (false);

    // Start a pressure measurement
    if (!acquire()) return (false);
    m_cmd = PRESSURE_CONV_CMD + (m_mode << 6);
    uint8_t req[2] = { CMD_REG, m_cmd };
    write(&req, sizeof(req));
    if (!release()) return (false);

    // Set start time for completion
    m_start = millis();
    return (true);
  }

  /**
   * Read the raw pressure sensor. Will wait for the conversion to
   * complete. Return true(1) if successful otherwise false(0).
   * @return bool.
   */
  bool read_pressure()
  {
    /** Pressure conversion time max table (ms), index with mode. */
    static const uint8_t PRESSURE_CONV_MS[] PROGMEM = {
      5, 8, 14, 26
    };

    // Check that a conversion request was issued
    if (m_cmd != (PRESSURE_CONV_CMD + (m_mode << 6))) return (false);
    m_cmd = 0;

    // Check if we need to wait for the conversion to complete
    uint16_t run = millis() - m_start;
    uint16_t ms = pgm_read_byte(&PRESSURE_CONV_MS[m_mode]);
    if (run < ms) delay(ms - run);

    // Read the raw pressure sensor data
    union {
      uint32_t as_int32;
      uint8_t as_uint8[4];
    } res;
    res.as_uint8[0] = 0;
    uint8_t reg = RES_REG;
    if (!acquire()) return (false);
    write(&reg, sizeof(reg));
    read(&res.as_uint8[1], 3);
    if (!release()) return (false);

    // Adjust for little endian and resolution (oversampling mode)
    int32_t UP = bswap32(res.as_int32) >> (8 - m_mode);
    int32_t B3, B6, X1, X2, X3;
    uint32_t B4, B7;

    // Pressure calculation
    B6 = B5 - 4000;
    X1 = (m_param.b2 * ((B6 * B6) >> 12)) >> 11;
    X2 = (m_param.ac2 * B6) >> 11;
    X3 = X1 + X2;
    B3 = ((((((int32_t) m_param.ac1) << 2) + X3) << m_mode) + 2) >> 2;
    X1 = (m_param.ac3 * B6) >> 13;
    X2 = (m_param.b1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    B4 = (m_param.ac4 * (uint32_t) (X3 + 32768)) >> 15;
    B7 = ((uint32_t) UP - B3) * (50000 >> m_mode);
    m_pressure = (B7 < 0x80000000) ? (B7 << 1) / B4 : (B7 / B4) << 1;
    X1 = (m_pressure >> 8) * (m_pressure >> 8);
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * m_pressure) >> 16;
    m_pressure += (X1 + X2 + 3791) >> 4;

    return (true);
  }

  /**
   * Sample and read the raw pressure sensor. Return true(1) if
   * successful otherwise false(0).
   * @return bool.
   */
  bool sample_pressure()
    __attribute__((always_inline))
  {
    return (sample_pressure_request() && read_pressure());
  }

  /**
   * Sample and read the raw temperature and pressure sensor. Return
   * true(1) if successful otherwise false(0). Retrieve calculated
   * values with temperature() and pressure().
   * @return bool.
   */
  bool sample()
    __attribute__((always_inline))
  {
    return (sample_temperature() && sample_pressure());
  }

  /**
   * Calculate temperature from the latest raw sensor reading.
   * @return calculated temperature in steps of 0.1 C
   */
  int16_t temperature() const
    __attribute__((always_inline))
  {
    return ((B5 + 8) >> 4);
  }

  /**
   * Return latest calculated pressure from temperature and pressure
   * raw sensor data.
   * @return calculated pressure in steps of 1 Pa (0,01 hPa).
   */
  int32_t pressure() const
  {
    return (m_pressure);
  }

protected:
  /** Temperature conversion time max (ms). */
  static const uint16_t TEMP_CONV_MS = 5;

  /**
   * Calibration coefficients (chap. 3.4, pp. 11). Data from the
   * device is in big-endian order.
   */
  struct param_t {
    int16_t ac1;
    int16_t ac2;
    int16_t ac3;
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t b1;
    int16_t b2;
    int16_t mb;
    int16_t mc;
    int16_t md;
  } __attribute__((packed));

  /**
   * EEPROM parameters, command and result registers (chap. 4.5, pp. 17).
   * Parameter and result registers are 16-bit, in big-endian order.
   */
  enum reg_t {
    COEFF_REG = 0xAA, 		//!< Calibration coefficients register address.
    CMD_REG = 0xF4,		//!< Command register address (8-bit).
    RES_REG = 0xF6		//!< Result register address (16-bit).
  } __attribute__((packed));

  /**
   * Measurement/Control register value (chap. 4.4, pp. 16).
   */
  enum cmd_t {
    TEMP_CONV_CMD = 0x2E,	//!< Temperature conversion command.
    PRESSURE_CONV_CMD = 0x34	//!< Pressure conversion command.
  } __attribute__((packed));

  /** Device calibration data (from EEPROM data registers). */
  param_t m_param;

  /** Pressure conversion mode. */
  Mode m_mode;

  /** Currrent command. */
  uint8_t m_cmd;

  /** Sample request start time (ms). */
  uint16_t m_start;

  /** Common intermediate temperature factor. */
  int32_t B5;

  /** Latest calculated pressure. */
  int32_t m_pressure;
};

#endif
