# Arduino-TWI

The TWI library is an abstract interface for I2C device drivers. The
library includes a hardware and software, and example device drivers
for Real-Time Clock (DS1307), 2-Wire Serial EEPROM (AT24CXX), I2C
Humidity and Temperature Sensor (Si70XX), and Remote 8-bit I/O
expander (PCF8574/PCF8574A).

The software implementation of the TWI interface uses the
[Arduino-GPIO](https://github.com/mikaelpatel/Arduino-GPIO)
library. Both bus manager implementations supports both repeated start
condition and device driver mutex on multi-tasking.

Repeated start condition allows combined write/read operations to one
or more devices without releasing the bus and thus with the guarantee
that the operation is not interrupted (when using multiple masters).

Device driver mutex allows a task to complete a device driver function
in a synchronized manner when using the
[Arduino-Scheduler](https://github.com/mikaelpatel/Arduino-Scheduler).

Version: 1.5

## Classes

* [Abstract Two-Wire Bus Manager and Device Driver Interface, TWI](./src/TWI.h)
* [Hardware Two-Wire Interface, Hardware::TWI](./src/Hardware/TWI.h)
* [Software Two-Wire Interface, Software::TWI](./src/Software/TWI.h)
* [Two-Wire Serial EEPROM, AT24CXX](./src/Driver/AT24CXX.h)
* [Real-Time Clock, DS1307](./src/Driver/DS1307.h)
* [Humidity and Temperature Sensor, Si70XX](./src/Driver/Si70XX.h)
* [Remote 8-bit I/O expander, PCF8574](./src/Driver/PCF8574.h)

## Example Sketches

* [AT24CXX](./examples/AT24CXX)
* [DS1307](./examples/DS1307)
* [Multitasking](./examples/Multitasking)
* [PCF8574](./examples/PCF8574)
* [Scanner](./examples/Scanner)
* [Si7021](./examples/Si7021)

## Dependencies

* [General Purpose Input/Output library for Arduino, GPIO](https://github.com/mikaelpatel/Arduino-GPIO)
