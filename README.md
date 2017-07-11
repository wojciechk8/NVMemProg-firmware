Firmware for the [NVMemProg](https://github.com/wojciechk8/NVMemProg) memory programmer.


### Building
Compiles with [sdcc](http://sdcc.sourceforge.net/). The [fx2lib](https://github.com/djmuhlestein/fx2lib)
library is required as well. Path to this library is configured in Makefile as FX2LIBDIR variable.
To compile the firmware, after fulfilling dependencies, run:
```
make
```


### Interface Modules
Interface Modules are C files with a memory interface implementation. They are
located in ifc_mod/ directory. File module.h in this directory declares functions
which should be implemented in each .c file. The firmware is compiled for each
such module, resulting in multiple Intel Hex targets in bin/ directory, with names
generated as follows:
```
[module name].ihx
```
A firmware file, therefore, consists of the main part (objects from the root directory) and a one
Interface Module, as an object from ifc_mod/.


### Programming
The compiled firmware can be loaded into EZ-USB microcontroller either manually
(cycfx2prog required):
```
make program IFC=[module name]
```
or automatically in the [host software](https://github.com/wojciechk8/NVMemProg_host), when
requesting an operation on a memory device.


### USB Endpoints utilization
The following endpoints are used to communicate through the USB bus:
* EP0:    Control endpoint, commands handling
* EP1OUT: FPGA configuration
* EP1IN:  Status reporting
* EP2OUT: Data transferred to the memory
* EP6IN:  Data read from the memory


### License
This firmware is licensed under GPL version 3. See the COPYING file for the full text of the license.
