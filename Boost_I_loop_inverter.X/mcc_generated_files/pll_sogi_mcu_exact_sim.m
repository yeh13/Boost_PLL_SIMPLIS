clear; clc; close all;

%% ============================================================
%  MCU-exact SOGI / SOGI-PLL simulation
%
%  This file mirrors the fixed-point math used in adc1.c:
%    - ADC callback rate = 40 kHz
%    - PLL/SOGI decimation = 8, effective fs = 5 kHz
%    - SOGI coefficients = Q30
%    - SOGI states = Q12 counts
%    - sin/cos table = Q15
%    - C-style signed shifts and rounding
%
%  MODE = "diag" reproduces the current MCU AN1 diagnostic callback.
%  The current adc1.c diagnostic path keeps pll_locked = 0 on purpose.
%
%  MODE = "pll" runs the same fixed-point SOGI plus the PLL section so
%  frequency/error behavior can be checked before moving code back to MCU.
%% ============================================================

MODE = "pll";                % "diag" or "pll"

ADC_ISR_HZ = 40000;
PLL_DECIM_N = 8;
FS_HZ = ADC_ISR_HZ / PLL_DECIM_N;

FIN_LIST_HZ = [55 60 65];
SIM_CYCLES = 80;
ADC_CENTER = 2048;
AMP_COUNTS = 1860;
PHASE_DEG = 0;

PLL_TABLE_SIZE = 360;
PLL_THETA_SUBDIV = 512;
PLL_STEP_Q = 12;
PLL_STEP_SCALE = bitshift(1, PLL_STEP_Q);
PLL_THETA_DEN_SHIFT = 21;

PLL_WNOM_STEP_Q = floor(60 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);
PLL_WMIN_STEP_Q = floor(45 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);
PLL_WMAX_STEP_Q = floor(75 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);
PLL_W57P5_STEP_Q = floor(57.5 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);
PLL_W62P5_STEP_Q = floor(62.5 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);

PLL_LOCK_ERR_TH = 80;
PLL_PI_DEADBAND_LOCK = 0;
PLL_LOCK_ON_ERR_TH = 170;
PLL_LOCK_OFF_ERR_TH = 300;
PLL_LOCK_COUNT_TH = 20;

PLL_KP_SHIFT_FAST = 2;
PLL_KI_SHIFT_FAST = 12;
PLL_KP_SHIFT_LOCK = 2;
PLL_KI_SHIFT_LOCK = 12;

PLL_INT_LIMIT = 120000;
PLL_INT_LEAK_SHIFT = 7;
GRID_OFFSET_SHIFT = 12;
PLL_LPF_SHIFT = 6;
PLL_FF_STEP_LPF_SHIFT = 4;
PLL_STEP_SLEW_Q = floor(100 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / (FS_HZ * FS_HZ));
PLL_TRIM_LIMIT_Q = floor(15 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);
PLL_USE_ZC_FEED_FORWARD = 0;

PLL_VMAG_MIN = 100;
PLL_ERR_SCALE = 1000;
PLL_ERR_LIMIT = 1000;
PLL_ERR_SIGN = 1;
PLL_ERR_BIAS = 0;
PLL_INPUT_VALID_AMP_COUNTS = 300;
PLL_INPUT_AMP_WINDOW_SAMPLES = 100;
PLL_SYNC_TO_AN1_ZC = 0;
PLL_DEMO_LOCK_ON_ZC = 0;
PLL_PHASE_COMP_DEG = 0;

SOGI_STATE_Q = 12;
IREF_PEAK_COUNTS = 458;
VREF_PEAK = 3787;

SOGI60_A1_Q30  = -2033145701;
SOGI60_A2_Q30  =  965191209;
SOGI60_B0_Q30  =   54275307;
SOGI60_QB0_Q30 =    2046131;
SOGI60_QB1_Q30 =    4092262;
SOGI60_QB2_Q30 =    2046131;

sin_table = round(sin((0:359) * pi / 180) * 32767);
sin_table(271) = -32767;     % match old MCU/MATLAB table quirk

fprintf('MCU-exact simulation mode=%s, ADC=%d Hz, decim=%d, SOGI/PLL fs=%d Hz\n', ...
    MODE, ADC_ISR_HZ, PLL_DECIM_N, FS_HZ);
