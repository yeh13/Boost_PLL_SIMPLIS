clear; clc; close all;

%% MCU PLL lock diagnostic
% This script mirrors the active adc1.c path:
%   AN1 ADC callback = 40 kHz
%   SOGI / PLL / ZC estimator = 5 kHz, after PLL_DECIM_N = 8
%   fixed-point SOGI Q30, state Q12, sine Q15
%
% It runs three modes:
%   current_mcu : exactly the current lock bug, an1_60hz_lock_count never increments
%   count_fix   : same code, but increments an1_60hz_lock_count from ZC frequency
%   proposed_v2 : matches the pasted proposal: ZC feed-forward enabled and
%                 lock = input_valid && zc_freq_valid && abs_err <= lock threshold

ADC_ISR_HZ = 40000;
PLL_DECIM_N = 8;
FS_HZ = ADC_ISR_HZ / PLL_DECIM_N;

FIN_LIST_HZ = [55 60 65];
SIM_CYCLES = 100;
ADC_CENTER = 2048;
AMP_COUNTS = 1860;
PHASE_DEG = 0;

cfg.table_size = 360;
cfg.theta_subdiv = 512;
cfg.step_q = 12;
cfg.step_scale = bitshift(1, cfg.step_q);
cfg.theta_den_shift = 21;

cfg.wnom_q = floor(60 * cfg.table_size * cfg.theta_subdiv * cfg.step_scale / FS_HZ);
cfg.wmin_q = floor(45 * cfg.table_size * cfg.theta_subdiv * cfg.step_scale / FS_HZ);
cfg.wmax_q = floor(75 * cfg.table_size * cfg.theta_subdiv * cfg.step_scale / FS_HZ);
cfg.w57p5_q = floor(57.5 * cfg.table_size * cfg.theta_subdiv * cfg.step_scale / FS_HZ);
cfg.w62p5_q = floor(62.5 * cfg.table_size * cfg.theta_subdiv * cfg.step_scale / FS_HZ);

cfg.lock_err_th = 80;
cfg.pi_deadband_lock = 0;
cfg.lock_on_err_th = 170;
cfg.lock_off_err_th = 300;
cfg.lock_count_th = 20;
cfg.kp_shift_fast = 2;
cfg.ki_shift_fast = 13;
cfg.kp_shift_lock = 2;
cfg.ki_shift_lock = 13;
cfg.int_limit = 120000;
cfg.int_leak_shift = 7;
cfg.grid_offset_shift = 12;
cfg.pll_lpf_shift = 6;
cfg.ff_step_lpf_shift = 4;
cfg.step_slew_q = floor(100 * cfg.table_size * cfg.theta_subdiv * cfg.step_scale / (FS_HZ * FS_HZ));
cfg.trim_limit_q = floor(15 * cfg.table_size * cfg.theta_subdiv * cfg.step_scale / FS_HZ);
cfg.use_zc_ff = 0;

cfg.vmag_min = 100;
cfg.err_scale = 1000;
cfg.err_limit = 1000;
cfg.err_sign = 1;
cfg.err_bias = 0;
cfg.input_valid_amp_counts = 300;
cfg.input_amp_window_samples = 100;

cfg.zc_hys_counts = 120;
cfg.zc_filter_shift = 2;
cfg.zc_hys_max_counts = 300;
cfg.zc_period_min_5k = 60;
cfg.zc_period_max_5k = 125;
cfg.zc_counter_max_5k = 200;
cfg.period_avg_cycles = 4;
cfg.lock_min_x10 = 590;
cfg.lock_max_x10 = 610;
cfg.zc_lock_count = 3;

cfg.sogi_state_q = 12;
cfg.iref_peak_counts = 458;
cfg.vref_peak = 3787;

cfg.sogi55 = [-2042651804, 973794573, 49973625, 1726965, 3453929, 1726965];
cfg.sogi60 = [-2033145701, 965191209, 54275307, 2046131, 4092262, 2046131];
cfg.sogi65 = [-2023645686, 956665874, 58537975, 2390732, 4781464, 2390732];

sin_table = round(sin((0:359) * pi / 180) * 32767);
sin_table(271) = -32767;

