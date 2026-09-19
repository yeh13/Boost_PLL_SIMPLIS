function [out, diagnostic] = stage17_model_60hz(adc, strategy)
% Independent int64 transcription of active callback; one row per 8 ADC calls.
% Arithmetic intermediates are int64, division explicitly fix/floor.
% Intended 12-bit test domain only. Host C parity must be executed separately.
if nargin<2, strategy='baseline'; end
assert(any(strcmp(strategy,{'baseline','locked_fixed_60'})),'Only the two 60 Hz strategies are supported');
c=firmware_config(); assert(isa(adc,'uint16') && mod(numel(adc),8)==0);
N=numel(adc)/8; out=zeros(N,18,'int64');
z=int64(0); offset=int64(2048*4096); va=z;vb=z; a1=z;a2=z;b1=z;b2=z;x1=z;x2=z;
err=z; slowq=z;slow=z;integ=z;step=c.nom;theta=z;raw=z;pierr=z;target=z;delta=z;
valid=false; cnt=0;lo=z;hi=z;pon=0;poff=0;phaseok=false;lon=0;loff=0;locked=false;
iref=z;vidx=z;vr=z;
hold=false;entry_count=0;release_count=0;
diagnostic=zeros(N,7,'int64');
% Columns: hold, entry_count, release_count, theta_before, integration_residual,
%          target_minus_previous_step, after_frequency_deadband.
% Existing source thresholds/counters: on 25/20; off 45/50.

for k=1:N
 theta_before=theta;before_deadband=z;after_deadband=z;
 u=int64(adc(8*k)); offset=offset+dv(u*4096-offset,int64(4096));
 center=sh(offset+2048,12); x=u-center;
 if cnt==0,lo=x;hi=x;else,lo=min(lo,x);hi=max(hi,x);end
 cnt=cnt+1;
 if cnt>=100,valid=dv(hi-lo,int64(2))>=300;cnt=0;end
 if ~valid
  hold=false;entry_count=0;release_count=0;
  va=z;vb=z;a1=z;a2=z;b1=z;b2=z;x1=z;x2=z;err=z;slowq=z;slow=z;integ=z;
  step=c.nom;theta=z;phaseok=false;locked=false;pon=0;poff=0;lon=0;loff=0;iref=z;vr=z;
 else
  f=dv(step*50000,c.full);
  if f<=600,j=1;pos=max(int64(0),min(int64(50),f-550));else,j=2;pos=max(int64(0),min(int64(50),f-600));end
  dd=c.coeff(j+1,:)-c.coeff(j,:);
  cf=c.coeff(j,:)+sign(dd).*dv(abs(dd).*pos+25,int64(50));
  xq=x*4096;
  va=qr(cf(3)*xq-cf(3)*x2-cf(1)*a1-cf(2)*a2,30);
  vb=qr(cf(4)*xq+cf(5)*x1+cf(6)*x2-cf(1)*b1-cf(2)*b2,30);
  x2=x1;x1=xq;a2=a1;a1=va;b2=b1;b1=vb;
  sn=lookup(theta+int64(4*2^21),c);cs=lookup(theta+int64(94*2^21),c);
  vq=qr(va*cs+vb*sn,15);
  mag=max(max(abs(va),abs(vb))+sh(min(abs(va),abs(vb)),1),int64(100*4096));
  raw=max(int64(-1000),min(int64(1000),dv(vq*1000,mag)));
  err=err+sh(raw-err,6);slowq=slowq+sh(err*256-slowq,5);slow=qr(slowq,8);
  if abs(slow)<=25
   pon=min(20,pon+1);poff=0;if pon>=20,phaseok=true;end
  elseif abs(slow)>45
   pon=0;poff=min(50,poff+1);if poff>=50,poff=0;phaseok=false;end
  end
  pierr=err;if abs(err)<=80 && abs(pierr)<=5,pierr=z;end
  if ~(pierr==0 && abs(err)<=80),integ=integ+pierr;end
  integ=max(-c.ilim,min(c.ilim,integ));
  if ~locked && abs(err)>80,kp=2;ki=13;else,kp=5;ki=15;end
  wd=sh(pierr*4096,kp)+sh(integ*4096,ki);wd=max(-c.trim,min(c.trim,wd));
  target=max(c.wmin,min(c.wmax,c.nom+wd));delta=target-step;
  before_deadband=delta;
  if abs(delta)<=c.fdead,delta=z;end
  after_deadband=delta;
  delta=max(-c.slew,min(c.slew,delta));step=step+delta;
  if strcmp(strategy,'locked_fixed_60')
   if hold
    % Same hysteresis semantics as source: middle band preserves off count.
    if abs(slow)>45,release_count=min(50,release_count+1);
    elseif abs(slow)<=25,release_count=0;end
    if release_count>=50 || ~locked
     hold=false;entry_count=0;release_count=0;
    end
   else
    % Require 20 consecutive jointly-qualified updates (4 ms at 5 kHz).
    if locked && phaseok && abs(slow)<=25,entry_count=min(20,entry_count+1);
    else,entry_count=0;end
    if entry_count>=20,hold=true;release_count=0;end
   end
   if hold,step=c.nom;end
  end
  theta=mod(theta+step,c.full);
  if abs(err)<=170
   lon=min(20,lon+1);loff=0;if lon>=20,locked=true;end
  elseif abs(err)>300
   lon=0;if locked,loff=min(100,loff+1);if loff>=100,loff=0;locked=false;end
   else,loff=0;locked=false;end
  end
  sr=lookup(theta,c);iref=sh(sr*925,15);
  tp=dv(theta*668+dv(c.full,int64(2)),c.full);if tp>=668,tp=z;end
  vidx=mod(tp,int64(334));vr=c.vref(double(vidx)+1);
 end
 residual=z;
 if valid,residual=mod(theta-theta_before-step+idivide(c.full,int64(2)),c.full)-idivide(c.full,int64(2));end
 diagnostic(k,:)=[int64(hold) int64(entry_count) int64(release_count) theta_before residual before_deadband after_deadband];
 out(k,:)=[offset int64(valid) va vb raw err slow pierr integ target delta step theta int64(phaseok) int64(locked) iref vidx vr];
end
end
function y=sh(x,q)
y=idivide(x,bitshift(int64(1),q),'floor');
end
function y=dv(x,d)
y=idivide(x,d,'fix');
end
function y=qr(x,q)
y=sign(x).*idivide(abs(x)+bitshift(int64(1),q-1),bitshift(int64(1),q),'floor');
end
function y=lookup(theta,c)
theta=mod(theta,c.full);idx=idivide(theta,int64(2^21),'floor');frac=mod(theta,int64(2^21));
v0=c.sin(double(idx)+1);v1=c.sin(mod(double(idx)+1,360)+1);dd=v1-v0;
y=v0+sign(dd).*idivide(abs(dd)*frac+int64(2^20),int64(2^21),'floor');
end