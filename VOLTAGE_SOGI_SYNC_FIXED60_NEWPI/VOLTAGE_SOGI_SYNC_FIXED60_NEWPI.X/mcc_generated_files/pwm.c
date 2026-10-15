/**
  PWM Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    pwm.c

  @Summary
    This is the generated driver implementation file for the PWM driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This source file provides APIs for PWM.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.171.1
        Device            :  dsPIC33CK256MP506
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.70
        MPLAB 	          :  MPLAB X v5.50
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/

#include "pwm.h"
#include "clock.h"
#include "parameter.h"
#include "voltage_sogi_sync.h"

/**
 Section: Driver Interface Function Definitions
*/

// PWM Default PWM Generator Interrupt Handler
static void (*PWM_Generator1InterruptHandler)(void) = NULL;
static void (*PWM_Generator7InterruptHandler)(void) = NULL;

void PWM_Initialize (void)
{
    // MCLKSEL AFPLLO - Auxiliary Clock with PLL Enabled; HRERR disabled; LOCK disabled; DIVSEL 1:16; 
    PCLKCON = 0x33;
    // FSCL 0; 
    FSCL = 0x00;
    // FSMINPER 0; 
    FSMINPER = 0x00;
    // MPHASE 0; 
    MPHASE = 0x00;
    // MDC 0; 
    MDC = 0x00;
    // MPER 16; 
    MPER = 0x10;
    // LFSR 0; 
    LFSR = 0x00;
    // CTA7EN disabled; CTA8EN disabled; CTA1EN disabled; CTA2EN disabled; CTA5EN disabled; CTA6EN disabled; CTA3EN disabled; CTA4EN disabled; 
    CMBTRIGL = 0x00;
    // CTB8EN disabled; CTB3EN disabled; CTB2EN disabled; CTB1EN disabled; CTB7EN disabled; CTB6EN disabled; CTB5EN disabled; CTB4EN disabled; 
    CMBTRIGH = 0x00;
    // PWMLFA PWMS1 or PWMS2;; S1APOL Positive logic; S2APOL Positive logic; PWMLFAD No Assignment; PWMS1A PWM1H; PWMS2A PWM1H; 
    LOGCONA = 0x00;
    // PWMLFB PWMS1 | PWMS2; S2BPOL Positive logic; PWMLFBD No Assignment; S1BPOL Positive logic; PWMS2B PWM1H; PWMS1B PWM1H; 
    LOGCONB = 0x00;
    // PWMLFC PWMS1 | PWMS2; PWMLFCD No Assignment; S2CPOL Positive logic; S1CPOL Positive logic; PWMS1C PWM1H; PWMS2C PWM1H; 
    LOGCONC = 0x00;
    // PWMS1D PWM1H; S1DPOL Positive logic; PWMLFD PWMS1 | PWMS2; PWMLFDD No Assignment; S2DPOL Positive logic; PWMS2D PWM1H; 
    LOGCOND = 0x00;
    // PWMS1E PWM1H; PWMS2E PWM1H; S1EPOL Positive logic; PWMLFE PWMS1 | PWMS2; S2EPOL Positive logic; PWMLFED No Assignment; 
    LOGCONE = 0x00;
    // S1FPOL Positive logic; PWMS2F PWM1H; PWMS1F PWM1H; S2FPOL Positive logic; PWMLFFD No Assignment; PWMLFF PWMS1 | PWMS2; 
    LOGCONF = 0x00;
    // EVTASEL PGTRGSEL bits; EVTASYNC Not synchronized; EVTAPOL Active-high; EVTAPGS PG1; EVTASTRD Stretched to 8 PWM clock cycles minimum; EVTAOEN disabled; 
    PWMEVTA = 0x00;
    // EVTBPGS PG1; EVTBSYNC Not synchronized; EVTBPOL Active-high; EVTBSEL PGTRGSEL bits; EVTBSTRD Stretched to 8 PWM clock cycles minimum; EVTBOEN disabled; 
    PWMEVTB = 0x00;
    // EVTCPGS PG1; EVTCPOL Active-high; EVTCSEL PGTRGSEL bits; EVTCSTRD Stretched to 8 PWM clock cycles minimum; EVTCSYNC Not synchronized; EVTCOEN disabled; 
    PWMEVTC = 0x00;
    // EVTDOEN disabled; EVTDSTRD Stretched to 8 PWM clock cycles minimum; EVTDPOL Active-high; EVTDPGS PG1; EVTDSEL PGTRGSEL bits; EVTDSYNC Not synchronized; 
    PWMEVTD = 0x00;
    // EVTEOEN disabled; EVTEPOL Active-high; EVTEPGS PG1; EVTESTRD Stretched to 8 PWM clock cycles minimum; EVTESEL PGTRGSEL bits; EVTESYNC Not synchronized; 
    PWMEVTE = 0x00;
    // EVTFPOL Active-high; EVTFPGS PG1; EVTFSTRD Stretched to 8 PWM clock cycles minimum; EVTFSEL PGTRGSEL bits; EVTFOEN disabled; EVTFSYNC Not synchronized; 
    PWMEVTF = 0x00;
    // MSTEN disabled; TRGMOD Single trigger mode; SOCS Self-trigger; UPDMOD SOC update; MPHSEL disabled; MPERSEL disabled; MDCSEL disabled; 
    PG1CONH = 0x00;
    // MSTEN disabled; TRGMOD Single trigger mode; SOCS Self-trigger; UPDMOD SOC update; MPHSEL disabled; MPERSEL disabled; MDCSEL disabled; 
    PG2CONH = 0x00;
    // MSTEN disabled; TRGMOD Single trigger mode; SOCS Self-trigger; UPDMOD SOC update; MPHSEL disabled; MPERSEL disabled; MDCSEL disabled; 
    PG7CONH = 0x00;
    // TRSET disabled; UPDREQ disabled; CLEVT disabled; TRCLR disabled; CAP disabled; SEVT disabled; FFEVT disabled; UPDATE disabled; FLTEVT disabled; 
    PG1STAT = 0x00;
    // TRSET disabled; UPDREQ disabled; CLEVT disabled; TRCLR disabled; CAP disabled; SEVT disabled; FFEVT disabled; UPDATE disabled; FLTEVT disabled; 
    PG2STAT = 0x00;
    // TRSET disabled; UPDREQ disabled; CLEVT disabled; TRCLR disabled; CAP disabled; SEVT disabled; FFEVT disabled; UPDATE disabled; FLTEVT disabled; 
    PG7STAT = 0x00;
    // FLTDAT 0; DBDAT 0; SWAP disabled; OVRENH disabled; OVRENL disabled; OSYNC User output overrides are synchronized to the local PWM time base; CLMOD disabled; FFDAT 0; CLDAT 0; OVRDAT 0; 
    PG1IOCONL = 0x00;
    // FLTDAT 0; DBDAT 0; SWAP disabled; OVRENH disabled; OVRENL disabled; OSYNC User output overrides are synchronized to the local PWM time base; CLMOD disabled; FFDAT 0; CLDAT 0; OVRDAT 0; 
    PG2IOCONL = 0x00;
    // FLTDAT 0; DBDAT 0; SWAP disabled; OVRENH disabled; OVRENL disabled; OSYNC User output overrides are synchronized to the local PWM time base; CLMOD disabled; FFDAT 0; CLDAT 0; OVRDAT 0; 
    PG7IOCONL = 0x00;
    // PENL enabled; DTCMPSEL PCI Sync Logic; PMOD Complementary; POLL Active-high; PENH enabled; CAPSRC Software; POLH Active-high; 
    PG1IOCONH = 0x0C;
    // PENL enabled; DTCMPSEL PCI Sync Logic; PMOD Complementary; POLL Active-high; PENH enabled; CAPSRC Software; POLH Active-high; 
    PG2IOCONH = 0x0C;
    // PENL enabled; DTCMPSEL PCI Sync Logic; PMOD Independent; POLL Active-high; PENH enabled; CAPSRC Software; POLH Active-high; 
    PG7IOCONH = 0x1C;
    // UPDTRG Duty Cycle; ADTR1PS 1:1; PGTRGSEL EOC event; ADTR1EN3 disabled; ADTR1EN1 enabled; ADTR1EN2 disabled; 
    PG1EVTL = 0x108;
    // UPDTRG Duty Cycle; ADTR1PS 1:1; PGTRGSEL EOC event; ADTR1EN3 disabled; ADTR1EN1 disabled; ADTR1EN2 disabled; 
    PG2EVTL = 0x08;
    PG2EVTLbits.ADTR1EN1 = 1;
    PG2EVTLbits.ADTR1PS = 7; // ADC trigger only; PWM remains 40 kHz
    // UPDTRG Duty Cycle; ADTR1PS 1:1; PGTRGSEL EOC event; ADTR1EN3 disabled; ADTR1EN1 disabled; ADTR1EN2 disabled; 
    PG7EVTL = 0x08;
    // ADTR2EN1 disabled; IEVTSEL EOC; SIEN disabled; FFIEN disabled; ADTR1OFS None; CLIEN disabled; FLTIEN disabled; ADTR2EN2 disabled; ADTR2EN3 disabled; 
    PG1EVTH = 0x00;
    // ADTR2EN1 disabled; IEVTSEL EOC; SIEN disabled; FFIEN disabled; ADTR1OFS None; CLIEN disabled; FLTIEN disabled; ADTR2EN2 disabled; ADTR2EN3 disabled; 
    PG2EVTH = 0x00;
    // ADTR2EN1 disabled; IEVTSEL EOC; SIEN disabled; FFIEN disabled; ADTR1OFS None; CLIEN disabled; FLTIEN disabled; ADTR2EN2 disabled; ADTR2EN3 disabled; 
    PG7EVTH = 0x00;
    // PSS Tied to 0; PPS Not inverted; SWTERM disabled; PSYNC disabled; TERM Manual Terminate; AQPS Not inverted; AQSS None; TSYNCDIS PWM EOC; 
    PG1FPCIL = 0x00;
    // PSS Tied to 0; PPS Not inverted; SWTERM disabled; PSYNC disabled; TERM Manual Terminate; AQPS Not inverted; AQSS None; TSYNCDIS PWM EOC; 
    PG2FPCIL = 0x00;
    // PSS Tied to 0; PPS Not inverted; SWTERM disabled; PSYNC disabled; TERM Manual Terminate; AQPS Not inverted; AQSS None; TSYNCDIS PWM EOC; 
    PG7FPCIL = 0x00;
    // TQPS Not inverted; LATMOD disabled; SWPCI Drives '0'; BPEN disabled; TQSS None; SWPCIM PCI acceptance logic; BPSEL PWM Generator 1; ACP Level-sensitive; 
    PG1FPCIH = 0x00;
    // TQPS Not inverted; LATMOD disabled; SWPCI Drives '0'; BPEN disabled; TQSS None; SWPCIM PCI acceptance logic; BPSEL PWM Generator 1; ACP Level-sensitive; 
    PG2FPCIH = 0x00;
    // TQPS Not inverted; LATMOD disabled; SWPCI Drives '0'; BPEN disabled; TQSS None; SWPCIM PCI acceptance logic; BPSEL PWM Generator 1; ACP Level-sensitive; 
    PG7FPCIH = 0x00;
    // PSS Tied to 0; PPS Not inverted; SWTERM disabled; PSYNC disabled; TERM Manual Terminate; AQPS Not inverted; AQSS None; TSYNCDIS PWM EOC; 
    PG1CLPCIL = 0x00;
    // PSS Tied to 0; PPS Not inverted; SWTERM disabled; PSYNC disabled; TERM Manual Terminate; AQPS Not inverted; AQSS None; TSYNCDIS PWM EOC; 
    PG2CLPCIL = 0x00;
    // PSS Tied to 0; PPS Not inverted; SWTERM disabled; PSYNC disabled; TERM Manual Terminate; AQPS Not inverted; AQSS None; TSYNCDIS PWM EOC; 
    PG7CLPCIL = 0x00;
    // PCIGT disabled; TQPS Not inverted; SWPCI Drives '0'; BPEN disabled; TQSS None; SWPCIM PCI acceptance logic; BPSEL PWM Generator 1; ACP Level-sensitive; 
    PG1CLPCIH = 0x00;
    // PCIGT disabled; TQPS Not inverted; SWPCI Drives '0'; BPEN disabled; TQSS None; SWPCIM PCI acceptance logic; BPSEL PWM Generator 1; ACP Level-sensitive; 
    PG2CLPCIH = 0x00;
    // PCIGT disabled; TQPS Not inverted; SWPCI Drives '0'; BPEN disabled; TQSS None; SWPCIM PCI acceptance logic; BPSEL PWM Generator 1; ACP Level-sensitive; 
    PG7CLPCIH = 0x00;
    // PSS Tied to 0; PPS Not inverted; SWTERM disabled; PSYNC disabled; TERM Manual Terminate; AQPS Not inverted; AQSS None; TSYNCDIS PWM EOC; 
    PG1FFPCIL = 0x00;
    // PSS Tied to 0; PPS Not inverted; SWTERM disabled; PSYNC disabled; TERM Manual Terminate; AQPS Not inverted; AQSS None; TSYNCDIS PWM EOC; 
    PG2FFPCIL = 0x00;
    // PSS Tied to 0; PPS Not inverted; SWTERM disabled; PSYNC disabled; TERM Manual Terminate; AQPS Not inverted; AQSS None; TSYNCDIS PWM EOC; 
    PG7FFPCIL = 0x00;
    // PCIGT disabled; TQPS Not inverted; SWPCI Drives '0'; BPEN disabled; TQSS None; SWPCIM PCI acceptance logic; BPSEL PWM Generator 1; ACP Level-sensitive; 
    PG1FFPCIH = 0x00;
    // PCIGT disabled; TQPS Not inverted; SWPCI Drives '0'; BPEN disabled; TQSS None; SWPCIM PCI acceptance logic; BPSEL PWM Generator 1; ACP Level-sensitive; 
    PG2FFPCIH = 0x00;
    // PCIGT disabled; TQPS Not inverted; SWPCI Drives '0'; BPEN disabled; TQSS None; SWPCIM PCI acceptance logic; BPSEL PWM Generator 1; ACP Level-sensitive; 
    PG7FFPCIH = 0x00;
    // PSS Tied to 0; PPS Not inverted; SWTERM disabled; PSYNC disabled; TERM Manual Terminate; AQPS Not inverted; AQSS None; TSYNCDIS PWM EOC; 
    PG1SPCIL = 0x00;
    // PSS Tied to 0; PPS Not inverted; SWTERM disabled; PSYNC disabled; TERM Manual Terminate; AQPS Not inverted; AQSS None; TSYNCDIS PWM EOC; 
    PG2SPCIL = 0x00;
    // PSS Tied to 0; PPS Not inverted; SWTERM disabled; PSYNC disabled; TERM Manual Terminate; AQPS Not inverted; AQSS None; TSYNCDIS PWM EOC; 
    PG7SPCIL = 0x00;
    // PCIGT disabled; TQPS Not inverted; SWPCI Drives '0'; BPEN disabled; TQSS None; SWPCIM PCI acceptance logic; BPSEL PWM Generator 1; ACP Level-sensitive; 
    PG1SPCIH = 0x00;
    // PCIGT disabled; TQPS Not inverted; SWPCI Drives '0'; BPEN disabled; TQSS None; SWPCIM PCI acceptance logic; BPSEL PWM Generator 1; ACP Level-sensitive; 
    PG2SPCIH = 0x00;
    // PCIGT disabled; TQPS Not inverted; SWPCI Drives '0'; BPEN disabled; TQSS None; SWPCIM PCI acceptance logic; BPSEL PWM Generator 1; ACP Level-sensitive; 
    PG7SPCIH = 0x00;
    // LEB 0; 
    PG1LEBL = 0x00;
    // LEB 0; 
    PG2LEBL = 0x00;
    // LEB 0; 
    PG7LEBL = 0x00;
    // PWMPCI 1; PLR disabled; PLF disabled; PHR disabled; PHF disabled; 
    PG1LEBH = 0x00;
    // PWMPCI 1; PLR disabled; PLF disabled; PHR disabled; PHF disabled; 
    PG2LEBH = 0x00;
    // PWMPCI 1; PLR disabled; PLF disabled; PHR disabled; PHF disabled; 
    PG7LEBH = 0x00;
    // PHASE 0; 
    PG1PHASE = 0x00;
    // PHASE 0; 
    PG2PHASE = 0x00;
    // PHASE 0; 
    PG7PHASE = 0x00;
    // DC 0; 
    PG1DC = 0x00;
    // DC 0; 
    PG2DC = 0x00;
    // DC 0; 
    PG7DC = 0x00;
    // DCA 0; 
    PG1DCA = 0x00;
    // DCA 0; 
    PG2DCA = 0x00;
    // DCA 0; 
    PG7DCA = 0x00;
    // PER 12499; 
    PG1PER = 0x30D3;
    // PER 12499; 
    PG2PER = 0x30D3;
    // PER 52082; 
    PG7PER = 0xCB72;
    // TRIGA 10625; 
    PG1TRIGA = 10625;
    // TRIGA 0; 
    PG2TRIGA = 10625; // BITEXACT_FAST AN1 sampling point
    // TRIGA 0; 
    PG7TRIGA = 0x00;
    // TRIGB 0; 
    PG1TRIGB = 0x00;
    // TRIGB 0; 
    PG2TRIGB = 0x00;
    // TRIGB 0; 
    PG7TRIGB = 0x00;
    // TRIGC 0; 
    PG1TRIGC = 0x00;
    // TRIGC 0; 
    PG2TRIGC = 0x00;
    // TRIGC 0; 
    PG7TRIGC = 0x00;
    // DTL 100; 
    PG1DTL = 0x64;
    // DTL 100; 
    PG2DTL = 0x64;
    // DTL 0; 
    PG7DTL = 0x00;
    // DTH 100; 
    PG1DTH = 0x64;
    // DTH 100; 
    PG2DTH = 0x64;
    // DTH 0; 
    PG7DTH = 0x00;
    
    /* Initialize PWM Generator Interrupt Handler*/
    PWM_SetGenerator1InterruptHandler(&PWM_Generator1_CallBack);
    PWM_SetGenerator7InterruptHandler(&PWM_Generator7_CallBack);
    
    //PWM Generator 1 Interrupt
    IFS4bits.PWM1IF = 0;
    IEC4bits.PWM1IE = 1;
    
    //PWM Generator 7 Interrupt
    IFS4bits.PWM7IF = 0;
    IEC4bits.PWM7IE = 1;
    

    //Wait until AUX PLL clock is locked
    while(!CLOCK_AuxPllLockStatusGet());

    // HREN disabled; MODSEL Independent Edge; TRGCNT 1; CLKSEL Master clock; ON enabled; 
    PG1CONL = 0x8008;
    // HREN disabled; MODSEL Independent Edge; TRGCNT 1; CLKSEL Master clock; ON enabled; 
    PG2CONL = 0x8008;
    // HREN disabled; MODSEL Independent Edge; TRGCNT 1; CLKSEL Master clock divided by clock divider circuit; ON enabled; 
    PG7CONL = 0x8010;
}