fprintf('Fixed SOGI = 60Hz Q30, state = Q%d, sin/cos = Q15\n\n', SOGI_STATE_Q);

all_result = cell(1, numel(FIN_LIST_HZ));

fprintf('fin  vaAvg vaMin vaMax  vbAvg vbMin vbMax  errAvg errPkPk freqAvg lockEnd\n');
fprintf('---  ----- ----- -----  ----- ----- -----  ------ ------- ------- -------\n');

for k = 1:numel(FIN_LIST_HZ)
    fin = FIN_LIST_HZ(k);
    r = run_mcu_case(fin, MODE, ADC_ISR_HZ, PLL_DECIM_N, SIM_CYCLES, ...
        ADC_CENTER, AMP_COUNTS, PHASE_DEG, sin_table, ...
        PLL_TABLE_SIZE, PLL_THETA_SUBDIV, PLL_STEP_Q, PLL_STEP_SCALE, PLL_THETA_DEN_SHIFT, ...
        PLL_WNOM_STEP_Q, PLL_WMIN_STEP_Q, PLL_WMAX_STEP_Q, PLL_W57P5_STEP_Q, PLL_W62P5_STEP_Q, ...
        PLL_LOCK_ERR_TH, PLL_PI_DEADBAND_LOCK, PLL_LOCK_ON_ERR_TH, PLL_LOCK_OFF_ERR_TH, PLL_LOCK_COUNT_TH, ...
        PLL_KP_SHIFT_FAST, PLL_KI_SHIFT_FAST, PLL_KP_SHIFT_LOCK, PLL_KI_SHIFT_LOCK, ...
        PLL_INT_LIMIT, PLL_INT_LEAK_SHIFT, GRID_OFFSET_SHIFT, PLL_LPF_SHIFT, PLL_FF_STEP_LPF_SHIFT, ...
        PLL_STEP_SLEW_Q, PLL_TRIM_LIMIT_Q, PLL_USE_ZC_FEED_FORWARD, ...
        PLL_VMAG_MIN, PLL_ERR_SCALE, PLL_ERR_LIMIT, PLL_ERR_SIGN, PLL_ERR_BIAS, ...
        PLL_INPUT_VALID_AMP_COUNTS, PLL_INPUT_AMP_WINDOW_SAMPLES, PLL_SYNC_TO_AN1_ZC, PLL_DEMO_LOCK_ON_ZC, ...
        PLL_PHASE_COMP_DEG, SOGI_STATE_Q, IREF_PEAK_COUNTS, VREF_PEAK, ...
        SOGI60_A1_Q30, SOGI60_A2_Q30, SOGI60_B0_Q30, SOGI60_QB0_Q30, SOGI60_QB1_Q30, SOGI60_QB2_Q30);

    all_result{k} = r;

    last_n = round(3 * r.fs / fin);
    idx = (numel(r.t)-last_n+1):numel(r.t);

    fprintf('%3.0f  %+5.0f %+5.0f %+5.0f  %+5.0f %+5.0f %+5.0f  %+6.1f %7.1f %7.3f %7d\n', ...
        fin, mean(r.va_count(idx)), min(r.va_count(idx)), max(r.va_count(idx)), ...
        mean(r.vb_count(idx)), min(r.vb_count(idx)), max(r.vb_count(idx)), ...
        mean(r.err_lpf(idx)), max(r.err_lpf(idx))-min(r.err_lpf(idx)), ...
        mean(r.freq_hz(idx)), r.lock(end));
end

r = all_result{1};
figure;
subplot(5,1,1);
plot(r.t, r.raw_adc, r.t, r.grid_offset);
grid on; legend('raw ADC','offset'); ylabel('ADC');

subplot(5,1,2);
plot(r.t, r.v_in_count, r.t, r.va_count, r.t, r.vb_count);
grid on; legend('vin count','SOGI va','SOGI vb'); ylabel('counts');

subplot(5,1,3);
plot(r.t, r.pg7_va_duty, r.t, r.pg7_vb_duty);
grid on; legend('PG7 VA duty','PG7 VB duty'); ylabel('%');

subplot(5,1,4);
plot(r.t, r.err_raw, r.t, r.err_lpf);
grid on; legend('raw err','err lpf'); ylabel('err');

subplot(5,1,5);
plot(r.t, r.freq_hz, r.t, r.lock * 10 + 45);
grid on; legend('PLL freq','lock marker'); ylabel('Hz'); xlabel('s');

