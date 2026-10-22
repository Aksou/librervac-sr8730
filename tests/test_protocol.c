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

// Host unit tests for the CORD protocol (cordlib): JSON parsing, packet
// framing (SEP = 0x1E), ring buffers, and the motor/move/beep commands.
//
// This also covers the XC32 fix: motor/move commands must be processed on
// non-SDCC compilers too (regression test for the #ifdef __SDCC removal).

#include "test_framework.h"

#include <string.h>
#include "xc.h"
#include "../cordlib/cord_connection.h"
#include "../cordlib/cord_protocol.h"
#include "../cordlib/cord_hw.h"
#include "../cordlib/cord_motors.h"
#include "../cordlib/cord_wheels.h"
#include "../cordlib/cord_sound.h"

// ---------------------------------------------------------------------------
// Callback doubles
// ---------------------------------------------------------------------------

static float motor_throttle[QUASI_MOTOR_LAST];
static float last_move_length;
static float last_move_radius;
static int move_calls;
static int arc_calls;
static float beep_freq;
static int beep_calls;

static void cb_set_motor_throttle(cord_motor_type motor, float throttle) {
    motor_throttle[motor] = throttle;
}

// cord_move/cord_move_arc are defined in cord_wheels.c; we link against the
// real ones and observe their side effects through hw_set_motor_throttle.
static uint_fast32_t sim_time_ms;
static uint_fast32_t cb_get_time_ms(void) {
    return sim_time_ms;
}

