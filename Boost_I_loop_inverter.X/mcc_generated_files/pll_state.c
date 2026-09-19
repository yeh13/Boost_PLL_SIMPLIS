/*
 * File:   pll_state.c
 * Author: 3920
 *
 * Created on February 4, 2026, 4:10 PM
 */



#include "pll_state.h"

#ifndef PLL_STATE_H
#define PLL_STATE_H

#include "parameter.h"   // ? ????? pll_theta, pll_mode ???

#ifdef __cplusplus
extern "C" {
#endif

// ??????
void PLL_Init(void);
void PLL_Step(uint16_t adcVal);   // ?????? function

#ifdef __cplusplus
}
#endif
#endif
pll_mode_t pll_mode = PLL_FREE_RUN;

float pll_theta = 0.0f;
float pll_integ = 0.0f;

volatile uint8_t pll_locked = 0;
volatile int16_t i_ref_count = 0;
volatile int16_t pll_vref_count = 0;
volatile uint8_t pll_sync_phase = 1;


