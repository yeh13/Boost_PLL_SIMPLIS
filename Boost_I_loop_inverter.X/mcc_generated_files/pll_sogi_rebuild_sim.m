clear; clc; close all;

FS_HZ = 5000;
FIN_LIST_HZ = [55 60 65];
SIM_CYCLES = 80;
ADC_CENTER = 2048;
AMP_COUNTS = 1860;
PHASE_DEG = 0;

SOGI_TUNE_HZ = 60;
SOGI_K = sqrt(2);
OFFSET_SHIFT = 12;

F_NOM_HZ = 60;
F_MIN_HZ = 50;
F_MAX_HZ = 75;
TRIM_LIMIT_HZ = 2.0;

% Float-domain PLL gains for architecture selection.
% After the detector is chosen, these can be translated back to shifts.
KP = 18.0;
KI = 180.0;
ERR_LPF_ALPHA = 1 / 128;
STEP_SLEW_HZ_PER_SAMPLE = 3 / FS_HZ;

DETECTOR_NAMES = {
    'alpha*cos + beta*sin'
    'alpha*cos - beta*sin'
    '-alpha*sin + beta*cos'
    'alpha*sin - beta*cos'
};

fprintf('Clean SOGI-PLL architecture sweep, fs=%d Hz\n', FS_HZ);
fprintf('SOGI fixed tune=%g Hz, offset IIR shift=%d\n\n', SOGI_TUNE_HZ, OFFSET_SHIFT);
fprintf('det sign fin  freqAvg freqMin freqMax errAvg errPkPk phaseErrDeg lockScore\n');
fprintf('--- ---- ---  ------- ------- ------- ------ ------- ----------- ---------\n');

best_score = inf;
best_result = [];
best_cfg = [];

for det = 1:4
    for err_sign = [-1 1]
        for fin = FIN_LIST_HZ
            result = run_case(fin, FS_HZ, SIM_CYCLES, ADC_CENTER, AMP_COUNTS, PHASE_DEG, ...
                SOGI_TUNE_HZ, SOGI_K, OFFSET_SHIFT, F_NOM_HZ, F_MIN_HZ, F_MAX_HZ, ...
                TRIM_LIMIT_HZ, KP, KI, ERR_LPF_ALPHA, STEP_SLEW_HZ_PER_SAMPLE, det, err_sign);

            last_n = round(FS_HZ / fin);
            idx = (numel(result.t)-last_n+1):numel(result.t);
            freq_avg = mean(result.freq_hz(idx));
            freq_min = min(result.freq_hz(idx));
            freq_max = max(result.freq_hz(idx));
            err_avg = mean(result.err_lpf(idx));
            err_pkpk = max(result.err_lpf(idx)) - min(result.err_lpf(idx));
            phase_err = circular_mean_deg(result.theta_grid(idx) - result.theta_pll(idx));

            score = abs(freq_avg - fin) * 8 + (freq_max - freq_min) * 4 + ...
                    abs(err_avg) * 0.6 + abs(phase_err) * 0.2 + err_pkpk * 0.05;

            fprintf('%3d %4d %3.0f  %7.3f %7.3f %7.3f %6.2f %7.2f %11.2f %9.3f\n', ...
                det, err_sign, fin, freq_avg, freq_min, freq_max, err_avg, err_pkpk, phase_err, score);

            if fin == 60 && score < best_score
                best_score = score;
                best_result = result;
                best_cfg = [det err_sign freq_avg freq_min freq_max err_avg err_pkpk phase_err score];
            end
        end
    end
end

fprintf('\nBest 60Hz architecture candidate:\n');
fprintf('detector %d: %s\n', best_cfg(1), DETECTOR_NAMES{best_cfg(1)});
fprintf('ERR_SIGN=%d\n', best_cfg(2));
fprintf('freq avg/min/max = %.4f / %.4f / %.4f Hz\n', best_cfg(3), best_cfg(4), best_cfg(5));
fprintf('err avg/pkpk = %.4f / %.4f\n', best_cfg(6), best_cfg(7));
fprintf('phase error avg = %.3f deg\n', best_cfg(8));

if ~isempty(best_result)
    figure;
    subplot(5,1,1);
    plot(best_result.t, best_result.raw_adc, best_result.t, best_result.offset);
    grid on; legend('raw ADC','offset');

    subplot(5,1,2);
    plot(best_result.t, best_result.alpha, best_result.t, best_result.beta);
    grid on; legend('alpha','beta');

    subplot(5,1,3);
    plot(best_result.t, best_result.err_raw, best_result.t, best_result.err_lpf);
    grid on; legend('err raw','err lpf');

    subplot(5,1,4);
    plot(best_result.t, best_result.freq_hz);
    grid on; ylabel('Hz');

    subplot(5,1,5);
    plot(best_result.t, wrap_to_180(best_result.theta_grid - best_result.theta_pll));
    grid on; ylabel('phase err deg'); xlabel('s');
