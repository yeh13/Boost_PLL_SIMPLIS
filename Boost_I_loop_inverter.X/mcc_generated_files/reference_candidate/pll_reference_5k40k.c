#include <stdint.h>
#include <stddef.h>

#define ADC_ISR_HZ          40000L
#define PLL_DECIM_N         8
#define FS_HZ               (ADC_ISR_HZ / PLL_DECIM_N)
#define ADC_CENTER          2048
#define PLL_TABLE_SIZE      360L
#define PLL_THETA_SUBDIV    512L
#define PLL_STEP_Q          12
#define PLL_STEP_SCALE      (1L << PLL_STEP_Q)
#define PLL_THETA_DEN_Q     (PLL_THETA_SUBDIV * PLL_STEP_SCALE)
#define PLL_THETA_DEN_SHIFT 21
#define PLL_WNOM_STEP_Q     ((int32_t)((60LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))
#define PLL_WMIN_STEP_Q     ((int32_t)((45LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))
#define PLL_WMAX_STEP_Q     ((int32_t)((75LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))
#define PLL_STEP_SLEW_Q     ((int32_t)((100LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / ((int64_t)FS_HZ * FS_HZ)))
#define PLL_FREQ_DEADBAND_Q ((int32_t)((1LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / (10LL * FS_HZ)))
#define PLL_TRIM_LIMIT_Q    ((int32_t)((15LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))
#define PLL_LOCK_ERR_TH     80
#define PLL_PI_DEADBAND_LOCK 5
#define PLL_LOCK_ON_ERR_TH  170
#define PLL_LOCK_OFF_ERR_TH 300
#define PLL_LOCK_COUNT_TH   20
#define PLL_UNLOCK_COUNT_TH 100
#define PLL_GRID_PHASE_ON_ERR_TH   25
#define PLL_GRID_PHASE_OFF_ERR_TH  45
#define PLL_GRID_PHASE_ON_COUNT_TH 20
#define PLL_GRID_PHASE_OFF_COUNT_TH 50
#define PLL_LPF_SHIFT       6
#define PLL_KP_SHIFT_FAST    2
#define PLL_KI_SHIFT_FAST   13
#define PLL_KP_SHIFT_LOCK   5
#define PLL_KI_SHIFT_LOCK   15
#define PLL_INT_LIMIT       ((int32_t)((30LL * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) / FS_HZ))
#define PLL_INT_LEAK_SHIFT  7
#define PLL_VMAG_MIN        100
#define PLL_ERR_SCALE       1000
#define PLL_ERR_LIMIT       1000
#define PLL_ERR_SIGN        1
#define PLL_ERR_BIAS        0
#define PLL_INPUT_VALID_AMP_COUNTS 300
#define PLL_INPUT_AMP_WINDOW_SAMPLES 100
#define GRID_OFFSET_Q       12
#define GRID_OFFSET_SHIFT   12
#define SOGI_STATE_Q        12
#define PLL_PHASE_COMP_DEG  4
#define PLL_OUTPUT_PHASE_COMP_DEG 0
#define SOGI60_A1_Q30   (-2033145701LL)
#define SOGI60_A2_Q30    (965191209LL)
#define SOGI60_B0_Q30    (54275307LL)
#define SOGI60_QB0_Q30   (2046131LL)
#define SOGI60_QB1_Q30   (4092262LL)
#define SOGI60_QB2_Q30   (2046131LL)

