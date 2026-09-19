% SOGI-PLL debug simulation for current adc1.c.
% Version: AFTER_SOFTPI - softer locked PI, clamped error, and filtered frequency command.
% Goal:
%   1. Check important debug values without powering the converter.
%   2. Verify that the SOGI-PLL can track a small AN1 frequency change.
%
% Notes:
%   - This models the active clean ADC1_channel_AN1_CallBack path.
%   - This version uses SOGI-PLL tracking, not zero-cross frequency lock.
%   - NORMALIZE_THETA_STEP_TO_MODEL_FS keeps the MATLAB model at physical
%     60Hz nominal while the firmware keeps its bench-calibrated theta gain.

clear; clc; close all;

%% User test knobs
sim_time_s = 3.00;
input_freq_1_hz = 60.0;
input_freq_2_hz = 60.4;
freq_step_time_s = 1.50;
an1_amp_counts = 900;
adc_offset_counts = 2048;
NORMALIZE_THETA_STEP_TO_MODEL_FS = true;
detector_phase_comp_deg = -1;
AUTO_SWEEP_DETECTOR_COMP = false;
detector_comp_sweep_deg = 0;
phase_ok_window_deg_est = asind(25 / 1000);

%% adc1.c constants
ADC_CENTER = 2048;
VREF_PEAK = 3787;
IREF_PEAK_COUNTS = 925;

ADC_ISR_HZ = 40000;
PLL_DECIM_N = 8;
FS_HZ = ADC_ISR_HZ / PLL_DECIM_N;
PLL_TABLE_SIZE = 360;
PLL_THETA_SUBDIV = 512;
PLL_VREF_TABLE_LEN = 334;
PLL_VREF_FULL_LEN = 2 * PLL_VREF_TABLE_LEN;

PLL_STEP_Q = 12;
PLL_STEP_SCALE = 2^PLL_STEP_Q;
PLL_THETA_STEP_GAIN_NUM = 8;
PLL_THETA_STEP_GAIN_DEN = 1;
PLL_FIXED_NCO_TEST = 0;
PLL_THETA_DEN_Q = PLL_THETA_SUBDIV * PLL_STEP_SCALE;
PLL_THETA_DEN_SHIFT = 21;

PLL_WNOM_STEP_Q = floor(60 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);
PLL_WMIN_STEP_Q = floor(45 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);
PLL_WMAX_STEP_Q = floor(75 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);

PLL_LOCK_ERR_TH = 80;
PLL_PI_DEADBAND_LOCK = 5;
PLL_LOCK_ON_ERR_TH = 170;
PLL_LOCK_OFF_ERR_TH = 300;
PLL_LOCK_COUNT_TH = 20;
PLL_UNLOCK_COUNT_TH = 100;
PLL_GRID_PHASE_ON_ERR_TH = 25;
PLL_GRID_PHASE_OFF_ERR_TH = 45;
PLL_GRID_PHASE_ON_COUNT_TH = 20;
PLL_GRID_PHASE_OFF_COUNT_TH = 50;

PLL_KP_SHIFT_FAST  = 2;
PLL_KI_SHIFT_FAST  = 13;

PLL_KP_SHIFT_TRACK = 6;   % softer proportional action after a phase disturbance
PLL_KI_SHIFT_TRACK = 15;

PLL_KP_SHIFT_LOCK  = 10;  % nearly remove P action in locked state
PLL_KI_SHIFT_LOCK  = 15;  % keep I action so frequency can settle

PLL_INT_LIMIT = floor(30 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);
PLL_INT_LEAK_SHIFT = 7;

GRID_OFFSET_Q = 12;
GRID_OFFSET_SHIFT = 12;
PLL_LPF_SHIFT = 6;
PLL_STEP_SLEW_Q = floor(40 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / (FS_HZ * FS_HZ));
PLL_TRIM_LIMIT_Q = floor(15 * PLL_TABLE_SIZE * PLL_THETA_SUBDIV * PLL_STEP_SCALE / FS_HZ);

PLL_VMAG_MIN = 100;
PLL_ERR_SCALE = 1000;
PLL_ERR_LIMIT = 1000;
PLL_ERR_SIGN = 1;
PLL_ERR_BIAS = 0;    % keep detector bias visible during debugging
PLL_ERR_PI_CLAMP_TRACK = 80;   % prevent transient phase error from kicking PLL frequency too hard
PLL_ERR_PI_CLAMP_LOCK  = 40;   % ignore large ripple in locked fine tracking
PLL_W_DELTA_LPF_SHIFT_TRACK = 3;   % 1/8 smoothing while recovering phase
PLL_W_DELTA_LPF_SHIFT_LOCK  = 5;   % 1/32 smoothing while locked
PLL_INPUT_VALID_AMP_COUNTS = 300;
PLL_INPUT_AMP_WINDOW_SAMPLES = 100;

SOGI_STATE_Q = 12;
SOGI55_A1_Q30 = -2042651804;
SOGI55_A2_Q30 =  973794573;
SOGI55_B0_Q30 =    49973625;
SOGI55_QB0_Q30 =    1726965;
SOGI55_QB1_Q30 =    3453929;
SOGI55_QB2_Q30 =    1726965;

