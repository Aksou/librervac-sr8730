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

// Host-side stub of the XC32 headers (<xc.h>, <plib.h>, <p32xxxx.h>).
// Used ONLY for compiling the firmware on the host (unit tests, cppcheck).
// It simulates the PIC32MX360F256L registers as memory unions so tests can
// drive pin levels and observe latch outputs.

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ---------------------------------------------------------------------------
// Simulated registers
//
// Each register is a union of a bitfield struct (so firmware code keeps its
// natural PORTBbits.RB14 syntax) and a raw byte array (so tests can poke pins
// generically). Bitfields are packed LSB-first (x86-64 gcc).
// ---------------------------------------------------------------------------

#define SIM_PIN_BITS(prefix, L) \
    unsigned prefix##L##0:1;  unsigned prefix##L##1:1;  unsigned prefix##L##2:1;  unsigned prefix##L##3:1; \
    unsigned prefix##L##4:1;  unsigned prefix##L##5:1;  unsigned prefix##L##6:1;  unsigned prefix##L##7:1; \
    unsigned prefix##L##8:1;  unsigned prefix##L##9:1;  unsigned prefix##L##10:1; unsigned prefix##L##11:1; \
    unsigned prefix##L##12:1; unsigned prefix##L##13:1; unsigned prefix##L##14:1; unsigned prefix##L##15:1;

#define SIM_PORT_REGS(L) \
    typedef union { struct { SIM_PIN_BITS(TRIS, L) }; unsigned char raw[2]; } TRIS##L##bits_t; \
    extern TRIS##L##bits_t TRIS##L##bits; \
    typedef union { struct { SIM_PIN_BITS(LAT, L) };  unsigned char raw[2]; } LAT##L##bits_t; \
    extern LAT##L##bits_t LAT##L##bits; \
    typedef union { struct { SIM_PIN_BITS(R, L) };    unsigned char raw[2]; } PORT##L##bits_t; \
    extern PORT##L##bits_t PORT##L##bits;

SIM_PORT_REGS(A)
SIM_PORT_REGS(B)
SIM_PORT_REGS(C)
SIM_PORT_REGS(D)
SIM_PORT_REGS(E)
SIM_PORT_REGS(F)
SIM_PORT_REGS(G)

typedef union { struct { \
    unsigned PCFG0:1;  unsigned PCFG1:1;  unsigned PCFG2:1;  unsigned PCFG3:1; \
    unsigned PCFG4:1;  unsigned PCFG5:1;  unsigned PCFG6:1;  unsigned PCFG7:1; \
    unsigned PCFG8:1;  unsigned PCFG9:1;  unsigned PCFG10:1; unsigned PCFG11:1; \
    unsigned PCFG12:1; unsigned PCFG13:1; unsigned PCFG14:1; unsigned PCFG15:1; \
}; unsigned char raw[2]; } AD1PCFGbits_t;
extern AD1PCFGbits_t AD1PCFGbits;

typedef union { struct { unsigned ON:1; }; unsigned char raw[1]; } AD1CON1bits_t;
extern AD1CON1bits_t AD1CON1bits;

typedef union { struct { unsigned JTAGEN:1; }; unsigned char raw[1]; } DDPCONbits_t;
extern DDPCONbits_t DDPCONbits;

typedef union { struct { unsigned ON:1; }; unsigned char raw[1]; } PMCONbits_t;
extern PMCONbits_t PMCONbits;

// ---------------------------------------------------------------------------
// Simulated peripheral state (defined by the test runner)
// ---------------------------------------------------------------------------

extern unsigned int sim_oc_duty[6];      ///< last duty cycle set on OC1..OC5
extern unsigned int sim_tmr4;            ///< free-running Timer4 value
extern int sim_t2_int_enable;            ///< Timer2 interrupt enable state
extern unsigned int sim_adc_values[16];  ///< values returned by ReadADC10()
extern int sim_int_disabled;             ///< global interrupt disable state

