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

#include "cordlib/cord_bumper.h"
#include "gpio.h"
#include "cord_hw.h"
#include "cord_protocol.h"
#include <stdbool.h>

// There is a change notification on the pin for the left switch, but not
// for the right switch. Go figure.
// This means that we can't have a proper handling of bumper presses based on
// interrupts. Not a big deal, just a slight inconvenience.
// What were they thinking? Was it so hard to find a free pin for that?
// Dear Samsung, let me help you. Pin 74 is Not Connected (grounded) and
// has CN0. You are welcome.

#define DEBOUNCE_TIME_MS 100 // paranoidal debouncing

bool pressed_left;
uint_fast32_t moment_left;
bool pressed_right;
uint_fast32_t moment_right;

void bumper_step() {
    bool pin_left = READPIN(PIN_BUMPER_SWITCH_LEFT);
    if (pin_left != pressed_left) {
        // the pin changed since the last accepted reading; it has to stay
        // in the new state for the whole debounce window before we accept it
        if (hw_get_time_ms() > moment_left + DEBOUNCE_TIME_MS) {
            pressed_left = pin_left;
            moment_left = hw_get_time_ms();
            if (pressed_left)
                cord_event_bumper(-45, 45);
        }
    } else {
        // stable level: (re)start the debounce window so that a change is
        // only accepted after DEBOUNCE_TIME_MS in the new state
        moment_left = hw_get_time_ms();
    }
    bool pin_right = READPIN(PIN_BUMPER_SWITCH_RIGHT);
    if (pin_right != pressed_right) {
        if (hw_get_time_ms() > moment_right + DEBOUNCE_TIME_MS) {
            pressed_right = pin_right;
            moment_right = hw_get_time_ms();
            if (pressed_right)
                cord_event_bumper(+45, 45);
        }
    } else {
        moment_right = hw_get_time_ms();
    }
}
