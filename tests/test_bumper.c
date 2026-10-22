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

// Host unit tests for bumper.c: debounced bumper presses must emit exactly
// one CORD "bump" event per press, with the correct angle sign.

#include "test_framework.h"

#include <string.h>
#include "xc.h"
#include "../bumper.h"
#include "../cordlib/cord_connection.h"
#include "../cordlib/cord_hw.h"
#include "../gpio.h"

// bumper.c internals (debounce state) reset between tests
extern bool pressed_left;
extern bool pressed_right;
extern uint_fast32_t moment_left;
extern uint_fast32_t moment_right;

static uint_fast32_t sim_time_ms = 0;
static uint_fast32_t cb_get_time_ms(void) {
    return sim_time_ms;
}

static void cb_toggle_interrupts(bool enable) {
    (void) enable;
}

// count "bump" events in the output buffer
static int count_bump_events(char *out, size_t max) {
    int count = 0;
    char *p;
    size_t n = 0;
    while (cord_connection_output_size() > 0 && n + 1 < max) {
        out[n++] = (char) cord_connection_output_pull();
    }
    out[n] = 0;
    for (p = out; (p = strstr(p, "\"e\":\"bump\"")) != NULL; p++)
        count++;
    return count;
}

// Advance time past the debounce window and run one step with the given pin
// levels, letting bumper_step() latch the new state.
static void settle(uint_fast32_t advance_ms, int left_pin, int right_pin) {
    sim_time_ms += advance_ms;
    PORTGbits.RG9 = left_pin;
    PORTGbits.RG15 = right_pin;
    bumper_step();
}

static void reset_all(void) {
    sim_time_ms = 1000;
    pressed_left = false;
    pressed_right = false;
    moment_left = 0;
    moment_right = 0;
    PORTGbits.RG9 = 1;  // PIN_BUMPER_SWITCH_LEFT: active LOW -> unpressed = high
    PORTGbits.RG15 = 1; // PIN_BUMPER_SWITCH_RIGHT: active LOW -> unpressed = high
    while (cord_connection_input_size() > 0)
        cord_connection_input_pull();
    while (cord_connection_output_size() > 0)
        cord_connection_output_pull();
}

static void run_tests(void) {
    char out[512];

    set_callback_get_time_ms(&cb_get_time_ms);
    set_callback_toggle_interrupts(&cb_toggle_interrupts);

    // left bumper press (pin goes low = pressed)
    reset_all();
    settle(200, 1, 1); // let the initial state settle past the debounce window
    settle(200, 0, 1); // press -> event emitted
    CHECK_EQ_INT(count_bump_events(out, sizeof(out)), 1);
    CHECK(strstr(out, "\"angle\":-45") != NULL); // left bumper

    // holding the bumper must not re-trigger (pin stays pressed the whole
    // time; bumper_step() latches pressed=true and does not emit again)
    settle(200, 0, 1);
    settle(200, 0, 1);
    CHECK_EQ_INT(count_bump_events(out, sizeof(out)), 0);
    // and it is still latched as pressed
    CHECK(pressed_left);

    // release and press again -> new event
    settle(200, 1, 1); // release
    settle(200, 0, 1); // press again
    CHECK_EQ_INT(count_bump_events(out, sizeof(out)), 1);

    // right bumper press
    reset_all();
    settle(200, 1, 1);
    settle(200, 1, 0); // right press -> event emitted
    CHECK_EQ_INT(count_bump_events(out, sizeof(out)), 1);
    CHECK(strstr(out, "\"angle\":45") != NULL); // right bumper

    // bounce faster than the debounce time is rejected: the pin must stay
    // stable for DEBOUNCE_TIME_MS before a transition is accepted
    reset_all();
    settle(200, 1, 1);
    settle(200, 0, 1); // stable press -> one event
    CHECK_EQ_INT(count_bump_events(out, sizeof(out)), 1);
    settle(200, 1, 1); // release
    CHECK_EQ_INT(count_bump_events(out, sizeof(out)), 0);
    settle(10, 0, 1);  // bounce: press (unstable)
    settle(10, 1, 1);  // bounce: release (unstable)
    settle(10, 0, 1);  // bounce: press (unstable)
    CHECK_EQ_INT(count_bump_events(out, sizeof(out)), 0); // nothing accepted yet
    settle(200, 0, 1); // now stable pressed for 200 ms -> accepted exactly once
    CHECK_EQ_INT(count_bump_events(out, sizeof(out)), 1);

    // both bumpers at once -> two events
    reset_all();
    settle(200, 1, 1);
    settle(200, 0, 0); // both pressed -> two events
    CHECK_EQ_INT(count_bump_events(out, sizeof(out)), 2);
}

TEST_MAIN()
