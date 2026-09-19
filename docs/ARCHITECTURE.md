# Target Architecture

## Signal Flow

Analog grid input
→ ADC sampling at 40 kHz timing
→ decimation by 8
→ 5 kHz voltage sample
→ SOGI
→ Park transform
→ vq
→ PLL PI
→ phase / frequency state
→ 40 kHz phase extrapolation
→ sine / current reference
→ current controller
→ duty command
→ PWM
→ power stage
→ current feedback

## Timing Domains

### 40 kHz Domain
Period:
25 us

Used for:
- PWM timing
- ADC raw timing
- phase delivery
- current-reference delivery
- final duty update where applicable

### 5 kHz Domain
Period:
200 us

Used for:
- SOGI update
- Park transform
- PLL PI update
- anchor phase update

Ratio:

40 kHz / 5 kHz = 8

Therefore there are 8 high-rate phase-delivery ticks per PLL update.

## Phase Delivery Requirement
At 60 Hz:

phase increment per 40 kHz tick:

360 × 60 / 40000 = 0.54 degrees per tick

The delivered phase must advance smoothly at 40 kHz and must not remain as a 5 kHz stair-step.

## Validation Principle
MATLAB is the algorithm reference.

The C implementation must demonstrate parity before being treated as authoritative.

Validation should include:
- 59 Hz
- 60 Hz
- 61 Hz
- phase increment
- phase wrap behavior
- vq
- PI output
- PLL step
- anchor phase
- 40 kHz delivered phase
- lock flags where applicable

## Safety Rule
Do not directly replace active firmware PLL code until:
- units are verified
- Q-format domains are verified
- parity test passes
- 40 kHz phase delivery is proven
