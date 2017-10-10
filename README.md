# Arduino-TWI

The TWI library is an abstract interface for I2C device drivers. The
library includes a hardware and software bus manager, and example
device drivers for I2C Humidity and Temperature Sensor (Si70XX),
Remote 8-bit I/O expander (PCF8574/PCF8574A), Digital Pressure
Sensor (BMP085), and Single/Multi-Channel 1-Wire Master (DS2482).

The software bus manager implementation of the TWI interface uses the
[Arduino-GPIO](https://github.com/mikaelpatel/Arduino-GPIO)
library. Both software and avr hardware bus manager implementations
supports repeated start condition and device driver mutex on
multi-tasking.

Repeated start condition allows combined write/read operations to one
or more devices without releasing the bus and thus with the guarantee
that the operation is not interrupted (when using multiple masters).

Device driver mutex allows a task to complete a device driver function
in a synchronized manner when using the
[Arduino-Scheduler](https://github.com/mikaelpatel/Arduino-Scheduler).

Version: 1.9

## Classes

* [Abstract Two-Wire Bus Manager and Device Driver Interface, TWI](./src/TWI.h)
* [AVR Two-Wire Bus Manager, Hardware::TWI](./src/Hardware/AVR/TWI.h)
* [SAM Two-Wire Bus Manager, Hardware::TWI](./src/Hardware/SAM/TWI.h)
* [Software Two-Wire Bus Manager, Software::TWI](./src/Software/TWI.h)
* [Digital Pressure Sensor, BMP085](./src/Driver/BMP085.h)
* [Humidity and Temperature Sensor, Si70XX](./src/Driver/Si70XX.h)
* [Remote 8-bit I/O expander, PCF8574](./src/Driver/PCF8574.h)
* [Single/Multi-Channel 1-Wire Master, DS2482-100/800](./src/Driver/DS2482.h)

## Example Sketches

* [Scanner](./examples/Scanner)
* [BMP085](./examples/BMP085)
* [DS2482](./examples/DS2482)
* [PCF8574](./examples/PCF8574)
* [Si7021](./examples/Si7021)

## Dependencies

* [Arduino-GPIO](https://github.com/mikaelpatel/Arduino-GPIO)
