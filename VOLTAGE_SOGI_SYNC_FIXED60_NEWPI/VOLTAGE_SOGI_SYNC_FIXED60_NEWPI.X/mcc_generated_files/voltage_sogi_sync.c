/* Extracted only from BITEXACT_FAST active AN1 core; no power-control code.
 * Fixed60/new-PI isolated candidate; see parent REPORT.md and static.diff.
 * PLL step and angle geometry: degree Q21, NOT Q12 phase.
 */
#include "voltage_sogi_sync.h"
#include "sogi_coeff_lut.h"
#define AN0_ISR_HZ          40000L
#define AN1_ISR_HZ          5000L
#define PLL_FS_HZ           5000L
#if AN1_ISR_HZ != PLL_FS_HZ
#error Hardware_AN1_rate_must_match_PLL_rate
#endif
#define PLL_TABLE_SIZE      360L
#define PLL_THETA_SUBDIV    512L

#define PLL_STEP_Q          12
#define PLL_STEP_SCALE      (1L << PLL_STEP_Q)
#define PLL_FIXED_NCO_TEST 0
#define PLL_THETA_DEN_Q     (PLL_THETA_SUBDIV * PLL_STEP_SCALE)
#define PLL_THETA_DEN_SHIFT 21

#define PLL_WNOM_STEP_Q     ((int32_t)((60LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / PLL_FS_HZ))
#define PLL_WMIN_STEP_Q     ((int32_t)((45LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / PLL_FS_HZ))
#define PLL_WMAX_STEP_Q     ((int32_t)((75LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / PLL_FS_HZ))
#define PLL_W55_STEP_Q      ((int32_t)((55LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / PLL_FS_HZ))
#define PLL_W65_STEP_Q      ((int32_t)((65LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / PLL_FS_HZ))
#define PLL_W57P5_STEP_Q    ((int32_t)((575LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / (10LL * PLL_FS_HZ)))
#define PLL_W62P5_STEP_Q    ((int32_t)((625LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / (10LL * PLL_FS_HZ)))

#define PLL_LOCK_ERR_TH        80
#define PLL_PI_DEADBAND_LOCK   5
#define PLL_LOCK_ON_ERR_TH     170
#define PLL_LOCK_OFF_ERR_TH    300
#define PLL_LOCK_COUNT_TH      20
#define PLL_UNLOCK_COUNT_TH    100
#define PLL_GRID_PHASE_ON_ERR_TH   25
#define PLL_GRID_PHASE_OFF_ERR_TH  45
#define PLL_GRID_PHASE_ON_COUNT_TH 20
#define PLL_GRID_PHASE_OFF_COUNT_TH 50

#define PLL_KP_SHIFT_FAST      2
#define PLL_KI_SHIFT_FAST      13

#define PLL_KP_SHIFT_LOCK      5
#define PLL_KI_SHIFT_LOCK      15

