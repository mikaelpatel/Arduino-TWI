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

#include "bcd.h"
#include "time.h"
#include "TWI.h"

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
   * Construct DS1307 device driver with bus address(0x68).
   */
  DS1307(TWI& twi) : TWI::Device(twi, 0x68) {}

  /**
   * Read current time from real-time clock. Return true(1)
   * if successful otherwise false(0).
   * @param[out] now time structure return value.
   * @return boolean.
   */
  bool get_time(struct tm& now)
  {
    // Read clock/calender structure from device
    rtc_t rtc;
    if (!read_ram(0, &rtc, sizeof(rtc))) return (false);

    // Convert to time structure
    now.tm_sec = rtc.seconds;
    now.tm_min = rtc.minutes;
    now.tm_hour = rtc.hours;
    now.tm_mday = rtc.date;
    now.tm_wday = rtc.day - 1;
    now.tm_mon = rtc.month - 1;
    now.tm_year = rtc.year + 100;
    return (true);
  }

  /**
   * Set the current time from real-time clock with the given
   * time. Return true(1) if successful otherwise false(0).
   * @param[in] now time structure to set.
   * @return boolean.
   */
  bool set_time(struct tm& now)
  {
    // Convert from time structure
    rtc_t rtc;
    rtc.seconds = now.tm_sec;
    rtc.minutes = now.tm_min;
    rtc.hours = now.tm_hour;
    rtc.date = now.tm_mday;
    rtc.day = now.tm_wday + 1;
    rtc.month = now.tm_mon + 1;
    rtc.year = now.tm_year - 100;

    // Write clock/calender structure to device
    return (write_ram(0, &rtc, sizeof(rtc)));
  }

  /**
   * Square Wave Output Rate Selection (pp. 9).
   */
  enum Rate {
    RS_1_HZ = 0,
    RS_4096_HZ = 1,
    RS_8192_HZ = 2,
    RS_32768_HZ = 3
  } __attribute__((packed));

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

  /** Start of application RAM. */
  const static uint8_t RAM_START = 0x08;

  /** End of application RAM. */
  const static uint8_t RAM_END = 0x3f;

  /** Max size of application RAM (56 bytes). */
  const static uint8_t RAM_MAX = RAM_END - RAM_START + 1;
  /**
   * Read ram block with the given size into the buffer. Return
   * true(1) if successful otherwise false.
   * @param[in] addr address on device.
   * @param[in] buf buffer to read from ram.
   * @param[in] count number of bytes to read.
   * @return bool.
   */
  bool read_ram(uint8_t addr, void* buf, size_t count)
  {
    if (!acquire()) return (false);
    if (write(&addr, sizeof(addr)) != sizeof(addr)) return (false);
    bool res = (read(buf, count) == (int) count);
    if (!release()) return (false);
    return (res);
  }

  /**
   * Write ram block at given position with the contents from buffer.
   * Return true(1) if successful otherwise false.
   * @param[in] addr address on device.
   * @param[in] buf buffer to write to ram.
   * @param[in] count number of bytes to write.
   * @return bool.
   */
  bool write_ram(uint8_t addr, const void* buf, size_t count)
  {
    if (count == 0) return (true);
    iovec_t vec[3];
    iovec_t* vp = vec;
    iovec_arg(vp, &addr, sizeof(addr));
    iovec_arg(vp, buf, count);
    iovec_end(vp);
    if (!acquire()) return (false);
    bool res = (write(vec) == (int) count + 1);
    if (!release()) return (false);
    return (res);
  }

protected:
  /**
   * The Timekeeper Clock/Calender Registers (Table 2, pp. 8).
   */
  struct rtc_t {
    bcd_t seconds;		//!< 00-59 Seconds.
    bcd_t minutes;		//!< 00-59 Minutes.
    bcd_t hours;		//!< 00-23 Hours.
    bcd_t day;			//!< 01-07 Day.
    bcd_t date;			//!< 01-31 Date.
    bcd_t month;		//!< 01-12 Month.
    bcd_t year;			//!< 00-99 Year.
  };

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
    operator uint8_t() { return (as_uint8); }

    /**
     * Default constructor will clear all bitfields.
     */
    control_t() : as_uint8(0) {}
  };

  /**
   * The Timekeeper Registers (Table 2, pp. 8).
   */
  struct timekeeper_t {
    rtc_t rtc;			//!< Clock/Calender Registers.
    control_t control;		//!< Control Register.
    uint8_t ram[RAM_MAX];	//!< Random Access Memory.
  };
};
#endif
