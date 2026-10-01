# Step 4C firmware-input digital parity

Reconstructed fresh result: **PASS**, 46,218 samples. UTC run: `20261001T063216_353015Z`.

Python integer model versus independently compiled source-extracted C oracle. The C oracle retains the full AN0 callback (including both reference branches), state clear, LUT, and Buck/Boost feedforward functions. Only host/register stubs and non-mutating trace instrumentation are added. PWM feedforward is updated before each AN0 call; this is a coherent input fixture, not an interrupt-race simulation. PLL state inputs are injected; PLL dynamics are not tested.

| Group | Samples |
|---|---:|
| reference_boundaries | 344 |
| error_boundaries | 18 |
| adc_sweep | 16384 |
| lut_and_reference_selection | 2672 |
| 60hz_120ms | 4800 |
| clamps_and_transitions | 2000 |
| random_inputs | 20000 |

| Field | Mismatches | Max absolute difference |
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

Coverage: `{"positive_correction_clamp": 17, "negative_correction_clamp": 1897, "upper_final_clamp": 8104, "lower_final_clamp": 1153, "y1_not_limited": 9257, "boost_to_buck": 2926, "buck_to_boost": 2928, "buck_filter_retained": 12596, "err_boundary_-4": 2, "err_boundary_-3": 2, "err_boundary_-2": 2, "err_boundary_-1": 2, "err_boundary_0": 2, "err_boundary_1": 2, "err_boundary_2": 2, "err_boundary_3": 2, "err_boundary_4": 2}`.

Reference -19..19 -> 0, +/-20 retained; error -2..2 -> 0, +/-3 retained. All 4096 ADC codes, all 334 LUT entries, both half-cycle polarities, lock/phase-valid gating, mode transitions, both correction clamps and both final-duty clamps are exercised. Buck clears controller history without resetting the ADC filter. Effective correction, not limited correction, is fed back as y1.

Signed 64-bit C accumulator; host arithmetic right shifts are checked at runtime. This validates host-executed firmware input arithmetic, not an XC16 target build, ADC calibration, board timing, SOGI/PLL parity, or power-stage stability.

Evidence: [metrics.json](step4c_results/20261001T063216_353015Z/metrics.json), inputs.csv, python.csv, c_oracle.csv, generated firmware_input_oracle.c, build.log in the same fresh directory. DLL is a generated build product.

## SHA256

- `Boost_I_loop_inverter.X\mcc_generated_files\adc1.c`: `d51a6f9652382b3538d77fde841034193f60a9cc0bbd7f4a872f408d072998bf`
- `Boost_I_loop_inverter.X\mcc_generated_files\pwm.c`: `794a84fdd4ad22690855056f8a77c757964c5a3d8d14671c65b9b274eac8ada0`
- `Boost_I_loop_inverter.X\mcc_generated_files\parameter.h`: `019e68650f8dba5737a9649e3de8093c5c51681572da4f1bc8447b8dc627282f`
- `Boost_I_loop_inverter.X\mcc_generated_files\clock.c`: `20db992d0ac9ebbe085fb8cb687e9f07a50e3ee492e019e9999b936e657a015e`
- `DAC test\CURRENT_CONTROL\Bidirection High Gain Inverter_Current_Control.sxsch`: `23c1708af6a6acd5b25d24b72d85e81aad96e98936be95b1872494482ed1ecaa`
- `DAC test\CURRENT_CONTROL\phase2a_step4c.py`: `1983d556355df624f1799c4f80252c1aa746ab0a6b9c1884496855dba8a43fc5`

Re-run with Python 3: `python phase2a_step4c.py`. Every invocation creates a unique directory and report; the top-level report is created only if absent. No firmware or existing report is overwritten.
