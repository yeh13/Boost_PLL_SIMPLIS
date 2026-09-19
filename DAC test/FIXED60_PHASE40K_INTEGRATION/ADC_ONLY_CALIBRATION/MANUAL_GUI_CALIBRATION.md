# Generic ADC-only：手動 GUI 校準（目前唯一執行流程）

狀態：WAITING FOR MANUAL GUI RUN — NOT VERIFIED。此次僅更新文件，沒有執行模擬。

## 已確認的測試平台問題

依使用者提供的實際 log，SIMPLIS Error 1071 位於 SIMPLIS_Data/native_adc_template_source.deck：ADC instance node list 與 subcircuit external nodes 不一致。這是 harness 結構錯誤，不是 ADC scaling 結果。

先前以 Prop NUMBITS 12 改值但保留舊 4-bit symbol 接腳，再抽取 generated model 的流程撤回。native_adc_template_source.*、native_adc_generated.net、point_*.net/deck、所有舊 .sxscr 只留作診斷，不再執行或修補。缺少 vector 是後續錯誤，不是量化測量。

不要使用舊 schematic、複製 ADC subcircuit、手動修 pin count，或直接編輯 NUMBITS property。由元件正常 GUI parameter dialog 讓 SIMPLIS 重建 symbol/model。

## 1. 建立空白 schematic、放置 native ADC

1. File > New，建立 schematic，確認目前 simulator mode 為 **SIMPLIS**。
2. Save As 新檔 ADC_ONLY_GUI.sxsch，放在此 ADC_ONLY_CALIBRATION 目錄；不要覆寫舊檔。
3. 若右側 Parts Selector 未顯示，用 **View > Show Part Selector**。
4. 在 Parts Selector 展開 **Digital Functions > A to D / D to A**，選 **Analog-to-Digital Converter**，按 Place。不要選 SystemDesigner ADC 或 Adjustable Voltage Reference ADC。
5. 開啟這顆 ADC 的正常參數 dialog，將 Number of Bits 設成 12，按 OK 讓 SIMPLIS 自動重建 symbol。確認現在有 D0…D11；以 GUI 產生的 pin 名為準，不以舊 netlist 的 node 編號接線。