static void cb_toggle_interrupts(bool enable) {
    (void) enable;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void reset_all(void) {
    size_t i;
    for (i = 0; i < QUASI_MOTOR_LAST; i++)
        motor_throttle[i] = 999.0f;
    last_move_length = 0;
    last_move_radius = 0;
    move_calls = 0;
    arc_calls = 0;
    beep_freq = 0;
    beep_calls = 0;
    sim_time_ms = 1000;
    // drain any leftover bytes
    while (cord_connection_input_size() > 0)
        cord_connection_input_pull();
    while (cord_connection_output_size() > 0)
        cord_connection_output_pull();
}

// Feed a whole CORD packet (JSON + SEP) into the input buffer and process
// all pending input.
static void send_packet(const char *json) {
    const char *p;
    for (p = json; *p; p++)
        cord_connection_input_put((uint_fast8_t) *p);
    cord_connection_input_put(0x1E); // SEP
    while (cord_connection_input_size() > 0)
        cord_protocol_process_input();
}

// Pull the output buffer into a string (packets are SEP-terminated).
static void read_output(char *out, size_t max) {
    size_t n = 0;
    while (cord_connection_output_size() > 0 && n + 1 < max)
        out[n++] = (char) cord_connection_output_pull();
    out[n] = 0;
}

static void run_tests(void) {
    char out[512];

    set_callback_set_motor_throttle(&cb_set_motor_throttle);
    set_callback_get_time_ms(&cb_get_time_ms);
    set_callback_toggle_interrupts(&cb_toggle_interrupts);

    // --- motor command (regression: was compiled out on XC32) ---
    reset_all();
    send_packet("{\"c\":\"motor\",\"motor\":\"wheel left\",\"throttle\":0.5}");
    CHECK_NEAR(motor_throttle[WHEEL_LEFT], 0.5, 0.0001);
    CHECK_NEAR(cord_get_motor_throttle(WHEEL_LEFT), 0.5, 0.0001);

    reset_all();
    send_packet("{\"c\":\"motor\",\"motor\":\"wheel right\",\"throttle\":-0.25}");
    CHECK_NEAR(motor_throttle[WHEEL_RIGHT], -0.25, 0.0001);

    reset_all();
    send_packet("{\"c\":\"motor\",\"motor\":\"vacuum\",\"throttle\":1}");
    CHECK_NEAR(motor_throttle[VACUUM], 1.0, 0.0001);

    reset_all();
    send_packet("{\"c\":\"motor\",\"motor\":\"brush main\",\"throttle\":0.75}");
    CHECK_NEAR(motor_throttle[BRUSH_MAIN], 0.75, 0.0001);

    reset_all();
    send_packet("{\"c\":\"motor\",\"motor\":\"brush sides\",\"throttle\":0.1}");
    CHECK_NEAR(motor_throttle[BRUSH_SIDES], 0.1, 0.0001);

    // unknown motor: no crash, no throttle change
    reset_all();
    send_packet("{\"c\":\"motor\",\"motor\":\"flux capacitor\",\"throttle\":1}");
    CHECK_NEAR(motor_throttle[WHEEL_LEFT], 999.0, 0.0001);

    // --- move command (regression: was compiled out on XC32) ---
    // cord_move sets both wheels to +0.5 and schedules a stop.
    reset_all();
    send_packet("{\"c\":\"move\",\"length\":1.5}");
    CHECK_NEAR(motor_throttle[WHEEL_LEFT], 0.5, 0.0001);
    CHECK_NEAR(motor_throttle[WHEEL_RIGHT], 0.5, 0.0001);
    // move-finished event is emitted once the move duration elapsed
    sim_time_ms += 1000;
    cord_wheels_step();
    read_output(out, sizeof(out));
    CHECK(strstr(out, "move-finished") != NULL);

    reset_all();
    send_packet("{\"c\":\"move\",\"length\":-1.5}");
    CHECK_NEAR(motor_throttle[WHEEL_LEFT], -0.5, 0.0001);
    CHECK_NEAR(motor_throttle[WHEEL_RIGHT], -0.5, 0.0001);

    // arc: tiny length with positive radius -> rotate left
    reset_all();
    send_packet("{\"c\":\"move\",\"length\":0.0,\"radius\":0.3}");
    // length == 0 means neither branch runs (length != 0 required)
    CHECK_NEAR(motor_throttle[WHEEL_LEFT], 999.0, 0.0001);

    // arc with real values
    reset_all();
    send_packet("{\"c\":\"move\",\"length\":0.5,\"radius\":0.3}");
    // arc move leaves wheels running until the move duration elapses
    sim_time_ms += 2000;
    cord_wheels_step();
    read_output(out, sizeof(out));
    CHECK(strstr(out, "move-finished") != NULL);

    // --- beep command (host only; atof-based) ---
    reset_all();
    send_packet("{\"c\":\"beep\",\"pitch\":440,\"duration\":0.1,\"volume\":0.5}");
    Beep *beep = cord_buzzer_get_beep();
    CHECK(beep != NULL);
    if (beep) {
        CHECK_NEAR(beep->frequency, 440.0, 0.001);
        CHECK_NEAR(beep->duration, 0.1, 0.0001);
        CHECK_NEAR(beep->volume, 0.5, 0.0001);
    }

    // --- malformed / hostile packets must not crash ---
    reset_all();
    send_packet("not json at all");
    send_packet("{}");
    send_packet("{\"c\":}");
    send_packet("[1,2,3]");
    send_packet("{\"c\":\"unknown\"}");
    send_packet(""); // empty packet: just a SEP
    CHECK(1); // reached here without crashing

    // packet split across multiple process_input calls (byte at a time)
    reset_all();
    {
        const char *json = "{\"c\":\"motor\",\"motor\":\"vacuum\",\"throttle\":0.9}";
        const char *p;
        for (p = json; *p; p++) {
            cord_connection_input_put((uint_fast8_t) *p);
            cord_protocol_process_input();
        }
        cord_connection_input_put(0x1E);
        cord_protocol_process_input();
        CHECK_NEAR(motor_throttle[VACUUM], 0.9, 0.0001);
    }

    // --- ring buffers ---
    reset_all();
    {
        size_t i;
        for (i = 0; i < CORD_INPUT_BUFFER_SIZE + 50; i++)
            cord_connection_input_put((uint_fast8_t) i);
        CHECK(cord_connection_input_size() <= CORD_INPUT_BUFFER_SIZE);
        for (i = 0; i < CORD_OUTPUT_BUFFER_SIZE + 50; i++)
            cord_connection_output_put((uint_fast8_t) i);
        CHECK(cord_connection_output_size() <= CORD_OUTPUT_BUFFER_SIZE);
    }

    // --- cord_write_packet framing ---
    reset_all();
    cord_write_packet(100, "{\"c\":\"info\",\"x\":%d}", 42);
    read_output(out, sizeof(out));
    CHECK(strcmp(out, "{\"c\":\"info\",\"x\":42}\x1E") == 0);

    // oversized packet is dropped
    reset_all();
    cord_write_packet(10, "{\"c\":\"info\",\"this-is-a-very-long-packet\":1}");
    CHECK_EQ_INT(cord_connection_output_size(), 0);
}

TEST_MAIN()
