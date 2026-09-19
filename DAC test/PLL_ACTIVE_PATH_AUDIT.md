# PLL Active Path Audit

Audit source: `d:\葉同隆\Boost_I_loop_inverter.X\mcc_generated_files\adc1.c`.
The active AN1 interrupt registration is `ADC1_channel_AN1_CallBack` at line 6313. `ADC1_channel_AN1_CallBack_OldUnused` and the commented callback beginning at line 11637 are not active paths.

## 1. Active build configuration

| Macro | Active value | Active branch | Inactive/alternative branch |
|---|---:|---|---|
| `PLL_CONTROL_ENABLE` | 0 | Does not independently enable PLL output; PLL math still runs in active AN1 callback | 1 enables the related output-use condition |
| `PLL_DECIM_N` | 16 | Firmware timing ratio: 40 kHz raw rate / 16 = 2.5 kHz PLL rate | Other values are compile-time rejected for this test |
| `AN1_RAW_ISR_HZ` | 40000 Hz | Raw AN1 timing reference | Other values must divide by 16 |
| `AN1_CALLBACK_HZ` / `FS_HZ` | 2500 Hz | Active PLL/SOGI update rate | Other values are rejected by checks |
| `PLL_DISABLE_FREQ_TRACKING_TEST` | 0 | Frequency PI tracking enabled | 1 disables frequency tracking |
| `PLL_FIXED_NCO_TEST` | 0 | Normal adaptive NCO | 1 holds nominal NCO |
| `PLL_ERR_SIGN_INVERT_TEST` | 0 | `PLL_ERR_SIGN = +1` | 1 gives `-1` |
| `PLL_DISABLE_INTEGRATOR_TEST` | 0 | Integrator enabled | 1 forces integrator to zero |
| `PLL_DISABLE_KP_TEST` | 0 | Normal PI path | 1 disables Kp and holds nominal step |
| `PLL_USE_MATLAB_PI_GAIN` | 1 | Active PI uses 30 rad/s and 150 rad/s^2 equivalent gains | 0 uses legacy shift-based PI branch |
| `PLL_USE_DYNAMIC_OFFSET` | 0 | `grid_offset = GRID_ADC_CENTER` every active callback | 1 runs offset tracker |
| `PLL_BYPASS_ZC_VALID` | 1 | Final `input_valid = an1_amp_valid`; ZC frequency validity does not gate PLL | 0 requires amplitude valid AND ZC frequency valid |
| `PLL_INPUT_FREQ_SANITY_ENABLE` | 1 | ZC sanity detector still runs for debug/validity logic, but bypass above wins | 0 skips frequency sanity requirement |
| `PLL_LIGHT_LOAD_VERIFY` | 1 | ZC debug block is reduced-rate/debug-only | 0 runs it at full callback rate |
| `PLL_SYNC_TO_AN1_ZC` | 0 | No ZC phase synchronization | 1 enables that path |
| `PLL_DEMO_LOCK_ON_ZC` | 0 | Normal lock criteria | 1 demo lock-on-ZC branch |
| `PLL_ZC_ONLY_DEBUG` | 1 | ZC remains diagnostic | 0 disables this debug selection |
| `GRID_READY_PROFILE` | `GRID_READY_PROFILE_WIDE` = 3 | WIDE thresholds and debounce | BRINGUP or FORMAL alternatives |
| `SOGI_FIXED_FREQ_TEST` | 0 | Normal active SOGI coefficient selection, but `PLL_BYPASS_ZC_VALID=1` forces fixed 60 Hz coefficients | 1 fixed-frequency test branch |
| `SOGI_COEFF_FIXED_60HZ_DEBUG_TEST` | 1 | Fixed 60 Hz coefficient debug configuration is selected | 0 allows other coefficient source |
| `PLL_PHASE_INIT_ENABLE` | 1 | 100-update warmup, then `PLL_PhaseInitFromSOGI` scan | 0 skips phase-init path |
| `PLL_FAST_PHASE_INIT_ENABLE` | 0 | Full 360-candidate SOGI phase scan | 1 deterministic fast start |
| `PLL_PHASE_COMP_DEG` | 4 deg | Used only by phase detector and phase-init scan | Other value changes detector phase |
| `PLL_OUTPUT_PHASE_COMP_DEG` | 0 deg | No additional output phase compensation | Nonzero shifts output lookup |
| `PLL_ERR_SCALE` | 1000 | milli-pu normalization | N/A |
| `PLL_ERR_BIAS` | 0 | No error bias | N/A |
| `PLL_ERR_LIMIT` | 1000 | Raw normalized error clamp +/-1000 | N/A |
| `PLL_VMAG_MIN` | 100 counts | Normalization floor | N/A |
| `PLL_LPF_SHIFT` | 6 | Fast error update by `/64` | N/A |
| `PLL_SOGI_TUNE_LPF_SHIFT` | 5 | Used only when ZC bypass is disabled; active bypass keeps SOGI fixed | N/A |
| `PLL_PI_FREEZE_STEP_DEBUG_TEST` | 0 | Normal step owner | 1 freezes step |
| `PLL_NCO_FIXED_60HZ_DEBUG_TEST` | 0 | Normal NCO | 1 fixed 60 Hz debug NCO |
| `PLL_TEST_FORCE_LOCK` | 0 | No forced lock | 1 force-locks test mode |
| `PLL_THREE_POINT_TEST_ENABLE` | 0 | Three-point test inactive | 1 enables 55/60/65 test mode |

