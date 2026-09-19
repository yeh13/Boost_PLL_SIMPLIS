clear; clc; close all;

FS_HZ = 5000;
FIN_LIST_HZ = [60];
SIM_CYCLES = 80;
AMP_COUNTS = 1860;
PHASE_DEG = 0;
PLOT_FIN_HZ = 60;

PLL_TABLE_SIZE = 360;
PLL_THETA_SUBDIV = 512;
PLL_STEP_Q = 12;
PLL_STEP_SCALE = 2^PLL_STEP_Q;

PLL_WNOM_STEP_Q = floor(60 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);
PLL_WMIN_STEP_Q = floor(50 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);
PLL_WMAX_STEP_Q = floor(75 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);
PLL_W57P5_STEP_Q = floor(57.5 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);
PLL_W62P5_STEP_Q = floor(62.5 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);

PLL_KP_SHIFT_FAST = 4;
PLL_KI_SHIFT_FAST = 10;
PLL_KP_SHIFT_LOCK = 6;
PLL_KI_SHIFT_LOCK = 12;

PLL_LOCK_ERR_TH = 80;
PLL_PI_DEADBAND_LOCK = 10;
PLL_LOCK_ON_ERR_TH = 80;
PLL_LOCK_OFF_ERR_TH = 180;
PLL_LOCK_COUNT_TH = 68;

PLL_INT_LIMIT = 120000;
PLL_INT_LEAK_SHIFT = 7;
PLL_LPF_SHIFT = 8;
PLL_STEP_SLEW_HZ = 3;

PLL_VMAG_MIN = 100;
PLL_ERR_SCALE = 1000;
PLL_ERR_LIMIT = 1000;

SOGI_STATE_Q = 12;
IREF_PEAK_COUNTS = 458;

USE_ZC_FEED_FORWARD = false;
TRIM_LIMIT_LIST_HZ = [0 1 3];
ERR_SIGN_LIST = [-1 1];
PHASE_COMP_LIST_DEG = -60:5:60;

sin_table = round(sin((0:359) * pi / 180) * 32767);
sin_table(271) = -32767;

step_slew_q = floor(PLL_STEP_SLEW_HZ * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);

fprintf('Sweep uses C-like PI at fs=%d Hz, use_zc_ff=%d\n', FS_HZ, USE_ZC_FEED_FORWARD);
fprintf('ERR duty: 50%% = zero error. STEP duty: 40%% = 60 Hz.\n\n');
fprintf('sign phase trim  ErrDutyAvg ErrDutyMin ErrDutyMax  StepAvg StepMin StepMax FinalHz\n');
fprintf('---- ----- ----  ---------- ---------- ----------  ------- ------- ------- -------\n');

best = [];
best_score = inf;
plot_result = [];

