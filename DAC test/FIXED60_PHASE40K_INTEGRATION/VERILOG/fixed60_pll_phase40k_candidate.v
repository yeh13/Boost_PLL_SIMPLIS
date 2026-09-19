// NEW CANDIDATE - NOT YET VERIFIED
// Single ordered clock process avoids an NBA mailbox / one-tick delivery delay.
// Numeric PLL body is retained from fixed60_pll_5k_candidate; no control retuning.
module fixed60_pll_phase40k_candidate(
 input wire [11:0] ac_input,input wire clk,input wire reset,
 output reg phase_ok,output reg locked,output reg hold_60hz,
 output wire [29:0] pll_phase_5k,output wire [29:0] pll_phase_post_5k,
 output wire signed [31:0] pll_step_q,output wire signed [31:0] slow_error,
 output wire input_valid,output reg pll_update,
 output reg [32:0] theta_40k,
 output reg signed [63:0] base_step_40k,
 output reg signed [63:0] phase_error,
 output reg signed [63:0] correction_quotient,
 output reg signed [63:0] correction_remainder,
 output reg signed [63:0] correction_applied,
 output reg signed [63:0] correction_remaining,
 output reg [2:0] substep,
 output reg signed [15:0] held_ref,output reg signed [15:0] extrapolated_ref,
 output reg signed [63:0] anchor_residual,
 output reg interval_complete);
localparam signed [63:0] FULL=754974720, NOM=9059696, SLEW=3019, FDEAD=15099;
reg signed [63:0] offset,x,xq,va,vb,a1,a2,b1,b2,x1,x2,err,slowq,slow,integ,step,theta;
reg signed [63:0] ca,cb,cc,cd,ce,cf,fx,pos,aa,bb,sn,cs,vq,mag,raw,pierr,wd,target,delta;
reg signed [63:0] lowv,highv,center,idx,frac,anchor,error_phase,tmp;
integer decim,cnt,pon,poff,lon,loff,kp,ki;
reg valid;
reg signed [63:0] phase_before;
integer entry_count,release_count;
assign pll_phase_5k=phase_before[29:0];
assign pll_phase_post_5k=theta[29:0];
assign pll_step_q=step[31:0];
assign slow_error=slow[31:0];
assign input_valid=valid;
function automatic signed [63:0] rnd(input signed [63:0] v,input integer q);
 begin if(v>=0)rnd=(v+(64'sd1<<<(q-1)))>>>q;else rnd=-((-v+(64'sd1<<<(q-1)))>>>q);end
endfunction
function automatic signed [63:0] ab(input signed [63:0] v);
 begin ab=v<0?-v:v;end
endfunction
function automatic signed [63:0] wrap(input signed [63:0] v,input signed [63:0] m);
 begin wrap=v%m;if(wrap<0)wrap=wrap+m;end
endfunction
function automatic signed [63:0] interp(input signed [63:0] a,input signed [63:0] b,input signed [63:0] p);
 begin if(b>=a)interp=a+((b-a)*p+25)/50;else interp=a-((a-b)*p+25)/50;end
endfunction
function automatic signed [63:0] sintab(input integer i);
 begin case(i)
