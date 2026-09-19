function c = firmware_config()
% Frozen from active E-drive source, 2026-09-10. No coefficient redesign.
root_dir = fileparts(mfilename('fullpath'));
c.fs_design=5000; c.decim=8; c.full=int64(360*512*4096);
c.nom=int64(floor(60*double(c.full)/5000));
c.wmin=int64(floor(45*double(c.full)/5000)); c.wmax=int64(floor(75*double(c.full)/5000));
c.ilim=int64(floor(30*double(c.full)/5000)); c.trim=int64(floor(15*double(c.full)/5000));
c.slew=int64(floor(100*double(c.full)/5000^2)); c.fdead=int64(floor(double(c.full)/50000));
c.coeff=int64([-2042651804 973794573 49973625 1726965 3453929 1726965;
-2033145701 965191209 54275307 2046131 4092262 2046131;
-2023645686 956665874 58537975 2390732 4781464 2390732]);
c.sin=int64(readmatrix(fullfile(root_dir,'sin_table.csv')));
c.vref=int64(readmatrix(fullfile(root_dir,'vref_table.csv')));
c.names={'offset_q','valid','va_q12','vb_q12','raw_error','error_lpf','error_slow','pi_error','integrator','target_step','slew_delta','actual_step','phase_q','phase_ok','locked','iref','vref_idx','vref'};
end
