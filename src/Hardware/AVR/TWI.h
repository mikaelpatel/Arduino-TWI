/**
 * @file Hardware/AVR/TWI.h
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

#ifndef HARDWARE_AVR_TWI_H
#define HARDWARE_AVR_TWI_H

#include "TWI.h"

/**
 * Hardware Two-Wire Interface (TWI) class.
 */
namespace Hardware {
class TWI : public ::TWI {
public:
  /**
   * Construct Two-Wire Interface (TWI).
   * @param[in] freq bus manager clock frequency (HZ).
   */
  TWI(uint32_t freq = DEFAULT_FREQ)
  {
    // Initiate hardware registers: baudrate and control
    TWBR = ((F_CPU / freq) - 16) / 2;
    TWSR = 0;
    TWCR = 0;
  }

  /**
   * @override{TWI}
   * Start transaction for given device driver. Return true(1) if
   * successful otherwise false(0).
   * @return bool.
   */
  virtual bool acquire()
  {
    // Wait for bus manager to become idle
    while (m_busy) yield();
    m_busy = true;

    // Acquire bus and issue start condition
    m_start = true;
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
    return iowait(START);
  }

  /**
   * @override{TWI}
   * Stop transaction. Mark the bus as available. Return true(1) if
   * successful otherwise false(0).
   * @return bool.
   */
  virtual bool release()
  {
    // Mark bus manager as idle
    m_start = false;
    m_busy = false;

    // Issue stop condition and release bus
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);

    // Allow the command to complete
    delayMicroseconds(10);
    return (true);
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
    if (!m_start) {
      TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
      if (!iowait(REP_START)) return (false);
    }
    m_start = false;

    // Address device with read request and check that it acknowledges
    TWDR = addr | 0x01;
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
    if (!iowait(MR_SLA_ACK)) return (-1);

    // Read bytes and acknowledge until required size
    uint8_t* bp = (uint8_t*) buf;
    size_t size = count;
    while (size--) {
      if (size != 0) {
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
	if (!iowait(MR_DATA_ACK)) return (-1);
      }
      else {
	TWCR = _BV(TWEN) | _BV(TWINT);
	if (!iowait(MR_DATA_NACK)) return (-1);
      }
      *bp++ = TWDR;
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
    if (!m_start) {
      TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
      if (!iowait(REP_START)) return (-1);
    }
    m_start = false;

    // Address device with write request and check that it acknowledges
    TWDR = addr | 0x00;
    TWCR = _BV(TWEN) | _BV(TWINT);
    if (!iowait(MT_SLA_ACK)) return (-1);
    if (vp == NULL) return (0);

    // Write given io vector buffers to device
    int count = 0;
    for(; vp->buf != NULL; vp++) {
      const uint8_t* bp = (const uint8_t*) vp->buf;
      size_t size = vp->size;
      count += size;
      while (size--) {
	TWDR = *bp++;
	TWCR = _BV(TWEN) | _BV(TWINT);
	if (!iowait(MT_DATA_ACK)) return (-1);
      }
    }
    return (count);
  }

protected:
  /** Status codes for Master Transmitter Mode. */
  enum {
    START = 0x08,		//!< Start condition transmitted.
    REP_START = 0x10,		//!< Repeated start transmitted.
    ARB_LOST = 0x38,		//!< Arbitration lost.
    MT_SLA_ACK = 0x18,		//!< Slave address/write sent, ACK received.
    MT_SLA_NACK = 0x20,		//!< dito, NACK received.
    MT_DATA_ACK = 0x28,		//!< Data write sent, ACK received.
    MT_DATA_NACK = 0x30,	//!< dito, NACK received.
    MR_SLA_ACK = 0x40,		//!< Slave address/read sent, ACK received.
    MR_SLA_NACK = 0x48,		//!< dito, NACK received.
    MR_DATA_ACK = 0x50,		//!< Data received, ACK sent.
    MR_DATA_NACK = 0x58,	//!< dito, NACK sent.
    MASK = 0xF8,		//!< Mask status code.
    BUS_ERROR = 0x00		//!< Bus error state.
  } __attribute__((packed));


  /**
   * Wait for command to complete and check status. Return true(1)
   * if correct status has been reached otherwise false(0).
   * @param[in] status to be reached.
   * @return bool.
   */
  bool iowait(uint8_t status)
  {
    loop_until_bit_is_set(TWCR, TWINT);
    return ((TWSR & MASK) == status);
  }

  /** Start condition issued flag. */
  bool m_start;
};
};

#endif
