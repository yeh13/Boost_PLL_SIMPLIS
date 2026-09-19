function d=phase_delivery_8tick(anchor,step,valid,full,initial_theta)
% NEW CANDIDATE - NOT YET VERIFIED
% Anchors are PRE-update PLL theta at ADC timestamp t_n. No re-anchoring.
% Internal unit = degree Q24 = 1/8 of original Q21 unit; base is exact step/8.
% Row substep s records theta(t_n+s/40k) and correction for the NEXT tick.
if nargin<5,initial_theta=int64(0);end
anchor=int64(anchor(:));step=int64(step(:));valid=logical(valid(:));
N=numel(anchor);assert(numel(step)==N && numel(valid)==N);
M=full*8;phase=initial_theta;trace=zeros(N*8,12,'int64');
interval=zeros(N,5,'int64');
for n=1:N
 a=anchor(n)*8;base=step(n);
 if valid(n)
  error=circ(a-phase,M);q=idivide(error,int64(8),'fix');r=error-q*8;
 else,error=int64(0);q=int64(0);r=int64(0);end
 remaining=error;acc=int64(0);total=int64(0);
 startphase=phase;target=mod(a+step(n)*8,M);
 for sub=0:7
  correction=q;acc=acc+abs(r);
  if acc>=8,correction=correction+sign(r);acc=acc-8;end
  if valid(n),increment=base+correction;else,increment=int64(0);correction=int64(0);end
  remaining=remaining-correction;total=total+correction;
  next=mod(phase+increment,M);
  residual=circ(next-phase-increment,M);
  j=(n-1)*8+sub+1;
  trace(j,:)=[a phase base error q r correction remaining int64(sub) increment residual target];
  phase=next;
 end
 interval(n,:)=[error total circ(target-phase,M) startphase phase];
end
d.trace=trace;d.interval=interval;d.modulus=M;
d.names={'pll_phase_anchor_ticks','theta_40k_ticks','base_step_40k_ticks','phase_error_ticks',...
 'correction_quotient_ticks','correction_remainder_ticks','correction_applied_next_ticks',...
 'correction_remaining_ticks','substep','increment_next_ticks','integration_residual_ticks','anchor_target_end_ticks'};
end
function e=circ(x,M)
e=mod(x+idivide(M,int64(2)),M)-idivide(M,int64(2));
end
