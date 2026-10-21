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

// Host unit tests for battery.c (voltage / current / temperature math).

#include "test_framework.h"

#include <string.h>
#include "xc.h"
#include "../battery.h"
#include "../charging.h"

// battery.c reads these buffers (volatile, filled by the ADC ISR)
extern volatile int_fast32_t adc_thermistor[75];
extern volatile int_fast32_t adc_before_shunt[75];
extern volatile int_fast32_t adc_after_shunt[75];

// charging.c state used by battery.c
extern bool configured;

static void fill_adc(int_fast32_t before, int_fast32_t after, int_fast32_t therm) {
    size_t i;
    for (i = 0; i < 75; i++) {
        adc_before_shunt[i] = before;
        adc_after_shunt[i] = after;
        adc_thermistor[i] = therm;
    }
}

static void run_tests(void) {
    // Voltage: mean adc * 3.3 / 1024 * 10.1 (divider Rd)
    fill_adc(512, 512, 0);
    CHECK_NEAR(get_battery_voltage_before_shunt(), 512.0 * 3.3 / 1024 * 10.1, 0.001);
    CHECK_NEAR(get_battery_voltage_after_shunt(), 512.0 * 3.3 / 1024 * 10.1, 0.001);

    fill_adc(0, 0, 0);
    CHECK_NEAR(get_battery_voltage_before_shunt(), 0.0, 0.001);

    fill_adc(1024, 1024, 0);
    CHECK_NEAR(get_battery_voltage_before_shunt(), 3.3 * 10.1, 0.01);

    // Current is only measurable while charging
    fill_adc(600, 500, 0);
    configured = false;
    CHECK_NEAR(get_battery_current(), 0.0, 0.0001);

    // While charging: mean(before-after) * 3.3 / 1024 * 10.1 / 0.3
    configured = true;
    float expected = 100.0 * 3.3 / 1024 * 10.1 / 0.3; // + current_offset (0 initially)
    CHECK_NEAR(get_battery_current(), expected, 0.01);

    // Temperature: Rt = R1 / (VCC / adc_voltage - 1 - R1 / R2), then cubic fit
    // With thermistor adc = 512: voltage = 1.65, Rt = 1.5 / (3.3/1.65 - 1 - 0.15) = 1.5/0.85
    fill_adc(0, 0, 512);
    float voltage = 512.0 * 3.3 / 1024;
    float Rt = 1.5f / (3.3f / voltage - 1 - 1.5f / 10);
    float expected_temp = -0.966249433923487
        + 42.4294209665111 / (0.129333893760934 * Rt + 0.307073187406113);
    CHECK_NEAR(get_battery_temperature(), expected_temp, 0.001);

    configured = false;
}

TEST_MAIN()