The active branch is also controlled by `PLL_USE_MATLAB_PI_GAIN=1`, `GRID_READY_PROFILE=3`, `PLL_BYPASS_ZC_VALID=1`, and the normal (non-test) switches above. Several similarly named macros in the file belong to old/diagnostic branches and do not change the active callback under these values.

## 2. Sampling and timing

| Quantity | Firmware value | Period / note |
|---|---:|---|
| Raw AN1 timing reference | 40,000 Hz | `Ts_ADC = 25 us` |
| AN1 callback / PLL callback | 2,500 Hz | `Ts_SOGI = Ts_PLL = 400 us` |
| Decimation | 16 | 40 kHz / 16 |
| SOGI update | 2,500 Hz | Active SOGI state update |
| PLL update | 2,500 Hz | Active PI, NCO, lock and phase state |
| ZC debug path | reduced by `PLL_ZC_DEBUG_DECIM_N=4` under light-load mode | It does not feed `pll_step_q` and does not gate final input-valid because bypass is enabled |
| Amplitude window | 100 callback samples | 40 ms if callback is 2.5 kHz; this is the active callback's local window rate |

The code registers the AN1 channel callback directly; the generated ADC/trigger configuration supplies the callback cadence. The firmware constants explicitly define raw timing as 40 kHz and callback/PLL timing as 2.5 kHz. The old callback contains an additional explicit decimator and is not the active one.

## 3. ADC and input scaling

- `GRID_ADC_CENTER = 1986` counts.
- ADC is 12-bit, unsigned raw range `0..4095` (`uint16_t adcVal` and 12-bit device ADC).
- Active centered input with `PLL_USE_DYNAMIC_OFFSET=0`:

```text
v_in_count = adcVal - GRID_ADC_CENTER
v_in_count range = -1986 .. +2109 counts
```

- The feasibility test defines its electrical input independently as `V(v_grid_in) * 1000`, so `+1 V = +1000 counts`, `-1 V = -1000 counts`. That is a test-model scaling, not a firmware ADC voltage calibration.

## 4. SOGI constants and equations

Active fixed 60 Hz coefficients are selected by `PLL_BYPASS_ZC_VALID=1`:

| Symbol | Integer Q30 value | Real value |
|---|---:|---:|
| `A1` | -1919669515 | -1.7871887779 |
| `A2` | 867878708 | 0.8083562928 |
| `B0` | 102931558 | 0.0958716398 |
| `QB0` | 7760857 | 0.0072258090 |
| `QB1` | 15521713 | 0.0144516180 |
| `QB2` | 7760857 | 0.0072258090 |

`SOGI_Q=30`, `SOGI_STATE_Q=12`. For each active SOGI update:

```text
v_in_count = adcVal - grid_offset
v_in_q = v_in_count << SOGI_STATE_Q

acc_va = B0*v_in_q - B0*sogi_vin_n2
          - A1*sogi_va_n1 - A2*sogi_va_n2

acc_vb = QB0*v_in_q + QB1*sogi_vin_n1 + QB2*sogi_vin_n2
          - A1*sogi_vb_n1 - A2*sogi_vb_n2

va_new = Q30_Round(acc_va)
vb_new = Q30_Round(acc_vb)
```