0: sintab=0;
1: sintab=572;
2: sintab=1144;
3: sintab=1715;
4: sintab=2286;
5: sintab=2856;
6: sintab=3425;
7: sintab=3993;
8: sintab=4560;
9: sintab=5126;
10: sintab=5690;
11: sintab=6252;
12: sintab=6813;
13: sintab=7371;
14: sintab=7927;
15: sintab=8481;
16: sintab=9032;
17: sintab=9580;
18: sintab=10126;
19: sintab=10668;
20: sintab=11207;
21: sintab=11743;
22: sintab=12275;
23: sintab=12803;
24: sintab=13328;
25: sintab=13848;
26: sintab=14364;
27: sintab=14876;
28: sintab=15383;
29: sintab=15886;
30: sintab=16383;
31: sintab=16876;
32: sintab=17364;
33: sintab=17846;
34: sintab=18323;
35: sintab=18794;
36: sintab=19260;
37: sintab=19720;
38: sintab=20173;
39: sintab=20621;
40: sintab=21062;
41: sintab=21497;
42: sintab=21925;
43: sintab=22347;
44: sintab=22762;
45: sintab=23170;
46: sintab=23571;
47: sintab=23964;
48: sintab=24351;
49: sintab=24730;
50: sintab=25101;
51: sintab=25465;
52: sintab=25821;
53: sintab=26169;
54: sintab=26509;
55: sintab=26841;
56: sintab=27165;
57: sintab=27481;
58: sintab=27788;
59: sintab=28087;
60: sintab=28377;
61: sintab=28659;
62: sintab=28932;
63: sintab=29196;
64: sintab=29451;
65: sintab=29697;
66: sintab=29934;
67: sintab=30162;
68: sintab=30381;
69: sintab=30591;
70: sintab=30791;
71: sintab=30982;
72: sintab=31163;
73: sintab=31335;
74: sintab=31498;
75: sintab=31650;
76: sintab=31794;
77: sintab=31927;
78: sintab=32051;
79: sintab=32165;
80: sintab=32269;
81: sintab=32364;
82: sintab=32448;
83: sintab=32523;
84: sintab=32587;
85: sintab=32642;
86: sintab=32687;
87: sintab=32722;
88: sintab=32747;
89: sintab=32762;
90: sintab=32767;
91: sintab=32762;
92: sintab=32747;
93: sintab=32722;
94: sintab=32687;
95: sintab=32642;
96: sintab=32587;
97: sintab=32523;
98: sintab=32448;
99: sintab=32364;
100: sintab=32269;
101: sintab=32165;
102: sintab=32051;
103: sintab=31927;
104: sintab=31794;
105: sintab=31650;
106: sintab=31498;
107: sintab=31335;
108: sintab=31163;
109: sintab=30982;
110: sintab=30791;
111: sintab=30591;
112: sintab=30381;
113: sintab=30162;
114: sintab=29934;
115: sintab=29697;
116: sintab=29451;
117: sintab=29196;
118: sintab=28932;
119: sintab=28659;
120: sintab=28377;
121: sintab=28087;
122: sintab=27788;
123: sintab=27481;
124: sintab=27165;
125: sintab=26841;
126: sintab=26509;
127: sintab=26169;
128: sintab=25821;
129: sintab=25465;
130: sintab=25101;
131: sintab=24730;
132: sintab=24351;
133: sintab=23964;
134: sintab=23571;
135: sintab=23170;
136: sintab=22762;
137: sintab=22347;
138: sintab=21925;
139: sintab=21497;
140: sintab=21062;
141: sintab=20621;
142: sintab=20173;
143: sintab=19720;
144: sintab=19260;
145: sintab=18794;
146: sintab=18323;
147: sintab=17846;
148: sintab=17364;
149: sintab=16876;
150: sintab=16383;
151: sintab=15886;
152: sintab=15383;
153: sintab=14876;
154: sintab=14364;
155: sintab=13848;
156: sintab=13328;
157: sintab=12803;
158: sintab=12275;
159: sintab=11743;
160: sintab=11207;
161: sintab=10668;
162: sintab=10126;
163: sintab=9580;
164: sintab=9032;
165: sintab=8481;
166: sintab=7927;
167: sintab=7371;
168: sintab=6813;
169: sintab=6252;
170: sintab=5690;
171: sintab=5126;
172: sintab=4560;
173: sintab=3993;
174: sintab=3425;
175: sintab=2856;
176: sintab=2286;
177: sintab=1715;
178: sintab=1144;
179: sintab=572;
180: sintab=0;
181: sintab=-572;
182: sintab=-1144;
183: sintab=-1715;
184: sintab=-2286;
185: sintab=-2856;
186: sintab=-3425;
187: sintab=-3993;
188: sintab=-4560;
189: sintab=-5126;
190: sintab=-5690;
191: sintab=-6252;
192: sintab=-6813;
193: sintab=-7371;
194: sintab=-7927;
195: sintab=-8481;
196: sintab=-9032;
197: sintab=-9580;
198: sintab=-10126;
199: sintab=-10668;
200: sintab=-11207;
201: sintab=-11743;
202: sintab=-12275;
203: sintab=-12803;
204: sintab=-13328;
205: sintab=-13848;
206: sintab=-14364;
207: sintab=-14876;
208: sintab=-15383;
209: sintab=-15886;
210: sintab=-16384;
211: sintab=-16876;
212: sintab=-17364;
213: sintab=-17846;
214: sintab=-18323;
215: sintab=-18794;
216: sintab=-19260;
217: sintab=-19720;
218: sintab=-20173;
219: sintab=-20621;
220: sintab=-21062;
221: sintab=-21497;
222: sintab=-21925;
223: sintab=-22347;
224: sintab=-22762;
225: sintab=-23170;
226: sintab=-23571;
227: sintab=-23964;
228: sintab=-24351;
229: sintab=-24730;
230: sintab=-25101;
231: sintab=-25465;
232: sintab=-25821;
233: sintab=-26169;
234: sintab=-26509;
235: sintab=-26841;
236: sintab=-27165;
237: sintab=-27481;
238: sintab=-27788;
239: sintab=-28087;
240: sintab=-28377;
241: sintab=-28659;
242: sintab=-28932;
243: sintab=-29196;
244: sintab=-29451;
245: sintab=-29697;
246: sintab=-29934;
247: sintab=-30162;
248: sintab=-30381;
249: sintab=-30591;
250: sintab=-30791;
251: sintab=-30982;
252: sintab=-31163;
253: sintab=-31335;
254: sintab=-31498;
255: sintab=-31650;
256: sintab=-31794;
257: sintab=-31927;
258: sintab=-32051;
259: sintab=-32165;
260: sintab=-32269;
261: sintab=-32364;
262: sintab=-32448;
263: sintab=-32523;
264: sintab=-32587;
265: sintab=-32642;
266: sintab=-32687;
267: sintab=-32722;
268: sintab=-32747;
269: sintab=-32762;
270: sintab=-32767;
271: sintab=-32762;
272: sintab=-32747;
273: sintab=-32722;
274: sintab=-32687;
275: sintab=-32642;
276: sintab=-32587;
277: sintab=-32523;
278: sintab=-32448;
279: sintab=-32364;
280: sintab=-32269;
281: sintab=-32165;
282: sintab=-32051;
283: sintab=-31927;
284: sintab=-31794;
285: sintab=-31650;
286: sintab=-31498;
287: sintab=-31335;
288: sintab=-31163;
289: sintab=-30982;
290: sintab=-30791;
291: sintab=-30591;
292: sintab=-30381;
293: sintab=-30162;
294: sintab=-29934;
295: sintab=-29697;
296: sintab=-29451;
297: sintab=-29196;
298: sintab=-28932;
299: sintab=-28659;
300: sintab=-28377;
301: sintab=-28087;
302: sintab=-27788;
303: sintab=-27481;
304: sintab=-27165;
305: sintab=-26841;
306: sintab=-26509;
307: sintab=-26169;
308: sintab=-25821;
309: sintab=-25465;
310: sintab=-25101;
311: sintab=-24730;
312: sintab=-24351;
313: sintab=-23964;
314: sintab=-23571;
315: sintab=-23170;
316: sintab=-22762;
317: sintab=-22347;
318: sintab=-21925;
319: sintab=-21497;
320: sintab=-21062;
321: sintab=-20621;
322: sintab=-20173;
323: sintab=-19720;
324: sintab=-19260;
325: sintab=-18794;
326: sintab=-18323;
327: sintab=-17846;
328: sintab=-17364;
329: sintab=-16876;
330: sintab=-16384;
331: sintab=-15886;
332: sintab=-15383;
333: sintab=-14876;
334: sintab=-14364;
335: sintab=-13848;
336: sintab=-13328;
337: sintab=-12803;
338: sintab=-12275;
339: sintab=-11743;
340: sintab=-11207;
341: sintab=-10668;
342: sintab=-10126;
343: sintab=-9580;
344: sintab=-9032;
345: sintab=-8481;
346: sintab=-7927;
347: sintab=-7371;
348: sintab=-6813;
349: sintab=-6252;
350: sintab=-5690;
351: sintab=-5126;
352: sintab=-4560;
353: sintab=-3993;
354: sintab=-3425;
355: sintab=-2856;
356: sintab=-2286;
357: sintab=-1715;
358: sintab=-1144;
359: sintab=-572;
 default:sintab=0;endcase end
