**ADC／PLL 檔案盤點與實際訊號路徑確認 — 2026-09-19**

本次只做檔案、程式路徑、暫存器與既有結果審查；沒有修改 firmware、schematic、Verilog、MATLAB、模擬參數或既有結果，也沒有重新執行模擬。本目錄只新增盤點文件。所稱「現行 firmware」是指定目錄中、由專案 Makefile 引用的 C 原始碼；尚無證據證明它就是板上目前燒錄的 binary。

**先讀結論**

| 要確認的項目 | 本次判定 |
|---|---|
| PWM carrier | 暫存器推導的標稱值為 **40 kHz**。PG1 使用 500 MHz master clock，不使用 PCLKCON 的 ÷16 clock-divider 分支。 |
| AN0、AN1 ADC sample／conversion interrupt rate | **5 kHz**：PG1 TRIGA 每 carrier 一次，再經 ADTR1PS=7，即硬體 ÷8。 |
| SOGI／PLL update rate | **625 Hz**：AN1 callback 又每 8 次才執行一次。程式內 FS_HZ=5000 是數值設計常數，與暫存器所排程的實際速率不同。 |
| 現行 PLL | `adc1.c:1211` 的 adaptive SOGI／Park／PI callback，Q30 coefficients、Q12 states、degree-Q21 phase；不是舊 Stage-3D，也不是 locked-fixed-60 candidate。 |
| 40 kHz PLL phase delivery | **現行 C 沒有**。40 kHz PWM callback 更新的是獨立 open-loop phase；PLL 相位與參考值在慢速 AN1 分支更新後保持。 |
| 現行 current controller | AN0 的 Boost 2P2Z，Q15、A={30010,2668}、B={26000,-32000,9640}；Buck 只有 feedforward。以目前硬體觸發設定，AN0 控制率也是 5 kHz。 |
| 可證明已校準成功的 ADC-only SIMPLIS model | **目前未能確認**。有 native SIMPLIS 的短暫執行資料，但存在 RTN 浮接及 code 不符合既定校準點的問題。不能把「模擬有產生資料」當成「ADC 正確」。 |
| 最適合沿用的 native ADC 起點 | `ADC_ONLY_CALIBRATION/native_adc_template_source.sxsch` 的現存 12-bit native ADC 結構，僅作待修復／待校準的基底。不是直接可宣告 PASS 的模型。 |
| 主 schematic | `DAC teat.sxsch` 是 **SIMetrix** 的 ADC＋Fixed60 candidate 接線；其 ADC clock 浮接，不能作為已成功閉合 ADC→PLL 的證據。 |
| 啟動／SSR／soft-start | 指定 active C 中未找到完整流程；舊報告描述另一版本，不能搬入並宣稱 firmware-equivalent。 |

上述速率是依原始碼與元件寄存器定義推導的標稱排程值，假設時鐘正常且 ISR 無漏服務；不是示波器實測。FRC 誤差、ISR execution time、interrupt overrun 及板上 image 身分仍未量測。

**1. 盤點範圍與版本證據**

兩個指定目錄均遞迴掃描，包括隱藏檔。開始新增報告前：`F:\DAC test` 有 164 個檔案；`mcc_generated_files` 有 33 個。另唯讀追查父專案 `main.c`、`parameter.h`、`nbproject/Makefile-default.mk` 與既有 dist/map，以確認實際 build 引用及是否存在其他 ISR override。

- [完整檔案清單、大小、修改時間及 SHA256](file_inventory.csv)：197 筆。
- [兩個 ZIP 的內部檔案清單與 SHA256](archive_inventory.csv)：36 個非目錄項目；未解壓或覆寫任何內容。
- 沒有找到適用的 `AGENTS.md`。
- 範圍內只有 **2 個 `.sxsch`**；沒有獨立 `.sxcmp`、`.sxprj`、`.sxproj` 或已完成的 switching power-stage schematic。兩個 ZIP 也沒有另外一份 ADC schematic。
- 既有 CSV／SVG 的用途、欄位、摘要與對應文件已檢查；不把舊曲線當成此次執行結果。原始 waveform／binary 檔保留，未重新跑模擬覆蓋。

`adc1.c`、`pwm.c`、本目錄 `parameter.h` 的 SHA256 與 `FIXED60_PHASE40K_INTEGRATION/RESULTS/FIRMWARE_SHA256.csv` 完全相同，雖然舊 manifest 路徑使用 E:。這證明檔案內容相同，不能單憑路徑或時間戳推斷新舊。

| 檔案 | SHA256 |
|---|---|
| adc1.c | D51A6F9652382B3538D77FDE841034193F60A9CC0BBD7F4A872F408D072998BF |
| pwm.c | 4312D96C589FAB3B533D02553BF7985D392D023479602B61008068E2A8C7B482 |
| parameter.h | 019E68650F8DBA5737A9649E3DE8093C5C51681572DA4F1BC8447B8DC627282F |

父專案 Makefile 的 `SOURCEFILES` 明確包含本目錄 adc1/pwm/pll_state/system/interrupt_manager，target 為 dsPIC33CK256MP506。`main()` 只有 `SYSTEM_Initialize()` 與空 while loop。沒有找到另一個 strong AN1 callback／ISR 或後續 handler 更換。父層另一份 `parameter.h` 只有舊 extern 宣告；`adc1.c`、`pwm.c` 的同目錄 include 使用本目錄那份。

父層 dist 有 2026-05 的 ELF／HEX／MAP；本次沒有反組譯核對其全部機器碼或讀取 MCU，因此不宣稱已證實部署版本。報告以目前來源及 call graph 為準。

**2. `F:\DAC test` 重要檔案與可用性**

下表路徑以 `F:\DAC test` 為根；`I/` 表示 `FIXED60_PHASE40K_INTEGRATION/`，`A/` 表示 `I/ADC_ONLY_CALIBRATION/`。