#define PLL_INT_LIMIT \
    ((int32_t)((30LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / PLL_FS_HZ))
#define PLL_INT_LEAK_SHIFT     7

#define GRID_ADC_CENTER        2048
#define GRID_OFFSET_Q          12
#define GRID_OFFSET_SHIFT      12
#define PLL_LPF_SHIFT          6
#define PLL_STEP_SLEW_Q        ((int32_t)((100LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / ((int64_t)PLL_FS_HZ * PLL_FS_HZ)))
#define PLL_FREQ_DEADBAND_Q    ((int32_t)((1LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / (10LL * PLL_FS_HZ)))
#define PLL_TRIM_LIMIT_Q       ((int32_t)((15LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / PLL_FS_HZ))

#define PLL_VMAG_MIN           100
#define PLL_ERR_SCALE          1000
#define PLL_ERR_LIMIT          1000
#define PLL_ERR_SIGN           1
#define PLL_ERR_BIAS           0
#define PLL_INPUT_VALID_AMP_COUNTS 300
#define PLL_INPUT_AMP_WINDOW_SAMPLES 100

#define PLL_PHASE_COMP_DEG     4
#define PLL_OUTPUT_PHASE_COMP_DEG 0

#define SOGI_Q                 30
#define SOGI_STATE_Q           12
#include "pll_fast_recip_lut.h"
#define SOGI_VA_GAIN_SHIFT    0
#define SOGI_VB_GAIN_SHIFT    0


#define ADC_INTERNAL_DUAL_TEST         0
#define ADC_TEST_FREQ_HZ               60L
#define ADC_TEST_AN1_AMP_COUNTS        1860
#define ADC_TEST_AN0_AMP_COUNTS        300
#define ADC_TEST_STEP_Q                ((int32_t)((ADC_TEST_FREQ_HZ * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / PLL_FS_HZ))

#define SOGI55_A1_Q30    (-2042651804LL)
#define SOGI55_A2_Q30     (973794573LL)
#define SOGI55_B0_Q30     (49973625LL)
#define SOGI55_QB0_Q30    (1726965LL)
#define SOGI55_QB1_Q30    (3453929LL)
#define SOGI55_QB2_Q30    (1726965LL)

#define SOGI60_A1_Q30    (-2033145701LL)
#define SOGI60_A2_Q30     (965191209LL)
#define SOGI60_B0_Q30     (54275307LL)
#define SOGI60_QB0_Q30    (2046131LL)
#define SOGI60_QB1_Q30    (4092262LL)
#define SOGI60_QB2_Q30    (2046131LL)

#define SOGI65_A1_Q30    (-2023645686LL)
#define SOGI65_A2_Q30     (956665874LL)
#define SOGI65_B0_Q30     (58537975LL)
#define SOGI65_QB0_Q30    (2390732LL)
#define SOGI65_QB1_Q30    (4781464LL)
#define SOGI65_QB2_Q30    (2390732LL)

const int16_t SinTable360[360] = {
     0,   572,  1144,  1715,  2286,  2856,  3425,  3993,  4560,  5126,
  5690,  6252,  6813,  7371,  7927,  8481,  9032,  9580, 10126, 10668,
 11207, 11743, 12275, 12803, 13328, 13848, 14364, 14876, 15383, 15886,
 16383, 16876, 17364, 17846, 18323, 18794, 19260, 19720, 20173, 20621,
 21062, 21497, 21925, 22347, 22762, 23170, 23571, 23964, 24351, 24730,
 25101, 25465, 25821, 26169, 26509, 26841, 27165, 27481, 27788, 28087,
 28377, 28659, 28932, 29196, 29451, 29697, 29934, 30162, 30381, 30591,
 30791, 30982, 31163, 31335, 31498, 31650, 31794, 31927, 32051, 32165,
 32269, 32364, 32448, 32523, 32587, 32642, 32687, 32722, 32747, 32762,
 32767, 32762, 32747, 32722, 32687, 32642, 32587, 32523, 32448, 32364,
 32269, 32165, 32051, 31927, 31794, 31650, 31498, 31335, 31163, 30982,
 30791, 30591, 30381, 30162, 29934, 29697, 29451, 29196, 28932, 28659,
 28377, 28087, 27788, 27481, 27165, 26841, 26509, 26169, 25821, 25465,
 25101, 24730, 24351, 23964, 23571, 23170, 22762, 22347, 21925, 21497,
 21062, 20621, 20173, 19720, 19260, 18794, 18323, 17846, 17364, 16876,
 16383, 15886, 15383, 14876, 14364, 13848, 13328, 12803, 12275, 11743,
 11207, 10668, 10126,  9580,  9032,  8481,  7927,  7371,  6813,  6252,
  5690,  5126,  4560,  3993,  3425,  2856,  2286,  1715,  1144,   572,
     0,  -572, -1144, -1715, -2286, -2856, -3425, -3993, -4560, -5126,
 -5690, -6252, -6813, -7371, -7927, -8481, -9032, -9580,-10126,-10668,
-11207,-11743,-12275,-12803,-13328,-13848,-14364,-14876,-15383,-15886,
-16384,-16876,-17364,-17846,-18323,-18794,-19260,-19720,-20173,-20621,
-21062,-21497,-21925,-22347,-22762,-23170,-23571,-23964,-24351,-24730,
-25101,-25465,-25821,-26169,-26509,-26841,-27165,-27481,-27788,-28087,
-28377,-28659,-28932,-29196,-29451,-29697,-29934,-30162,-30381,-30591,
-30791,-30982,-31163,-31335,-31498,-31650,-31794,-31927,-32051,-32165,
-32269,-32364,-32448,-32523,-32587,-32642,-32687,-32722,-32747,-32762,
-32767,-32762,-32747,-32722,-32687,-32642,-32587,-32523,-32448,-32364,
-32269,-32165,-32051,-31927,-31794,-31650,-31498,-31335,-31163,-30982,
-30791,-30591,-30381,-30162,-29934,-29697,-29451,-29196,-28932,-28659,
-28377,-28087,-27788,-27481,-27165,-26841,-26509,-26169,-25821,-25465,
-25101,-24730,-24351,-23964,-23571,-23170,-22762,-22347,-21925,-21497,
-21062,-20621,-20173,-19720,-19260,-18794,-18323,-17846,-17364,-16876,
-16384,-15886,-15383,-14876,-14364,-13848,-13328,-12803,-12275,-11743,
-11207,-10668,-10126, -9580, -9032, -8481, -7927, -7371, -6813, -6252,
 -5690, -5126, -4560, -3993, -3425, -2856, -2286, -1715, -1144,  -572
};
volatile int16_t dbg_an1_raw      = 0;
volatile int16_t dbg_grid_v       = 0;
volatile int16_t dbg_pll_vq       = 0;
volatile int16_t dbg_pll_raw_err  = 0;
volatile int16_t dbg_pll_err      = 0;
volatile int16_t dbg_pll_err_slow = 0;
volatile int32_t dbg_pll_step     = 0;   /* integer step only */
volatile int32_t dbg_pll_step_q   = 0;   /* Q12 step */
volatile int16_t dbg_pll_freq_x10 = 0;
volatile int16_t dbg_zc_freq_x10 = 0;
volatile int16_t dbg_sogi_freq_x10 = 600;
volatile int16_t dbg_zc_offset = GRID_ADC_CENTER;
volatile int16_t dbg_zc_v = 0;
volatile uint16_t dbg_zc_period_5k = 0;
volatile int16_t dbg_pll_step_pct_x10 = 0;
volatile uint16_t dbg_an1_period_samples = 0;
volatile int16_t dbg_an1_amp_counts = 0;
volatile uint8_t dbg_an1_input_valid = 0;
volatile uint16_t dbg_an1_isr_counter = 0;
volatile uint16_t dbg_an1_rate_count = 0;
volatile uint16_t dbg_an1_rate_hz = 0;
volatile uint16_t dbg_an1_rate_duty_x10 = 0;
volatile uint16_t dbg_pll_idx     = 0;
volatile uint8_t dbg_pll_phase_ok = 0;
static int32_t sogi_va = 0;
static int32_t sogi_vb = 0;

static int32_t sogi_va_n1 = 0;
static int32_t sogi_va_n2 = 0;
static int32_t sogi_vb_n1 = 0;
static int32_t sogi_vb_n2 = 0;
static int32_t sogi_vin_n1 = 0;
static int32_t sogi_vin_n2 = 0;
static int32_t grid_offset = GRID_ADC_CENTER;
static int32_t grid_offset_q = ((int32_t)GRID_ADC_CENTER << GRID_OFFSET_Q);

static int64_t sogi_a1_q30 = SOGI60_A1_Q30;
static int64_t sogi_a2_q30 = SOGI60_A2_Q30;
static int64_t sogi_b0_q30 = SOGI60_B0_Q30;
static int64_t sogi_qb0_q30 = SOGI60_QB0_Q30;
static int64_t sogi_qb1_q30 = SOGI60_QB1_Q30;
static int64_t sogi_qb2_q30 = SOGI60_QB2_Q30;

/* =========================================================
   PLL states
   ========================================================= */

static int32_t pll_err_lpf = 0;
static int32_t pll_err_slow_lpf = 0;
static int32_t pll_err_slow_lpf_q8 = 0;
static int32_t pll_int_acc = 0;

static int32_t pll_step_q = PLL_WNOM_STEP_Q;
static int64_t theta_acc_q = 0;

static uint16_t pll_idx = 0;
static uint16_t pll_lock_count = 0;
static uint16_t pll_unlock_count = 0;
static uint16_t pll_phase_ok_count = 0;
static uint16_t pll_phase_bad_count = 0;


/* Single producer AN1 IPL1; single consumer PWM1 IPL3.
 * Write inactive slot, publish aligned 16-bit selector LAST. PWM1 suspends AN1,
 * so its snapshot cannot race slot reuse. No spin/seqlock that could deadlock
 * after preemption. No foreground readers of this interface are supported.
 * Generation wraps mod 65536; consumer runs each 25 us, crossings ~120/s.
 */
volatile uint8_t sogi_sync_valid = 0, sogi_grid_polarity = 0;
volatile uint8_t pll_locked = 0;
volatile uint16_t sogi_cross_generation = 0;
volatile uint32_t sogi_cross_count = 0, pll_update_count = 0;
volatile uint32_t accepted_cross_count = 0, valid_enter_count = 0, valid_exit_count = 0;
static volatile SogiSyncSnapshot sync_slots[2];
static volatile uint16_t sync_slot = 0;
static uint8_t sync_initialized = 0, sync_previous = 0;
static void SyncPublish(uint8_t valid, uint8_t polarity)
{
    uint16_t next = sync_slot ^ 1U;
    if (!valid) {
        if (sync_initialized) valid_exit_count++;
        sync_initialized = 0;
    } else if (!sync_initialized) {
        valid_enter_count++;
        sync_previous = polarity;
        sync_initialized = 1;
    } else if (polarity != sync_previous) {
        sync_previous = polarity;
        sogi_cross_generation++;
        sogi_cross_count++;
        accepted_cross_count++;
    }
    sogi_sync_valid = valid;
    sogi_grid_polarity = polarity;
    sync_slots[next].valid = valid;
    sync_slots[next].polarity = polarity;
    sync_slots[next].generation = sogi_cross_generation;
    __asm__ volatile ("" ::: "memory");
    sync_slot = next;
}
SogiSyncSnapshot VoltageSogiSync_Read(void)
{
    uint16_t slot = sync_slot;
    SogiSyncSnapshot result;
    result.valid = sync_slots[slot].valid;
    result.polarity = sync_slots[slot].polarity;
    result.generation = sync_slots[slot].generation;
    return result;
}

static int32_t PLL_Shift64(int64_t x, uint8_t q)
{
    if (x >= 0)
    {
        return (int32_t)(x >> q);
    }
    else
    {
        return -(int32_t)(((-x) + ((1LL << q) - 1LL)) >> q);
    }
}

static int32_t Q30_Round(int64_t x)
{
    if (x >= 0)
    {
        return (int32_t)((x + (1LL << 29)) >> 30);
    }
    else
    {
        return -(int32_t)(((-x) + (1LL << 29)) >> 30);
    }
}

static int32_t Q15_Round(int64_t x)
{
    if (x >= 0)
    {
        return (int32_t)((x + (1LL << 14)) >> 15);
    }
    else
    {
        return -(int32_t)(((-x) + (1LL << 14)) >> 15);
    }
}

static int32_t PLL_SinInterp(uint16_t idx, int64_t theta_frac_q)
{
    uint16_t idx1;
    int32_t y0;
    int32_t y1;
    int32_t dy;
    int64_t den;
    int64_t interp;

    idx1 = idx + 1;

    if (idx1 >= PLL_TABLE_SIZE)
        idx1 = 0;

    y0 = (int32_t)SinTable360[idx];
    y1 = (int32_t)SinTable360[idx1];

    dy = y1 - y0;

    den = PLL_THETA_DEN_Q;

    if (dy >= 0)
    {
        interp = (int32_t)((((int64_t)dy * theta_frac_q) + (den / 2)) >> 21);
    }
    else
    {
        interp = -(int32_t)((((int64_t)(-dy) * theta_frac_q) + (den / 2)) >> 21);
    }

    return y0 + (int32_t)interp;
}

static int32_t PLL_NormalizeExact(int32_t vq_q, int32_t vmag_q)
{
    uint32_t d, z, r, scaled_product, magnitude;
    uint16_t e = 0;
    uint64_t n, low_product, high_product;
    uint32_t quotient;
    int64_t remainder;

    if ((vmag_q < 409600L) || (vmag_q > 10240000L))
        return (int32_t)(((int64_t)PLL_ERR_SIGN * vq_q * PLL_ERR_SCALE) / vmag_q);

    d = (uint32_t)vmag_q;
    z = d;
    /* Integer bit length, bounded to 18..23 for this denominator domain. */
    if (z >= 65536UL) { z >>= 16; e += 16; }
    if (z >= 256UL)   { z >>= 8;  e += 8; }
    if (z >= 16UL)    { z >>= 4;  e += 4; }
    if (z >= 4UL)     { z >>= 2;  e += 2; }
    if (z >= 2UL)    { e += 1; }
    z = d << (24U - e);
    r = pll_recip_q24[(uint16_t)((z - 16777216UL) >> 16)];
    scaled_product = (uint32_t)(((uint64_t)z * r) >> 24);
    r = (uint32_t)(((uint64_t)r * (33554432UL - scaled_product)) >> 24);

    magnitude = (vq_q < 0) ? (uint32_t)(-(int64_t)vq_q) : (uint32_t)vq_q;
    n = (uint64_t)magnitude * 1000ULL;
    low_product = (uint64_t)(uint32_t)n * r;
    high_product = (n >> 32) * r;
    quotient = (uint32_t)((high_product + (low_product >> 32)) >> (e - 8U));
    remainder = (int64_t)n - (int64_t)((uint64_t)quotient * d);
    while (remainder >= (int64_t)d) { quotient++; remainder -= d; }
    while (remainder < 0) { quotient--; remainder += d; }
    return (vq_q < 0) ? -(int32_t)quotient : (int32_t)quotient;
}

static int16_t PLL_StepToFreqX10(int32_t step_q)
{
    /* Exact over PLL_WMIN_STEP_Q..PLL_WMAX_STEP_Q (45..75 Hz).
       floor(step*50000/754974720) = floor(floor(step*3125/2^20)/45). */
    uint32_t x = (uint32_t)(((uint64_t)step_q * 3125ULL) >> 20);
    return (int16_t)((x * 11651UL) >> 19);
}

static uint16_t PLL_AddDegrees(uint16_t idx, int16_t degrees)
{
    int16_t idx_tmp;

    idx_tmp = (int16_t)idx + degrees;

    while (idx_tmp < 0)
        idx_tmp += PLL_TABLE_SIZE;

    while (idx_tmp >= PLL_TABLE_SIZE)
        idx_tmp -= PLL_TABLE_SIZE;

    return (uint16_t)idx_tmp;
}

/* Fixed existing 60.0 Hz row: (600 - 550) = 50. No frequency mapping. */
#define SOGI_FIXED60_LUT_INDEX 50U
static void SOGI_LoadCoeffFixed60(void)
{
    dbg_sogi_freq_x10 = 600;
    sogi_a1_q30 = sogi_coeff_lut[SOGI_FIXED60_LUT_INDEX][0];
    sogi_a2_q30 = sogi_coeff_lut[SOGI_FIXED60_LUT_INDEX][1];
    sogi_b0_q30 = sogi_coeff_lut[SOGI_FIXED60_LUT_INDEX][2];
    sogi_qb0_q30 = sogi_coeff_lut[SOGI_FIXED60_LUT_INDEX][3];
    sogi_qb1_q30 = sogi_coeff_lut[SOGI_FIXED60_LUT_INDEX][4];
    sogi_qb2_q30 = sogi_qb0_q30;
}

/* e=error/1000; S=sum(error); Ts=1/5000.
 * delta_step=(30*e + 300*Ts*S/1000)*(180/pi)*2^21/5000.
 * Q24 gains: round(720.9477515845177*2^24),
 *            round(1.4418955031690353*2^24).
 * Keep original S accumulator/clamp. Truncate combined result toward zero,
 * exactly matching offline E; explicit shifts avoid runtime 64-bit division.
 */
#define PLL_NEW_KP_STEP_Q24 12095496153LL
#define PLL_NEW_KI_STEP_Q24 24190992LL
static int32_t PLL_NewPIToStep(int32_t error, int32_t sum)
{
    int64_t value = (int64_t)error * PLL_NEW_KP_STEP_Q24 +
                    (int64_t)sum * PLL_NEW_KI_STEP_Q24;
    if (value >= 0)
        return (int32_t)(value >> 24);
    return -(int32_t)((-value) >> 24);
}


void VoltageSogiSync_Update(uint16_t adcVal)
{
    static int32_t an1_amp_min = 0;
    static int32_t an1_amp_max = 0;
    static int32_t an1_amp_counts = 0;
    static uint16_t an1_amp_cnt = 0;
    static uint8_t an1_signal_valid = 0;
    static uint16_t pll_debug_slow_cnt = 0;

    int32_t v_in_count;
    int32_t v_in_q;
    int64_t acc_va;
    int64_t acc_vb;
    int32_t va_new;
    int32_t vb_new;
    int32_t sin_theta;
    int32_t cos_theta;
    int32_t vq_q;
    int32_t raw_err;
    int32_t abs_va;
    int32_t abs_vb;
    int32_t vmag_q;
    int32_t vmag_min_q;
    int32_t abs_err_lpf;
    int32_t abs_err_phase;
    int32_t pll_err_pi;
    int32_t w_delta_q;
    int32_t pll_step_target_q;
#if (PLL_FIXED_NCO_TEST == 0)
    int32_t pll_step_diff_q;
#endif
#if (PLL_FIXED_NCO_TEST != 0)
    int32_t leak;
#endif
    int32_t mag_max;
    int32_t mag_min;
    uint16_t step_degrees;
    uint16_t theta_det_idx;
    uint16_t theta_output_idx;

    pll_update_count++;
    dbg_an1_isr_counter++;
    dbg_an1_rate_count++;

    // AN1 hardware trigger is 5 kHz; no software decimation.

    dbg_an1_raw = (int16_t)adcVal;

    {
        int32_t grid_offset_err_q;
        int32_t grid_offset_delta_q;

        grid_offset_err_q = ((int32_t)adcVal << GRID_OFFSET_Q) - grid_offset_q;

        if (grid_offset_err_q >= 0)
            grid_offset_delta_q = grid_offset_err_q >> GRID_OFFSET_SHIFT;
        else
            grid_offset_delta_q = -((-grid_offset_err_q) >> GRID_OFFSET_SHIFT);

        grid_offset_q += grid_offset_delta_q;
        grid_offset = (grid_offset_q + (1L << (GRID_OFFSET_Q - 1))) >> GRID_OFFSET_Q;
    }

    v_in_count = (int32_t)adcVal - grid_offset;
    dbg_grid_v = (int16_t)v_in_count;
    dbg_zc_offset = (int16_t)grid_offset;

    if (an1_amp_cnt == 0)
    {
        an1_amp_min = v_in_count;
        an1_amp_max = v_in_count;
    }
    else
    {
        if (v_in_count < an1_amp_min)
            an1_amp_min = v_in_count;

        if (v_in_count > an1_amp_max)
            an1_amp_max = v_in_count;
    }

    an1_amp_cnt++;

    if (an1_amp_cnt >= PLL_INPUT_AMP_WINDOW_SAMPLES)
    {
        an1_amp_counts = (an1_amp_max - an1_amp_min) / 2;
        an1_signal_valid = (an1_amp_counts >= PLL_INPUT_VALID_AMP_COUNTS) ? 1U : 0U;
        dbg_an1_amp_counts = (int16_t)an1_amp_counts;
        dbg_an1_input_valid = an1_signal_valid;
        an1_amp_cnt = 0;
    }

    if ((an1_signal_valid == 0) && (PLL_FIXED_NCO_TEST == 0))
    {
        pll_locked = 0;
        dbg_pll_phase_ok = 0;
        pll_lock_count = 0;
        pll_unlock_count = 0;
        pll_phase_ok_count = 0;
        pll_phase_bad_count = 0;
        pll_err_lpf = 0;
        pll_err_slow_lpf = 0;
        pll_err_slow_lpf_q8 = 0;
        pll_int_acc = 0;
        pll_step_q = PLL_WNOM_STEP_Q;
        theta_acc_q = 0;
        pll_idx = 0;

        sogi_va = 0;
        sogi_vb = 0;
        sogi_va_n1 = 0;
        sogi_va_n2 = 0;
        sogi_vb_n1 = 0;
        sogi_vb_n2 = 0;
        sogi_vin_n1 = 0;
        sogi_vin_n2 = 0;

        SyncPublish(0U, 1U);

        return;
    }

    SOGI_LoadCoeffFixed60();

    v_in_q = v_in_count * (1L << SOGI_STATE_Q);

    acc_va = 0;
    acc_va += sogi_b0_q30 * v_in_q;
    acc_va -= sogi_b0_q30 * sogi_vin_n2;
    acc_va -= sogi_a1_q30 * sogi_va_n1;
    acc_va -= sogi_a2_q30 * sogi_va_n2;

    acc_vb = 0;
    acc_vb += sogi_qb0_q30 * v_in_q;
    acc_vb += sogi_qb1_q30 * sogi_vin_n1;
    acc_vb += sogi_qb2_q30 * sogi_vin_n2;
    acc_vb -= sogi_a1_q30 * sogi_vb_n1;
    acc_vb -= sogi_a2_q30 * sogi_vb_n2;

    va_new = Q30_Round(acc_va);
    vb_new = Q30_Round(acc_vb);

    sogi_va = va_new;
    sogi_vb = vb_new;
    sogi_vin_n2 = sogi_vin_n1;
    sogi_vin_n1 = v_in_q;
    sogi_va_n2 = sogi_va_n1;
    sogi_va_n1 = va_new;
    sogi_vb_n2 = sogi_vb_n1;
    sogi_vb_n1 = vb_new;

    theta_det_idx = PLL_AddDegrees(pll_idx, PLL_PHASE_COMP_DEG);
    sin_theta = PLL_SinInterp(theta_det_idx, theta_acc_q);
    cos_theta = PLL_SinInterp(PLL_AddDegrees(theta_det_idx, 90), theta_acc_q);

    vq_q = Q15_Round(((int64_t)sogi_va * cos_theta) +
                     ((int64_t)sogi_vb * sin_theta));
    dbg_pll_vq = (int16_t)PLL_Shift64(vq_q, SOGI_STATE_Q);

    abs_va = (sogi_va >= 0) ? sogi_va : -sogi_va;
    abs_vb = (sogi_vb >= 0) ? sogi_vb : -sogi_vb;

    if (abs_va >= abs_vb)
    {
        mag_max = abs_va;
        mag_min = abs_vb;
    }
    else
    {
        mag_max = abs_vb;
        mag_min = abs_va;
    }

    vmag_q = mag_max + (mag_min >> 1);
    vmag_min_q = PLL_VMAG_MIN * (1L << SOGI_STATE_Q);

    if (vmag_q < vmag_min_q)
        vmag_q = vmag_min_q;

    raw_err = PLL_NormalizeExact(vq_q, vmag_q);
    raw_err -= PLL_ERR_BIAS;

    if (raw_err > PLL_ERR_LIMIT)
        raw_err = PLL_ERR_LIMIT;
    else if (raw_err < -PLL_ERR_LIMIT)
        raw_err = -PLL_ERR_LIMIT;

    dbg_pll_raw_err = (int16_t)raw_err;
    pll_err_lpf = pll_err_lpf + ((raw_err - pll_err_lpf) >> PLL_LPF_SHIFT);
    dbg_pll_err = (int16_t)pll_err_lpf;

    pll_err_slow_lpf_q8 += ((((int32_t)pll_err_lpf << 8) - pll_err_slow_lpf_q8) >> 5);

    if (pll_err_slow_lpf_q8 >= 0)
        pll_err_slow_lpf = (pll_err_slow_lpf_q8 + 128) >> 8;
    else
        pll_err_slow_lpf = -(((-pll_err_slow_lpf_q8) + 128) >> 8);

    dbg_pll_err_slow = (int16_t)pll_err_slow_lpf;
    abs_err_lpf = (pll_err_lpf >= 0) ? pll_err_lpf : -pll_err_lpf;
    abs_err_phase = (pll_err_slow_lpf >= 0) ? pll_err_slow_lpf : -pll_err_slow_lpf;

    if (abs_err_phase <= PLL_GRID_PHASE_ON_ERR_TH)
    {
        if (pll_phase_ok_count < PLL_GRID_PHASE_ON_COUNT_TH)
            pll_phase_ok_count++;

        pll_phase_bad_count = 0;

        if (pll_phase_ok_count >= PLL_GRID_PHASE_ON_COUNT_TH)
            dbg_pll_phase_ok = 1;
    }
    else if (abs_err_phase > PLL_GRID_PHASE_OFF_ERR_TH)
    {
        pll_phase_ok_count = 0;

        if (pll_phase_bad_count < PLL_GRID_PHASE_OFF_COUNT_TH)
            pll_phase_bad_count++;

        if (pll_phase_bad_count >= PLL_GRID_PHASE_OFF_COUNT_TH)
        {
            pll_phase_bad_count = 0;
            dbg_pll_phase_ok = 0;
        }
    }

    pll_err_pi = pll_err_lpf;

    if ((abs_err_lpf <= PLL_LOCK_ERR_TH) &&
        (pll_err_pi <= PLL_PI_DEADBAND_LOCK) &&
        (pll_err_pi >= -PLL_PI_DEADBAND_LOCK))
    {
        pll_err_pi = 0;
    }

    if ((pll_err_pi == 0) && (abs_err_lpf <= PLL_LOCK_ERR_TH))
    {
#if (PLL_FIXED_NCO_TEST != 0)
        if (pll_int_acc > 0)
        {
            leak = PLL_Shift64(pll_int_acc, PLL_INT_LEAK_SHIFT);
            if (leak < 1)
                leak = 1;
            pll_int_acc -= leak;
            if (pll_int_acc < 0)
                pll_int_acc = 0;
        }
        else if (pll_int_acc < 0)
        {
            leak = PLL_Shift64(-pll_int_acc, PLL_INT_LEAK_SHIFT);
            if (leak < 1)
                leak = 1;
            pll_int_acc += leak;
            if (pll_int_acc > 0)
                pll_int_acc = 0;
        }
#endif
    }
    else
    {
        pll_int_acc += pll_err_pi;
    }

    if (pll_int_acc > PLL_INT_LIMIT)
        pll_int_acc = PLL_INT_LIMIT;
    else if (pll_int_acc < -PLL_INT_LIMIT)
        pll_int_acc = -PLL_INT_LIMIT;

    w_delta_q = PLL_NewPIToStep(pll_err_pi, pll_int_acc);

    if (w_delta_q > PLL_TRIM_LIMIT_Q)
        w_delta_q = PLL_TRIM_LIMIT_Q;
    else if (w_delta_q < -PLL_TRIM_LIMIT_Q)
        w_delta_q = -PLL_TRIM_LIMIT_Q;

    pll_step_target_q = PLL_WNOM_STEP_Q + w_delta_q;

    if (pll_step_target_q > PLL_WMAX_STEP_Q)
        pll_step_target_q = PLL_WMAX_STEP_Q;
    else if (pll_step_target_q < PLL_WMIN_STEP_Q)
        pll_step_target_q = PLL_WMIN_STEP_Q;

#if (PLL_FIXED_NCO_TEST != 0)
    pll_int_acc = 0;
    pll_step_q = PLL_WNOM_STEP_Q;
#else
    pll_step_diff_q = pll_step_target_q - pll_step_q;

    if ((pll_step_diff_q <= PLL_FREQ_DEADBAND_Q) &&
        (pll_step_diff_q >= -PLL_FREQ_DEADBAND_Q))
    {
        pll_step_diff_q = 0;
    }

    if (pll_step_diff_q > PLL_STEP_SLEW_Q)
        pll_step_diff_q = PLL_STEP_SLEW_Q;
    else if (pll_step_diff_q < -PLL_STEP_SLEW_Q)
        pll_step_diff_q = -PLL_STEP_SLEW_Q;

    pll_step_q += pll_step_diff_q;
#endif

    theta_acc_q += pll_step_q;
    step_degrees = (uint16_t)(theta_acc_q >> PLL_THETA_DEN_SHIFT);
    theta_acc_q -= ((int64_t)step_degrees << PLL_THETA_DEN_SHIFT);

    pll_idx += step_degrees;
    if (pll_idx >= PLL_TABLE_SIZE)
        pll_idx -= PLL_TABLE_SIZE;

    dbg_pll_idx = pll_idx;
    dbg_pll_step = PLL_Shift64(pll_step_q, PLL_STEP_Q);
    dbg_pll_step_q = pll_step_q;
    // Frequency debug is refreshed in the 32-sample slow block below.

    if ((an1_signal_valid != 0) && (abs_err_lpf <= PLL_LOCK_ON_ERR_TH))
    {
        if (pll_lock_count < PLL_LOCK_COUNT_TH)
            pll_lock_count++;

        pll_unlock_count = 0;

        if (pll_lock_count >= PLL_LOCK_COUNT_TH)
            pll_locked = 1;
    }
    else if ((an1_signal_valid == 0) || (abs_err_lpf > PLL_LOCK_OFF_ERR_TH))
    {
        pll_lock_count = 0;

        if (pll_locked != 0)
        {
            if (pll_unlock_count < PLL_UNLOCK_COUNT_TH)
                pll_unlock_count++;

            if (pll_unlock_count >= PLL_UNLOCK_COUNT_TH)
            {
                pll_unlock_count = 0;
                pll_locked = 0;
            }
        }
        else
        {
            pll_unlock_count = 0;
            pll_locked = 0;
        }
    }

    theta_output_idx = PLL_AddDegrees(pll_idx, PLL_OUTPUT_PHASE_COMP_DEG);
    SyncPublish((uint8_t)((an1_signal_valid != 0) && (pll_locked != 0) &&
                          (dbg_pll_phase_ok != 0)),
                (uint8_t)(theta_output_idx < 180U));

    pll_debug_slow_cnt++;
    if (pll_debug_slow_cnt >= 32)
    {
        pll_debug_slow_cnt = 0;
    dbg_pll_freq_x10 = PLL_StepToFreqX10(pll_step_q);
    }

}
