# TODO

## Immediate
- [ ] Audit `pll_reference_5k40k.c` variable units and Q formats.
- [ ] Determine exact meaning of `pll_step_q`.
- [ ] Verify whether `phase_step_40k_q = pll_step_q / 8` is valid.
- [ ] Build or repair executable parity workflow.
- [ ] Compare C vs MATLAB at 59 Hz.
- [ ] Compare C vs MATLAB at 60 Hz.
- [ ] Compare C vs MATLAB at 61 Hz.
- [ ] Report first mismatch and maximum absolute error.

## After Parity
- [ ] Confirm smooth 40 kHz phase output.
- [ ] Confirm 0.54 degree/tick behavior at 60 Hz.
- [ ] Integrate validated phase delivery with current-reference generation.
- [ ] Check current-loop interaction.
- [ ] Move validated controller into SIMPLIS.

## SIMPLIS
- [ ] Finish ADC-only calibration.
- [ ] Verify ADC clock and analog input path.
- [ ] Complete seven-point ADC calibration.
- [ ] Connect validated PLL model.
- [ ] Connect current controller.
- [ ] Validate startup and zero-cross sequencing.

## Firmware Integration
- [ ] Keep active firmware unchanged until candidate is validated.
- [ ] Create dedicated integration branch before firmware modification.
- [ ] Record SHA256 of validated reference inputs.