| 檔案／群組 | 內容與證據 | 本次分類 |
|---|---|---|
| `DAC teat.sxsch` | 模擬控制文字為 `.simulator SIMETRIX`、`.tran 1`；啟用 U5 Fixed60 phase40k、U6 ADC_12；舊 inverter_controller 元件 disabled。 | 現存主編輯檔；非 native SIMPLIS ADC-only。 |
| `design.net`、`design.out` | U6 為 SIMetrix `AD_Converter`；輸入 range=3.3、offset=1.65；60 Hz ±1 V sine；40 kHz source 只接 U5 clock。out 有 1 s transient 完成統計。 | 有 engine 完成證據，無 ADC 正確取樣／PLL lock 證據。 |
| `vsx_root.v/.vvp/.v.log` | simulator 生成 HDL bench／編譯產物；目前內容涉及 Fixed60 candidate。 | 可追查連線／編譯，不能當來源模型或數值 PASS。 |
| `A/native_adc_template_source.sxsch` | 現在是 native 12-bit ADC，19 pins；RANGE=4096/1860、OFFSET=0；40 kHz pulse，delay=50 ns；`.tran 2u`。 | 最接近可沿用的 native ADC 圖，但目前接線不合格。 |
| `A/SIMPLIS_Data/native_adc_template_source.net/.deck/.lst` | deck 的 X$U1 與 subckt 均為 19 external nodes；RTN 對應 top node 21，沒有接 ground。 | 最新生成內容已不能套用舊「4-bit pin mismatch」結論。 |
| `A/SIMPLIS_Data/*.t0/.t2/.tc/.init/.dbg/.GrpName` | `.t0` 記載結束時間 2e-6；`.t2` 有 1003 data points；`.init` ADC 終值 2048；group=`simplis_tran3`。 | 證明存在短 transient 執行資料；不證明預期 ±1 V→188/3908。 |
| `A/native_adc_generated.net` | ADC template 生成來源及模型抽取產物。 | generator provenance，非驗證完成的 circuit。 |
| `A/point_00.net` 至 `point_06.net`、`point_00.deck` | 7 個 DC 校準 candidate；12-bit unsigned、range=2.2021505376344086、offset=0、25 µs clock、60 µs transient。 | 待量測測試平台。point_00 的 RTN 是 0，與後來 schematic deck 的浮接版本不同。 |
| `A/generate_native_adc.sxscr`、`run_point_00.sxscr`、`run_all_adc_only.sxscr` | model generation、preprocess、RunSIMPLIS、Show/export；部分硬編碼 E: 路徑。 | 歷史 scripts；本次未執行。 |
| `A/test_points_expected_and_status.csv` | 七筆 `SIMPLIS_actual` 全空、全部 `NOT_MEASURED`。 | 預期值表，沒有量測 PASS。 |
| `A/point_00_raw.txt`、`simulation_errors.txt` | 零位元組。 | 不是 waveform，也不是「沒有錯誤」的證明。 |
| `A/README.md`、`MANUAL_GUI_CALIBRATION.md` | 舊 Error 1071、改以 GUI calibration 的歷史說明。 | 文件部分已落後於現存 12-bit schematic/deck，需與實檔分開判讀。 |
| `I/VERILOG/fixed60_pll_5k_candidate.v` | 40 kHz clock→÷8；接近現行數值 SOGI／PI，但另有 `hold_60hz` 策略。 | 可重用數值結構，不是完全等效的現行 PLL。 |
| `I/VERILOG/fixed60_pll_phase40k_candidate.v` | 同上，加 degree-Q24 40 kHz delivery、8-tick quotient/remainder correction、pre/post anchor。 | 現有 phase delivery 候選；現行 MCU C 沒有這段行為。 |
| `I/VERILOG/*_tb.v`、`phase_delivery_semantics_reference.py` | full candidate／phase delivery 驗證程式與算術參考。 | 測試資源；log 未提供完整 parity PASS。 |
| `I/VERILOG/*.v.log`、`*.vvp` | candidate log 只有兩個 time steps、零 thread schedule；兩個 TB log 只有 `Compiling VVP ...`。 | 不能解讀成已完成 controller transient 驗證。 |
| `I/VERILOG/fixed60_pll_phase40k_simplis_analog.va` | 使用 `timer(0,1/FS_ADC)` 產生 `round(2048+1860*V)`；雖有 clk pin，取樣並非受它控制。 | 另一個 ADC 行為 wrapper；不適合作為 PWM trigger 等效的 native ADC 基底。 |
| `I/MATLAB/stage17_model_60hz.m`、`firmware_config.m` | int64 transcription，提供 `baseline` 與 `locked_fixed_60` 兩種策略。 | **baseline 是目前最接近 active callback 的既有 reference 起點**；仍需 C width／排程 parity。 |
| `I/MATLAB/sin_table.csv`、`vref_table.csv` | 逐項比對現行 C 的 360／334 entries。 | 完全相同，可沿用，不重算 sine table。 |
| `I/MATLAB/phase_delivery_8tick.m`、`sine_q15_ticks.m`、`check_delivery_vectors.m` | 既有 5k→40k 補間／預測與算術檢查。 | phase delivery candidate reference。 |
| `I/MATLAB/run_fixed60_phase40k.m`、`plot_fixed60_integration.m`、`reference_metrics.m`、`export_fixed60_phase40k_short_golden.m` | 測試與輸出管線；exporter 明確選 `locked_fixed_60`。 | 支援既有 candidate，非現行 firmware baseline 的 oracle。 |
| `I/MATLAB/RESULTS/golden_fixed60_phase40k_short.csv` | 79,993 rows、19 columns、0.000175～1.999975 s；最後 hold/phase_ok/locked=1；SHA 與舊文件一致。 | 真實存在的 MATLAB golden 檔，**不是 SIMPLIS waveform**。 |
| `inverter_controller.v` | ÷16、2.5 kHz SOGI-only；center=1986；沒有 PLL/current/PWM。 | 舊模型，參數不適用。 |
| `pll_lock_test.va`、`pll_stage3d_active_test.va` | 40k÷16、2.5 kHz；real arithmetic 與不同 PI／validity／phase-init 架構。 | 舊 feasibility/reference，不能拿來替代現行 fixed-point PLL。 |
| `dac_test_source.v`、`bus_to_voltage.va`、`bus_to_voltage-main.c/.info/.mak` | 固定 signed code DAC transport test、16-bit signed bus→類比觀察轉換與生成 C。 | 顯示／介面工具，並非 ADC 校準或 current control。 |
| `PLL_*.md`、`README_PLL_FEASIBILITY.md` | 包含舊 2.5 kHz／Stage-3D 程式稽核、振幅、phase compensation sweep 與 robustness。 | 歷史設計脈絡，必須核對來源後使用。 |
| `CURRENT_REFERENCE_*.md` 與 interpolation plots | 舊 ÷16 causal interpolation、reference polarity／相位分析。 | 不是現行 C 的 40 kHz PLL delivery 實作。 |
| `CURRENT_POLARITY_STARTUP_CONTRACT_AUDIT.md` 與 outputs | 舊啟動釋放、gate edges、signed frame 檢查。 | 其 SSR／soft-start／gate contract 不存在於目前 active C。 |
| `FULL_CONTROLLER_*.md`、各 STAGE1／STAGE2 outputs | dry-run 及 provisional averaged plant 結果；半載、滿載、transition、FFT 的 CSV／SVG。 | 只能證明該舊模型的結果，不能證明現行 switching plant 閉迴路。 |
| `I/RESULTS/*`、README／INTEGRATION_GUIDE／SIMPLIS_CONTROLLER_SETUP | 舊 inventory、來源 SHA、介面規格與未完成項目。 | SHA 可追溯；其中描述 schematic／版本的文字有過時內容。 |
| `FIXED60_PHASE40K_INTEGRATION.zip`、`I/MATLAB.zip` | 分別有 23／13 個檔案項目，不含額外 ADC schematic／ADC 結果。 | 備份快照，不按 ZIP 名稱視為最新版本。 |

