# PLL amplitude and phase analysis

## Scope and test conditions

This report changes no firmware and no PLL/SOGI dynamics. The sampled-model
results use the equations and update order in `pll_lock_test.va`: 40 kHz raw
sampling, one non-overlapping 100-sample amplitude window, and a 2.5 kHz
SOGI/PLL update. Every input is a zero-offset, zero-phase, 60 Hz sine.

Results labelled **simulation measured** below are from an offline numerical
replay of the sampled Verilog-A equations. The added analog diagnostics permit
the same measurements to be checked directly in SIMPLIS.

## Input amplitude sweep — simulation measured

There are 400 detector windows per one-second run. `Dropout` means that the
original validity flag makes at least one high-to-low transition. Mode 0 lock
means a sustained lock at the end of the run, not a momentary assertion before
the next validity reset.

| Peak counts | Vpk | Vpp | ADC range feasible | amp min | amp max | amp average | Valid | Invalid | Valid duty | Dropout | Mode 0 sustained lock | Mode 1 lock |
|---:|---:|---:|:---:|---:|---:|---:|---:|---:|---:|:---:|:---:|:---:|
| 500 | 0.500 V | 1.000 V | Yes | 46.369 | 222.288 | 146.136 | 0 | 400 | 0.00% | No (never high) | No | Yes |
| 750 | 0.750 V | 1.500 V | Yes | 69.553 | 333.432 | 219.203 | 199 | 201 | 49.75% | Yes | No | Yes |
| 1000 | 1.000 V | 2.000 V | Yes | 92.737 | 444.576 | 292.271 | 320 | 80 | 80.00% | Yes | No (momentary only) | Yes |
| 1250 | 1.250 V | 2.500 V | Yes | 115.921 | 555.720 | 365.339 | 320 | 80 | 80.00% | Yes | No (momentary only) | Yes |
| 1500 | 1.500 V | 3.000 V | Yes | 139.106 | 666.864 | 438.407 | 320 | 80 | 80.00% | Yes | No (momentary only) | Yes |
| 1750 | 1.750 V | 3.500 V | Yes | 162.290 | 778.008 | 511.475 | 320 | 80 | 80.00% | Yes | No (momentary only) | Yes |
| 2000 | 2.000 V | 4.000 V | **No** | 185.474 | 889.151 | 584.542 | 320 | 80 | 80.00% | Yes | No (momentary only) | Yes |

At `GRID_ADC_CENTER=1986`, a symmetric centered waveform is limited to
`min(1986, 4095-1986) = 1986` peak counts. The 2000-count mathematical case
would demand ADC codes -14 through 3986 and is therefore not physically valid
without clipping at the low rail.

## Original validity detector — analytical calculation

For a discretely sampled 60 Hz sine and every possible phase placement of a
100-sample window at 40 kHz (the 100 samples span 53.46 degrees), an exhaustive
phase search gives

```text
minimum amp_window / input_peak = 0.0534264469
maximum amp_window / input_peak = 0.4497867052
```

Consequently, guaranteeing the 300-count threshold in every window requires

```text
required centered ADC peak = 300 / 0.0534264469
                           = 5615.20 counts
required ADC peak-to-peak  = 11230.39 counts
```

This cannot fit a 12-bit ADC, and especially cannot fit around center 1986,
where the maximum symmetric peak is 1986 counts. The periodic original-valid
dropout is therefore an inherent consequence of this detector/window/threshold
combination for the tested physical range; this analysis does not change the
threshold.

## Grid versus PLL phase — simulation measured

Sign convention: positive means the second signal's positive-going crossing
occurs later (lags); negative means it occurs earlier (leads). Crossings are
interpolated between samples, paired with the nearest same-cycle crossing, and
wrapped to `[-180, +180)`.

Statistics start five measured grid cycles after lock and cover 20 cycles.

