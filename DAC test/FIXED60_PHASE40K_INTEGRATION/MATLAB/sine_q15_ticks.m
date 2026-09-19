function y = sine_q15_ticks(theta,M,table)
% Exact source Q15 table interpolation, including eighth-Q-unit phase fractions.
% M=360*2^24; one degree = 2^24 ticks.
theta=mod(int64(theta),M);den=idivide(M,int64(360));idx=idivide(theta,den,'floor');
frac=mod(theta,den);v0=table(double(idx)+1);v1=table(mod(double(idx)+1,360)+1);delta=v1-v0;
y=v0+sign(delta).*idivide(abs(delta).*frac+idivide(den,int64(2)),den,'floor');
end
