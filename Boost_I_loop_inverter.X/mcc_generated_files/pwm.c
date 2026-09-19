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
#include "parameter.h"    // ?? extern ????

/**
 Section: Driver Interface Function Definitions
*/

// PWM Default PWM Generator Interrupt Handler
static void (*PWM_Generator1InterruptHandler)(void) = NULL;
static void (*PWM_Generator7InterruptHandler)(void) = NULL;
/* ===== ???PWM / PLL ???? ===== */

static int32_t PWM_VrefGet(void);
static uint8_t PWM_PhaseGet(void);
void BOOST_DUTY(int32_t vref_cmd);
void BUCK_DUTY(int32_t vref_cmd, uint8_t phase_cmd);


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
    // PENL enabled; DTCMPSEL PCI Sync Logic; PMOD Independent; POLL Active-high; PENH enabled; CAPSRC Software; POLH Active-high; 
    PG1IOCONH = 0x1C;
    // PENL enabled; DTCMPSEL PCI Sync Logic; PMOD Complementary; POLL Active-high; PENH enabled; CAPSRC Software; POLH Active-high; 
    
    // PENL enabled; DTCMPSEL PCI Sync Logic; PMOD Complementary; POLL Active-high; PENH enabled; CAPSRC Software; POLH Active-high; 
    PG2IOCONH = 0x0C;
    // PENL enabled; DTCMPSEL PCI Sync Logic; PMOD Independent; POLL Active-high; PENH enabled; CAPSRC Software; POLH Active-high; 
    PG7IOCONH = 0x1C;
    // UPDTRG Duty Cycle; ADTR1PS 1:8; PGTRGSEL EOC event; ADTR1EN3 disabled; ADTR1EN1 enabled; ADTR1EN2 disabled;
    PG1EVTL = 0x108;
    PG1EVTLbits.ADTR1PS = 7;
    // UPDTRG Duty Cycle; ADTR1PS 1:1; PGTRGSEL EOC event; ADTR1EN3 disabled; ADTR1EN1 disabled; ADTR1EN2 disabled; 
    PG2EVTL = 0x08;
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
    // DC 20%; PG7 is used as a scope/debug output.
    PG7DC = 0x09C3;
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
    // PER 12499; 
    PG7PER = 0x30D3;
    // TRIGA 10625; 
    PG1TRIGA = 10625;
    // TRIGA 0; 
    PG2TRIGA = 0x00;
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
    // HREN disabled; MODSEL Independent Edge; TRGCNT 1; CLKSEL Master clock; ON enabled; 
    PG7CONL = 0x8008;
}

/* =========================================================
   PWM control global variables
   ---------------------------------------------------------
   pwm.c only generates:
   1. sine table index i
   2. positive / negative half-cycle phase
   3. Buck_PWM / Boost_PWM base duty

   Actual PG1DC / PG2DC output is written in ADC AN0 current loop.
   ========================================================= */

unsigned short phase = 0;
unsigned short i = 0, j = 1, k = 0;

const int16_t Vdc = 574;    // 48V input ADC equivalent

#define OPEN_LOOP_PWM_ISR_HZ      40000UL
#define OPEN_LOOP_TABLE_LEN       334UL
#define OPEN_LOOP_PHASE_Q         16UL
#define OPEN_LOOP_FULL_CYCLE_Q    ((uint32_t)(2UL * OPEN_LOOP_TABLE_LEN * (1UL << OPEN_LOOP_PHASE_Q)))
#define OPEN_LOOP_STEP_Q          ((uint32_t)(((uint64_t)OPEN_LOOP_OUTPUT_FREQ_HZ * OPEN_LOOP_FULL_CYCLE_Q + (OPEN_LOOP_PWM_ISR_HZ / 2UL)) / OPEN_LOOP_PWM_ISR_HZ))
#define OPEN_LOOP_FREQ_MIN_HZ     50L
#define OPEN_LOOP_FREQ_MAX_HZ     75L

static uint32_t open_loop_phase_acc_q = 0;

volatile power_mode_t power_mode = MODE_BUCK;

volatile int32_t Buck_PWM = 0;
volatile int32_t Boost_PWM = 0;

volatile int16_t finallyDuty_buck = 0;
volatile int16_t finallyDuty_boost = 0;