The active integer implementation uses Q30 coefficients, Q12 states, and signed 64-bit accumulators. `Q30_Round(x)` is symmetric:

```text
x >= 0:  (x + 2^29) >> 30
x <  0: -(((-x) + 2^29) >> 30)
```

State update order is exactly:

```text
sogi_vin_n2 = sogi_vin_n1; sogi_vin_n1 = v_in_q
sogi_va_n2  = sogi_va_n1;  sogi_va_n1  = va_new
sogi_vb_n2  = sogi_vb_n1;  sogi_vb_n1  = vb_new
```

SOGI debug count views use the Q12-to-Q0 conversion (`state >> 12`). The feasibility model uses real states and omits Q30 rounding, so its SOGI output is an approximation. The active normalization uses Q12 SOGI counts: `100 counts` means `100 << 12` internal state units.

## 5. Phase detector

- `PLL_PHASE_COMP_DEG = 4`.
- `PLL_OUTPUT_PHASE_COMP_DEG = 0`.
- Active lookup uses `PLL_SinInterp(index, theta_acc_q)` over the signed Q15 sine table (`-32768..+32767`), with cosine obtained as sine at index + 90 degrees.

```text
theta_det_idx = pll_idx + 4 degrees
sin_theta = PLL_SinInterp(theta_det_idx, theta_acc_q)
cos_theta = PLL_SinInterp(theta_det_idx + 90 degrees, theta_acc_q)

vq_q = Q15_Round(sogi_va*cos_theta + sogi_vb*sin_theta)
dbg_pll_vq = vq_q >> SOGI_STATE_Q
```

`sin_theta` and `cos_theta` are Q15; `sogi_va/vb` are Q12-scaled signed states; `vq_q` is Q12-scaled. `Q15_Round` rounds the signed product sum by the Q15 half-LSB before shifting (the active code calls the existing signed Q15 rounding helper). `PLL_ERR_SIGN_INVERT_TEST=0` makes `PLL_ERR_SIGN=+1`.

```text
raw_err = PLL_ERR_SIGN * vq_q * PLL_ERR_SCALE / vmag_q - PLL_ERR_BIAS
        = +1 * vq_q * 1000 / vmag_q - 0
raw_err = clamp(raw_err, -1000, +1000)   [milli-pu]
```

## 6. Magnitude normalization

```text
abs_va = (sogi_va >= 0) ? sogi_va : -sogi_va
abs_vb = (sogi_vb >= 0) ? sogi_vb : -sogi_vb

if (abs_va >= abs_vb) {
    mag_max = abs_va;
    mag_min = abs_vb;
} else {
    mag_max = abs_vb;
    mag_min = abs_va;
}

vmag_q = mag_max + (mag_min >> 1)
vmag_min_q = PLL_VMAG_MIN << SOGI_STATE_Q = 100 << 12
if (vmag_q < vmag_min_q) vmag_q = vmag_min_q
```

`vmag_q` is in Q12 SOGI-state units. The equivalent displayed SOGI count is `vmag_q >> 12`; no square root is used.

## 7. Fast error LPF

`PLL_LPF_SHIFT=6`:

```text
err_lpf[n] = err_lpf[n-1] + ((raw_err[n] - err_lpf[n-1]) >> 6)
```

It is an integer right-shift implementation, nominal alpha `1/64 = 0.015625` at 2.5 kHz. The one-pole discrete time constant is approximately `Ts/alpha = 25.6 ms`; the corresponding approximate -3 dB corner is about `6.2 Hz` (`fc = -ln(1-alpha)*Fs/(2*pi)`).

## 8. Slow error LPF

Active state is `pll_err_slow_lpf_q8`, with Q8 scaling (`1 Q0 error count = 256 Q8 units`). Constants:

```text
PLL_ERR_LPF_DIFF_LIMIT_Q8 = 500 << 8 = 128000 Q8 units
PLL_ERR_SLOW_LPF_LIMIT_Q8 = 1000 << 8 = 256000 Q8 units
update divisor = 32 (shift 5)
```

Per active update:

