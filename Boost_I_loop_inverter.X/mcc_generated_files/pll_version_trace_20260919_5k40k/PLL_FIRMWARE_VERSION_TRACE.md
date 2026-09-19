**5 kHz PLL／40 kHz phase firmware 版本追查報告**

查核日期：2026-09-19。範圍：F:\DAC test 與整個 F:\Boost_I_loop_inverter.X。

**結論：在這兩個目錄及全部 3 個 ZIP 中，未找到可證明「5 kHz PLL anchor + PLL-derived 40 kHz extrapolation，而且 C 與 MATLAB golden 完全一致」的 MCU C source。**

找到的最接近實作是 Fixed60 MATLAB／Verilog candidate。它確實包含 5 kHz anchor 與 40 kHz delivery，但沒有相對應、已驗證的 MCU C 版本；其 HOLD_60HZ 還會把 actual step 固定為 nominal。因此不能認定它已完成 59／60／61 Hz 的追頻驗證。

本次只讀取、搜尋、計算 hash、分析既有 CSV；沒有修改或執行 firmware、SIMPLIS、MATLAB、HDL，沒有重新產生 golden，沒有把 .v 重建為 C。僅新增本報告及本次盤點附件，未覆寫舊報告或任何既有檔案。

**1. 搜尋覆蓋及證據層級**

- 递迴含隱藏檔共 327 個既有檔案：DAC test 164、整個 MCU 專案 163；其中 4 個是上一輪 audit_20260919 的產物，保留但不作獨立歷史證據。排除這 4 個後為 323 個。
- 已盤點所有目錄，不限檔名中的 backup、old、test、stage、integration、fixed60、phase40k、5k、pll、golden、copy、複製、日期等字串。
- 展開前以記憶體唯讀串流檢查全部 ZIP 的 67 個檔案 entry；沒有解壓覆寫，沒有發現 nested ZIP。
- 實體檔案中共 16 個 .c、18 個 .h。DAC 唯一 .c 是 SIMetrix 產生的 bus_to_voltage-main.c，不是 MCU PLL；MCU 目錄另有主程式、周邊驅動、OLED 字型與圖像資料，沒有第二套隱藏 PLL C。
- 對 .c/.h/.txt/.md/.m/.csv 及 .v/.va 搜尋名稱、流程、rate 數值與 golden 用語；另檢查 .mc3、Makefile、IDE 設定、.o.d、.map，以及 .elf/.o 可讀來源路徑／符號。
- 找到的備份壓縮檔僅下表 3 個。沒有另外的 old／backup／copy 日期版 C 目錄。
- 「原始碼直接證據」、「歷史文件描述」、「既有數值 trace」、「本次未找到」分開判定。檔案時間僅供排序，不能代替版本血緣或驗證證據。

| ZIP 完整路徑 | 檔案 entries | 內容判讀 |
|---|---:|---|
| F:\Boost_I_loop_inverter.X\closed_loop_inverter.zip | 31 | 2026-03-06 entry 時間的 C 專案；浮點 PLL，AN1 缺少實際週期觸發，非目標版本 |
| F:\DAC test\FIXED60_PHASE40K_INTEGRATION.zip | 23 | MATLAB、Verilog、文件與 hashes；沒有 MCU C。23 個 entry 均與目前對應實體檔 hash 相同 |
| F:\DAC test\FIXED60_PHASE40K_INTEGRATION\MATLAB.zip | 13 | MATLAB package，沒有 C、沒有保存的 golden CSV。12 個與實體檔一致；exporter 是較舊的不同版本 |

**2. 所有相關 candidate 與歷史線索**

路徑縮寫：
- P = F:\Boost_I_loop_inverter.X
- M = F:\Boost_I_loop_inverter.X\mcc_generated_files
- D = F:\DAC test
- I = F:\DAC test\FIXED60_PHASE40K_INTEGRATION
- ZIP!entry 表示壓縮檔內路徑，並非新解壓出的檔案。
- 「模型」rate 是輸入時間軸／模型排程，不能當成 MCU register 的實際 rate。

