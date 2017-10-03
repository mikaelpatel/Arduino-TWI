/**
 * @file PCF8574.h
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

#ifndef PCF8574_H
#define PCF8574_H

#include "TWI.h"

/**
 * Driver for the PCF8574/PCF8574A Remote 8-bit I/O expander for
 * I2C-bus with interrupt.
 *
 * @section Circuit
 * @code
 *                          PCF8574A
 *                       +------------+
 * (GND)---[ ]---------1-|A0       VCC|-16--------------(VCC)
 * (GND)---[ ]---------2-|A1       SDA|-15-----------(SDA/A4)
 * (GND)---[ ]---------3-|A2       SCL|-14-----------(SCL/A5)
 * (P0)----------------4-|P0       INT|-13--------------(EXT)
 * (P1)----------------5-|P1        P7|-12---------------(P7)
 * (P2)----------------6-|P2        P6|-11---------------(P6)
 * (P3)----------------7-|P3        P5|-10---------------(P5)
 * (GND)---------------8-|GND       P4|-9----------------(P4)
 *                       +------------+
 * @endcode
 *
 * @section References
 * 1. NXP Semiconductors Product data sheet, Rev. 5, 27 May 2013.
 */
class PCF8574 : protected TWI::Device {
public:
  /**
   * Construct connection to PCF8574 Remote 8-bit I/O expander with
   * given sub-address.
   * @param[in] twi bus manager.
   * @param[in] subaddr sub-address (0..7, default 7).
   */
  PCF8574(TWI& twi, uint8_t subaddr = 7) :
    TWI::Device(twi, 0x20 | (subaddr & 0x7)),
    m_ddr(0xff),
    m_port(0)
  {}

  /**
   * Get data direction for port; 0 for output, 1 for input.
   * @return data direction.
   */
  uint8_t ddr()
  {
    return (m_ddr);
  }

  /**
   * Set data direction for port pin P0..P7; 0 for output, 1 for input.
   * @param[in] ddr data direction mask.
   */
  void ddr(uint8_t ddr)
  {
    m_ddr = ddr;
    m_port |= m_ddr;
    acquire();
    Device::write(&m_port, sizeof(m_port));
    release();
  }

  /**
   * Read pins and return current values.
   * @return input pin values.
   */
  uint8_t read()
  {
    uint8_t res;
    acquire();
    Device::read(&res, sizeof(res));
    release();
    return ((res & m_ddr) | m_port);
  }

  /**
   * Get data port values.
   * @return
   */
  uint8_t port()
  {
    return (m_port);
  }

  /**
   * Write given value to the output pins.
   * @param[in] value.
   * @return bool.
   */
  void write(uint8_t value)
  {
    acquire();
    m_port = value | m_ddr;
    Device::write(&m_port, sizeof(m_port));
    release();
  }

  /**
   * Write given values to the output pins.
   * @param[in] buf pointer to data to write to device.
   * @param[in] size of buffer.
   */
  void write(void* buf, size_t size)
  {
    acquire();
    uint8_t* bp = (uint8_t*) buf;
    size_t n = size;
    while (n--) *bp++ |= m_ddr;
    Device::write(buf, size);
    release();
  }

  /**
   * PCF8574 based General Purpose Digital I/O pin template class.
   * @param[in] PIN index on device (0..7).
   */
  template<uint8_t PIN>
  class GPIO {
  public:
    /** Pin bit position mask. */
    static const uint8_t MASK = (1 << (PIN & 0x7));

    /**
     * Construct pin handler with given PCF8574 device driver.
     */
    GPIO(PCF8574& dev) : m_dev(dev) {}

    /**
     * Set input mode.
     */
    void input()
      __attribute__((always_inline))
    {
      m_dev.ddr(m_dev.ddr() | MASK);
    }

    /**
     * Set output mode.
     */
    void output()
      __attribute__((always_inline))
    {
      m_dev.ddr(m_dev.ddr() & ~MASK);
    }

    /**
     * Read pin state. Return true is set otherwise false.
     * @return bool.
     */
    bool read()
    {
      if (m_dev.ddr() & MASK)
	return ((m_dev.read() & MASK) != 0);
      return ((m_dev.port() & MASK) != 0);
    }

    /**
     * Read pin state. Return true is set otherwise false.
     * Shorthand for read().
     * @return bool.
     */
    operator bool()
      __attribute__((always_inline))
    {
      return (read());
    }

    /**
     * Write pin state one(1) if given value is non-zero,
     * otherwise zero(0).
     */
    void write(int value)
    {
      uint8_t data = m_dev.port();
      if (value)
	data |= MASK;
      else
	data &= ~MASK;
      m_dev.write(data);
    }

    /**
     * Assign pin state one(1) if given value is non-zero,
     * otherwise zero(0). Shorthand for write().
     */
    void operator=(int value)
      __attribute__((always_inline))
    {
      write(value);
    }

  protected:
    PCF8574& m_dev;
  };

protected:
  /** Data Direction Register, 0 = output, 1 = input. */
  uint8_t m_ddr;

  /** Port Register to mask and maintain output pin values. */
  uint8_t m_port;

  /**
   * Construct connection to PCF8574 Remote 8-bit I/O expander with
   * given address.
   * @param[in] twi bus manager.
   * @param[in] addr bus address.
   * @param[in] subaddr device sub address.
   */
  PCF8574(TWI& twi, uint8_t addr, uint8_t subaddr) :
    TWI::Device(twi, addr | (subaddr & 0x7)),
    m_ddr(0xff),
    m_port(0)
  {}
};

class PCF8574A : public PCF8574 {
public:
  /**
   * Construct connection to PCF8574A Remote 8-bit I/O expander with
   * given sub-address.
   * @param[in] twi bus manager.
   * @param[in] subaddr sub-address (0..7, default 7).
   */
  PCF8574A(TWI& twi, uint8_t subaddr = 7) : PCF8574(twi, 0x38, subaddr) {}
};
#endif