```text
slow_target_q8 = pll_err_lpf << 8
slow_diff_q8   = slow_target_q8 - pll_err_slow_lpf_q8

if (slow_diff_q8 > +128000) slow_diff_q8 = +128000
if (slow_diff_q8 < -128000) slow_diff_q8 = -128000

slow_next_q8 = pll_err_slow_lpf_q8 + (slow_diff_q8 >> 5)

if (slow_next_q8 > +256000) slow_next_q8 = +256000
if (slow_next_q8 < -256000) slow_next_q8 = -256000

pll_err_slow_lpf_q8 = slow_next_q8

if (q8 >= 0)
    pll_err_slow_lpf = (q8 + 128) >> 8
else
    pll_err_slow_lpf = -(((-q8) + 128) >> 8)
```

The integer firmware shift is not a generic floating `/32`; signed shift/rounding behavior is part of the active implementation.

## 9. PI controller

Active branch is `PLL_USE_MATLAB_PI_GAIN=1`.

Error selection:

```text
if (pll_locked && dbg_pll_phase_ok)
    pll_err_pi = pll_err_slow_lpf
else
    pll_err_pi = pll_err_lpf

if (pll_locked && dbg_pll_phase_ok && abs(pll_err_slow_lpf) <= 5)
    pll_err_pi = 0
```

Firmware constants:

```text
PLL_PI_KP_MHZ_PER_UNIT          = 4775  mHz/(pu)
PLL_PI_KI_MHZ_PER_SEC_PER_UNIT  = 23873 mHz/(pu*s)
PLL_PI_INT_LIMIT_MHZ            = 8000 mHz = 8 Hz
PLL_STEP_PER_HZ_Q               = (360*512*4096)/2500 = 301989.888 Q/Hz
```

The milli-pu error is converted by the integer expression's `1,000,000` denominator. In equivalent continuous units:

```text
Kp = 4.775 Hz/pu = 30 rad/s
Ki = 23.873 Hz/s/pu = 150 rad/s^2
```

Active integer PI equations:

```text
next_int_acc = pll_int_acc + pll_err_pi
p_delta_q = (pll_err_pi * 4775 * PLL_STEP_PER_HZ_Q) / 1,000,000
i_delta_q = (next_int_acc * 23873 * PLL_STEP_PER_HZ_Q) /
            (1,000,000 * 2500)
w_delta_q = p_delta_q + i_delta_q
pll_step_target_q = PLL_WNOM_STEP_Q + w_delta_q
```

The firmware's `pll_err_pi` is an integer milli-pu-like Q0 value; the gain constants are expressed in milli-Hz-compatible integer units so the effective physical gain is applied without treating `raw_err=1000` as `1000 pu`. The feasibility model explicitly uses `e_pu = err_mpu / 1000.0`.

## 10. Integrator and anti-windup

```text
PLL_PI_INT_LIMIT_MHZ = 8000 mHz
PLL_PI_INT_LIMIT_RAW = (8000*1000*2500)/23873
                      = approximately 837766 raw accumulator units
```

Each enabled PI update forms `next_int_acc`. It is first bounded to `+/-PLL_PI_INT_LIMIT_RAW`. Then the prospective step target is calculated. If:

```text
step_target_q64 > PLL_OMEGA_MAX_STEP_Q && pll_err_pi > 0
```

or:

```text
step_target_q64 < PLL_OMEGA_MIN_STEP_Q && pll_err_pi < 0
```

the integrator write is frozen and `pll_int_acc` retains its previous value. Otherwise `pll_int_acc = next_int_acc`. This is anti-windup beyond a plain clamp.

Integrator resets at least on:

- `input_valid == 0` normal active reset path: set to 0.
- phase-init completion: set to 0 along with error filters and counters.
- `PLL_DISABLE_INTEGRATOR_TEST != 0`: forced to 0.
- initial/power-on state initialization.

## 11. Frequency and NCO step constants

Base geometry:

```text
PLL_STEP_Q       = 12
PLL_STEP_SCALE   = 4096
PLL_TABLE_SIZE   = 360
PLL_THETA_SUBDIV = 512
PLL_THETA_DEN_Q  = 512*4096 = 2,097,152 (fractional Q12 per table degree)
PLL_THETA_DEN_SHIFT = 21
```

Firmware step formula:

```text
step_q = frequency_hz * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ
Hz = step_q * FS_HZ / (PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE)
```

With `FS_HZ=2500`, the exact real derived values and integer macro truncation are:

| Frequency | Step Q real | Macro integer value |
|---:|---:|---:|
| 55 Hz | 16,609,443.84 | 16,609,443 |
| 57.5 Hz | 17,364,418.56 | 17,364,418 |
| 58 Hz | 17,515,412.48 | 17,515,412 |
| 60 Hz | 18,119,393.28 | 18,119,393 (`PLL_WNOM_STEP_Q`) |
| 62 Hz | 18,723,374.08 | 18,723,374 |
| 62.5 Hz | 18,874,368.00 | 18,874,368 |
| 65 Hz | 19,327,353.60 | 19,327,353 |

`W58` and `W62` are derived here; the source has `W55`, `W57P5`, `WNOM`, `W62P5`, `W65`, but no dedicated `W58/W62` macros.

`PLL_OMEGA_MIN_STEP_Q = PLL_W55_STEP_Q`; `PLL_OMEGA_MAX_STEP_Q = PLL_W65_STEP_Q`.

## 12. Slew limiter

Active macros:

```text
PLL_STEP_SLEW_ACQ_Q = (100*360*512*4096)/(2500*2500)
                    = 12079.59552 Q/update
                    = 12079 integer Q/update after cast

PLL_STEP_SLEW_LOCK_Q = (60*360*512*4096)/(2500*2500)
                     = 7247.757312 Q/update
                     = 7247 integer Q/update after cast
```

Conversion:

```text
Hz/update = slew_q * FS_HZ / (360*512*4096)
Hz/s      = Hz/update * FS_HZ
```

Thus the macro intent is exactly `100 Hz/s` acquisition and `60 Hz/s` locked; integer truncation makes the implemented values approximately `99.995 Hz/s` and `59.994 Hz/s`.

- ACQ slew is used unless both `pll_locked` and `dbg_pll_phase_ok` are true.
- LOCK slew is used only when both are true.
- The step difference is clamped to `+/-step_slew_q` before adding to `pll_step_q`.

## 13. Theta / NCO update

The active NCO is updated at the 2.5 kHz PLL rate, not at 40 kHz:

```text
dbg_pll_phase_before_q32 = phase(pll_idx, theta_acc_q)
theta_acc_q += pll_step_q
step_degrees = theta_acc_q >> PLL_THETA_DEN_SHIFT
theta_acc_q -= step_degrees << PLL_THETA_DEN_SHIFT
pll_idx += step_degrees
if (pll_idx >= PLL_TABLE_SIZE) pll_idx -= PLL_TABLE_SIZE
```

The active source uses `PLL_THETA_STEP_GAIN_NUM=1` and `DEN=1`. `pll_idx` is the integer degree coordinate; `theta_acc_q` is the fractional coordinate. The sine output uses `PLL_SinInterp(pll_idx, theta_acc_q)` and is signed Q15. The feasibility model uses a real angle and equivalent update:

```text
theta += 2*pi*frequency_est/FS_PLL
if (theta >= 2*pi) theta -= 2*pi
```

That is an intentional real-model approximation of the firmware table/fraction accumulator.

## 14. Active input-valid detector

Active call chain:

```text
ADC AN1 interrupt registration (line 6313)
  -> ADC1_channel_AN1_CallBack(adcVal) (line 8608)
  -> grid_offset = GRID_ADC_CENTER because PLL_USE_DYNAMIC_OFFSET=0
  -> v_in_count = adcVal - grid_offset
  -> update an1_amp_min/an1_amp_max over PLL_INPUT_AMP_WINDOW_SAMPLES
  -> an1_amp_counts = (an1_amp_max - an1_amp_min)/2
  -> an1_amp_valid = (an1_amp_counts >= AN1_INPUT_ENABLE_AMP_COUNTS)
  -> ZC estimator updates an1_freq_valid for diagnostics/sanity
  -> PLL_BYPASS_ZC_VALID=1 forces input_valid = an1_amp_valid
  -> pll_locked uses input_valid
```

Active constants:

```text
PLL_INPUT_AMP_WINDOW_SAMPLES = 100 callback samples
AN1_AMP_PROFILE = BRINGUP
AN1_INPUT_VALID_AMP_COUNTS_BRINGUP = 60
AN1_GRID_PRESENT_MIN_AMP_COUNTS = 300
AN1_INPUT_ENABLE_AMP_COUNTS = max(60, 300) = 300
```

The active code at lines 8898-8901 computes exactly `(max-min)/2` and a single ON comparison `>=300`. The window is non-overlapping: min/max are initialized when the window counter is zero, accumulated for 100 callbacks, evaluated, then the counter returns to zero. It is not sliding, rolling, or peak-hold.