unsigned short phase=0, ZeroCrossing=0;
unsigned short i=0, j=1, k=0;
//const int16_t Vdc = 1195;   //Vdc = 100V
const int16_t Vdc = 574;    //VDC = 48V
//const int16_t Vdc = 430;    //VDC = 36V

//closed loop ref table
const int16_t VrefTable[334]={
  {0},{36},{71},{107},{142},{178},{214},{249},{285},{320},{356},{391},{427},{462},{497},{533},{568},{603},{638},{673},{708},{743},{778},{813},{848},{882},{917},{951},{986},{1020},{1055},{1089},{1123},{1157},{1191},{1224},{1258},{1292},{1325},{1358},{1391},{1425},{1457},{1490},{1523},{1556},{1588},{1620},{1652},{1684},{1716},{1748},{1779},{1811},{1842},{1873},{1904},{1935},{1965},{1995},{2026},{2056},{2085},{2115},{2145},{2174},{2203},{2232},{2260},{2289},{2317},{2345},{2373},{2401},{2428},{2455},{2482},{2509},{2536},{2562},{2588},{2614},{2640},{2665},{2690},{2715},{2740},{2765},{2789},{2813},{2836},{2860},{2883},{2906},{2929},{2951},{2974},{2995},{3017},{3039},{3060},{3080},{3101},{3121},{3141},{3161},{3181},{3200},{3219},{3237},{3256},{3274},{3292},{3309},{3326},{3343},{3360},{3376},{3392},{3408},{3423},{3438},{3453},{3467},{3482},{3495},{3509},{3522},{3535},{3548},{3560},{3572},{3584},{3595},{3606},{3617},{3627},{3637},{3647},{3657},{3666},{3674},{3683},{3691},{3699},{3706},{3713},{3720},{3727},{3733},{3739},{3744},{3749},{3754},{3759},{3763},{3767},{3770},{3774},{3776},{3779},{3781},{3783},{3784},{3786},{3786},{3787},{3787},{3787},{3786},{3786},{3784},{3783},{3781},{3779},{3776},{3774},{3770},{3767},{3763},{3759},{3754},{3749},{3744},{3739},{3733},{3727},{3720},{3713},{3706},{3699},{3691},{3683},{3674},{3666},{3657},{3647},{3637},{3627},{3617},{3606},{3595},{3584},{3572},{3560},{3548},{3535},{3522},{3509},{3495},{3482},{3467},{3453},{3438},{3423},{3408},{3392},{3376},{3360},{3343},{3326},{3309},{3292},{3274},{3256},{3237},{3219},{3200},{3181},{3161},{3141},{3121},{3101},{3080},{3060},{3039},{3017},{2995},{2974},{2951},{2929},{2906},{2883},{2860},{2836},{2813},{2789},{2765},{2740},{2715},{2690},{2665},{2640},{2614},{2588},{2562},{2536},{2509},{2482},{2455},{2428},{2401},{2373},{2345},{2317},{2289},{2260},{2232},{2203},{2174},{2145},{2115},{2085},{2056},{2026},{1995},{1965},{1935},{1904},{1873},{1842},{1811},{1779},{1748},{1716},{1684},{1652},{1620},{1588},{1556},{1523},{1490},{1457},{1425},{1391},{1358},{1325},{1292},{1258},{1224},{1191},{1157},{1123},{1089},{1055},{1020},{986},{951},{917},{882},{848},{813},{778},{743},{708},{673},{638},{603},{568},{533},{497},{462},{427},{391},{356},{320},{285},{249},{214},{178},{142},{107},{71},{36}
};
/*
//open loop table
const int16_t VrefTable[334]={
    {0},{38},{76},{114},{153},{191},{229},{267},{305},{343},{381},{419},{457},{495},{533},{571},{608},{646},{684},{721},{759},{796},{834},{871},{908},{945},{982},{1019},{1056},{1093},{1130},{1166},{1203},{1239},{1276},{1312},{1348},{1384},{1420},{1455},{1491},{1526},{1562},{1597},{1632},{1667},{1701},{1736},{1770},{1805},{1839},{1873},{1906},{1940},{1973},{2007},{2040},{2073},{2105},{2138},{2170},{2202},{2234},{2266},{2298},{2329},{2360},{2391},{2422},{2452},{2483},{2513},{2543},{2572},{2602},{2631},{2660},{2688},{2717},{2745},{2773},{2801},{2828},{2856},{2883},{2909},{2936},{2962},{2988},{3014},{3039},{3064},{3089},{3114},{3138},{3162},{3186},{3209},{3233},{3256},{3278},{3301},{3323},{3344},{3366},{3387},{3408},{3428},{3449},{3469},{3488},{3508},{3527},{3545},{3564},{3582},{3600},{3617},{3634},{3651},{3668},{3684},{3700},{3715},{3730},{3745},{3760},{3774},{3788},{3801},{3814},{3827},{3840},{3852},{3864},{3875},{3886},{3897},{3908},{3918},{3927},{3937},{3946},{3955},{3963},{3971},{3979},{3986},{3993},{4000},{4006},{4012},{4017},{4022},{4027},{4032},{4036},{4040},{4043},{4046},{4049},{4051},{4053},{4055},{4056},{4057},{4057},{4058},{4057},{4057},{4056},{4055},{4053},{4051},{4049},{4046},{4043},{4040},{4036},{4032},{4027},{4022},{4017},{4012},{4006},{4000},{3993},{3986},{3979},{3971},{3963},{3955},{3946},{3937},{3927},{3918},{3908},{3897},{3886},{3875},{3864},{3852},{3840},{3827},{3814},{3801},{3788},{3774},{3760},{3745},{3730},{3715},{3700},{3684},{3668},{3651},{3634},{3617},{3600},{3582},{3564},{3545},{3527},{3508},{3488},{3469},{3449},{3428},{3408},{3387},{3366},{3344},{3323},{3301},{3278},{3256},{3233},{3209},{3186},{3162},{3138},{3114},{3089},{3064},{3039},{3014},{2988},{2962},{2936},{2909},{2883},{2856},{2828},{2801},{2773},{2745},{2717},{2688},{2660},{2631},{2602},{2572},{2543},{2513},{2483},{2452},{2422},{2391},{2360},{2329},{2298},{2266},{2234},{2202},{2170},{2138},{2105},{2073},{2040},{2007},{1973},{1940},{1906},{1873},{1839},{1805},{1770},{1736},{1701},{1667},{1632},{1597},{1562},{1526},{1491},{1455},{1420},{1384},{1348},{1312},{1276},{1239},{1203},{1166},{1130},{1093},{1056},{1019},{982},{945},{908},{871},{834},{796},{759},{721},{684},{646},{608},{571},{533},{495},{457},{419},{381},{343},{305},{267},{229},{191},{153},{114},{76},{38}
    
};
*/
int32_t Buck_PWM = 0, Boost_PWM = 0;
int32_t boost_dynamic_coefficient = 0;
int16_t finallyDuty_buck = 0;
int16_t finallyDuty_boost = 0;
int32_t B_Coefficient_BOOST_finally[3]={0,0,0};
/* PWM1-owned diagnostics; tick unit = one serviced PWM1 callback (nominal 25 us).
 * Ring reason 1=crossing, 2=guard. Original OR short-circuit gives crossing priority.
 * Halt CPU for coherent 32-bit diagnostic/ring reads. Never used for control.
 */
