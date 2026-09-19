/**
  ADC1 Generated Driver File
  Device: dsPIC33CK256MP506
*/

#include "adc1.h"
#include "parameter.h"

/* =========================================================
   Three-point PLL/SOGI test override
   ========================================================= */

#ifndef PLL_CONTROL_ENABLE
#define PLL_CONTROL_ENABLE 0
#endif

/* =========================================================
   Callback function pointers
   ========================================================= */

static void (*ADC1_CommonDefaultInterruptHandler)(void);
static void (*ADC1_channel_AN24DefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_channel_AN25DefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_channel_AN0DefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_channel_AN1DefaultInterruptHandler)(uint16_t adcVal);

/* =========================================================
   External variables from pwm.c
   ========================================================= */

extern unsigned short phase;
extern unsigned short i;

extern const int16_t Vdc;
extern const int16_t VrefTable[334];

extern volatile power_mode_t power_mode;

extern volatile int32_t Buck_PWM;
extern volatile int32_t Boost_PWM;

extern volatile int16_t finallyDuty_buck;
extern volatile int16_t finallyDuty_boost;

/* =========================================================
   Basic constants
   ========================================================= */

#define ADC_CENTER          2048
#define VREF_PEAK           3787

#define DUTY_BUCK_MAX       12500
#define DUTY_BOOST_MAX      10625

#define ERR_DEADBAND        3
#define ADC_FILT_SHIFT      2

#define IREF_PEAK_COUNTS    925
#define IREF_ZERO_BAND      20
#define IREF_FROM_VREF_Q    15
#define IREF_FROM_VREF_GAIN_Q \
    ((int32_t)((((int64_t)IREF_PEAK_COUNTS << IREF_FROM_VREF_Q) + (VREF_PEAK / 2)) / VREF_PEAK))

#define BOOST_TEMP_LIMIT    2500
#define MODE_UNKNOWN        255

/* =========================================================
   Boost current-loop 2P2Z coefficients
   ========================================================= */

const int32_t A_Coefficient_BOOST[2] = {
    30010,
     2668
};

const int32_t B_Coefficient_BOOST[3] = {
     26000,
    -32000,
      9640
};

/* Buck coefficients reserved. Buck mode is open-loop now. */
const int32_t A_Coefficient_BUCK[2] = {
    -53039,
     20271
};

const int32_t B_Coefficient_BUCK[3] = {
      6650,
    -11592,
      5006
};

/* =========================================================
   Current-loop states
   ========================================================= */

static int32_t adc0_filt = ADC_CENTER;

static int32_t duty_boost[2] = {0, 0};
static int16_t err_boost[3]  = {0, 0, 0};

static int32_t duty_buck[2] = {0, 0};
static int16_t err_buck[3]  = {0, 0, 0};

static uint8_t last_current_mode = MODE_UNKNOWN;

/* =========================================================
   Current-loop debug variables
   ========================================================= */

volatile int16_t dbg_adc0_raw   = 0;
volatile int16_t dbg_adc0_filt  = 0;
volatile int16_t dbg_iref_cmd   = 0;
volatile int16_t dbg_ierr       = 0;
volatile int32_t dbg_temp       = 0;
volatile int32_t dbg_mode       = 0;
volatile int32_t dbg_mode_sw    = 0;

/* =========================================================
   PLL sine table, Q15
   ========================================================= */

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
/* =========================================================
   SOGI-PLL parameters
   AN1 = AC Source / PCC voltage feedback
   ========================================================= */

#define ADC_ISR_HZ          40000L
#define PLL_DECIM_N         8
#define FS_HZ               (ADC_ISR_HZ / PLL_DECIM_N)
#define PLL_TABLE_SIZE      360L
#define PLL_THETA_SUBDIV    512L
#define PLL_VREF_TABLE_LEN  334L
#define PLL_VREF_FULL_LEN   (2L * PLL_VREF_TABLE_LEN)

#define PLL_STEP_Q          12
#define PLL_STEP_SCALE      (1L << PLL_STEP_Q)
#define PLL_FIXED_NCO_TEST 0
#define PLL_THETA_DEN_Q     (PLL_THETA_SUBDIV * PLL_STEP_SCALE)
#define PLL_THETA_DEN_SHIFT 21

