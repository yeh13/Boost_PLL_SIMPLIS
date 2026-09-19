# Project Status

Last updated: 2026-09-19

## Current Repository
GitHub repository:
`yeh13/Boost_PLL_SIMPLIS`

Main local repository:
`F:\BoostPLLSIMPLIS`

## Current Firmware Findings
- PWM frequency: 40 kHz.
- Current active AN0 / AN1 callback rate is effectively 5 kHz because of `ADTR1PS=7`.
- Current active SOGI / PLL is additionally software-decimated by 8.
- Therefore the current active PLL update rate is approximately 625 Hz.
- The current active firmware does not yet implement the final desired PLL-derived 40 kHz phase delivery architecture.

## Desired Architecture
- 40 kHz raw ADC / PWM timing
- software decimation by 8
- 5 kHz SOGI / PLL update
- PLL-derived 40 kHz phase delivery
- current reference generation
- current-loop control
- PWM / power stage
- current feedback

## Reference Candidate
Current candidate files:

`Boost_I_loop_inverter.X/mcc_generated_files/reference_candidate/`

Important files:
- `pll_reference_5k40k.c`
- `pll_reference_5k40k_parity.py`
- `reference_trace_template.csv`
- `README.md`

Status:
- Candidate only.
- Not yet validated against MATLAB.
- Not yet approved for active firmware integration.

## MATLAB / Fixed60 Reference
Primary reference area:

`DAC test/FIXED60_PHASE40K_INTEGRATION/MATLAB/`

Important reference:
- Fixed60 phase delivery
- 5 kHz anchor concept
- smooth 40 kHz phase delivery

The MATLAB reference currently has stronger evidence than the C candidate.

## SIMPLIS Status
ADC-only SIMPLIS work exists, but calibration is not yet completed.

Known issue:
- Native ADC template had incomplete calibration / connection issues.
- Seven-point ADC calibration remains unfinished.

Full SIMPLIS controller integration should wait until C / MATLAB parity is credible.

## Current Priority
1. Audit the mathematical units and phase domains in `pll_reference_5k40k.c`.
2. Verify whether `pll_step_q / 8` is mathematically valid.
3. Build executable C / MATLAB parity tests.
4. Test 59 / 60 / 61 Hz.
5. Only after parity passes, move toward SIMPLIS integration.
