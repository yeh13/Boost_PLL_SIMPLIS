function [values,spectrum,zc] = reference_metrics(t,phase_ticks,reference,anchor_residual,M)
% Common 40 kHz, exactly 20-second window. No Signal Processing Toolbox.
fs=40000;grid_hz=60;y=double(reference(:))/32768;t=t(:);N=numel(t);
assert(N==20*fs,'Use identical 20-second window for both references');
phase_deg=double(phase_ticks(:))*360/double(M);
pe=mod(phase_deg-360*grid_hz*t+180,360)-180;
zc=cell(1,2);errors=cell(1,2);
for polarity=1:2
 if polarity==1,ii=find(y(1:end-1)<0 & y(2:end)>=0);offset=0;
 else,ii=find(y(1:end-1)>0 & y(2:end)<=0);offset=0.5;end
 assert(numel(ii)>10,'Insufficient crossings');
 % Linear crossing estimate is METROLOGY only, never reference generation.
 zc{polarity}=t(ii)-y(ii).*(t(ii+1)-t(ii))./(y(ii+1)-y(ii));
 nearest=(round(zc{polarity}*grid_hz-offset)+offset)/grid_hz;
 errors{polarity}=(zc{polarity}-nearest)*1e6;
end
fit=polyfit((0:numel(zc{1})-1)',zc{1},1);fundamental=1/fit(1);
% Periodic Hann and +/-2 bins integrate the small nominal-frequency leakage.
w=0.5-0.5*cos(2*pi*(0:N-1)'/N);
Y=fft((y-mean(y)).*w);F=(0:floor(N/2))'*fs/N;
P=abs(Y(1:numel(F))).^2;P(2:end-1)=2*P(2:end-1);
P=P/(N*sum(w.^2));df=fs/N;
fund=find(abs(F-fundamental)<=2.01*df);
harmonic_power=0;
% THD explicitly includes harmonics 2..40 (up to ~2.4 kHz).
for h=2:40,harmonic_power=harmonic_power+sum(P(abs(F-h*fundamental)<=2.01*df));end
nonfund=F>0 & F<20000;nonfund(fund)=false;
base=sum(P(fund));assert(base>0);
values=[fundamental;sqrt(mean(pe.^2));max(abs(pe));...
 sqrt(mean(errors{1}.^2));sqrt(mean(errors{2}.^2));...
 100*sqrt(harmonic_power/base);100*sqrt(sum(P(nonfund))/base);...
 max(abs(double(anchor_residual)))*360/double(M)];
spectrum=table(F,10*log10(max(P,realmin)/base),'VariableNames',{'Frequency_Hz','Power_dBc_per_bin'});
end
