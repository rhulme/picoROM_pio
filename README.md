# picoROM_pio
This is a proof-of-concept based on the idea of [picoROM](https://github.com/nickbild/picoROM) to simulate an EEPROM chip simply but using just PIO and DMA to improve the performance.  I have no immediate use for this project myself but it piqued my interest to see what kind of improvement could be achieved (see below).


The code as presented here just has an address and data bus of 4 bits to make testing easier but these are defined in [eeprom.pio](eeprom.pio) and can be easily changed and will not affect the efficiency.

The data bus is currently limited to a maximum of 8 bits but this can be increased by changing `eeprom_data` from `uint8_t` to `uint16_t` and changing the [DMA transfer size](https://github.com/rhulme/picoROM_pio/blob/59c7fecd0b3fbbd9fe42e8ed0e4a63d1154a1bf2/main.c#L70) of the second channel in `init()` from `DMA_SIZE_8` to `DMA_SIZE_16`.


A second PIO program ([eeprom_test.pio](eeprom_test.pio)) is started to cycle through addresses.  The output pins for the test program are mapped to the same pins as the main program.

 The test code is currently limited to an address bus of up to 16 bits but this can be increased in the same way as the data bus by changing the `eeprom_addr` array from `uint16_t` to `uint32_t` and changing the DMA transfer size in `init_test()` from `DMA_SIZE_16` to `DMA_SIZE_32`.  At some point though you're going to run out of pins!

Using only PIO and DMA means that the timing is not affected by any software running on the main cores.

Leaving sysclk running at the usual 125MHz, the lag between the address and data buses appears to vary between approximately 160nS and 215nS.  The variation is almost certainly due to the fact that the sequence of "read address bus, fetch data" is performed constantly and so depending on when the address bus changes during that sequence the lag may sometimes be more and sometimes less.

If a lower guaranteed lag is required, the chip can be overclocked by increasing sysclk.


***
***Since I approached the implementation as an exercise in using PIO and DMA without any actual use for it, I unfortunately didn't compare the results properly with the original implementation.  I've now done that [here](docs/README.md), which should be read before blindly using the code presented here.***
***


## Implementation
The code is fairly straightforward.  There are only really the following requirements:
* The address bus pins must be contiguous.
* The data bus pins must be contiguous.
* The array holding the EEPROM data must start on an address aligned to the size of the EEPROM's memory space.

The data bus pins are currently defined to follow the address bus pins but this is not a requirement.

The PIO state machine communicates via a transmit (to the state machine) and a receive (from the state machine) FIFO.  Two DMA channels are used.

After priming the SM with the top-most bits of the base address of the EEPROM in the Pico's memory space, the operation is as follows:

1. Shift the address bus in to the ISR register
1. Shift the top-most bits in to form the absolute address of the required data byte
1. This address is automatically pushed into the RX FIFO
1. The first DMA channel is triggered and copies the address into the "from" address register of the second DMA channel, which automatically starts the second channel
1. The second DMA channel reads from the address provided and writes the byte into the SM's TX FIFO.
1. When the transfer is complete, the first DMA channel is re-primed to wait for the next address.
1. The SM pulls the data in and puts it on the data bus
1. Loop back to start

There may be other improvements that could be made but for now I'll leave that as an exercise for someone else.

## Building
I built the project under Linux but I assume there is no reason why it should not build under Windows or MacOS
```
mkdir build
cd build
cmake ..
make
```
