# Existing source classification (before integration directory)
Full paths, sizes and hashes: INVENTORY_BEFORE.csv and OLD_FILES_SHA256.csv.

| Existing file/group | Classification | Evidence / reuse |
|---|---|---|
| inverter_controller.v | 2.5 kHz SOGI only | /16, DECIM_MAX=15, Q30 bank -1919669515/867878708/...; comments explicitly say no PLL/current/PWM. Reuse equation/rounding structure only, not old parameters. |
| pll_lock_test.va | 2.5 kHz Verilog-A PLL feasibility model | FS_PLL=2500, DECIM_N=16. Not current firmware-equivalent PLL. |
| pll_stage3d_active_test.va | 2.5 kHz Stage-3D Verilog-A | FS_PLL=2500, DECIM_N=16; different phase-init/PI/validity pipeline. |
| dac_test_source.v | VSXA DAC transport test | Seven fixed signed codes, no PLL. |
| vsx_root.v | Generated simulator bench | Includes/instantiates dac_test_source only. |
| bus_to_voltage.va | Analog bus conversion | Not a PLL or extrapolator. |
| DAC teat.sxsch / design.net / design.out | Existing simulator schematic/netlist/output | design.net instantiates pll_lock_test; old absolute .LOAD path retained untouched. |
| CURRENT_REFERENCE_INTERPOLATION_INTENT_AUDIT.md and plots/CSV | 2.5k -> 40k causal /16 interpolation | Document says NOT forward prediction; last tick snaps to past target, fixed lag. Not a reusable /8 extrapolator. |
| Other 17 markdown / 27 CSV / 38 SVG artifacts | Historical audits and results | Their PASS claims concern their own old models, not this integration. |

Initial counts: .v 3; .vh 0; .m 0; .md 17; .csv 27; .va 3; .sxsch 1; .net 1;
plus logs, compiled .vvp and auxiliary .c/.mak/.info/.out files.
No 5 kHz PLL or correct 5k->40k extrapolator implementation was found in DAC test.
The previous DCAC package has a 5 kHz PLL candidate but its extrapolator snaps to each anchor.
That snap implementation conflicts with the current no-anchor-reset requirement and is not silently reused.