| Candidate／路徑 | ADC rate | PLL rate | 40k phase 來源 | SOGI | PI | phase_ok | locked | golden trace | 判定 |
|---|---|---|---|---|---|---|---|---|---|
| C1：M\adc1.c active + M\pwm.c | AN0/AN1 5 kHz | **625 Hz** | 獨立 open-loop NCO；鎖定時另可讀 held PLL reference | Q30 coeff/Q12 state，55/60/65 bank | shift PI | 有 | 有 | manifest 指向此 C，但無 C parity trace | 實體 active C；缺 5 kHz 實際時序與 PLL-derived 40k delivery |
| C2：同一 adc1.c 的 ADC1_channel_AN1_CallBack_OldUnused，line 1665 | 未被呼叫 | 未執行；若沿用現 register/counter 仍是 625 Hz | 無另一個 40k extrapolator | 固定點／診斷分支 | 有 | 有 | 有／診斷分支 | 無 | 歷史函式，不是另一份已驗證 firmware |
| C3：同一 adc1.c，line 2459 起的註解 callback | 無執行 | 無執行 | 無 | 註解保留 | 註解保留 | 不作有效旗標 | 不作有效旗標 | 無 | 無法作可建置版本或驗證版本 |
| C4：P\closed_loop_inverter.zip!closed_loop_inverter.X\mcc_generated_files\adc1.c + pwm.c | AN0 40 kHz；AN1 未配置週期 trigger | 初始化路徑沒有週期 PLL；TS 名義 40 kHz | PWM 直接讀 float pll_theta，沒有兩速率 extrapolation | float biquad | float Kp=150、Ki=18000 | 無 | pll_mode 幅值遲滯；獨立 pll_locked 只找到初始化 0 | 無 | 早期 C；雖 postscaler=0，仍不是目標 |
| H1：D\PLL_ACTIVE_PATH_AUDIT.md + pll_lock_test.va | 模型 40 kHz | 模型 2.5 kHz，÷16 | 非 5k→40k extrapolator | 舊 fixed-60 模型／real arithmetic | 舊 PLL PI | 模型有 | 模型有 | 59/60/61 Hz replay 表，無 C parity | 舊 callback 世代；引用 D: 的 C 不在提供目錄 |
| H2：D\PLL_STAGE3D_ACTIVE_FEASIBILITY.md + pll_stage3d_active_test.va | 文件／模型 40 kHz | 2.5 kHz，÷16 | Stage-4 Q32 mailbox；另有因果插值文件 | Q30/Q12 設計；VA real 表達 | fixed norm/Q20、I ÷16 | inline branch 不寫，維持 0 | inline branch 不寫，維持 0 | 60 Hz 數值 replay，非 C-MATLAB golden | 和 active callback／Fixed60 都不同代；完整 C 缺失 |
| H3：D\CURRENT_REFERENCE_INTERPOLATION_INTENT_AUDIT.md + CURRENT_REFERENCE_INTERPOLATION_PLOTS\interpolation_trace.csv | 文件 raw 40 kHz | anchor 2.5 kHz | **向上一個 mailbox target 的 16-tick 因果插值，末 tick snap** | 沿用 Stage-3D | 沿用 Stage-3D | 沿用舊 branch | 沿用舊 branch | 有平滑 trace，無對應 C source | PLL-derived smoothing，但不是 5k anchor 向前 extrapolation |
| H4：D\FULL_CONTROLLER_SIMULATION_PLAN.md、FULL_CONTROLLER_DRY_RUN_STAGE1*.md、FULL_CONTROLLER_STAGE2_SIMPLIFIED_PLANT.md | 文件 40 kHz | 沿用 2.5 kHz Stage-3D | Stage-4 插值／current-reference | 沿用 | 沿用 | 另用 readiness | 非 legacy locked 門控 | 有 startup/current replay CSV | 存在整合版本的歷史證據，完整 C 缺失 |
| M1：M\pll_sogi_sim.m、pll_sogi_rebuild_sim.m | 模擬設定 | 5 kHz 設計 | 沒有目標 40k delivery | 固定點近似／float 探索 | 有 | 依腳本 | 有模型判斷 | 未找到配套 C trace | 探索模型，非 C candidate |
| M2：M\pll_sogi_mcu_exact_sim.m、pll_mcu_lock_diagnostic.m | 假設 40 kHz | 假設 ÷8=5 kHz | 沒有目標 extrapolator | 舊固定點對應 | 舊參數／診斷分支 | 依腳本 | 有／診斷 modes | 無保存的 C 全欄位 parity 結果 | 名稱 exact 不等於已驗證的 C |
| M3：M\pll_fixed60_debug_step_sim.m | 模擬假設 | 5 kHz 設計 | 無目標 40k delivery | 55/60/65 banks | AFTER_SOFTPI，額外 smoothing | 有 | 有 | 無 C golden 對應 | 不同 PI 世代，不能代替 active 或 stage17 |
| M4：I\MATLAB\stage17_model_60hz.m + firmware_config.m | 輸入模型 40 kHz | 每 8 ADC calls＝5 kHz | 單獨檔案只產生 anchors | Q30/Q12，int64 模型 | active transcription；另有 locked_fixed_60 | 有 | 有 | 18-column 內部 output schema；沒有保存的 C 全欄位 trace | 最接近 active 數值演算法；不是 MCU C |
| M5：M4 + I\MATLAB\phase_delivery_8tick.m | 模型 40 kHz | 5 kHz | PLL pre-update anchor + actual step + 8-tick distributed correction | 同 M4 | 同 M4；golden 選 Fixed60 HOLD | 有 | 有 | **現存 79,993-row MATLAB golden** | 最接近目標的完整參考模型；未證明有等價 C |
| V1：I\VERILOG\fixed60_pll_5k_candidate.v | clk_40k | ÷8＝5 kHz | 無完整 40k phase delivery | 固定點 | shift PI + HOLD_60HZ | 有 | 有 | 無單獨完成 parity 結果 | PLL-only HDL candidate |
| V2：I\VERILOG\fixed60_pll_phase40k_candidate.v | clk_40k | ÷8＝5 kHz | **PLL-derived anchor/step，Q24 每 tick 推進及誤差分配** | 固定點 | shift PI + HOLD_60HZ | 有 | 有 | 對應 MATLAB golden；TB 註明 runtime comparison pending | 架構最接近；非已驗證 MCU C |
| B1：P\dist\default\debug／production 的 .elf/.map，及 build 的 .o | 未重新證實 | 未重新證實 | 未找到目標 mailbox/extrapolation 符號 | binary | binary | 有相關符號 | 有相關符號 | 無 source-bound golden | 歷史建置產物，不可當成找回完整 C 或 parity 證據 |
| X1：文件提及 earlier DCAC 5 kHz candidate、Stage 1.7 package | 文件稱 40k/5k | 文件稱 5k | earlier DCAC 有每 anchor snap；本套件另建 no-reset delivery | 未取得原包 | 未取得原包 | 未取得原包 | 未取得原包 | Stage1.7 Fixed60 PASS 是 user-reported | 只有來源描述，未找到 original package／C |