#define PLL_WNOM_STEP_Q     ((int32_t)((60LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))
#define PLL_WMIN_STEP_Q     ((int32_t)((45LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))
#define PLL_WMAX_STEP_Q     ((int32_t)((75LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))
#define PLL_W55_STEP_Q      ((int32_t)((55LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))
#define PLL_W65_STEP_Q      ((int32_t)((65LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))
#define PLL_W57P5_STEP_Q    ((int32_t)((575LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / (10LL * FS_HZ)))
#define PLL_W62P5_STEP_Q    ((int32_t)((625LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / (10LL * FS_HZ)))

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
    ((int32_t)((30LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))
#define PLL_INT_LEAK_SHIFT     7

#define GRID_ADC_CENTER        2048
#define GRID_OFFSET_Q          12
#define GRID_OFFSET_SHIFT      12
#define PLL_LPF_SHIFT          6
#define PLL_FF_STEP_LPF_SHIFT  4
#define PLL_STEP_SLEW_Q        ((int32_t)((100LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / ((int64_t)FS_HZ * FS_HZ)))
#define PLL_FREQ_DEADBAND_Q    ((int32_t)((1LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / (10LL * FS_HZ)))
#define PLL_TRIM_LIMIT_Q       ((int32_t)((15LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))
#define PLL_LOCK_TRIM_LIMIT_Q  ((int32_t)((2LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))
#define PLL_USE_ZC_FEED_FORWARD 0
#define PLL_60HZ_LOCK_MIN_X10   590
#define PLL_60HZ_LOCK_MAX_X10   610
#define PLL_60HZ_ZC_LOCK_COUNT  3U
#define AN1_PERIOD_AVG_CYCLES  4

#define PLL_VMAG_MIN           100
#define PLL_ERR_SCALE          1000
#define PLL_ERR_LIMIT          1000
#define PLL_ERR_SIGN           1
#define PLL_ERR_BIAS           0
#define PLL_INPUT_VALID_AMP_COUNTS 300
#define PLL_INPUT_AMP_WINDOW_SAMPLES 100
#define AN1_ZC_HYS_COUNTS        40
#define AN1_ZC_FILTER_SHIFT      2
#define AN1_ZC_HYS_MAX_COUNTS    300
#define AN1_ZC_PERIOD_MIN_5K     60U
#define AN1_ZC_PERIOD_MAX_5K     125U
#define AN1_ZC_COUNTER_MAX_5K    200U
#define AN1_ZC_MIN_PERIOD_SAMPLES AN1_ZC_PERIOD_MIN_5K
#define AN1_ZC_MAX_PERIOD_SAMPLES AN1_ZC_PERIOD_MAX_5K
#define AN1_FREQ_WINDOW_SAMPLES 1000U
#define PLL_SYNC_TO_AN1_ZC     0
#define PLL_DEMO_LOCK_ON_ZC    0
#define PLL_ZC_ONLY_DEBUG      1

#define PLL_PHASE_COMP_DEG     4
#define PLL_OUTPUT_PHASE_COMP_DEG 0

/* =========================================================
   55 / 60 / 65 Hz discrete test mode

   Use this mode when the continuous adaptive tracking path is not stable
   enough for bench testing. Build three firmware images by changing only
   PLL_TEST_FREQ_MODE to 55, 60, or 65.

   PLL_THREE_POINT_TEST_ENABLE = 1:
       SOGI coefficients and PLL base step are fixed by PLL_TEST_FREQ_MODE.

   PLL_TEST_FORCE_LOCK = 1:
       force pll_locked after AN1 input is valid, so AN0 current loop uses
       pll_vref_count / i_ref_count. Set to 0 later for true lock evaluation.

   PLL_TEST_FORCE_PLL_OUTPUT = 1:
       let AN0 use PLL reference even if parameter.h has PLL_CONTROL_ENABLE = 0.
   ========================================================= */
#define PLL_THREE_POINT_TEST_ENABLE   0
#define PLL_TEST_FREQ_MODE            60
#define PLL_TEST_FORCE_LOCK           0
#define PLL_TEST_FORCE_PLL_OUTPUT     0

#define PLL_OUTPUT_USE_PLL \
    ((((PLL_TEST_FORCE_PLL_OUTPUT) != 0) || ((PLL_CONTROL_ENABLE) != 0)) && \
     (pll_locked != 0) && (dbg_pll_phase_ok != 0))

/* =========================================================
   Tustin SOGI fixed coefficients
   fs = 5kHz, fg = 55/60/65Hz, k = 1.414
   Q30 format
   ========================================================= */

#define SOGI_Q                 30
#define SOGI_STATE_Q           12
#define SOGI_VA_GAIN_SHIFT    0
#define SOGI_VB_GAIN_SHIFT    0

#define SOGI_USE_INTERNAL_TEST_SINE   0
#define SOGI_TEST_FREQ_HZ             60L
#define SOGI_TEST_AMP_COUNTS          1860
#define SOGI_TEST_STEP_Q              ((int32_t)((SOGI_TEST_FREQ_HZ * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))

#define ADC_INTERNAL_DUAL_TEST         0
#define ADC_TEST_FREQ_HZ               60L
#define ADC_TEST_AN1_AMP_COUNTS        1860
#define ADC_TEST_AN0_AMP_COUNTS        300
#define ADC_TEST_STEP_Q                ((int32_t)((ADC_TEST_FREQ_HZ * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))

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

/* =========================================================
   PLL debug variables
   ========================================================= */

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
volatile uint16_t dbg_pll_vref_idx = 0;
volatile int16_t dbg_test_an0_count = 0;
volatile int16_t dbg_test_an1_count = 0;
volatile int16_t dbg_test_sync_err  = 0;
volatile uint8_t dbg_pll_phase_ok = 0;

extern volatile uint8_t pll_locked;
extern volatile int16_t i_ref_count;
extern volatile int16_t pll_vref_count;
extern volatile uint8_t pll_sync_phase;

/* =========================================================
   PG7 debug selector
   ========================================================= */

#define PLL_DBG_CONST        0
#define PLL_DBG_AN1          1
#define PLL_DBG_SOGI_VA      2
#define PLL_DBG_SOGI_VB      3
#define PLL_DBG_VQ           4
#define PLL_DBG_RAW_ERR      5
#define PLL_DBG_ERR_LPF      6
#define PLL_DBG_STEP_Q       7
#define PLL_DBG_IREF         8
#define PLL_DBG_LOCK         9
#define PLL_DBG_AN1_AMP      10
#define PLL_DBG_VA_AMP       11
#define PLL_DBG_VB_AMP       12
#define PLL_DBG_AN1_PERIOD   13
#define PLL_DBG_TEST_AN0     14
#define PLL_DBG_TEST_AN1     15
#define PLL_DBG_TEST_SYNC    16
#define PLL_DBG_LOCK_BUZZ    17
#define PLL_DBG_INPUT_VALID  18
#define PLL_DBG_AN1_ISR_TOGGLE 19   
#define PLL_DBG_AN1_RATE_DUTY  20
#define PLL_DBG_OUTPUT_FREQ    21
#define PLL_DBG_ZC_FREQ        22
#define PLL_DBG_ZC_TOGGLE      23
#define PLL_DBG_ZC_OFFSET      24
#define PLL_DBG_ZC_V           25
#define PLL_DBG_ZC_PERIOD_5K  26
#define PLL_DBG_PHASE_OK       27
#define PLL_DBG_VREF_IDX       28
#define PLL_DBG_ERR_FINE       29
#define PLL_DBG_ERR_FINE_SLOW  30
#define PLL_DBG_THETA_PHASE    31

#define PLL_DEBUG_SELECT  PLL_DBG_LOCK  
/* =========================================================
   SOGI states
   All SOGI states are Q12 counts.
   Example:
   actual counts = sogi_va >> SOGI_STATE_Q
   ========================================================= */

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
static int32_t pll_ff_step_q = PLL_WNOM_STEP_Q;
static int32_t pll_ff_step_target_q = PLL_WNOM_STEP_Q;
static int64_t theta_acc_q = 0;

static uint16_t pll_idx = 0;
static uint16_t pll_lock_count = 0;
static uint16_t pll_unlock_count = 0;
static uint16_t pll_phase_ok_count = 0;
static uint16_t pll_phase_bad_count = 0;

static uint16_t adc_test_an0_idx = 0;
static uint16_t adc_test_an1_idx = 0;
static int64_t adc_test_an0_theta_acc_q = 0;
static int64_t adc_test_an1_theta_acc_q = 0;

/* =========================================================
   Fixed-point helpers
   ========================================================= */

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

/*
   Linear interpolation for SinTable360.

   idx:
       0~359 degree index

   theta_frac_q:
       fractional position inside current 1-degree interval.
       range: 0 ~ (PLL_THETA_SUBDIV << PLL_STEP_Q)

   return:
       Q15 sine value.
*/
static int32_t PLL_SinLookup(uint16_t idx)
{
    if (idx >= PLL_TABLE_SIZE)
        idx -= PLL_TABLE_SIZE;

    return SinTable360[idx];
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


/* =========================================================
   Three-point test helpers
   ========================================================= */
static int32_t PLL_GetTestBaseStepQ(void)
{
#if (PLL_TEST_FREQ_MODE == 55)
    return PLL_W55_STEP_Q;
#elif (PLL_TEST_FREQ_MODE == 65)
    return PLL_W65_STEP_Q;
#else
    return PLL_WNOM_STEP_Q;
#endif
}

static int16_t PLL_GetTestFreqX10(void)
{
#if (PLL_TEST_FREQ_MODE == 55)
    return 550;
#elif (PLL_TEST_FREQ_MODE == 65)
    return 650;
#else
    return 600;
#endif
}

static void SOGI_LoadTestFreqCoeff(void)
{
#if (PLL_TEST_FREQ_MODE == 55)
    sogi_a1_q30  = SOGI55_A1_Q30;
    sogi_a2_q30  = SOGI55_A2_Q30;
    sogi_b0_q30  = SOGI55_B0_Q30;
    sogi_qb0_q30 = SOGI55_QB0_Q30;
    sogi_qb1_q30 = SOGI55_QB1_Q30;
    sogi_qb2_q30 = SOGI55_QB2_Q30;
#elif (PLL_TEST_FREQ_MODE == 65)
    sogi_a1_q30  = SOGI65_A1_Q30;
    sogi_a2_q30  = SOGI65_A2_Q30;
    sogi_b0_q30  = SOGI65_B0_Q30;
    sogi_qb0_q30 = SOGI65_QB0_Q30;
    sogi_qb1_q30 = SOGI65_QB1_Q30;
    sogi_qb2_q30 = SOGI65_QB2_Q30;
#else
    sogi_a1_q30  = SOGI60_A1_Q30;
    sogi_a2_q30  = SOGI60_A2_Q30;
    sogi_b0_q30  = SOGI60_B0_Q30;
    sogi_qb0_q30 = SOGI60_QB0_Q30;
    sogi_qb1_q30 = SOGI60_QB1_Q30;
    sogi_qb2_q30 = SOGI60_QB2_Q30;
#endif
}

static int16_t PLL_StepToFreqX10(int32_t step_q)
{
    return (int16_t)(((int64_t)step_q * FS_HZ * 10LL) /
                     ((int64_t)PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE));
}

static int32_t PLL_FreqX10ToStepQ(int32_t freq_x10)
{
    int64_t step_q;

    if (freq_x10 < 450)
        freq_x10 = 450;
    else if (freq_x10 > 750)
        freq_x10 = 750;

    step_q = ((int64_t)freq_x10 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV *
              PLL_STEP_SCALE) / ((int64_t)FS_HZ * 10LL);

    if (step_q > PLL_WMAX_STEP_Q)
        step_q = PLL_WMAX_STEP_Q;
    else if (step_q < PLL_WMIN_STEP_Q)
        step_q = PLL_WMIN_STEP_Q;

    return (int32_t)step_q;
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

static int64_t SOGI_InterpQ30(int16_t freq_x10,
                              int16_t f0_x10,
                              int16_t f1_x10,
                              int64_t c0_q30,
                              int64_t c1_q30)
{
    int16_t span;
    int16_t pos;
    int64_t delta;

    if (freq_x10 <= f0_x10)
        return c0_q30;

    if (freq_x10 >= f1_x10)
        return c1_q30;

    span = f1_x10 - f0_x10;
    pos = freq_x10 - f0_x10;
    delta = c1_q30 - c0_q30;

    if (delta >= 0)
        return c0_q30 + ((delta * pos) + (span / 2)) / span;

    return c0_q30 - (((-delta) * pos) + (span / 2)) / span;
}

static void SOGI_LoadCoeffFromStepQ(int32_t step_q)
{
    int16_t freq_x10;

    freq_x10 = PLL_StepToFreqX10(step_q);

    if (freq_x10 <= 600)
    {
        sogi_a1_q30  = SOGI_InterpQ30(freq_x10, 550, 600, SOGI55_A1_Q30,  SOGI60_A1_Q30);
        sogi_a2_q30  = SOGI_InterpQ30(freq_x10, 550, 600, SOGI55_A2_Q30,  SOGI60_A2_Q30);
        sogi_b0_q30  = SOGI_InterpQ30(freq_x10, 550, 600, SOGI55_B0_Q30,  SOGI60_B0_Q30);
        sogi_qb0_q30 = SOGI_InterpQ30(freq_x10, 550, 600, SOGI55_QB0_Q30, SOGI60_QB0_Q30);
        sogi_qb1_q30 = SOGI_InterpQ30(freq_x10, 550, 600, SOGI55_QB1_Q30, SOGI60_QB1_Q30);
        sogi_qb2_q30 = SOGI_InterpQ30(freq_x10, 550, 600, SOGI55_QB2_Q30, SOGI60_QB2_Q30);
    }
    else
    {
        sogi_a1_q30  = SOGI_InterpQ30(freq_x10, 600, 650, SOGI60_A1_Q30,  SOGI65_A1_Q30);
        sogi_a2_q30  = SOGI_InterpQ30(freq_x10, 600, 650, SOGI60_A2_Q30,  SOGI65_A2_Q30);
        sogi_b0_q30  = SOGI_InterpQ30(freq_x10, 600, 650, SOGI60_B0_Q30,  SOGI65_B0_Q30);
        sogi_qb0_q30 = SOGI_InterpQ30(freq_x10, 600, 650, SOGI60_QB0_Q30, SOGI65_QB0_Q30);
        sogi_qb1_q30 = SOGI_InterpQ30(freq_x10, 600, 650, SOGI60_QB1_Q30, SOGI65_QB1_Q30);
        sogi_qb2_q30 = SOGI_InterpQ30(freq_x10, 600, 650, SOGI60_QB2_Q30, SOGI65_QB2_Q30);
    }

    dbg_sogi_freq_x10 = freq_x10;
}

/* =========================================================
   Helper
   ========================================================= */
static uint16_t PG7_MapSigned(int32_t x, int32_t min_x, int32_t max_x)
{
    int64_t y;

    if (x > max_x)
        x = max_x;
    else if (x < min_x)
        x = min_x;

    y = ((int64_t)(x - min_x) * PG7PER) / (max_x - min_x);

    if (y > PG7PER)
        y = PG7PER;
    else if (y < 0)
        y = 0;

    return (uint16_t)y;
}
static uint16_t PG7_MapUnsigned(int32_t x, int32_t min_x, int32_t max_x)
{
    int64_t y;

    if (x > max_x)
        x = max_x;
    else if (x < min_x)
        x = min_x;

    y = ((int64_t)(x - min_x) * PG7PER) / (max_x - min_x);

    if (y > PG7PER)
        y = PG7PER;
    else if (y < 0)
        y = 0;

    return (uint16_t)y;
}
#define DBG_AMP_WINDOW_SAMPLES 667

static int32_t dbg_amp_min = 0;
static int32_t dbg_amp_max = 0;
static int32_t dbg_amp_val = 0;
static uint16_t dbg_amp_cnt = 0;

static void PG7_OutputAmplitude(int32_t x, int32_t max_amp)
{
    if (dbg_amp_cnt == 0)
    {
        dbg_amp_min = x;
        dbg_amp_max = x;
    }
    else
    {
        if (x < dbg_amp_min)
            dbg_amp_min = x;

        if (x > dbg_amp_max)
            dbg_amp_max = x;
    }

    dbg_amp_cnt++;

    if (dbg_amp_cnt >= DBG_AMP_WINDOW_SAMPLES)
    {
        dbg_amp_val = (dbg_amp_max - dbg_amp_min) / 2;
        dbg_amp_cnt = 0;
    }

    PG7DC = PG7_MapSigned(dbg_amp_val, 0, max_amp);
}
static void CURRENT_LOOP_StateClear(void)
{
    err_boost[0] = 0;
    err_boost[1] = 0;
    err_boost[2] = 0;

    duty_boost[0] = 0;
    duty_boost[1] = 0;

    err_buck[0] = 0;
    err_buck[1] = 0;
    err_buck[2] = 0;

    duty_buck[0] = 0;
    duty_buck[1] = 0;
}

/* =========================================================
   ADC Initialize
   ========================================================= */

void ADC1_Initialize(void)
{
    ADCON1L = (0x8000 & 0x7FFF);
    ADCON1H = 0x60;
    ADCON2L = 0x00;
    ADCON2H = 0x01;
    ADCON3L = 0x00;
    ADCON3H = (0xC281 & 0xFF00);
    ADCON4L = 0x00;
    ADCON4H = 0x00;

    ADMOD0L = 0x00;
    ADMOD0H = 0x00;
    ADMOD1L = 0x00;
    ADMOD1H = 0x00;

    /* Enable AN0 and AN1 interrupts */
    ADIEL = 0x03;

    /* Keep AN24 and AN25 enabled if your project still uses them */
    ADIEH = 0x300;

    ADCMP0ENL = 0x00;
    ADCMP1ENL = 0x00;
    ADCMP2ENL = 0x00;
    ADCMP3ENL = 0x00;

    ADCMP0ENH = 0x00;
    ADCMP1ENH = 0x00;
    ADCMP2ENH = 0x00;
    ADCMP3ENH = 0x00;

    ADCMP0LO = 0x00;
    ADCMP1LO = 0x00;
    ADCMP2LO = 0x00;
    ADCMP3LO = 0x00;

    ADCMP0HI = 0x00;
    ADCMP1HI = 0x00;
    ADCMP2HI = 0x00;
    ADCMP3HI = 0x00;

    ADFL0CON = 0x400;
    ADFL1CON = 0x400;
    ADFL2CON = 0x400;
    ADFL3CON = 0x400;

    ADCMP0CON = 0x00;
    ADCMP1CON = 0x00;
    ADCMP2CON = 0x00;
    ADCMP3CON = 0x00;

    ADLVLTRGL = 0x00;
    ADLVLTRGH = 0x00;

    ADCORE0L = 0x00;
    ADCORE1L = 0x00;
    ADCORE0H = 0x300;
    ADCORE1H = 0x300;

    ADEIEL = 0x00;
    ADEIEH = 0x00;

    ADCON5H = (0xF00 & 0xF0FF);

    ADC1_SetCommonInterruptHandler(&ADC1_CallBack);
    ADC1_Setchannel_AN24InterruptHandler(&ADC1_channel_AN24_CallBack);
    ADC1_Setchannel_AN25InterruptHandler(&ADC1_channel_AN25_CallBack);
    ADC1_Setchannel_AN0InterruptHandler(&ADC1_channel_AN0_CallBack);
    ADC1_Setchannel_AN1InterruptHandler(&ADC1_channel_AN1_CallBack);

    IFS12bits.ADCAN24IF = 0;
    IEC12bits.ADCAN24IE = 1;

    IFS12bits.ADCAN25IF = 0;
    IEC12bits.ADCAN25IE = 1;

    IFS5bits.ADCAN0IF = 0;
    IEC5bits.ADCAN0IE = 1;

    IFS5bits.ADCAN1IF = 0;
    IEC5bits.ADCAN1IE = 1;

    ADCON5Hbits.WARMTIME = 0xF;

    ADCON1Lbits.ADON = 0x1;

    ADC1_SharedCorePowerEnable();
    ADC1_Core0PowerEnable();
    ADC1_Core1PowerEnable();

    /*
       AN0 = current feedback
       AN1 = grid / PCC voltage feedback
       Both triggered by PWM1 Trigger1.
    */
    ADTRIG0L = 0x0404;

    ADTRIG0H = 0x00;
    ADTRIG1L = 0x00;
    ADTRIG1H = 0x00;
    ADTRIG2L = 0x00;
    ADTRIG2H = 0x00;
    ADTRIG3L = 0x00;
    ADTRIG3H = 0x00;
    ADTRIG4L = 0x00;
    ADTRIG4H = 0x00;

    /*
       AN24 / AN25 keep software trigger.
       Change this later only if you need them synchronized.
    */
    ADTRIG6L = 0x101;
}

/* =========================================================
   ADC power functions
   ========================================================= */

void ADC1_Core0PowerEnable(void)
{
    ADCON5Lbits.C0PWR = 1;
    while (ADCON5Lbits.C0RDY == 0);
    ADCON3Hbits.C0EN = 1;
}

void ADC1_Core1PowerEnable(void)
{
    ADCON5Lbits.C1PWR = 1;
    while (ADCON5Lbits.C1RDY == 0);
    ADCON3Hbits.C1EN = 1;
}

void ADC1_SharedCorePowerEnable(void)
{
    ADCON5Lbits.SHRPWR = 1;
    while (ADCON5Lbits.SHRRDY == 0);
    ADCON3Hbits.SHREN = 1;
}

/* =========================================================
   Common callback
   ========================================================= */

void __attribute__((weak)) ADC1_CallBack(void)
{
}

void ADC1_SetCommonInterruptHandler(void* handler)
{
    ADC1_CommonDefaultInterruptHandler = handler;
}

void __attribute__((weak)) ADC1_Tasks(void)
{
    if (IFS5bits.ADCIF)
    {
        if (ADC1_CommonDefaultInterruptHandler)
        {
            ADC1_CommonDefaultInterruptHandler();
        }

        IFS5bits.ADCIF = 0;
    }
}

/* =========================================================
   AN24 / AN25 callbacks
   ========================================================= */

void __attribute__((weak)) ADC1_channel_AN24_CallBack(uint16_t adcVal)
{
    (void)adcVal;
}

void ADC1_Setchannel_AN24InterruptHandler(void* handler)
{
    ADC1_channel_AN24DefaultInterruptHandler = handler;
}

void __attribute__((__interrupt__, auto_psv, weak)) _ADCAN24Interrupt(void)
{
    uint16_t valchannel_AN24 = ADCBUF24;

    if (ADC1_channel_AN24DefaultInterruptHandler)
    {
        ADC1_channel_AN24DefaultInterruptHandler(valchannel_AN24);
    }

    IFS12bits.ADCAN24IF = 0;
}

void __attribute__((weak)) ADC1_channel_AN25_CallBack(uint16_t adcVal)
{
    (void)adcVal;
}

void ADC1_Setchannel_AN25InterruptHandler(void* handler)
{
    ADC1_channel_AN25DefaultInterruptHandler = handler;
}

void __attribute__((__interrupt__, auto_psv, weak)) _ADCAN25Interrupt(void)
{
    uint16_t valchannel_AN25 = ADCBUF25;

    if (ADC1_channel_AN25DefaultInterruptHandler)
    {
        ADC1_channel_AN25DefaultInterruptHandler(valchannel_AN25);
    }

    IFS12bits.ADCAN25IF = 0;
}

/* =========================================================
   AN0 callback: current loop
   ========================================================= */

void __attribute__((weak)) ADC1_channel_AN0_CallBack(uint16_t adcVal)
{
    int64_t temp = 0;

    int32_t adc_used = 0;
    int32_t vref_cmd = 0;
    int32_t iref_amp = 0;
    int32_t iref_cmd = 0;
    int32_t iref_signed = 0;
    int32_t err_now = 0;
    uint8_t phase_cmd = 0;

    int32_t duty_cmd = 0;
    int32_t duty_boost_prev = 0;

    uint8_t current_mode = MODE_BUCK;

#if (ADC_INTERNAL_DUAL_TEST == 1)
    {
        int32_t test_sin;

        test_sin = PLL_SinInterp(adc_test_an0_idx, adc_test_an0_theta_acc_q);
        dbg_test_an0_count = (int16_t)(((int32_t)test_sin * ADC_TEST_AN0_AMP_COUNTS) >> 15);
        adcVal = (uint16_t)(ADC_CENTER + dbg_test_an0_count);

        adc_test_an0_theta_acc_q += ADC_TEST_STEP_Q;

        while (adc_test_an0_theta_acc_q >= ((int64_t)PLL_THETA_SUBDIV << PLL_STEP_Q))
        {
            adc_test_an0_theta_acc_q -= ((int64_t)PLL_THETA_SUBDIV << PLL_STEP_Q);
            adc_test_an0_idx++;

            if (adc_test_an0_idx >= PLL_TABLE_SIZE)
                adc_test_an0_idx = 0;
        }
    }
#endif

#if (GRID_TIE_REQUIRE_PLL_LOCK != 0)
    if (((PLL_TEST_FORCE_PLL_OUTPUT) == 0) &&
        ((PLL_CONTROL_ENABLE) != 0) &&
        ((pll_locked == 0) || (dbg_pll_phase_ok == 0)))
    {
        CURRENT_LOOP_StateClear();
        i_ref_count = 0;
        pll_vref_count = 0;
        finallyDuty_buck = 0;
        finallyDuty_boost = 0;
        PG1DC = 0;
        PG2DC = 0;
        return;
    }
#endif

    if (PLL_OUTPUT_USE_PLL)
    {
        vref_cmd = pll_vref_count;
        phase_cmd = pll_sync_phase;
    }
    else
    {
        vref_cmd = VrefTable[i];
        phase_cmd = (uint8_t)phase;
    }

    if (vref_cmd >= Vdc)
        current_mode = MODE_BOOST;
    else
        current_mode = MODE_BUCK;

    dbg_mode = current_mode;

    if (last_current_mode == MODE_UNKNOWN)
    {
        last_current_mode = current_mode;
    }
    else if (current_mode != last_current_mode)
    {
        CURRENT_LOOP_StateClear();

        finallyDuty_boost = Boost_PWM;
        finallyDuty_buck  = Buck_PWM;

        last_current_mode = current_mode;
        dbg_mode_sw++;
    }

    /*
       Current reference follows PLL angle when PLL control is enabled and locked.
       Before lock, keep the original open-loop table reference.
    */
    if (PLL_OUTPUT_USE_PLL)
    {
        iref_signed = i_ref_count;

        if ((iref_signed < IREF_ZERO_BAND) && (iref_signed > -IREF_ZERO_BAND))
            iref_signed = 0;

        iref_cmd = ADC_CENTER + iref_signed;
    }
    else
    {
        iref_amp = ((int32_t)vref_cmd * IREF_FROM_VREF_GAIN_Q +
                    (1L << (IREF_FROM_VREF_Q - 1))) >> IREF_FROM_VREF_Q;

        if (iref_amp < IREF_ZERO_BAND)
            iref_amp = 0;

        if (phase_cmd == 1)
            iref_cmd = ADC_CENTER + iref_amp;
        else
            iref_cmd = ADC_CENTER - iref_amp;
    }

    dbg_iref_cmd = (int16_t)iref_cmd;

    dbg_adc0_raw = (int16_t)adcVal;

    adc0_filt = adc0_filt + (((int32_t)adcVal - adc0_filt) >> ADC_FILT_SHIFT);
    adc_used = adc0_filt;

    dbg_adc0_filt = (int16_t)adc_used;

    if (phase_cmd == 1)
        err_now = iref_cmd - adc_used;
    else
        err_now = adc_used - iref_cmd;

    if ((err_now < ERR_DEADBAND) && (err_now > -ERR_DEADBAND))
        err_now = 0;

    dbg_ierr = (int16_t)err_now;

    if (current_mode == MODE_BOOST)
    {
        err_boost[0] = (int16_t)err_now;

        temp = 0;

        temp += (int64_t)B_Coefficient_BOOST[0] * err_boost[0];
        temp += (int64_t)B_Coefficient_BOOST[1] * err_boost[1];
        temp += (int64_t)B_Coefficient_BOOST[2] * err_boost[2];

        temp += (int64_t)A_Coefficient_BOOST[0] * duty_boost[0];
        temp += (int64_t)A_Coefficient_BOOST[1] * duty_boost[1];

        temp >>= 15;

        if (temp > BOOST_TEMP_LIMIT)
            temp = BOOST_TEMP_LIMIT;
        else if (temp < -BOOST_TEMP_LIMIT)
            temp = -BOOST_TEMP_LIMIT;

        finallyDuty_buck = Buck_PWM;

        duty_cmd = Boost_PWM + (int32_t)temp;

        if (duty_cmd > DUTY_BOOST_MAX)
            duty_cmd = DUTY_BOOST_MAX;
        else if (duty_cmd < 0)
            duty_cmd = 0;

        duty_boost_prev = duty_boost[0];

        finallyDuty_boost = (int16_t)duty_cmd;

        duty_boost[0] = finallyDuty_boost - Boost_PWM;

        err_boost[2] = err_boost[1];
        err_boost[1] = err_boost[0];

        duty_boost[1] = duty_boost_prev;

        dbg_temp = (int32_t)temp;
    }
    else
    {
        finallyDuty_buck = Buck_PWM;
        finallyDuty_boost = 0;

        CURRENT_LOOP_StateClear();

        dbg_temp = 0;
    }

    PG1DC = finallyDuty_buck;
    PG2DC = finallyDuty_boost;
}

void ADC1_Setchannel_AN0InterruptHandler(void* handler)
{
    ADC1_channel_AN0DefaultInterruptHandler = handler;
}

void __attribute__((__interrupt__, auto_psv, weak)) _ADCAN0Interrupt(void)
{
    uint16_t valchannel_AN0 = ADCBUF0;

    if (ADC1_channel_AN0DefaultInterruptHandler)
    {
        ADC1_channel_AN0DefaultInterruptHandler(valchannel_AN0);
    }

    IFS5bits.ADCAN0IF = 0;
}

/* =========================================================
   AN1 callback: standard SOGI-PLL
   1. AN1 is decimated from 40kHz to 5kHz before SOGI/PLL update.
   2. SOGI VA/VB are the alpha/beta inputs for Park transform.
   3. Park Vq is the PLL phase detector error.
   4. PI output trims omega; integrating omega gives phase.
   5. SOGI internal state uses Q12; PLL step uses Q12 fixed-point.
   ========================================================= */

void __attribute__((weak)) ADC1_channel_AN1_CallBack(uint16_t adcVal)
{
    static uint8_t pll_decim_cnt = 0;
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
    int32_t sin_ref;
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
    uint16_t pll_vref_idx;
    int32_t table_phase;
    int64_t theta_pos_q;
    int64_t theta_full_q;

    dbg_an1_isr_counter++;
    dbg_an1_rate_count++;

    pll_decim_cnt++;
    if (pll_decim_cnt < PLL_DECIM_N)
        return;

    pll_decim_cnt = 0;
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

        i_ref_count = 0;
        pll_vref_count = 0;
        pll_sync_phase = 1;

#if (AN1_RATE_DUTY_DEBUG_ENABLE == 0)
#if ((PLL_DEBUG_SELECT == PLL_DBG_LOCK) || \
     (PLL_DEBUG_SELECT == PLL_DBG_INPUT_VALID) || \
     (PLL_DEBUG_SELECT == PLL_DBG_PHASE_OK))
        PG7DC = (uint16_t)(((uint32_t)PG7PER * 20UL) / 100UL);
#endif
#endif

        return;
    }

    SOGI_LoadCoeffFromStepQ(pll_step_q);

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

    raw_err = (int32_t)(((int64_t)PLL_ERR_SIGN * vq_q * PLL_ERR_SCALE) / vmag_q);
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

    if ((pll_locked == 0) && (abs_err_lpf > PLL_LOCK_ERR_TH))
    {
        w_delta_q = PLL_Shift64((int64_t)pll_err_pi * PLL_STEP_SCALE, PLL_KP_SHIFT_FAST) +
                    PLL_Shift64((int64_t)pll_int_acc * PLL_STEP_SCALE, PLL_KI_SHIFT_FAST);
    }
    else
    {
        w_delta_q = PLL_Shift64((int64_t)pll_err_pi * PLL_STEP_SCALE, PLL_KP_SHIFT_LOCK) +
                    PLL_Shift64((int64_t)pll_int_acc * PLL_STEP_SCALE, PLL_KI_SHIFT_LOCK);
    }

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
    dbg_pll_freq_x10 = PLL_StepToFreqX10(pll_step_q);

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
    sin_ref = PLL_SinInterp(theta_output_idx, theta_acc_q);
    i_ref_count = (int16_t)(((int32_t)sin_ref * IREF_PEAK_COUNTS) >> 15);
    pll_sync_phase = (sin_ref >= 0) ? 1U : 0U;

    theta_pos_q = ((int64_t)theta_output_idx * PLL_THETA_DEN_Q) + theta_acc_q;
    theta_full_q = (int64_t)PLL_TABLE_SIZE * PLL_THETA_DEN_Q;

    table_phase = (int32_t)(((theta_pos_q * PLL_VREF_FULL_LEN) +
                              (theta_full_q / 2)) / theta_full_q);

    if (table_phase >= PLL_VREF_FULL_LEN)
        table_phase = 0;

    if (table_phase >= PLL_VREF_TABLE_LEN)
        pll_vref_idx = (uint16_t)(table_phase - PLL_VREF_TABLE_LEN);
    else
        pll_vref_idx = (uint16_t)table_phase;

    if (pll_vref_idx >= PLL_VREF_TABLE_LEN)
        pll_vref_idx = (uint16_t)(PLL_VREF_TABLE_LEN - 1);

    dbg_pll_vref_idx = pll_vref_idx;
    pll_vref_count = VrefTable[pll_vref_idx];

    pll_debug_slow_cnt++;
    if (pll_debug_slow_cnt >= 32)
        pll_debug_slow_cnt = 0;

#if (AN1_RATE_DUTY_DEBUG_ENABLE == 0)
#if (PLL_DEBUG_SELECT == PLL_DBG_LOCK)
    if (pll_locked)
        PG7DC = (uint16_t)(((uint32_t)PG7PER * 80UL) / 100UL);
    else
        PG7DC = (uint16_t)(((uint32_t)PG7PER * 20UL) / 100UL);
#elif (PLL_DEBUG_SELECT == PLL_DBG_OUTPUT_FREQ)
    {
        int64_t pg7_cmd64;
        int32_t freq_x10;

        freq_x10 = PLL_StepToFreqX10(pll_step_q);
        pg7_cmd64 = ((int64_t)(freq_x10 - 550) * PG7PER) / 100;

        if (pg7_cmd64 > PG7PER)
            pg7_cmd64 = PG7PER;
        else if (pg7_cmd64 < 0)
            pg7_cmd64 = 0;

        PG7DC = (uint16_t)pg7_cmd64;
    }
#elif (PLL_DEBUG_SELECT == PLL_DBG_PHASE_OK)
    if (dbg_pll_phase_ok)
        PG7DC = (uint16_t)(((uint32_t)PG7PER * 80UL) / 100UL);
    else
        PG7DC = (uint16_t)(((uint32_t)PG7PER * 20UL) / 100UL);
#elif (PLL_DEBUG_SELECT == PLL_DBG_VQ)
    PG7DC = PG7_MapSigned(PLL_Shift64(vq_q, SOGI_STATE_Q), -300, 300);
#elif (PLL_DEBUG_SELECT == PLL_DBG_ERR_FINE)
    PG7DC = PG7_MapSigned(pll_err_lpf, -100, 100);
#elif (PLL_DEBUG_SELECT == PLL_DBG_IREF)
    PG7DC = PG7_MapSigned(i_ref_count, -500, 500);
#elif (PLL_DEBUG_SELECT == PLL_DBG_VREF_IDX)
    PG7DC = PG7_MapUnsigned(dbg_pll_vref_idx, 0, PLL_VREF_TABLE_LEN - 1);
#elif (PLL_DEBUG_SELECT == PLL_DBG_THETA_PHASE)
    if (theta_output_idx < 180U)
        PG7DC = (uint16_t)(((uint32_t)PG7PER * 80UL) / 100UL);
    else
        PG7DC = (uint16_t)(((uint32_t)PG7PER * 20UL) / 100UL);
#elif (PLL_DEBUG_SELECT == PLL_DBG_AN1_AMP)
    PG7_OutputAmplitude(v_in_count, 2200);
#elif (PLL_DEBUG_SELECT == PLL_DBG_VA_AMP)
    PG7_OutputAmplitude(PLL_Shift64(sogi_va, SOGI_STATE_Q), 2200);
#elif (PLL_DEBUG_SELECT == PLL_DBG_VB_AMP)
    PG7_OutputAmplitude(PLL_Shift64(sogi_vb, SOGI_STATE_Q), 2200);
#else
    PG7DC = 0;
#endif
#endif
}

static void ADC1_channel_AN1_CallBack_OldUnused(uint16_t adcVal)
{
    static uint8_t pll_decim_cnt = 0;
    static int32_t an1_amp_min = 0;
    static int32_t an1_amp_max = 0;
    static int32_t an1_amp_counts = 0;
    static uint16_t an1_amp_cnt = 0;
    static uint8_t an1_signal_valid = 0;
    static int32_t zc_offset_5k = GRID_ADC_CENTER;
    static int32_t zc_v_filt_5k = 0;
    static uint16_t zc_sample_count_5k = 0;
    static uint8_t zc_armed_5k = 0;
    static uint32_t zc_period_sum_5k = 0;
    static uint8_t zc_period_count_5k = 0;
    static uint16_t an1_period_samples = 0;
    static uint8_t an1_zc_seen = 0;
    static uint16_t an1_zc_sample_count = 0;
    static uint8_t an1_zc_armed = 0;
    static uint8_t an1_zc_freq_valid = 0;
    static uint8_t an1_zc_toggle = 0;
    static uint8_t an1_60hz_lock_count = 0;
    static uint16_t an1_zc_window_sample_count = 0;
    static uint8_t an1_zc_window_cross_count = 0;
    static uint8_t pll_debug_slow_cnt = 0;
    static uint8_t pll_recip_slow_cnt = 0;
    static uint8_t an1_isr_toggle = 0;
    static int32_t pll_vmag_recip_q28 = 0;
    static uint16_t pll_pg7_step_dc = 0;

    int32_t v_in_count;
    int32_t v_in_q;
    int64_t acc_va;
    int64_t acc_vb;
    int32_t va_new;
    int32_t vb_new;
    int32_t sin_theta;
    int32_t cos_theta;
    uint16_t theta_comp_idx;
    int32_t sin_ref;
    int32_t vq_q;
    int32_t raw_err;
    int32_t abs_va;
    int32_t abs_vb;
    int32_t vmag_q;
    int32_t vmag_min_q;
    int32_t abs_err_lpf;
    int32_t pll_err_pi;
    int32_t kp_shift;
    int32_t ki_shift;
    int32_t deadband;
    int32_t pll_base_step_q;
    int32_t w_delta_q;
    int32_t pll_step_target_q;
    int32_t pll_step_diff_q;
    int32_t trim_limit_q;
    int32_t leak;
    uint16_t step_degrees;
    uint16_t pll_vref_idx;
    uint16_t theta_output_idx;
    int16_t pll_freq_now_x10;
    uint8_t do_slow_debug = 0;
    uint8_t lock_mode;
    uint8_t input_valid;
    uint8_t pll_lock_good;
    uint8_t pll_lock_bad;

    dbg_an1_isr_counter++;
    dbg_an1_rate_count++;
    an1_isr_toggle ^= 1U;

    /* =========================================================
       PART 1: 40kHz ??????Zero-Crossing?- ???????
       ========================================================= */
    

    /* =========================================================
       PART 2: 8 ???? (Decimator)??????? 5kHz ? SOGI-PLL
       ========================================================= */
    pll_decim_cnt++;
    if (pll_decim_cnt < PLL_DECIM_N)
    {
    #if (PLL_DEBUG_SELECT == PLL_DBG_AN1_ISR_TOGGLE)
        if (an1_isr_toggle != 0)
            PG7DC = PG7PER;
        else
            PG7DC = 0;
    #endif
        return;
    }
    pll_decim_cnt = 0;

    dbg_an1_raw = (int16_t)adcVal;

#if (SOGI_USE_INTERNAL_TEST_SINE == 1)
    {
        int32_t test_sin;

        test_sin = PLL_SinInterp(adc_test_an1_idx, adc_test_an1_theta_acc_q);
        dbg_test_an1_count = (int16_t)(((int32_t)test_sin * SOGI_TEST_AMP_COUNTS) >> 15);

        v_in_count = dbg_test_an1_count;
        grid_offset = GRID_ADC_CENTER;
        grid_offset_q = ((int32_t)GRID_ADC_CENTER << GRID_OFFSET_Q);
        dbg_an1_raw = (int16_t)(GRID_ADC_CENTER + dbg_test_an1_count);

        adc_test_an1_theta_acc_q += SOGI_TEST_STEP_Q;

        while (adc_test_an1_theta_acc_q >= ((int64_t)PLL_THETA_SUBDIV << PLL_STEP_Q))
        {
            adc_test_an1_theta_acc_q -= ((int64_t)PLL_THETA_SUBDIV << PLL_STEP_Q);
            adc_test_an1_idx++;

            if (adc_test_an1_idx >= PLL_TABLE_SIZE)
                adc_test_an1_idx = 0;
        }
    }
#else
    {
        int32_t grid_offset_err_q;
        int32_t grid_offset_delta_q;

        grid_offset_err_q = ((int32_t)adcVal << GRID_OFFSET_Q) - grid_offset_q;

        if (grid_offset_err_q >= 0)
        {
            grid_offset_delta_q = grid_offset_err_q >> GRID_OFFSET_SHIFT;
        }
        else
        {
            grid_offset_delta_q = -((-grid_offset_err_q) >> GRID_OFFSET_SHIFT);
        }

        grid_offset_q += grid_offset_delta_q;
        grid_offset = (grid_offset_q + (1L << (GRID_OFFSET_Q - 1))) >> GRID_OFFSET_Q;
    }

    v_in_count = (int32_t)adcVal - grid_offset;
#endif
    dbg_grid_v = (int16_t)v_in_count;

    if (an1_amp_cnt == 0)
    {
        an1_amp_min = v_in_count;
        an1_amp_max = v_in_count;
    }
    else
    {
        if (v_in_count < an1_amp_min) an1_amp_min = v_in_count;
        if (v_in_count > an1_amp_max) an1_amp_max = v_in_count;
    }

    an1_amp_cnt++;

    if (an1_amp_cnt >= PLL_INPUT_AMP_WINDOW_SAMPLES)
    {
        an1_amp_counts = (an1_amp_max - an1_amp_min) / 2;
        an1_signal_valid = (an1_amp_counts >= PLL_INPUT_VALID_AMP_COUNTS) ? 1 : 0;
        dbg_an1_amp_counts = (int16_t)an1_amp_counts;
        dbg_an1_input_valid = an1_signal_valid;
        an1_amp_cnt = 0;
    }

    /* =========================================================
   5kHz AN1 zero-crossing frequency estimator
   This runs after PLL_DECIM_N, same rate as SOGI/PLL.
   ========================================================= */
{
    static int32_t zc_v_filt_5k = 0;
    static int32_t zc_v_prev_5k = 0;
    static uint16_t zc_sample_count_5k = 0;
    static uint8_t zc_armed_5k = 0;
    static uint32_t zc_period_sum_5k = 0;
    static uint8_t zc_period_count_5k = 0;

    int32_t zc_v;
    int32_t zc_hys;

    zc_v_filt_5k = zc_v_filt_5k +
                   ((v_in_count - zc_v_filt_5k) >> AN1_ZC_FILTER_SHIFT);

    zc_v = zc_v_filt_5k;
    dbg_zc_v = (int16_t)zc_v;

    zc_hys = an1_amp_counts >> 3;

    if (zc_hys < AN1_ZC_HYS_COUNTS)
        zc_hys = AN1_ZC_HYS_COUNTS;
    else if (zc_hys > AN1_ZC_HYS_MAX_COUNTS)
        zc_hys = AN1_ZC_HYS_MAX_COUNTS;

    if (an1_signal_valid == 0)
    {
        zc_sample_count_5k = 0;
        zc_armed_5k = 0;
        zc_period_sum_5k = 0;
        zc_period_count_5k = 0;
        an1_zc_freq_valid = 0;
        an1_zc_toggle = 0;
        zc_v_prev_5k = zc_v;
        dbg_zc_freq_x10 = 0;
        dbg_an1_period_samples = 0;
    }
    else
    {
        if (zc_sample_count_5k < AN1_ZC_COUNTER_MAX_5K)
            zc_sample_count_5k++;

        if (zc_v <= -zc_hys)
        {
            zc_armed_5k = 1;
        }

        /*
           Count one full period on filtered negative-to-positive crossing.
           The armed flag still requires a real negative half-cycle first.
        */
        if ((zc_armed_5k != 0) && (zc_v_prev_5k < 0) && (zc_v >= 0))
        {
            uint16_t zc_period_samples;

            zc_period_samples = zc_sample_count_5k;
            zc_armed_5k = 0;
            zc_sample_count_5k = 0;
            dbg_zc_period_5k = zc_period_samples;

            an1_zc_toggle ^= 1U;

            if ((zc_period_samples >= AN1_ZC_PERIOD_MIN_5K) &&
                (zc_period_samples <= AN1_ZC_PERIOD_MAX_5K))
            {
                int32_t zc_freq_x10;

                an1_zc_seen = 1;
                an1_period_samples = zc_period_samples;
                dbg_an1_period_samples = zc_period_samples;
                dbg_zc_freq_x10 =
                    (int16_t)(((int32_t)FS_HZ * 10L + ((int32_t)zc_period_samples / 2)) /
                              (int32_t)zc_period_samples);

                zc_period_sum_5k += zc_period_samples;
                zc_period_count_5k++;

                if (zc_period_count_5k >= AN1_PERIOD_AVG_CYCLES)
                {
                    zc_freq_x10 =
                        ((int32_t)FS_HZ * 10L * (int32_t)zc_period_count_5k +
                         (int32_t)(zc_period_sum_5k / 2U)) /
                        (int32_t)zc_period_sum_5k;

                    dbg_zc_freq_x10 = (int16_t)zc_freq_x10;

                    if ((zc_freq_x10 >= 450) && (zc_freq_x10 <= 750))
                    {
                        pll_ff_step_target_q = PLL_FreqX10ToStepQ(zc_freq_x10);

                        if ((zc_freq_x10 >= PLL_60HZ_LOCK_MIN_X10) &&
                            (zc_freq_x10 <= PLL_60HZ_LOCK_MAX_X10))
                        {
                            if (an1_60hz_lock_count < PLL_60HZ_ZC_LOCK_COUNT)
                                an1_60hz_lock_count++;
                        }
                        else
                        {
                            an1_60hz_lock_count = 0;
                        }

                        if (an1_zc_freq_valid == 0)
                        {
                            an1_zc_freq_valid = 1;
#if (PLL_USE_ZC_FEED_FORWARD != 0)
                            pll_ff_step_q = pll_ff_step_target_q;
                            pll_step_q = pll_ff_step_target_q;
#endif
                        }
                    }
                    else
                    {
                        an1_zc_freq_valid = 0;
                        an1_60hz_lock_count = 0;
                    }

                    zc_period_sum_5k = 0;
                    zc_period_count_5k = 0;
                }
            }
            else
            {
                an1_zc_freq_valid = 0;
                an1_60hz_lock_count = 0;
                zc_period_sum_5k = 0;
                zc_period_count_5k = 0;
            }
        }

        zc_v_prev_5k = zc_v;
    }
}
    input_valid = an1_signal_valid;

    if (input_valid == 0)
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
        dbg_pll_err_slow = 0;
        pll_int_acc = 0;
#if (PLL_THREE_POINT_TEST_ENABLE != 0)
        pll_ff_step_target_q = PLL_GetTestBaseStepQ();
        pll_ff_step_q = PLL_GetTestBaseStepQ();
        pll_step_q = PLL_GetTestBaseStepQ();
        SOGI_LoadTestFreqCoeff();
#else
        pll_ff_step_target_q = PLL_WNOM_STEP_Q;
        pll_ff_step_q = PLL_WNOM_STEP_Q;
        pll_step_q = PLL_WNOM_STEP_Q;
#endif
        theta_acc_q = 0;
        pll_idx = 0;

        sogi_va = 0; sogi_vb = 0;
        sogi_va_n1 = 0; sogi_va_n2 = 0;
        sogi_vb_n1 = 0; sogi_vb_n2 = 0;
        sogi_vin_n1 = 0; sogi_vin_n2 = 0;

        an1_period_samples = 0;
        an1_zc_seen = 0;
        an1_zc_sample_count = 0;
        an1_zc_armed = 0;
        an1_zc_freq_valid = 0;
        an1_zc_toggle = 0;
        an1_60hz_lock_count = 0;
        an1_zc_window_sample_count = 0;
        an1_zc_window_cross_count = 0;
        zc_sample_count_5k = 0;
        zc_armed_5k = 0;
        zc_period_sum_5k = 0;
        zc_period_count_5k = 0;
        zc_v_filt_5k = 0;
        zc_offset_5k = GRID_ADC_CENTER;
        dbg_zc_freq_x10 = 0;
        dbg_sogi_freq_x10 = 600;

        i_ref_count = 0;
        pll_vref_count = 0;
        pll_sync_phase = 1;
        return;
    }

#if (PLL_THREE_POINT_TEST_ENABLE != 0)
    pll_ff_step_target_q = PLL_GetTestBaseStepQ();
    pll_ff_step_q = PLL_GetTestBaseStepQ();
    dbg_an1_period_samples = (uint16_t)(((int32_t)FS_HZ * 10L + (PLL_GetTestFreqX10() / 2)) / PLL_GetTestFreqX10());
    SOGI_LoadTestFreqCoeff();
#else
#if (PLL_USE_ZC_FEED_FORWARD != 0)
    pll_ff_step_q = pll_ff_step_q + ((pll_ff_step_target_q - pll_ff_step_q) >> PLL_FF_STEP_LPF_SHIFT);
    SOGI_LoadCoeffFromStepQ(pll_ff_step_q);
#else
    pll_ff_step_target_q = PLL_WNOM_STEP_Q;
    pll_ff_step_q = PLL_WNOM_STEP_Q;
    SOGI_LoadCoeffFromStepQ(pll_step_q);
#endif
#endif

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

    theta_comp_idx = PLL_AddDegrees(pll_idx, PLL_PHASE_COMP_DEG);

    sin_theta = PLL_SinInterp(theta_comp_idx, theta_acc_q);
    cos_theta = PLL_SinInterp(PLL_AddDegrees(theta_comp_idx, 90), theta_acc_q);

    vq_q = Q15_Round(((int64_t)sogi_va * cos_theta) + ((int64_t)sogi_vb * sin_theta));
    dbg_pll_vq = (int16_t)PLL_Shift64(vq_q, SOGI_STATE_Q);

    abs_va = (sogi_va >= 0) ? sogi_va : -sogi_va;
    abs_vb = (sogi_vb >= 0) ? sogi_vb : -sogi_vb;
    vmag_q = (abs_va > abs_vb) ? abs_va : abs_vb;
    vmag_min_q = PLL_VMAG_MIN * (1L << SOGI_STATE_Q);

    if (vmag_q < vmag_min_q) vmag_q = vmag_min_q;

    pll_recip_slow_cnt++;
    if ((pll_recip_slow_cnt >= 8) || (pll_vmag_recip_q28 == 0))
    {
        pll_recip_slow_cnt = 0;
        pll_vmag_recip_q28 = (int32_t)((1LL << 28) / vmag_q);
        if (pll_vmag_recip_q28 < 1) pll_vmag_recip_q28 = 1;
    }

    raw_err = (int32_t)((PLL_ERR_SIGN * (int64_t)vq_q * PLL_ERR_SCALE * pll_vmag_recip_q28) >> 28);
    raw_err -= PLL_ERR_BIAS;

    if (raw_err > PLL_ERR_LIMIT) raw_err = PLL_ERR_LIMIT;
    else if (raw_err < -PLL_ERR_LIMIT) raw_err = -PLL_ERR_LIMIT;

    dbg_pll_raw_err = (int16_t)raw_err;
    pll_err_lpf = pll_err_lpf + ((raw_err - pll_err_lpf) >> PLL_LPF_SHIFT);
    dbg_pll_err = (int16_t)pll_err_lpf;

    pll_err_slow_lpf_q8 = pll_err_slow_lpf_q8 +
                          ((((int32_t)pll_err_lpf << 8) - pll_err_slow_lpf_q8) >> 5);

    if (pll_err_slow_lpf_q8 >= 0)
    {
        pll_err_slow_lpf = (pll_err_slow_lpf_q8 + 128) >> 8;
    }
    else
    {
        pll_err_slow_lpf = -(((-pll_err_slow_lpf_q8) + 128) >> 8);
    }

    dbg_pll_err_slow = (int16_t)pll_err_slow_lpf;

    abs_err_lpf = (pll_err_lpf >= 0) ? pll_err_lpf : -pll_err_lpf;

    if (abs_err_lpf <= PLL_GRID_PHASE_ON_ERR_TH)
    {
        if (pll_phase_ok_count < PLL_GRID_PHASE_ON_COUNT_TH)
            pll_phase_ok_count++;

        pll_phase_bad_count = 0;

        if (pll_phase_ok_count >= PLL_GRID_PHASE_ON_COUNT_TH)
            dbg_pll_phase_ok = 1;
    }
    else if (abs_err_lpf > PLL_GRID_PHASE_OFF_ERR_TH)
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

    if (abs_err_lpf > PLL_LOCK_ERR_TH)
    {
        kp_shift = PLL_KP_SHIFT_FAST;
        ki_shift = PLL_KI_SHIFT_FAST;
        deadband = 0;
        lock_mode = 0;
    }
    else
    {
        kp_shift = PLL_KP_SHIFT_LOCK;
        ki_shift = PLL_KI_SHIFT_LOCK;
        deadband = PLL_PI_DEADBAND_LOCK;
        lock_mode = 1;
    }

    pll_err_pi = pll_err_lpf;

    if ((pll_err_pi <= deadband) && (pll_err_pi >= -deadband))
        pll_err_pi = 0;

    if ((pll_err_pi == 0) && (lock_mode == 1))
    {
        if (pll_int_acc > 0)
        {
            leak = PLL_Shift64(pll_int_acc, PLL_INT_LEAK_SHIFT);
            if (leak < 1) leak = 1;
            pll_int_acc -= leak;
            if (pll_int_acc < 0) pll_int_acc = 0;
        }
        else if (pll_int_acc < 0)
        {
            leak = PLL_Shift64(-pll_int_acc, PLL_INT_LEAK_SHIFT);
            if (leak < 1) leak = 1;
            pll_int_acc += leak;
            if (pll_int_acc > 0) pll_int_acc = 0;
        }
    }
    else
    {
        pll_int_acc += pll_err_pi;
    }

    if (pll_int_acc > PLL_INT_LIMIT) pll_int_acc = PLL_INT_LIMIT;
    else if (pll_int_acc < -PLL_INT_LIMIT) pll_int_acc = -PLL_INT_LIMIT;

    w_delta_q = PLL_Shift64((int64_t)pll_err_pi * PLL_STEP_SCALE, kp_shift) +
                PLL_Shift64((int64_t)pll_int_acc * PLL_STEP_SCALE, ki_shift);

    if (lock_mode == 1)
        trim_limit_q = PLL_LOCK_TRIM_LIMIT_Q;
    else
        trim_limit_q = PLL_TRIM_LIMIT_Q;

    if (w_delta_q > trim_limit_q) w_delta_q = trim_limit_q;
    else if (w_delta_q < -trim_limit_q) w_delta_q = -trim_limit_q;

#if (PLL_THREE_POINT_TEST_ENABLE != 0)
    w_delta_q = 0;
    pll_base_step_q = PLL_GetTestBaseStepQ();
#elif (PLL_USE_ZC_FEED_FORWARD != 0)
    pll_base_step_q = pll_ff_step_q;
#else
    pll_base_step_q = PLL_WNOM_STEP_Q;
#endif
    pll_step_target_q = pll_base_step_q + w_delta_q;

    if (pll_step_target_q > PLL_WMAX_STEP_Q) pll_step_target_q = PLL_WMAX_STEP_Q;
    else if (pll_step_target_q < PLL_WMIN_STEP_Q) pll_step_target_q = PLL_WMIN_STEP_Q;

#if (PLL_THREE_POINT_TEST_ENABLE != 0)
    pll_step_q = pll_step_target_q;
#else
    pll_step_diff_q = pll_step_target_q - pll_step_q;

    if (pll_step_diff_q > PLL_STEP_SLEW_Q) pll_step_diff_q = PLL_STEP_SLEW_Q;
    else if (pll_step_diff_q < -PLL_STEP_SLEW_Q) pll_step_diff_q = -PLL_STEP_SLEW_Q;

    pll_step_q += pll_step_diff_q;
#endif
    dbg_pll_step = PLL_Shift64(pll_step_q, PLL_STEP_Q);
    dbg_pll_step_q = pll_step_q;

    pll_debug_slow_cnt++;
    if (pll_debug_slow_cnt >= 32)
    {
        int64_t pg7_step_cmd64;

        pll_debug_slow_cnt = 0;
        do_slow_debug = 1;

        dbg_pll_freq_x10 = PLL_StepToFreqX10(pll_step_q);
        dbg_pll_step_pct_x10 = (int16_t)(((int64_t)(pll_step_q - PLL_WMIN_STEP_Q) * 1000LL) / (PLL_WMAX_STEP_Q - PLL_WMIN_STEP_Q));

        pg7_step_cmd64 = ((int64_t)(pll_step_q - PLL_WMIN_STEP_Q) * PG7PER) / (PLL_WMAX_STEP_Q - PLL_WMIN_STEP_Q);

        if (pg7_step_cmd64 > PG7PER) pg7_step_cmd64 = PG7PER;
        else if (pg7_step_cmd64 < 0) pg7_step_cmd64 = 0;

        pll_pg7_step_dc = (uint16_t)pg7_step_cmd64;
    }

    theta_acc_q += pll_step_q;
    step_degrees = (uint16_t)(theta_acc_q >> PLL_THETA_DEN_SHIFT);
    theta_acc_q -= ((int64_t)step_degrees << PLL_THETA_DEN_SHIFT);

    pll_idx += step_degrees;
    if (pll_idx >= PLL_TABLE_SIZE) pll_idx -= PLL_TABLE_SIZE;

    dbg_pll_idx = pll_idx;
    pll_freq_now_x10 = PLL_StepToFreqX10(pll_step_q);

#if ((PLL_THREE_POINT_TEST_ENABLE != 0) && (PLL_TEST_FORCE_LOCK != 0))
    pll_lock_count = PLL_LOCK_COUNT_TH;
    pll_unlock_count = 0;
    pll_locked = 1;
#elif (PLL_DEMO_LOCK_ON_ZC != 0)
    if (an1_zc_seen != 0)
    {
        pll_lock_count = PLL_LOCK_COUNT_TH;
        pll_unlock_count = 0;
        pll_locked = 1;
    }
    else
    {
        pll_lock_count = 0;
        pll_unlock_count = 0;
        pll_locked = 0;
    }
#else
    pll_lock_good = ((input_valid != 0) &&
                     (abs_err_lpf <= PLL_LOCK_ON_ERR_TH)) ? 1U : 0U;

    pll_lock_bad = ((input_valid == 0) ||
                    (abs_err_lpf > PLL_LOCK_OFF_ERR_TH)) ? 1U : 0U;

    if (pll_lock_good != 0)
    {
        if (pll_lock_count < PLL_LOCK_COUNT_TH)
            pll_lock_count++;

        pll_unlock_count = 0;

        if (pll_lock_count >= PLL_LOCK_COUNT_TH)
            pll_locked = 1;
    }
    else if (pll_lock_bad != 0)
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
#endif

    theta_output_idx = PLL_AddDegrees(pll_idx, PLL_OUTPUT_PHASE_COMP_DEG);

    sin_ref = PLL_SinInterp(theta_output_idx, theta_acc_q);
    i_ref_count = (int16_t)(((int32_t)sin_ref * IREF_PEAK_COUNTS) >> 15);

    if (sin_ref >= 0)
    {
        pll_sync_phase = 1;
    }
    else
    {
        pll_sync_phase = 0;
    }

    {
        int64_t theta_pos_q;
        int64_t theta_full_q;
        int32_t table_phase;

        theta_pos_q = ((int64_t)theta_output_idx * PLL_THETA_DEN_Q) + theta_acc_q;
        theta_full_q = (int64_t)PLL_TABLE_SIZE * PLL_THETA_DEN_Q;

        table_phase = (int32_t)(((theta_pos_q * PLL_VREF_FULL_LEN) +
                                  (theta_full_q / 2)) / theta_full_q);

        if (table_phase >= PLL_VREF_FULL_LEN)
            table_phase = 0;

        if (table_phase >= PLL_VREF_TABLE_LEN)
            pll_vref_idx = (uint16_t)(table_phase - PLL_VREF_TABLE_LEN);
        else
            pll_vref_idx = (uint16_t)table_phase;

        if (pll_vref_idx >= PLL_VREF_TABLE_LEN)
            pll_vref_idx = (uint16_t)(PLL_VREF_TABLE_LEN - 1);

        dbg_pll_vref_idx = pll_vref_idx;
        pll_vref_count = VrefTable[pll_vref_idx];
    }

    /* =========================================================
       PART 5: PG7 Debug Output ??????
       ========================================================= */
PG7_DEBUG_BLOCK:
#if (AN1_RATE_DUTY_DEBUG_ENABLE == 0)
#if ((PLL_DEBUG_SELECT == PLL_DBG_AN1) || \
     (PLL_DEBUG_SELECT == PLL_DBG_SOGI_VA) || \
     (PLL_DEBUG_SELECT == PLL_DBG_SOGI_VB) || \
     (PLL_DEBUG_SELECT == PLL_DBG_VQ) || \
     (PLL_DEBUG_SELECT == PLL_DBG_RAW_ERR) || \
     (PLL_DEBUG_SELECT == PLL_DBG_AN1_ISR_TOGGLE) || \
     (PLL_DEBUG_SELECT == PLL_DBG_AN1_AMP) || \
     (PLL_DEBUG_SELECT == PLL_DBG_VA_AMP) || \
     (PLL_DEBUG_SELECT == PLL_DBG_VB_AMP) || \
     (PLL_DEBUG_SELECT == PLL_DBG_ZC_V) || \
     (PLL_DEBUG_SELECT == PLL_DBG_ZC_TOGGLE) || \
     (PLL_DEBUG_SELECT == PLL_DBG_ERR_LPF) || \
     (PLL_DEBUG_SELECT == PLL_DBG_ERR_FINE) || \
     (PLL_DEBUG_SELECT == PLL_DBG_ERR_FINE_SLOW))
    if (1)
