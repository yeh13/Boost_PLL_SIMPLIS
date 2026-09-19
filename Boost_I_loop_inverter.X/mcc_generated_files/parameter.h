/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef PARAMETER_H
#define PARAMETER_H

#include <xc.h>
#include <stdint.h> 

#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    MODE_BUCK = 0,
    MODE_BOOST = 1
} power_mode_t;

extern volatile power_mode_t power_mode;
extern volatile int32_t Boost_PWM;
extern volatile int32_t Buck_PWM;
/* =========================
 * PLL / Synchronization
 * ========================= */

typedef enum {
    PLL_FREE_RUN = 0,
    PLL_LOCKED
} pll_mode_t;

extern pll_mode_t pll_mode;
extern float pll_theta;    // ? ?? volatile

extern float pll_integ;    // ? ????????????? pll_integ?

/* ?????? / ? PLL ?? */
extern volatile uint8_t pll_locked;     // ? extern only
extern volatile uint8_t dbg_pll_phase_ok;

/* ? AN0 ??????? i_ref?counts? */
extern volatile int16_t i_ref_count;    // ? extern only
extern volatile int16_t pll_vref_count;
extern volatile uint8_t pll_sync_phase;

/* =========================
 * Modulation / Phase State
 * ========================= */
extern unsigned short phase;
extern unsigned short i;
extern unsigned short k;

/* =========================
 * DC / Reference Tables
 * ========================= */
extern const int16_t Vdc;
extern const int16_t VrefTable[334];

/* =========================
 * PWM / Control Outputs
 * ========================= */

extern volatile int16_t finallyDuty_buck;
extern volatile int16_t finallyDuty_boost;

/* =========================
 * Boost Dynamic Compensation
 * ========================= */
extern int32_t boost_dynamic_coefficient;
extern int32_t B_Coefficient_BOOST_finally[3];

/* ====== Calibration (macro OK ? header ???) ====== */
#define COUNTS_PER_AMP      460
#define I_RMS_TARGET_A      0.5f
#define I_PEAK_TARGET_COUNTS ((int16_t)(I_RMS_TARGET_A * 1.41421356f * COUNTS_PER_AMP))
#define IREF_PHASE_COMP     0.0f

/*
   Keep PLL measurement/debug separated from power control while tuning.
   0: PWM/current loop use the original table phase/reference.
   1: PWM/current loop use PLL-generated phase/reference after pll_locked.
*/
#define PLL_CONTROL_ENABLE  1

/*
   Grid-tie safety gate.
   1: when PLL is not locked, force PWM/current reference off instead of
      falling back to the open-loop 60Hz table.
*/
#define GRID_TIE_REQUIRE_PLL_LOCK  0

/*
   Open-loop demo output frequency.
   Change this to 60UL or 65UL later when checking those grid frequencies.
*/
#define OPEN_LOOP_OUTPUT_FREQ_HZ  60UL

/*
   PG7 debug mode for measuring the real AN1 ISR frequency without MPLAB Watch.
   PWM1 is used as a 40kHz time base. PG7 duty becomes:
       duty = AN1_ISR_Hz / 5000Hz
*/
#define AN1_RATE_DUTY_DEBUG_ENABLE  0

extern volatile uint16_t dbg_an1_rate_count;
extern volatile uint16_t dbg_an1_rate_hz;
extern volatile uint16_t dbg_an1_rate_duty_x10;
#ifdef __cplusplus
}
#endif

#endif /* PARAMETER_H */




