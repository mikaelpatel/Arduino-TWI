# Arduino-TWI

The TWI library is an abstract interface for I2C device drivers. The
library includes a software implementation using GPIO and example
device drivers for the DS1307, Real-Time Clock, and AT24CXX, 2-Wire
Serial EEPROM.

The software implementation support both repeated start condition
and device driver mutex on multi-tasking.

Repeated start condition allows combined write/read operations to one
or more devices without releasing the bus and thus with the guarantee
that the operation is not interrupted.

Device driver mutex allows a task to complete a device driver function
in a synchronized manner.

## Classes

* [Abstract Two-Wire Interface, TWI](./src/TWI.h)
* [Software Two-Wire Interface, Software::TWI](./src/Software/TWI.h)
* [AT24CXX, Two-Wire Serial EEPROM](./src/AT24CXX.h)
* [DS1307, Real-Time Clock](./src/DS1307.h)

## Example Sketches

* [AT24CXX](./examples/AT24CXX)
* [DS1307](./examples/DS1307)
* [Multitasking](./examples/Multitasking)
* [Scanner](./examples/Scanner)