D\inverter_controller.v 另經內容排除：雖檔頭寫 40 kHz，實際 decimation=16，只有 2.5 kHz SOGI；不能因檔名「controller」將其當完整 PLL。dac_test_source.v／vsx_root.v 是 DAC stimulus／生成 bench；bus_to_voltage-main.c 是 Verilog-A 介面生成物。

**3. C 原始碼與 rate 的實際確認**

C1 active：

PWM 40,000 Hz → PG1EVTLbits.ADTR1PS=7 → PWM1 Trigger1 每 8 周期一次 → ADTRIG0L=0x0404 觸發 AN0/AN1 → AN1 callback 5,000 次/s → pll_decim_cnt 每 8 次才執行 → SOGI/Park/PI/NCO 實際 625 次/s。

證據位置：
- M\pwm.c:139–140：PG1EVTL=0x108 後又寫 ADTR1PS=7；不可只讀初始化整數或註解。
- M\adc1.c:167–169：ADC_ISR_HZ=40000、PLL_DECIM_N=8、FS_HZ=5000 是演算法設計常數。
- M\adc1.c:847：AN1 callback 註冊。
- M\adc1.c:874：ADTRIG0L=0x0404。
- M\adc1.c:1211、1260–1267：active callback 與 software ÷8。
- M\adc1.c:1540：PLL phase 在這個 decimated callback 更新。
- M\adc1.c:3192：_ADCAN1Interrupt 的 callback 路徑。
- M\pwm.c:307–314、441–459：40 kHz open_loop_phase_acc_q，使用固定 OPEN_LOOP_STEP_Q。
- M\pwm.c:495–512：locked && phase_ok 時選 PLL reference／polarity；只是取低速已發布值，不是以 PLL step 每 25 µs 推進 theta。

