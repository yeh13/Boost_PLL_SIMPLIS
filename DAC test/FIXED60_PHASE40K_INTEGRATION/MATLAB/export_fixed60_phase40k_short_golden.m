function export_fixed60_phase40k_short_golden()
% Verification-only helper. It does not change the candidate algorithm.
% Generates a short deterministic 60 Hz golden trace for full RTL parity checking.
root_dir=fileparts(mfilename('fullpath')); addpath(root_dir);

c=firmware_config();
Fs=40000; Fpll=5000; duration=2.0; % short, deterministic, easy to debug

tadc=(0:Fs*duration-1)'/Fs;
adc=uint16(round(2048 + 1860*sin(2*pi*60*tadc)));
[r,pdiag]=stage17_model_60hz(adc,'locked_fixed_60');

% Current stage17_model_60hz pdiag contract:
% col1 = hold
% col2 = entry_count
% col3 = release_count
% col4 = theta_before
% col5 = integration_residual
% col6 = target_minus_previous_step
% col7 = after_frequency_deadband
assert(size(pdiag,2)==7,'Unexpected stage17_model_60hz diagnostic format: expected 7 columns.');
assert(size(r,2)==18,'Unexpected stage17_model_60hz output format: expected 18 columns.');

% Use the same authoritative sources already used by run_fixed60_phase40k.m.
anchors=pdiag(:,4); % pre-update PLL phase anchor at each 5 kHz update
steps=r(:,12);      % actual_step from the 5 kHz model output
valid=r(:,2)~=0;

d=phase_delivery_8tick(anchors,steps,valid,c.full);
% Raw phase-delivery generator output is N anchors * 8 rows.
% The full-candidate RTL golden comparison window is the physically valid ADC window,
% therefore only the rows that remain inside the 2-second stimulus are exported.
N5=numel(anchors);
N40= numel(tadc(8:end)); % physical comparison window: first valid PLL anchor at t=7/40000; last valid sample at t=79999/40000
trace=d.trace(1:N40,:);
M=d.modulus;

time_s=tadc(8:end);
held=anchors(repelem((1:N5)',8));
held=held(1:N40);
theta=double(trace(:,2));
base=double(trace(:,3));
phase_error=double(trace(:,4));
q=double(trace(:,5));
remainder=double(trace(:,6));
applied=double(trace(:,7));
remaining=double(trace(:,8));
substep=double(trace(:,9));
held_phase=double(held)*8; % same anchor time converted to Q24-phase units for the export
extrapolated_phase=double(theta);
held_ref=sine_q15_ticks(held_phase,M,c.sin);
extrapolated_ref=sine_q15_ticks(theta,M,c.sin);

% 5 kHz states are expanded onto the canonical 40 kHz comparison timeline by
% the same anchor-to-substep mapping that defines the valid input window.
idx=repelem((1:N5)',8);
idx=idx(1:N40);
hold_flags_40k = double(pdiag(idx,1));
phase_ok_flags_40k = double(r(idx,14));
locked_flags_40k = double(r(idx,15));
actual_step_40k = double(r(idx,12));
pll_step_q_40k = double(r(idx,12));

assert(numel(time_s)==N40,'time_s length mismatch: expected %d, got %d',N40,numel(time_s));
assert(numel(theta)==N40,'theta_40k length mismatch: expected %d, got %d',N40,numel(theta));
assert(numel(base)==N40,'base_step_40k length mismatch: expected %d, got %d',N40,numel(base));
assert(numel(phase_error)==N40,'phase_error length mismatch: expected %d, got %d',N40,numel(phase_error));
assert(numel(q)==N40,'correction_quotient length mismatch: expected %d, got %d',N40,numel(q));
assert(numel(remainder)==N40,'correction_remainder length mismatch: expected %d, got %d',N40,numel(remainder));
assert(numel(applied)==N40,'correction_applied length mismatch: expected %d, got %d',N40,numel(applied));
assert(numel(remaining)==N40,'correction_remaining length mismatch: expected %d, got %d',N40,numel(remaining));
assert(numel(substep)==N40,'substep length mismatch: expected %d, got %d',N40,numel(substep));
assert(numel(held_ref)==N40,'held_ref length mismatch: expected %d, got %d',N40,numel(held_ref));
assert(numel(extrapolated_ref)==N40,'extrapolated_ref length mismatch: expected %d, got %d',N40,numel(extrapolated_ref));
assert(numel(hold_flags_40k)==N40,'hold_60hz length mismatch: expected %d, got %d',N40,numel(hold_flags_40k));
assert(numel(phase_ok_flags_40k)==N40,'phase_ok length mismatch: expected %d, got %d',N40,numel(phase_ok_flags_40k));
assert(numel(locked_flags_40k)==N40,'locked length mismatch: expected %d, got %d',N40,numel(locked_flags_40k));
assert(numel(actual_step_40k)==N40,'actual_step length mismatch: expected %d, got %d',N40,numel(actual_step_40k));
assert(numel(pll_step_q_40k)==N40,'pll_step_q length mismatch: expected %d, got %d',N40,numel(pll_step_q_40k));

out = table(time_s(:), held_phase(:), pll_step_q_40k(:), actual_step_40k(:), hold_flags_40k(:), phase_ok_flags_40k(:), locked_flags_40k(:), ...
    theta(:), base(:), phase_error(:), q(:), remainder(:), applied(:), remaining(:), substep(:), ...
    held_phase(:), theta(:), held_ref(:), extrapolated_ref(:), 'VariableNames', {...
    'time_s','pll_phase_anchor','pll_step_q','actual_step','hold_60hz','phase_ok','locked', ...
    'theta_40k','base_step_40k','phase_error','correction_quotient','correction_remainder', ...
    'correction_applied','correction_remaining','substep','held_phase','extrapolated_phase','held_ref','extrapolated_ref'});

outdir=fullfile(root_dir,'RESULTS');
if ~exist(outdir,'dir'), mkdir(outdir); end
outpath=fullfile(outdir,'golden_fixed60_phase40k_short.csv');
writetable(out,outpath);
fprintf('Wrote short golden trace: %s\n', outpath);
end