volatile uint32_t sogi_pwm_tick = 0, i_guard_reset_count = 0, pll_cross_reset_count = 0;
volatile uint32_t sogi_cross_gap_count = 0, sogi_guard_then_cross_count = 0;
volatile uint32_t sogi_last_guard_to_cross_ticks = 0;
typedef struct { uint32_t tick; uint16_t index_before, reason, generation, polarity; } SyncResetRecord;
volatile SyncResetRecord sogi_reset_log[64];
volatile uint16_t sogi_reset_log_head = 0;
static uint16_t sogi_last_seen_generation = 0;
static uint8_t sogi_guard_pending = 0;
static uint32_t sogi_last_guard_tick = 0;
void __attribute__ ((weak)) PWM_Generator1_CallBack(void)
{
    // Add Application code here
    SogiSyncSnapshot sync = VoltageSogiSync_Read();
    uint16_t generation_delta = (uint16_t)(sync.generation - sogi_last_seen_generation);
    uint16_t index_before, reason = 0;
    ZeroCrossing = 0;
    sogi_last_seen_generation = sync.generation;
    if (sync.valid && generation_delta != 0U) {
        phase = sync.polarity;
        ZeroCrossing = 1;
    }
    index_before = i;
    if ( ZeroCrossing == 1 || i > 333 ) {
        i = 0;
        reason = (ZeroCrossing == 1) ? 1U : 2U;
    }
    if ( VrefTable[i] >= Vdc ) {
        BOOST_DUTY();
        if ( phase == 1 ) Buck_PWM = 12500;
        else Buck_PWM = 0;
    }
    else{
        BUCK_DUTY();
        Boost_PWM = 0;
    }
    //PG1DC = Buck_PWM;
    //PG2DC = Boost_PWM;
    /* Observation only, after the original reset and feedforward calculations.
     * AN0 remains able to preempt; ring timestamps are serviced PWM ticks.
     */
    sogi_pwm_tick++;
    if (reason == 1U) {
        if (generation_delta != 1U) sogi_cross_gap_count++;
        pll_cross_reset_count++;
        if (sogi_guard_pending) {
            sogi_last_guard_to_cross_ticks = sogi_pwm_tick - sogi_last_guard_tick;
            sogi_guard_then_cross_count++;
            sogi_guard_pending = 0;
        }
    }
    if (reason == 2U) {
        i_guard_reset_count++;
        sogi_last_guard_tick = sogi_pwm_tick;
        sogi_guard_pending = 1;
    }
    if (reason != 0U) {
        uint16_t slot = sogi_reset_log_head;
        sogi_reset_log[slot].tick = sogi_pwm_tick;
        sogi_reset_log[slot].index_before = index_before;
        sogi_reset_log[slot].reason = reason;
        sogi_reset_log[slot].generation = sync.generation;
        sogi_reset_log[slot].polarity = phase;
        sogi_reset_log_head = (slot + 1U) & 63U;
    }
}