// ---------------------------------------------------------------------------
// Peripheral library constants (values are irrelevant on the host)
// ---------------------------------------------------------------------------

#define T1_ON 0
#define T1_PS_1_1 0
#define T1_SOURCE_INT 0
#define T2_ON 0
#define T2_PS_1_8 0
#define T2_SOURCE_INT 0
#define T3_ON 0
#define T3_PS_1_1 0
#define T3_SOURCE_INT 0
#define T4_ON 0
#define T4_PS_1_8 0
#define T4_SOURCE_INT 0
#define T5_ON 0
#define T5_PS_1_8 0
#define T5_SOURCE_INT 0

#define OC_ON 0
#define OC_TIMER2_SRC 0
#define OC_TIMER3_SRC 0
#define OC_PWM_FAULT_PIN_DISABLE 0

#define CN_ON 0
#define CN_IDLE_CON 0
#define CN1_ENABLE 0
#define CN1_PULLUP_ENABLE 0

#define CT_INT_ON 0
#define CT_INT_PRIOR_1 0
#define CT_INT_SUB_PRIOR_3 0

#define ADC_MODULE_ON 0
#define ADC_FORMAT_INTG32 0
#define ADC_CLK_AUTO 0
#define ADC_AUTO_SAMPLING_ON 0
#define ADC_VREF_AVDD_AVSS 0
#define ADC_OFFSET_CAL_DISABLE 0
#define ADC_SCAN_ON 0
#define ADC_SAMPLES_PER_INT_3 0
#define ADC_ALT_BUF_ON 0
#define ADC_ALT_INPUT_OFF 0
#define ADC_CONV_CLK_INTERNAL_RC 0
#define ADC_SAMPLE_TIME_31 0
#define ADC_CH0_NEG_SAMPLEA_NVREF 0
#define ADC_CH0_POS_SAMPLEA_AN2 0
#define ADC_CH0_POS_SAMPLEA_AN3 0
#define ADC_CH0_POS_SAMPLEA_AN4 0
#define ADC_INT_ON 0
#define ADC_INT_PRI_3 0
#define ADC_INT_SUB_PRI_2 0

#define ENABLE_AN2_ANA 0
#define ENABLE_AN3_ANA 0
#define ENABLE_AN4_ANA 0

#define SKIP_SCAN_AN0 0
#define SKIP_SCAN_AN1 0
#define SKIP_SCAN_AN2 0
#define SKIP_SCAN_AN3 0
#define SKIP_SCAN_AN4 0
#define SKIP_SCAN_AN5 0
#define SKIP_SCAN_AN6 0
#define SKIP_SCAN_AN7 0
#define SKIP_SCAN_AN8 0
#define SKIP_SCAN_AN9 0
#define SKIP_SCAN_AN10 0
#define SKIP_SCAN_AN11 0
#define SKIP_SCAN_AN12 0
#define SKIP_SCAN_AN13 0
#define SKIP_SCAN_AN14 0
#define SKIP_SCAN_AN15 0

// Interrupt vectors and priorities (only used as __ISR arguments)
#define _TIMER_1_VECTOR 0
#define _TIMER_2_VECTOR 0
#define _TIMER_3_VECTOR 0
#define _TIMER_4_VECTOR 0
#define _TIMER_5_VECTOR 0
#define _CORE_TIMER_VECTOR 0
#define _CHANGE_NOTICE_VECTOR 0
#define _ADC_VECTOR 0

#define IPL1SOFT 0
#define IPL3SOFT 0
#define IPL4SOFT 0
#define IPL6SOFT 0
#define IPL6AUTO 0
#define IPL7SOFT 0

#define __ISR(vector, priority)

// ---------------------------------------------------------------------------
// Peripheral library functions (stubbed)
// ---------------------------------------------------------------------------