因此既不能將「PWM callback=40k」視為「PLL theta delivery=40k」，也不能將「FS_HZ=5000」視為實際 5k anchor。

C4 ZIP：

- PG1EVTL=0x108，沒有其後 ADTR1PS=7：postscaler 欄位為 0，即 1:1。
- 同樣 PG1PER=0x30D3、PG1CONL=0x8008、aux clock 設定，PWM/AN0 trigger 是 40 kHz。
- adc1.c:130 的 ADIEL=0x01：AN1 的 ADC channel interrupt enable 未開。
- adc1.c:236 的 ADTRIG0L=0x0004：AN0=PWM1 Trigger1，AN1=None。
- IEC5bits.ADCAN1IE=1 與 callback 註冊本身不能補上缺失的 trigger／ADIEL。
- main.c 只初始化後空迴圈；完整 ZIP C 搜尋未找到另一處啟動 AN1 週期採樣或直接呼叫此 PLL。
- TS=25 µs 與 float SOGI 係數只表示名義 40k 設計，沒有 software ÷8、沒有 5k anchor。
- pwm.c 直接讀 pll_theta，沒有 frequency-derived per-tick advance；pll_locked 的寫入只找到初始化 0，與 pll_mode 分離。

P\closed_loop_inverter.mc3 的 PG1EVTL/ADTR1PS setting 也保存 1:1，但它是 MCC 設定快照，沒有目標 PLL C body，而且不會覆蓋 active pwm.c 執行時寫入的 7。不能據此改判 active rate。

**4. Golden 反查：確實追到 C hash，但追到的是目前 active C**

可追溯鏈：

I\MATLAB\RESULTS\golden_fixed60_phase40k_short.csv
← I\MATLAB\export_fixed60_phase40k_short_golden.m
← stage17_model_60hz(adc, 'locked_fixed_60') + phase_delivery_8tick(...)
← firmware_config.m／sin_table.csv／vref_table.csv
← I\RESULTS\FIRMWARE_SHA256.csv 的 E:\Boost_I_loop_inverter.X\mcc_generated_files\adc1.c
＝現在 F: active adc1.c 的相同 bytes/hash。

最關鍵限制：
- stage17_model_60hz.m:1–4 明確稱「Independent int64 transcription」，並寫「Host C parity must be executed separately」。
- README.md:32–38 稱 stage17 等來自 earlier Stage 1.7 package，先前 Fixed60 PASS 為 user-reported；distributed-correction 是新 candidate。
- RESULTS\INVENTORY_CLASSIFICATION.md:19 指 earlier DCAC candidate 每個 anchor 會 snap；README 不能解讀成「找到了先前 no-reset C」。
- SIMPLIS_CONTROLLER_SETUP.md:14 記錄後續使用者 MATLAB Online Fixed60／synthetic delivery／integration／exporter PASS。這是 MATLAB 結果來源說明，沒有宣稱同一 C binary 全欄位 parity。
- fixed60_pll_phase40k_candidate_full_tb.v:88 仍寫 runtime comparison pending；現存 .v.log 不提供完整 parity PASS。
- 未找到 Stage 1.6、C vs MATLAB completely matched 的對應原始報告、host C harness、C output CSV、完整比較 diff 或其 source hash 綁定。
- golden CSV 是 delivery trace，並非 offset/valid/va/vb/vq/pi/vref/iref 的 C-versus-MATLAB 全欄位對照。stage17 的 18 欄 output schema 有 offset_q、valid、va_q12、vb_q12、raw_error、error_lpf、error_slow、pi_error、integrator、target_step、slew_delta、actual_step、phase_q、phase_ok、locked、iref、vref_idx、vref；現存 golden 未保存整份此輸出，也沒有獨立 raw vq 欄。
- manifest 表示某些檔案被凍結／保持不變，不能單獨當成「這份 C 實作了新增的 HOLD 與 delivery，且完成 parity」的證明。

