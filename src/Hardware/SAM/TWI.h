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

#ifndef HARDWARE_SAM_TWI_H
#define HARDWARE_SAM_TWI_H

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
    (void) freq;
  }

  /**
   * @override{TWI}
   * Start transaction for given device driver. Return true(1) if
   * successful otherwise false(0).
   * @return bool.
   */
  virtual bool acquire()
  {
    return (false);
  }

  /**
   * @override{TWI}
   * Stop transaction. Mark the bus as available. Return true(1) if
   * successful otherwise false(0).
   * @return bool.
   */
  virtual bool release()
  {
    return (false);
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
    (void) addr;
    (void) buf;
    (void) count;
    return (-1);
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
    (void) addr;
    (void) vp;
    return (-1);
  }
};
};

#endif
