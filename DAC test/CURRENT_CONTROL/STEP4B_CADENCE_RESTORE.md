# Step 4B cadence reconstruction — 2026-10-01

**PASS — current source configuration audit.** No firmware edit was necessary or made. This is not a fresh board measurement, XC16 timing/WCET test, or proof that every real interrupt completes on time.

Base branch: `debug-analog-wrapper-wip`; HEAD: `eb5256ac960791e937f6787f28bae4d98598cdbd`. Audit uses current working files, not only committed files. The existing uncommitted `pwm.c` change already sets `ADTR1PS = 0`; HEAD has `7`. That difference was preserved.

## Current source evidence

Paths below are relative to `Boost_I_loop_inverter.X/mcc_generated_files/`.

| Evidence | Value / consequence |
|---|---|
| `clock.c`, CLOCK_Initialize | ACLKCON1=0x8101, APLLFBD1=0x7D, APLLDIV1=0x21: nominal 8 MHz FRC x125 /2 = 500 MHz auxiliary PLL output |
| `pwm.c:71`, PCLKCON | 0x33, auxiliary PLL selected; divider branch is available but not selected by PG1 |
| `pwm.c:283`, PG1CONL | 0x8008, master clock directly, independent-edge PWM |
| `pwm.c:229`, PG1PER | 0x30D3 = 12499; 12500 nominal 2 ns ticks = 25 us = 40 kHz |
| `pwm.c:235`, PG1TRIGA | 10625: trigger compare at 85% of carrier, nominal 21.25 us; this is not a duty denominator |
| `pwm.c:139–140` | PG1EVTL=0x108 followed by PG1EVTLbits.ADTR1PS=0: trigger every PWM cycle |
| `pwm.c:146` | PG1EVTH=0, no trigger-cycle offset |
| `adc1.c:874` | ADTRIG0L=0x0404: AN0 and AN1 use PWM1 Trigger1 |
| `adc1.c:795,855–859` | ADIEL=0x03; ADCAN0IE=1 and ADCAN1IE=1 |
| `adc1.c:846–847,1190,3192` | Both callbacks registered and called from their conversion interrupts |
| `adc1.c:167–169` | ADC_ISR_HZ=40000L, PLL_DECIM_N=8, FS_HZ=ADC_ISR_HZ/PLL_DECIM_N |
| `adc1.c:996–1182` | AN0 filtering/current controller runs each callback, no /8 gate |
| `adc1.c:1257–1265` | AN1 raw counters increment each callback; every eighth callback passes the gate to SOGI/PLL |
| `system.c:135–138` | Clock, ADC and PWM initialization called |

Configured nominal cadence: PWM **40 kHz**; AN0/controller **40 kHz**; AN1 raw **40 kHz**; SOGI/PLL **5 kHz**. The AN1 divider selects every eighth sample; it is not an eight-sample average. The existing clock interpretation is also documented in `audit_20260919/ADC_PLL_INVENTORY_REPORT.md`, timing table; its old postscaler=7 finding applies to the earlier source only.

## Current firmware contract audit

| Contract | Current source | Result |
|---|---|---|
| Boost A / B | adc1.c:73–83, [30010,2668] / [26000,-32000,9640] | Matches |
| Accumulator / shift | adc1.c:998,1135–1142, int64_t products and arithmetic >>15 | Matches; host shift checked in Step 4C |
| Correction clamp | BOOST_TEMP_LIMIT=2500, adc1.c:1144–1147 | -2500..2500 |
| Final Boost clamp | DUTY_BOOST_MAX=10625, adc1.c:1153–1156 | 0..10625 |
| Feedforward domain | pwm.c:515–524, (vref_cmd - Vdc)*12500/(vref_cmd + Vdc) | 12500 denominator domain |
| Anti-windup history | adc1.c:1162 | y1 = finallyDuty_boost - Boost_PWM, not merely limited correction |
| Buck behavior | adc1.c:1171–1178 | Feedforward only, controller history clear |
| ADC filter | adc1.c:1114; CURRENT_LOOP_StateClear at 757 | +=(adcVal-filter)>>2; clear function does not reset filter |
| Reference zero band | adc1.c:1091,1101 | -19..19 suppressed; +/-20 retained |
| Error deadband | adc1.c:1124 | -2..2 suppressed; +/-3 retained |
| Inherited voltage reference | pwm.c:304,334 onward | Vdc=574 and 334-entry LUT unchanged |
| Reference selection | parameter.h; adc1.c:1050 onward | PLL_CONTROL_ENABLE=1, GRID_TIE_REQUIRE_PLL_LOCK=0; locked AND phase-valid selects PLL reference, otherwise LUT |

Step 4C freshly exercises both reference paths using injected PLL outputs and actual LUT values. It does not validate the PLL itself or change its phase delivery. Physical voltage calibration remains **NOT FORMALLY RE-VALIDATED**; 574 is a voltage-reference-domain value, not a current ADC scale or current reference.

## SHA256 of audited current files

- adc1.c: `d51a6f9652382b3538d77fde841034193f60a9cc0bbd7f4a872f408d072998bf`
- pwm.c: `794a84fdd4ad22690855056f8a77c757964c5a3d8d14671c65b9b274eac8ada0`
- parameter.h: `019e68650f8dba5737a9649e3de8093c5c51681572da4f1bc8447b8dc627282f`
- clock.c: `20db992d0ac9ebbe085fb8cb687e9f07a50e3ee492e019e9999b936e657a015e`

Fresh Step 4C run `step4c_results/20261001T063216_353015Z/metrics.json` records these source hashes and verifies preservation after execution. Earlier status text claiming current ADTR1PS=7 / effective PLL 625 Hz is historical and superseded for these current hashes only.