endfunction
function automatic signed [63:0] sine(input signed [63:0] th);
 reg signed [63:0] t,j,fr,y0,dy;
 begin t=wrap(th,FULL);j=t>>>21;fr=t%(64'sd1<<<21);y0=sintab(j);dy=sintab((j+1)%360)-y0;
 sine=y0+rnd(dy*fr,21);end
endfunction

// Delivery units: degree Q24, full turn FULL*8. base_step_40k numeric code
// equals PLL step Q21 code, thus its physical increment is precisely step/8.
reg signed [63:0] delivery_theta,delivery_anchor,delivery_target;
reg signed [63:0] remainder_accumulator,correction_sum,delivery_increment;
function automatic signed [63:0] sine40(input signed [63:0] th);
 reg signed [63:0] t,j,fr,y0,dy;
 begin
 t=wrap(th,FULL*8);j=t>>>24;fr=t%(64'sd1<<<24);
 y0=sintab(j);dy=sintab((j+1)%360)-y0;
 sine40=y0+rnd(dy*fr,24);
 end
endfunction

always @(posedge clk or posedge reset) begin
 if(reset)begin
 offset=8388608;x=0;xq=0;va=0;vb=0;a1=0;a2=0;b1=0;b2=0;x1=0;x2=0;
 err=0;slowq=0;slow=0;integ=0;step=NOM;theta=0;decim=0;cnt=0;lowv=0;highv=0;
 pon=0;poff=0;lon=0;loff=0;valid=0;phase_ok=0;locked=0;phase_before=0;hold_60hz=0;entry_count=0;release_count=0;pll_update=0;
 delivery_theta=0;delivery_anchor=0;delivery_target=0;
 remainder_accumulator=0;correction_sum=0;delivery_increment=0;
 theta_40k=0;base_step_40k=NOM;phase_error=0;correction_quotient=0;correction_remainder=0;
 correction_applied=0;correction_remaining=0;substep=0;held_ref=0;extrapolated_ref=0;
 anchor_residual=0;interval_complete=0;
 end else begin
 pll_update=0;decim=decim+1;
 if(decim==8)begin
 pll_update=1;phase_before=theta;
 decim=0;offset=offset+((64'sd1*ac_input*4096-offset)/4096);center=(offset+2048)>>>12;x=ac_input-center;
 if(cnt==0)begin lowv=x;highv=x;end else begin if(x<lowv)lowv=x;if(x>highv)highv=x;end
 cnt=cnt+1;if(cnt==100)begin valid=((highv-lowv)/2>=300);cnt=0;end
 if(!valid)begin
 va=0;vb=0;a1=0;a2=0;b1=0;b2=0;x1=0;x2=0;err=0;slowq=0;slow=0;integ=0;step=NOM;theta=0;
 phase_ok=0;locked=0;pon=0;poff=0;lon=0;loff=0;hold_60hz=0;entry_count=0;release_count=0;
 end else begin
 fx=step*50000/FULL;
 if(fx<=600)begin
 pos=fx-550;if(pos<0)pos=0;if(pos>50)pos=50;
 ca=interp(-2042651804,-2033145701,pos);cb=interp(973794573,965191209,pos);
 cc=interp(49973625,54275307,pos);cd=interp(1726965,2046131,pos);
 ce=interp(3453929,4092262,pos);cf=cd;
 end else begin
 pos=fx-600;if(pos<0)pos=0;if(pos>50)pos=50;
 ca=interp(-2033145701,-2023645686,pos);cb=interp(965191209,956665874,pos);
 cc=interp(54275307,58537975,pos);cd=interp(2046131,2390732,pos);
 ce=interp(4092262,4781464,pos);cf=cd;
 end
 xq=x*4096;va=rnd(cc*xq-cc*x2-ca*a1-cb*a2,30);vb=rnd(cd*xq+ce*x1+cf*x2-ca*b1-cb*b2,30);
 x2=x1;x1=xq;a2=a1;a1=va;b2=b1;b1=vb;
 sn=sine(theta+8388608);cs=sine(theta+197132288);vq=rnd(va*cs+vb*sn,15);
 aa=ab(va);bb=ab(vb);if(aa>=bb)mag=aa+(bb>>>1);else mag=bb+(aa>>>1);
 if(mag<409600)mag=409600;raw=vq*1000/mag;if(raw>1000)raw=1000;if(raw< -1000)raw=-1000;
 err=err+((raw-err)>>>6);slowq=slowq+((err*256-slowq)>>>5);slow=rnd(slowq,8);
 if(ab(slow)<=25)begin if(pon<20)pon=pon+1;poff=0;if(pon>=20)phase_ok=1;end
 else if(ab(slow)>45)begin pon=0;if(poff<50)poff=poff+1;if(poff>=50)begin poff=0;phase_ok=0;end end
 pierr=err;if(ab(err)<=80 && ab(pierr)<=5)pierr=0;
 if(!(pierr==0 && ab(err)<=80))integ=integ+pierr;
 if(integ>4529848)integ=4529848;if(integ< -4529848)integ=-4529848;
 if(!locked && ab(err)>80)begin kp=2;ki=13;end else begin kp=5;ki=15;end
 wd=((pierr*4096)>>>kp)+((integ*4096)>>>ki);
 if(wd>2264924)wd=2264924;if(wd< -2264924)wd=-2264924;
 target=NOM+wd;if(target>11324620)target=11324620;if(target<6794772)target=6794772;
 delta=target-step;if(ab(delta)<=FDEAD)delta=0;if(delta>SLEW)delta=SLEW;if(delta< -SLEW)delta=-SLEW;
 step=step+delta;
 if(hold_60hz)begin
  if(ab(slow)>45)begin if(release_count<50)release_count=release_count+1;end
  else if(ab(slow)<=25)release_count=0;
  if(release_count>=50 || !locked)begin hold_60hz=0;entry_count=0;release_count=0;end
 end else begin
  if(locked && phase_ok && ab(slow)<=25)begin if(entry_count<20)entry_count=entry_count+1;end
  else entry_count=0;
  if(entry_count>=20)begin hold_60hz=1;release_count=0;end
 end
 if(hold_60hz)step=NOM;
 theta=wrap(theta+step,FULL);
 if(ab(err)<=170)begin if(lon<20)lon=lon+1;loff=0;if(lon>=20)locked=1;end
 else if(ab(err)>300)begin lon=0;if(locked)begin if(loff<100)loff=loff+1;if(loff>=100)begin loff=0;locked=0;end end else loff=0;end
 end
 end
 // PLL computation above completes first, in THIS clock event.
 // phase_before represents this ADC timestamp; delivery_theta already holds
 // the prediction for this SAME timestamp from the preceding event.
 interval_complete=0;
 if(pll_update)begin
  substep=0;delivery_anchor=phase_before*8;base_step_40k=step;
  phase_error=0;
  if(valid)phase_error=wrap(delivery_anchor-delivery_theta+FULL*4,FULL*8)-FULL*4;
  correction_quotient=phase_error/8;
  correction_remainder=phase_error-correction_quotient*8;
  remainder_accumulator=0;correction_sum=0;correction_remaining=phase_error;
  delivery_target=wrap(delivery_anchor+step*8,FULL*8);
 end else substep=substep+1'b1;
 // Present current theta without ANY anchor assignment/reset.
 theta_40k=delivery_theta[32:0];
 held_ref=sine(phase_before);
 extrapolated_ref=sine40(delivery_theta);
 // Debug correction_applied is the outgoing increment toward NEXT edge.
 correction_applied=correction_quotient;
 remainder_accumulator=remainder_accumulator+ab(correction_remainder);
 if(remainder_accumulator>=8)begin
  if(correction_remainder>0)correction_applied=correction_applied+1;
  else if(correction_remainder<0)correction_applied=correction_applied-1;
  remainder_accumulator=remainder_accumulator-8;
 end
 delivery_increment=0;
 if(valid)delivery_increment=base_step_40k+correction_applied;
 else correction_applied=0;
 correction_remaining=correction_remaining-correction_applied;
 correction_sum=correction_sum+correction_applied;
 // Store next-time phase; output theta_40k remains the CURRENT-time sample.
 delivery_theta=wrap(delivery_theta+delivery_increment,FULL*8);
 if(substep==7 && valid)begin
  anchor_residual=wrap(delivery_target-delivery_theta+FULL*4,FULL*8)-FULL*4;
  interval_complete=1;
 end
 end
end
endmodule