舊 `PLL_ACTIVE_PATH_AUDIT.md` 的來源是 `d:\葉同隆\...\adc1.c`，描述 ÷16／Stage-3D。舊 full-controller 結果使用 40 kHz current execution、574/594/554 hysteresis、+3.55° reference offset、每 tick +16 Q15 soft-start；現行 C 無這些 active 邏輯，因此舊 PASS 不可移植為本次 PASS。

**3. ADC-only 成功版本判定：內容比檔名重要**

`native_adc_template_source.sxsch` 目前的 U1 symbol 已有 D0～D11、POFL、NOFL、DR、IN、CLK、EN、RTN，共 19 pins。生成 deck 的呼叫也有 19 nodes，與 `.subckt` 一致。這已不同於舊文件記載的 4-bit symbol 改 NUMBITS 導致 Error 1071 的版本。

但本次追到以下實際連線：

```text
U1 D0..D11 = 4,5,7,8,9,10,11,12,14,15,16,17
U1 POFL/NOFL/DR = 18/19/20
U1 IN/CLK/EN/RTN = 2/3/6/21
X$V1 的內部 V1: 2 1 SIN VOFFSET=-1 APEAK=0
外部 X$V1: 2 0 ...  → top V(0)-V(2)=-1，故 V(2)=+1 V
RTN top node 21 只接 U1，沒有接 node 0
```

因此，symbol 顯示 `-1` 不代表 ADC 實際收到 -1 V；這份 source 因 pin 極性而使 IN 對地為 +1 V。ADC 量到的是 IN−RTN，RTN 浮接，不能以 IN 對地值代替 ADC 差動輸入。最終 `.init` 為 code=2048，而按既定測試契約 +1 V 應為3908、-1 V 應為188。這個 2048 與浮接 RTN 被拉向 IN 的情況相符；未讀出完整數位事件序列，不將推測當成波形量測。

此外，CLK=25 µs period，但 transient 只有2 µs、delay=50 ns，最多包含第一個上升緣，不能驗證連續40 kHz取樣。EN 接的是 pulse 的反相輸出，在1～1.5 µs禁能，不是新校準說明要求的固定 high。舊4-bit probe位置也未隨12-bit pins全面更新：原標為 POFL/NOFL/DR 的 nodes 11/12/14，現在實際是 D6/D7/D8。

主 `DAC teat.sxsch` 的問題不同：`design.net:18` 產生 `V4_P` 40 kHz source，U5 clk 接這個 net；`design.net:44` 的 ADC clock 卻叫 `U6_Clock`，全 netlist 沒有 source／其他元件連上它。模擬完成不表示 ADC 真的每25 µs轉換。文件中預定的延迟buffer也尚未出現在實際netlist。

**判定：保留上述兩條證據鏈，但目前沒有一份可直接標記為「ADC transfer＋sample timing 已驗證成功」的模型。** 最適合 native SIMPLIS 整合的現有元件是 native schematic 中的12-bit ADC；先在副本確認RTN、source polarity、EN、clock及完整bus，再決定是否作正式基底。七點 `point_*.net` 可用作校準規格參考，不能當作已有測量證據。

**4. firmware 重要檔案與 active path**

