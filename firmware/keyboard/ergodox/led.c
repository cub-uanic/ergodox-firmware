/* ----------------------------------------------------------------------------
 * Copyright (c) 2013 Ben Blazak <benblazak.dev@gmail.com>
 * Released under The MIT License (see "doc/licenses/MIT.md")
 * Project located at <https://github.com/benblazak/ergodox-firmware>
 * ------------------------------------------------------------------------- */

/**                                                                 description
 * Implements the "LED" section of '.../firmware/keyboard.h'
 *
 * Code is specific to Teensy 2.0
 */

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "../../../firmware/keyboard.h"

// ----------------------------------------------------------------------------

#ifndef OPT__LED_BRIGHTNESS
    #error "OPT__LED_BRIGHTNESS not defined"
#endif
/**                                      macros/OPT__LED_BRIGHTNESS/description
 * A percentage of maximum brightness, with '1' being greatest and '0' being
 * not quite off
 */

#ifdef ENABLE_LEFT_LED
uint8_t _led_11_state = 0;  // left red (top)
uint8_t _led_12_state = 0;  // left yellow (middle)
uint8_t _led_13_state = 0;  // left green (bottom)
#endif

// ----------------------------------------------------------------------------

void kb__led__on(uint8_t led) {
    switch(led) {
        case 1: (DDRB |=  (1<<5)); break;  // topmost
        case 2: (DDRB |=  (1<<6)); break;  // middle
        case 3: (DDRB |=  (1<<7)); break;  // bottommost
        case 4:                    break;
        case 5:                    break;
#ifdef ENABLE_LEFT_LED
        case 11: _led_11_state = 1; break;
        case 12: _led_12_state = 1; break;
        case 13: _led_13_state = 1; break;
#endif
    };
}

void kb__led__off(uint8_t led) {
    switch(led) {
        case 1: (DDRB &= ~(1<<5)); break;  // topmost
        case 2: (DDRB &= ~(1<<6)); break;  // middle
        case 3: (DDRB &= ~(1<<7)); break;  // bottommost
        case 4:                    break;
        case 5:                    break;
#ifdef ENABLE_LEFT_LED
        case 11: _led_11_state = 0; break;
        case 12: _led_12_state = 0; break;
        case 13: _led_13_state = 0; break;
#endif
    };
}

void kb__led__set(uint8_t led, float n) {
    switch(led) {
        case 1: (OCR1A = (uint8_t)((n) * 0xFF)); break;  // topmost
        case 2: (OCR1B = (uint8_t)((n) * 0xFF)); break;  // middle
        case 3: (OCR1C = (uint8_t)((n) * 0xFF)); break;  // bottommost
        case 4:                                  break;
        case 5:                                  break;
#ifdef ENABLE_LEFT_LED
        case 11:                                 break;
        case 12:                                 break;
        case 13:                                 break;
#endif
    };
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool kb__led__read(uint8_t led) {
    switch(led) {
        case 1: return (PINB & (1<<5));  // topmost
        case 2: return (PINB & (1<<6));  // middle
        case 3: return (PINB & (1<<7));  // bottommost
#ifdef ENABLE_LEFT_LED
        case 11: return(_led_11_state);
        case 12: return(_led_12_state);
        case 13: return(_led_13_state);
#endif
        case 4: ;
        case 5: ;
    };
    return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void kb__led__all_on(void) {
    for (uint8_t i=1; i<=3; i++)
        kb__led__on(i);
#ifdef ENABLE_LEFT_LED
    for (uint8_t i=11; i<=13; i++)
        kb__led__on(i);
#endif
}

void kb__led__all_off(void) {
    for (uint8_t i=1; i<=3; i++)
        kb__led__off(i);
#ifdef ENABLE_LEFT_LED
    for (uint8_t i=11; i<=13; i++)
        kb__led__off(i);
#endif
}

void kb__led__all_set(float n) {
    for (uint8_t i=1; i<=3; i++)
        kb__led__set(i, n);
#ifdef ENABLE_LEFT_LED
    for (uint8_t i=11; i<=13; i++)
        kb__led__set(i, n);
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void kb__led__state__power_on(void) {
    kb__led__all_set( OPT__LED_BRIGHTNESS / 10 );
    kb__led__all_on();
}

void kb__led__state__ready(void) {
    kb__led__all_off();
    kb__led__all_set( OPT__LED_BRIGHTNESS );
}

void kb__led__delay__usb_init(void) {
    // need to delay for a total of ~1 second
    kb__led__set( 1, OPT__LED_BRIGHTNESS );
    _delay_ms(333);
    kb__led__set( 2, OPT__LED_BRIGHTNESS );
    _delay_ms(333);
    kb__led__set( 3, OPT__LED_BRIGHTNESS );
    _delay_ms(333);
}