**5. 既有 golden 的唯讀數值檢查**

沒有重新跑 exporter，直接讀現存 CSV：

| 項目 | 結果 |
|---|---:|
| rows | 79,993 |
| time | 0.000175～1.999975 s |
| sampling interval | 25 µs（文字浮點解析有約 1e-16 s 誤差） |
| 第一次 locked=1 | 0.097575 s |
| 第一次 phase_ok=1 | 0.543775 s |
| 第一次 hold_60hz=1 | 0.547575 s |
| 最後 1 秒的 theta_40k increment | 每次皆 9,059,696 degree-Q24 ticks |
| 最後 1 秒平均頻率 | 59.9999957614475 Hz |
| 最後 1 秒 40,000 intervals 中零相位增量 | 0 |
| 最後 1 秒 hold／phase_ok／locked 非 1 筆數 | 0 |

此資料支持「這份 MATLAB Fixed60 trace 有平滑 40k theta」，不支持「MCU C 已實作並通過」，也不支持 59/61 Hz 追頻。
degree-Q21 的 5k nominal step 和 degree-Q24 的 40k step 恰好同為 9059696，是 Q-format 放大 8 倍、時間間隔縮小 8 倍所致，不是 rate 相同。

**6. 59／60／61 Hz 證據分代**

D\PLL_PHASE_COMP_ROBUSTNESS.md 的 frequency sweep：

| 輸入 | 文件 PLL 結果 | 3 s lock | 所屬版本 |
|---:|---:|---|---|
| 59 Hz | 59.000069 Hz | Yes | 舊 pll_lock_test.va／2.5 kHz sampled replay |
| 60 Hz | 60.000458 Hz | Yes | 同上 |
| 61 Hz | 60.999898 Hz | Yes | 同上 |

這份文件固定 analysis-only PHASE_COMP_DEG=15.42、PLL_VALID_BYPASS_TEST=1；文件也揭露 validity update rate 和 firmware 不一致。phase/time 部分是對既有 60 Hz anchor 的分析後處理，不能全數稱為新 SIMPLIS 量測。

D\CURRENT_REFERENCE_INTERPOLATION_INTENT_AUDIT.md 與 interpolation_trace.csv 顯示較早 2.5k→40k 的 16-tick 插值已很平滑，平均約 0.539999490°/tick；但與同時間 ideal forward phase 仍有約 -8.099992° 固定延遲，且演算法最後 tick snap 到 target。它不是所找的 5k extrapolation。

Fixed60 package 明確只做 60.000 Hz stimulus；現存腳本與 golden 不提供目標 5k+40k 版本的 59／61 Hz 完整結果。HOLD_60HZ 會固定 step，即使有 PLL-derived acquisition，仍不能等同一般連續追頻 PLL。

**7. 版本時間線與血緣**

以下時間以本機 LastWriteTime／ZIP entry timestamp 為線索；來源與 SHA 優先於時間。

