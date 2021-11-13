#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "eeprom.pio.h"
#include "eeprom_test.pio.h"

#define EEPROM_SIZE (1 << eeprom_addr_bus_width)

// Address pins.
#define A0 0

// Data pins.
#define D0 (A0 + eeprom_addr_bus_width)

// Will be filled with addresses for the test routine to put on the bus
static uint16_t eeprom_addr[EEPROM_SIZE] __attribute__ ((aligned (EEPROM_SIZE * sizeof(uint16_t))));

// EEPROM data
static uint8_t eeprom_data[EEPROM_SIZE] __attribute__ ((aligned (EEPROM_SIZE))) = {0};


void init()
{
    PIO pio = pio0;

    // Our assembled program needs to be loaded into this PIO's instruction
    // memory. This SDK function will find a location (offset) in the
    // instruction memory where there is enough space for our program. We need
    // to remember this location!
    uint offsetAddr = pio_add_program(pio, &eeprom_program);

    // Find a free state machine on our chosen PIO (erroring if there are
    // none). Configure it to run our program, and start it, using the
    // helper function we included in our .pio file.
    uint smAddr = pio_claim_unused_sm(pio, true);
    eeprom_init(pio, smAddr, offsetAddr, A0, D0);
    
    // Pass the top bits of the EEPROM's base address to the state machine
    pio_sm_put_blocking(pio, smAddr, ((uint32_t)eeprom_data) >> eeprom_addr_bus_width);

    // We need two DMA channels
    int addr_chan = dma_claim_unused_channel(true);
    int data_chan = dma_claim_unused_channel(true);

    // The address channel transfers the EEPROM address from the SM's RX FIFO
    // to the data channel's read address trigger
    dma_channel_config c = dma_channel_get_default_config(addr_chan);

    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(pio, smAddr, false));

    dma_channel_configure(
        addr_chan,
        &c,
        &dma_hw->ch[data_chan].al3_read_addr_trig,  // Write to READ_ADDR_TRIG of data channel
        &pio->rxf[smAddr],                          // Read from state machine's RX FIFO
        1,                                          // Halt after each read
        false                                       // Don't start yet
    );


    // The data channel reads a byte from the EEPROM memory (address is
    // set by the address channel) and writes it to the SM's RX FIFO
    c = dma_channel_get_default_config(data_chan);

    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, false);
    channel_config_set_chain_to(&c, addr_chan);     // Trigger the address channel again when done

    dma_channel_configure(
        data_chan,
        &c,
        &pio->txf[smAddr],                          // Write to state machine's TX FIFO
        &eeprom_data[0],                            // Read from EEPROM memory (will be overwritten)
        1,                                          // Halt after each read
        false                                       // Don't start yet
    );

    // Everything is ready to go. Start listening for an address from the state machine.
    dma_start_channel_mask(1u << addr_chan);
}

void initTest()
{
    PIO pio = pio1;

    // Cycle through each address in turn
    for(uint32_t i = 0; i < EEPROM_SIZE; i++)
        eeprom_addr[i] = i;
    
    uint offset = pio_add_program(pio, &eeprom_test_program);
    uint sm = pio_claim_unused_sm(pio, true);
    eeprom_test_init(pio, sm, offset, A0);

    int chan = dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_ring(&c, false, eeprom_test_bus_width + 1); // (log2(sizeof(uint16_t)) == 1)

    dma_channel_configure(
        chan,
        &c,
        &pio->txf[sm],      // Initial write address
        &eeprom_addr[0],    // Initial read address
        0xffffffff,         // Count
        true                // Start immediately
    );
}

int main()
{
    // Fill the EEPROM with dummy data
    for(uint32_t i = 0; i < EEPROM_SIZE; i++)
        eeprom_data[i] = i % 0xff;

    // Overclock a bit if you need to
    //set_sys_clock_khz(133000, true);
    
    init();
    sleep_ms(1);
    initTest();
    
    while(true)
        tight_loop_contents();
}