SOGI60_A1_Q30 = -2033145701;
SOGI60_A2_Q30 =  965191209;
SOGI60_B0_Q30 =    54275307;
SOGI60_QB0_Q30 =    2046131;
SOGI60_QB1_Q30 =    4092262;
SOGI60_QB2_Q30 =    2046131;

SOGI65_A1_Q30 = -2023645686;
SOGI65_A2_Q30 =  956665874;
SOGI65_B0_Q30 =    58537975;
SOGI65_QB0_Q30 =    2390732;
SOGI65_QB1_Q30 =    4781464;
SOGI65_QB2_Q30 =    2390732;

SinTable360 = round(32767 * sin((0:359) * pi / 180));
VrefTable = [ ...
   0,36,71,107,142,178,214,249,285,320, ...
   356,391,427,462,497,533,568,603,638,673, ...
   708,743,778,813,848,882,917,951,986,1020, ...
   1055,1089,1123,1157,1191,1224,1258,1292,1325,1358, ...
   1391,1425,1457,1490,1523,1556,1588,1620,1652,1684, ...
   1716,1748,1779,1811,1842,1873,1904,1935,1965,1995, ...
   2026,2056,2085,2115,2145,2174,2203,2232,2260,2289, ...
   2317,2345,2373,2401,2428,2455,2482,2509,2536,2562, ...
   2588,2614,2640,2665,2690,2715,2740,2765,2789,2813, ...
   2836,2860,2883,2906,2929,2951,2974,2995,3017,3039, ...
   3060,3080,3101,3121,3141,3161,3181,3200,3219,3237, ...
   3256,3274,3292,3309,3326,3343,3360,3376,3392,3408, ...
   3423,3438,3453,3467,3482,3495,3509,3522,3535,3548, ...
   3560,3572,3584,3595,3606,3617,3627,3637,3647,3657, ...
   3666,3674,3683,3691,3699,3706,3713,3720,3727,3733, ...
   3739,3744,3749,3754,3759,3763,3767,3770,3774,3776, ...
   3779,3781,3783,3784,3786,3786,3787,3787,3787,3786, ...
   3786,3784,3783,3781,3779,3776,3774,3770,3767,3763, ...
   3759,3754,3749,3744,3739,3733,3727,3720,3713,3706, ...
   3699,3691,3683,3674,3666,3657,3647,3637,3627,3617, ...
   3606,3595,3584,3572,3560,3548,3535,3522,3509,3495, ...
   3482,3467,3453,3438,3423,3408,3392,3376,3360,3343, ...
   3326,3309,3292,3274,3256,3237,3219,3200,3181,3161, ...
   3141,3121,3101,3080,3060,3039,3017,2995,2974,2951, ...
   2929,2906,2883,2860,2836,2813,2789,2765,2740,2715, ...
   2690,2665,2640,2614,2588,2562,2536,2509,2482,2455, ...
   2428,2401,2373,2345,2317,2289,2260,2232,2203,2174, ...
   2145,2115,2085,2056,2026,1995,1965,1935,1904,1873, ...
   1842,1811,1779,1748,1716,1684,1652,1620,1588,1556, ...
   1523,1490,1457,1425,1391,1358,1325,1292,1258,1224, ...
   1191,1157,1123,1089,1055,1020,986,951,917,882, ...
   848,813,778,743,708,673,638,603,568,533, ...
   497,462,427,391,356,320,285,249,214,178, ...
   142,107,71,36];

if AUTO_SWEEP_DETECTOR_COMP
    sweep_err_mean = zeros(size(detector_comp_sweep_deg));
    sweep_err_pkpk = zeros(size(detector_comp_sweep_deg));
    sweep_phase_ok = zeros(size(detector_comp_sweep_deg));

    for sweep_k = 1:numel(detector_comp_sweep_deg)
        sweep_log = run_sogi_pll_sim(detector_comp_sweep_deg(sweep_k), false);
        sweep_pre = sweep_log.t > (freq_step_time_s - 0.40) & sweep_log.t < freq_step_time_s;
        sweep_err_mean(sweep_k) = mean(sweep_log.err(sweep_pre));
        sweep_err_pkpk(sweep_k) = max(sweep_log.err(sweep_pre)) - min(sweep_log.err(sweep_pre));
        sweep_phase_ok(sweep_k) = mean(sweep_log.phase_ok(sweep_pre));
    end

    [~, best_i] = min(abs(sweep_err_mean));
    detector_phase_comp_deg = detector_comp_sweep_deg(best_i);

    fprintf('Auto sweep selected detector_phase_comp_deg = %+d deg\n', detector_phase_comp_deg);
    fprintf('Sweep best: errMean=%.2f errPkPk=%d phaseOkRatio=%.3f\n', ...
        sweep_err_mean(best_i), sweep_err_pkpk(best_i), sweep_phase_ok(best_i));
end

sim = run_sogi_pll_sim(detector_phase_comp_deg, true);
t = sim.t;
log_freq_in = sim.freq_in;
log_an1 = sim.an1;
log_vin = sim.vin;
log_va = sim.va;
log_vb = sim.vb;
log_vq = sim.vq;
log_raw_err = sim.raw_err;
log_err = sim.err;
log_err_slow = sim.err_slow;
log_lock = sim.lock;
log_phase_ok = sim.phase_ok;
log_idx = sim.idx;
log_vref_idx = sim.vref_idx;
log_iref = sim.iref;
log_vref = sim.vref;
log_phase = sim.phase;
log_freq_x100 = sim.freq_x100;
log_input_valid = sim.input_valid;