for trim_hz = TRIM_LIMIT_LIST_HZ
    trim_limit_q = floor(trim_hz * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);

    for err_sign = ERR_SIGN_LIST
        for phase_comp_deg = PHASE_COMP_LIST_DEG
            for fin = FIN_LIST_HZ
                result = run_pll_case(fin, FS_HZ, SIM_CYCLES, AMP_COUNTS, PHASE_DEG, ...
                    sin_table, PLL_TABLE_SIZE, PLL_THETA_SUBDIV, PLL_STEP_Q, PLL_STEP_SCALE, ...
                    PLL_WNOM_STEP_Q, PLL_WMIN_STEP_Q, PLL_WMAX_STEP_Q, PLL_W57P5_STEP_Q, PLL_W62P5_STEP_Q, ...
                    PLL_KP_SHIFT_FAST, PLL_KI_SHIFT_FAST, PLL_KP_SHIFT_LOCK, PLL_KI_SHIFT_LOCK, ...
                    PLL_LOCK_ERR_TH, PLL_PI_DEADBAND_LOCK, PLL_LOCK_ON_ERR_TH, PLL_LOCK_OFF_ERR_TH, PLL_LOCK_COUNT_TH, ...
                    PLL_INT_LIMIT, PLL_INT_LEAK_SHIFT, PLL_LPF_SHIFT, trim_limit_q, step_slew_q, ...
                    PLL_VMAG_MIN, PLL_ERR_SCALE, PLL_ERR_LIMIT, SOGI_STATE_Q, IREF_PEAK_COUNTS, ...
                    err_sign, phase_comp_deg, USE_ZC_FEED_FORWARD);

                last_cycle_n = round(FS_HZ / fin);
                last_idx = (length(result.t)-last_cycle_n+1):length(result.t);

                err_duty_avg = mean(result.err_duty_log(last_idx));
                err_duty_min = min(result.err_duty_log(last_idx));
                err_duty_max = max(result.err_duty_log(last_idx));
                step_duty_avg = mean(result.step_duty_log(last_idx));
                step_duty_min = min(result.step_duty_log(last_idx));
                step_duty_max = max(result.step_duty_log(last_idx));

                score = abs(err_duty_avg - 50) + 2 * abs(step_duty_avg - 40) + ...
                        0.2 * (step_duty_max - step_duty_min);

                if score < best_score
                    best_score = score;
                    best = [err_sign phase_comp_deg trim_hz err_duty_avg err_duty_min err_duty_max ...
                            step_duty_avg step_duty_min step_duty_max result.freq_log(end)];
                    plot_result = result;
                end

                if abs(phase_comp_deg) <= 20 && trim_hz <= 1
                    fprintf('%4d %5d %4.1f  %10.2f %10.2f %10.2f  %7.2f %7.2f %7.2f %7.3f\n', ...
                        err_sign, phase_comp_deg, trim_hz, err_duty_avg, err_duty_min, err_duty_max, ...
                        step_duty_avg, step_duty_min, step_duty_max, result.freq_log(end));
                end
            end
        end
    end
end

fprintf('\nBest candidate:\n');
fprintf('ERR_SIGN=%d, PLL_PHASE_COMP_DEG=%d, TRIM_LIMIT_HZ=%.1f\n', best(1), best(2), best(3));
fprintf('ERR duty avg/min/max = %.2f / %.2f / %.2f %%\n', best(4), best(5), best(6));
fprintf('STEP duty avg/min/max = %.2f / %.2f / %.2f %%\n', best(7), best(8), best(9));
fprintf('Final frequency = %.4f Hz\n', best(10));

if ~isempty(plot_result)
    figure;
    subplot(5,1,1); plot(plot_result.t, plot_result.v_in_count, plot_result.t, plot_result.va_log, plot_result.t, plot_result.vb_log); grid on; legend('vin','va','vb');
    subplot(5,1,2); plot(plot_result.t, plot_result.vq_log, plot_result.t, plot_result.raw_err_log, plot_result.t, plot_result.err_lpf_log); grid on; legend('vq','raw err','err lpf');
    subplot(5,1,3); plot(plot_result.t, plot_result.freq_log); grid on; ylabel('Hz');
    subplot(5,1,4); plot(plot_result.t, plot_result.err_duty_log, plot_result.t, 50 + 0*plot_result.t); grid on; ylabel('ERR duty %');
    subplot(5,1,5); plot(plot_result.t, plot_result.step_duty_log, plot_result.t, 40 + 0*plot_result.t); grid on; ylabel('STEP duty %'); xlabel('s');
end

