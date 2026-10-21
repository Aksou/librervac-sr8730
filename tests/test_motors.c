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

// Host unit tests for motors.c (PWM + phase logic).

#include "test_framework.h"

#include "xc.h"
#include "../motors.h"
#include "../gpio.h"
#include "../settings.h"
#include "../cordlib/cord_motors.h"

// motors.c internals exposed for testing
extern volatile int pwm_main_brush;
extern volatile int pwm_left_wheel;
extern volatile int pwm_right_wheel;

#define TIMER_TICKS (FOSC / (20000) / 1 * FPB_DIV)
#define DEADBAND 0.65

static void reset_sim(void) {
    unsigned int i;
    for (i = 0; i < 6; i++)
        sim_oc_duty[i] = 0xFFFFFFFF;
    TRISDbits = (TRISDbits_t){{0}};
    LATDbits = (LATDbits_t){{0}};
    LATCbits = (LATCbits_t){{0}};
    LATAbits = (LATAbits_t){{0}};
    LATGbits = (LATGbits_t){{0}};
}

static void run_tests(void) {
    // WHEEL_LEFT forward: phase pin set to active level
    reset_sim();
    set_motor_throttle(WHEEL_LEFT, 1.0f);
    CHECK_EQ_INT(LATCbits.LATC1, 0); // PIN_WHEEL_LEFT_PHASE active LOW, on = forward
    CHECK_EQ_INT(pwm_left_wheel > 0, 1);
    // full throttle: deadband + (1-deadband)*1
    CHECK_EQ_INT(pwm_left_wheel, (int)((DEADBAND + (1.0 - DEADBAND) * 1.0) * TIMER_TICKS));

    // WHEEL_LEFT reverse: phase pin at inactive level
    reset_sim();
    set_motor_throttle(WHEEL_LEFT, -0.5f);
    CHECK_EQ_INT(LATCbits.LATC1, 1);
    CHECK(pwm_left_wheel > 0);
    CHECK(pwm_left_wheel < (int)((DEADBAND + (1.0 - DEADBAND)) * TIMER_TICKS));

    // WHEEL_LEFT stop: zero throttle
    reset_sim();
    set_motor_throttle(WHEEL_LEFT, 0.0f);
    CHECK_EQ_INT(pwm_left_wheel, 0);

    // WHEEL_RIGHT forward (active HIGH phase)
    reset_sim();
    set_motor_throttle(WHEEL_RIGHT, 1.0f);
    CHECK_EQ_INT(LATGbits.LATG7, 1);
    CHECK_EQ_INT(pwm_right_wheel > 0, 1);

    // WHEEL_RIGHT reverse
    reset_sim();
    set_motor_throttle(WHEEL_RIGHT, -1.0f);
    CHECK_EQ_INT(LATGbits.LATG7, 0);

    // BRUSH_MAIN forward
    reset_sim();
    set_motor_throttle(BRUSH_MAIN, 1.0f);
    CHECK_EQ_INT(LATCbits.LATC2, 0); // active LOW
    CHECK(pwm_main_brush > 0);

    // BRUSH_MAIN reverse
    reset_sim();
    set_motor_throttle(BRUSH_MAIN, -1.0f);
    CHECK_EQ_INT(LATCbits.LATC2, 1);

    // BRUSH_SIDES on/off
    reset_sim();
    set_motor_throttle(BRUSH_SIDES, 1.0f);
    CHECK_EQ_INT(LATAbits.LATA4, 1);
    set_motor_throttle(BRUSH_SIDES, 0.0f);
    CHECK_EQ_INT(LATAbits.LATA4, 0);

    // VACUUM on/off
    reset_sim();
    set_motor_throttle(VACUUM, 1.0f);
    CHECK_EQ_INT(LATDbits.LATD5, 1);
    set_motor_throttle(VACUUM, 0.0f);
    CHECK_EQ_INT(LATDbits.LATD5, 0);

    // small throttle still above deadband: (0.65 + 0.35*0.1) = 0.685 of full scale
    reset_sim();
    set_motor_throttle(WHEEL_LEFT, 0.1f);
    CHECK(pwm_left_wheel > 0);
    CHECK_EQ_INT(pwm_left_wheel, (int)((0.65 + 0.35 * 0.1) * TIMER_TICKS));
}

TEST_MAIN()