#define OpenTimer1(config, period)   ((void)0)
#define OpenTimer2(config, period)   ((void)0)
#define OpenTimer3(config, period)   ((void)0)
#define OpenTimer4(config, period)   ((void)0)
#define OpenTimer5(config, period)   ((void)0)
#define CloseTimer1()                ((void)0)
#define CloseTimer2()                ((void)0)
#define CloseTimer3()                ((void)0)
#define CloseTimer4()                ((void)0)
#define CloseTimer5()                ((void)0)

#define OpenOC1(config, duty, period) ((void)0)
#define OpenOC2(config, duty, period) ((void)0)
#define OpenOC3(config, duty, period) ((void)0)
#define OpenOC4(config, duty, period) ((void)0)
#define OpenOC5(config, duty, period) ((void)0)

#define SetDCOC1PWM(duty) (sim_oc_duty[1] = (unsigned int)(duty))
#define SetDCOC2PWM(duty) (sim_oc_duty[2] = (unsigned int)(duty))
#define SetDCOC3PWM(duty) (sim_oc_duty[3] = (unsigned int)(duty))
#define SetDCOC4PWM(duty) (sim_oc_duty[4] = (unsigned int)(duty))
#define SetDCOC5PWM(duty) (sim_oc_duty[5] = (unsigned int)(duty))

#define OpenADC10(a, b, c, d, e)     ((void)0)
#define CloseADC10()                 ((void)0)
#define SetChanADC10(channels)       ((void)0)
#define ConfigIntADC10(config)       ((void)0)
#define EnableADC10()                ((void)0)
#define ReadADC10(index)             (sim_adc_values[(index) & 15])
#define ReadActiveBufferADC10()      (0)

#define ReadTimer4()                 (sim_tmr4)

#define OpenCoreTimer(period)        ((void)0)
#define UpdateCoreTimer(period)      ((void)0)

#define SYSTEMConfigPerformance(clock) ((void)0)

static inline int sim_int_disable(void) { sim_int_disabled = 1; return 0; }
static inline int sim_int_enable(void) { sim_int_disabled = 0; return 0; }
static inline int sim_int_restore(int status) { sim_int_disabled = status; return 0; }
#define INTDisableInterrupts()       sim_int_disable()
#define INTEnableInterrupts()        sim_int_enable()
#define INTRestoreInterrupts(status) sim_int_restore(status)
#define INTEnableSystemMultiVectoredInt() ((void)0)

#define mCNOpen(config, pins, pullups) ((void)0)
#define mCNSetIntPriority(priority)  ((void)0)
#define mCNClearIntFlag()            ((void)0)
#define mCNIntEnable(enable)         ((void)0)
#define mPORTCRead()                 ((void)0)

#define mT1SetIntPriority(p)         ((void)0)
#define mT1SetIntSubPriority(p)      ((void)0)
#define mT1ClearIntFlag()            ((void)0)
#define mT1IntEnable(enable)         ((void)0)

#define mT2SetIntPriority(p)         ((void)0)
#define mT2SetIntSubPriority(p)      ((void)0)
#define mT2ClearIntFlag()            ((void)0)
#define mT2IntEnable(enable)         (sim_t2_int_enable = (enable))
#define mT2GetIntEnable()            (sim_t2_int_enable)

#define mT3SetIntPriority(p)         ((void)0)
#define mT3SetIntSubPriority(p)      ((void)0)
#define mT3ClearIntFlag()            ((void)0)
#define mT3IntEnable(enable)         ((void)0)

#define mT4SetIntPriority(p)         ((void)0)
#define mT4ClearIntFlag()            ((void)0)
#define mT4IntEnable(enable)         ((void)0)

#define mT5SetIntPriority(p)         ((void)0)
#define mT5ClearIntFlag()            ((void)0)
#define mT5IntEnable(enable)         ((void)0)

#define mAD1ClearIntFlag()           ((void)0)
#define mCTClearIntFlag()            ((void)0)
#define mConfigIntCoreTimer(config)  ((void)0)
