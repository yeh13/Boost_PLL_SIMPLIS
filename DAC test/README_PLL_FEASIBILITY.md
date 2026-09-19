# PLL-only feasibility model

Files:

- `pll_lock_test.va`: sampled-data Verilog-A SOGI + PLL model.
- `README_PLL_FEASIBILITY.md`: active firmware audit and expected 60 Hz behavior.

## Scope

This model intentionally contains only the grid input, SOGI, phase detector,
filters, PI/NCO, lock criteria, and analog debug outputs. It does not model a
current loop, PWM, duty, MOSFET, power stage, ADC switching noise, or current
reference.

The input is interpreted as `v_in_count = V(v_grid_in) * 1000`. Therefore a
clean 1 V peak, 60 Hz source is +/-1000 signed counts.

## Sampling and SOGI

The timer samples at 40 kHz (`Ts = 25 us`) and performs one SOGI/PLL update
every 16 samples (`Fs_PLL = 2.5 kHz`, `Ts_PLL = 400 us`). The equations and
state order match the active SOGI path in `adc1.c`:

```text
va[n] = B0*(vin[n] - vin[n-2]) - A1*va[n-1] - A2*va[n-2]
vb[n] = QB0*vin[n] + QB1*vin[n-1] + QB2*vin[n-2]
       - A1*vb[n-1] - A2*vb[n-2]
```

The coefficients are the active fixed 60 Hz values:

```text
A1=-1919669515/2^30   A2=867878708/2^30   B0=102931558/2^30
QB0=7760857/2^30      QB1=15521713/2^30   QB2=7760857/2^30
```

The phase detector is deliberately the firmware direction, not a generic
SRF replacement: `theta_det = theta + 4 deg` and
`vq = va*cos(theta_det) + vb*sin(theta_det)`. The NCO output has no extra
phase compensation (`PLL_OUTPUT_PHASE_COMP_DEG = 0`).

## Scaling and filters

Magnitude uses `max(abs(va),abs(vb)) + min(abs(va),abs(vb))/2`, with a 100-count
floor. The normalized error is clamped to +/-1000 milli-pu. The PI input is
explicitly `e_pu = err_mpu / 1000.0`; thus `err_mpu * 30` is intentionally not
used. Gains are `Kp=30/(2*pi)=4.774648 Hz/pu` and
`Ki=150/(2*pi)=23.873241 Hz/s/pu`.

The fast filter is `err_lpf += (raw_err-err_lpf)/64`. The slow filter retains
the firmware Q8 behavior: target `err_lpf*256`, delta clamp +/-`500<<8`,
update by `/32`, output clamp +/-`1000<<8`, and symmetric Q8-to-Q0 rounding.

## Active firmware constants

| Function | Active value |
|---|---:|
| `PLL_DECIM_N` / `AN1_CALLBACK_HZ` | 16 / 2500 Hz |
| `PLL_PHASE_COMP_DEG` | 4 deg |
| `PLL_OUTPUT_PHASE_COMP_DEG` | 0 deg |
| `PLL_ERR_SIGN` / bias | +1 / 0 |
| `PLL_VMAG_MIN` | 100 counts |
| PI integrator contribution limit | +/-8 Hz (`PLL_PI_INT_LIMIT_MHZ=8000`) |
| target clamp | 55 to 65 Hz |
| `PLL_STEP_SLEW_ACQ_Q` | 100*360*512*4096/2500^2 = 12079.596 Q/update = 0.040 Hz/update |
| `PLL_STEP_SLEW_LOCK_Q` | 60*360*512*4096/2500^2 = 7247.757 Q/update = 0.024 Hz/update |
| WIDE lock step delta | 150 mHz = 0.150 Hz/update |
| WIDE phase ON/OFF | 180 / 360 mpu; 10 / 300 updates |
| WIDE lock/unlock | 180 / 360 mpu; 20 / 120 updates |
| WIDE frequency window | 58 to 62 Hz |

The Q conversion used above is `Hz = step_q * Fs_PLL /
(360*512*4096)`. The slew macro has units of Q/update, so its equivalent Hz/s
is 100 Hz/s during acquisition and 60 Hz/s while locked.

## Input-valid audit

The active callback accumulates centered raw AN1 samples in a 100-sample
window, computes `(raw_max-raw_min)/2`, and declares amplitude valid at the
bring-up threshold of 300 counts (`AN1_INPUT_ENABLE_AMP_COUNTS`). With
`PLL_BYPASS_ZC_VALID=1`, this amplitude result is the active `input_valid`;
the filtered zero-crossing path is diagnostic and does not gate the PLL.
This avoids declaring a normal sine invalid at its zero crossing.

## Start modes

`PHASE_INIT_ENABLE=0` is the required first test: theta=0, frequency=60 Hz,
integrator and counters cold. This tests PI/NCO acquisition without hiding it.

`PHASE_INIT_ENABLE=1` is the comparison mode. It waits 100 PLL updates for
SOGI warm-up, then seeds theta from the warmed SOGI phase before enabling PI,
matching the purpose of `PLL_PhaseInitFromSOGI`. Both modes remain selectable
through the Verilog-A parameter.

## Analog outputs

`v_input_dbg`, `v_sogi_alpha`, and `v_sogi_beta` use 1000 counts/V.
`v_pll_sin` is -1 to +1 V. `v_phase_error` is 1000 mpu/V, `v_freq_dbg` is
10 Hz/V, and both flags are 0/5 V. The model prints `lock_time`,
`phase_ok_time`, maximum frequency excursion from 60 Hz, and the mean absolute
slow error after 0.45 s at the end of a 0.5 s run.

## Expected clean 60 Hz result

After the 100-sample amplitude window, the SOGI outputs should settle to
approximately equal-amplitude quadrature waveforms. In cold start, theta and
the PLL sine should acquire from theta=0; frequency should return near 60 Hz,
relative phase should stop drifting, slow phase error should settle, and both
`v_phase_ok` and `v_locked` should eventually reach 5 V. Exact lock time is a
simulation result, not assumed here.

If cold start fails, inspect the recorded traces in this order: SOGI alpha/beta
sign and quadrature, phase-detector sign, the 4 degree compensation, PI scaling,
filter latency, slew limiter, integrator anti-windup, and finally lock criteria.

## SIMPLIS verification still required

The real Verilog-A model does not prove fixed-point equivalence. SIMPLIS should
still verify the actual ADC center/analog scaling, Verilog-A timer scheduling,
solver behavior at the sampled events, Q30 rounding effects, ADC quantization,
the generated `pll_locked` and `dbg_pll_phase_ok` analog debug paths, and the
later 59/61/58/62 Hz, step-change, and 55/65 Hz cases.