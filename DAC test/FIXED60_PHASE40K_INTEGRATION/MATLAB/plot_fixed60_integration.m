function plot_fixed60_integration(outdir,t,r,idx,held,theta,href,eref,trace,window,events,c,sh,se)
% NEW CANDIDATE - NOT YET VERIFIED. Plot data, not precomputed reference images.
M=double(c.full)*8;take=window(1):min(window(end),window(1)+round(3*40000/60));
overview=1:8:numel(t);at=((1:size(r,1))'*8-1)/40000;
fig=figure('Visible','off');plot(t(take),sin(2*pi*60*t(take)),t(take),double(eref(take))/32768);
legend('Grid AC','Extrapolated Q15 reference');grid on;xlabel('Time (s)');ylabel('Normalized');
emit(fig,outdir,'01_grid_reference');
fig=figure('Visible','off');plot(at,double(r(:,12))*5000/double(c.full));grid on;
xlabel('Time (s)');ylabel('PLL Hz');emit(fig,outdir,'02_pll_frequency');
fig=figure('Visible','off');
ph=mod(double(held)*360/M-360*60*t+180,360)-180;
pe=mod(double(theta)*360/M-360*60*t+180,360)-180;
plot(t(overview),ph(overview),t(overview),pe(overview));legend('Held','Extrapolated');grid on;
xlabel('Time (s)');ylabel('Circular phase error (deg)');emit(fig,outdir,'03_phase_error');
fig=figure('Visible','off');plot(at,double(r(:,7)));yline(25,':');yline(-25,':');yline(45,'--');yline(-45,'--');
grid on;xlabel('Time (s)');ylabel('Slow error (counts)');emit(fig,outdir,'04_slow_error');
fig=figure('Visible','off');plot(at,double(r(:,14)),at,double(r(:,15)),'--');legend('phase_ok','locked');
grid on;ylim([-0.1 1.1]);xlabel('Time (s)');emit(fig,outdir,'05_phase_ok_locked');
fig=figure('Visible','off');plot(t(take),double(held(take))*360/M,t(take),double(theta(take))*360/M);
legend('Held phase','Extrapolated phase');grid on;xlabel('Time (s)');ylabel('Wrapped degrees');
emit(fig,outdir,'06_phase_delivery');
fig=figure('Visible','off');plot(t(take),double(href(take))/32768,t(take),double(eref(take))/32768);
legend('Held sine','Extrapolated sine');grid on;xlabel('Time (s)');emit(fig,outdir,'07_sine_delivery');
fig=figure('Visible','off');
for k=1:2
 center=events{k+1}(1);z=max(1,center-40):min(numel(t),center+40);
 subplot(2,1,k);stairs(t(z),double(href(z))/32768);hold on;plot(t(z),double(eref(z))/32768);
 plot(t(z),sin(2*pi*60*t(z)),':');legend('Held','Extrapolated','Grid');grid on;xlabel('Time (s)');
end
emit(fig,outdir,'08_positive_negative_zc_zoom');
fig=figure('Visible','off');center=events{1}(2);z=center-10:center+10;
subplot(2,1,1);stairs(t(z),double(held(z))*360/M);hold on;plot(t(z),double(theta(z))*360/M);
xline(t(center),':');legend('Held','Extrapolated');ylabel('Degrees');grid on;
subplot(2,1,2);stairs(t(z),double(trace(z,7))*360/M);xline(t(center),':');
ylabel('Correction on NEXT tick (deg)');xlabel('Time (s)');grid on;emit(fig,outdir,'09_anchor_boundary_zoom');
fig=figure('Visible','off');semilogx(sh.Frequency_Hz(2:end),sh.Power_dBc_per_bin(2:end));hold on;
semilogx(se.Frequency_Hz(2:end),se.Power_dBc_per_bin(2:end));xlim([30 20000]);ylim([-180 5]);
legend('Held','Extrapolated');grid on;xlabel('Hz');ylabel('dBc per FFT bin');
emit(fig,outdir,'10_spectrum');
fig=figure('Visible','off');
for k=1:2
 center=events{k+3}(1);z=max(1,center-20):min(numel(t),center+20);
 subplot(2,1,k);plot(t(z),double(held(z))*360/M,t(z),double(theta(z))*360/M);
 grid on;xlabel('Time (s)');ylabel('Degrees');
end
emit(fig,outdir,'11_peak_wrap_zoom');
end
function emit(fig,path,name)
saveas(fig,fullfile(path,[name '.png']));savefig(fig,fullfile(path,[name '.fig']));close(fig);
end