#else
    if (do_slow_debug != 0)
#endif
    {
#if (PLL_DEBUG_SELECT == PLL_DBG_CONST)
        PG7DC = (uint16_t)(((uint32_t)PG7PER * 50UL) / 100UL);

#elif (PLL_DEBUG_SELECT == PLL_DBG_AN1)
        PG7DC = PG7_MapSigned(v_in_count, -2000, 2000);

#elif (PLL_DEBUG_SELECT == PLL_DBG_SOGI_VA)
        PG7DC = PG7_MapSigned(PLL_Shift64(sogi_va, SOGI_STATE_Q), -2200, 2200);

#elif (PLL_DEBUG_SELECT == PLL_DBG_SOGI_VB)
        PG7DC = PG7_MapSigned(PLL_Shift64(sogi_vb, SOGI_STATE_Q), -2200, 2200);

#elif (PLL_DEBUG_SELECT == PLL_DBG_VQ)
        PG7DC = PG7_MapSigned(PLL_Shift64(vq_q, SOGI_STATE_Q), -300, 300);

#elif (PLL_DEBUG_SELECT == PLL_DBG_RAW_ERR)
        PG7DC = PG7_MapSigned(raw_err, -200, 200);

#elif (PLL_DEBUG_SELECT == PLL_DBG_ERR_LPF)
        PG7DC = PG7_MapSigned(pll_err_lpf, -500, 500);

#elif (PLL_DEBUG_SELECT == PLL_DBG_ERR_FINE)
        PG7DC = PG7_MapSigned(pll_err_lpf, -100, 100);

#elif (PLL_DEBUG_SELECT == PLL_DBG_ERR_FINE_SLOW)
        PG7DC = PG7_MapSigned(pll_err_slow_lpf, -100, 100);

#elif (PLL_DEBUG_SELECT == PLL_DBG_STEP_Q)
        PG7DC = pll_pg7_step_dc;

#elif (PLL_DEBUG_SELECT == PLL_DBG_OUTPUT_FREQ)
        PG7DC = pll_pg7_step_dc;

#elif (PLL_DEBUG_SELECT == PLL_DBG_ZC_FREQ)
        PG7DC = PG7_MapSigned(dbg_zc_freq_x10, 450, 750);

#elif (PLL_DEBUG_SELECT == PLL_DBG_ZC_TOGGLE)
        if (an1_zc_toggle != 0) PG7DC = PG7PER;
        else                    PG7DC = 0;

#elif (PLL_DEBUG_SELECT == PLL_DBG_IREF)
        PG7DC = PG7_MapSigned(i_ref_count, -500, 500);

#elif (PLL_DEBUG_SELECT == PLL_DBG_LOCK)
        if (pll_locked) PG7DC = (uint16_t)(((uint32_t)PG7PER * 80UL) / 100UL);
        else            PG7DC = (uint16_t)(((uint32_t)PG7PER * 20UL) / 100UL);

#elif (PLL_DEBUG_SELECT == PLL_DBG_INPUT_VALID)
        if (input_valid) PG7DC = (uint16_t)(((uint32_t)PG7PER * 80UL) / 100UL);
        else             PG7DC = (uint16_t)(((uint32_t)PG7PER * 20UL) / 100UL);

#elif (PLL_DEBUG_SELECT == PLL_DBG_AN1_ISR_TOGGLE)
        if (an1_isr_toggle != 0) PG7DC = PG7PER;
        else                     PG7DC = 0;

#elif (PLL_DEBUG_SELECT == PLL_DBG_AN1_AMP)
        PG7_OutputAmplitude(v_in_count, 2200);

#elif (PLL_DEBUG_SELECT == PLL_DBG_VA_AMP)
        PG7_OutputAmplitude(PLL_Shift64(sogi_va, SOGI_STATE_Q), 2200);

#elif (PLL_DEBUG_SELECT == PLL_DBG_VB_AMP)
        PG7_OutputAmplitude(PLL_Shift64(sogi_vb, SOGI_STATE_Q), 2200);

#elif (PLL_DEBUG_SELECT == PLL_DBG_AN1_PERIOD)
        PG7DC = PG7_MapSigned(an1_period_samples, AN1_ZC_MIN_PERIOD_SAMPLES, AN1_ZC_MAX_PERIOD_SAMPLES);

#elif (PLL_DEBUG_SELECT == PLL_DBG_ZC_OFFSET)
        PG7DC = PG7_MapSigned(dbg_zc_offset, 0, 4095);

#elif (PLL_DEBUG_SELECT == PLL_DBG_ZC_V)
        if (dbg_zc_v >= 0)
            PG7DC = PG7PER;
        else
            PG7DC = 0;

#elif (PLL_DEBUG_SELECT == PLL_DBG_ZC_PERIOD_5K)
        PG7DC = PG7_MapSigned(dbg_zc_period_5k, AN1_ZC_PERIOD_MIN_5K, AN1_ZC_PERIOD_MAX_5K);

#elif (PLL_DEBUG_SELECT == PLL_DBG_PHASE_OK)
        if (dbg_pll_phase_ok) PG7DC = (uint16_t)(((uint32_t)PG7PER * 80UL) / 100UL);
        else                  PG7DC = (uint16_t)(((uint32_t)PG7PER * 20UL) / 100UL);

#elif (PLL_DEBUG_SELECT == PLL_DBG_VREF_IDX)
        PG7DC = PG7_MapUnsigned(dbg_pll_vref_idx, 0, PLL_VREF_TABLE_LEN - 1);

#else
        PG7DC = 0;
#endif
    }
