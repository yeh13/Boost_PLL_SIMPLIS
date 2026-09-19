# Run the 60-Hz integration candidate
NEW CANDIDATE - NOT YET VERIFIED

Upload this entire MATLAB folder, then run:
```matlab
run_fixed60_phase40k
```
All dependencies are in this folder. No fixed drive paths, toolbox or C compiler required.
Input: 60.000 Hz, 30 seconds. A common 20-second steady window starts at least one second after
first HOLD entry. Results are saved under RESULTS/<timestamp>.

stage17_model_60hz.m, firmware_config.m and sine/Vref tables remain unchanged.
phase_delivery_8tick.m is the NEW quotient/remainder distributed correction candidate.
See INTEGRATION_GUIDE.md for timing, units, metric definitions and verification limits.
The user-reported standalone Fixed60Hz PASS is not a PASS for this new integration.
