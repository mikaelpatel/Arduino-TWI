/**
 * @file Hardware/SAM/TWI.h
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
#include "include/twi.h"
#include "variant.h"

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
  TWI(uint32_t freq = DEFAULT_FREQ) :
    m_state(IDLE_STATE)
  {
    // Initiate hardware registers
    pmc_enable_periph_clk(WIRE_INTERFACE_ID);
    PIO_Configure(g_APinDescription[PIN_WIRE_SDA].pPort,
		  g_APinDescription[PIN_WIRE_SDA].ulPinType,
		  g_APinDescription[PIN_WIRE_SDA].ulPin,
		  g_APinDescription[PIN_WIRE_SDA].ulPinConfiguration);
    PIO_Configure(g_APinDescription[PIN_WIRE_SCL].pPort,
		  g_APinDescription[PIN_WIRE_SCL].ulPinType,
		  g_APinDescription[PIN_WIRE_SCL].ulPin,
		  g_APinDescription[PIN_WIRE_SCL].ulPinConfiguration);
    m_twi = WIRE_INTERFACE;
    TWI_ConfigureMaster(m_twi, freq, VARIANT_MCK);
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
    m_state = BUSY_STATE;
    return (true);
  }

  /**
   * @override{TWI}
   * Stop transaction. Mark the bus as available. Return true(1) if
   * successful otherwise false(0).
   * @return bool.
   */
  virtual bool release()
  {
    bool res = true;

    // Check for terminating write sequence with stop condition
    if (m_state == WRITE_STATE) {
      TWI_Stop(m_twi);
      res = TWI_WaitTransferComplete(m_twi, RETRY_MAX);
    }

    // Mark bus manager as idle
    m_busy = false;
    m_state = IDLE_STATE;
    return (res);
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
    // Ignore zero length read
    if (count == 0) return (0);

    // Check if stop condition is needed before read
    if (m_state == WRITE_STATE) {
      TWI_Stop(m_twi);
      if (!TWI_WaitTransferComplete(m_twi, RETRY_MAX)) return (-1);
    }
    m_state = BUSY_STATE;

    // Read requested bytes from device
    int res = 0;
    TWI_StartRead(m_twi, addr >> 1, 0, 0);
    if (!TWI_WaitByteReceived(m_twi, RETRY_MAX)) return (-1);
    uint8_t* bp = (uint8_t*) buf;
    size_t size = count;
    while (size--) {
      if (size == 0) TWI_SendSTOPCondition(m_twi);
      if (!TWI_WaitByteReceived(m_twi, RETRY_MAX)) break;
      *bp++ = TWI_ReadByte(m_twi);
      res += 1;
    }
    TWI_WaitTransferComplete(m_twi, RETRY_MAX);

    // Return number of bytes read
    return (res);
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
    // Check for scan of given device
    if (vp == NULL) {
      TWI_StartWrite(m_twi, addr >> 1, 0, 0, 0);
      TWI_Stop(m_twi);
      if (!TWI_WaitTransferComplete(m_twi, RETRY_MAX)) return (-1);
      return (0);
    }

    // Check for preceeding write state
    bool start = (m_state != WRITE_STATE);
    int res = 0;
    m_state = WRITE_STATE;

    // Write buffer sequence to the device
    for(; vp->buf != NULL; vp++) {
      const uint8_t* bp = (const uint8_t*) vp->buf;
      size_t size = vp->size;
      while (size--) {
	if (start) {
	  TWI_StartWrite(m_twi, addr >> 1, 0, 0, *bp++);
	  start = false;
	}
	else
	  TWI_WriteByte(m_twi, *bp++);
	if (!TWI_WaitByteSent(m_twi, RETRY_MAX)) return (-1);
	res += 1;
      }
    }
    // Do not terminate with a stop condition. Additional
    // write may follow

    return (res);
  }

protected:
  /** Maximum number of retries. */
  static const uint32_t RETRY_MAX = 100000;

  /** TWI instance (libsam/twi). */
  Twi* m_twi;

  /** Device driver states. */
  enum state_t {
    IDLE_STATE,
    BUSY_STATE,
    WRITE_STATE
  };
  state_t m_state;

  /* TWI/Wire support functions. */
  static bool TWI_WaitTransferComplete(Twi* twi, uint32_t retry)
  {
    uint32_t sr;
    do {
      sr = TWI_GetStatus(twi);
      if (sr & TWI_SR_NACK) return false;
      if (--retry == 0) return false;
    } while ((sr & TWI_SR_TXCOMP) != TWI_SR_TXCOMP);
    return true;
  }

  static bool TWI_WaitByteSent(Twi* twi, uint32_t retry)
  {
    uint32_t sr;
    do {
      sr = TWI_GetStatus(twi);
      if (sr & TWI_SR_NACK) return false;
      if (--retry == 0) return false;
    } while ((sr & TWI_SR_TXRDY) != TWI_SR_TXRDY);
    return true;
  }

  static bool TWI_WaitByteReceived(Twi* twi, uint32_t retry)
  {
    uint32_t sr;
    do {
      sr = TWI_GetStatus(twi);
      if (sr & TWI_SR_NACK) return false;
      if (--retry == 0) return false;
    } while ((sr & TWI_SR_RXRDY) != TWI_SR_RXRDY);
    return true;
  }
};
};

#endif