| 時間／世代 | 實體／文件證據 | 能確認與不能確認 |
|---|---|---|
| 2026-03-06（ZIP entry；ZIP 本身 03-16） | closed_loop_inverter.zip | 浮點 TS=25 µs PLL，AN1 trigger/ADIEL 不完整，沒有 5k/40k extrapolator |
| 2026-05-13～05-20 | M\pll_*.m | 5k 數值探索、診斷與 AFTER_SOFTPI；不是同一套 verified C |
| 2026-05-20 | debug .elf/.map | 來源字串 C:\Users\3920\Desktop\Boost_I_loop_inverter.X；含現目錄已缺的 delay.c、spi1.c、oled_debug.c。沒有找回這些原始 source |
| 2026-05-21 | active adc1.c/pwm.c；production .elf/.map | 現行 fixed-point callback，實際 5k ADC→625 PLL。production 來源字串 E:\Boost_I_loop_inverter.X |
| 2026-08-26 早期文件 | PLL_ACTIVE_PATH_AUDIT／舊 callback 模型 | 引用 d:\葉同隆\Boost_I_loop_inverter.X\mcc_generated_files\adc1.c，描述 2.5k；和目前 F: C 不同 |
| 2026-08-26 後續文件 | Stage-3D／Stage-4／current dry-run | 40k raw→÷16→2.5k、Q32 mailbox、16-tick causal interpolation、startup/current integration；完整來源 C 未隨資料搬入 |
| 日期不完整，09-10 文件回溯 | earlier DCAC／Stage 1.7 | 5k candidate、anchor snap／Fixed60 USER-REPORTED PASS。原包和 C 未找到；Stage 1.6 未找到 |
| 2026-09-10 14:33～15:01 | stage17、5k .v、phase_delivery_8tick.m、phase40k .v | 從 active 數值邏輯衍生 Fixed60 與新 distributed-correction 模型；無 MCU C 新版本 |
| 2026-09-10 15:37 | FIXED60_PHASE40K_INTEGRATION.zip | 保存上列 23 檔，相同 hash，不是另一個 firmware 世代 |
| 2026-09-10 16:28／16:32 | MATLAB.zip exporter／ZIP時間 | exporter 3007 bytes；和稍後實體版本不同 |
| 2026-09-10 16:57 | 實體 exporter 5153 bytes、testbenches | exporter 加入明確 diagnostic columns 與物理時間窗等處理；ZIP 內 exporter 不應混用 |
| 2026-09-10 17:25／18:50 | golden CSV／SIMPLIS_CONTROLLER_SETUP | 現存 authoritative MATLAB trace 與使用者 MATLAB PASS 記錄；不等於 C parity PASS |

這不是一條可證明的「同一 branch 一直向前演進」時間線：8 月文件引用 D: 的來源，9 月 package 卻以和 5 月 active 檔相同的 E: hash 為 baseline，明顯有不同來源樹並存。

**8. 和 active C 的差異**

沒有找到目標 C，因此不能提供「找到的目標 C vs active C」的真實 source diff。以下只比較現存模型與 C，避免假裝兩者都是 firmware：

| 項目 | 現行 active C | stage17／phase40k Fixed60 candidate |
|---|---|---|
| 實際 ADC scheduling | PWM postscaler ÷8＝5k | stimulus／clk_40k＝40k；没有 MCU registers |
| software decimation | ÷8 → 625 Hz | ÷8 → 5k |
| SOGI | Q30/Q12，55/60/65 三 bank | 同一組設計 coefficients／數值來源 |
| PI | fast shift 2/13；track shift 5/15；deadband、clamp、slew | 相近數值 transcription；需 C parity，不能只憑係數認定全等 |
| 額外 Fixed60 hold | active 沒有該 HOLD_60HZ 狀態機 | locked+phase_ok+stable entry 後 actual step 強制 nominal |
| phase anchor | callback post-update 產生 reference | delivery 取 pre-update theta，和 post-update phase 分開 |
| phase delivery | open-loop accumulator；PLL refs 可被 held 讀取 | degree-Q24，actual step 每 40k tick 推進，circular error 分 8 ticks quotient/remainder correction |
| 旗標時間尺度 | counter 每 625 Hz 更新 | 每 5k Hz 更新 |
| C 實作 | 有、hash 明確 | 未找到相對應 MCU C |
| 原始驗證 | 未找到所需 C-MATLAB parity bundle | 現存 MATLAB golden／使用者 MATLAB PASS 記錄 |

March ZIP C 則另差在 float SOGI、Kp=150/Ki=18000、ADC_CENTER=2050、名義 40k TS、AN1 trigger=None、不同 current PI、PWM 直接讀 float theta，不能當作「active C 只差 ADTR1PS」的候選。

