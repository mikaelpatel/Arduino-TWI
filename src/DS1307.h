/**
 * @file DS1307.h
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

#ifndef DS1307_H
#define DS1307_H

#include "TWI.h"

#ifndef TIME_H
struct time_t {
  uint8_t seconds;		//!< 00-59 Seconds.
  uint8_t minutes;		//!< 00-59 Minutes.
  uint8_t hours;		//!< 00-23 Hours.
  uint8_t day;			//!< 01-07 Day.
  uint8_t date;			//!< 01-31 Date.
  uint8_t month;		//!< 01-12 Month.
  uint8_t year;			//!< 00-99 Year.
};
#endif

/**
 * Driver for the DS1307, 64 X 8, Serial I2C Real-Time Clock,
 * a low-power, full binary-coded decimal (BCD) clock/calendar plus
 * 56 bytes of NV SRAM.
 *
 * For further details see Maxim Integrated product description;
 * http://datasheets.maximintegrated.com/en/ds/DS1307.pdf
 *
 * @section Circuit
 * @code
 *                       TinyRTC(DS1307)
 *                       +------------+
 *                     1-|SQ          |
 *                     2-|DS        DS|-1
 * (A5/SCL)------------3-|SCL      SCL|-2
 * (A4/SDA)------------4-|SDA      SDA|-3
 * (VCC)---------------5-|VCC      VCC|-4
 * (GND)---------------6-|GND      GND|-5
 *                     7-|BAT         |
 *                       +------------+
 * @endcode
 */
class DS1307 : protected TWI::Device {
public:
  /**
   * The Timekeeper Control Register bitfields (pp. 9).
   */
  union control_t {
    uint8_t as_uint8;		//!< Unsigned byte access.
    struct {			//!< Bitfield access.
      uint8_t rs:2;		//!< Rate Select.
      uint8_t reserved1:2;	//!< Reserved/1.
      uint8_t sqwe:1;		//!< Square-Ware Enable.
      uint8_t reserved2:2;	//!< Reserved/2.
      uint8_t out:1;		//!< Output Control.
    };

    /**
     * Cast control register bit-field to byte.
     * @return byte representation.
     */
    operator uint8_t()
    {
      return (as_uint8);
    }

    /**
     * Default constructor will clear all bitfields.
     */
    control_t()
    {
      as_uint8 = 0;
    }
  };

  /**
   * Rate Selection (pp. 9).
   */
  enum Rate {
    RS_1_HZ = 0,
    RS_4096_HZ = 1,
    RS_8192_HZ = 2,
    RS_32768_HZ = 3
  } __attribute__((packed));

  /**
   * The Timekeeper Registers (Table 2, pp. 8).
   */
  struct timekeeper_t {
    time_t clock;
    control_t control;
  };

  /** Start of application RAM. */
  const static uint8_t RAM_START = sizeof(timekeeper_t);

  /** End of application RAM. */
  const static uint8_t RAM_END = 0x3f;

  /** Max size of application RAM (56 bytes). */
  const static uint8_t RAM_MAX = RAM_END - RAM_START + 1;

  /**
   * Construct DS1307 device driver with bus address(0x68).
   */
  DS1307(TWI& twi) : TWI::Device(twi, 0x68) {}

  /**
   * Read current time from real-time clock. Return true(1)
   * if successful otherwise false(0).
   * @param[out] now time structure return value.
   * @return boolean.
   */
  bool get_time(time_t& now)
  {
    return (read_ram(0, &now, sizeof(now)));
  }

  /**
   * Set the current time from real-time clock with the given
   * time. Return true(1) if successful otherwise false(0).
   * @param[in] now time structure to set.
   * @return boolean.
   */
  bool set_time(time_t& now)
  {
    return (write_ram(0, &now, sizeof(now)));
  }

  /**
   * Enable clock output with given rate. Return true(1) if successful
   * otherwise false(0).
   * @param[in] rs rate selection (default 1 Hz).
   * @return bool.
   */
  bool enable(Rate rs = RS_1_HZ)
  {
    control_t control;
    control.rs = rs;
    control.sqwe = 1;
    control.out = 1;
    uint8_t addr = offsetof(DS1307::timekeeper_t, control);
    int count = write_ram(addr, &control, sizeof(control));
    return (count == sizeof(control));
  }

  /**
   * Disable clock output. Return true(1) if successful otherwise
   * false(0).
   * @return bool.
   */
  bool disable()
  {
    control_t control;
    uint8_t addr = offsetof(DS1307::timekeeper_t, control);
    int count = write_ram(addr, &control, sizeof(control));
    return (count == sizeof(control));
  }

  /**
   * Read ram block with the given size into the buffer.
   * Return number of bytes read or negative error code.
   * @param[in] addr address on device.
   * @param[in] buf buffer to read from ram.
   * @param[in] count number of bytes to read.
   * @return number of bytes or negative error code.
   */
  bool read_ram(uint8_t addr, void* buf, size_t count)
  {
    if (write(&addr, sizeof(addr)) != sizeof(addr)) return (false);
    return (read(buf, count) == (int) count);
  }

  /**
   * Write ram block at given position with the contents from buffer.
   * Return number of bytes written or negative error code.
   * @param[in] addr address on device.
   * @param[in] buf buffer to write to ram.
   * @param[in] count number of bytes to write.
   * @return number of bytes or negative error code.
   */
  bool write_ram(uint8_t addr, const void* buf, size_t count)
  {
    if (count == 0) return (0);
    iovec_t vec[3];
    iovec_t* vp = vec;
    iovec_arg(vp, &addr, sizeof(addr));
    iovec_arg(vp, buf, count);
    iovec_end(vp);
    return (write(vec) == (int) count + 1);
  }
};
#endif