%% Summary
pre = t > (freq_step_time_s - 0.40) & t < freq_step_time_s;
post = t > (sim_time_s - 0.50);
fprintf('SOGI-PLL tracking mode, theta normalized to model FS: %d\n', NORMALIZE_THETA_STEP_TO_MODEL_FS);
fprintf('Before step: lockRatio=%.3f phaseOkRatio=%.3f errMean=%.2f errPkPk=%d\n', ...
    mean(log_lock(pre)), mean(log_phase_ok(pre)), mean(log_err(pre)), max(log_err(pre))-min(log_err(pre)));
fprintf('After step : lockRatio=%.3f phaseOkRatio=%.3f errMean=%.2f errPkPk=%d\n', ...
    mean(log_lock(post)), mean(log_phase_ok(post)), mean(log_err(post)), max(log_err(post))-min(log_err(post)));
fprintf('Frequency : before=%.2f Hz after=%.2f Hz targetAfter=%.2f Hz\n', ...
    mean(log_freq_x100(pre)) / 100, mean(log_freq_x100(post)) / 100, input_freq_2_hz);
fprintf('Final lock=%d phaseOk=%d idx=%d vrefIdx=%d\n', ...
    log_lock(end), log_phase_ok(end), log_idx(end), log_vref_idx(end));

pre_err_mean = mean(log_err(pre));
phase_bias_deg = asind(max(min(pre_err_mean / 1000, 1), -1));
fprintf('Estimated remaining detector bias ~= %.2f deg from ERR mean.\n', phase_bias_deg);

freq_error_hz = abs(input_freq_2_hz - input_freq_1_hz);
if freq_error_hz > 0
    phase_drift_deg_per_s = 360 * freq_error_hz;
    phase_ok_time_est_s = phase_ok_window_deg_est / phase_drift_deg_per_s;
    fprintf('Input frequency step: %.3f Hz mismatch would drift %.1f deg/s without PLL tracking.\n', ...
        freq_error_hz, phase_drift_deg_per_s);
    fprintf('With PHASE_ON=%d, a fixed NCO phase-ok window would be only about %.3f s.\n', ...
        PLL_GRID_PHASE_ON_ERR_TH, phase_ok_time_est_s);
end

%% Plots
figure('Name','SOGI-PLL debug');
tiledlayout(5,1);

nexttile;
plot(t, log_freq_in, 'LineWidth', 1); hold on;
plot(t, log_freq_x100 / 100, 'LineWidth', 1); grid on;
ylabel('Hz'); title('Input frequency and PLL estimated frequency');
legend('AN1 input','PLL step');

nexttile;
plot(t, log_lock * 80, 'LineWidth', 1); hold on;
plot(t, log_phase_ok * 50, 'LineWidth', 1); grid on;
ylabel('state'); legend('LOCK*80','PHASE\_OK*50');

nexttile;
plot(t, log_vq, 'LineWidth', 1); hold on;
plot(t, log_err, 'LineWidth', 1);
plot(t, log_err_slow, 'LineWidth', 1); grid on;
ylabel('err'); legend('VQ','ERR','ERR\_SLOW');

nexttile;
plot(t, log_va, 'LineWidth', 1); hold on;
plot(t, log_vb, 'LineWidth', 1);
plot(t, log_vin, 'LineWidth', 1); grid on;
ylabel('counts'); legend('VA','VB','AN1-centered');

nexttile;
plot(t, log_idx, 'LineWidth', 1); hold on;
plot(t, log_vref_idx, 'LineWidth', 1);
plot(t, log_iref, 'LineWidth', 1); grid on;
ylabel('idx/iref'); xlabel('time (s)');
legend('theta idx','vref idx','iref');

figure('Name','Reference output');
plot(t, log_vref, 'LineWidth', 1); hold on;
plot(t, log_iref, 'LineWidth', 1); grid on;
legend('pll\_vref\_count','i\_ref\_count');
xlabel('time (s)');

if AUTO_SWEEP_DETECTOR_COMP
    figure('Name','Detector compensation sweep');
    yyaxis left;
    plot(detector_comp_sweep_deg, sweep_err_mean, '-o', 'LineWidth', 1); grid on;
    ylabel('ERR mean');
    yyaxis right;
    plot(detector_comp_sweep_deg, sweep_phase_ok, '-s', 'LineWidth', 1);
    ylabel('PHASE OK ratio');
    xlabel('detector phase comp deg');
end

