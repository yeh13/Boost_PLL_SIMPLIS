# Step 5A fresh feedforward reconstruction

Result: **PASS**. Fresh run: `20261001T065636_126812Z`. Step 5B was not executed.

## Scope and method

Original Step 3 schematic was copied into the dedicated run folder and freshly netlisted/preprocessed. Baseline, firmware, controller, PLL, LUT, Buck controller, and Step 4B/4C artifacts retain their recorded SHA256 values. No current-controller output was connected. The transient retains the existing Step 3 voltage-control circuit solely to observe the original feedforward X block; its voltage-loop correction is not the comparison command.

The source X is `NLB_MULT1_DIV1` with GAIN=1, VOL=0, VOH=20, RIN=10G, ROUT=50 and LOWPASS_FREQ=10 MHz. Static uses the same library model and original SUMMER2 add/subtract models, driven by 25 independent constant Vref/Vdc pairs. Vdc=100 V. Points cover the boundary, middle/high reference and D=0.85 neighborhood (including exactly 0.85 and points above it). No 0.85 clamp is added to the original continuous X block.

Formal residual: **12500 * (X - (Vref-Vdc)/(Vref+Vdc))**, evaluated only where Vref>=Vdc. 12500 is the period count domain; 10625 is not a denominator. This is a domain correspondence test, not calibration of inherited firmware Vdc=574.

Transient: original 60 Hz reference chain, 100 V source, R31=196 ohms, fsw=40 kHz, `.TRAN 120m 0`. Same-row V(227), V(226), V(235) supply actual Vref, Vdc, X; V(228)/V(237) independently checks the add/subtract chain. Analysis includes the entire 0–120 ms Boost region. Main RMS and mean use equal weighting per saved sample (event samples are nonuniform); the metrics also include a separate interpolated 40 kHz / 0.85-carrier-phase diagnostic. No results from older datasets were used.

## Measured results

| Metric | Fresh result | Historical comparison |
|---|---:|---:|
| Static operating points | 25 | — |
| Static rows per channel | 241 | — |
| Static comparisons | 6025 | — |
| Static max residual [count] | 0.000601852 | ~0.000417 |
| Transient total rows | 806608 | — |
| Transient Boost samples | 623258 | — |
| Transient max residual [count] | 0.762483207 | ~3.815614 |
| Transient RMS residual [count] | 0.073224159 | ~1.252406 |
| Transient mean residual [count] | +0.020167134 | ~+0.797733 |
| Mode-boundary max [count] | 0.762483207 | ~0.509691 |
| Mode-boundary samples | 1606 | — |
| Add/subtract-chain max residual [count] | 0.003120344 | — |
| Engine return code, static / transient | 0 / 0 | required 0 |

Mode-boundary definition: **0 <= Vref-Vdc <= 1 V** on the Boost side. Historical source and exact historical masks are unavailable; values are compared by scale, not assumed to be from identical vectors. Static CLI t2 stores X to about seven significant decimal digits: roundoff alone can reach 0.000625 count in the high-duty region. Its observed residual remains below the predeclared 0.001-count static threshold. The exact 0.85 operating point is included in static/residuals.csv.

Review bounds used for reconstruction: static max <0.001 count; transient max <5 counts, RMS <2 counts, |mean|<1.5 counts, boundary max <1 count. These are reconstruction review bounds based on the historical scale, not recovered historical test criteria. They do not establish closed-loop stability or bit-exact runtime parity. The finite-bandwidth/PWL solver output and saved-sample interpolation account for nonzero dynamic differences; no controller parameter was adjusted to force agreement.

## Fresh-data proof

`.OPTIONS SAVE_INSTANTS_FILES` is a flag with **no =value**. Local engine error 1105 confirms assignments are invalid; enabling the flag produces retained t1 alongside CLI t2. See [OUTPUT_CONTRACT.md](step5a_reconstruction/OUTPUT_CONTRACT.md) and local SIMPLIS Reference sections 8.5/8.6. The successful CLI t2 contains actual numeric rows, not only metadata.

Both simulations ran `simplis.exe engine.deck -f` only after preprocessing succeeded. Each case directory contained no t1/t2 before launch. Nonempty new t1/t2, engine exit=0, matching recorded SHA256, timestamps within the process interval (2-second removable-volume timestamp rounding), monotonic times, header/actual point-count equality and exact 0–0.12 s span all passed before residual analysis. `.TRAN 120m 0` was retained; no `.PRINT` was added. `.KEEP *V` captures the observation voltages.

## Evidence and reproduction

Run directory: `step5a_reconstruction/20261001T065636_126812Z/`.

- `manifest.json`: baseline and firmware preservation hashes and static stimuli.
- `static/` and `transient/`: input.net, preprocessed engine.deck, engine.console.log, new t1/t2, freshness.json, dataset_gate.json and metrics.json.
- `static/residuals.csv` and `transient/boost_residuals.csv`: measured residuals.
- `step5a_run.py prepare_static` creates a new run; `step5a_analyze.py <run>/static` must PASS before `step5a_run.py transient --run <run>`; then `step5a_analyze.py <run>/transient` and `step5a_report.py <run>`.
- DLLs, t1/t2, engine intermediates, extracted local manuals and failed diagnostic runs are generated evidence, not controller source. Failed/missing-data runs were never analyzed.

SHA256 preservation verified at publication:

- `F:\BoostPLLSIMPLIS\DAC test\CURRENT_CONTROL\Bidirection High Gain Inverter_Current_Control.sxsch`: `23c1708af6a6acd5b25d24b72d85e81aad96e98936be95b1872494482ed1ecaa`
- `F:\BoostPLLSIMPLIS\Boost_I_loop_inverter.X\mcc_generated_files\adc1.c`: `d51a6f9652382b3538d77fde841034193f60a9cc0bbd7f4a872f408d072998bf`
- `F:\BoostPLLSIMPLIS\Boost_I_loop_inverter.X\mcc_generated_files\pwm.c`: `794a84fdd4ad22690855056f8a77c757964c5a3d8d14671c65b9b274eac8ada0`
- `F:\BoostPLLSIMPLIS\DAC test\CURRENT_CONTROL\phase2a_step4c.py`: `1983d556355df624f1799c4f80252c1aa746ab0a6b9c1884496855dba8a43fc5`
- `F:\BoostPLLSIMPLIS\DAC test\CURRENT_CONTROL\STEP4B_CADENCE_RESTORE.md`: `b667fb4ae06a9a41d529802a1f5df0404ac2e5d7b49b44306c2495ec31e8aec0`
- `F:\BoostPLLSIMPLIS\DAC test\CURRENT_CONTROL\STEP4C_ADC_INPUT_PARITY.md`: `beb0ecfc645832b4f2677eb857e125cb842780b1f12b5ef628fe06046d8c15a0`

Next: eligible for Step 5B reconstruction in the next round. No runtime Verilog or current closed-loop integration was created.
