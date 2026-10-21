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

// Definitions of the simulated PIC32 registers and peripheral state
// declared in the stub <xc.h>. Only linked into host test binaries.

#include "xc.h"

#define SIM_PORT_REGS_DEF(L) \
    TRIS##L##bits_t TRIS##L##bits = {{0}}; \
    LAT##L##bits_t  LAT##L##bits  = {{0}}; \
    PORT##L##bits_t PORT##L##bits = {{0}};

SIM_PORT_REGS_DEF(A)
SIM_PORT_REGS_DEF(B)
SIM_PORT_REGS_DEF(C)
SIM_PORT_REGS_DEF(D)
SIM_PORT_REGS_DEF(E)
SIM_PORT_REGS_DEF(F)
SIM_PORT_REGS_DEF(G)

AD1PCFGbits_t AD1PCFGbits = {{0}};
AD1CON1bits_t AD1CON1bits = {{0}};
DDPCONbits_t  DDPCONbits  = {{0}};
PMCONbits_t   PMCONbits   = {{0}};

unsigned int sim_oc_duty[6];
unsigned int sim_tmr4;
int sim_t2_int_enable;
unsigned int sim_adc_values[16];
int sim_int_disabled;