function result = run_pll_case(FIN_HZ, FS_HZ, SIM_CYCLES, AMP_COUNTS, PHASE_DEG, ...
    sin_table, PLL_TABLE_SIZE, PLL_THETA_SUBDIV, PLL_STEP_Q, PLL_STEP_SCALE, ...
    PLL_WNOM_STEP_Q, PLL_WMIN_STEP_Q, PLL_WMAX_STEP_Q, PLL_W57P5_STEP_Q, PLL_W62P5_STEP_Q, ...
    PLL_KP_SHIFT_FAST, PLL_KI_SHIFT_FAST, PLL_KP_SHIFT_LOCK, PLL_KI_SHIFT_LOCK, ...
    PLL_LOCK_ERR_TH, PLL_PI_DEADBAND_LOCK, PLL_LOCK_ON_ERR_TH, PLL_LOCK_OFF_ERR_TH, PLL_LOCK_COUNT_TH, ...
    PLL_INT_LIMIT, PLL_INT_LEAK_SHIFT, PLL_LPF_SHIFT, trim_limit_q, step_slew_q, ...
    PLL_VMAG_MIN, PLL_ERR_SCALE, PLL_ERR_LIMIT, SOGI_STATE_Q, IREF_PEAK_COUNTS, ...
    ERR_SIGN, PHASE_COMP_DEG, USE_ZC_FEED_FORWARD)

    N = round(FS_HZ / FIN_HZ * SIM_CYCLES);
    t = (0:N-1) / FS_HZ;
    v_in_count = round(AMP_COUNTS * sin(2*pi*FIN_HZ*t + PHASE_DEG*pi/180));
    pll_ff_step_q = PLL_WNOM_STEP_Q;
    pll_ff_step_target_q = PLL_WNOM_STEP_Q;

    sogi_va_n1 = 0; sogi_va_n2 = 0;
    sogi_vb_n1 = 0; sogi_vb_n2 = 0;
    sogi_vin_n1 = 0; sogi_vin_n2 = 0;

    pll_err_lpf = 0;
    pll_int_acc = 0;
    pll_step_q = PLL_WNOM_STEP_Q;
    theta_acc_q = 0;
    pll_idx = 0;
    pll_lock_count = 0;
    pll_locked = 0;

    va_log = zeros(1,N);
    vb_log = zeros(1,N);
    vq_log = zeros(1,N);
    raw_err_log = zeros(1,N);
    err_lpf_log = zeros(1,N);
    freq_log = zeros(1,N);
    iref_log = zeros(1,N);
    lock_log = zeros(1,N);
    err_duty_log = zeros(1,N);
    step_duty_log = zeros(1,N);

    for n = 1:N
        if pll_step_q < PLL_W57P5_STEP_Q
            [SOGI_A1_Q30, SOGI_A2_Q30, SOGI_B0_Q30, SOGI_QB0_Q30, SOGI_QB1_Q30, SOGI_QB2_Q30] = sogi_coeff_fixed(55);
        elseif pll_step_q > PLL_W62P5_STEP_Q
            [SOGI_A1_Q30, SOGI_A2_Q30, SOGI_B0_Q30, SOGI_QB0_Q30, SOGI_QB1_Q30, SOGI_QB2_Q30] = sogi_coeff_fixed(65);
        else
            [SOGI_A1_Q30, SOGI_A2_Q30, SOGI_B0_Q30, SOGI_QB0_Q30, SOGI_QB1_Q30, SOGI_QB2_Q30] = sogi_coeff_fixed(60);
        end

        v_in_q = v_in_count(n) * 2^SOGI_STATE_Q;

        acc_va = int64(SOGI_B0_Q30) * int64(v_in_q) ...
               - int64(SOGI_B0_Q30) * int64(sogi_vin_n2) ...
               - int64(SOGI_A1_Q30) * int64(sogi_va_n1) ...
               - int64(SOGI_A2_Q30) * int64(sogi_va_n2);

        acc_vb = int64(SOGI_QB0_Q30) * int64(v_in_q) ...
               + int64(SOGI_QB1_Q30) * int64(sogi_vin_n1) ...
               + int64(SOGI_QB2_Q30) * int64(sogi_vin_n2) ...
               - int64(SOGI_A1_Q30) * int64(sogi_vb_n1) ...
               - int64(SOGI_A2_Q30) * int64(sogi_vb_n2);

        va_new = q30_round(acc_va);
        vb_new = q30_round(acc_vb);

        sogi_vin_n2 = sogi_vin_n1;
        sogi_vin_n1 = v_in_q;
        sogi_va_n2 = sogi_va_n1;
        sogi_va_n1 = va_new;
        sogi_vb_n2 = sogi_vb_n1;
        sogi_vb_n1 = vb_new;

        theta_comp_idx = mod(pll_idx + PHASE_COMP_DEG, PLL_TABLE_SIZE);
        sin_theta = sin_interp_c(sin_table, theta_comp_idx, theta_acc_q, PLL_THETA_SUBDIV, PLL_STEP_Q);
        cos_theta = sin_interp_c(sin_table, mod(theta_comp_idx + 90, PLL_TABLE_SIZE), theta_acc_q, PLL_THETA_SUBDIV, PLL_STEP_Q);

        vq_q = q15_round(int64(va_new) * int64(cos_theta) + int64(vb_new) * int64(sin_theta));

        vmag_q = max([abs(va_new), abs(vb_new), PLL_VMAG_MIN * 2^SOGI_STATE_Q]);
        raw_err = floor(double(ERR_SIGN * int64(vq_q) * int64(PLL_ERR_SCALE)) / double(vmag_q));
        raw_err = min(max(raw_err, -PLL_ERR_LIMIT), PLL_ERR_LIMIT);

        pll_err_lpf = pll_err_lpf + shift_c(raw_err - pll_err_lpf, PLL_LPF_SHIFT);

        abs_err_lpf = abs(pll_err_lpf);
        if abs_err_lpf > PLL_LOCK_ERR_TH
            kp_shift = PLL_KP_SHIFT_FAST;
            ki_shift = PLL_KI_SHIFT_FAST;
            deadband = 0;
            lock_mode = 0;
        else
            kp_shift = PLL_KP_SHIFT_LOCK;
            ki_shift = PLL_KI_SHIFT_LOCK;
            deadband = PLL_PI_DEADBAND_LOCK;
            lock_mode = 1;
        end

        pll_err_pi = pll_err_lpf;
        if pll_err_pi <= deadband && pll_err_pi >= -deadband
            pll_err_pi = 0;
        end

        if pll_err_pi == 0 && lock_mode == 1
            if pll_int_acc > 0
                leak = max(1, shift_c(pll_int_acc, PLL_INT_LEAK_SHIFT));
                pll_int_acc = max(0, pll_int_acc - leak);
            elseif pll_int_acc < 0
                leak = max(1, shift_c(-pll_int_acc, PLL_INT_LEAK_SHIFT));
                pll_int_acc = min(0, pll_int_acc + leak);
            end
        else
            pll_int_acc = pll_int_acc + pll_err_pi;
        end

        pll_int_acc = min(max(pll_int_acc, -PLL_INT_LIMIT), PLL_INT_LIMIT);

        w_delta_q = shift_c(int64(pll_err_pi) * int64(PLL_STEP_SCALE), kp_shift) ...
                  + shift_c(int64(pll_int_acc) * int64(PLL_STEP_SCALE), ki_shift);
        w_delta_q = min(max(w_delta_q, -trim_limit_q), trim_limit_q);

        if USE_ZC_FEED_FORWARD
            pll_base_step_q = pll_ff_step_q;
        else
            pll_base_step_q = PLL_WNOM_STEP_Q;
        end

        pll_step_target_q = pll_base_step_q + w_delta_q;
        pll_step_target_q = min(max(pll_step_target_q, PLL_WMIN_STEP_Q), PLL_WMAX_STEP_Q);

        pll_step_diff_q = pll_step_target_q - pll_step_q;
        pll_step_diff_q = min(max(pll_step_diff_q, -step_slew_q), step_slew_q);
        pll_step_q = pll_step_q + pll_step_diff_q;

        theta_acc_q = theta_acc_q + pll_step_q;
        while theta_acc_q >= PLL_THETA_SUBDIV * PLL_STEP_SCALE
            theta_acc_q = theta_acc_q - PLL_THETA_SUBDIV * PLL_STEP_SCALE;
            pll_idx = pll_idx + 1;
            if pll_idx >= PLL_TABLE_SIZE
                pll_idx = 0;
            end
        end

        sin_ref = sin_interp_c(sin_table, pll_idx, theta_acc_q, PLL_THETA_SUBDIV, PLL_STEP_Q);
        i_ref_count = floor(double(sin_ref * IREF_PEAK_COUNTS) / 32768);

        if abs(pll_err_lpf) <= PLL_LOCK_ON_ERR_TH
            pll_lock_count = min(pll_lock_count + 1, PLL_LOCK_COUNT_TH);
            if pll_lock_count >= PLL_LOCK_COUNT_TH
                pll_locked = 1;
            end
        elseif abs(pll_err_lpf) > PLL_LOCK_OFF_ERR_TH
            pll_lock_count = 0;
            pll_locked = 0;
        end

        va_log(n) = floor(double(va_new) / 2^SOGI_STATE_Q);
        vb_log(n) = floor(double(vb_new) / 2^SOGI_STATE_Q);
        vq_log(n) = floor(double(vq_q) / 2^SOGI_STATE_Q);
        raw_err_log(n) = raw_err;
        err_lpf_log(n) = pll_err_lpf;
        freq_log(n) = double(pll_step_q) * FS_HZ / (PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE);
        iref_log(n) = i_ref_count;
        lock_log(n) = pll_locked;
        err_duty_log(n) = pg7_map_signed(pll_err_lpf, -500, 500);
        step_duty_log(n) = 100 * double(pll_step_q - PLL_WMIN_STEP_Q) / double(PLL_WMAX_STEP_Q - PLL_WMIN_STEP_Q);
    end

    result.t = t;
    result.v_in_count = v_in_count;
    result.va_log = va_log;
    result.vb_log = vb_log;
    result.vq_log = vq_log;
    result.raw_err_log = raw_err_log;
    result.err_lpf_log = err_lpf_log;
    result.freq_log = freq_log;
    result.iref_log = iref_log;
    result.lock_log = lock_log;
    result.err_duty_log = err_duty_log;
    result.step_duty_log = step_duty_log;
