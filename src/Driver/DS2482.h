/**
 * @file DS2482.h
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2017, Mikael Patel
 *
 * This library is free hardware; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Hardware Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef DS2482_H
#define DS2482_H

#include "TWI.h"

/**
 * TWI Device Driver for DS2482, Single-Channel 1-Wire Master, TWI to
 * OWI Bridge Device.
 */
class DS2482 : protected TWI::Device {
public:
  /**
   * Construct one wire bus manager for DS2482.
   * @param[in] twi bus manager.
   * @param[in] subaddr sub-address for device (0..3).
   */
  DS2482(TWI& twi, uint8_t subaddr = 0) :
    TWI::Device(twi, 0x18 | (subaddr & 0x03))
  {
  }

  /**
   * Reset the one wire bus and check that at least one device is
   * presence.
   * @return true(1) if successful otherwise false(0).
   */
  bool one_wire_reset()
  {
    status_t status;
    uint8_t cmd;
    int count;
    bool res = false;

    // Issue one wire reset command
    cmd = ONE_WIRE_RESET;
    if (!TWI::Device::acquire()) return (false);
    count = TWI::Device::write(&cmd, sizeof(cmd));
    if (count != sizeof(cmd)) goto error;
    if (one_wire_await(status)) res = status.PPD;

  error:
    if (!TWI::Device::release()) return (false);
    return (res);
  }

  /**
   * Read a single bit from one wire bus. Returns value and true(1) if
   * successful otherwise false(0).
   * @param[out] value bit read.
   * @return true(1) if successful otherwise false(0).
   */
  bool one_wire_read_bit(bool& value)
  {
    status_t status;
    uint8_t cmd[2];
    int count;
    bool res = false;

    // Issue one wire single bit command with read data time slot
    cmd[0] = ONE_WIRE_SINGLE_BIT;
    cmd[1] = 0x80;
    if (!TWI::Device::acquire()) return (false);
    count = TWI::Device::write(cmd, sizeof(cmd));
    if (count != sizeof(cmd)) goto error;

    // Wait for one wire operation to complete
    res = one_wire_await(status);
    value = status.SBR;

  error:
    if (!TWI::Device::release()) return (false);
    return (res);
  }

  /**
   * Write a single bit to one wire bus. Returns true if successful
   * otherwise false.
   * @param[in] value bit to write.
   * @return bool.
   */
  bool one_wire_write_bit(bool value)
  {
    status_t status;
    uint8_t cmd[2];
    int count;
    bool res = false;

    // Issue one wire single bit command with given data
    cmd[0] = ONE_WIRE_SINGLE_BIT;
    cmd[1] = (value ? 0x80 : 0x00);
    if (!TWI::Device::acquire()) return (false);
    count = TWI::Device::write(cmd, sizeof(cmd));
    if (count != sizeof(cmd)) goto error;
    res = one_wire_await(status);

  error:
    if (!TWI::Device::release()) return (false);
    return (res);
  }

  /**
   * Read byte (8-bits) from one wire bus. Returns value and true(1)
   * if successful otherwise false(0).
   * @param[out] value bit read.
   * @return true(1) if successful otherwise false(0).
   */
  virtual bool one_wire_read_byte(uint8_t& value)
  {
    status_t status;
    uint8_t cmd;
    int count;

    // Issue one wire read byte command
    cmd = ONE_WIRE_READ_BYTE;
    if (!TWI::Device::acquire()) return (false);
    count = TWI::Device::write(&cmd, sizeof(cmd));
    if (count != sizeof(cmd)) goto error;

    // Wait for one wire operation to complete
    if (!one_wire_await(status)) goto error;
    if (!TWI::Device::release()) return (false);

    // Read data register value
    return (set_read_pointer(READ_DATA_REGISTER, value));

  error:
    TWI::Device::release();
    return (false);
  }