void BOOST_DUTY() 
{
    Boost_PWM = VrefTable[i] - Vdc;
    Boost_PWM *= 12500;
    Boost_PWM /= VrefTable[i] + Vdc;
    if ( Boost_PWM >= 12500) Boost_PWM = 12500;
    else if (Boost_PWM <= 0) Boost_PWM = 0;
}

void BUCK_DUTY() 
{
    Buck_PWM = VrefTable[i];
    Buck_PWM *= 6250;
    Buck_PWM /= Vdc;
    if ( phase == 1 ){
        Buck_PWM = 6250 + Buck_PWM;
    }
    else {
        Buck_PWM = 6250 - Buck_PWM;
    }
    if ( Buck_PWM >= 12500) Buck_PWM = 12500;
    else if (Buck_PWM <= 0) Buck_PWM = 0;
}

void PWM_SetGenerator1InterruptHandler(void *handler)
{
    PWM_Generator1InterruptHandler = handler;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _PWM1Interrupt (  )
{
    if(PWM_Generator1InterruptHandler)
    {
        // PWM Generator1 interrupt handler function
        PWM_Generator1InterruptHandler();
    }
    
    // clear the PWM Generator1 interrupt flag
    IFS4bits.PWM1IF = 0; 
}

void __attribute__ ((weak)) PWM_Generator2_CallBack(void)
{
    // Add Application code here
}

void PWM_Generator2_Tasks(void)
{
    if(IFS4bits.PWM2IF)
    {
        // PWM Generator2 callback function 
        PWM_Generator2_CallBack();

        // clear the PWM Generator2 interrupt flag
        IFS4bits.PWM2IF = 0;
    }
}

void __attribute__ ((weak)) PWM_Generator7_CallBack(void)
{
    // Add Application code here
    if ( j > 10 ) j = 1;
    if ( j <= 5 ) {
        PG7DC = 52083;
    }
    else {
        PG7DC = 0x00;
    }
    j++;
}

void PWM_SetGenerator7InterruptHandler(void *handler)
{
    PWM_Generator7InterruptHandler = handler;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _PWM7Interrupt (  )
{
    if(PWM_Generator7InterruptHandler)
    {
        // PWM Generator7 interrupt handler function
        PWM_Generator7InterruptHandler();
    }
    
    // clear the PWM Generator7 interrupt flag
    IFS4bits.PWM7IF = 0; 
}


void __attribute__ ((weak)) PWM_EventA_CallBack(void)
{
    // Add Application code here
}

void PWM_EventA_Tasks(void)
{
    if(IFS10bits.PEVTAIF)
    {
     
        // PWM EventA callback function 
        PWM_EventA_CallBack();
	
        // clear the PWM EventA interrupt flag
        IFS10bits.PEVTAIF = 0;
    }
}
void __attribute__ ((weak)) PWM_EventB_CallBack(void)
{
    // Add Application code here
}

void PWM_EventB_Tasks(void)
{
    if(IFS10bits.PEVTBIF)
    {
     
        // PWM EventB callback function 
        PWM_EventB_CallBack();
	
        // clear the PWM EventB interrupt flag
        IFS10bits.PEVTBIF = 0;
    }
}
void __attribute__ ((weak)) PWM_EventC_CallBack(void)
{
    // Add Application code here
}

void PWM_EventC_Tasks(void)
{
    if(IFS10bits.PEVTCIF)
    {
     
        // PWM EventC callback function 
        PWM_EventC_CallBack();
	
        // clear the PWM EventC interrupt flag
        IFS10bits.PEVTCIF = 0;
    }
}
void __attribute__ ((weak)) PWM_EventD_CallBack(void)
{
    // Add Application code here
}

void PWM_EventD_Tasks(void)
{
    if(IFS10bits.PEVTDIF)
    {
     
        // PWM EventD callback function 
        PWM_EventD_CallBack();
	
        // clear the PWM EventD interrupt flag
        IFS10bits.PEVTDIF = 0;
    }
}
void __attribute__ ((weak)) PWM_EventE_CallBack(void)
{
    // Add Application code here
}

void PWM_EventE_Tasks(void)
{
    if(IFS10bits.PEVTEIF)
    {
     
        // PWM EventE callback function 
        PWM_EventE_CallBack();
	
        // clear the PWM EventE interrupt flag
        IFS10bits.PEVTEIF = 0;
    }
}
void __attribute__ ((weak)) PWM_EventF_CallBack(void)
{
    // Add Application code here
}

void PWM_EventF_Tasks(void)
{
    if(IFS10bits.PEVTFIF)
    {
     
        // PWM EventF callback function 
        PWM_EventF_CallBack();
	
        // clear the PWM EventF interrupt flag
        IFS10bits.PEVTFIF = 0;
    }
}