| 檔案／位置 | 作用 |
|---|---|
| `adc1.c:49` | current ADC中心、Iref、limits與coefficients。 |
| `adc1.c:167` | PLL數值設計rate／decimation、phase units、PI、lock、SOGI banks。 |
| `adc1.c:448` | floor shift、Q30／Q15 signed rounding；`:505` sine table空間線性內插；`:657` adaptive SOGI coefficient選擇。 |
| `adc1.c:778` | ADC registers、handler註冊、dedicated cores與trigger mapping。 |
| `adc1.c:996`、`:1190` | AN0 current callback及ADCBUF0 ISR入口。 |
| `adc1.c:1211`、`:3192` | **active** AN1 PLL callback及ADCBUF1 ISR入口。 |
| `adc1.c:1665` | `ADC1_channel_AN1_CallBack_OldUnused`，沒有呼叫點；後方還有大量註解掉的歷史實作。 |
| `adc1.h` | AN0 dedicated Core0、AN1 dedicated Core1；ADC API／handler宣告，沒有另一套PLL演算法。 |
| `pwm.c:68` | PWM clock、period、TRIGA、postscaler、輸出模式、deadtime、interrupt enable。 |
| `pwm.c:301`、`:375`、`:495` | 334點半週表、60Hz open-loop NCO、PWM callback、PLL／open-loop reference選擇與Buck/Boost feedforward。 |
| `pwm.h`、`pwm_module_features.h` | PWM API，不能因為存在fault API就認定power protection已啟用。 |
| `parameter.h:98` | current calibration宣告；`:108` PLL_CONTROL_ENABLE=1；`:115` GRID_TIE_REQUIRE_PLL_LOCK=0。 |
| `pll_state.c/.h` | `pll_locked`、`i_ref_count`、`pll_vref_count`、`pll_sync_phase`等共享變數。float pll_theta/pll_integ不是active SOGI/PI state；PLL_Init/PLL_Step只有宣告。 |
| `clock.c:49` | FRC、主PLL、aux PLL時鐘設定。 |
| `interrupt_manager.c:51` | AN0 priority7、PWM1 priority3、AN1/PWM7 priority1。 |
| `system.c:132` | pin→clock→interrupt→ADC→PWM→global interrupt初始化。 |
| `pin_manager.c/.h` | analog pin enable及GPIO初始值；沒有SSR sequencing。 |
| `traps.c`、reset/system/watchdog相關檔 | 通用MCU fault/reset機制；未找到grid/current protection state machine。 |

本目錄5個MATLAB檔也不能由檔名認定等效：`pll_sogi_mcu_exact_sim.m`、`pll_mcu_lock_diagnostic.m` 仍有 Iref=458及不同gain/deadband；`pll_fixed60_debug_step_sim.m` 有track/lock Kp shift=6/10及額外平滑；`pll_sogi_sim.m`、`pll_sogi_rebuild_sim.m` 是候選探索／重建。這些不是目前C的參數來源。

**5. 實際ADC、PWM、PLL timing推導**

| 設定 | 原始碼數值 | 解讀 |
|---|---|---|
| Auxiliary clock | ACLKCON1=0x8101、APLLFBD1=125、APLLDIV1=0x21 | 標稱8MHz FRC，pre÷1、×125、post÷2÷1 → AFPLLO=500MHz。 |
| PWM master select | PCLKCON=0x33 | MCLKSEL=3選AFPLLO；DIVSEL=3提供另一條÷16分支。 |
| Generator clock/mode | PG1/2/7CONL=0x8008 | ON=1、CLKSEL=1直接master、HREN=0、independent edge；**不選÷16分支**。 |
| PWM period | PG1/2/7PER=0x30D3=12499 | 12500個2ns ticks →25µs→40kHz。 |
| ADC trigger compare | PG1TRIGA=10625 | carrier內約21.25µs、85%位置；不是PWM EOC取樣。 |
| ADC trigger enable/postscale | PG1EVTL=0x0108後再寫ADTR1PS=7 | 最終相關值0x3908；只開TRIGA→Trigger1，每8個compare輸出一次。 |
| Trigger offset／PWM IRQ | PG1EVTH=0 | ADTR1OFS=0；IEVTSEL=EOC。PWM IRQ cadence與ADC postscale不同。 |
| ADC channel trigger | ADTRIG0L=0x0404 | AN0與AN1均選PWM1 Trigger1。 |
| Input/core/format | ADCON4H=0；ADMOD0L=0；ADCORE0H/1H=0x0300；ADCON1H=0x60 | 專用AN0/AN1、single-ended unsigned、12-bit integer。 |
| Aperture設定 | ADCON4L=0；ADCORE0L/1L=0 | SAMC0EN/1EN=0；觸發後停止track並進入conversion，未啟用額外可編程延遲取樣。 |
| Interrupt enable | ADIEL=3；AN0/AN1IE=1；ADEIEL=0、ADEIEH=0 | 一般conversion-complete interrupt，沒有early interrupt路徑。 |
| PLL software divider | pll_decim_cnt++；<8 return；第8次reset | 只保留每第8筆ADC值，不是8點平均，也沒有先做anti-alias FIR。 |

```text
F_PWM            = 500,000,000 / (12,499 + 1) = 40,000 Hz
F_TRIGA_COMPARE  = F_PWM                       = 40,000 Hz
F_ADC_AN0_AN1    = F_PWM / (7 + 1)             =  5,000 Hz
T_ADC           = 200 µs
F_SOGI_PLL      = 5,000 / 8                    =    625 Hz
T_PLL           = 1.6 ms
F_CURRENT_LOOP  = F_ADC_AN0                    =  5,000 Hz
```

