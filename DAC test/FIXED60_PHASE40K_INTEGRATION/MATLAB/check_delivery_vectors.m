function report=check_delivery_vectors(c)
% NEW CANDIDATE - NOT YET VERIFIED
% Synthetic anchor transport tests at unchanged 60-Hz step; no other AC frequency.
M=c.full*8;degree=int64(2^24);
errors=int64([-2^24*2 -19 -9 -8 -7 -1 0 1 7 8 9 19 2^24*2]);
ok=false(numel(errors)+3,1);names=cell(size(ok));
for k=1:numel(errors)
 % arbitrary eighth-Q initial state exercises remainder 0..7 of both signs.
 seed=mod(-errors(k),M);
 d=phase_delivery_8tick(int64(0),c.nom,true,c.full,seed);
 ok(k)=d.interval(1,1)==errors(k) && d.interval(1,2)==errors(k) && ...
 d.interval(1,3)==0 && d.trace(end,8)==0 && all(d.trace(:,11)==0) && ...
 all(abs(d.trace(:,7))<=idivide(abs(errors(k))+7,int64(8),'floor'));
 names{k}=sprintf('signed error %d ticks: exact sum, endpoint, no reset',errors(k));
end
d=phase_delivery_8tick(int64(2^21),c.nom,true,c.full,int64(359)*degree);
ok(end-2)=d.interval(1,1)==2*degree;names{end-2}='359 to 1 degrees is +2 degrees';
d=phase_delivery_8tick(int64(359*2^21),c.nom,true,c.full,degree);
ok(end-1)=d.interval(1,1)==-2*degree;names{end-1}='1 to 359 degrees is -2 degrees';
% Deliberately non-integer /8 base step: no base remainder is lost in Q24.
step=c.nom+1;anchor=mod((int64(0):int64(15))'*step,c.full);
d=phase_delivery_8tick(anchor,repmat(step,16,1),true(16,1),c.full);
oracle=mod((int64(0):int64(127))'*step,M);
ok(end)=isequal(d.trace(:,2),oracle) && all(d.interval(:,3)==0);
names{end}='128 sample-time points match exact ramp (detects one-tick latency)';
report=table(names,ok,'VariableNames',{'Check','Satisfied'});
end