**9. SSR／ZC／soft-start／current loop 整合版本**

找到歷史整合的文件與 replay CSV，沒有找到那份完整 C：

- Stage-3D／Stage-4 文件描述 DIAG_LOAD_STAGE_SELECT=DIAG_LOAD_STAGE_CURRENT_LOOP、DIAG_STAGE4_PHASE_CHAIN_ENABLE、Q32 mailbox。
- FULL_CONTROLLER_SIMULATION_PLAN.md:228 起：
  STATE_PLL_LOCKING → STATE_WAIT_GRID_VALID → STATE_WAIT_ZC_TO_COMMAND_SSR → STATE_WAIT_NEXT_ZC_TO_START_PWM → STATE_GRID_CURRENT_RUN，另有 STATE_FAULT。
- 文件稱 GRID_TEST_MODE_NO_SSR=1，因此存在 SSR 邏輯也不能說物理 SSR 已啟用／驗證。
- soft-start 每 40k tick +16 Q15 至 32768。
- 舊 current replay 使用 574 初始 boundary、594/554 hysteresis；active C 不是這套狀態機。
- Stage1 最終 dry-run／Stage2 simplified plant 有結果，但不等於目標 5k PLL C 的 integrated PASS，也不是完整 switching power-stage sign-off。

缺少上述函式及狀態機所屬的 adc1.c/pwm.c/parameter.h 與建置配置。不能從文件片段拼成新 C，也不能把這一代的測試套到 Fixed60 package。

**10. 使用者八項問題的明確回答**

1. 最接近「5k PLL + 40k PLL-derived extrapolation」的是 I\VERILOG\fixed60_pll_phase40k_candidate.v，及 I\MATLAB\stage17_model_60hz.m + phase_delivery_8tick.m。**不是 MCU C**，且目前的 golden 是 Fixed60 模式。
2. **沒有找到曾與 MATLAB golden 完全一致的那份 C source 及其 parity 證據。** 找到的 manifest C 等於現在 active C，不能證明目標版本存在於此目錄。
3. 因目標 C 未找到，**無法提供該目標 C 的 SHA256**。能提供的 active C、March ZIP C、closest HDL、MATLAB、golden hashes 列於下方，均明確標示身分。
4. 不能做不存在的目標 C diff；可確認 active 缺正確實際 5k anchor cadence、PLL-derived 40k delivery、Fixed60 HOLD／pre-anchor delivery 狀態，且它沒有舊 Stage-4 全啟動整合。
5. active：ADTR1PS=7、software ÷8、PLL=625 Hz。March ZIP：ADTR1PS=0，但 AN1 trigger=None/ADIEL1=0，無實際週期 PLL，沒有 ÷8。5k模型：40k input ÷8＝5k，沒有 MCU register 證據。
6. active 的 40k accumulator 是 open-loop；candidate delivery 是 PLL-derived（HOLD 時頻率 step 固定 nominal）；舊 Stage-4 是 PLL-derived causal interpolation，不是本次目標的 5k extrapolator。
7. 有 SSR/ZC/soft-start/current-loop 的舊整合文件、CSV；沒有找到相應完整可建置 C，也不是已確認的 5k 版本。
8. **目前没有符合條件的 MCU firmware reference 可直接指定。** 應把 active C 保留為「已知基線」，把 Fixed60 MATLAB／HDL＋golden 保留為「模型參考」，繼續尋找原 Stage1.6／Stage1.7／DCAC 的完整來源與比較包。不能先拿 closest HDL 冒充 firmware reference。

下一輪最具體的來源線索：
- d:\葉同隆\Boost_I_loop_inverter.X\mcc_generated_files\adc1.c（8 月文件引用；本次指定兩根目錄中缺失）。
- C:\Users\3920\Desktop\Boost_I_loop_inverter.X（debug ELF 的 source path，含缺失周邊 source）。
- E:\Boost_I_loop_inverter.X（production／manifest；其中 manifest adc1.c 已證實就是目前 F: 同 hash，單靠找到相同副本不會補上 extrapolator）。
- earlier Stage 1.7 package、earlier DCAC 5 kHz candidate 的原始 archive／host C harness／CSV comparator。
- 所需補件是一組同版本 C+headers+PWM/ADC registers+build defines+golden input/C output/comparison report，尤其是 40k ISR 的 anchor/step 消費與 phase delivery 實作。沒有找回前不自行補寫。