%% Simulation function
function log = run_sogi_pll_sim(detector_phase_comp_deg_arg, keep_logs)
    if nargin < 2
        keep_logs = true;
    end

    names = { ...
        'sim_time_s', 'input_freq_1_hz', 'input_freq_2_hz', ...
        'freq_step_time_s', 'an1_amp_counts', 'adc_offset_counts', ...
        'NORMALIZE_THETA_STEP_TO_MODEL_FS', 'IREF_PEAK_COUNTS', ...
        'FS_HZ', 'PLL_TABLE_SIZE', 'PLL_THETA_SUBDIV', ...
        'PLL_VREF_TABLE_LEN', ...
        'PLL_VREF_FULL_LEN', 'PLL_STEP_SCALE', ...
        'PLL_THETA_STEP_GAIN_NUM', 'PLL_THETA_STEP_GAIN_DEN', ...
        'PLL_FIXED_NCO_TEST', 'PLL_THETA_DEN_Q', 'PLL_THETA_DEN_SHIFT', ...
        'PLL_WNOM_STEP_Q', 'PLL_WMIN_STEP_Q', 'PLL_WMAX_STEP_Q', ...
        'PLL_LOCK_ERR_TH', 'PLL_PI_DEADBAND_LOCK', ...
        'PLL_LOCK_ON_ERR_TH', 'PLL_LOCK_OFF_ERR_TH', ...
        'PLL_LOCK_COUNT_TH', 'PLL_UNLOCK_COUNT_TH', ...
        'PLL_GRID_PHASE_ON_ERR_TH', 'PLL_GRID_PHASE_OFF_ERR_TH', ...
        'PLL_GRID_PHASE_ON_COUNT_TH', 'PLL_GRID_PHASE_OFF_COUNT_TH', ...
        'PLL_KP_SHIFT_FAST', 'PLL_KI_SHIFT_FAST', ...
        'PLL_KP_SHIFT_TRACK', 'PLL_KI_SHIFT_TRACK', ...
        'PLL_KP_SHIFT_LOCK', 'PLL_KI_SHIFT_LOCK', ...
        'PLL_INT_LIMIT', 'PLL_INT_LEAK_SHIFT', ...
        'GRID_OFFSET_Q', 'GRID_OFFSET_SHIFT', 'PLL_LPF_SHIFT', ...
        'PLL_STEP_SLEW_Q', 'PLL_TRIM_LIMIT_Q', 'PLL_VMAG_MIN', ...
        'PLL_ERR_SCALE', 'PLL_ERR_LIMIT', 'PLL_ERR_SIGN', 'PLL_ERR_BIAS', ...
        'PLL_ERR_PI_CLAMP_TRACK', 'PLL_ERR_PI_CLAMP_LOCK', ...
        'PLL_W_DELTA_LPF_SHIFT_TRACK', 'PLL_W_DELTA_LPF_SHIFT_LOCK', ...
        'PLL_INPUT_VALID_AMP_COUNTS', 'PLL_INPUT_AMP_WINDOW_SAMPLES', ...
        'SOGI_STATE_Q', ...
        'SOGI55_A1_Q30', 'SOGI55_A2_Q30', 'SOGI55_B0_Q30', ...
        'SOGI55_QB0_Q30', 'SOGI55_QB1_Q30', 'SOGI55_QB2_Q30', ...
        'SOGI60_A1_Q30', 'SOGI60_A2_Q30', ...
        'SOGI60_B0_Q30', 'SOGI60_QB0_Q30', 'SOGI60_QB1_Q30', ...
        'SOGI60_QB2_Q30', ...
        'SOGI65_A1_Q30', 'SOGI65_A2_Q30', 'SOGI65_B0_Q30', ...
        'SOGI65_QB0_Q30', 'SOGI65_QB1_Q30', 'SOGI65_QB2_Q30', ...
        'SinTable360', 'VrefTable' ...
    };

    for k = 1:numel(names)
        eval([names{k} ' = evalin(''base'', ''' names{k} ''');']);
    end

    detector_phase_comp_deg = detector_phase_comp_deg_arg;

%% State variables
N = floor(sim_time_s * FS_HZ);
t = (0:N-1)' / FS_HZ;

grid_offset_q = adc_offset_counts * 2^GRID_OFFSET_Q;
grid_offset = adc_offset_counts;
sogi_va = 0; sogi_vb = 0;
sogi_va_n1 = 0; sogi_va_n2 = 0;
sogi_vb_n1 = 0; sogi_vb_n2 = 0;
sogi_vin_n1 = 0; sogi_vin_n2 = 0;

pll_err_lpf = 0;
pll_err_slow_lpf = 0;
pll_err_slow_lpf_q8 = 0;
pll_int_acc = 0;
pll_step_q = PLL_WNOM_STEP_Q;
pll_w_delta_filt_q = 0;
% SOGI coefficient frequency is filtered separately so coefficient changes
% do not follow every small PLL step ripple.
sogi_step_q = PLL_WNOM_STEP_Q;
theta_acc_q = 0;
pll_idx = 0;
pll_lock_count = 0;
pll_unlock_count = 0;
pll_phase_ok_count = 0;
pll_phase_bad_count = 0;
pll_locked = 0;
phase_ok = 0;

an1_amp_min = 0;
an1_amp_max = 0;
an1_amp_counts_meas = 0;
an1_amp_cnt = 0;
an1_signal_valid = 0;

theta_gain = PLL_THETA_STEP_GAIN_NUM / PLL_THETA_STEP_GAIN_DEN;
if NORMALIZE_THETA_STEP_TO_MODEL_FS
    theta_gain = 1;
end

%% Logs
log_freq_in = zeros(N,1);
log_an1 = zeros(N,1);
log_vin = zeros(N,1);
log_va = zeros(N,1);
log_vb = zeros(N,1);
log_vq = zeros(N,1);
log_raw_err = zeros(N,1);
log_err = zeros(N,1);
log_err_slow = zeros(N,1);
log_lock = zeros(N,1);
log_phase_ok = zeros(N,1);
log_idx = zeros(N,1);
log_vref_idx = zeros(N,1);
log_iref = zeros(N,1);
log_vref = zeros(N,1);
log_phase = zeros(N,1);
log_freq_x100 = zeros(N,1);
log_input_valid = zeros(N,1);

%% Input phase generator
input_phase = 0;

for n = 1:N
    if t(n) < freq_step_time_s
        fin = input_freq_1_hz;
    else
        fin = input_freq_2_hz;
    end
    log_freq_in(n) = fin;

    input_phase = input_phase + 2*pi*fin/FS_HZ;
    if input_phase >= 2*pi
        input_phase = input_phase - 2*pi;
    end

    adcVal = round(adc_offset_counts + an1_amp_counts * sin(input_phase));
    log_an1(n) = adcVal;

    grid_offset_err_q = adcVal * 2^GRID_OFFSET_Q - grid_offset_q;
    grid_offset_delta_q = c_rshift(grid_offset_err_q, GRID_OFFSET_SHIFT);
    grid_offset_q = grid_offset_q + grid_offset_delta_q;
    grid_offset = floor((grid_offset_q + 2^(GRID_OFFSET_Q - 1)) / 2^GRID_OFFSET_Q);

    v_in_count = adcVal - grid_offset;
    log_vin(n) = v_in_count;

    if an1_amp_cnt == 0
        an1_amp_min = v_in_count;
        an1_amp_max = v_in_count;
    else
        an1_amp_min = min(an1_amp_min, v_in_count);
        an1_amp_max = max(an1_amp_max, v_in_count);
    end

    an1_amp_cnt = an1_amp_cnt + 1;
    if an1_amp_cnt >= PLL_INPUT_AMP_WINDOW_SAMPLES
        an1_amp_counts_meas = floor((an1_amp_max - an1_amp_min) / 2);
        an1_signal_valid = an1_amp_counts_meas >= PLL_INPUT_VALID_AMP_COUNTS;
        an1_amp_cnt = 0;
    end
    log_input_valid(n) = an1_signal_valid;

    % Smooth SOGI tuning frequency.  PLL NCO still uses pll_step_q
    % directly, but SOGI coefficients use sogi_step_q to avoid hunting.
    % BALANCED version: SOGI coefficient step follows PLL step with /32 smoothing.
    % /64 was too slow and caused phase_ok bang-bang; /16 can be too reactive.
    sogi_step_diff_q = pll_step_q - sogi_step_q;
    if sogi_step_diff_q >= 0
        sogi_step_q = sogi_step_q + floor((sogi_step_diff_q + 16) / 32);
    else
        sogi_step_q = sogi_step_q - floor((-sogi_step_diff_q + 16) / 32);
    end

    sogi_c = sogi_coeff_from_step(sogi_step_q, FS_HZ, PLL_TABLE_SIZE, ...
        PLL_THETA_SUBDIV, PLL_STEP_SCALE);

    v_in_q = v_in_count * 2^SOGI_STATE_Q;
    acc_va = sogi_c.b0 * v_in_q ...
           - sogi_c.b0 * sogi_vin_n2 ...
           - sogi_c.a1 * sogi_va_n1 ...
           - sogi_c.a2 * sogi_va_n2;
    acc_vb = sogi_c.qb0 * v_in_q ...
           + sogi_c.qb1 * sogi_vin_n1 ...
           + sogi_c.qb2 * sogi_vin_n2 ...
           - sogi_c.a1 * sogi_vb_n1 ...
           - sogi_c.a2 * sogi_vb_n2;

    va_new = q_round(acc_va, 30);
    vb_new = q_round(acc_vb, 30);
    sogi_va = va_new;
    sogi_vb = vb_new;

    sogi_vin_n2 = sogi_vin_n1;
    sogi_vin_n1 = v_in_q;
    sogi_va_n2 = sogi_va_n1;
    sogi_va_n1 = va_new;
    sogi_vb_n2 = sogi_vb_n1;
    sogi_vb_n1 = vb_new;

    theta_det_idx = mod(pll_idx + detector_phase_comp_deg, PLL_TABLE_SIZE);
    sin_theta = sin_interp(SinTable360, theta_det_idx, theta_acc_q, PLL_THETA_DEN_Q);
    cos_theta = sin_interp(SinTable360, mod(theta_det_idx + 90, PLL_TABLE_SIZE), theta_acc_q, PLL_THETA_DEN_Q);
    vq_q = q_round(sogi_va * cos_theta + sogi_vb * sin_theta, 15);

    abs_va = abs(sogi_va);
    abs_vb = abs(sogi_vb);
    mag_max = max(abs_va, abs_vb);
    mag_min = min(abs_va, abs_vb);
    vmag_q = mag_max + c_rshift(mag_min, 1);
    vmag_q = max(vmag_q, PLL_VMAG_MIN * 2^SOGI_STATE_Q);

    raw_err = floor((PLL_ERR_SIGN * vq_q * PLL_ERR_SCALE) / vmag_q) - PLL_ERR_BIAS;
    raw_err = min(max(raw_err, -PLL_ERR_LIMIT), PLL_ERR_LIMIT);

    pll_err_lpf = pll_err_lpf + c_rshift(raw_err - pll_err_lpf, PLL_LPF_SHIFT);
    pll_err_slow_lpf_q8 = pll_err_slow_lpf_q8 + c_rshift((pll_err_lpf * 256) - pll_err_slow_lpf_q8, 5);
    if pll_err_slow_lpf_q8 >= 0
        pll_err_slow_lpf = floor((pll_err_slow_lpf_q8 + 128) / 256);
    else
        pll_err_slow_lpf = -floor((-pll_err_slow_lpf_q8 + 128) / 256);
    end
    abs_err_lpf = abs(pll_err_lpf);
    abs_err_phase = abs(pll_err_slow_lpf);

    if abs_err_phase <= PLL_GRID_PHASE_ON_ERR_TH
        pll_phase_ok_count = min(pll_phase_ok_count + 1, PLL_GRID_PHASE_ON_COUNT_TH);
        pll_phase_bad_count = 0;
        if pll_phase_ok_count >= PLL_GRID_PHASE_ON_COUNT_TH
            phase_ok = 1;
        end
    elseif abs_err_phase > PLL_GRID_PHASE_OFF_ERR_TH
        pll_phase_ok_count = 0;
        pll_phase_bad_count = min(pll_phase_bad_count + 1, PLL_GRID_PHASE_OFF_COUNT_TH);
        if pll_phase_bad_count >= PLL_GRID_PHASE_OFF_COUNT_TH
            pll_phase_bad_count = 0;
            phase_ok = 0;
        end
    end

    % ===== PI error source =====
    % This version intentionally makes the post-step / locked PI softer.
    % The previous versions allowed the proportional term and integrator to
    % kick the estimated frequency too hard, which created square-wave-like
    % PLL step hunting after the 60 -> 60.4 Hz transition.
    if pll_locked && phase_ok
        pll_err_pi = pll_err_slow_lpf;
        pll_err_pi = min(max(pll_err_pi, -PLL_ERR_PI_CLAMP_LOCK), PLL_ERR_PI_CLAMP_LOCK);
    elseif pll_locked
        pll_err_pi = pll_err_lpf;
        pll_err_pi = min(max(pll_err_pi, -PLL_ERR_PI_CLAMP_TRACK), PLL_ERR_PI_CLAMP_TRACK);
    else
        pll_err_pi = pll_err_lpf;
    end

    % Only apply deadband in the fine locked state.  Do not use a large
    % deadband while recovering, otherwise phase error accumulates and then
    % the loop makes a large correction.
    if pll_locked && phase_ok && abs(pll_err_pi) <= PLL_PI_DEADBAND_LOCK
        pll_err_pi = 0;
    end

    % ===== Integrator update =====
    if pll_err_pi == 0 && abs_err_lpf <= PLL_LOCK_ERR_TH
        if PLL_FIXED_NCO_TEST
            if pll_int_acc > 0
                leak = max(c_shift_round(pll_int_acc, PLL_INT_LEAK_SHIFT), 1);
                pll_int_acc = max(pll_int_acc - leak, 0);
            elseif pll_int_acc < 0
                leak = max(c_shift_round(-pll_int_acc, PLL_INT_LEAK_SHIFT), 1);
                pll_int_acc = min(pll_int_acc + leak, 0);
            end
        end
    else
        if pll_locked && ~phase_ok
            % During phase recovery, integrate at half rate to prevent windup.
            pll_int_acc = pll_int_acc + c_shift_round(pll_err_pi, 1);
        else
            pll_int_acc = pll_int_acc + pll_err_pi;
        end
    end
    pll_int_acc = min(max(pll_int_acc, -PLL_INT_LIMIT), PLL_INT_LIMIT);

    % ===== FAST / TRACK / LOCK gain selection =====
    if (~pll_locked) || (abs_err_lpf > PLL_LOCK_OFF_ERR_TH)
        kp_shift = PLL_KP_SHIFT_FAST;
        ki_shift = PLL_KI_SHIFT_FAST;
    elseif (~phase_ok) || (abs_err_phase > PLL_GRID_PHASE_OFF_ERR_TH)
        kp_shift = PLL_KP_SHIFT_TRACK;
        ki_shift = PLL_KI_SHIFT_TRACK;
    else
        kp_shift = PLL_KP_SHIFT_LOCK;
        ki_shift = PLL_KI_SHIFT_LOCK;
    end

    w_delta_cmd_q = c_shift_round(pll_err_pi * PLL_STEP_SCALE, kp_shift) ...
                  + c_shift_round(pll_int_acc * PLL_STEP_SCALE, ki_shift);
    w_delta_cmd_q = min(max(w_delta_cmd_q, -PLL_TRIM_LIMIT_Q), PLL_TRIM_LIMIT_Q);

    % Filter the frequency correction command in locked mode.  This is the
    % main change that prevents the PLL estimated frequency from jumping down
    % and up like a square wave after a frequency disturbance.
    if ~pll_locked
        pll_w_delta_filt_q = w_delta_cmd_q;
    else
        if phase_ok
            w_lpf_shift = PLL_W_DELTA_LPF_SHIFT_LOCK;
        else
            w_lpf_shift = PLL_W_DELTA_LPF_SHIFT_TRACK;
        end
        w_delta_diff_q = w_delta_cmd_q - pll_w_delta_filt_q;
        w_delta_step_q = c_shift_round(w_delta_diff_q, w_lpf_shift);
        if w_delta_step_q == 0 && w_delta_diff_q ~= 0
            if w_delta_diff_q > 0
                w_delta_step_q = 1;
            else
                w_delta_step_q = -1;
            end
        end
        pll_w_delta_filt_q = pll_w_delta_filt_q + w_delta_step_q;
    end

    w_delta_q = min(max(pll_w_delta_filt_q, -PLL_TRIM_LIMIT_Q), PLL_TRIM_LIMIT_Q);
    pll_step_target_q = min(max(PLL_WNOM_STEP_Q + w_delta_q, PLL_WMIN_STEP_Q), PLL_WMAX_STEP_Q);

    if PLL_FIXED_NCO_TEST
        pll_int_acc = 0;
        pll_w_delta_filt_q = 0;
        pll_step_q = PLL_WNOM_STEP_Q;
    else
        pll_step_diff_q = pll_step_target_q - pll_step_q;
        pll_step_diff_q = min(max(pll_step_diff_q, -PLL_STEP_SLEW_Q), PLL_STEP_SLEW_Q);
        pll_step_q = pll_step_q + pll_step_diff_q;
    end

    theta_acc_q = theta_acc_q + floor(pll_step_q * theta_gain);
    step_degrees = floor(theta_acc_q / 2^PLL_THETA_DEN_SHIFT);
    theta_acc_q = theta_acc_q - step_degrees * 2^PLL_THETA_DEN_SHIFT;
    pll_idx = mod(pll_idx + step_degrees, PLL_TABLE_SIZE);

    if an1_signal_valid && abs_err_lpf <= PLL_LOCK_ON_ERR_TH
        pll_lock_count = min(pll_lock_count + 1, PLL_LOCK_COUNT_TH);
        pll_unlock_count = 0;
        if pll_lock_count >= PLL_LOCK_COUNT_TH
            pll_locked = 1;
        end
    elseif (~an1_signal_valid) || abs_err_lpf > PLL_LOCK_OFF_ERR_TH
        pll_lock_count = 0;
        if pll_locked
            pll_unlock_count = min(pll_unlock_count + 1, PLL_UNLOCK_COUNT_TH);
            if pll_unlock_count >= PLL_UNLOCK_COUNT_TH
                pll_unlock_count = 0;
                pll_locked = 0;
            end
        else
            pll_unlock_count = 0;
            pll_locked = 0;
        end
    end

    sin_ref = sin_interp(SinTable360, pll_idx, theta_acc_q, PLL_THETA_DEN_Q);
    i_ref_count = floor((sin_ref * IREF_PEAK_COUNTS) / 32768);
    pll_sync_phase = sin_ref >= 0;

    theta_pos_q = pll_idx * PLL_THETA_DEN_Q + theta_acc_q;
    theta_full_q = PLL_TABLE_SIZE * PLL_THETA_DEN_Q;
    table_phase = floor((theta_pos_q * PLL_VREF_FULL_LEN + theta_full_q / 2) / theta_full_q);
    if table_phase >= PLL_VREF_FULL_LEN
        table_phase = 0;
    end
    if table_phase >= PLL_VREF_TABLE_LEN
        pll_vref_idx = table_phase - PLL_VREF_TABLE_LEN;
    else
        pll_vref_idx = table_phase;
    end
    pll_vref_idx = min(pll_vref_idx, PLL_VREF_TABLE_LEN - 1);
    pll_vref_count = VrefTable(pll_vref_idx + 1);

    log_va(n) = c_shift_round(sogi_va, SOGI_STATE_Q);
    log_vb(n) = c_shift_round(sogi_vb, SOGI_STATE_Q);
    log_vq(n) = c_shift_round(vq_q, SOGI_STATE_Q);
    log_raw_err(n) = raw_err;
    log_err(n) = pll_err_lpf;
    log_err_slow(n) = pll_err_slow_lpf;
    log_lock(n) = pll_locked;
    log_phase_ok(n) = phase_ok;
    log_idx(n) = pll_idx;
    log_vref_idx(n) = pll_vref_idx;
    log_iref(n) = i_ref_count;
    log_vref(n) = pll_vref_count;
    log_phase(n) = pll_sync_phase;
    log_freq_x100(n) = step_to_freq_x100(pll_step_q, FS_HZ, PLL_TABLE_SIZE, PLL_THETA_SUBDIV, PLL_STEP_SCALE);
end

log.t = t;
log.freq_in = log_freq_in;
log.an1 = log_an1;
log.vin = log_vin;
log.va = log_va;
log.vb = log_vb;
log.vq = log_vq;
log.raw_err = log_raw_err;
log.err = log_err;
log.err_slow = log_err_slow;
log.lock = log_lock;
log.phase_ok = log_phase_ok;
log.idx = log_idx;
log.vref_idx = log_vref_idx;
log.iref = log_iref;
log.vref = log_vref;
log.phase = log_phase;
log.freq_x100 = log_freq_x100;
log.input_valid = log_input_valid;
end

%% Helpers
function y = c_rshift(x, q)
    y = floor(double(x) / 2^q);
end

function y = c_shift_round(x, q)
    if x >= 0
        y = floor(double(x) / 2^q);
    else
        y = -floor((double(-x) + (2^q - 1)) / 2^q);
    end
end

function y = q_round(x, q)
    if x >= 0
        y = floor((double(x) + 2^(q-1)) / 2^q);
    else
        y = -floor((double(-x) + 2^(q-1)) / 2^q);
    end
end

function y = sin_interp(tab, idx0, frac_q, den_q)
    idx0 = mod(idx0, 360);
    idx1 = mod(idx0 + 1, 360);
    y0 = tab(idx0 + 1);
    y1 = tab(idx1 + 1);
    dy = y1 - y0;
    if dy >= 0
        interp = floor((dy * frac_q + den_q / 2) / den_q);
    else
        interp = -floor((-dy * frac_q + den_q / 2) / den_q);
    end
    y = y0 + interp;
end

function c = sogi_coeff_from_step(step_q, fs_hz, table_size, theta_subdiv, step_scale)
    freq_x10 = step_to_freq_x10(step_q, fs_hz, table_size, theta_subdiv, step_scale);

    SOGI55_A1_Q30 = evalin('base', 'SOGI55_A1_Q30');
    SOGI55_A2_Q30 = evalin('base', 'SOGI55_A2_Q30');
    SOGI55_B0_Q30 = evalin('base', 'SOGI55_B0_Q30');
    SOGI55_QB0_Q30 = evalin('base', 'SOGI55_QB0_Q30');
    SOGI55_QB1_Q30 = evalin('base', 'SOGI55_QB1_Q30');
    SOGI55_QB2_Q30 = evalin('base', 'SOGI55_QB2_Q30');

    SOGI60_A1_Q30 = evalin('base', 'SOGI60_A1_Q30');
    SOGI60_A2_Q30 = evalin('base', 'SOGI60_A2_Q30');
    SOGI60_B0_Q30 = evalin('base', 'SOGI60_B0_Q30');
    SOGI60_QB0_Q30 = evalin('base', 'SOGI60_QB0_Q30');
    SOGI60_QB1_Q30 = evalin('base', 'SOGI60_QB1_Q30');
    SOGI60_QB2_Q30 = evalin('base', 'SOGI60_QB2_Q30');

    SOGI65_A1_Q30 = evalin('base', 'SOGI65_A1_Q30');
    SOGI65_A2_Q30 = evalin('base', 'SOGI65_A2_Q30');
    SOGI65_B0_Q30 = evalin('base', 'SOGI65_B0_Q30');
    SOGI65_QB0_Q30 = evalin('base', 'SOGI65_QB0_Q30');
    SOGI65_QB1_Q30 = evalin('base', 'SOGI65_QB1_Q30');
    SOGI65_QB2_Q30 = evalin('base', 'SOGI65_QB2_Q30');

    if freq_x10 <= 600
        c.a1  = interp_coeff(freq_x10, 550, 600, SOGI55_A1_Q30,  SOGI60_A1_Q30);
        c.a2  = interp_coeff(freq_x10, 550, 600, SOGI55_A2_Q30,  SOGI60_A2_Q30);
        c.b0  = interp_coeff(freq_x10, 550, 600, SOGI55_B0_Q30,  SOGI60_B0_Q30);
        c.qb0 = interp_coeff(freq_x10, 550, 600, SOGI55_QB0_Q30, SOGI60_QB0_Q30);
        c.qb1 = interp_coeff(freq_x10, 550, 600, SOGI55_QB1_Q30, SOGI60_QB1_Q30);
        c.qb2 = interp_coeff(freq_x10, 550, 600, SOGI55_QB2_Q30, SOGI60_QB2_Q30);
    else
        c.a1  = interp_coeff(freq_x10, 600, 650, SOGI60_A1_Q30,  SOGI65_A1_Q30);
        c.a2  = interp_coeff(freq_x10, 600, 650, SOGI60_A2_Q30,  SOGI65_A2_Q30);
        c.b0  = interp_coeff(freq_x10, 600, 650, SOGI60_B0_Q30,  SOGI65_B0_Q30);
        c.qb0 = interp_coeff(freq_x10, 600, 650, SOGI60_QB0_Q30, SOGI65_QB0_Q30);
        c.qb1 = interp_coeff(freq_x10, 600, 650, SOGI60_QB1_Q30, SOGI65_QB1_Q30);
        c.qb2 = interp_coeff(freq_x10, 600, 650, SOGI60_QB2_Q30, SOGI65_QB2_Q30);
    end
end

function y = interp_coeff(x, x0, x1, y0, y1)
    if x <= x0
        y = y0;
    elseif x >= x1
        y = y1;
    else
        y = round(y0 + (y1 - y0) * (x - x0) / (x1 - x0));
    end
end

function f = step_to_freq_x10(step_q, fs_hz, table_size, theta_subdiv, step_scale)
    f = floor(step_q * fs_hz * 10 / (table_size * theta_subdiv * step_scale));
end

function f = step_to_freq_x100(step_q, fs_hz, table_size, theta_subdiv, step_scale)
    den = table_size * theta_subdiv * step_scale;
    f = floor((step_q * fs_hz * 100 + den / 2) / den);
end
