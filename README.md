# Arduino-TWI

The TWI library is an abstract interface for I2C device drivers. The
library also includes a software implementation using GPIO and an
example device driver for the DS1307, Real-Time Clock, and AT24CXX,
2-Wire Serial EEPROM.

## Classes

* [Abstract Two-Wire Interface, TWI](./src/TWI.h)
* [Software Two-Wire Interface, Software::TWI](./src/Software/TWI.h)
* [AT24CXX, Two-Wire Serial EEPROM](./src/AT24CXX.h)
* [DS1307, Real-Time Clock](./src/DS1307.h)

## Example Sketches

* [AT24CXX](./examples/AT24CXX.h)
* [DS1307](./examples/DS1307)
* [Scanner](./examples/Scanner)