  /**
   * Write byte (8-bits) to one wire bus. Returns true(1) if
   * successful otherwise false(0).
   * @param[in] value byte write.
   * @return true(1) if successful otherwise false(0).
   */
  bool one_wire_write_byte(uint8_t value)
  {
    status_t status;
    uint8_t cmd[2];
    int count;
    bool res = false;

    // Issue one wire write byte command with given data
    cmd[0] = ONE_WIRE_WRITE_BYTE;
    cmd[1] = value;
    if (!TWI::Device::acquire()) return (res);
    count = TWI::Device::write(cmd, sizeof(cmd));
    if (count != sizeof(cmd)) goto error;
    res = one_wire_await(status);

  error:
    if (!TWI::Device::release()) return (false);
    return (res);
  }

  /**
   * Search (rom and alarm) support function. Reads 2-bits and writes
   * given direction 1-bit value when discrepancy 0b00 read. Writes
   * one(1) when 0b01 read, zero(0) on 0b10. Reading 0b11 is an error
   * state.
   * @param[in,out] dir bit to write when discrepancy read.
   * @return 2-bits read and bit written.
   */
  int8_t one_wire_triplet(uint8_t& dir)
  {
    status_t status;
    uint8_t cmd[2];
    int count;

    // Issue one wire single bit command with given data
    cmd[0] = ONE_WIRE_TRIPLET;
    cmd[1] = (dir ? 0x80 : 0x00);
    if (!TWI::Device::acquire()) return (-1);
    count = TWI::Device::write(cmd, sizeof(cmd));
    if (count != sizeof(cmd)) goto error;

    // Wait for one wire operation to complete
    if (!one_wire_await(status)) goto error;
    if (!TWI::Device::release()) return (-1);
    dir = status.DIR;
    return ((status >> 5) & 0x3);

  error:
    TWI::Device::release();
    return (-1);
  }

  /**
   * Global reset of device state machine logic. Returns true if
   * successful otherwise false.
   * @return bool.
   */
  bool device_reset()
  {
    status_t status;
    uint8_t cmd;
    int count;

    // Issue device reset command
    cmd = DEVICE_RESET;
    if (!TWI::Device::acquire()) return (false);
    count = TWI::Device::write(&cmd, sizeof(cmd));
    if (count != sizeof(cmd)) goto error;

    // Check status register for device reset
    count = TWI::Device::read(&status, sizeof(status));
    if (!TWI::Device::release()) return (false);
    return ((count == sizeof(status)) && status.RST);

  error:
    TWI::Device::release();
    return (false);
  }

  /**
   * Configure one wire bus master with given parameters. Returns true
   * if successful otherwise false.
   * @param[in] apu active pull-up (default true).
   * @param[in] spu strong pull-up (default false).
   * @param[in] iws one wire speed (default false).
   * @return bool.
   */
  bool write_configuration(bool apu = true, bool spu = false, bool iws = false)
  {
    config_t config;
    status_t status;
    uint8_t cmd[2];
    int count;

    // Set configuration bit-fields
    config.APU = apu;
    config.SPU = spu;
    config.IWS = iws;
    config.COMP = ~config;

    // Issue write configuration command with given setting
    cmd[0] = WRITE_CONGIFURATION;
    cmd[1] = config;
    if (!TWI::Device::acquire()) return (false);
    count = TWI::Device::write(cmd, sizeof(cmd));
    if (count != sizeof(cmd)) goto error;

    // Read status and check configuration
    count = TWI::Device::read(&status, sizeof(status));
    if (!TWI::Device::release()) return (false);
    return ((count == sizeof(status)) && !status.RST);

  error:
    TWI::Device::release();
    return (false);
  }

  /**
   * Device Registers, pp. 5. Valid Pointer Codes, pp. 10.
   */
  enum Register {
    STATUS_REGISTER = 0xf0,
    READ_DATA_REGISTER = 0xe1,
    CHANNEL_SELECTION_REGISTER = 0xd2,
    CONFIGURATION_REGISTER = 0xc3
  } __attribute__((packed));