fprintf('MCU lock diagnostic, ADC=%d Hz, decim=%d, SOGI/PLL/ZC=%d Hz\n', ...
    ADC_ISR_HZ, PLL_DECIM_N, FS_HZ);
fprintf('Important: active adc1.c lock uses an1_60hz_lock_count, but current code never increments it.\n\n');

fprintf('Mode         fin  valid%% rawZC valZC updZC zcFreqAvg zcFreqMin zcFreqMax countEnd errAvg errPkPk pllFreq lockEnd firstLock\n');
fprintf('-----------  ---  ------ ----- ----- ----- --------- --------- --------- -------- ------ ------- ------- ------- ---------\n');

all_current = cell(size(FIN_LIST_HZ));
all_fixed = cell(size(FIN_LIST_HZ));
all_proposed = cell(size(FIN_LIST_HZ));

for mode_i = 1:3
    count_fix = (mode_i == 2);
    proposed_v2 = (mode_i == 3);
    if proposed_v2
        mode_name = 'proposed_v2';
    elseif count_fix
        mode_name = 'count_fix';
    else
        mode_name = 'current_mcu';
    end

    for k = 1:numel(FIN_LIST_HZ)
        fin = FIN_LIST_HZ(k);
        r = run_case(fin, count_fix, proposed_v2, cfg, sin_table, ADC_ISR_HZ, PLL_DECIM_N, ...
            SIM_CYCLES, ADC_CENTER, AMP_COUNTS, PHASE_DEG);

        if proposed_v2
            all_proposed{k} = r;
        elseif count_fix
            all_fixed{k} = r;
        else
            all_current{k} = r;
        end

        valid_pct = 100 * mean(r.input_valid);
        zc_idx = find(r.zc_freq_update ~= 0);
        if isempty(zc_idx)
            zc_avg = 0; zc_min = 0; zc_max = 0;
        else
            zc_vals = r.zc_freq_x10(zc_idx) / 10;
            zc_avg = mean(zc_vals);
            zc_min = min(zc_vals);
            zc_max = max(zc_vals);
        end

        last_n = round(3 * r.fs / fin);
        idx = (numel(r.t)-last_n+1):numel(r.t);
        first_lock_idx = find(r.lock ~= 0, 1, 'first');
        if isempty(first_lock_idx)
            first_lock = -1;
        else
            first_lock = r.t(first_lock_idx);
        end

        fprintf('%-11s  %3.0f  %6.1f %5d %5d %5d %9.2f %9.2f %9.2f %8d %+6.1f %7.1f %7.3f %7d %9.4f\n', ...
            mode_name, fin, valid_pct, nnz(r.zc_event_raw), nnz(r.zc_event_valid), nnz(r.zc_freq_update), ...
            zc_avg, zc_min, zc_max, ...
            r.zc_lock_count(end), mean(r.err_lpf(idx)), max(r.err_lpf(idx))-min(r.err_lpf(idx)), ...
            mean(r.freq_hz(idx)), r.lock(end), first_lock);
    end
end

fprintf('\nDiagnosis:\n');
fprintf('1) If current_mcu lockEnd is 0 but count_fix locks at 60 Hz, the lock blocker is missing an1_60hz_lock_count increment.\n');
fprintf('2) If zcFreqAvg is near the input frequency, ZC period math is OK at this simulated sample rate.\n');
fprintf('3) proposed_v2 is NOT a 60Hz-only lock. It can lock at 55/65 Hz if phase error is small enough.\n\n');

r0 = all_current{2};
r1 = all_fixed{2};
r2 = all_proposed{2};

figure('Name', '60Hz MCU lock diagnostic');
subplot(5,1,1);
plot(r0.t, r0.v_in_count);
grid on; ylabel('AN1 cnt'); title('60Hz input, MCU-exact diagnostic');

subplot(5,1,2);
plot(r0.t, r0.input_valid, r0.t, r0.zc_event_raw);
grid on; ylabel('flags'); legend('input valid','raw ZC event');