**11. SHA256 身分表**

| 身分 | 完整路徑 | SHA256 |
|---|---|---|
| 目標已驗證 5k/40k MCU C | 未找到 | 不可提供／不可用其他檔案代替 |
| active C，也是 Fixed60 manifest 所指 C | F:\Boost_I_loop_inverter.X\mcc_generated_files\adc1.c | D51A6F9652382B3538D77FDE841034193F60A9CC0BBD7F4A872F408D072998BF |
| active PWM | F:\Boost_I_loop_inverter.X\mcc_generated_files\pwm.c | 4312D96C589FAB3B533D02553BF7985D392D023479602B61008068E2A8C7B482 |
| active parameter | F:\Boost_I_loop_inverter.X\mcc_generated_files\parameter.h | 019E68650F8DBA5737A9649E3DE8093C5C51681572DA4F1BC8447B8DC627282F |
| March ZIP 內 C entry（不是 ZIP 外殼 hash） | F:\Boost_I_loop_inverter.X\closed_loop_inverter.zip!closed_loop_inverter.X/mcc_generated_files/adc1.c | 901A83ADBD1A5E03E65E99A991C1BC1D076A463F469B162BDC55A51647D50FCF |
| March ZIP 內 PWM entry | F:\Boost_I_loop_inverter.X\closed_loop_inverter.zip!closed_loop_inverter.X/mcc_generated_files/pwm.c | F9DA8F011B863E76C04B02600798CAD9F91175F73439B2585C54C5AEBC391159 |
| closest HDL candidate | F:\DAC test\FIXED60_PHASE40K_INTEGRATION\VERILOG\fixed60_pll_phase40k_candidate.v | A4BE11244FB417D0D769E72C96482A7B40CC004FF2E9203FB35901E839844CE6 |
| 5k HDL core | F:\DAC test\FIXED60_PHASE40K_INTEGRATION\VERILOG\fixed60_pll_5k_candidate.v | 9BD421F6A95609065698CB24AA1C920CB1742E4B38518DC6220F8DABEA29E44C |
| stage17 MATLAB | F:\DAC test\FIXED60_PHASE40K_INTEGRATION\MATLAB\stage17_model_60hz.m | EA6674FDB66394C3B62B3C3BA2A886D3C06ADAAF21ADBC6EB61858AD1C50DD76 |
| phase delivery MATLAB | F:\DAC test\FIXED60_PHASE40K_INTEGRATION\MATLAB\phase_delivery_8tick.m | 9CF7904F48C405569B2E25508D97FE4C5A4839A34D17C2BFB769CACD8CA473D0 |
| golden CSV | F:\DAC test\FIXED60_PHASE40K_INTEGRATION\MATLAB\RESULTS\golden_fixed60_phase40k_short.csv | B6F2026667216AC88CF82C162AC72F983B0671998E02EEB04AA5DF55E27FD566 |
| 目前實體 exporter | F:\DAC test\FIXED60_PHASE40K_INTEGRATION\MATLAB\export_fixed60_phase40k_short_golden.m | 395A931DBAD43394FE1946D2A8C2A1FE0A09BEAF2E2BFB3F1A2A550F205977C9 |
| MATLAB.zip 內舊 exporter | F:\DAC test\FIXED60_PHASE40K_INTEGRATION\MATLAB.zip!MATLAB/export_fixed60_phase40k_short_golden.m | CC89E989CBA6CF7C6E616C30C62AFDC327B7AA5A919157DA85BEE9B5C575D4E6 |

file_inventory.json 保存本次所有 327 個既有檔案的完整路徑、大小、時間與 hash；archive_inventory.json 保存 67 個 ZIP entry 的 hash、时间、與實體檔比較。preservation_check.json 保存本次原檔是否改變的檢查結果。

