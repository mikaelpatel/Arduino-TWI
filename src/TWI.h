/**
 * @file TWI.h
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

#ifndef TWI_H
#define TWI_H

#include "iovec.h"
#include "bswap.h"

/**
 * Abstract Two-Wire Interface (TWI) class.
 */
class TWI {
public:
  /**
   * Abstract Two-Wire Interface Device Driver class.
   */
  class Device {
  public:
    /**
     * Construct Two-Wire Interface Device Driver with given bus and
     * device address.
     * @param[in] twi bus manager.
     * @param[in] addr device address.
     */
    Device(TWI& twi, uint8_t addr) :
      m_twi(twi),
      m_addr(addr << 1)
    {
    }

    /**
     * Start transaction. Return true(1) if successful otherwise
     * false(0).
     * @return bool.
     */
    bool acquire()
    {
      return (m_twi.acquire());
    }

    /**
     * Stop transaction. Return true(1) if successful otherwise
     * false(0).
     * @return bool.
     */
    bool release()
    {
      return (m_twi.release());
    }

    /**
     * Read data from device to given buffer.
     * @param[in] buf buffer pointer.
     * @param[in] count buffer size in bytes.
     * @return number of bytes read or negative error code.
     */
    int read(void* buf, size_t count)
    {
      return (m_twi.read(m_addr, buf, count));
    }

    /**
     * Write data from the given buffer to device.
     * @param[in] buf buffer pointer.
     * @param[in] count buffer size in bytes.
     * @return number of bytes written or negative error code.
     */
    int write(const void* buf, size_t count)
    {
      return (m_twi.write(m_addr, buf, count));
    }

    /**
     * Write data to device with from given io vector.
     * @param[in] vp io vector pointer.
     * @return number of bytes written or negative error code.
     */
    int write(iovec_t* vp)
    {
      return (m_twi.write(m_addr, vp));
    }

  protected:
    /** Two-Wire Interface Manager. */
    TWI& m_twi;

    /** Device address. */
    uint8_t m_addr;
  };

  /** Default Two-Wire Interface clock: 100 KHz. */
  static const uint32_t DEFAULT_FREQ = 100000L;

  /**
   * Default constructor.
   */
  TWI() :
    m_busy(false)
  {}

  /**
   * @override{TWI}
   * Start bus transaction. Return true(1) if successful otherwise
   * false(0).
   * @return bool.
   */
  virtual bool acquire() = 0;

  /**
   * @override{TWI}
   * Stop bus transaction. Return true(1) if successful otherwise
   * false(0).
   * @return bool.
   */
  virtual bool release() = 0;

  /**
   * @override{TWI}
   * Read data from device with given address into given buffer.
   * @param[in] addr device address.
   * @param[in] buf buffer pointer.
   * @param[in] count buffer size in bytes.
   * @return number of bytes read or negative error code.
   */
  virtual int read(uint8_t addr, void* buf, size_t count) = 0;

  /**
   * @override{TWI}
   * Write data to device with given address from given buffer.
   * @param[in] addr device address.
   * @param[in] buf buffer pointer.
   * @param[in] count buffer size in bytes.
   * @return number of bytes written or negative error code.
   */
  virtual int write(uint8_t addr, const void* buf, size_t count)
  {
    iovec_t vec[2];
    iovec_t* vp = vec;
    iovec_arg(vp, buf, count);
    iovec_end(vp);
    return (write(addr, vec));
  }

  /**
   * @override{TWI}
   * Write data to device with from given io vector.
   * @param[in] addr device address.
   * @param[in] vp io vector pointer.
   * @return number of bytes written or negative error code.
   */
  virtual int write(uint8_t addr, iovec_t* vp) = 0;

protected:
  /** Device driver semaphore. */
  volatile bool m_busy;
};
#endif