subplot(5,1,3);
plot(r0.t, r0.zc_freq_x10 / 10, r1.t, r1.zc_lock_count * 10, ...
     r0.t, r0.zc_freq_update * 70);
grid on; ylabel('Hz / count'); legend('ZC freq','fixed count x10','freq update');

subplot(5,1,4);
plot(r0.t, r0.err_lpf, r0.t, r0.freq_hz);
grid on; ylabel('err / Hz'); legend('err lpf','PLL freq');

subplot(5,1,5);
plot(r0.t, r0.lock * 0.6, r1.t, r1.lock * 0.8, r2.t, r2.lock);
grid on; ylabel('lock'); xlabel('s'); legend('current MCU','with count fix','proposed v2');

function r = run_case(fin, count_fix, proposed_v2, cfg, sin_table, adc_hz, decim_n, sim_cycles, adc_center, amp_counts, phase_deg)
    fs = adc_hz / decim_n;
    n_adc = round(adc_hz / fin * sim_cycles);
    raw_adc_all = round(adc_center + amp_counts * sin(2*pi*fin*(0:n_adc-1)/adc_hz + phase_deg*pi/180));
    n_pll = floor(n_adc / decim_n);
    t = (0:n_pll-1) / fs;

    grid_offset = adc_center;
    an1_amp_min = 0; an1_amp_max = 0; an1_amp_counts = 0; an1_amp_cnt = 0;
    input_valid = 0;

    zc_v_filt_5k = 0; zc_sample_count_5k = 0; zc_armed_5k = 0;
    zc_period_sum_5k = 0; zc_period_count_5k = 0; zc_freq_x10 = 0;
    an1_zc_freq_valid = 0; an1_60hz_lock_count = 0;

    sogi_vin_n1 = 0; sogi_vin_n2 = 0;
    sogi_va_n1 = 0; sogi_va_n2 = 0;
    sogi_vb_n1 = 0; sogi_vb_n2 = 0;
    sogi_va = 0; sogi_vb = 0;

    pll_ff_step_target_q = cfg.wnom_q;
    pll_ff_step_q = cfg.wnom_q;
    pll_err_lpf = 0;
    pll_int_acc = 0;
    pll_step_q = cfg.wnom_q;
    theta_acc_q = 0;
    pll_idx = 0;
    pll_locked = 0;
    pll_lock_count = 0;
    pll_vmag_recip_q28 = 0;
    pll_recip_slow_cnt = 0;

    v_in_log = zeros(1, n_pll);
    input_valid_log = zeros(1, n_pll);
    zc_event_raw_log = zeros(1, n_pll);
    zc_event_valid_log = zeros(1, n_pll);
    zc_freq_update_log = zeros(1, n_pll);
    zc_period_log = zeros(1, n_pll);
    zc_freq_log = zeros(1, n_pll);
    zc_count_log = zeros(1, n_pll);
    err_lpf_log = zeros(1, n_pll);
    raw_err_log = zeros(1, n_pll);
    freq_log = zeros(1, n_pll);
    lock_log = zeros(1, n_pll);
    va_log = zeros(1, n_pll);
    vb_log = zeros(1, n_pll);

    m = 0;
    for n = 1:n_adc
        if mod(n-1, decim_n) ~= decim_n-1
            continue;
        end

        m = m + 1;
        adc_val = raw_adc_all(n);
        raw_err = 0;
        raw_zc_event = 0;
        valid_zc_event = 0;
        zc_freq_update = 0;
        zc_period_samples = 0;

        grid_offset = grid_offset + shift_c(adc_val - grid_offset, cfg.grid_offset_shift);
        v_in_count = adc_val - grid_offset;

        zc_v_filt_5k = zc_v_filt_5k + shift_c(v_in_count - zc_v_filt_5k, cfg.zc_filter_shift);
        zc_v = zc_v_filt_5k;

        zc_hys = floor(an1_amp_counts / 8);
        if zc_hys < cfg.zc_hys_counts
            zc_hys = cfg.zc_hys_counts;
        elseif zc_hys > cfg.zc_hys_max_counts
            zc_hys = cfg.zc_hys_max_counts;
        end

        if input_valid == 0
            zc_sample_count_5k = 0; zc_armed_5k = 0;
            zc_period_sum_5k = 0; zc_period_count_5k = 0;
            an1_zc_freq_valid = 0; zc_freq_x10 = 0;
        else
            if zc_sample_count_5k < cfg.zc_counter_max_5k
                zc_sample_count_5k = zc_sample_count_5k + 1;
            end

            if zc_v <= -zc_hys
                zc_armed_5k = 1;
            end

            if zc_armed_5k ~= 0 && zc_v >= zc_hys
                raw_zc_event = 1;
                zc_period_samples = zc_sample_count_5k;
                zc_armed_5k = 0;
                zc_sample_count_5k = 0;

                if zc_period_samples >= cfg.zc_period_min_5k && zc_period_samples <= cfg.zc_period_max_5k
                    valid_zc_event = 1;
                    zc_period_sum_5k = zc_period_sum_5k + zc_period_samples;
                    zc_period_count_5k = zc_period_count_5k + 1;

                    if zc_period_count_5k >= cfg.period_avg_cycles
                        zc_freq_x10 = floor((fs * 10 * zc_period_count_5k + floor(zc_period_sum_5k / 2)) / zc_period_sum_5k);
                        zc_freq_update = 1;

                        if zc_freq_x10 >= 450 && zc_freq_x10 <= 750
                            pll_ff_step_target_q = freq_x10_to_step_q(zc_freq_x10, cfg, fs);

                            if an1_zc_freq_valid == 0
                                an1_zc_freq_valid = 1;
                                pll_ff_step_q = pll_ff_step_target_q;
                                pll_step_q = pll_ff_step_target_q;
                            end
                        end

                        if count_fix
                            if zc_freq_x10 >= cfg.lock_min_x10 && zc_freq_x10 <= cfg.lock_max_x10
                                an1_60hz_lock_count = min(an1_60hz_lock_count + 1, cfg.zc_lock_count);
                            else
                                an1_60hz_lock_count = 0;
                            end
                        end

                        zc_period_sum_5k = 0;
                        zc_period_count_5k = 0;
                    end
                else
                    zc_period_sum_5k = 0;
                    zc_period_count_5k = 0;
                end
            end
        end

        if an1_amp_cnt == 0
            an1_amp_min = v_in_count;
            an1_amp_max = v_in_count;
        else
            if v_in_count < an1_amp_min, an1_amp_min = v_in_count; end
            if v_in_count > an1_amp_max, an1_amp_max = v_in_count; end
        end

        an1_amp_cnt = an1_amp_cnt + 1;
        if an1_amp_cnt >= cfg.input_amp_window_samples
            an1_amp_counts = floor((an1_amp_max - an1_amp_min) / 2);
            input_valid = an1_amp_counts >= cfg.input_valid_amp_counts;
            an1_amp_cnt = 0;
        end

        if input_valid == 0
            pll_locked = 0;
            pll_err_lpf = 0;
            pll_int_acc = 0;
            pll_lock_count = 0;
            pll_ff_step_target_q = cfg.wnom_q;
            pll_ff_step_q = cfg.wnom_q;
            pll_step_q = cfg.wnom_q;
            theta_acc_q = 0;
            pll_idx = 0;
            sogi_va = 0; sogi_vb = 0;
            sogi_va_n1 = 0; sogi_va_n2 = 0;
            sogi_vb_n1 = 0; sogi_vb_n2 = 0;
            sogi_vin_n1 = 0; sogi_vin_n2 = 0;
            an1_60hz_lock_count = 0;
        else
            pll_ff_step_q = pll_ff_step_q + shift_c(pll_ff_step_target_q - pll_ff_step_q, cfg.ff_step_lpf_shift);
            c = sogi_coeff_from_step(pll_ff_step_q, cfg);

            v_in_q = v_in_count * 2^cfg.sogi_state_q;
            acc_va = int64(c(3)) * int64(v_in_q) ...
                   - int64(c(3)) * int64(sogi_vin_n2) ...
                   - int64(c(1)) * int64(sogi_va_n1) ...
                   - int64(c(2)) * int64(sogi_va_n2);
            acc_vb = int64(c(4)) * int64(v_in_q) ...
                   + int64(c(5)) * int64(sogi_vin_n1) ...
                   + int64(c(6)) * int64(sogi_vin_n2) ...
                   - int64(c(1)) * int64(sogi_vb_n1) ...
                   - int64(c(2)) * int64(sogi_vb_n2);
            va_new = q30_round(acc_va);
            vb_new = q30_round(acc_vb);

            sogi_va = va_new; sogi_vb = vb_new;
            sogi_vin_n2 = sogi_vin_n1; sogi_vin_n1 = v_in_q;
            sogi_va_n2 = sogi_va_n1; sogi_va_n1 = va_new;
            sogi_vb_n2 = sogi_vb_n1; sogi_vb_n1 = vb_new;

            sin_theta = sin_interp(sin_table, pll_idx, theta_acc_q, cfg);
            cos_theta = sin_interp(sin_table, mod(pll_idx + 90, cfg.table_size), theta_acc_q, cfg);
            vq_q = q15_round(int64(sogi_va) * int64(cos_theta) + int64(sogi_vb) * int64(sin_theta));

            vmag_q = max(abs(sogi_va), abs(sogi_vb));
            vmag_min_q = cfg.vmag_min * 2^cfg.sogi_state_q;
            if vmag_q < vmag_min_q, vmag_q = vmag_min_q; end

            pll_recip_slow_cnt = pll_recip_slow_cnt + 1;
            if pll_recip_slow_cnt >= 8 || pll_vmag_recip_q28 == 0
                pll_recip_slow_cnt = 0;
                pll_vmag_recip_q28 = floor(2^28 / double(vmag_q));
                if pll_vmag_recip_q28 < 1, pll_vmag_recip_q28 = 1; end
            end

            raw_err = floor(double(cfg.err_sign * int64(vq_q) * int64(cfg.err_scale) * int64(pll_vmag_recip_q28)) / 2^28);
            raw_err = raw_err - cfg.err_bias;
            raw_err = min(max(raw_err, -cfg.err_limit), cfg.err_limit);
            pll_err_lpf = pll_err_lpf + shift_c(raw_err - pll_err_lpf, cfg.pll_lpf_shift);

            abs_err_lpf = abs(pll_err_lpf);
            if abs_err_lpf > cfg.lock_err_th
                kp_shift = cfg.kp_shift_fast;
                ki_shift = cfg.ki_shift_fast;
                deadband = 0;
                lock_mode = 0;
            else
                kp_shift = cfg.kp_shift_lock;
                ki_shift = cfg.ki_shift_lock;
                deadband = cfg.pi_deadband_lock;
                lock_mode = 1;
            end

            pll_err_pi = pll_err_lpf;
            if pll_err_pi <= deadband && pll_err_pi >= -deadband
                pll_err_pi = 0;
            end

            if pll_err_pi == 0 && lock_mode == 1
                if pll_int_acc > 0
                    leak = max(1, shift_c(pll_int_acc, cfg.int_leak_shift));
                    pll_int_acc = max(0, pll_int_acc - leak);
                elseif pll_int_acc < 0
                    leak = max(1, shift_c(-pll_int_acc, cfg.int_leak_shift));
                    pll_int_acc = min(0, pll_int_acc + leak);
                end
            else
                pll_int_acc = pll_int_acc + pll_err_pi;
            end

            pll_int_acc = min(max(pll_int_acc, -cfg.int_limit), cfg.int_limit);
            w_delta_q = shift_c(int64(pll_err_pi) * int64(cfg.step_scale), kp_shift) + ...
                        shift_c(int64(pll_int_acc) * int64(cfg.step_scale), ki_shift);
            w_delta_q = min(max(w_delta_q, -cfg.trim_limit_q), cfg.trim_limit_q);

            if proposed_v2 || cfg.use_zc_ff ~= 0
                pll_base_step_q = pll_ff_step_q;
            else
                pll_base_step_q = cfg.wnom_q;
            end

            pll_step_target_q = min(max(pll_base_step_q + w_delta_q, cfg.wmin_q), cfg.wmax_q);
            pll_step_diff_q = min(max(pll_step_target_q - pll_step_q, -cfg.step_slew_q), cfg.step_slew_q);
            pll_step_q = pll_step_q + pll_step_diff_q;

            theta_acc_q = theta_acc_q + pll_step_q;
            step_degrees = floor(double(theta_acc_q) / 2^cfg.theta_den_shift);
            theta_acc_q = theta_acc_q - int64(step_degrees) * int64(2^cfg.theta_den_shift);
            pll_idx = pll_idx + step_degrees;
            if pll_idx >= cfg.table_size
                pll_idx = pll_idx - cfg.table_size;
            end

            if proposed_v2
                if input_valid ~= 0 && an1_zc_freq_valid ~= 0 && abs_err_lpf <= cfg.lock_on_err_th
                    if pll_lock_count < cfg.lock_count_th
                        pll_lock_count = pll_lock_count + 1;
                    end
                    if pll_lock_count >= cfg.lock_count_th
                        pll_locked = 1;
                    end
                elseif an1_zc_freq_valid == 0 || abs_err_lpf > cfg.lock_off_err_th
                    pll_lock_count = 0;
                    pll_locked = 0;
                end
            else
                pll_locked = an1_60hz_lock_count >= cfg.zc_lock_count;
            end
        end

        v_in_log(m) = v_in_count;
        input_valid_log(m) = input_valid;
        zc_event_raw_log(m) = raw_zc_event;
        zc_event_valid_log(m) = valid_zc_event;
        zc_freq_update_log(m) = zc_freq_update;
        zc_period_log(m) = zc_period_samples;
        zc_freq_log(m) = zc_freq_x10;
        if proposed_v2
            zc_count_log(m) = pll_lock_count;
        else
            zc_count_log(m) = an1_60hz_lock_count;
        end
        raw_err_log(m) = raw_err;
        err_lpf_log(m) = pll_err_lpf;
        freq_log(m) = double(pll_step_q) * fs / (cfg.table_size * cfg.theta_subdiv * cfg.step_scale);
        lock_log(m) = pll_locked;
        va_log(m) = shift_c(sogi_va, cfg.sogi_state_q);
        vb_log(m) = shift_c(sogi_vb, cfg.sogi_state_q);
    end

    r.fs = fs;
    r.t = t;
    r.v_in_count = v_in_log;
    r.input_valid = input_valid_log;
    r.zc_event_raw = zc_event_raw_log;
    r.zc_event_valid = zc_event_valid_log;
    r.zc_freq_update = zc_freq_update_log;
    r.zc_period = zc_period_log;
    r.zc_freq_x10 = zc_freq_log;
    r.zc_lock_count = zc_count_log;
    r.raw_err = raw_err_log;
    r.err_lpf = err_lpf_log;
    r.freq_hz = freq_log;
    r.lock = lock_log;
    r.va = va_log;
    r.vb = vb_log;
end

function c = sogi_coeff_from_step(step_q, cfg)
    if step_q < cfg.w57p5_q
        c = cfg.sogi55;
    elseif step_q > cfg.w62p5_q
        c = cfg.sogi65;
    else
        c = cfg.sogi60;
    end
end

function step_q = freq_x10_to_step_q(freq_x10, cfg, fs)
    freq_x10 = min(max(freq_x10, 450), 750);
    step_q = floor(double(int64(freq_x10) * int64(cfg.table_size) * int64(cfg.theta_subdiv) * int64(cfg.step_scale)) / double(int64(fs) * 10));
    step_q = min(max(step_q, cfg.wmin_q), cfg.wmax_q);
end

function y = sin_lookup(tbl, idx)
    idx = mod(idx, 360);
    y = tbl(idx + 1);
end

function y = sin_interp(tbl, idx, theta_acc_q, cfg)
    s0 = sin_lookup(tbl, idx);
    s1 = sin_lookup(tbl, idx + 1);
    frac_q = floor(double(theta_acc_q) / 2^(cfg.theta_den_shift - 15));
    y = s0 + q15_round(int64(s1 - s0) * int64(frac_q));
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
