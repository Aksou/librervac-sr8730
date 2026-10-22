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

// Minimal unit-test framework for host tests (no external dependencies).

#pragma once

#include <stdio.h>

static int tf_failures = 0;
static int tf_checks = 0;

#define CHECK(cond) do { \
    tf_checks++; \
    if (!(cond)) { \
        tf_failures++; \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define CHECK_EQ_INT(actual, expected) do { \
    tf_checks++; \
    long tf_a = (long)(actual); \
    long tf_e = (long)(expected); \
    if (tf_a != tf_e) { \
        tf_failures++; \
        printf("FAIL %s:%d: %s == %ld, expected %ld\n", \
                __FILE__, __LINE__, #actual, tf_a, tf_e); \
    } \
} while (0)

#define CHECK_NEAR(actual, expected, eps) do { \
    tf_checks++; \
    double tf_a = (double)(actual); \
    double tf_e = (double)(expected); \
    double tf_d = tf_a - tf_e; \
    if (tf_d < 0) tf_d = -tf_d; \
    if (tf_d > (eps)) { \
        tf_failures++; \
        printf("FAIL %s:%d: %s == %f, expected %f (+/- %f)\n", \
                __FILE__, __LINE__, #actual, tf_a, tf_e, (double)(eps)); \
    } \
} while (0)

#define TEST_MAIN() \
    int main(void) { \
        run_tests(); \
        printf("%s: %d checks, %d failures\n", __FILE__, tf_checks, tf_failures); \
        return tf_failures == 0 ? 0 : 1; \
    }