static const int16_t SinTable360[360] = {
    0, 572, 1144, 1715, 2286, 2856, 3425, 3993, 4560, 5126,
    5690, 6252, 6813, 7371, 7927, 8481, 9032, 9580, 10126, 10668,
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
    11207, 10668, 10126, 9580, 9032, 8481, 7927, 7371, 6813, 6252,
    5690, 5126, 4560, 3993, 3425, 2856, 2286, 1715, 1144, 572,
    0, -572, -1144, -1715, -2286, -2856, -3425, -3993, -4560, -5126,
    -5690, -6252, -6813, -7371, -7927, -8481, -9032, -9580, -10126, -10668,
    -11207, -11743, -12275, -12803, -13328, -13848, -14364, -14876, -15383, -15886,
    -16384, -16876, -17364, -17846, -18323, -18794, -19260, -19720, -20173, -20621,
    -21062, -21497, -21925, -22347, -22762, -23170, -23571, -23964, -24351, -24730,
    -25101, -25465, -25821, -26169, -26509, -26841, -27165, -27481, -27788, -28087,
    -28377, -28659, -28932, -29196, -29451, -29697, -29934, -30162, -30381, -30591,
    -30791, -30982, -31163, -31335, -31498, -31650, -31794, -31927, -32051, -32165,
    -32269, -32364, -32448, -32523, -32587, -32642, -32687, -32722, -32747, -32762,
    -32767, -32762, -32747, -32722, -32687, -32642, -32587, -32523, -32448, -32364,
    -32269, -32165, -32051, -31927, -31794, -31650, -31498, -31335, -31163, -30982,
    -30791, -30591, -30381, -30162, -29934, -29697, -29451, -29196, -28932, -28659,
    -28377, -28087, -27788, -27481, -27165, -26841, -26509, -26169, -25821, -25465,
    -25101, -24730, -24351, -23964, -23571, -23170, -22762, -22347, -21925, -21497,
    -21062, -20621, -20173, -19720, -19260, -18794, -18323, -17846, -17364, -16876,
    -16384, -15886, -15383, -14876, -14364, -13848, -13328, -12803, -12275, -11743,
    -11207, -10668, -10126, -9580, -9032, -8481, -7927, -7371, -6813, -6252,
    -5690, -5126, -4560, -3993, -3425, -2856, -2286, -1715, -1144, -572
};

