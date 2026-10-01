# Step 5B firmware integer command

**PASS — DIGITAL DOMAIN VALIDATED**

PHYSICAL VOLTAGE CALIBRATION: **NOT FORMALLY RE-VALIDATED**

Fresh run `20261001T073208_039093Z`: **7 groups, 46,090 samples, 18 formal fields**. No historical vectors were available; these are new deterministic reconstruction vectors, not a claim to reproduce historical 55,100 samples.

| Group | Samples | Field mismatches |
|---|---:|---:|
| steady_state | 12000 | 0 |
| mode_transition | 8000 | 0 |
| clamp_stress | 16384 | 0 |
| buck_state_clear | 4000 | 0 |
| positive_negative_phase | 5344 | 0 |
| reference_zero_band_boundary | 344 | 0 |
| error_deadband_boundary | 18 | 0 |

## Implementation reuse

Directly imports the unchanged Step 4C `Model`, `oracle_build`, `FIELDS`, and `INPUTS`; does not invoke its main() or rerun Step 4C. SHA256 guards require the original validated Step 4C source hashes. The independently compiled C oracle retains extracted actual AN0 callback, LUT, reference selection, state clear, and Buck/Boost feedforward functions. Python and C do not share arithmetic implementation. Only the vector generator and comparison/CSV harness are new.

Boost_PWM uses C integer division of `(vref_cmd-574)*12500/(vref_cmd+574)` in the Boost region. Because both operands are nonnegative there, Python floor division agrees with C truncation toward zero. Buck bypasses Boost division and sets Boost_PWM=0. No negative-division approximation is used. A=[30010,2668], B=[26000,-32000,9640], signed int64 accumulator, arithmetic >>15, correction [-2500,+2500], finalDuty [0,10625], and y1=finalDuty-Boost_PWM are unchanged. The C host width and negative right-shift checks run in the reused fixture. No new counts/V calibration or VrefTable changes.

## Parity

| Field | Mismatches | Max difference |
|---|---:|---:|
| adc0_filt | 0 | 0 |
| iref_cmd | 0 | 0 |
| err_raw | 0 | 0 |
| err | 0 | 0 |
| mode | 0 | 0 |
| Boost_PWM | 0 | 0 |
| Buck_PWM | 0 | 0 |
| acc | 0 | 0 |
| raw | 0 | 0 |
| limited | 0 | 0 |
| finalDuty | 0 | 0 |
| effective_correction | 0 | 0 |
| x1 | 0 | 0 |
| x2 | 0 | 0 |
| y1 | 0 | 0 |
| y2 | 0 | 0 |
| finalDuty_buck | 0 | 0 |
| mode_switches | 0 | 0 |

## Coverage

- correction_upper_clipped: 2975
- correction_lower_clipped: 1981
- finalDuty_upper_clipped: 5543
- finalDuty_lower_clipped: 2683
- finalDuty_at_10625: 5555
- finalDuty_at_zero: 8562
- y1_differs_from_limited: 8226
- boost_to_buck: 336
- buck_to_boost: 336
- buck_clear_samples: 5790
- buck_filter_retained: 5697
- phase_0: 23103
- phase_1: 22987
- pll_input_path: 30082
- lut_input_path: 16008

Clamp statistics count samples with strict pre-clamp violations, not distinct time episodes. Exact-limit counts are reported separately. Buck clears x1/x2/y1/y2 every callback, retains ADC filter history, and outputs Buck feedforward only. Boost re-entry has x2=y2=0. Both phases, LUT and injected PLL-reference paths, reference -21..21, and error -4..4 are explicitly checked. PLL dynamics and interrupt race timing are outside this digital fixture.

## Duty normalization / CSV

Dboost_fw=finalDuty/12500.0, observed range [0,0.85]; maximum finalDuty=10625. All 18 integer fields are serialized as integer decimal strings. Duty is serialized with eight decimal places and read using Decimal; duty*12500 recovers the exact finalDuty. Roundtrip mismatches=0; CSV input replay through C mismatches=0.

Digital arithmetic evidence does not establish physical voltage calibration, target MCU execution timing, PLL phase delivery, runtime Verilog behavior, analog duty output, or power-stage stability. The inherited 574/LUT domain remains in use.

Evidence: [metrics.json](step5b_results/20261001T073208_039093Z/metrics.json), inputs.csv, python.csv, c_oracle.csv, firmware_input_oracle.c, build.log in that run directory. DLL is generated, not formal source. Reproduce with Python 3: `python phase2a_step5b.py`; every invocation uses a unique directory.

## SHA256

- `F:\BoostPLLSIMPLIS\Boost_I_loop_inverter.X\mcc_generated_files\adc1.c`: `d51a6f9652382b3538d77fde841034193f60a9cc0bbd7f4a872f408d072998bf`
- `F:\BoostPLLSIMPLIS\Boost_I_loop_inverter.X\mcc_generated_files\pwm.c`: `794a84fdd4ad22690855056f8a77c757964c5a3d8d14671c65b9b274eac8ada0`
- `F:\BoostPLLSIMPLIS\Boost_I_loop_inverter.X\mcc_generated_files\parameter.h`: `019e68650f8dba5737a9649e3de8093c5c51681572da4f1bc8447b8dc627282f`
- `F:\BoostPLLSIMPLIS\Boost_I_loop_inverter.X\mcc_generated_files\clock.c`: `20db992d0ac9ebbe085fb8cb687e9f07a50e3ee492e019e9999b936e657a015e`
- `F:\BoostPLLSIMPLIS\DAC test\CURRENT_CONTROL\Bidirection High Gain Inverter_Current_Control.sxsch`: `23c1708af6a6acd5b25d24b72d85e81aad96e98936be95b1872494482ed1ecaa`
- `F:\BoostPLLSIMPLIS\DAC test\CURRENT_CONTROL\phase2a_step4c.py`: `1983d556355df624f1799c4f80252c1aa746ab0a6b9c1884496855dba8a43fc5`
- `F:\BoostPLLSIMPLIS\DAC test\CURRENT_CONTROL\STEP4B_CADENCE_RESTORE.md`: `b667fb4ae06a9a41d529802a1f5df0404ac2e5d7b49b44306c2495ec31e8aec0`
- `F:\BoostPLLSIMPLIS\DAC test\CURRENT_CONTROL\STEP4C_ADC_INPUT_PARITY.md`: `beb0ecfc645832b4f2677eb857e125cb842780b1f12b5ef628fe06046d8c15a0`
- `F:\BoostPLLSIMPLIS\DAC test\CURRENT_CONTROL\STEP5A_FRESH_RESULTS.md`: `045b4c7cf7be1331137e8020fa5d6953e67bf7711883800fbccce9df0109fefa`
