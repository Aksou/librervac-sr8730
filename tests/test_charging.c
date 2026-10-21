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

// Host unit tests for charging.c (dock connect/disconnect, charge detection).

#include "test_framework.h"

#include "xc.h"
#include "../charging.h"
#include "../battery.h"
#include "../cordlib/cord_hw.h"
#include "../cordlib/cord_sound.h"

// battery.c buffers
extern volatile int_fast32_t adc_thermistor[75];
extern volatile int_fast32_t adc_before_shunt[75];
extern volatile int_fast32_t adc_after_shunt[75];

// charging.c internals exposed for testing
extern bool configured;
extern bool docked;
extern volatile bool noisy;
extern volatile unsigned int charging_duty_cycle;

// cordlib time callback (hw_get_time_ms needs one)
static uint_fast32_t sim_time_ms = 0;
static uint_fast32_t test_get_time_ms(void) {
    return sim_time_ms;
}

static void fill_adc(int_fast32_t before, int_fast32_t after, int_fast32_t therm) {
    size_t i;
    for (i = 0; i < 75; i++) {
        adc_before_shunt[i] = before;
        adc_after_shunt[i] = after;
        adc_thermistor[i] = therm;
    }
}

static void reset_state(void) {
    configured = false;
    docked = false;
    noisy = false;
    charging_duty_cycle = 0;
    sim_time_ms = 0;
    fill_adc(0, 0, 0);
    // battery ~16V -> status ~0.55 (discharging scale)
    fill_adc(522, 522, 0);
    sim_oc_duty[3] = 0;
}

static void run_tests(void) {
    set_callback_get_time_ms(&test_get_time_ms);

    // Initially: not docked, not charging
    reset_state();
    CHECK(!is_docked());
    CHECK(!is_charging());

    // dock_connect with a low battery starts charging
    reset_state();
    dock_connect();
    CHECK(is_docked());
    CHECK(is_charging()); // battery ~16.5V -> status (16.5-14.8)/2.2 ~ 0.77 < 0.85
    CHECK_EQ_INT(charging_duty_cycle, 1000 / 30);

    // dock_connect with a full battery does not start charging
    reset_state();
    fill_adc(560, 560, 0); // ~17.2V -> status > 1 -> clamped to 1.0
    dock_connect();
    CHECK(is_docked());
    CHECK(!is_charging());

    // dock_disconnect stops charging and clears docked
    reset_state();
    dock_connect();
    dock_disconnect();
    CHECK(!is_docked());
    CHECK(!is_charging());

    // charging_step increases duty cycle when current is low
    reset_state();
    dock_connect();
    unsigned int duty_before = charging_duty_cycle;
    sim_time_ms += 100;
    charging_step(); // current is 0 (before == after), well below max
    CHECK(charging_duty_cycle > duty_before);

    // charging_step decreases duty cycle when current is too high
    // battery current = mean(before-after)*3.3/1024*10.1/0.3; need > 0.95A
    // -> delta > 0.95 * 0.3 * 1024 / (3.3 * 10.1) ~ 8.76 -> use 100
    fill_adc(600, 500, 0);
    duty_before = charging_duty_cycle;
    sim_time_ms += 100;
    charging_step();
    CHECK(charging_duty_cycle < duty_before);

    // duty cycle never drops below the initial value
    charging_duty_cycle = 1000 / 30;
    fill_adc(600, 500, 0);
    sim_time_ms += 100;
    charging_step();
    CHECK(charging_duty_cycle >= 1000 / 30);

    // detect_charged: voltage dropping means charged
    reset_state();
    dock_connect();
    CHECK(is_charging());
    sim_time_ms += 61000;
    fill_adc(500, 500, 0); // voltage dropped below the value seen a minute ago
    charging_step(); // calls detect_charged internally
    CHECK(!is_charging()); // charging stopped -> charged

    configured = false;
    docked = false;
}

TEST_MAIN()