There is **no active amplitude OFF threshold of 200**, no amplitude hysteresis, and no amplitude confirmation/debounce counter in this active branch. The `AN1_VMAG_VALID_ON_Q=300<<12`, `AN1_VMAG_VALID_OFF_Q=200<<12`, `AN1_VALID_CONFIRM_SAMPLES=75`, and `AN1_LOST_CONFIRM_SAMPLES=8` definitions belong to the later Stage-4/magnitude path and are not the active `input_valid` assignment in this callback.

ZC path:

```text
an1_freq_valid = frequency sanity result from filtered ZC period
```

But with `PLL_BYPASS_ZC_VALID=1`:

```text
input_valid = an1_amp_valid
```

The ZC path does not feed `pll_step_q` and does not gate final PLL input validity in the active bypass branch.

### Firmware state effects when input is invalid

In the active callback's normal `if (input_valid == 0)` path, firmware resets:

```text
pll_locked = 0
dbg_pll_phase_ok = 0
pll_lock_count = 0
pll_unlock_count = 0
pll_phase_ok_count = 0
pll_phase_bad_count = 0
pll_err_lpf = 0
pll_err_slow_lpf = 0
pll_err_slow_lpf_q8 = 0
pll_int_acc = 0
pll_step_q = PLL_WNOM_STEP_Q
pll_step_target_q = PLL_WNOM_STEP_Q
sogi_tune_step_q = PLL_WNOM_STEP_Q
theta_acc_q = 0
pll_idx = 0
pll_phase_init_cnt = 0
pll_phase_initialized = 0
pll_pi_enabled = 0
sogi_va/vb = 0
sogi_va_n1/n2 = 0
sogi_vb_n1/n2 = 0
sogi_vin_n1/n2 = 0
```

Thus theta and SOGI are not free-running through an invalid-input interval in the active firmware path. On a newly valid transition, step, PI, and grid-ready state are reinitialized before reacquisition.

## 15. `phase_ok` state machine

Active WIDE constants:

```text
PLL_GRID_PHASE_ON_ERR_TH  = 180 mpu
PLL_GRID_PHASE_OFF_ERR_TH = 360 mpu
PLL_GRID_PHASE_ON_COUNT_TH = 10 updates
PLL_GRID_PHASE_OFF_COUNT_TH = 300 updates
```

Behavior:

```text
if (pll_pi_enabled == 0 || input_valid == 0) {
    phase_ok = 0;
    phase_on_count = 0;
    phase_off_count = 0;
} else if (abs(slow_err) <= 180) {
    phase_on_count++ up to 10;
    phase_off_count = 0;
    phase_ok = 1 when on_count >= 10;
} else if (abs(slow_err) > 360) {
    phase_on_count = 0;
    phase_off_count++ up to 300;
    phase_ok = 0 when off_count >= 300;
}
```

For values between 180 and 360, the counters hold according to the active branch; they are not an instantaneous `phase_ok = (abs(error)<=threshold)` assignment. Invalid input or disabled PI clears phase state immediately.

## 16. `pll_locked` state machine

Active WIDE lock thresholds:

```text
PLL_LOCK_ON_ERR_TH = 180 mpu
PLL_LOCK_OFF_ERR_TH = 360 mpu
PLL_FREQ_MIN_X10_ACTIVE = 580  => 58.0 Hz
PLL_FREQ_MAX_X10_ACTIVE = 620  => 62.0 Hz
PLL_STEP_DELTA_LOCK_MAX_MHZ = 150 mHz
PLL_LOCK_COUNT_TH = 20 updates
PLL_UNLOCK_COUNT_TH = 120 updates
```

The active lock condition is:

```text
lock_condition =
    (pll_pi_enabled != 0) &&
    (input_valid != 0) &&
    (abs(pll_err_lpf) <= 180) &&
    (58.0 <= frequency_est <= 62.0) &&
    (abs(pll_step_diff_q) <= PLL_STEP_DELTA_LOCK_MAX_Q)
```

`phase_ok` is **not** required by the active `pll_locked` acquisition condition. It only selects the locked PI error/slew behavior after phase state is ready.

When lock condition is true, `pll_lock_count` increments up to 20, `pll_unlock_count` clears, and `pll_locked` sets at 20. When any lock condition is false, the lock count clears. If already locked, an invalid condition increments `pll_unlock_count` up to 120; `pll_locked` clears at 120. If not already locked, unlock count remains zero and `pll_locked` remains zero.