static int32_t sat_i32(int32_t x, int32_t lo, int32_t hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static int32_t Q30_Round(int64_t x)
{
    if (x >= 0)
        return (int32_t)((x + (1LL << 29)) >> 30);
    return -(int32_t)(((-x) + (1LL << 29)) >> 30);
}

static int32_t Q15_Round(int64_t x)
{
    if (x >= 0)
        return (int32_t)((x + (1LL << 14)) >> 15);
    return -(int32_t)(((-x) + (1LL << 14)) >> 15);
}

static uint16_t pll_add_degrees(uint16_t idx, int16_t degrees)
{
    int16_t idx_tmp = (int16_t)idx + degrees;
    while (idx_tmp < 0) idx_tmp += PLL_TABLE_SIZE;
    while (idx_tmp >= PLL_TABLE_SIZE) idx_tmp -= PLL_TABLE_SIZE;
    return (uint16_t)idx_tmp;
}

static int32_t pll_sin_lookup(uint16_t idx)
{
    if (idx >= PLL_TABLE_SIZE) idx -= PLL_TABLE_SIZE;
    return SinTable360[idx];
}

static int32_t pll_sin_interp(uint16_t idx, int64_t theta_frac_q)
{
    uint16_t idx1 = idx + 1U;
    int32_t y0;
    int32_t y1;
    int32_t dy;
    int64_t den = PLL_THETA_DEN_Q;
    int64_t interp;

    if (idx1 >= PLL_TABLE_SIZE)
        idx1 = 0U;

    y0 = SinTable360[idx];
    y1 = SinTable360[idx1];
    dy = y1 - y0;

    if (dy >= 0)
        interp = (((int64_t)dy * theta_frac_q) + (den / 2)) >> 21;
    else
        interp = -((((int64_t)(-dy) * theta_frac_q) + (den / 2)) >> 21);

    return y0 + (int32_t)interp;
}

static int32_t pll_step_to_freq_x10(int32_t step_q)
{
    return (int32_t)(((int64_t)step_q * FS_HZ * 10LL) /
                      ((int64_t)PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE));
}

typedef struct
{
    /* 40 kHz ADC raw and 5 kHz PLL state */
    uint16_t decim_count;
    int32_t adc_raw;
    int32_t adc_centered;
    int32_t grid_offset_q;
    int32_t grid_offset;
    int32_t v_in_q;

    int32_t sogi_va_q12;
    int32_t sogi_vb_q12;
    int32_t sogi_vin_n1_q12;
    int32_t sogi_vin_n2_q12;
    int32_t sogi_va_n1_q12;
    int32_t sogi_va_n2_q12;
    int32_t sogi_vb_n1_q12;
    int32_t sogi_vb_n2_q12;

    int64_t sogi_a1_q30;
    int64_t sogi_a2_q30;
    int64_t sogi_b0_q30;
    int64_t sogi_qb0_q30;
    int64_t sogi_qb1_q30;
    int64_t sogi_qb2_q30;

    int32_t pll_err_lpf;
    int32_t pll_err_slow_lpf;
    int32_t pll_int_acc;
    int32_t pll_step_q;
    int32_t pll_step_target_q;
    int32_t w_delta_q;
    int64_t theta_acc_q;
    uint16_t pll_idx;
    uint8_t locked;
    uint8_t phase_ok;
    int16_t vref_count;
    int16_t iref_count;

    /* 40 kHz phase delivery derived from the 5 kHz PLL anchor */
    int64_t phase_anchor_q;
    int64_t phase_step_q;
    int64_t phase_40k_q;
    uint32_t phase_40k_tick;
    uint32_t phase_40k_wrap_count;
    uint32_t phase_update_count;
    int32_t freq_correction_q;
    int32_t vq_q;
    int32_t raw_err;
    int32_t abs_err_lpf;
    int32_t abs_err_phase;
    int32_t phase_step_40k_q;
} ref_pll_ctx_t;

static void ref_pll_ctx_init(ref_pll_ctx_t *ctx)
{
    ctx->decim_count = 0;
    ctx->adc_raw = 0;
    ctx->adc_centered = 0;
    ctx->grid_offset_q = ((int32_t)ADC_CENTER << GRID_OFFSET_Q);
    ctx->grid_offset = ADC_CENTER;
    ctx->v_in_q = 0;

    ctx->sogi_va_q12 = 0;
    ctx->sogi_vb_q12 = 0;
    ctx->sogi_vin_n1_q12 = 0;
    ctx->sogi_vin_n2_q12 = 0;
    ctx->sogi_va_n1_q12 = 0;
    ctx->sogi_va_n2_q12 = 0;
    ctx->sogi_vb_n1_q12 = 0;
    ctx->sogi_vb_n2_q12 = 0;

    ctx->sogi_a1_q30 = SOGI60_A1_Q30;
    ctx->sogi_a2_q30 = SOGI60_A2_Q30;
    ctx->sogi_b0_q30 = SOGI60_B0_Q30;
    ctx->sogi_qb0_q30 = SOGI60_QB0_Q30;
    ctx->sogi_qb1_q30 = SOGI60_QB1_Q30;
    ctx->sogi_qb2_q30 = SOGI60_QB2_Q30;

    ctx->pll_err_lpf = 0;
    ctx->pll_err_slow_lpf = 0;
    ctx->pll_int_acc = 0;
    ctx->pll_step_q = PLL_WNOM_STEP_Q;
    ctx->pll_step_target_q = PLL_WNOM_STEP_Q;
    ctx->w_delta_q = 0;
    ctx->theta_acc_q = 0;
    ctx->pll_idx = 0;
    ctx->locked = 0;
    ctx->phase_ok = 0;
    ctx->vref_count = 0;
    ctx->iref_count = 0;

    ctx->phase_anchor_q = 0;
    ctx->phase_step_q = 0;
    ctx->phase_40k_q = 0;
    ctx->phase_40k_tick = 0;
    ctx->phase_40k_wrap_count = 0;
    ctx->phase_update_count = 0;
    ctx->freq_correction_q = 0;
    ctx->vq_q = 0;
    ctx->raw_err = 0;
    ctx->abs_err_lpf = 0;
    ctx->abs_err_phase = 0;
    ctx->phase_step_40k_q = 0;
}

static int32_t PLL_Shift64(int64_t x, uint8_t q)
{
    if (x >= 0)
        return (int32_t)(x >> q);
    return -(int32_t)(((-x) + ((1LL << q) - 1LL)) >> q);
}

static void ref_pll_load_sogi_coeff(ref_pll_ctx_t *ctx)
{
    (void)ctx;
    ctx->sogi_a1_q30 = SOGI60_A1_Q30;
    ctx->sogi_a2_q30 = SOGI60_A2_Q30;
    ctx->sogi_b0_q30 = SOGI60_B0_Q30;
    ctx->sogi_qb0_q30 = SOGI60_QB0_Q30;
    ctx->sogi_qb1_q30 = SOGI60_QB1_Q30;
    ctx->sogi_qb2_q30 = SOGI60_QB2_Q30;
}

static int32_t ref_pll_phase_step_q_from_freq_x10(int32_t freq_x10)
{
    int64_t step_q;
    if (freq_x10 < 450) freq_x10 = 450;
    else if (freq_x10 > 750) freq_x10 = 750;
    step_q = ((int64_t)freq_x10 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE) /
             ((int64_t)FS_HZ * 10LL);
    if (step_q > PLL_WMAX_STEP_Q) step_q = PLL_WMAX_STEP_Q;
    else if (step_q < PLL_WMIN_STEP_Q) step_q = PLL_WMIN_STEP_Q;
    return (int32_t)step_q;
}

static void ref_pll_update_40k_phase(ref_pll_ctx_t *ctx, uint32_t tick_index)
{
    int64_t full_q = (int64_t)PLL_TABLE_SIZE * PLL_THETA_DEN_Q;
    int64_t phase_q = ctx->phase_anchor_q + ((int64_t)tick_index * ctx->phase_step_40k_q);

    while (phase_q >= full_q)
    {
        phase_q -= full_q;
        ctx->phase_40k_wrap_count++;
    }
    while (phase_q < 0)
    {
        phase_q += full_q;
        ctx->phase_40k_wrap_count++;
    }

    ctx->phase_40k_q = phase_q;
    ctx->phase_40k_tick = tick_index;
    ctx->phase_update_count++;
}

static void ref_pll_advance_5k_anchor(ref_pll_ctx_t *ctx)
{
    int64_t full_q = (int64_t)PLL_TABLE_SIZE * PLL_THETA_DEN_Q;
    ctx->phase_anchor_q = ((int64_t)ctx->pll_idx * PLL_THETA_DEN_Q) + ctx->theta_acc_q;
    while (ctx->phase_anchor_q >= full_q) ctx->phase_anchor_q -= full_q;
    while (ctx->phase_anchor_q < 0) ctx->phase_anchor_q += full_q;

    ctx->phase_step_q = (int64_t)ctx->pll_step_q;
    ctx->phase_step_40k_q = (ctx->phase_step_q + 4LL) >> 3;
    if (ctx->phase_step_40k_q == 0) ctx->phase_step_40k_q = 1;

    /* This matches the requirement: 5k phase anchor is updated first,
       then 8 x 40k ticks move forward using phase_step_40k_q. */
    ctx->phase_40k_tick = 0;
    ctx->phase_40k_q = ctx->phase_anchor_q;
}

static void ref_pll_process_5k(ref_pll_ctx_t *ctx, uint16_t adc_raw)
{
    int32_t v_in_count;
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
    int32_t mag_max;
    int32_t mag_min;
    int32_t vmag_q;
    int32_t vmag_min_q;
    int32_t abs_err_lpf;
    int32_t abs_err_phase;
    int32_t pll_err_pi;
    int32_t w_delta_q;
    int32_t pll_step_diff_q;
    uint16_t step_degrees;
    uint16_t theta_det_idx;
    int32_t pll_step_target_q;
    int64_t full_q;

    ctx->adc_raw = (int32_t)adc_raw;
    {
        int32_t grid_offset_err_q = ((int32_t)adc_raw << GRID_OFFSET_Q) - ctx->grid_offset_q;
        int32_t grid_offset_delta_q;

        if (grid_offset_err_q >= 0)
            grid_offset_delta_q = grid_offset_err_q >> GRID_OFFSET_SHIFT;
        else
            grid_offset_delta_q = -((-grid_offset_err_q) >> GRID_OFFSET_SHIFT);

        ctx->grid_offset_q += grid_offset_delta_q;
        ctx->grid_offset = (ctx->grid_offset_q + (1L << (GRID_OFFSET_Q - 1))) >> GRID_OFFSET_Q;
    }

    v_in_count = (int32_t)adc_raw - ctx->grid_offset;
    ctx->adc_centered = v_in_count;
    ctx->v_in_q = v_in_count * (1L << SOGI_STATE_Q);

    acc_va = 0;
    acc_va += ctx->sogi_b0_q30 * ctx->v_in_q;
    acc_va -= ctx->sogi_b0_q30 * ctx->sogi_vin_n2_q12;
    acc_va -= ctx->sogi_a1_q30 * ctx->sogi_va_n1_q12;
    acc_va -= ctx->sogi_a2_q30 * ctx->sogi_va_n2_q12;

    acc_vb = 0;
    acc_vb += ctx->sogi_qb0_q30 * ctx->v_in_q;
    acc_vb += ctx->sogi_qb1_q30 * ctx->sogi_vin_n1_q12;
    acc_vb += ctx->sogi_qb2_q30 * ctx->sogi_vin_n2_q12;
    acc_vb -= ctx->sogi_a1_q30 * ctx->sogi_vb_n1_q12;
    acc_vb -= ctx->sogi_a2_q30 * ctx->sogi_vb_n2_q12;

    va_new = Q30_Round(acc_va);
    vb_new = Q30_Round(acc_vb);

    ctx->sogi_va_q12 = va_new;
    ctx->sogi_vb_q12 = vb_new;
    ctx->sogi_vin_n2_q12 = ctx->sogi_vin_n1_q12;
    ctx->sogi_vin_n1_q12 = ctx->v_in_q;
    ctx->sogi_va_n2_q12 = ctx->sogi_va_n1_q12;
    ctx->sogi_va_n1_q12 = va_new;
    ctx->sogi_vb_n2_q12 = ctx->sogi_vb_n1_q12;
    ctx->sogi_vb_n1_q12 = vb_new;

    ref_pll_load_sogi_coeff(ctx);
    theta_det_idx = pll_add_degrees(ctx->pll_idx, PLL_PHASE_COMP_DEG);
    sin_theta = pll_sin_interp(theta_det_idx, ctx->theta_acc_q);
    cos_theta = pll_sin_interp(pll_add_degrees(theta_det_idx, 90), ctx->theta_acc_q);

    vq_q = Q15_Round(((int64_t)ctx->sogi_va_q12 * cos_theta) +
                     ((int64_t)ctx->sogi_vb_q12 * sin_theta));
    ctx->vq_q = vq_q;

    abs_va = (ctx->sogi_va_q12 >= 0) ? ctx->sogi_va_q12 : -ctx->sogi_va_q12;
    abs_vb = (ctx->sogi_vb_q12 >= 0) ? ctx->sogi_vb_q12 : -ctx->sogi_vb_q12;

    if (abs_va >= abs_vb) {
        mag_max = abs_va;
        mag_min = abs_vb;
    } else {
        mag_max = abs_vb;
        mag_min = abs_va;
    }

    vmag_q = mag_max + (mag_min >> 1);
    vmag_min_q = PLL_VMAG_MIN * (1L << SOGI_STATE_Q);
    if (vmag_q < vmag_min_q) vmag_q = vmag_min_q;

    raw_err = (int32_t)(((int64_t)PLL_ERR_SIGN * vq_q * PLL_ERR_SCALE) / vmag_q);
    raw_err -= PLL_ERR_BIAS;
    raw_err = sat_i32(raw_err, -PLL_ERR_LIMIT, PLL_ERR_LIMIT);
    ctx->raw_err = raw_err;

    ctx->pll_err_lpf = ctx->pll_err_lpf + ((raw_err - ctx->pll_err_lpf) >> PLL_LPF_SHIFT);
    ctx->pll_err_slow_lpf += ((((int32_t)ctx->pll_err_lpf << 8) - ctx->pll_err_slow_lpf) >> 5);

    abs_err_lpf = (ctx->pll_err_lpf >= 0) ? ctx->pll_err_lpf : -ctx->pll_err_lpf;
    abs_err_phase = (ctx->pll_err_slow_lpf >= 0) ? ctx->pll_err_slow_lpf : -ctx->pll_err_slow_lpf;
    ctx->abs_err_lpf = abs_err_lpf;
    ctx->abs_err_phase = abs_err_phase;

    if (abs_err_phase <= PLL_GRID_PHASE_ON_ERR_TH) {
        ctx->phase_ok = 1;
    } else if (abs_err_phase > PLL_GRID_PHASE_OFF_ERR_TH) {
        ctx->phase_ok = 0;
    }

    pll_err_pi = ctx->pll_err_lpf;
    if ((abs_err_lpf <= PLL_LOCK_ERR_TH) &&
        (pll_err_pi <= PLL_PI_DEADBAND_LOCK) &&
        (pll_err_pi >= -PLL_PI_DEADBAND_LOCK)) {
        pll_err_pi = 0;
    }

    if ((pll_err_pi == 0) && (abs_err_lpf <= PLL_LOCK_ERR_TH)) {
        /* no-integrator leak in this simplified candidate; exact parity is retained in the alpha/beta loop */
    } else {
        ctx->pll_int_acc += pll_err_pi;
    }

    ctx->pll_int_acc = sat_i32(ctx->pll_int_acc, -PLL_INT_LIMIT, PLL_INT_LIMIT);

    if ((ctx->locked == 0) && (abs_err_lpf > PLL_LOCK_ERR_TH)) {
        w_delta_q = (int32_t)PLL_Shift64((int64_t)pll_err_pi * PLL_STEP_SCALE, PLL_KP_SHIFT_FAST) +
                    (int32_t)PLL_Shift64((int64_t)ctx->pll_int_acc * PLL_STEP_SCALE, PLL_KI_SHIFT_FAST);
    } else {
        w_delta_q = (int32_t)PLL_Shift64((int64_t)pll_err_pi * PLL_STEP_SCALE, PLL_KP_SHIFT_LOCK) +
                    (int32_t)PLL_Shift64((int64_t)ctx->pll_int_acc * PLL_STEP_SCALE, PLL_KI_SHIFT_LOCK);
    }
    w_delta_q = sat_i32(w_delta_q, -PLL_TRIM_LIMIT_Q, PLL_TRIM_LIMIT_Q);
    ctx->w_delta_q = w_delta_q;

    ctx->pll_step_target_q = PLL_WNOM_STEP_Q + w_delta_q;
    if (ctx->pll_step_target_q > PLL_WMAX_STEP_Q) ctx->pll_step_target_q = PLL_WMAX_STEP_Q;
    else if (ctx->pll_step_target_q < PLL_WMIN_STEP_Q) ctx->pll_step_target_q = PLL_WMIN_STEP_Q;

    pll_step_diff_q = ctx->pll_step_target_q - ctx->pll_step_q;
    if ((pll_step_diff_q <= PLL_FREQ_DEADBAND_Q) && (pll_step_diff_q >= -PLL_FREQ_DEADBAND_Q)) pll_step_diff_q = 0;
    if (pll_step_diff_q > PLL_STEP_SLEW_Q) pll_step_diff_q = PLL_STEP_SLEW_Q;
    else if (pll_step_diff_q < -PLL_STEP_SLEW_Q) pll_step_diff_q = -PLL_STEP_SLEW_Q;
    ctx->pll_step_q += pll_step_diff_q;
    ctx->freq_correction_q = ctx->pll_step_q;

    ctx->theta_acc_q += ctx->pll_step_q;
    step_degrees = (uint16_t)(ctx->theta_acc_q >> PLL_THETA_DEN_SHIFT);
    ctx->theta_acc_q -= ((int64_t)step_degrees << PLL_THETA_DEN_SHIFT);
    ctx->pll_idx += step_degrees;
    if (ctx->pll_idx >= PLL_TABLE_SIZE) ctx->pll_idx -= PLL_TABLE_SIZE;

    if ((abs_err_lpf <= PLL_LOCK_ON_ERR_TH)) {
        ctx->locked = 1;
    } else if (abs_err_lpf > PLL_LOCK_OFF_ERR_TH) {
        ctx->locked = 0;
    }

    ref_pll_advance_5k_anchor(ctx);
}

int main(void)
{
    ref_pll_ctx_t ctx;
    uint16_t sample_40k = 0;
    uint32_t tick;
    uint32_t sample_5k = 0;

    ref_pll_ctx_init(&ctx);

    for (sample_40k = 0; sample_40k < 4000; ++sample_40k)
    {
        uint16_t adc_raw = (uint16_t)(ADC_CENTER + 900 + (int32_t)((sample_40k % 200) - 100));
        ctx.decim_count++;

        if (ctx.decim_count >= PLL_DECIM_N)
        {
            ctx.decim_count = 0;
            ref_pll_process_5k(&ctx, adc_raw);
            sample_5k++;

            /* For the next 8 40 kHz ticks, the phase at 40 kHz is derived from the fresh 5 kHz PLL anchor. */
            for (tick = 0; tick < 8; ++tick)
            {
                ref_pll_update_40k_phase(&ctx, tick);
            }
        }
    }

    return 0;
}