register解碼已對照目標器件家族的 [Microchip資料手冊](https://ww1.microchip.com/downloads/en/DeviceDoc/dsPIC33CK256MP508-Family-Data-Sheet-DS70005349H.pdf)（Registers 12-1、12-12、12-17、12-18、13-2、13-7、13-12、13-27）。時間零點／第一筆觸發還取決於PWM enable與postscaler初態，不能只從平均頻率替ADC第一筆timestamp下定論。

`ADC_ISR_HZ=40000`、`FS_HZ=5000` 不會設定硬體。它們只參與coefficients、phase step、frequency display等數值。以nominal step=9059696計算，程式假設5kHz時對應59.9999958Hz；若實際每秒只積分625次，相位速度僅**7.49999947Hz**。45～75Hz的step限制在實際625Hz cadence下僅對應5.625～9.375Hz。因此這份來源不能以「debug frequency接近60」證明已跟上60Hz輸入。

`dbg_pll_freq_x10` 使用design FS_HZ計算，且整數截斷使nominal step顯示599（59.9Hz）；`SOGI_LoadCoeffFromStepQ` 也因此從59.9Hz選係數，而非直接固定60Hz bank。這兩點都必須保留才是數值等效。

AN1 priority1可能被AN0 priority7與PWM1 priority3延後；本次未取得ISR耗時或overrun量測。`dbg_an1_isr_counter`與`dbg_an1_rate_count`在每筆callback增加，但`AN1_RATE_DUTY_DEBUG_ENABLE=0`，現行程式沒有啟用其自動Hz輸出。ADC aperture與conversion-complete／ISR入口也不是同一時間；1ps／50ps模擬delay不能視為MCU latency。

**6. 實際PLL訊號路徑**

```text
外部grid voltage與類比前端（電壓分壓／偏壓比例未提供）
  → AN1 / Dedicated Core1
  → PG1TRIGA @ carrier 85% → hardware ÷8 → 5 ksample/s
  → ADCBUF1，12-bit unsigned count
  → _ADCAN1Interrupt → registered ADC1_channel_AN1_CallBack
  → software ÷8，保留第8筆 → 625 updates/s
  → offset estimator（初值2048，Q12 IIR）
  → adcVal - rounded_offset → centered count
  → 100-update peak-to-peak/2 amplitude validity
  → invalid：清SOGI/PLL/lock/phase/ref state並return
  → adaptive SOGI Q30係數 × Q12 count state
  → va、vb
  → detector theta = pre-update phase +4°
  → interpolated Q15 sin/cos
  → Park vq = round_Q15(va*cos + vb*sin)
  → normalize by max(|va|,|vb|)+min(|va|,|vb|)/2，最低100 counts
  → raw_error = trunc(vq*1000/mag)，clamp±1000
  → error LPF /64 → slow LPF /32（Q8 state）
  → phase_ok hysteresis
  → PI deadband、integrator、gain selection、trim clamp
  → 45～75 design-Hz step limits、frequency deadband、slew limit
  → degree-Q21 phase accumulator / modulo360°
  → lock hysteresis
  → post-update sine×925 → i_ref_count
  → sine符號 → pll_sync_phase
  → phase×668取最近table index → VrefTable[0..333] → pll_vref_count
```

offset在decimation之後才更新：`offset_q += trunc((adc*4096-offset_q)/4096)`，center=`(offset_q+2048)>>12`。grid scaling沒有轉成伏特或乘固定1860；只做去中心及count→Q12的×4096。1860是部分測試源的振幅，不是active AN1硬體校準gain。

**路徑的缺口**：active callback中沒有grid zero-cross detector、positive-zero-cross event、SSR或grid-ready state。`AN1_ZC_*`、59～61Hz ZC lock參數雖仍define，但其實作位於OldUnused／註解區。`pll_sync_phase`只是估計sine的正負，不等同獨立實測grid ZC。

PWM1 ISR每25µs的`open_loop_phase_acc_q`是獨立60Hz表格相位。`PLL_CONTROL_ENABLE=1 && pll_locked && dbg_pll_phase_ok`成立時，PWM callback改讀慢速`pll_vref_count`／`pll_sync_phase`；**沒有**把PLL step積分成40kHz theta。`PLL_SinInterp`只是在兩個角度table entries之間插值，沒有提高相位更新頻率。

**7. active PLL數值參數與算術契約**

| 項目 | 現行值／行為 |
|---|---|
| 數值design Fs | 5000Hz；實際排程625Hz。 |
| Phase units | 360 table entries、512 subdivisions、STEP_Q=12；因此1°=2^21 raw，整圈754974720。 |
| Nominal/min/max step | 9059696 / 6794772 / 11324620 degree-Q21 raw per PLL update。 |
| Grid offset | 初值2048；Q12 state，estimator ÷4096，每PLL update才執行。 |
| Input validity | 100-update window，(max−min)/2 ≥300 counts；actual window160ms，design window20ms。 |
| SOGI arithmetic | coefficients Q30，state int32 Q12 counts，multiply/accumulate int64，signed nearest rounding；**沒有額外state saturation**。 |
| Sin/cos | 360項int16 Q15表＋角度fraction線性內插；表有個別+16383/−16384不對稱，不宜重新產生。 |
| Phase compensation | detector+4°；output0°。 |
| Error scaling/limits | sign=+1、bias=0、scale1000；mag最低100 Q12 counts；raw clamp±1000。 |
| Error filters | fast `e += (raw-e)>>6`；slow Q8 state `+= ((e<<8)-slow_q8)>>5`，取signed rounded integer。 |
| PI deadband | |e|≤5時PI error=0（外層|e|≤80）；積分保持，active branch無leak。 |
| PI fast gains | 未locked且|e|>80：Kp shift2，Ki shift13。 |
| PI lock/small-error gains | 其餘：Kp shift5，Ki shift15；不必已locked才切換此組。 |
| PI實際公式 | `I += e_PI`；`wd=floor(e_PI*4096/2^kp)+floor(I*4096/2^ki)`。不是可直接套用的連續時間Kp/Ki。 |
| 增益換算 | raw-step units：fast P=1024/e、I項=0.5/I；slow P=128/e、I項=0.125/I，仍需各自floor。 |
| Integrator clamp | ±4529848（error累積量）；不是Hz輸出。 |
| PI trim clamp | ±2264924 raw（design±15Hz）。 |
| Step target | NOM+wd，再夾min/max。 |
| Step deadband／slew | target−actual在±15099內設0；否則每update限制±3019。 |
| Locked on | valid且|fast error|≤170，on counter達20。 |
| Locked off | invalid在前段立刻reset；有效訊號下|fast error|>300、off counter達100。 |
| phase_ok on/off | |slow error|≤25達20次；>45達50次關閉。 |
| Hysteresis中間區 | 對應counter保留；不能擅改成「任何不符即歸零」的連續計數器。 |
| Actual counter時間 | on20=32ms、phase-off50=80ms、unlock100=160ms；不等於design的4/10/20ms。 |
| Active switches | PLL_FIXED_NCO_TEST=0；internal dual ADC test=0；PLL_CONTROL_ENABLE=1；GRID_TIE_REQUIRE_PLL_LOCK=0。 |
| 非active參數 | three-point helper、ZC feedforward/lock常數及LOCK_TRIM_LIMIT不能只因define存在就加入active model。 |

SOGI係數bank如下，依上一個step所換算的整數freq_x10於55↔60↔65間作signed nearest線性內插，區間外停在端點；不是無條件固定60Hz：

| Q30 coefficient | 55Hz bank | 60Hz bank | 65Hz bank |
|---|---:|---:|---:|
| a1 | -2042651804 | -2033145701 | -2023645686 |
| a2 | 973794573 | 965191209 | 956665874 |
| b0 | 49973625 | 54275307 | 58537975 |
| qb0 | 1726965 | 2046131 | 2390732 |
| qb1 | 3453929 | 4092262 | 4781464 |
| qb2 | 1726965 | 2046131 | 2390732 |

因nominal step→freq_x10截斷為599，第一次有效運算載入的a1/a2/b0/qb0/qb1/qb2實際為 **−2033335823、965363276、54189273、2039748、4079495、2039748**。單純拷貝60Hz bank作固定SOGI會有差異。

```text
va[n] = round_Q30(b0*xq[n] - b0*xq[n-2] - a1*va[n-1] - a2*va[n-2])
vb[n] = round_Q30(qb0*xq[n] + qb1*xq[n-1] + qb2*xq[n-2]
                  - a1*vb[n-1] - a2*vb[n-2])
```

`PLL_Shift64`對負數做floor；C signed division向零截斷；Q30/Q15 round為最近值、正負half tie遠離零；offset的delta用向零截斷。移植時必須分開保留，不能統一改成四捨五入。active C含int16/int32 narrowing，而candidate廣泛用int64；未做overflow parity前不能稱bit-exact。

**8. current loop路徑與參數**

```text
實際power-stage current → 類比sensor/bias（硬體比例尚待校準）
 → AN0/Core0 → 同PG1 Trigger1、5ksample/s → ADCBUF0
 → _ADCAN0Interrupt（priority7）→ current callback
 → 選PLL reference或open-loop table reference
 → vref>=574選Boost，否則Buck
 → mode改變時clear補償器history
 → Iref中心2048，正負由phase決定，zero band20
 → adc0_filt += (raw-adc0_filt)>>2
 → 正半週error=Iref-adc_filt；負半週error=adc_filt-Iref
 → |error|<3設0
 → Boost：2P2Z → correction夾±2500 → 加Boost_PWM → duty夾0..10625
 → Buck：Buck_PWM直接輸出、Boost=0、clear 2P2Z state
 → PG1DC=finallyDuty_buck、PG2DC=finallyDuty_boost
 → SOC register更新 → PWM輸出／driver／power stage → sensor
```

feedforward由PWM1 callback標稱40kHz計算；實際PG1DC/PG2DC由AN0標稱5kHz寫入，因此不能把feedforward callback率當成current controller或duty寫入率。PG1/PG2目前各自self-trigger；不能在模型中無證據假設已設master/slave同步。兩條ISR之間reference與base-duty的採用順序，後續整合應明確保留，而非同一理想tick內一次原子更新所有量。

| 項目 | 現行值／解讀 |
|---|---|
| Current ADC | unsigned raw、中心2048；filter初始2048、IIR1/4。 |
| Counts/A | header宣告460；active controller全程使用counts，沒有把A轉count的運算。實際sensor的V/A與polarity仍缺板級證據。 |
| Current target | 真正active是`IREF_PEAK_COUNTS=925`；header的`I_RMS_TARGET_A=0.5`及其約325-count peak宏沒有被current path引用。 |
| Iref生成 | PLL路徑`(sin_q15*925)>>15`；fallback從Vref用Q15 gain8004換成peak counts。 |
| Iref zero band | PLL signed嚴格−20<Iref<20設0；fallback幅度<20設0。 |
| Boost coefficients | A0=30010、A1=2668，B0=26000、B1=−32000、B2=9640；Q15，A項在程式中為加號。 |
| 補償公式 | `temp=(26000*e[n]-32000*e[n-1]+9640*e[n-2]+30010*u[n-1]+2668*u[n-2])>>15`。 |
| State／saturation | temp夾±2500；duty夾0..10625；存回history的是最終duty−本次feedforward，不一定等於temp。mode transition及Buck每次清state。 |
| Buck controller | A={−53039,20271}、B={6650,−11592,5006}雖存在，active path未用；Buck是open-loop/feedforward。 |
| Voltage command | VrefTable peak3787；Vdc=574，註解對應48V，但不是動態Vdc ADC feedback。 |
| 切換boundary | vref≥574→Boost；<574→Buck；**沒有594/554遲滯**。 |
| Boost feedforward | floor／C整數除法：12500*(vref−574)/(vref+574)，base夾0..12500；最終current輸出另夾0..10625（85%）。 |
| Buck feedforward | 6250±trunc(vref*6250/574)，phase1用+、phase0用−；夾0..12500。Boost區PG1用12500或0決定極性。 |
| PWM duty limits | Buck0..12500、Boost0..10625；period register=12499，控制器full-scale用12500。 |
| PWM output模式 | PG1IOCONH=0x1C為Independent；PG2IOCONH=0x0C為Complementary。不能沿用舊「兩者皆complementary」gate驗證。 |
| Deadtime設定 | PG1/2 DTH/DTL=100（500MHz換算200ns）；設定值不等於已驗證所有driver／gate實際deadtime。 |
| 未鎖定時 | GRID_TIE_REQUIRE_PLL_LOCK=0，因此fallback到60Hz open-loop reference，不是強制關PWM等待PLL。 |

若暫時把460counts/A視為有效校準，925peak counts約為1.42190Arms；其0.5倍約462.5peak counts、0.71095Arms。這只是**current command半幅**換算，不等於已知道硬體額定半載，也不是header的0.5Arms。真正半載測試前需確認額定功率／電流與current sensor比例，先保留既有925count full-command基準。

active C沒有SSR enable／next-ZC PWM release／reference soft-start流程。PG1/2 ON在初始化就設1，初始DC=0；fault/current-limit PCI與ADC comparator registers為0，未見power protection state machine。現有duty／PI saturation與MCU traps不能替代這套啟動保護邏輯。

**9. firmware與SIMPLIS／SIMetrix ADC對照**

| 項目 | 現行firmware | native ADC candidate | 主SIMetrix U6 |
|---|---|---|---|
| 輸入意義 | AN1 pin voltage，外部grid前端比例未知 | 測試normalized grid，±1V預定映射±1860counts | 直接餵±1V sine |
| Range／Vref | ADCON3L REFSEL=0，AVDD/AVSS；不能由C證實實板AVDD精確3.3V | RANGE=2.2021505376344086、OFFSET=0；沒有physical Vref pin | input_range=3.3、input_offset=1.65 |
| Bits／code | 12-bit single-ended unsigned integer | 12-bit UNSIGNED | 12 output bits，default offset-binary |
| Offset | raw初始中心2048，slow adaptive offset removal | converter range midpoint0，不是firmware count offset；初始IC0、final init2048 | converter offset1.65V，不是adc count2048 |
| Scaling | centered count×4096供SOGI；不含1860counts/V輸入gain | 設計意圖1860counts/V，但七點未量測 | 跨3.3V的12-bit範圍，與1860counts/V契約不同 |
| Aperture | PWM TRIGA約21.25µs處、硬體÷8；ADC硬體停止取樣後轉換 | rising edge後sample delay1ps；現存clock首緣50ns | rising edge sample，但clock目前未接source |
| Sample rate | 標稱5kHz | 設定40kHz，但只存2µs資料，未驗證重複取樣 | 不能因旁邊有40kHz源就認定ADC有40kHz取樣 |
| Conversion／ready | 實體ADC/core clock與ISR latency；非zero-time | convert50ps、ready再10ps；只是模型設定 | convert1µs、data-valid再100ns |
| Reference ground | AVSS | 現存RTN浮接node21，必須釐清 | 對地analog input |
| Decoded count證據 | 未取得本次硬體trace | final init2048；七DC測點actual空白 | out無raw count數值trace，只有engine完成 |
| PLL輸入排程 | ADC5k→軟體÷8→625Hz | 候選設計預想ADC40k→÷8→5kHz | U5自有40k clock，ADC資料來源無clock |

AVDD/AVSS reference與12-bit mode依 [Microchip器件資料手冊](https://ww1.microchip.com/downloads/en/DeviceDoc/dsPIC33CK256MP508-Family-Data-Sheet-DS70005349H.pdf)解碼。若只作理想3.3V ADC近似，LSB約3.3/4096=0.805664mV；1860counts peak相當於約1.498535V的ADC pin振幅，再加1.65V偏壓。這是規格換算，不是量到的板級前端。

native ADC的OFFSET是range midpoint，應以IN相對RTN的電壓理解；其symbol/model由正常parameter編輯生成。[SIMPLIS 8.3 Generic ADC說明](https://help.simetrix.co.uk/8.3/simplis/dp_a2d/topics/adc.htm)。SIMetrix `AD_Converter`在clock上升緣取樣，convert_time後更新data；與SIMPLIS native元件是兩種不同device。[SIMetrix 8.3 ADC說明](https://help.simetrix.co.uk/8.3/simetrix/simulator_reference/topics/digitalmixedsignaldevicereference_analog_digitalconverter.htm)。

目前測試契約為`clip(round(2048+1860*Vin),0,4095)`；native模型實際bin-boundary／tie／saturation仍未量測。不能擅把offset改半LSB後就宣稱與MCU或MATLAB完全一致。12-bit規格相同也不足以證明各自轉換邊界相同。

raw count應按`sum(Di*2^i), i=0..11`解碼，確認D0是LSB、D11是MSB，不能把bit電壓直接相加。主圖部分64-bit signed debug probe只設定[32:0]，`extrapolated_ref` probe未設signed；未修正顯示前不應用這些probe判斷相位／負半週是否錯誤。

**10. 現有候選PLL與現行firmware的差異**

`fixed60_pll_5k_candidate.v`與`fixed60_pll_phase40k_candidate.v`共用許多active C係數／rounding／filters／lock thresholds，但加入`hold_60hz`：locked、phase_ok及slow error條件滿足20次後，把step強制設為NOM；離開hold另有50次計數。現行C沒有這個狀態，PLL_FIXED_NCO_TEST也為0。因此它們不能直接當作firmware-equivalent PLL。

phase40k candidate另增加degree-Q24 delivery、pre-update anchor、每8tick分配circular error的quotient/remainder；現行C沒有這些states。候選`pll_phase_5k`是pre-update anchor，`pll_phase_post_5k`是post-update結果；現行`i_ref_count`使用post-update theta。不能混用兩個時間語義，亦不能把golden位移一格掩蓋排程差異。

既有MATLAB `stage17_model_60hz(adc,'baseline')`最接近目前active數值路徑，table與coeff來源已核對；但是目前保存golden使用`'locked_fixed_60'`。保留既有golden作candidate reference，後續baseline比較另存新結果，不覆寫原檔、不宣稱目前已通過C／HDL parity。

`golden_fixed60_phase40k_short.csv` SHA256：`B6F2026667216AC88CF82C162AC72F983B0671998E02EEB04AA5DF55E27FD566`。

**11. 下一步建議檔案與執行順序（尚未修改）**

| 優先 | 檔案／目標 | 具體工作 |
|---|---|---|
| 1 | `A/native_adc_template_source.sxsch` 的保留副本，例如 `ADC_ONLY_FIRMWARE_BASELINE.sxsch` | 沿用現有native12-bit元件，確認RTN接地、source極性、固定EN、clock與所有12bits／DR／overflow probes。先單獨重現DC校準，不加PLL。 |
| 2 | ADC calibration結果另存CSV、run log、generated deck及waveform | 測−1、0、+1V及half-LSB兩側、rails；記錄decoded raw、sample edge、data-ready，而不只看IC。保留舊結果。 |
| 3 | `I/MATLAB/stage17_model_60hz.m` baseline與現行C | 用既有baseline分支建立來源對照與新baseline trace；保留各種rounding/narrowing/lock-counter行為。先不改coeff或PI。 |
| 4 | 既有 `fixed60_pll_5k_candidate.v` 的明確baseline副本 | 以active C為準，分離candidate-only HOLD及40k delivery；恢復C widths與可觀察states。沿用既有算法，不重建另一套理想浮點PLL。 |
| 5 | 基於ADC副本的 `ADC_PLL_FIRMWARE_BASELINE.sxsch` | 接analog grid→ADC→count→active SOGI／PLL，重現硬體÷8與軟體÷8。只測PLL；本次不建立檔案。 |
| 6 | `fixed60_pll_phase40k_candidate.v`／`phase_delivery_8tick.m` 保留 | 另作已存在的40k phase-delivery比較候選。不能偷偷加進「現行firmware等效」baseline。 |
| 7 | `SIMPLIS_CONTROLLER_SETUP.md`、ADC README／status表 | 更新已查證版本差異、rate、RTN／clock缺口、probe width、E:舊路徑；不重寫generated net/deck作來源修正。 |

**目前不建議動`adc1.c`、`pwm.c`、`parameter.h`或任何MCU控制邏輯。** 若目標是ADC40k／PLL5k，必須先釐清是否另有已驗證的板上image或未提供的來源。對此F:來源直接改ADTR1PS=0，會同時改變AN0 current rate和AN1 rate，屬於實質firmware改動，不在本階段執行。

60Hz PLL-only後續應分清兩個測試：其一重現當前暫存器排程（5kADC／625PLL），其二僅作清楚標示的design-timing reference（40kADC／5kPLL）。前者不能因不鎖定就暗中替換後者。現存fixed60＋40k delivery也另列candidate，不與baseline混稱。

以下probe是下一階段規格，本次沒有建立模型或probe：

| 要求probe | 現行來源／單位與注意事項 |
|---|---|
| analog grid input | 外部原始grid source；V或明確normalized unit。 |
| ADC input voltage | ADC IN−RTN／MCU AN1−AVSS；分開顯示front-end前後。 |
| ADC raw count | 12-bit unsigned bus，每次conversion；不能只用`dbg_an1_raw`，因它在decimation後才更新。 |
| ADC centered value | `v_in_count`／`dbg_grid_v`，signed count；另probe `grid_offset_q`。 |
| SOGI va／vb | `sogi_va`／`sogi_vb`，int32 Q12 counts；顯示counts時除4096。 |
| vq | `vq_q` Q12；debug int16 trunc值需另標。 |
| PLL PI output | `w_delta_q`，degree-Q21 raw/update；同時記integrator、target、actual step。 |
| PLL frequency | 分別記design計算值、按actual625Hz換算值、theta差分所得實際頻率。 |
| PLL theta anchor | pre-update `pll_idx*2^21+theta_acc_q`；另外保存post-update及detector+4°。 |
| 40k theta | active C無對應state；baseline只能顯示40k觀察tick上的held theta。既有candidate才有真正每tick delivery。 |
| zero-cross | active C無grid ZC state；測試平台可建立只觀察、不回授的input ZC，以及估計sine正向過零供對照。 |
| phase_ok／locked | `dbg_pll_phase_ok`／`pll_locked`；加on/off counters、valid signal避免假鎖誤判。 |

PLL-only通過後才做current ADC→Iref→error→Boost 2P2Z／Buck feedforward→duty→**實際plant current回授**。先確認「半載」的額定值及sensor比例，再依半載steady、半載→滿載、滿載steady、Buck/Boost transition執行。舊Stage2 plant用placeholder dynamics及reference決定operating-point current，不能取代真實switching power stage回授。

最後才整合Power on→PLL locking→grid ready→positive ZC→SSR→next ZC→PWM enable→Iref soft-start→current regulation。這是要求的後續目標；目前C並未實作完整順序，仍須找到與先前驗證結果對應的真正來源，不能從舊文件臆造為現行實作。

**本階段已完成與尚未證實的界線**：已完成目錄／archive盤點、active call graph、register timing推導、PLL／current參數與ADC模型差異、table／來源hash核對；尚未確認板上image、板級Vref／sensor比例、ADC校準成功版本、ISR實測rate、C/HDL parity、60Hz PLL-only及任何power-stage閉迴路PASS。