end

function result = run_case(fin, fs, cycles, adc_center, amp, phase_deg, ...
    sogi_tune_hz, sogi_k, offset_shift, f_nom, f_min, f_max, trim_limit, ...
    kp, ki, err_lpf_alpha, step_slew, detector_id, err_sign)

    n_total = round(fs / fin * cycles);
    t = (0:n_total-1) / fs;
    theta_grid_rad = 2*pi*fin*t + deg2rad(phase_deg);
    raw_adc = adc_center + amp * sin(theta_grid_rad);

    [a1, a2, b0, qb0, qb1, qb2] = sogi_coeff_float(fs, sogi_tune_hz, sogi_k);

    offset = adc_center;
    x1 = 0; x2 = 0;
    alpha1 = 0; alpha2 = 0;
    beta1 = 0; beta2 = 0;

    theta = 0;
    freq = f_nom;
    integ = 0;
    err_lpf = 0;

    alpha_log = zeros(1, n_total);
    beta_log = zeros(1, n_total);
    offset_log = zeros(1, n_total);
    err_raw_log = zeros(1, n_total);
    err_lpf_log = zeros(1, n_total);
    freq_log = zeros(1, n_total);
    theta_log = zeros(1, n_total);

    for n = 1:n_total
        offset = offset + (raw_adc(n) - offset) / (2^offset_shift);
        x0 = raw_adc(n) - offset;

        alpha = b0*x0 - b0*x2 - a1*alpha1 - a2*alpha2;
        beta = qb0*x0 + qb1*x1 + qb2*x2 - a1*beta1 - a2*beta2;

        x2 = x1; x1 = x0;
        alpha2 = alpha1; alpha1 = alpha;
        beta2 = beta1; beta1 = beta;

        sin_t = sin(theta);
        cos_t = cos(theta);

        switch detector_id
            case 1
                vq = alpha*cos_t + beta*sin_t;
            case 2
                vq = alpha*cos_t - beta*sin_t;
            case 3
                vq = -alpha*sin_t + beta*cos_t;
            otherwise
                vq = alpha*sin_t - beta*cos_t;
        end

        mag = max([abs(alpha), abs(beta), 100]);
        err = err_sign * 1000 * vq / mag;
        err = min(max(err, -1000), 1000);
        err_lpf = err_lpf + err_lpf_alpha * (err - err_lpf);

        integ = integ + err_lpf / fs;
        integ = min(max(integ, -trim_limit / max(ki, 1e-9)), trim_limit / max(ki, 1e-9));

        trim = kp * err_lpf / 1000 + ki * integ;
        trim = min(max(trim, -trim_limit), trim_limit);

        target_freq = min(max(f_nom + trim, f_min), f_max);
        df = target_freq - freq;
        df = min(max(df, -step_slew), step_slew);
        freq = freq + df;

        theta = theta + 2*pi*freq/fs;
        theta = mod(theta, 2*pi);

        alpha_log(n) = alpha;
        beta_log(n) = beta;
        offset_log(n) = offset;
        err_raw_log(n) = err;
        err_lpf_log(n) = err_lpf;
        freq_log(n) = freq;
        theta_log(n) = rad2deg(theta);
    end

    result.t = t;
    result.raw_adc = raw_adc;
    result.offset = offset_log;
    result.alpha = alpha_log;
    result.beta = beta_log;
    result.err_raw = err_raw_log;
    result.err_lpf = err_lpf_log;
    result.freq_hz = freq_log;
    result.theta_pll = theta_log;
    result.theta_grid = wrap_to_180(rad2deg(theta_grid_rad));
end

function [a1, a2, b0, qb0, qb1, qb2] = sogi_coeff_float(fs, fg, k)
    wt = 2*pi*fg/fs;
    x = 2*k*wt;
    y = wt*wt;
    den = x + y + 4;
    a1 = -2*(4 - y)/den;
    a2 = -(x - y - 4)/den;
    b0 = x/den;
    qb0 = k*y/den;
    qb1 = 2*qb0;
    qb2 = qb0;
end

function y = wrap_to_180(x)
    y = mod(x + 180, 360) - 180;
end

function y = circular_mean_deg(x)
    r = deg2rad(x);
    y = rad2deg(atan2(mean(sin(r)), mean(cos(r))));
end