function r = run_mcu_case(fin, mode, adc_hz, decim_n, sim_cycles, adc_center, amp_counts, phase_deg, ...
    sin_table, table_size, theta_subdiv, step_q, step_scale, theta_den_shift, ...
    wnom_q, wmin_q, wmax_q, w57p5_q, w62p5_q, ...
    lock_err_th, pi_deadband_lock, lock_on_err_th, lock_off_err_th, lock_count_th, ...
    kp_shift_fast, ki_shift_fast, kp_shift_lock, ki_shift_lock, ...
    int_limit, int_leak_shift, offset_shift, lpf_shift, ff_step_lpf_shift, ...
    step_slew_q, trim_limit_q, use_zc_ff, ...
    vmag_min, err_scale, err_limit, err_sign, err_bias, ...
    input_valid_amp_counts, input_amp_window_samples, sync_to_zc, demo_lock_on_zc, ...
    phase_comp_deg, sogi_state_q, iref_peak_counts, vref_peak, ...
    a1_q30, a2_q30, b0_q30, qb0_q30, qb1_q30, qb2_q30)

    fs = adc_hz / decim_n;
    n_adc = round(adc_hz / fin * sim_cycles);
    raw_adc_all = round(adc_center + amp_counts * sin(2*pi*fin*(0:n_adc-1)/adc_hz + phase_deg*pi/180));

    n_pll = floor(n_adc / decim_n);
    t = (0:n_pll-1) / fs;

    grid_offset = adc_center;
    grid_offset_q = adc_center * 2^offset_shift;
    an1_amp_min = 0;
    an1_amp_max = 0;
    an1_amp_counts = 0;
    an1_amp_cnt = 0;
    input_valid = 0;
    an1_prev_count = 0;
    an1_zc_seen = 0;

    sogi_vin_n1 = 0; sogi_vin_n2 = 0;
    sogi_va_n1 = 0; sogi_va_n2 = 0;
    sogi_vb_n1 = 0; sogi_vb_n2 = 0;
    sogi_va = 0; sogi_vb = 0;

    pll_ff_step_q = wnom_q;
    pll_ff_step_target_q = wnom_q;
    pll_err_lpf = 0;
    pll_int_acc = 0;
    pll_step_q = wnom_q;
    theta_acc_q = 0;
    pll_idx = 0;
    pll_lock_count = 0;
    pll_locked = 0;
    pll_vmag_recip_q28 = 0;
    pll_recip_slow_cnt = 0;

    raw_adc = zeros(1, n_pll);
    offset_log = zeros(1, n_pll);
    vin_log = zeros(1, n_pll);
    va_count = zeros(1, n_pll);
    vb_count = zeros(1, n_pll);
    err_raw_log = zeros(1, n_pll);
    err_lpf_log = zeros(1, n_pll);
    freq_log = zeros(1, n_pll);
    lock_log = zeros(1, n_pll);
    pg7_va_duty = zeros(1, n_pll);
    pg7_vb_duty = zeros(1, n_pll);

    m = 0;
    for n = 1:n_adc
        if mod(n-1, decim_n) ~= decim_n-1
            continue;
        end

        m = m + 1;
        adc_val = raw_adc_all(n);
        raw_err = 0;

        grid_offset_q = grid_offset_q + adc_val - grid_offset;
        grid_offset = floor(grid_offset_q / 2^offset_shift);
        v_in_count = adc_val - grid_offset;

        if an1_amp_cnt == 0
            an1_amp_min = v_in_count;
            an1_amp_max = v_in_count;
        else
            if v_in_count < an1_amp_min
                an1_amp_min = v_in_count;
            end
            if v_in_count > an1_amp_max
                an1_amp_max = v_in_count;
            end
        end

        an1_amp_cnt = an1_amp_cnt + 1;
        if an1_amp_cnt >= input_amp_window_samples
            an1_amp_counts = floor((an1_amp_max - an1_amp_min) / 2);
            input_valid = an1_amp_counts >= input_valid_amp_counts;
            an1_amp_cnt = 0;
        end

        if strcmp(mode, "pll") && ~input_valid
            pll_locked = 0;
            pll_lock_count = 0;
            pll_err_lpf = 0;
            pll_int_acc = 0;
            pll_ff_step_target_q = wnom_q;
            pll_ff_step_q = wnom_q;
            pll_step_q = wnom_q;
            theta_acc_q = 0;
            pll_idx = 0;
            sogi_va = 0; sogi_vb = 0;
            sogi_va_n1 = 0; sogi_va_n2 = 0;
            sogi_vb_n1 = 0; sogi_vb_n2 = 0;
            sogi_vin_n1 = 0; sogi_vin_n2 = 0;
        else
            if (an1_prev_count < 0) && (v_in_count >= 0)
                if sync_to_zc ~= 0
                    pll_idx = 0;
                    theta_acc_q = -wnom_q;
                    pll_step_q = wnom_q;
                end
                an1_zc_seen = 1;
            end
            an1_prev_count = v_in_count;

            v_in_q = v_in_count * 2^sogi_state_q;

            acc_va = int64(b0_q30) * int64(v_in_q) ...
                   - int64(b0_q30) * int64(sogi_vin_n2) ...
                   - int64(a1_q30) * int64(sogi_va_n1) ...
                   - int64(a2_q30) * int64(sogi_va_n2);

            acc_vb = int64(qb0_q30) * int64(v_in_q) ...
                   + int64(qb1_q30) * int64(sogi_vin_n1) ...
                   + int64(qb2_q30) * int64(sogi_vin_n2) ...
                   - int64(a1_q30) * int64(sogi_vb_n1) ...
                   - int64(a2_q30) * int64(sogi_vb_n2);

            va_new = q30_round(acc_va);
            vb_new = q30_round(acc_vb);

            sogi_va = va_new;
            sogi_vb = vb_new;
            sogi_vin_n2 = sogi_vin_n1;
            sogi_vin_n1 = v_in_q;
            sogi_va_n2 = sogi_va_n1;
            sogi_va_n1 = va_new;
            sogi_vb_n2 = sogi_vb_n1;
            sogi_vb_n1 = vb_new;

            if strcmp(mode, "pll")
                theta_comp_idx = mod(pll_idx + phase_comp_deg, table_size);
                sin_theta = sin_lookup(sin_table, theta_comp_idx);
                cos_theta = sin_lookup(sin_table, theta_comp_idx + 90);

                vq_q = q15_round(int64(sogi_va) * int64(cos_theta) + int64(sogi_vb) * int64(sin_theta));

                abs_va = abs(sogi_va);
                abs_vb = abs(sogi_vb);
                vmag_q = max(abs_va, abs_vb);
                vmag_min_q = vmag_min * 2^sogi_state_q;
                if vmag_q < vmag_min_q
                    vmag_q = vmag_min_q;
                end

                pll_recip_slow_cnt = pll_recip_slow_cnt + 1;
                if pll_recip_slow_cnt >= 8 || pll_vmag_recip_q28 == 0
                    pll_recip_slow_cnt = 0;
                    pll_vmag_recip_q28 = floor(2^28 / double(vmag_q));
                    if pll_vmag_recip_q28 < 1
                        pll_vmag_recip_q28 = 1;
                    end
                end

                raw_err = floor(double(err_sign * int64(vq_q) * int64(err_scale) * int64(pll_vmag_recip_q28)) / 2^28);
                raw_err = raw_err - err_bias;
                raw_err = min(max(raw_err, -err_limit), err_limit);

                pll_err_lpf = pll_err_lpf + shift_c(raw_err - pll_err_lpf, lpf_shift);

                abs_err_lpf = abs(pll_err_lpf);
                if abs_err_lpf > lock_err_th
                    kp_shift = kp_shift_fast;
                    ki_shift = ki_shift_fast;
                    deadband = 0;
                    lock_mode = 0;
                else
                    kp_shift = kp_shift_lock;
                    ki_shift = ki_shift_lock;
                    deadband = pi_deadband_lock;
                    lock_mode = 1;
                end

                pll_err_pi = pll_err_lpf;
                if pll_err_pi <= deadband && pll_err_pi >= -deadband
                    pll_err_pi = 0;
                end

                if pll_err_pi == 0 && lock_mode == 1
                    if pll_int_acc > 0
                        leak = max(1, shift_c(pll_int_acc, int_leak_shift));
                        pll_int_acc = max(0, pll_int_acc - leak);
                    elseif pll_int_acc < 0
                        leak = max(1, shift_c(-pll_int_acc, int_leak_shift));
                        pll_int_acc = min(0, pll_int_acc + leak);
                    end
                else
                    pll_int_acc = pll_int_acc + pll_err_pi;
                end

                pll_int_acc = min(max(pll_int_acc, -int_limit), int_limit);

                w_delta_q = shift_c(int64(pll_err_pi) * int64(step_scale), kp_shift) + ...
                            shift_c(int64(pll_int_acc) * int64(step_scale), ki_shift);
                w_delta_q = min(max(w_delta_q, -trim_limit_q), trim_limit_q);

                if use_zc_ff ~= 0
                    pll_base_step_q = pll_ff_step_q;
                else
                    pll_base_step_q = wnom_q;
                end

                pll_step_target_q = pll_base_step_q + w_delta_q;
                pll_step_target_q = min(max(pll_step_target_q, wmin_q), wmax_q);

                pll_step_diff_q = pll_step_target_q - pll_step_q;
                pll_step_diff_q = min(max(pll_step_diff_q, -step_slew_q), step_slew_q);
                pll_step_q = pll_step_q + pll_step_diff_q;

                theta_acc_q = theta_acc_q + pll_step_q;
                step_degrees = floor(double(theta_acc_q) / 2^theta_den_shift);
                theta_acc_q = theta_acc_q - int64(step_degrees) * int64(2^theta_den_shift);
                pll_idx = pll_idx + step_degrees;
                if pll_idx >= table_size
                    pll_idx = pll_idx - table_size;
                elseif pll_idx < 0
                    pll_idx = pll_idx + table_size;
                end

                if demo_lock_on_zc ~= 0
                    pll_locked = an1_zc_seen ~= 0;
                elseif pll_err_lpf <= lock_on_err_th && pll_err_lpf >= -lock_on_err_th
                    pll_lock_count = min(pll_lock_count + 1, lock_count_th);
                    if pll_lock_count >= lock_count_th
                        pll_locked = 1;
                    end
                elseif pll_err_lpf > lock_off_err_th || pll_err_lpf < -lock_off_err_th
                    pll_lock_count = 0;
                    pll_locked = 0;
                end
            else
                raw_err = 0;
                pll_err_lpf = 0;
                pll_locked = 0;
            end
        end

        raw_adc(m) = adc_val;
        offset_log(m) = grid_offset;
        vin_log(m) = v_in_count;
        va_count(m) = shift_c(sogi_va, sogi_state_q);
        vb_count(m) = shift_c(sogi_vb, sogi_state_q);
        err_raw_log(m) = raw_err;
        err_lpf_log(m) = pll_err_lpf;
        freq_log(m) = double(pll_step_q) * fs / (table_size * theta_subdiv * step_scale);
        lock_log(m) = pll_locked;
        pg7_va_duty(m) = pg7_map_signed(va_count(m), -2200, 2200);
        pg7_vb_duty(m) = pg7_map_signed(vb_count(m), -2200, 2200);
    end

    r.fs = fs;
    r.t = t;
    r.raw_adc = raw_adc;
    r.grid_offset = offset_log;
    r.v_in_count = vin_log;
    r.va_count = va_count;
    r.vb_count = vb_count;
    r.err_raw = err_raw_log;
    r.err_lpf = err_lpf_log;
    r.freq_hz = freq_log;
    r.lock = lock_log;
    r.pg7_va_duty = pg7_va_duty;
    r.pg7_vb_duty = pg7_vb_duty;
end

function y = sin_lookup(tbl, idx)
    idx = mod(idx, 360);
    y = tbl(idx + 1);
end

function y = pg7_map_signed(x, xmin, xmax)
    x = min(max(x, xmin), xmax);
    y = 100 * double(x - xmin) / double(xmax - xmin);
end

function y = shift_c(x, q)
    if x >= 0
        y = floor(double(x) / 2^q);
    else
        y = -floor((double(-x) + (2^q - 1)) / 2^q);
    end
end

function y = q30_round(x)
    if x >= 0
        y = floor((double(x) + 2^29) / 2^30);
    else
        y = -floor((double(-x) + 2^29) / 2^30);
    end
end

function y = q15_round(x)
    if x >= 0
        y = floor((double(x) + 2^14) / 2^15);
    else
        y = -floor((double(-x) + 2^14) / 2^15);
    end
end