Unlock causes are: PI disabled, input invalid, fast error above 360, frequency outside 58..62 Hz, or step change above 150 mHz equivalent. `phase_ok` is not a direct unlock condition.

## 17. Phase initialization

`PLL_PHASE_INIT_ENABLE=1`, `PLL_FAST_PHASE_INIT_ENABLE=0` is the active branch.

- Trigger: input becomes valid and `pll_phase_initialized==0`.
- Warmup: `PLL_PHASE_INIT_WARMUP_SAMPLES=100` callback updates. At 2.5 kHz this is 40 ms.
- During warmup: PI disabled, lock/phase counters and error filters are cleared, nominal step is restored; SOGI continues warming while theta/NCO debug behavior remains nominal in the active firmware path.
- Search: `PLL_PhaseInitFromSOGI(v_in_count)` scans candidate `pll_idx=0..359` degrees.
- Detector candidate: `theta_det = candidate_idx + 4 deg`; `vq_test = Q15_Round(va*cos(theta_det) + vb*sin(theta_det))`.
- Criterion: minimum absolute `|vq_test|`; when input sign is usable (`|v_in_count|>50`), prefer candidates whose `sin(candidate_idx)` has the same sign as the input.
- Result: `pll_idx = best_idx`, `theta_acc_q = 0`; then phase initialized and PI enabled.
- After completion: error filters, integrator, lock and phase counters are reset to zero before normal PI acquisition.
- Timeout/fallback: no separate timeout in this active phase-init block. The scan always completes when the 100-update warmup count is reached. The compile-time fast branch is the only fallback and is inactive.

Before phase-init completes, input validity controls whether the whole PLL/SOGI state is reset; PI is disabled. The normal active firmware does not use the phase-init scan as a cold-start test.

## 18. Reset paths

The following are the important active runtime reset writers; old/commented branches are excluded.

| Condition | Variables reset/frozen | Assigned value |
|---|---|---|
| `input_valid == 0` in active callback | SOGI states `sogi_va/vb`, all SOGI history, `theta_acc_q`, `pll_idx`, error LPFs, integrator, step, PI, phase and lock flags/counters | zeros, except step = `PLL_WNOM_STEP_Q` and `pll_phase_initialized=0` |
| input becomes valid (`input_valid != 0 && prev_input_valid == 0`) | step/integrator and grid-ready bookkeeping | step nominal, integrator 0, counters/grid-ready cleared |
| phase-init warmup | PI, lock/phase flags/counters, LPFs, integrator, step | PI 0, flags/counters/LPFs/integrator 0, step nominal |
| phase-init completion | error LPFs, integrator, lock/phase counters/flags | zeros; then PI enabled |
| `PLL_DISABLE_INTEGRATOR_TEST != 0` | integrator | 0 |
| normal PI update | integrator | accumulated error, with limit and anti-windup freeze |
| phase/lock thresholds | only related counters/flags | phase/lock counter reset or increment per state machine |
| power-on/static initialization | all PLL/SOGI state | source-defined zero/nominal initial state |

The active source contains additional writes in diagnostic/Stage-3 sections. Those are conditional branches and must not be confused with the default active AN1 callback path.

## 19. Output debug and sine

Active output coordinate:

```text
theta_output_idx = PLL_AddDegrees(pll_idx, PLL_OUTPUT_PHASE_COMP_DEG)
sin_ref = PLL_SinInterp(theta_output_idx, theta_acc_q)
```

Because `PLL_OUTPUT_PHASE_COMP_DEG=0`, output is the current PLL phase. `sin_ref` is signed Q15, approximately `-32768..+32767`; it is not half-wave mapped, offset, or made absolute. The active firmware uses it to form signed current/reference values. The 4-degree phase compensation is only used in the detector and phase-init scan.

## 20. Firmware versus `pll_lock_test.va`