end

function y = pg7_map_signed(x, xmin, xmax)
    x = min(max(x, xmin), xmax);
    y = 100 * double(x - xmin) / double(xmax - xmin);
end

function [a1_q30, a2_q30, b0_q30, qb0_q30, qb1_q30, qb2_q30] = sogi_coeff_fixed(fg)
    if fg == 55
        a1_q30 = -2042651804; a2_q30 = 973794573; b0_q30 = 49973625;
        qb0_q30 = 1726965; qb1_q30 = 3453929; qb2_q30 = 1726965;
    elseif fg == 65
        a1_q30 = -2023645686; a2_q30 = 956665874; b0_q30 = 58537975;
        qb0_q30 = 2390732; qb1_q30 = 4781464; qb2_q30 = 2390732;
    else
        a1_q30 = -2033145701; a2_q30 = 965191209; b0_q30 = 54275307;
        qb0_q30 = 2046131; qb1_q30 = 4092262; qb2_q30 = 2046131;
    end
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

function y = sin_interp_c(tbl, idx0, theta_frac_q, subdiv, step_q)
    idx0 = mod(idx0, 360);
    idx = idx0 + 1;
    idx1 = mod(idx0 + 1, 360) + 1;
    y0 = tbl(idx);
    y1 = tbl(idx1);
    dy = y1 - y0;
    den = subdiv * 2^step_q;

    if dy >= 0
        interp = floor((dy * theta_frac_q + den / 2) / den);
    else
        interp = -floor(((-dy) * theta_frac_q + den / 2) / den);
    end

    y = y0 + interp;
end