#endif
}
//void __attribute__((weak)) ADC1_channel_AN1_CallBack(uint16_t adcVal)
//{
//    int32_t v_in_count;
//    int32_t v_in_q;
//
//    int64_t acc_va;
//    int64_t acc_vb;
//
//    int32_t va_new;
//    int32_t vb_new;
//
//    int32_t sin_theta;
//    int32_t cos_theta;
//    uint16_t theta_comp_idx;
//    int32_t sin_ref;
//    int32_t sin_abs;
//
//    int32_t vq_q;
//    int32_t raw_err;
//
//    int32_t abs_va;
//    int32_t abs_vb;
//    int32_t vmag_q;
//    int32_t vmag_min_q;
//
//    int32_t abs_err_lpf;
//    int32_t pll_err_pi;
//    int32_t kp_shift;
//    int32_t ki_shift;
//    int32_t deadband;
//    int32_t pll_base_step_q;
//    int32_t w_delta_q;
//    int32_t pll_step_target_q;
//    int32_t pll_step_diff_q;
//    uint8_t lock_mode;
//    uint8_t input_valid;
//
//    int32_t leak;
//    int64_t pg7_cmd64;
//
//    dbg_an1_raw = (int16_t)adcVal;
//    dbg_an1_isr_counter++;
//    dbg_an1_rate_count++;
//
//#if (ADC_INTERNAL_DUAL_TEST == 1)
//
//{
//    int32_t test_sin;
//    int32_t an0_scaled;
//
//    test_sin = PLL_SinInterp(adc_test_an1_idx, adc_test_an1_theta_acc_q);
//    dbg_test_an1_count = (int16_t)(((int32_t)test_sin * ADC_TEST_AN1_AMP_COUNTS) >> 15);
//    v_in_count = dbg_test_an1_count;
//
//    an0_scaled = ((int32_t)dbg_test_an0_count * ADC_TEST_AN1_AMP_COUNTS) / ADC_TEST_AN0_AMP_COUNTS;
//    dbg_test_sync_err = (int16_t)(an0_scaled - (int32_t)dbg_test_an1_count);
//
//    adc_test_an1_theta_acc_q += ADC_TEST_STEP_Q;
//
//    while (adc_test_an1_theta_acc_q >= ((int64_t)PLL_THETA_SUBDIV << PLL_STEP_Q))
//    {
//        adc_test_an1_theta_acc_q -= ((int64_t)PLL_THETA_SUBDIV << PLL_STEP_Q);
//        adc_test_an1_idx++;
//
//        if (adc_test_an1_idx >= PLL_TABLE_SIZE)
//            adc_test_an1_idx = 0;
//    }
//}
//
//#elif (SOGI_USE_INTERNAL_TEST_SINE == 1)
//
//{
//    static uint16_t test_idx = 0;
//    static int64_t test_theta_acc_q = 0;
//
//    int32_t test_sin;
//
//    /*
//       Internal 60Hz sine test.
//       This bypasses ADC input and tests only the SOGI math on MCU.
//    */
//    test_sin = PLL_SinInterp(test_idx, test_theta_acc_q);
//
//    v_in_count = (int32_t)(((int64_t)test_sin * SOGI_TEST_AMP_COUNTS) >> 15);
//
//    test_theta_acc_q += PLL_WNOM_STEP_Q;
//
//    while (test_theta_acc_q >= ((int64_t)PLL_THETA_SUBDIV << PLL_STEP_Q))
//    {
//        test_theta_acc_q -= ((int64_t)PLL_THETA_SUBDIV << PLL_STEP_Q);
//
//        test_idx++;
//
//        if (test_idx >= PLL_TABLE_SIZE)
//            test_idx = 0;
//    }
//}
//
//#else
//
///*
//   Real AN1 input.
//   AN1 is 0~3.3V ADC signal.
//   Track and remove measured ADC center offset.
//*/
//grid_offset = grid_offset + (((int32_t)adcVal - grid_offset) >> GRID_OFFSET_SHIFT);
//v_in_count = (int32_t)adcVal - grid_offset;
//
//#endif
//
//dbg_grid_v = (int16_t)v_in_count;
//
///*
//   Reject noise-only input. Without this gate, ADC offset/noise can create
//   fake zero-crossings and make the PLL hunt even when AN1 is disconnected.
//*/
//static int32_t an1_amp_min = 0;
//static int32_t an1_amp_max = 0;
//static int32_t an1_amp_counts = 0;
//static uint16_t an1_amp_cnt = 0;
//static uint8_t an1_signal_valid = 0;
//
//if (an1_amp_cnt == 0)
//{
//    an1_amp_min = v_in_count;
//    an1_amp_max = v_in_count;
//}
//else
//{
//    if (v_in_count < an1_amp_min)
//        an1_amp_min = v_in_count;
//
//    if (v_in_count > an1_amp_max)
//        an1_amp_max = v_in_count;
//}
//
//an1_amp_cnt++;
//
//if (an1_amp_cnt >= PLL_INPUT_AMP_WINDOW_SAMPLES)
//{
//    an1_amp_counts = (an1_amp_max - an1_amp_min) / 2;
//    an1_signal_valid = (an1_amp_counts >= PLL_INPUT_VALID_AMP_COUNTS) ? 1 : 0;
//    dbg_an1_amp_counts = (int16_t)an1_amp_counts;
//    dbg_an1_input_valid = an1_signal_valid;
//    an1_amp_cnt = 0;
//}
//
//input_valid = an1_signal_valid;
//
//if (input_valid == 0)
//{
//    pll_locked = 0;
//    pll_lock_count = 0;
//    pll_err_lpf = 0;
//    pll_int_acc = 0;
//    pll_ff_step_target_q = PLL_WNOM_STEP_Q;
//    pll_ff_step_q = PLL_WNOM_STEP_Q;
//    pll_step_q = PLL_WNOM_STEP_Q;
//    theta_acc_q = 0;
//    pll_idx = 0;
//
//    sogi_va = 0;
//    sogi_vb = 0;
//    sogi_va_n1 = 0;
//    sogi_va_n2 = 0;
//    sogi_vb_n1 = 0;
//    sogi_vb_n2 = 0;
//    sogi_vin_n1 = 0;
//    sogi_vin_n2 = 0;
//
//    i_ref_count = 0;
//    pll_vref_count = 0;
//    pll_sync_phase = 1;
//    v_in_count = 0;
//}
//
//#define AN1_ZC_HYS_COUNTS 500
//
//static uint16_t an1_sample_cnt = 0;
//static uint16_t an1_period_samples = 0;
//static uint32_t an1_period_sum = 0;
//static uint8_t an1_period_count = 0;
//static uint8_t an1_zc_armed = 0;
//
//an1_sample_cnt++;
//
//if (an1_sample_cnt > 1000)
//{
//    an1_sample_cnt = 1000;
//}
//
///*
//   ??? -500 counts????????????
//*/
//if (v_in_count <= -AN1_ZC_HYS_COUNTS)
//{
//    an1_zc_armed = 1;
//}
//
///*
//   ??? +500 counts??????????
//*/
//if ((an1_zc_armed == 1) && (v_in_count >= AN1_ZC_HYS_COUNTS))
//{
//    an1_period_samples = an1_sample_cnt;
//    dbg_an1_period_samples = an1_period_samples;
//
//    if ((an1_period_samples >= 20) && (an1_period_samples <= 40))
//    {
//        an1_period_sum += an1_period_samples;
//        an1_period_count++;
//
//        if (an1_period_count >= AN1_PERIOD_AVG_CYCLES)
//        {
//            pll_ff_step_target_q = (int32_t)(((int64_t)PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE *
//                                              an1_period_count) /
//                                             an1_period_sum);
//
//            if (pll_ff_step_target_q > PLL_WMAX_STEP_Q)
//                pll_ff_step_target_q = PLL_WMAX_STEP_Q;
//            else if (pll_ff_step_target_q < PLL_WMIN_STEP_Q)
//                pll_ff_step_target_q = PLL_WMIN_STEP_Q;
//
//            an1_period_sum = 0;
//            an1_period_count = 0;
//        }
//    }
//
//    an1_sample_cnt = 0;
//    an1_zc_armed = 0;
//}
//
//pll_ff_step_q = pll_ff_step_q +
//                ((pll_ff_step_target_q - pll_ff_step_q) >> PLL_FF_STEP_LPF_SHIFT);
//
///*
//   Keep SOGI tuned near the PLL estimated frequency.
//   Use the previous PLL step, because the current sample has not updated the
//   PLL yet. The AN1 period still provides the feed-forward base frequency.
//*/
//if (pll_step_q < PLL_W57P5_STEP_Q)
//{
//    sogi_a1_q30 = SOGI55_A1_Q30;
//    sogi_a2_q30 = SOGI55_A2_Q30;
//    sogi_b0_q30 = SOGI55_B0_Q30;
//    sogi_qb0_q30 = SOGI55_QB0_Q30;
//    sogi_qb1_q30 = SOGI55_QB1_Q30;
//    sogi_qb2_q30 = SOGI55_QB2_Q30;
//}
//else if (pll_step_q > PLL_W62P5_STEP_Q)
//{
//    sogi_a1_q30 = SOGI65_A1_Q30;
//    sogi_a2_q30 = SOGI65_A2_Q30;
//    sogi_b0_q30 = SOGI65_B0_Q30;
//    sogi_qb0_q30 = SOGI65_QB0_Q30;
//    sogi_qb1_q30 = SOGI65_QB1_Q30;
//    sogi_qb2_q30 = SOGI65_QB2_Q30;
//}
//else
//{
//    sogi_a1_q30 = SOGI60_A1_Q30;
//    sogi_a2_q30 = SOGI60_A2_Q30;
//    sogi_b0_q30 = SOGI60_B0_Q30;
//    sogi_qb0_q30 = SOGI60_QB0_Q30;
//    sogi_qb1_q30 = SOGI60_QB1_Q30;
//    sogi_qb2_q30 = SOGI60_QB2_Q30;
//}
//
//    /*
//       Convert input to Q12 counts.
//       Do not use left shift on negative numbers.
//    */
//    v_in_q = v_in_count * (1L << SOGI_STATE_Q);
//
//    /*
//       Tustin SOGI with Q12 internal state.
//
//       va_q[n] = b0*v_q[n] - b0*v_q[n-2]
//                 - a1*va_q[n-1] - a2*va_q[n-2]
//
//       vb_q[n] = qb0*v_q[n] + qb1*v_q[n-1] + qb2*v_q[n-2]
//                 - a1*vb_q[n-1] - a2*vb_q[n-2]
//    */
//acc_va = 0;
//acc_va += sogi_b0_q30 * v_in_q;
//acc_va -= sogi_b0_q30 * sogi_vin_n2;
//acc_va -= sogi_a1_q30 * sogi_va_n1;
//acc_va -= sogi_a2_q30 * sogi_va_n2;
//
//acc_vb = 0;
//acc_vb += sogi_qb0_q30 * v_in_q;
//acc_vb += sogi_qb1_q30 * sogi_vin_n1;
//acc_vb += sogi_qb2_q30 * sogi_vin_n2;
//acc_vb -= sogi_a1_q30 * sogi_vb_n1;
//acc_vb -= sogi_a2_q30 * sogi_vb_n2;
//
//va_new = Q30_Round(acc_va);
//vb_new = Q30_Round(acc_vb);
//
//    sogi_va = va_new;
//    sogi_vb = vb_new;
//
//    /*
//       Shift SOGI states.
//       All are Q12 counts.
//    */
//    sogi_vin_n2 = sogi_vin_n1;
//    sogi_vin_n1 = v_in_q;
//
//    sogi_va_n2 = sogi_va_n1;
//    sogi_va_n1 = va_new;
//
//    sogi_vb_n2 = sogi_vb_n1;
//    sogi_vb_n1 = vb_new;
//
//    /*
//       Park transform with fractional angle interpolation.
//       theta_acc_q is the fractional location inside current 1-degree interval.
//    */
//    theta_comp_idx = (pll_idx + PLL_PHASE_COMP_DEG) % PLL_TABLE_SIZE;
//    sin_theta = PLL_SinInterp(theta_comp_idx, theta_acc_q);
//    cos_theta = PLL_SinInterp((theta_comp_idx + 90) % PLL_TABLE_SIZE, theta_acc_q);
//
//    /*
//       sogi_va / sogi_vb are Q12 counts.
//       sin/cos are Q15.
//       vq_q is Q12 counts.
//    */
//    vq_q = Q15_Round(((int64_t)sogi_va * cos_theta) +
//                     ((int64_t)sogi_vb * sin_theta));
//
//    dbg_pll_vq = (int16_t)PLL_Shift64(vq_q, SOGI_STATE_Q);
//
//    /*
//       Normalize error:
//       raw_err = vq / |V| * 1000
//       raw_err unit = milli-pu.
//    */
//    abs_va = (sogi_va >= 0) ? sogi_va : -sogi_va;
//    abs_vb = (sogi_vb >= 0) ? sogi_vb : -sogi_vb;
//
//    if (abs_va > abs_vb)
//        vmag_q = abs_va;
//    else
//        vmag_q = abs_vb;
//
//    vmag_min_q = PLL_VMAG_MIN * (1L << SOGI_STATE_Q);
//
//    if (vmag_q < vmag_min_q)
//        vmag_q = vmag_min_q;
//
//    raw_err = (int32_t)(((int64_t)vq_q * PLL_ERR_SCALE) / vmag_q);
//
//    if (raw_err > PLL_ERR_LIMIT)
//        raw_err = PLL_ERR_LIMIT;
//    else if (raw_err < -PLL_ERR_LIMIT)
//        raw_err = -PLL_ERR_LIMIT;
//
//    dbg_pll_raw_err = (int16_t)raw_err;
//
//    /*
//       LPF:
//       Keep normal right-shift version.
//       Do NOT add minimum ? update, otherwise pll_step will shake.
//    */
//    pll_err_lpf = pll_err_lpf + ((raw_err - pll_err_lpf) >> PLL_LPF_SHIFT);
//
//    dbg_pll_err = (int16_t)pll_err_lpf;
//
//    /*
//       PLL PI controller:
//       Fast mode during startup / large error.
//       Lock mode during steady state / small error.
//    */
//    abs_err_lpf = (pll_err_lpf >= 0) ? pll_err_lpf : -pll_err_lpf;
//
//    if (abs_err_lpf > PLL_LOCK_ERR_TH)
//    {
//        kp_shift = PLL_KP_SHIFT_FAST;
//        ki_shift = PLL_KI_SHIFT_FAST;
//        deadband = 0;
//        lock_mode = 0;
//    }
//    else
//    {
//        kp_shift = PLL_KP_SHIFT_LOCK;
//        ki_shift = PLL_KI_SHIFT_LOCK;
//        deadband = PLL_PI_DEADBAND_LOCK;
//        lock_mode = 1;
//    }
//
//    pll_err_pi = pll_err_lpf;
//
//    if ((pll_err_pi <= deadband) && (pll_err_pi >= -deadband))
//    {
//        pll_err_pi = 0;
//    }
//
//    /*
//       Integrator:
//       The measured AN1 period is the feed-forward frequency, so when the PLL
//       is near lock the integrator should only trim phase and then leak back
//       toward zero. This prevents a tiny fixed error from slowly walking the
//       generated frequency away.
//    */
//    if ((pll_err_pi == 0) && (lock_mode == 1))
//    {
//        if (pll_int_acc > 0)
//        {
//            leak = PLL_Shift64(pll_int_acc, PLL_INT_LEAK_SHIFT);
//
//            if (leak < 1)
//                leak = 1;
//
//            pll_int_acc -= leak;
//
//            if (pll_int_acc < 0)
//                pll_int_acc = 0;
//        }
//        else if (pll_int_acc < 0)
//        {
//            leak = PLL_Shift64(-pll_int_acc, PLL_INT_LEAK_SHIFT);
//
//            if (leak < 1)
//                leak = 1;
//
//            pll_int_acc += leak;
//
//            if (pll_int_acc > 0)
//                pll_int_acc = 0;
//        }
//    }
//    else
//    {
//        pll_int_acc += pll_err_pi;
//    }
//
//    if (pll_int_acc > PLL_INT_LIMIT)
//        pll_int_acc = PLL_INT_LIMIT;
//    else if (pll_int_acc < -PLL_INT_LIMIT)
//        pll_int_acc = -PLL_INT_LIMIT;
//
//    /*
//       Important:
//       Convert to Q12 step first, then shift.
//       Do not use:
//           (pll_err_lpf >> 4) << PLL_STEP_Q
//       because it loses fractional resolution.
//    */
//    w_delta_q = PLL_Shift64((int64_t)pll_err_pi * PLL_STEP_SCALE, kp_shift) +
//                PLL_Shift64((int64_t)pll_int_acc * PLL_STEP_SCALE, ki_shift);
//
//    if (w_delta_q > PLL_TRIM_LIMIT_Q)
//        w_delta_q = PLL_TRIM_LIMIT_Q;
//    else if (w_delta_q < -PLL_TRIM_LIMIT_Q)
//        w_delta_q = -PLL_TRIM_LIMIT_Q;
//
//    pll_base_step_q = pll_ff_step_q;
//    pll_step_target_q = pll_base_step_q + w_delta_q;
//
//    if (pll_step_target_q > PLL_WMAX_STEP_Q)
//        pll_step_target_q = PLL_WMAX_STEP_Q;
//    else if (pll_step_target_q < PLL_WMIN_STEP_Q)
//        pll_step_target_q = PLL_WMIN_STEP_Q;
//
//    pll_step_diff_q = pll_step_target_q - pll_step_q;
//
//    if (pll_step_diff_q > PLL_STEP_SLEW_Q)
//        pll_step_diff_q = PLL_STEP_SLEW_Q;
//    else if (pll_step_diff_q < -PLL_STEP_SLEW_Q)
//        pll_step_diff_q = -PLL_STEP_SLEW_Q;
//
//    pll_step_q += pll_step_diff_q;
//
//    dbg_pll_step = PLL_Shift64(pll_step_q, PLL_STEP_Q);
//    dbg_pll_step_q = pll_step_q;
//    dbg_pll_freq_x10 = (int16_t)(((int64_t)pll_step_q * FS_HZ * 10LL) /
//                                  ((int64_t)PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE));
//    dbg_pll_step_pct_x10 = (int16_t)(((int64_t)(pll_step_q - PLL_WMIN_STEP_Q) * 1000LL) /
//                                      (PLL_WMAX_STEP_Q - PLL_WMIN_STEP_Q));
//
//    /*
//       Phase accumulator, Q12 fractional version.
//    */
//    theta_acc_q += pll_step_q;
//
//    while (theta_acc_q >= ((int64_t)PLL_THETA_SUBDIV << PLL_STEP_Q))
//    {
//        theta_acc_q -= ((int64_t)PLL_THETA_SUBDIV << PLL_STEP_Q);
//
//        pll_idx++;
//
//        if (pll_idx >= PLL_TABLE_SIZE)
//            pll_idx = 0;
//    }
//
//    dbg_pll_idx = pll_idx;
//
//    /*
//       Lock detection with hysteresis and dwell time.
//       Real ADC sampling has ripple/jitter, so do not unlock on one sample.
//    */
//    if ((input_valid != 0) &&
//        (pll_err_lpf <= PLL_LOCK_ON_ERR_TH) &&
//        (pll_err_lpf >= -PLL_LOCK_ON_ERR_TH))
//    {
//        if (pll_lock_count < PLL_LOCK_COUNT_TH)
//        {
//            pll_lock_count++;
//        }
//
//        if (pll_lock_count >= PLL_LOCK_COUNT_TH)
//        {
//            pll_locked = 1;
//        }
//    }
//    else if ((pll_err_lpf > PLL_LOCK_OFF_ERR_TH) ||
//             (pll_err_lpf < -PLL_LOCK_OFF_ERR_TH))
//    {
//        pll_lock_count = 0;
//        pll_locked = 0;
//    }
//
//    /*
//       PLL-generated current reference.
//       Use interpolated sine, not integer-degree sine table only.
//    */
//    sin_ref = PLL_SinInterp(pll_idx, theta_acc_q);
//
//    i_ref_count = (int16_t)(((int32_t)sin_ref * IREF_PEAK_COUNTS) >> 15);
//
//    if (sin_ref >= 0)
//    {
//        sin_abs = sin_ref;
//        pll_sync_phase = 1;
//    }
//    else
//    {
//        sin_abs = -sin_ref;
//        pll_sync_phase = 0;
//    }
//
//    if (sin_abs > 32767)
//        sin_abs = 32767;
//
//    pll_vref_count = (int16_t)(((int32_t)sin_abs * VREF_PEAK) >> 15);
//
//   /* =========================================================
//   PG7 Debug Output
//   Change PLL_DEBUG_SELECT to observe different internal signals.
//   ========================================================= */
//{
//    int32_t dbg_val;
//
//#if (PLL_DEBUG_SELECT == PLL_DBG_CONST)
//
//    PG7DC = (uint16_t)((int64_t)PG7PER * 40 / 100);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_AN1)
//
//    PG7DC = PG7_MapSigned(v_in_count, -2000, 2000);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_SOGI_VA)
//
//    dbg_val = PLL_Shift64(sogi_va, SOGI_STATE_Q);
//    PG7DC = PG7_MapSigned(dbg_val, -2200, 2200);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_SOGI_VB)
//
//    dbg_val = PLL_Shift64(sogi_vb, SOGI_STATE_Q);
//    PG7DC = PG7_MapSigned(dbg_val, -2200, 2200);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_VQ)
//
//    dbg_val = PLL_Shift64(vq_q, SOGI_STATE_Q);
//    PG7DC = PG7_MapSigned(dbg_val, -300, 300);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_RAW_ERR)
//
//    PG7DC = PG7_MapSigned(raw_err, -200, 200);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_ERR_LPF)
//
//    PG7DC = PG7_MapSigned(pll_err_lpf, -500, 500);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_STEP_Q)
//
//    pg7_cmd64 = ((int64_t)(pll_step_q - PLL_WMIN_STEP_Q) * PG7PER) /
//                (PLL_WMAX_STEP_Q - PLL_WMIN_STEP_Q);
//
//    if (pg7_cmd64 > PG7PER)
//        pg7_cmd64 = PG7PER;
//    else if (pg7_cmd64 < 0)
//        pg7_cmd64 = 0;
//
//    PG7DC = (uint16_t)pg7_cmd64;
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_IREF)
//
//    PG7DC = PG7_MapSigned(i_ref_count, -500, 500);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_LOCK)
//
//    if (pll_locked)
//        PG7DC = (uint16_t)((int64_t)PG7PER * 80 / 100);
//    else
//        PG7DC = (uint16_t)((int64_t)PG7PER * 20 / 100);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_LOCK_BUZZ)
//
//    /*
//       PG7 buzzer/indicator mode.
//       Locked: short beep pulse, repeated about once per second.
//       Unlocked: output off.
//    */
//    {
//        static uint16_t buzz_cnt = 0;
//
//        if (pll_locked)
//        {
//            buzz_cnt++;
//
//            if (buzz_cnt >= FS_HZ)
//                buzz_cnt = 0;
//
//            if (buzz_cnt < (FS_HZ / 10))
//                PG7DC = (uint16_t)((int64_t)PG7PER * 50 / 100);
//            else
//                PG7DC = 0;
//        }
//        else
//        {
//            buzz_cnt = 0;
//            PG7DC = 0;
//        }
//    }
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_INPUT_VALID)
//
//    if (input_valid)
//        PG7DC = (uint16_t)((int64_t)PG7PER * 80 / 100);
//    else
//        PG7DC = (uint16_t)((int64_t)PG7PER * 20 / 100);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_AN1_ISR_TOGGLE)
//
//    /*
//       Toggle PG7 duty every time AN1 ISR runs.
//       Measure the low-frequency envelope on PG7:
//           AN1 ISR rate = measured toggle frequency * 2
//    */
//    {
//        static uint8_t an1_isr_toggle = 0;
//
//        an1_isr_toggle ^= 1;
//
//        if (an1_isr_toggle)
//            PG7DC = (uint16_t)((int64_t)PG7PER * 50 / 100);
//        else
//            PG7DC = 0;
//    }
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_AN1_RATE_DUTY)
//
//    /*
//       PG7 is updated from PWM1 callback using a 40kHz time base.
//       Do not write PG7DC here.
//    */
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_AN1_AMP)
//
//    /*
//       AN1 input amplitude.
//       If AN1 is about ?800 counts, PG7 should be around 80%.
//    */
//    PG7_OutputAmplitude(v_in_count, 2200);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_VA_AMP)
//
//    /*
//       SOGI va amplitude.
//       Should be close to AN1 amplitude.
//    */
//    dbg_val = PLL_Shift64(sogi_va, SOGI_STATE_Q);
//    PG7_OutputAmplitude(dbg_val, 2200);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_VB_AMP)
//
//    /*
//       SOGI vb amplitude.
//       Should be close to va amplitude.
//    */
//    dbg_val = PLL_Shift64(sogi_vb, SOGI_STATE_Q);
//    PG7_OutputAmplitude(dbg_val, 2200);
//#elif (PLL_DEBUG_SELECT == PLL_DBG_AN1_PERIOD)
//
//    /*
//       AN1 digital period in samples.
//       If fs = 1.8kHz and input = 60Hz,
//       period should be about 30 samples.
//       
//       Mapping:
//       0 samples    -> 0%
//       1000 samples -> 100%
//       30 samples   -> about 3.0%
//    */
//    PG7DC = PG7_MapSigned(an1_period_samples, 0, 1000);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_TEST_AN0)
//
//    PG7DC = PG7_MapSigned(dbg_test_an0_count, -500, 500);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_TEST_AN1)
//
//    PG7DC = PG7_MapSigned(dbg_test_an1_count, -2200, 2200);
//
//#elif (PLL_DEBUG_SELECT == PLL_DBG_TEST_SYNC)
//
//    PG7DC = PG7_MapSigned(dbg_test_sync_err, -500, 500);
//
//#else
//
//    PG7DC = 0;
//
//#endif
//}
//}

void ADC1_Setchannel_AN1InterruptHandler(void* handler)
{
    ADC1_channel_AN1DefaultInterruptHandler = handler;
}

void __attribute__((__interrupt__, auto_psv, weak)) _ADCAN1Interrupt(void)
{
    uint16_t valchannel_AN1 = ADCBUF1;

    if (ADC1_channel_AN1DefaultInterruptHandler)
    {
        ADC1_channel_AN1DefaultInterruptHandler(valchannel_AN1);
    }

    IFS5bits.ADCAN1IF = 0;
}

/**
  End of File
*/