void BOOST_DUTY(int32_t vref_cmd);
void BUCK_DUTY(int32_t vref_cmd, uint8_t phase_cmd);

/* =========================================================
   334-point half-cycle voltage reference table
   ========================================================= */

const int16_t VrefTable[334] = {
   0,   36,   71,  107,  142,  178,  214,  249,  285,  320,
 356,  391,  427,  462,  497,  533,  568,  603,  638,  673,
 708,  743,  778,  813,  848,  882,  917,  951,  986, 1020,
 1055,1089,1123,1157,1191,1224,1258,1292,1325,1358,
 1391,1425,1457,1490,1523,1556,1588,1620,1652,1684,
 1716,1748,1779,1811,1842,1873,1904,1935,1965,1995,
 2026,2056,2085,2115,2145,2174,2203,2232,2260,2289,
 2317,2345,2373,2401,2428,2455,2482,2509,2536,2562,
 2588,2614,2640,2665,2690,2715,2740,2765,2789,2813,
 2836,2860,2883,2906,2929,2951,2974,2995,3017,3039,
 3060,3080,3101,3121,3141,3161,3181,3200,3219,3237,
 3256,3274,3292,3309,3326,3343,3360,3376,3392,3408,
 3423,3438,3453,3467,3482,3495,3509,3522,3535,3548,
 3560,3572,3584,3595,3606,3617,3627,3637,3647,3657,
 3666,3674,3683,3691,3699,3706,3713,3720,3727,3733,
 3739,3744,3749,3754,3759,3763,3767,3770,3774,3776,
 3779,3781,3783,3784,3786,3786,3787,3787,3787,3786,
 3786,3784,3783,3781,3779,3776,3774,3770,3767,3763,
 3759,3754,3749,3744,3739,3733,3727,3720,3713,3706,
 3699,3691,3683,3674,3666,3657,3647,3637,3627,3617,
 3606,3595,3584,3572,3560,3548,3535,3522,3509,3495,
 3482,3467,3453,3438,3423,3408,3392,3376,3360,3343,
 3326,3309,3292,3274,3256,3237,3219,3200,3181,3161,
 3141,3121,3101,3080,3060,3039,3017,2995,2974,2951,
 2929,2906,2883,2860,2836,2813,2789,2765,2740,2715,
 2690,2665,2640,2614,2588,2562,2536,2509,2482,2455,
 2428,2401,2373,2345,2317,2289,2260,2232,2203,2174,
 2145,2115,2085,2056,2026,1995,1965,1935,1904,1873,
 1842,1811,1779,1748,1716,1684,1652,1620,1588,1556,
 1523,1490,1457,1425,1391,1358,1325,1292,1258,1224,
 1191,1157,1123,1089,1055,1020, 986, 951, 917, 882,
  848, 813, 778, 743, 708, 673, 638, 603, 568, 533,
  497, 462, 427, 391, 356, 320, 285, 249, 214, 178,
  142, 107,  71,  36
};

/* =========================================================
   PWM1 interrupt callback
   ---------------------------------------------------------
   Only calculate base duty.
   Do NOT write PG1DC / PG2DC here.
   ========================================================= */