[SIMPLIS 8.3 ADC 官方位置及動態產生方式](https://help.simetrix.co.uk/8.3/simplis/dp_a2d/topics/adc.htm)

## 2. ADC parameter dialog：第一組 candidate

| Field / property | 填入值 |
|---|---|
| Code / CODE | UNSIGNED |
| Number of Bits / NUMBITS | 12 |
| Input Range / RANGE | 2.2021505376344086 |
| Input Offset / OFFSET | 0 |
| Trigger / TRIG_COND | 0_TO_1 / Rising |
| Sample Delay / SAMPLE_DELAY | 1p |
| Convert Time / CONVERT_TIME | 50p |
| Data Ready Delay / DATA_READY_DELAY | 10p |
| Minimum Clock Width / MIN_CLK | 10p |
| Enable Delay / ENABLE_DELAY | 15p |
| Initial Condition / IC | 2048 |
| Initial Condition of Data Ready | NOT_READY |
| Initial Condition of Overflow | NONE |
| Input Resistance / RIN | 10Meg |
| Output Resistance / ROUT | 10 |
| Threshold / TH | 2.5 |
| Hysteresis / HYSTWD | 1 |
| Output Low Voltage / VOL | 0 |
| Output High Voltage / VOH | 5 |
| Ground Reference | 使用 ADC 的 RTN，接共同 ground |

Resistance、threshold、logic levels 在 Interface 頁面（依正常 dialog 顯示）。
若 GUI 不接受某欄位，保留畫面/log，先不要用 raw property 或 netlist 繞過。

RANGE/OFFSET 尚未驗證。OFFSET 始終保持 0，不先套半個 LSB 修正。
IC=2048 是初始輸出，**不是 conversion 結果**；第一筆用 -1V，預期轉換後從2048變成188，才能看出 ADC 確實工作。

## 3. 接腳與 ground

| Pin | 接法 |
|---|---|
| IN | calibration DC source 正端 |
| RTN | 共同 analog ground / node 0 |
| CLK（或 clock 三角形 pin） | 唯一 native Digital Pulse Source 的 normal OUT |
| EN | 固定 logic high，建議另一顆 +5V DC source 正端；其負端接共同 ground |
| D0…D11 | 各拉一小段線接 probe；不要接 ground，不要彼此短接 |
| DR | 拉線接 probe，確認 conversion 完成 |
| POFL、NOFL | 建議接 probe；不觀察時可不接 |
| clock 的 inverted output | 不啟用；若仍顯示，可不接 |

IN、CLK、EN、RTN 不可浮接。Ground 是所有 analog DC sources 的共同回路，不是額外獨立 DGND 電位。

本測試明確選用 **帶 Ground Ref 的 Digital Pulse Source**，Ground Ref 接同一 node 0。ADC 接類比輸入，RTN 必須接地。
純 digital-to-digital 連接一般可以省略 clock 的 Ground Ref，但本手動測試固定接地便於一致觀察。

[Ground Ref 官方規則](https://help.simetrix.co.uk/9.2/simplis/dp_00_master/topics/he_when_is_ground_ref_required.htm)

## 4. Clock source

Parts Selector：**Digital Functions > Sources > Digital Pulse Source**。

| Field | 設定 |
|---|---|
| Period | 25u |
| Width / High width | 12.5u |
| Delay | 1u |
| Output Low / VOL | 0 |
| Output High / VOH | 5 |
| Output Resistance / ROUT | 10 |
| Inverted / Complementary Output | No |
| Ground Ref | Yes，接共同 ground |

只有這一顆 clock，沒有 DUT clock 或 delay buffer。
本次是 DC 校準，不需要 PLL/golden phase alignment。

[Digital Pulse Source 官方欄位與型別](https://help.simetrix.co.uk/8.2/simplis/dp_sources/topics/digitalpulsesource.htm)

## 5. DC source 與七個測點

選 **Place > Voltage Sources > Power Supply**，放一般固定電壓源。
正端接 ADC IN，負端接 ground。保持此極性，負電壓直接在 DC value 填負號。
不選 sine、AC analysis source、PWL 或 timer source。

另放一顆同類 DC source，value=5，正端接 EN，負端接 ground。

每次只改 calibration source 的 DC value，其他參數不變；每次都從頭跑 transient。

| DC voltage / V | MATLAB expected | SIMPLIS actual | 判定 |
|---|---:|---|---|
| -1.0 | 188 | 待量測 | 未判定 |
| -0.0002741935484 | 2047 | 待量測 | 未判定 |
| -0.0002634408602 | 2048 | 待量測 | 未判定 |
| 0 | 2048 | 待量測 | 未判定 |
| +0.0002634408602 | 2048 | 待量測 | 未判定 |
| +0.0002741935484 | 2049 | 待量測 | 未判定 |
| +1.0 | 3908 | 待量測 | 未判定 |

[DC source 官方放置方式](https://help.simetrix.co.uk/8.1/simplis/user_manual/topics/parts_circuitstimulus.htm)

## 6. Probes 與輸出解讀

用一般 voltage probe / schematic 的 Probe 功能，放在：
IN、CLK、EN、DR、D0…D11；建議加 POFL、NOFL。
第一輪各 bit 分開看，不必先建立 bus；沒有 V3 接 bus 的問題。

D0 是 LSB，D11 是 MSB。邏輯低/高設定為0/5V，settled bit 解讀為0或1。
decimal = D0 + 2*D1 + 4*D2 + … + 2048*D11。
不是把各 bit 的電壓直接加起來，也不是把12條線直接短接。

若 GUI 提供 bus value 顯示，選 unsigned decimal，先核對 bit ordering：
2048 = binary 100000000000（只有 D11=1）。
188 = hex 0BC；2047=7FF；2048=800；2049=801；3908=F44。
負電壓不代表輸出是 signed code，仍是 unsigned 0…4095。

## 7. Minimal transient 與手動執行

Simulator > Choose Analysis：
- 僅選 Transient；不選 POP、AC。
- Stop time = 60u。
- Start saving data at t = 0。
- Number of Plot Points = 1001。
- 勾選 Force New Analysis；每個 DC 點均從初始狀態重跑。
- 不套用之前的 final snapshot / back-annotated state。

保存新 schematic，使用正常 Run/F9 執行目前新 schematic。
不要執行 .sxscr、Run Netlist 或手動 .deck。讓 GUI 產生自己的 SIMPLIS_Data。

Clock 上升緣應在1us、26us、51us。
第一筆 data 更新的設定時間是1.000050us，DR ready 是1.000060us；這些目前只是預期時間，不是實測。
先在2us等穩定位置讀code；再用事件游標放大第一次conversion確認delay。
1001 plot points 的均勻間距不是50ps，不能用均勻點間的插值證明50ps timing；保留 simulator transition/event points。
DC 不變時第二、三次 conversion 的code可完全不跳；用DR與clock確認持續觸發。
如果code仍為IC=2048，不能直接判PASS，先確認EN、clock、DR和第一個-1V測點。

若Error1071重現，停下並提供新GUI schematic/symbol與log，不修generated node list。
七點全一致後才可稱這組scaling provisional PASS；全域round/tie規則仍不能只靠七點證明。
若半LSB附近不一致，保持OFFSET=0，先量transition bracket，再決定是否有證據支持第二組。

## 8. 排除無關模型與 script path audit

新的 blank schematic 只含上述DC sources、native ADC、clock、grounds、probes。
不得加入任何 .v、.va、.LOAD、.include 或舊 controller hierarchy。
兩支 *_tb.v、abandoned .va、fixed60 DUT 與舊 V3/bus schematic 全部不參與。
只打開舊檔的editor tab不等同電路載入；以目前新schematic與其run log為準。不要刪除舊檔。

已逐行檢查此目錄3支 .sxscr：
- generate_native_adc.sxscr：OpenSchem/Netlist 的完整路徑有雙引號，但 Prop NUMBITS 流程停用。
- run_point_00.sxscr：PreProcessNetlist/RunSIMPLIS/Show 的完整路徑有雙引號；整支停用。
- run_all_adc_only.sxscr：使用相對檔名，依賴working directory；整支停用。

Unknown command "E:\DAC" 代表呼叫入口的路徑被拆開；腳本內有引號不保證外部呼叫端亦正確。
本輪完全用GUI Run，不輸入裸的 E:\DAC test\... 指令，也不另提供未驗證的啟動腳本。
舊腳本保持原樣作診斷證據，未修改或重跑。

## 9. 本次停止點

本次只準備本文件；沒有建立ADC schematic、沒有執行SIMPLIS。
等待使用者手動建立新ADC-only schematic及提供截圖/log。
不接DUT，不建GRID_PLL_CONTROLLER，不改Verilog/MATLAB/golden/firmware，不做power stage，不宣稱ADC PASS。