  /**
   * Set the read pointer to the specified register. Return register
   * value or negative error code.
   * @param[in] addr register address.
   * @param[out] value read.
   * @return true(1) if successful otherwise false(0).
   */
  bool set_read_pointer(Register addr, uint8_t& value)
  {
    uint8_t cmd[2];
    int count;
    bool res = false;

    // Issue set read pointer command with given pointer
    cmd[0] = SET_READ_POINTER;
    cmd[1] = (uint8_t) addr;
    if (!TWI::Device::acquire()) return (false);
    count = TWI::Device::write(cmd, sizeof(cmd));
    if (count != sizeof(cmd)) goto error;

    // Read register value
    count = TWI::Device::read(&value, sizeof(value));
    res = (count == sizeof(value));

  error:
    if (!TWI::Device::release()) return (false);
    return (res);
  }

  /**
   * Select given channel (DS2482-800). Return true if successful
   * otherwise false.
   * @param[in] chan channel number (0..7).
   * @return bool.
   */
  bool channel_select(uint8_t chan)
  {
    uint8_t cmd[2];
    int count;

    // Check channel number
    if (chan > 7) return (false);

    // Issue channel select command with channel code
    cmd[0] = CHANNEL_SELECT;
    cmd[1] = (~chan << 4) | chan;
    if (!TWI::Device::acquire()) return (false);
    count = TWI::Device::write(cmd, sizeof(cmd));
    if (!TWI::Device::release()) return (false);
    return (count == sizeof(cmd));
  }

protected:
  /**
   * Function Commands, pp. 9-15.
   */
  enum {
    DEVICE_RESET = 0xf0,	//!< Device Reset.
    SET_READ_POINTER = 0xe1,	//!< Set Read Pointer.
    WRITE_CONGIFURATION = 0xd2,	//!< Write Configuration.
    CHANNEL_SELECT = 0xc3,	//!< Channel Select.
    ONE_WIRE_RESET = 0xb4,	//!< 1-Wire Reset.
    ONE_WIRE_SINGLE_BIT = 0x87,	//!< 1-Wire Single Bit.
    ONE_WIRE_WRITE_BYTE = 0xa5,	//!< 1-Wire Write Byte.
    ONE_WIRE_READ_BYTE = 0x96,	//!< 1-Wire Read Byte.
    ONE_WIRE_TRIPLET = 0x78	//!< 1-Wire Triplet.
  } __attribute__((packed));

  /**
   * Status Register, bit-fields, pp. 8-9.
   */
  union status_t {
    uint8_t as_uint8;		//!< Unsigned byte access.
    struct {			//!< Bitfield access (little endian).
      uint8_t IWB:1;		//!< 1-Wire Busy.
      uint8_t PPD:1;		//!< Presence-Pulse Detect.
      uint8_t SD:1;		//!< Short Detected.
      uint8_t LL:1;		//!< Logic Level.
      uint8_t RST:1;		//!< Device Reset.
      uint8_t SBR:1;		//!< Single Bit Result.
      uint8_t TSB:1;		//!< Triplet Second Bit.
      uint8_t DIR:1;		//!< Branch Direction Taken.
    };
    operator uint8_t()
    {
      return (as_uint8);
    }
  };

  /**
   * Configuration Register, bit-fields, pp. 5-6.
   */
  union config_t {
    uint8_t as_uint8;		//!< Unsigned byte access.
    struct {			//!< Bitfield access (little endian).
      uint8_t APU:1;		//!< Active Pullup.
      uint8_t ZERO:1;		//!< Always Zero(0).
      uint8_t SPU:1;		//!< Strong Pullup.
      uint8_t IWS:1;		//!< 1-Wire Speed.
      uint8_t COMP:4;		//!< Complement of lower 4-bits.
    };
    operator uint8_t()
    {
      return (as_uint8);
    }
    config_t()
    {
      as_uint8 = 0;
    }
  };

  /** Number of one-wire polls */
  static const int POLL_MAX = 20;

  /**
   * Wait for the one wire operation to complete. Poll the device
   * status.
   * @param[out] status device status on completion.
   * @return true(1) if successful otherwise false(0).
   */
  bool one_wire_await(status_t& status)
  {
    // Wait for one wire operation to complete
    for (int i = 0; i < POLL_MAX; i++) {
      int count = TWI::Device::read(&status, sizeof(status));
      if (count == sizeof(status) && !status.IWB) return (true);
    }
    return (false);
  }

};
#endif