| Area | Classification | Firmware actual | Feasibility model |
|---|---|---|---|
| Sampling | APPROXIMATION | 40 kHz raw timing, 2.5 kHz callback/SOGI/PLL | 40 kHz timer, `/16`, 2.5 kHz update; same intended timing |
| Input scaling | APPROXIMATION | `adcVal-GRID_ADC_CENTER`, center 1986, 12-bit ADC | direct `V(input)*1000`; explicitly defined test scaling |
| Input-valid window | MATCH for window mechanics | 100 callback samples, non-overlap, `(max-min)/2` | same 100-sample non-overlap calculation |
| Input-valid threshold | MISMATCH | active ON is `>=300`; no active OFF=200 | model adds ON `>=300` and OFF `<=200` hysteresis state |
| Input-valid debounce | MISMATCH | no confirmation/debounce counter on active amplitude flag | model has one-window update counters, effectively immediate at window boundary |
| ZC validity | MATCH in final effect | `PLL_BYPASS_ZC_VALID=1` makes final `input_valid=an1_amp_valid`; ZC is not final gate | no ZC path; final effect same |
| SOGI coefficients | APPROXIMATION | fixed Q30 integer coefficients | same numeric coefficients as real values |
| SOGI equation | MATCH topology | exact two-pole alpha/beta difference equations | same topology and update order |
| Q30 rounding | MISSING | symmetric `Q30_Round` on every SOGI update | real arithmetic, no Q30 rounding |
| SOGI state format | APPROXIMATION | Q12 integer state | real count-scaled state |
| Magnitude normalization | MATCH topology | max + half min, 100-count Q12 floor | same max + half min and 100-count real floor |
| Fast LPF | APPROXIMATION | integer `/64` right shift | real `/64` |
| Slow LPF | APPROXIMATION | Q8 state, signed shifts, delta/state clamps, symmetric rounding | same Q8 structure and limits, real division for shifts |
| PI scaling | APPROXIMATION | integer mHz/step denominators | explicit `e_pu=err/1000`, continuous-equivalent gains |
| PI error source | MATCH | slow error only when locked AND phase_ok, otherwise fast | same |
| Soft-track freeze | MATCH | locked AND phase_ok AND `abs(slow)<=5` | same |
| Integrator limit | APPROXIMATION | raw accumulator limit ~837766 and integer denominators | direct +/-8 Hz integral representation |
| Anti-windup | APPROXIMATION | prospective step-target directional freeze plus integer clamp | direct frequency-target directional undo plus clamp |
| Frequency limits | MATCH | 55..65 Hz NCO capability, 58..62 lock window | same |
| Slew limiter | APPROXIMATION | integer Q step deltas, ACQ/LOCK selected by locked AND phase_ok | equivalent 100/60 Hz/s real rates |
| Theta/NCO | APPROXIMATION | `pll_idx` + Q fractional accumulator, table geometry | real theta in radians with equivalent 2.5 kHz increment |
| Theta reset policy | MATCH | invalid input resets SOGI/theta/PLL state in active firmware | normal model also resets on invalid input; `NCO_FIXED_60HZ_TEST` intentionally bypasses this for isolation |
| `phase_ok` | MATCH thresholds/counters | 180/360, 10/300, persistent state | same thresholds/counters |
| `pll_locked` | MATCH criteria | PI, input, fast error, frequency, step delta; no phase_ok requirement; 20/120 | same intended criteria and debounce |
| Phase-init | APPROXIMATION | 100 callback warmup, firmware 360-point Q15 scan with input-sign preference | 100-update real scan with equivalent sign preference |
| Output sine | APPROXIMATION | signed Q15 `PLL_SinInterp`, output phase comp 0 | real `sin(theta)`; no offset/clamp |
| Extra fixed-NCO mode | MISSING in firmware default | no equivalent active default mode | model-only `NCO_FIXED_60HZ_TEST` diagnostic switch |

## Bottom-line audit facts

1. The active firmware amplitude-valid path really is the 100-sample non-overlapping raw window and a single `>=300` comparison. The `200` OFF threshold and 75/8 confirmation constants are not used by that active assignment.
2. With `PLL_BYPASS_ZC_VALID=1`, the final PLL validity flag is the amplitude flag; ZC does not rescue or debounce it.
3. When that active flag is false, firmware explicitly resets SOGI, theta, NCO step, integrator, filters, phase state, and lock state.
4. Active firmware `pll_locked` acquisition does not require `phase_ok`; it requires PI enabled, input valid, fast error <=180, frequency 58..62 Hz, and step delta <=150 mHz.
5. The feasibility model is topology-faithful but not bit-exact: it differs mainly in fixed-point rounding/shift semantics, input scaling abstraction, input-valid hysteresis, and real-valued PI/integrator implementation.
