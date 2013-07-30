/* ----------------------------------------------------------------------------
 * Copyright (c) 2012, 2013 Ben Blazak <benblazak.dev@gmail.com>
 * Released under The MIT License (see "doc/licenses/MIT.md")
 * Project located at <https://github.com/benblazak/ergodox-firmware>
 * ------------------------------------------------------------------------- */

/**                                                                 description
 * MCP23018 (driven by Teensy 2.0 over TWI) specific code, helping to implement
 * the "controller" section of '.../firmware/keyboard.h'
 */


#include <stdbool.h>
#include <stdint.h>
#include <util/twi.h>
#include "../../../../firmware/keyboard.h"
#include "../../../../firmware/lib/twi.h"
#include "./mcp23018.h"

// ----------------------------------------------------------------------------

#if OPT__KB__ROWS != 6 || OPT__KB__COLUMNS != 14
    #error "Expecting different keyboard dimensions"
#endif

#if  ( OPT__MCP23018__DRIVE_ROWS && OPT__MCP23018__DRIVE_COLUMNS )   \
 || !( OPT__MCP23018__DRIVE_ROWS || OPT__MCP23018__DRIVE_COLUMNS )
    #error "MCP23018 pin drive direction incorrectly set"
#endif
/**                              macros/(group) pin drive direction/description
 * Select which set of pins (rows or columns) will drive (alternate between
 * hi-Z and drive low), and which will be inputs (hi-Z)
 *
 * Members:
 * - `OPT__MCP23018__DRIVE_ROWS`
 * - `OPT__MCP23018__DRIVE_COLUMNS`
 *
 * 
 * Notes:
 *
 * - You must set exactly one of these variables to `1`, and the other to `0`
 *
 * - If you are using internal diodes (inside the key switches), set
 *   `OPT__MCP23018__DRIVE_ROWS` to `1`
 * 
 * - If the diode cathode is towards the square solder pad, set
 *   `OPT__MCP23018__DRIVE_COLUMNS` to `1`
 * 
 * - If the diode cathode is towards the circular solder pad, set
 *   `OPT__MCP23018__DRIVE_ROWS` to `1`
 */

// ----------------------------------------------------------------------------

// register addresses (see "mcp23018.md")
#define IODIRA 0x00  // i/o direction register
#define IODIRB 0x01
#define GPPUA  0x0C  // GPIO pull-up resistor register
#define GPPUB  0x0D
#define GPIOA  0x12  // general purpose i/o port register (write modifies OLAT)
#define GPIOB  0x13
#define OLATA  0x14  // output latch register
#define OLATB  0x15

// TWI aliases
#define  TWI_ADDR        0b0100000
#define  TWI_ADDR_WRITE  ( (TWI_ADDR<<1) | TW_WRITE )
#define  TWI_ADDR_READ   ( (TWI_ADDR<<1) | TW_READ  )

#ifdef ENABLE_LEFT_LED
extern uint8_t _led_11_state;  // left red (top)
extern uint8_t _led_12_state;  // left yellow (middle)
extern uint8_t _led_13_state;  // left green (bottom)
#define LED_11_BIT_SHIFT    7  // in A
#define LED_12_BIT_SHIFT    6  // in B
#define LED_13_BIT_SHIFT    7  // in B
#endif

// ----------------------------------------------------------------------------

/**                                        functions/mcp23018__init/description
 * Initialize the MCP23018
 *
 * Returns:
 * - success: `0`
 * - failure: twi status code
 *
 * Notes:
 * - `twi__stop()` must be called *exactly once* for each twi block, the way
 *   things are currently set up.  this may change in the future.
 */
uint8_t mcp23018__init(void) {
    uint8_t ret;

    // set pin direction
    // - unused  : input  : 1
    // - input   : input  : 1
    // - driving : output : 0
    twi__start();
    ret = twi__send(TWI_ADDR_WRITE);
    if (ret) goto out;  // make sure we got an ACK
    twi__send(IODIRA);
    #if OPT__MCP23018__DRIVE_ROWS
#ifndef ENABLE_LEFT_LED
        twi__send(0b11111111);  // IODIRA
        twi__send(0b11000000);  // IODIRB
#else
        twi__send(0b01111111);  // IODIRA
        twi__send(0b00000000);  // IODIRB
#endif
    #elif OPT__MCP23018__DRIVE_COLUMNS
#ifndef ENABLE_LEFT_LED
        twi__send(0b10000000);  // IODIRA
        twi__send(0b11111111);  // IODIRB
#else
        twi__send(0b00000000);  // IODIRA
        twi__send(0b00111111);  // IODIRB
#endif
    #endif
    twi__stop();

    // set pull-up
    // - unused  : on  : 1
    // - input   : on  : 1
    // - driving : off : 0
    twi__start();
    ret = twi__send(TWI_ADDR_WRITE);
    if (ret) goto out;  // make sure we got an ACK
    twi__send(GPPUA);
    #if OPT__MCP23018__DRIVE_ROWS
#ifndef ENABLE_LEFT_LED
        twi__send(0b11111111);  // GPPUA
        twi__send(0b11000000);  // GPPUB
#else
        twi__send(0b01111111);  // GPPUA
        twi__send(0b00000000);  // GPPUB
#endif
    #elif OPT__MCP23018__DRIVE_COLUMNS
#ifndef ENABLE_LEFT_LED
        twi__send(0b10000000);  // GPPUA
        twi__send(0b11111111);  // GPPUB
#else
        twi__send(0b00000000);  // GPPUA
        twi__send(0b00111111);  // GPPUB
#endif
    #endif
    twi__stop();

    // set logical value (doesn't matter on inputs)
    // - unused  : hi-Z : 1
    // - input   : hi-Z : 1
    // - driving : hi-Z : 1
    twi__start();
    ret = twi__send(TWI_ADDR_WRITE);
    if (ret) goto out;  // make sure we got an ACK
    twi__send(OLATA);
#ifndef ENABLE_LEFT_LED
    twi__send(0b11111111);  //OLATA
    twi__send(0b11111111);  //OLATB
#else
    twi__send(0b11111111 & ~(_led_11_state<<LED_11_BIT_SHIFT));  //OLATA
    twi__send(0b11111111 & ~(_led_12_state<<LED_12_BIT_SHIFT) & ~(_led_13_state<<LED_13_BIT_SHIFT));  //OLATB
#endif

out:
    twi__stop();
    return ret;
}