| Peak counts | Grid Hz | PLL Hz at 2 s | Current deg | Signed avg deg | Avg abs deg | Max abs deg | Min deg | Max deg | Std dev deg | Avg dt us |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 500 | 60.000000 | 59.999687 | -2.384667 | -2.384667 | 2.384667 | 2.384667 | -2.384667 | -2.384666 | 0.0000005 | -110.401 |
| 750 | 60.000000 | 59.999778 | -2.496198 | -2.496199 | 2.496199 | 2.496200 | -2.496200 | -2.496198 | 0.0000005 | -115.565 |
| 1000 | 60.000000 | 59.999914 | -2.568147 | -2.568148 | 2.568148 | 2.568149 | -2.568149 | -2.568147 | 0.0000005 | -118.896 |
| 1250 | 60.000000 | 59.999950 | -2.606056 | -2.606057 | 2.606057 | 2.606058 | -2.606058 | -2.606056 | 0.0000005 | -120.651 |
| 1500 | 60.000000 | 59.999709 | -2.643252 | -2.643253 | 2.643253 | 2.643254 | -2.643254 | -2.643252 | 0.0000005 | -122.373 |
| 1750 | 60.000000 | 59.999765 | -2.678641 | -2.678641 | 2.678641 | 2.678642 | -2.678642 | -2.678641 | 0.0000005 | -124.011 |
| 2000 | 60.000000 | 60.000010 | -2.692690 | -2.692691 | 2.692691 | 2.692692 | -2.692692 | -2.692690 | 0.0000005 | -124.662 |

All Mode 1 cases locked. For the 1000-count baseline, lock first asserted at
approximately 8 ms. Mode 0 is intentionally excluded from steady-state phase
statistics because its recurrent validity reset prevents sustained lock.

## Baseline phase chain — simulation measured

This table uses the 1000-count Mode 1 case and measured grid frequency. A
positive value is lag and a negative value is lead.

| Signal pair | Phase difference | Direction | Equivalent time |
|---|---:|---|---:|
| Grid -> SOGI alpha | +0.151139 deg | alpha lags grid | +6.997 us |
| SOGI alpha -> SOGI beta | +89.984296 deg | beta lags alpha | +4165.940 us |
| Grid -> PLL output sine | -2.568148 deg | PLL leads grid | -118.896 us |
| Phase detector internal compensation | +4.000000 deg | added only inside detector angle | +185.185 us equivalent |

The internal `PLL_PHASE_COMP_DEG=4` is used only in
`theta_det = theta + 4 deg`. It is not added to `v_pll_sin`, because
`PLL_OUTPUT_PHASE_COMP_DEG=0`. Therefore the measured Grid-to-PLL row is the
actual model output phase and must not be post-corrected by four degrees.

The measured alpha/beta direction is **beta lags alpha by 89.9843 degrees**.

## Degree to time conversion — analytical calculation

Using the measured grid period (`16.6666667 ms`, approximately 60.000000 Hz):

| Phase | Time |
|---:|---:|
| 1 deg | 46.296 us |
| 2 deg | 92.593 us |
| 4 deg | 185.185 us |
| 5 deg | 231.481 us |
| 10 deg | 462.963 us |

## Added SIMPLIS analog diagnostics

The following outputs are driven continuously by `pll_lock_test.va`:

| Output | Meaning | Scaling |
|---|---|---|
| `v_phase_diff_deg_dbg` | PLL positive ZC relative to nearest grid positive ZC | 10 mV/degree |
| `v_phase_dt_dbg` | signed PLL-minus-grid crossing time | 1 V/ms |
| `v_grid_freq_meas_dbg` | frequency from consecutive grid positive crossings | 1 V/10 Hz |
| `v_sogi_phase_diff_dbg` | beta positive ZC relative to nearest alpha positive ZC | 10 mV/degree |
| `v_input_alpha_phase_dbg` | alpha positive ZC relative to nearest input positive ZC | 10 mV/degree |

For the first SIMPLIS phase run, set `PLL_VALID_BYPASS_TEST=1`, use 1000
centered peak counts, and expose all five new pins on the Verilog-A symbol.
Also retain `v_input_valid_original_dbg` and `v_input_valid_for_pll_dbg` to
confirm that original validity toggles while PLL validity remains at 5 V.

## Conclusion

**Simulation measured:** Mode 1 produces lock and stable phase measurements
for every mathematical amplitude case. Mode 0 cannot maintain lock because
the original amplitude-valid result drops periodically. The baseline SOGI pair
is quadrature, with beta lagging alpha by approximately 89.984 degrees.

**Analytical calculated:** a 100-sample detector window at 40 kHz can see only
5.3426% of sine peak in its worst phase placement. Guaranteeing a 300-count
window therefore requires about 5615 centered peak counts, which is impossible
for the stated 12-bit ADC centered at 1986.

No firmware conclusion or firmware change is made in this report.
