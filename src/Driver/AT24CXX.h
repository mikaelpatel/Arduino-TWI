/**
 * @file AT24CXX.h
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

#ifndef AT24CXX_H
#define AT24CXX_H

#include "TWI.h"

#ifndef CHARBITS
#define CHARBITS 8
#endif

/**
 * Driver for the AT24CXX 2-Wire Serial EEPROM. Allows page write and
 * block read. Supports device AT24C32 (8K) to AT24C512 (64K). Default
 * AT24CXX device is AT24C32.
 *
 * @section Circuit
 * The TinyRTC with DS1307 also contains a 24C32 EEPROM.
 * @code
 *                       TinyRTC(24C32)
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
class AT24CXX : protected TWI::Device {
public:
  /** Number of bytes in max write page size. */
  const uint16_t PAGE_MAX;

  /** Memory addres page mask. */
  const uint16_t PAGE_MASK;

  /** Number of bytes on device. */
  const size_t SIZE;

  /**
   * Construct AT24CXX serial TWI EEPROM device access to given
   * chip address, page and memory size.
   * @param[in] twi bus manager.
   * @param[in] subaddr chip address (0..7, default 0).
   * @param[in] size in Kbits (default 32).
   * @param[in] page_max size of memory page (default 32 byte).
   */
  AT24CXX(TWI& twi,
	  uint8_t subaddr = 0,
	  const size_t size = 32,
	  const uint16_t page_max = 32) :
    TWI::Device(twi, 0x50 | (subaddr & 0x07)),
    PAGE_MAX(page_max),
    PAGE_MASK(page_max - 1),
    SIZE((size / CHARBITS) * 1024)
  {}

  /**
   * Return true(1) if the device is ready, write cycle is completed,
   * otherwise false(0).
   * @return bool.
   */
  bool is_ready()
  {
    if (!acquire()) return (false);
    int res = write(NULL);
    if (!release()) return (false);
    return (res == 0);
  }

  /**
   * Read eeprom block with the given size into the buffer from the
   * address. Return number of bytes read or negative error code.
   * @param[in] dest buffer to read from eeprom.
   * @param[in] src address in eeprom to read from.
   * @param[in] count number of bytes to read.
   * @return number of bytes or negative error code.
   */
  int read(void* dest, uint16_t src, size_t count)
  {
    uint8_t retry = RETRY_MAX;
    src = __builtin_bswap16(src);
    do {
      if (!acquire()) return (-1);
      int res = write(&src, sizeof(src));
      if (res == sizeof(src)) {
	res = read(dest, count);
	if (!release()) break;
	if (res == (int) count) return (res);
      }
      if (!release()) return (-1);
      delay(RETRY_DELAY_MS);
    } while (--retry);
    return (-1);
  }

  /**
   * Write eeprom block at given address with the contents from the
   * buffer. Return number of bytes written or negative error code.
   * @param[in] dest address in eeprom to read write to.
   * @param[in] src buffer to write to eeprom.
   * @param[in] count number of bytes to write.
   * @return number of bytes or negative error code.
   */
  int write(uint16_t dest, const void* src, size_t count)
  {
    uint8_t* p = (uint8_t*) src;
    size_t s = count;
    size_t n = PAGE_MAX - (dest & PAGE_MASK);
    if (n > s) n = s;
    while (1) {
      uint8_t retry = RETRY_MAX;
      uint16_t addr = __builtin_bswap16(dest);
      iovec_t vec[3];
      iovec_t* vp = vec;
      int res = -1;
      iovec_arg(vp, &addr, sizeof(addr));
      iovec_arg(vp, p, n);
      iovec_end(vp);
      do {
	if (!acquire()) continue;
	res = write(vec);
	if (!release()) continue;
	if (res > 0) break;
	delay(RETRY_DELAY_MS);
      } while (--retry);
      if (res < 0) return (-1);
      s -= n;
      if (s == 0) return (count);
      dest += n;
      p += n;
      n = (s < PAGE_MAX ? s : PAGE_MAX);
    }
  }

  using TWI::Device::read;
  using TWI::Device::write;

private:
  /** Maximum number of read/write page retries: 20 ms */
  static const uint8_t RETRY_MAX = 20;

  /** Retry delay time: 1 ms */
  static const uint8_t RETRY_DELAY_MS = 1;
};

/**
 * The AT24C32 provides 32,768 bits of serial electrically erasable
 * and programmable read only memory (EEPROM) organized as 4096 words
 * of 8 bits each. 32-Byte page write mode.
 *
 * See Atmel Product description (Rev. 0336K-SEEPR-7/03),
 * www.atmel.com/images/doc0336.pdf
 */
class AT24C32 : public AT24CXX {
public:
  AT24C32(TWI& twi, uint8_t subaddr = 0) :
    AT24CXX(twi, subaddr, 32, 32)
  {}
};

/**
 * The AT24C64 provides 65,536 bits of serial electrically erasable
 * and programmable read only memory (EEPROM) organized as 8192 words
 * of 8 bits each. 32-Byte page write mode.
 *
 * See Atmel Product description (Rev. 0336K-SEEPR-7/03),
 * www.atmel.com/images/doc0336.pdf
 */
class AT24C64 : public AT24CXX {
public:
  AT24C64(TWI& twi, uint8_t subaddr = 0) :
    AT24CXX(twi, subaddr, 64, 32)
  {}
};

/**
 * The AT24C128 provides 131,072 bits of serial electrically erasable
 * and programmable read only memory (EEPROM) organized as 16,384
 * words of 8 bits each. 64-Byte page write mode.
 *
 * See Atmel Product description (Rev. 0670T–SEEPR–3/07),
 * http://www.atmel.com/Images/doc0670.pdf
 */
class AT24C128 : public AT24CXX {
public:
  AT24C128(TWI& twi, uint8_t subaddr = 0) :
    AT24CXX(twi, subaddr, 128, 64)
  {}
};

/**
 * The AT24C256 provides 262,144 bits of serial electrically erasable
 * and programmable read only memory (EEPROM) organized as
 * 32,768 words of 8 bits each. 64-Byte page write mode.
 *
 * See Atmel Product description (Rev. 0670T–SEEPR–3/07),
 * http://www.atmel.com/Images/doc0670.pdf
 */
class AT24C256 : public AT24CXX {
public:
  AT24C256(TWI& twi, uint8_t subaddr = 0) :
    AT24CXX(twi, subaddr, 256, 64)
  {}
};

/**
 * The AT24C512 provides 524,288 bits of serial electrically erasable
 * and programmable read only memory (EEPROM) organized as 65,536
 * words of 8 bits each. 128-Byte page write mode.
 *
 * See Atmel Product description (Rev. 1116O–SEEPR–1/07),
 * http://www.atmel.com/Images/doc1116.pdf
 */
class AT24C512 : public AT24CXX {
public:
  AT24C512(TWI& twi, uint8_t subaddr = 0) :
    AT24CXX(twi, subaddr, 512, 128)
  {}
};
#endif