/**                               functions/mcp23018__update_matrix/description
 * Update the MCP23018 (left hand) half of the given matrix
 *
 * Arguments:
 * - `matrix`: A matrix of booleans, indicating whether the key at the given
 *   matrix location is pressed or released
 *
 * Returns:
 * - success: `0`
 * - failure: twi status code
 */
uint8_t mcp23018__update_matrix(bool matrix[OPT__KB__ROWS][OPT__KB__COLUMNS]) {
    uint8_t ret, data;

    // initialize things, just to make sure
    // - it's not appreciably faster to skip this, and it takes care of the
    //   case when the i/o expander isn't plugged in during the first
    //   init()
    ret = mcp23018__init();

    // if there was an error
    if (ret) {
        // clear our part of the matrix
        for (uint8_t row=0; row<=5; row++)
            for (uint8_t col=0; col<=6; col++)
                matrix[row][col] = 0;

        return ret;
    }

    // update our part of the matrix ..........................................

    #if OPT__MCP23018__DRIVE_ROWS
        for (uint8_t row=0; row<=5; row++) {
            // set active row low  : 0
            // set other rows hi-Z : 1
            twi__start();
            twi__send(TWI_ADDR_WRITE);
            twi__send(GPIOB);
#ifndef ENABLE_LEFT_LED
            twi__send( 0xFF & ~(1<<(5-row)) );
#else
            twi__send( 0xFF & ~(1<<(5-row)) & ~(_led_12_state<<LED_12_BIT_SHIFT) & ~(_led_13_state<<LED_13_BIT_SHIFT));
#endif
            twi__stop();

            // read column data
            twi__start();
            twi__send(TWI_ADDR_WRITE);
            twi__send(GPIOA);
            twi__start();
            twi__send(TWI_ADDR_READ);
            twi__read(&data);
            twi__stop();

            // update matrix
            for (uint8_t col=0; col<=6; col++) {
                matrix[row][col] = !( data & (1<<col) );
            }
        }

        // set all rows hi-Z : 1
        twi__start();
        twi__send(TWI_ADDR_WRITE);
        twi__send(GPIOB);
#ifndef ENABLE_LEFT_LED
        twi__send(0xFF);
#else
        twi__send( 0xFF & ~(_led_12_state<<LED_12_BIT_SHIFT) & ~(_led_13_state<<LED_13_BIT_SHIFT));
#endif
        twi__stop();

    #elif OPT__MCP23018__DRIVE_COLUMNS
        for (uint8_t col=0; col<=6; col++) {
            // set active column low  : 0
            // set other columns hi-Z : 1
            twi__start();
            twi__send(TWI_ADDR_WRITE);
            twi__send(GPIOA);
#ifndef ENABLE_LEFT_LED
            twi__send( 0xFF & ~(1<<col) );
#else
            twi__send( 0xFF & ~(1<<col) & ~(_led_11_state<<LED_11_BIT_SHIFT));
#endif
            twi__stop();

            // read row data
            twi__start();
            twi__send(TWI_ADDR_WRITE);
            twi__send(GPIOB);
            twi__start();
            twi__send(TWI_ADDR_READ);
            twi__read(&data);
            twi__stop();

            // update matrix
            for (uint8_t row=0; row<=5; row++) {
                matrix[row][col] = !( data & (1<<(5-row)) );
            }
        }

        // set all columns hi-Z : 1
        twi__start();
        twi__send(TWI_ADDR_WRITE);
        twi__send(GPIOA);
#ifndef ENABLE_LEFT_LED
        twi__send(0xFF);
#else
        twi__send( 0xFF & ~(_led_11_state<<LED_11_BIT_SHIFT));
#endif
        twi__stop();

    #endif

    // /update our part of the matrix .........................................

    return ret;  // success
}