void PWM_Generator1_CallBack(void)
{
    int32_t vref_cmd;
    uint8_t phase_cmd;

#if (GRID_TIE_REQUIRE_PLL_LOCK != 0)
    if ((PLL_CONTROL_ENABLE != 0) &&
        ((pll_locked == 0) || (dbg_pll_phase_ok == 0)))
    {
        Buck_PWM = 0;
        Boost_PWM = 0;
        finallyDuty_buck = 0;
        finallyDuty_boost = 0;
        PG1DC = 0;
        PG2DC = 0;
        return;
    }
#endif

#if (AN1_RATE_DUTY_DEBUG_ENABLE != 0)
    {
        static uint16_t rate_window_pwm_cnt = 0;
        uint16_t an1_count_snapshot;
        uint32_t duty_x10;

        /*
           4000 PWM1 interrupts at 40kHz = 0.1s.
           AN1 ISR Hz = AN1 count in this window * 10.
           PG7 duty maps 0Hz..5kHz -> 0.0%..100.0%.
        */
        rate_window_pwm_cnt++;

        if (rate_window_pwm_cnt >= 4000)
        {
            rate_window_pwm_cnt = 0;
            an1_count_snapshot = dbg_an1_rate_count;
            dbg_an1_rate_count = 0;

            dbg_an1_rate_hz = (uint16_t)(an1_count_snapshot * 10U);
            duty_x10 = ((uint32_t)dbg_an1_rate_hz * 1000UL) / 5000UL;

            if (duty_x10 > 1000UL)
                duty_x10 = 1000UL;

            dbg_an1_rate_duty_x10 = (uint16_t)duty_x10;
            PG7DC = (uint16_t)(((uint32_t)PG7PER * duty_x10) / 1000UL);
        }
    }
#endif

#if 0
    {
        int64_t pg7_cmd64;

        pg7_cmd64 = ((int64_t)((int32_t)OPEN_LOOP_OUTPUT_FREQ_HZ - OPEN_LOOP_FREQ_MIN_HZ) * PG7PER) /
                    (OPEN_LOOP_FREQ_MAX_HZ - OPEN_LOOP_FREQ_MIN_HZ);

        if (pg7_cmd64 > PG7PER)
            pg7_cmd64 = PG7PER;
        else if (pg7_cmd64 < 0)
            pg7_cmd64 = 0;

        PG7DC = (uint16_t)pg7_cmd64;
    }
#endif

    open_loop_phase_acc_q += OPEN_LOOP_STEP_Q;
    if (open_loop_phase_acc_q >= OPEN_LOOP_FULL_CYCLE_Q)
    {
        open_loop_phase_acc_q -= OPEN_LOOP_FULL_CYCLE_Q;
    }

    {
        uint16_t table_phase = (uint16_t)(open_loop_phase_acc_q >> OPEN_LOOP_PHASE_Q);

        if (table_phase >= OPEN_LOOP_TABLE_LEN)
        {
            phase = 1;
            i = (unsigned short)(table_phase - OPEN_LOOP_TABLE_LEN);
        }
        else
        {
            phase = 0;
            i = (unsigned short)table_phase;
        }

        if (i >= OPEN_LOOP_TABLE_LEN)
        {
            i = (unsigned short)(OPEN_LOOP_TABLE_LEN - 1UL);
        }
    }

    vref_cmd = PWM_VrefGet();
    phase_cmd = PWM_PhaseGet();

    if (vref_cmd >= Vdc)
    {
        power_mode = MODE_BOOST;

        BOOST_DUTY(vref_cmd);

        if (phase_cmd == 1)
            Buck_PWM = 12500;
        else
            Buck_PWM = 0;
    }
    else
    {
        power_mode = MODE_BUCK;

        BUCK_DUTY(vref_cmd, phase_cmd);

        Boost_PWM = 0;
    }
}

/* =========================================================
   Base duty calculation
   ========================================================= */

static int32_t PWM_VrefGet(void)
{
    if ((PLL_CONTROL_ENABLE != 0) &&
        (pll_locked != 0) &&
        (dbg_pll_phase_ok != 0))
        return pll_vref_count;

    return VrefTable[i];
}

static uint8_t PWM_PhaseGet(void)
{
    if ((PLL_CONTROL_ENABLE != 0) &&
        (pll_locked != 0) &&
        (dbg_pll_phase_ok != 0))
        return pll_sync_phase;

    return (uint8_t)phase;
}

void BOOST_DUTY(int32_t vref_cmd)
{
    Boost_PWM = vref_cmd - Vdc;
    Boost_PWM *= 12500;
    Boost_PWM /= vref_cmd + Vdc;

    if (Boost_PWM >= 12500)
        Boost_PWM = 12500;
    else if (Boost_PWM <= 0)
        Boost_PWM = 0;
}

void BUCK_DUTY(int32_t vref_cmd, uint8_t phase_cmd)
{
    Buck_PWM = vref_cmd;
    Buck_PWM *= 6250;
    Buck_PWM /= Vdc;

    if (phase_cmd == 1)
        Buck_PWM = 6250 + Buck_PWM;
    else
        Buck_PWM = 6250 - Buck_PWM;

    if (Buck_PWM >= 12500)
        Buck_PWM = 12500;
    else if (Buck_PWM <= 0)
        Buck_PWM = 0;
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
