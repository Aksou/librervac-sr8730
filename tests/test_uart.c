/*
 * This file is part of the LibreRVAC project
 *
 * Copyright © 2015-2016
 *     Aleks-Daniel Jakimenko-Aleksejev <alex.jakimenko@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Host unit tests for uart.c: the software-UART decoder must recover bytes
// from a simulated edge stream (start bit, 8 data bits LSB-first, stop bit),
// including the auto-baud training sequence.
//
// Note: this decoder is edge-driven, not timer-driven. On real hardware the
// change-notice ISR fires on every pin edge and uart_process_edges() runs in
// the main loop; edges that arrive while a frame is being decoded are
// timestamped relative to the (free-running) Timer4 and reconstructed from
// the elapsed time. In this test we feed every edge and call
// uart_process_edges() after each one, which matches the timing model the
// decoder expects (edges spaced by exactly one bit time).

#include "test_framework.h"

#include "xc.h"
#include "../uart.h"
#include "../cordlib/cord_connection.h"
#include "../cordlib/cord_protocol.h"
#include "../cordlib/cord_sound.h"
#include "../cordlib/cord_hw.h"
#include "../gpio.h"

void ChangeNotice_Handler(void); // the RX change-notice ISR

// uart.c internals exposed for testing
extern volatile bool initialized;
extern volatile uint_fast16_t ticks_per_bit;
extern volatile bool sent;
extern volatile uint_fast32_t ticks;
void T4Interrupt(void); // the Timer4 ISR drives the TX bit-bang

static uint_fast32_t sim_time_ms = 1000;
static uint_fast32_t cb_get_time_ms(void) {
    return sim_time_ms;
}

// Simulate one bit time on the RX pin: set the level, advance the timers,
// and fire the change-notice ISR (exactly what the hardware does on an edge).
static void feed_bit(int level, uint_fast16_t bit_ticks) {
    PORTCbits.RC13 = level ? 1 : 0; // PIN_CONNECTION_RX, active HIGH
    sim_tmr4 += bit_ticks;
    ticks += bit_ticks;
    ChangeNotice_Handler();
}

// Feed a full UART frame, calling the decoder after every edge.
static void feed_frame(uint8_t byte, uint_fast16_t bit_ticks) {
    int i;
    feed_bit(1, bit_ticks); uart_process_edges(); // idle / previous stop
    feed_bit(0, bit_ticks); uart_process_edges(); // start bit
    for (i = 0; i < 8; i++) {
        feed_bit((byte >> i) & 1, bit_ticks);
        uart_process_edges();
    }
    feed_bit(1, bit_ticks); uart_process_edges(); // stop bit
    sim_time_ms += 10;
}

static void reset_uart(void) {
    initialized = false;
    sent = true;
    ticks = 0;
    ticks_per_bit = UINT_FAST16_MAX;
    sim_time_ms = 1000;
    while (cord_connection_input_size() > 0)
        cord_connection_input_pull();
    while (cord_connection_output_size() > 0)
        cord_connection_output_pull();
}

static void run_tests(void) {
    size_t i;

    set_callback_get_time_ms(&cb_get_time_ms);

    // --- auto-baud training ---
    // The decoder trains on the first 10 edges (start + 8 bits + stop) and
    // only starts decoding afterwards. 0xAA (0b10101010) alternates every
    // bit, which exercises the edge timing the most.
    reset_uart();
    for (i = 0; i < 12; i++)
        feed_frame(0xAA, 100);
    CHECK(initialized);
    // 12 training frames x 10 edges = 120 edges; the decoder trains on edge
    // 10 and then decodes a byte per 10 edges, so the training frames
    // themselves end up in the input buffer.
    CHECK(cord_connection_input_size() >= 4);

    // --- decode real bytes ---
    reset_uart();
    for (i = 0; i < 12; i++)
        feed_frame(0xAA, 100);
    CHECK(initialized);
    while (cord_connection_input_size() > 0)
        cord_connection_input_pull(); // drain training bytes

    feed_frame(0x00, 100);
    feed_frame(0xFF, 100);
    feed_frame('Z', 100);
    feed_frame(0x1E, 100); // SEP
    CHECK_EQ_INT(cord_connection_input_size(), 4);
    CHECK_EQ_INT(cord_connection_input_pull(), 0x00);
    CHECK_EQ_INT(cord_connection_input_pull(), 0xFF);
    CHECK_EQ_INT(cord_connection_input_pull(), 'Z');
    CHECK_EQ_INT(cord_connection_input_pull(), 0x1E);

    // --- a full CORD packet survives the UART round trip ---
    reset_uart();
    for (i = 0; i < 12; i++)
        feed_frame(0xAA, 100);
    while (cord_connection_input_size() > 0)
        cord_connection_input_pull();
    {
        const char *packet = "{\"c\":\"beep\"}";
        const char *p;
        for (p = packet; *p; p++)
            feed_frame((uint8_t) *p, 100);
        feed_frame(0x1E, 100);
        // feed into the protocol layer
        while (cord_connection_input_size() > 0)
            cord_protocol_process_input();
        // beep queued (host build supports atof): exactly one beep
        Beep *beep = cord_buzzer_get_beep();
        CHECK(beep != NULL);
        if (beep)
            CHECK_NEAR(beep->frequency, 0.0, 0.001); // no pitch given -> 0
    }

    // --- TX path: output buffer is drained into output_byte and sent flag flips ---
    reset_uart();
    initialized = true;
    ticks_per_bit = 100;
    cord_connection_output_put('Q');
    uart_process_edges();
    CHECK(!sent); // a byte is being transmitted
    {
        int k;
        for (k = 0; k < 11; k++)
            T4Interrupt(); // start + 8 data + stop
    }
    CHECK(sent);
    CHECK_EQ_INT(cord_connection_output_size(), 0);
    // TX pin idles high after the stop bit (active HIGH)
    CHECK_EQ_INT(LATBbits.LATB7, 1);
}

TEST_MAIN()
