# 5 kHz PLL anchor + 40 kHz phase delivery reference candidate

This folder contains a stand-alone, non-invasive reference candidate built to match the Fixed60 MATLAB model and the active 5 kHz SOGI-PLL structure in adc1.c without modifying the original firmware.

## Scope

The candidate intentionally implements only:

- 40 kHz ADC input stream
- decimation counter = 8
- 5 kHz sample path
- offset removal and scaling
- SOGI
- Park transform and Vq
- PLL PI and frequency correction
- 5 kHz phase anchor
- 40 kHz PLL-derived phase delivery

It deliberately excludes:

- SSR
- power stage
- current loop
- soft-start
- redesign of the PLL algorithm

## Data flow

40 kHz ADC raw
→ decimation counter = 8
→ 5 kHz sample
→ offset removal
→ scaling
→ SOGI
→ Park transform
→ vq
→ PLL PI
→ frequency correction
→ 5 kHz phase anchor

Then for the derived phase delivery:

5 kHz PLL update
→ phase anchor refreshed
→ phase step refreshed from PLL estimate
→ 8 x 40 kHz ticks emitted
→ each tick advances phase by phase_step / 8

This phase path is derived from the PLL estimate. It is not a separate fixed 60 Hz open-loop accumulator.

## Critical parity rules

1. Use the fixed60 MATLAB model as the only algorithm reference.
2. Keep all Q-format values matched to the active implementation:
   - SOGI and PLL states in Q12 / Q30 as used by adc1.c.
   - `theta_acc_q` and phase accumulation remain in the same Q21-domain used by `PLL_THETA_DEN_Q`.
3. Preserve exact rounding behavior:
   - Q30_Round is used for SOGI updates.
   - Q15_Round is used for the Park detector output.
4. Preserve saturation behavior:
   - `pll_step_q` is clamped to `PLL_WMIN_STEP_Q .. PLL_WMAX_STEP_Q`.
   - `pll_int_acc` is clamped to `PLL_INT_LIMIT`.
   - `raw_err` is clamped to `PLL_ERR_LIMIT`.
5. Keep the 5 kHz anchor and 40 kHz phase continuous across the 8-tick boundary.

## State variables summary

| Variable | Type | Q-format | Saturation | Update rate | MATLAB name |
|---|---|---:|---|---|---|
| adc_raw | uint16_t | raw counts | none | 40 kHz | adc_raw |
| adc_centered | int32_t | raw ADC - offset | none | 5 kHz | adc_centered |
| grid_offset_q | int32_t | Q12 | none | 5 kHz | grid_offset_q |
| v_in_q | int32_t | Q12 | none | 5 kHz | v_in_q |
| sogi_va_q12 | int32_t | Q12 | none | 5 kHz | va |
| sogi_vb_q12 | int32_t | Q12 | none | 5 kHz | vb |
| vq_q | int32_t | Q12 | none | 5 kHz | vq |
| pll_err_lpf | int32_t | integer error domain | yes | 5 kHz | err_lpf / raw_err |
| pll_int_acc | int32_t | integer PI state | yes | 5 kHz | integ |
| pll_step_q | int32_t | Q12 | yes | 5 kHz | freq step / omega command |
| theta_acc_q | int64_t | Q21 | no wrap, then normalized | 5 kHz | theta or theta_acc |
| pll_idx | uint16_t | 0..359 index | wrap | 5 kHz | theta index |
| phase_anchor_q | int64_t | Q21 | wrap to 1 cycle | 5 kHz + 40 kHz phase update | phase_anchor |
| phase_step_40k_q | int64_t | Q12 scaled to 40 kHz tick | yes | 5 kHz | phase_step |
| phase_40k_q | int64_t | Q21 | wrap to 1 cycle | 40 kHz | phase_40k |
| locked | uint8_t | boolean | none | 5 kHz | locked |
| phase_ok | uint8_t | boolean | none | 5 kHz | phase_ok |
| vref_count | int16_t | raw table index | none | 5 kHz | vref |
| iref_count | int16_t | raw count output | none | 5 kHz | iref |

## Known rounding sources

If MATLAB and C differ by a count or by one Q-format step, the first place to inspect is:

- Q30_Round in the SOGI state update.
- Q15_Round in the Park Vq output.
- `theta_acc_q` extraction into `step_degrees` using `>> PLL_THETA_DEN_SHIFT`.
- `phase_step_40k_q = pll_step_q / 8` from the 5 kHz anchor to the 40 kHz phase stream.

This is not 'close enough'; it is a deterministic integer-format difference that must be tracked explicitly in the parity report.

## Offline parity test

The companion script `pll_reference_5k40k_parity.py` checks a MATLAB golden CSV against a C-generated CSV using the exact column list below:

sample_40k,sample_5k,adc_raw,adc_centered,va,vb,vq,pi,freq_correction,phase_anchor,phase_step,phase_40k,phase_ok,locked,vref,iref

It reports:

- exact match count
- max absolute error
- first mismatch sample
- mismatch fields
- C value
- MATLAB value

## Files

- `pll_reference_5k40k.c`
- `pll_reference_5k40k_parity.py`

## Notes

The candidate implements the fixed60 algorithm path and intentionally preserves the same fixed-point sequence as the MATLAB reference, while keeping the original active firmware untouched.
