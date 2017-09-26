/**
 * @file Software/TWI.h
 * @version 1.1
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

#ifndef SOFTWARE_TWI_H
#define SOFTWARE_TWI_H

#include "TWI.h"
#include "GPIO.h"

/**
 * Software Two-Wire Interface (TWI) template class using GPIO.
 * @param[in] SDA_PIN board pin for data output signal.
 * @param[in] SCL_PIN board pin for clock output signal.
 */
namespace Software {
template<BOARD::pin_t SDA_PIN, BOARD::pin_t SCL_PIN>
class TWI : public ::TWI {
public:
  /**
   * Construct Two-Wire Interface (TWI) instance with given template
   * parameters. Initiate GPIO pins for data and clock for open drain
   * mode.
   */
  TWI()
  {
    m_sda.open_drain();
    m_scl.open_drain();
  }

  /**
   * @override{TWI}
   * Start transaction for given device driver. Return true(1) if
   * successful otherwise false(0).
   * @return bool.
   */
  virtual bool acquire()
  {
    while (m_busy) yield();
    m_busy = true;
    m_start = true;
    return (start_condition());
  }

  /**
   * @override{TWI}
   * Stop transaction. Mark the bus as available. Return true(1) if
   * successful otherwise false(0).
   * @return bool.
   */
  virtual bool release()
  {
    m_start = false;
    m_busy = false;
    return (stop_condition());
  }

  /**
   * @override{TWI}
   * Read data from device with given address into given buffer.
   * @param[in] addr device address.
   * @param[in] buf buffer pointer.
   * @param[in] count buffer size in bytes.
   * @return number of bytes read or negative error code.
   */
  virtual int read(uint8_t addr, void* buf, size_t count)
  {
    // Check if repeated start condition should be generated
    if (!m_start && !repeated_start_condition()) return (-1);
    m_start = false;

    // Address device with read request and check that it acknowledges
    bool nack;
    if (!write_byte(addr | 1, nack) || nack) return (-1);

    // Read bytes and acknowledge until required size
    uint8_t* bp = (uint8_t*) buf;
    size_t size = count;
    while (size--) {
      bool ack = (size != 0);
      uint8_t data;
      if (!read_byte(data, ack)) return (-1);
      *bp++ = data;
    }
    return (count);
  }

  /**
   * @override{TWI}
   * Write data to device with from given io vector.
   * @param[in] addr device address.
   * @param[in] vp io vector pointer.
   * @return number of bytes written or negative error code.
   */
  virtual int write(uint8_t addr, iovec_t* vp)
  {
    // Check if repeated start condition should be generated
    if (!m_start && !repeated_start_condition()) return (-1);
    m_start = false;

    // Address device with write request and check that it acknowledges
    bool nack;
    if (!write_byte(addr | 0, nack) || nack) return (-1);
    if (vp == NULL) return (0);

    // Write given io vector buffers to device
    int count = 0;
    for(; vp->buf != NULL; vp++) {
      const uint8_t* bp = (const uint8_t*) vp->buf;
      size_t size = vp->size;
      count += size;
      while (size--) {
	uint8_t data = *bp++;
	if (!write_byte(data, nack) || nack) return (-1);
      }
    }
    return (count);
  }

protected:
  /** Start condition delay time: 4.0 us (100 kHz) */
  static const int T1 = 4;

  /** Basic clock delay time: 4.7 us (100 kHz) */
  static const int T2 = 5;

  /** Maximum number of clock stretching retries: 100 us */
  static const int CLOCK_STRETCHING_RETRY_MAX = 25;

  /** Data signal pin. */
  GPIO<SDA_PIN> m_sda;

  /** Clock signal pin. */
  GPIO<SCL_PIN> m_scl;

  /** Transaction state; start or repeated start condition. */
  bool m_start;

  /**
   * Allow device to stretch clock signal. Return true(1) if
   * successful otherwise false(0).
   * @return bool.
   */
  bool clock_stretching()
  {
    for (int retry = 0; retry < CLOCK_STRETCHING_RETRY_MAX; retry++) {
      if (m_scl) return (true);
      delayMicroseconds(T1);
    }
    return (false);
  }

  /**
   * Generate start condition. Return true(1) if successful otherwise
   * false(0).
   * @return bool.
   */
  bool start_condition()
  {
    m_sda.input();
    if (m_sda == 0) return (false);
    m_sda.output();
    delayMicroseconds(T1);
    m_scl.output();
    return (true);
  }

  /**
   * Generate repeated start condition. Return true(1) if successful otherwise
   * false(0).
   * @return bool.
   */
  bool repeated_start_condition()
  {
    delayMicroseconds(T1);
    m_sda.input();
    if (m_sda == 0) return (false);
    m_scl.input();
    delayMicroseconds(T2);
    m_sda.output();
    delayMicroseconds(T1);
    m_scl.output();
    return (true);
  }

  /**
   * Generate stop condition. Return true(1) if successful otherwise
   * false(0).
   * @return bool.
   */
  bool stop_condition()
  {
    delayMicroseconds(T1);
    m_sda.output();
    m_scl.input();
    delayMicroseconds(T1);
    if (!clock_stretching()) return (false);
    m_sda.input();
    return (m_sda == 0);
  }

  /**
   * Write bit to device. Return true(1) if successful otherwise
   * false(0).
   * @param[in] value to write to device.
   * @return bool.
   */
  bool write_bit(bool value)
  {
    if (value) m_sda.input(); else m_sda.output();
    delayMicroseconds(T2);
    m_scl.input();
    delayMicroseconds(T1);
    if (!clock_stretching()) return (false);
    m_scl.output();
    return (true);
  }

  /**
   * Read bit to device. Return true(1) if successful otherwise
   * false(0).
   * @param[out] value read from device.
   * @return bool.
   */
  bool read_bit(bool& value)
  {
    m_sda.input();
    delayMicroseconds(T2);
    m_scl.input();
    delayMicroseconds(T1);
    if (!clock_stretching()) return (false);
    value = m_sda;
    m_scl.output();
    return (true);
  }

  /**
   * Write byte to device. Return true(1) and nack bit if successful
   * otherwise false(0).
   * @param[in] byte to write to device.
   * @param[out] nack from device.
   * @return bool.
   */
  bool write_byte(uint8_t byte, bool& nack)
  {
    for (int i = 0; i < 8; i++) {
      if (!write_bit(byte & 0x80)) return (false);
      byte <<= 1;
    }
    return (read_bit(nack));
  }

  /**
   * Read byte to device. Return true(1) if successful otherwise
   * false(0). The parameter ack signals if additional read with
   * follow.
   * @param[in] byte to write to device.
   * @param[out] ack to device.
   * @return bool.
   */
  bool read_byte(uint8_t& byte, bool ack)
  {
    bool value;
    byte = 0;
    for (int i = 0; i < 8; i++) {
      if (!read_bit(value)) return (false);
      byte = (byte << 1) | value;
    }
    return (write_bit(!ack));
  }
};
};
#endif
