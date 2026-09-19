# Project Instructions

## Project Goal
Target control architecture:

40 kHz raw ADC / PWM timing
→ 5 kHz SOGI / PLL update
→ PLL-derived 40 kHz phase delivery
→ current reference
→ current controller
→ PWM / power stage
→ current feedback

## Important Rules
1. Do not directly modify validated active firmware unless explicitly requested.
2. The current active firmware PLL path is not the final target architecture.
3. The target PLL architecture is:
   - 40 kHz raw timing
   - decimation by 8
   - 5 kHz SOGI / PLL
   - PLL-derived 40 kHz phase extrapolation
4. Fixed60 MATLAB is the current algorithm reference for phase delivery.
5. `reference_candidate` is only a candidate C implementation and must not be described as validated.
6. C / MATLAB parity must pass before full SIMPLIS integration.
7. Do not mix conclusions from Stage-3D, Stage-4, Fixed60, and active firmware unless explicitly comparing them.
8. Preserve validated files. Use candidate files, branches, or new files for experimental changes.
9. Record important SHA256 values when creating a new validated reference.
10. Large simulation outputs should be written to files instead of pasted into chat.
11. After completing an important task, update `docs/PROJECT_STATUS.md` and `docs/TODO.md`.

## Important Directories
- `Boost_I_loop_inverter.X/`
  - MCU firmware project
- `Boost_I_loop_inverter.X/mcc_generated_files/`
  - ADC, PWM, PLL and generated MCU source files
- `Boost_I_loop_inverter.X/mcc_generated_files/reference_candidate/`
  - 5 kHz PLL + 40 kHz phase candidate implementation
- `DAC test/`
  - SIMPLIS, MATLAB, Verilog and controller validation files
- `DAC test/FIXED60_PHASE40K_INTEGRATION/`
  - Fixed60 reference and phase delivery integration work

## Before Making Changes
Always read:
1. `AGENTS.md`
2. `docs/PROJECT_STATUS.md`
3. `docs/ARCHITECTURE.md`
4. `docs/TODO.md`

Then inspect only files directly relevant to the requested task.
