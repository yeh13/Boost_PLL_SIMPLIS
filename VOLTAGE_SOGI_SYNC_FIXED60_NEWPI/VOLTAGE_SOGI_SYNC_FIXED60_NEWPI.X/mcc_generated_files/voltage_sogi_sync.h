#ifndef VOLTAGE_SOGI_SYNC_H
#define VOLTAGE_SOGI_SYNC_H
#include <stdint.h>
typedef struct { uint16_t valid, polarity, generation; } SogiSyncSnapshot;
extern volatile uint8_t sogi_sync_valid, sogi_grid_polarity;
extern volatile uint8_t pll_locked, dbg_pll_phase_ok, dbg_an1_input_valid;
extern volatile uint16_t sogi_cross_generation;
extern volatile uint32_t sogi_cross_count, pll_update_count;
extern volatile uint32_t accepted_cross_count, valid_enter_count, valid_exit_count;
/* 32-bit variables are diagnostic only: read with CPU halted, never control ISR input. */
void VoltageSogiSync_Update(uint16_t adc);
SogiSyncSnapshot VoltageSogiSync_Read(void);
/* Original RC15 marker, disabled until the board pin is confirmed safe.
 * No pin configuration or GPIO use occurs in the delivered build. */
#ifndef VOLTAGE_SOGI_RC15_MARKER
#define VOLTAGE_SOGI_RC15_MARKER 0
#endif
#if VOLTAGE_SOGI_RC15_MARKER
#define SOGI_TIMING_HIGH() __asm__ volatile ("bset _LATC, #15" ::: "memory")
#define SOGI_TIMING_LOW() __asm__ volatile ("bclr _LATC, #15" ::: "memory")
#else
#define SOGI_TIMING_HIGH() ((void)0)
#define SOGI_TIMING_LOW() ((void)0)
#endif
#endif
