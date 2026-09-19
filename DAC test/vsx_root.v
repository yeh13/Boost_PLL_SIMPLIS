//
// SIMetrix Verilog top level Bench
//

`include "FIXED60_PHASE40K_INTEGRATION/VERILOG/fixed60_pll_phase40k_candidate.v"

`timescale 1fs/1fs
module vsx_root() ;

    wire [11:0] VSX$_0 ;
    reg VSX$_0_0 ;
    reg VSX$_0_1 ;
    reg VSX$_0_2 ;
    reg VSX$_0_3 ;
    reg VSX$_0_4 ;
    reg VSX$_0_5 ;
    reg VSX$_0_6 ;
    reg VSX$_0_7 ;
    reg VSX$_0_8 ;
    reg VSX$_0_9 ;
    reg VSX$_0_10 ;
    reg VSX$_0_11 ;
    reg VSX$_1 ;
    reg VSX$_2 ;
    wire VSX$_3 ;
    wire VSX$_4 ;
    wire VSX$_5 ;
    wire [29:0] VSX$_6 ;
    wire VSX$_6_0 ;
    wire VSX$_6_1 ;
    wire VSX$_6_2 ;
    wire VSX$_6_3 ;
    wire VSX$_6_4 ;
    wire VSX$_6_5 ;
    wire VSX$_6_6 ;
    wire VSX$_6_7 ;
    wire VSX$_6_8 ;
    wire VSX$_6_9 ;
    wire VSX$_6_10 ;
    wire VSX$_6_11 ;
    wire VSX$_6_12 ;
    wire VSX$_6_13 ;
    wire VSX$_6_14 ;
    wire VSX$_6_15 ;
    wire VSX$_6_16 ;
    wire VSX$_6_17 ;
    wire VSX$_6_18 ;
    wire VSX$_6_19 ;
    wire VSX$_6_20 ;
    wire VSX$_6_21 ;
    wire VSX$_6_22 ;
    wire VSX$_6_23 ;
    wire VSX$_6_24 ;
    wire VSX$_6_25 ;
    wire VSX$_6_26 ;
    wire VSX$_6_27 ;
    wire VSX$_6_28 ;
    wire VSX$_6_29 ;
    wire [29:0] VSX$_7 ;
    wire VSX$_7_0 ;
    wire VSX$_7_1 ;
    wire VSX$_7_2 ;
    wire VSX$_7_3 ;
    wire VSX$_7_4 ;
    wire VSX$_7_5 ;
    wire VSX$_7_6 ;
    wire VSX$_7_7 ;
    wire VSX$_7_8 ;
    wire VSX$_7_9 ;
    wire VSX$_7_10 ;
    wire VSX$_7_11 ;
    wire VSX$_7_12 ;
    wire VSX$_7_13 ;
    wire VSX$_7_14 ;
    wire VSX$_7_15 ;
    wire VSX$_7_16 ;
    wire VSX$_7_17 ;
    wire VSX$_7_18 ;
    wire VSX$_7_19 ;
    wire VSX$_7_20 ;
    wire VSX$_7_21 ;
    wire VSX$_7_22 ;
    wire VSX$_7_23 ;
    wire VSX$_7_24 ;
    wire VSX$_7_25 ;
    wire VSX$_7_26 ;
    wire VSX$_7_27 ;
    wire VSX$_7_28 ;
    wire VSX$_7_29 ;
    wire [31:0] VSX$_8 ;
    wire VSX$_8_0 ;
    wire VSX$_8_1 ;
    wire VSX$_8_2 ;
    wire VSX$_8_3 ;
    wire VSX$_8_4 ;
    wire VSX$_8_5 ;
    wire VSX$_8_6 ;
    wire VSX$_8_7 ;
    wire VSX$_8_8 ;
    wire VSX$_8_9 ;
    wire VSX$_8_10 ;
    wire VSX$_8_11 ;
    wire VSX$_8_12 ;
    wire VSX$_8_13 ;
    wire VSX$_8_14 ;
    wire VSX$_8_15 ;
    wire VSX$_8_16 ;
    wire VSX$_8_17 ;
    wire VSX$_8_18 ;
    wire VSX$_8_19 ;
    wire VSX$_8_20 ;
    wire VSX$_8_21 ;
    wire VSX$_8_22 ;
    wire VSX$_8_23 ;
    wire VSX$_8_24 ;
    wire VSX$_8_25 ;
    wire VSX$_8_26 ;
    wire VSX$_8_27 ;
    wire VSX$_8_28 ;
    wire VSX$_8_29 ;
    wire VSX$_8_30 ;
    wire VSX$_8_31 ;
    wire [31:0] VSX$_9 ;
    wire VSX$_9_0 ;
    wire VSX$_9_1 ;
    wire VSX$_9_2 ;
    wire VSX$_9_3 ;
    wire VSX$_9_4 ;
    wire VSX$_9_5 ;
    wire VSX$_9_6 ;
    wire VSX$_9_7 ;
    wire VSX$_9_8 ;
    wire VSX$_9_9 ;
    wire VSX$_9_10 ;
    wire VSX$_9_11 ;
    wire VSX$_9_12 ;
    wire VSX$_9_13 ;
    wire VSX$_9_14 ;
    wire VSX$_9_15 ;
    wire VSX$_9_16 ;
    wire VSX$_9_17 ;
    wire VSX$_9_18 ;
    wire VSX$_9_19 ;
    wire VSX$_9_20 ;
    wire VSX$_9_21 ;
    wire VSX$_9_22 ;
    wire VSX$_9_23 ;
    wire VSX$_9_24 ;
    wire VSX$_9_25 ;
    wire VSX$_9_26 ;
    wire VSX$_9_27 ;
    wire VSX$_9_28 ;
    wire VSX$_9_29 ;
    wire VSX$_9_30 ;
    wire VSX$_9_31 ;
    wire VSX$_10 ;
    wire VSX$_11 ;
    wire [32:0] VSX$_12 ;
    wire VSX$_12_0 ;
    wire VSX$_12_1 ;
    wire VSX$_12_2 ;
    wire VSX$_12_3 ;
    wire VSX$_12_4 ;
    wire VSX$_12_5 ;
    wire VSX$_12_6 ;
    wire VSX$_12_7 ;
    wire VSX$_12_8 ;
    wire VSX$_12_9 ;
    wire VSX$_12_10 ;
    wire VSX$_12_11 ;
    wire VSX$_12_12 ;
    wire VSX$_12_13 ;
    wire VSX$_12_14 ;
    wire VSX$_12_15 ;
    wire VSX$_12_16 ;
    wire VSX$_12_17 ;
    wire VSX$_12_18 ;
    wire VSX$_12_19 ;
    wire VSX$_12_20 ;
    wire VSX$_12_21 ;
    wire VSX$_12_22 ;
    wire VSX$_12_23 ;
    wire VSX$_12_24 ;
    wire VSX$_12_25 ;
    wire VSX$_12_26 ;
    wire VSX$_12_27 ;
    wire VSX$_12_28 ;
    wire VSX$_12_29 ;
    wire VSX$_12_30 ;
    wire VSX$_12_31 ;
    wire VSX$_12_32 ;
    wire [63:0] VSX$_13 ;
    wire VSX$_13_0 ;
    wire VSX$_13_1 ;
    wire VSX$_13_2 ;
    wire VSX$_13_3 ;
    wire VSX$_13_4 ;
    wire VSX$_13_5 ;
    wire VSX$_13_6 ;
    wire VSX$_13_7 ;
    wire VSX$_13_8 ;
    wire VSX$_13_9 ;
    wire VSX$_13_10 ;
    wire VSX$_13_11 ;
    wire VSX$_13_12 ;
    wire VSX$_13_13 ;
    wire VSX$_13_14 ;
    wire VSX$_13_15 ;
    wire VSX$_13_16 ;
    wire VSX$_13_17 ;
    wire VSX$_13_18 ;
    wire VSX$_13_19 ;
    wire VSX$_13_20 ;
    wire VSX$_13_21 ;
    wire VSX$_13_22 ;
    wire VSX$_13_23 ;
    wire VSX$_13_24 ;
    wire VSX$_13_25 ;
    wire VSX$_13_26 ;
    wire VSX$_13_27 ;
    wire VSX$_13_28 ;
    wire VSX$_13_29 ;
    wire VSX$_13_30 ;
    wire VSX$_13_31 ;
    wire VSX$_13_32 ;
    wire VSX$_13_33 ;
    wire VSX$_13_34 ;
    wire VSX$_13_35 ;
    wire VSX$_13_36 ;
    wire VSX$_13_37 ;
    wire VSX$_13_38 ;
    wire VSX$_13_39 ;
    wire VSX$_13_40 ;
    wire VSX$_13_41 ;
    wire VSX$_13_42 ;
    wire VSX$_13_43 ;
    wire VSX$_13_44 ;
    wire VSX$_13_45 ;
    wire VSX$_13_46 ;
    wire VSX$_13_47 ;
    wire VSX$_13_48 ;
    wire VSX$_13_49 ;
    wire VSX$_13_50 ;
    wire VSX$_13_51 ;
    wire VSX$_13_52 ;
    wire VSX$_13_53 ;
    wire VSX$_13_54 ;
    wire VSX$_13_55 ;
    wire VSX$_13_56 ;
    wire VSX$_13_57 ;
    wire VSX$_13_58 ;
    wire VSX$_13_59 ;
    wire VSX$_13_60 ;
    wire VSX$_13_61 ;
    wire VSX$_13_62 ;
    wire VSX$_13_63 ;
    wire [63:0] VSX$_14 ;
    wire VSX$_14_0 ;
    wire VSX$_14_1 ;
    wire VSX$_14_2 ;
    wire VSX$_14_3 ;
    wire VSX$_14_4 ;
    wire VSX$_14_5 ;
    wire VSX$_14_6 ;
    wire VSX$_14_7 ;
    wire VSX$_14_8 ;
    wire VSX$_14_9 ;
    wire VSX$_14_10 ;
    wire VSX$_14_11 ;
    wire VSX$_14_12 ;
    wire VSX$_14_13 ;
    wire VSX$_14_14 ;
    wire VSX$_14_15 ;
    wire VSX$_14_16 ;
    wire VSX$_14_17 ;
    wire VSX$_14_18 ;
    wire VSX$_14_19 ;
    wire VSX$_14_20 ;
    wire VSX$_14_21 ;
    wire VSX$_14_22 ;
    wire VSX$_14_23 ;
    wire VSX$_14_24 ;
    wire VSX$_14_25 ;
    wire VSX$_14_26 ;
    wire VSX$_14_27 ;
    wire VSX$_14_28 ;
    wire VSX$_14_29 ;
    wire VSX$_14_30 ;
    wire VSX$_14_31 ;
    wire VSX$_14_32 ;
    wire VSX$_14_33 ;
    wire VSX$_14_34 ;
    wire VSX$_14_35 ;
    wire VSX$_14_36 ;
    wire VSX$_14_37 ;
    wire VSX$_14_38 ;
    wire VSX$_14_39 ;
    wire VSX$_14_40 ;
    wire VSX$_14_41 ;
    wire VSX$_14_42 ;
    wire VSX$_14_43 ;
    wire VSX$_14_44 ;
    wire VSX$_14_45 ;
    wire VSX$_14_46 ;
    wire VSX$_14_47 ;
    wire VSX$_14_48 ;
    wire VSX$_14_49 ;
    wire VSX$_14_50 ;
    wire VSX$_14_51 ;
    wire VSX$_14_52 ;
    wire VSX$_14_53 ;
    wire VSX$_14_54 ;
    wire VSX$_14_55 ;
    wire VSX$_14_56 ;
    wire VSX$_14_57 ;
    wire VSX$_14_58 ;
    wire VSX$_14_59 ;
    wire VSX$_14_60 ;
    wire VSX$_14_61 ;
    wire VSX$_14_62 ;
    wire VSX$_14_63 ;
    wire [63:0] VSX$_15 ;
    wire VSX$_15_0 ;
    wire VSX$_15_1 ;
    wire VSX$_15_2 ;
    wire VSX$_15_3 ;
    wire VSX$_15_4 ;
    wire VSX$_15_5 ;
    wire VSX$_15_6 ;
    wire VSX$_15_7 ;
    wire VSX$_15_8 ;
    wire VSX$_15_9 ;
    wire VSX$_15_10 ;
    wire VSX$_15_11 ;
    wire VSX$_15_12 ;
    wire VSX$_15_13 ;
    wire VSX$_15_14 ;
    wire VSX$_15_15 ;
    wire VSX$_15_16 ;
    wire VSX$_15_17 ;
    wire VSX$_15_18 ;
    wire VSX$_15_19 ;
    wire VSX$_15_20 ;
    wire VSX$_15_21 ;
    wire VSX$_15_22 ;
    wire VSX$_15_23 ;
    wire VSX$_15_24 ;
    wire VSX$_15_25 ;
    wire VSX$_15_26 ;
    wire VSX$_15_27 ;
    wire VSX$_15_28 ;
    wire VSX$_15_29 ;
    wire VSX$_15_30 ;
    wire VSX$_15_31 ;
    wire VSX$_15_32 ;
    wire VSX$_15_33 ;
    wire VSX$_15_34 ;
    wire VSX$_15_35 ;
    wire VSX$_15_36 ;
    wire VSX$_15_37 ;
    wire VSX$_15_38 ;
    wire VSX$_15_39 ;
    wire VSX$_15_40 ;
    wire VSX$_15_41 ;
    wire VSX$_15_42 ;
    wire VSX$_15_43 ;
    wire VSX$_15_44 ;
    wire VSX$_15_45 ;
    wire VSX$_15_46 ;
    wire VSX$_15_47 ;
    wire VSX$_15_48 ;
    wire VSX$_15_49 ;
    wire VSX$_15_50 ;
    wire VSX$_15_51 ;
    wire VSX$_15_52 ;
    wire VSX$_15_53 ;
    wire VSX$_15_54 ;
    wire VSX$_15_55 ;
    wire VSX$_15_56 ;
    wire VSX$_15_57 ;
    wire VSX$_15_58 ;
    wire VSX$_15_59 ;
    wire VSX$_15_60 ;
    wire VSX$_15_61 ;
    wire VSX$_15_62 ;
    wire VSX$_15_63 ;
    wire [63:0] VSX$_16 ;
    wire VSX$_16_0 ;
    wire VSX$_16_1 ;
    wire VSX$_16_2 ;
    wire VSX$_16_3 ;
    wire VSX$_16_4 ;
    wire VSX$_16_5 ;
    wire VSX$_16_6 ;
    wire VSX$_16_7 ;
    wire VSX$_16_8 ;
    wire VSX$_16_9 ;
    wire VSX$_16_10 ;
    wire VSX$_16_11 ;
    wire VSX$_16_12 ;
    wire VSX$_16_13 ;
    wire VSX$_16_14 ;
    wire VSX$_16_15 ;
    wire VSX$_16_16 ;
    wire VSX$_16_17 ;
    wire VSX$_16_18 ;
    wire VSX$_16_19 ;
    wire VSX$_16_20 ;
    wire VSX$_16_21 ;
    wire VSX$_16_22 ;
    wire VSX$_16_23 ;
    wire VSX$_16_24 ;
    wire VSX$_16_25 ;
    wire VSX$_16_26 ;
    wire VSX$_16_27 ;
    wire VSX$_16_28 ;
    wire VSX$_16_29 ;
    wire VSX$_16_30 ;
    wire VSX$_16_31 ;
    wire VSX$_16_32 ;
    wire VSX$_16_33 ;
    wire VSX$_16_34 ;
    wire VSX$_16_35 ;
    wire VSX$_16_36 ;
    wire VSX$_16_37 ;
    wire VSX$_16_38 ;
    wire VSX$_16_39 ;
    wire VSX$_16_40 ;
    wire VSX$_16_41 ;
    wire VSX$_16_42 ;
    wire VSX$_16_43 ;
    wire VSX$_16_44 ;
    wire VSX$_16_45 ;
    wire VSX$_16_46 ;
    wire VSX$_16_47 ;
    wire VSX$_16_48 ;
    wire VSX$_16_49 ;
    wire VSX$_16_50 ;
    wire VSX$_16_51 ;
    wire VSX$_16_52 ;
    wire VSX$_16_53 ;
    wire VSX$_16_54 ;
    wire VSX$_16_55 ;
    wire VSX$_16_56 ;
    wire VSX$_16_57 ;
    wire VSX$_16_58 ;
    wire VSX$_16_59 ;
    wire VSX$_16_60 ;
    wire VSX$_16_61 ;
    wire VSX$_16_62 ;
    wire VSX$_16_63 ;
    wire [63:0] VSX$_17 ;
    wire VSX$_17_0 ;
    wire VSX$_17_1 ;
    wire VSX$_17_2 ;
    wire VSX$_17_3 ;
    wire VSX$_17_4 ;
    wire VSX$_17_5 ;
    wire VSX$_17_6 ;
    wire VSX$_17_7 ;
    wire VSX$_17_8 ;
    wire VSX$_17_9 ;
    wire VSX$_17_10 ;
    wire VSX$_17_11 ;
    wire VSX$_17_12 ;
    wire VSX$_17_13 ;
    wire VSX$_17_14 ;
    wire VSX$_17_15 ;
    wire VSX$_17_16 ;
    wire VSX$_17_17 ;
    wire VSX$_17_18 ;
    wire VSX$_17_19 ;
    wire VSX$_17_20 ;
    wire VSX$_17_21 ;
    wire VSX$_17_22 ;
    wire VSX$_17_23 ;
    wire VSX$_17_24 ;
    wire VSX$_17_25 ;
    wire VSX$_17_26 ;
    wire VSX$_17_27 ;
    wire VSX$_17_28 ;
    wire VSX$_17_29 ;
    wire VSX$_17_30 ;
    wire VSX$_17_31 ;
    wire VSX$_17_32 ;
    wire VSX$_17_33 ;
    wire VSX$_17_34 ;
    wire VSX$_17_35 ;
    wire VSX$_17_36 ;
    wire VSX$_17_37 ;
    wire VSX$_17_38 ;
    wire VSX$_17_39 ;
    wire VSX$_17_40 ;
    wire VSX$_17_41 ;
    wire VSX$_17_42 ;
    wire VSX$_17_43 ;
    wire VSX$_17_44 ;
    wire VSX$_17_45 ;
    wire VSX$_17_46 ;
    wire VSX$_17_47 ;
    wire VSX$_17_48 ;
    wire VSX$_17_49 ;
    wire VSX$_17_50 ;
    wire VSX$_17_51 ;
    wire VSX$_17_52 ;
    wire VSX$_17_53 ;
    wire VSX$_17_54 ;
    wire VSX$_17_55 ;
    wire VSX$_17_56 ;
    wire VSX$_17_57 ;
    wire VSX$_17_58 ;
    wire VSX$_17_59 ;
    wire VSX$_17_60 ;
    wire VSX$_17_61 ;
    wire VSX$_17_62 ;
    wire VSX$_17_63 ;
    wire [63:0] VSX$_18 ;
    wire VSX$_18_0 ;
    wire VSX$_18_1 ;
    wire VSX$_18_2 ;
    wire VSX$_18_3 ;
    wire VSX$_18_4 ;
    wire VSX$_18_5 ;
    wire VSX$_18_6 ;
    wire VSX$_18_7 ;
    wire VSX$_18_8 ;
    wire VSX$_18_9 ;
    wire VSX$_18_10 ;
    wire VSX$_18_11 ;
    wire VSX$_18_12 ;
    wire VSX$_18_13 ;
    wire VSX$_18_14 ;
    wire VSX$_18_15 ;
    wire VSX$_18_16 ;
    wire VSX$_18_17 ;
    wire VSX$_18_18 ;
    wire VSX$_18_19 ;
    wire VSX$_18_20 ;
    wire VSX$_18_21 ;
    wire VSX$_18_22 ;
    wire VSX$_18_23 ;
    wire VSX$_18_24 ;
    wire VSX$_18_25 ;
    wire VSX$_18_26 ;
    wire VSX$_18_27 ;
    wire VSX$_18_28 ;
    wire VSX$_18_29 ;
    wire VSX$_18_30 ;
    wire VSX$_18_31 ;
    wire VSX$_18_32 ;
    wire VSX$_18_33 ;
    wire VSX$_18_34 ;
    wire VSX$_18_35 ;
    wire VSX$_18_36 ;
    wire VSX$_18_37 ;
    wire VSX$_18_38 ;
    wire VSX$_18_39 ;
    wire VSX$_18_40 ;
    wire VSX$_18_41 ;
    wire VSX$_18_42 ;
    wire VSX$_18_43 ;
    wire VSX$_18_44 ;
    wire VSX$_18_45 ;
    wire VSX$_18_46 ;
    wire VSX$_18_47 ;
    wire VSX$_18_48 ;
    wire VSX$_18_49 ;
    wire VSX$_18_50 ;
    wire VSX$_18_51 ;
    wire VSX$_18_52 ;
    wire VSX$_18_53 ;
    wire VSX$_18_54 ;
    wire VSX$_18_55 ;
    wire VSX$_18_56 ;
    wire VSX$_18_57 ;
    wire VSX$_18_58 ;
    wire VSX$_18_59 ;
    wire VSX$_18_60 ;
    wire VSX$_18_61 ;
    wire VSX$_18_62 ;
    wire VSX$_18_63 ;
    wire [2:0] VSX$_19 ;
    wire VSX$_19_0 ;
    wire VSX$_19_1 ;
    wire VSX$_19_2 ;
    wire [15:0] VSX$_20 ;
    wire VSX$_20_0 ;
    wire VSX$_20_1 ;
    wire VSX$_20_2 ;
    wire VSX$_20_3 ;
    wire VSX$_20_4 ;
    wire VSX$_20_5 ;
    wire VSX$_20_6 ;
    wire VSX$_20_7 ;
    wire VSX$_20_8 ;
    wire VSX$_20_9 ;
    wire VSX$_20_10 ;
    wire VSX$_20_11 ;
    wire VSX$_20_12 ;
    wire VSX$_20_13 ;
    wire VSX$_20_14 ;
    wire VSX$_20_15 ;
    wire [15:0] VSX$_21 ;
    wire VSX$_21_0 ;
    wire VSX$_21_1 ;
    wire VSX$_21_2 ;
    wire VSX$_21_3 ;
    wire VSX$_21_4 ;
    wire VSX$_21_5 ;
    wire VSX$_21_6 ;
    wire VSX$_21_7 ;
    wire VSX$_21_8 ;
    wire VSX$_21_9 ;
    wire VSX$_21_10 ;
    wire VSX$_21_11 ;
    wire VSX$_21_12 ;
    wire VSX$_21_13 ;
    wire VSX$_21_14 ;
    wire VSX$_21_15 ;
    wire [63:0] VSX$_22 ;
    wire VSX$_22_0 ;
    wire VSX$_22_1 ;
    wire VSX$_22_2 ;
    wire VSX$_22_3 ;
    wire VSX$_22_4 ;
    wire VSX$_22_5 ;
    wire VSX$_22_6 ;
    wire VSX$_22_7 ;
    wire VSX$_22_8 ;
    wire VSX$_22_9 ;
    wire VSX$_22_10 ;
    wire VSX$_22_11 ;
    wire VSX$_22_12 ;
    wire VSX$_22_13 ;
    wire VSX$_22_14 ;
    wire VSX$_22_15 ;
    wire VSX$_22_16 ;
    wire VSX$_22_17 ;
    wire VSX$_22_18 ;
    wire VSX$_22_19 ;
    wire VSX$_22_20 ;
    wire VSX$_22_21 ;
    wire VSX$_22_22 ;
    wire VSX$_22_23 ;
    wire VSX$_22_24 ;
    wire VSX$_22_25 ;
    wire VSX$_22_26 ;
    wire VSX$_22_27 ;
    wire VSX$_22_28 ;
    wire VSX$_22_29 ;
    wire VSX$_22_30 ;
    wire VSX$_22_31 ;
    wire VSX$_22_32 ;
    wire VSX$_22_33 ;
    wire VSX$_22_34 ;
    wire VSX$_22_35 ;
    wire VSX$_22_36 ;
    wire VSX$_22_37 ;
    wire VSX$_22_38 ;
    wire VSX$_22_39 ;
    wire VSX$_22_40 ;
    wire VSX$_22_41 ;
    wire VSX$_22_42 ;
    wire VSX$_22_43 ;
    wire VSX$_22_44 ;
    wire VSX$_22_45 ;
    wire VSX$_22_46 ;
    wire VSX$_22_47 ;
    wire VSX$_22_48 ;
    wire VSX$_22_49 ;
    wire VSX$_22_50 ;
    wire VSX$_22_51 ;
    wire VSX$_22_52 ;
    wire VSX$_22_53 ;
    wire VSX$_22_54 ;
    wire VSX$_22_55 ;
    wire VSX$_22_56 ;
    wire VSX$_22_57 ;
    wire VSX$_22_58 ;
    wire VSX$_22_59 ;
    wire VSX$_22_60 ;
    wire VSX$_22_61 ;
    wire VSX$_22_62 ;
    wire VSX$_22_63 ;
    wire VSX$_23 ;
    assign VSX$_0[0] = VSX$_0_0 ; // node: BUS1#0
    assign VSX$_0[1] = VSX$_0_1 ; // node: BUS1#1
    assign VSX$_0[2] = VSX$_0_2 ; // node: BUS1#2
    assign VSX$_0[3] = VSX$_0_3 ; // node: BUS1#3
    assign VSX$_0[4] = VSX$_0_4 ; // node: BUS1#4
    assign VSX$_0[5] = VSX$_0_5 ; // node: BUS1#5
    assign VSX$_0[6] = VSX$_0_6 ; // node: BUS1#6
    assign VSX$_0[7] = VSX$_0_7 ; // node: BUS1#7
    assign VSX$_0[8] = VSX$_0_8 ; // node: BUS1#8
    assign VSX$_0[9] = VSX$_0_9 ; // node: BUS1#9
    assign VSX$_0[10] = VSX$_0_10 ; // node: BUS1#10
    assign VSX$_0[11] = VSX$_0_11 ; // node: BUS1#11
    assign VSX$_6_0 = VSX$_6[0] ; // node: BUS2#0
    assign VSX$_6_1 = VSX$_6[1] ; // node: BUS2#1
    assign VSX$_6_2 = VSX$_6[2] ; // node: BUS2#2
    assign VSX$_6_3 = VSX$_6[3] ; // node: BUS2#3
    assign VSX$_6_4 = VSX$_6[4] ; // node: BUS2#4
    assign VSX$_6_5 = VSX$_6[5] ; // node: BUS2#5
    assign VSX$_6_6 = VSX$_6[6] ; // node: BUS2#6
    assign VSX$_6_7 = VSX$_6[7] ; // node: BUS2#7
    assign VSX$_6_8 = VSX$_6[8] ; // node: BUS2#8
    assign VSX$_6_9 = VSX$_6[9] ; // node: BUS2#9
    assign VSX$_6_10 = VSX$_6[10] ; // node: BUS2#10
    assign VSX$_6_11 = VSX$_6[11] ; // node: BUS2#11
    assign VSX$_6_12 = VSX$_6[12] ; // node: BUS2#12
    assign VSX$_6_13 = VSX$_6[13] ; // node: BUS2#13
    assign VSX$_6_14 = VSX$_6[14] ; // node: BUS2#14
    assign VSX$_6_15 = VSX$_6[15] ; // node: BUS2#15
    assign VSX$_6_16 = VSX$_6[16] ; // node: BUS2#16
    assign VSX$_6_17 = VSX$_6[17] ; // node: BUS2#17
    assign VSX$_6_18 = VSX$_6[18] ; // node: BUS2#18
    assign VSX$_6_19 = VSX$_6[19] ; // node: BUS2#19
    assign VSX$_6_20 = VSX$_6[20] ; // node: BUS2#20
    assign VSX$_6_21 = VSX$_6[21] ; // node: BUS2#21
    assign VSX$_6_22 = VSX$_6[22] ; // node: BUS2#22
    assign VSX$_6_23 = VSX$_6[23] ; // node: BUS2#23
    assign VSX$_6_24 = VSX$_6[24] ; // node: BUS2#24
    assign VSX$_6_25 = VSX$_6[25] ; // node: BUS2#25
    assign VSX$_6_26 = VSX$_6[26] ; // node: BUS2#26
    assign VSX$_6_27 = VSX$_6[27] ; // node: BUS2#27
    assign VSX$_6_28 = VSX$_6[28] ; // node: BUS2#28
    assign VSX$_6_29 = VSX$_6[29] ; // node: BUS2#29
    assign VSX$_7_0 = VSX$_7[0] ; // node: BUS3#0
    assign VSX$_7_1 = VSX$_7[1] ; // node: BUS3#1
    assign VSX$_7_2 = VSX$_7[2] ; // node: BUS3#2
    assign VSX$_7_3 = VSX$_7[3] ; // node: BUS3#3
    assign VSX$_7_4 = VSX$_7[4] ; // node: BUS3#4
    assign VSX$_7_5 = VSX$_7[5] ; // node: BUS3#5
    assign VSX$_7_6 = VSX$_7[6] ; // node: BUS3#6
    assign VSX$_7_7 = VSX$_7[7] ; // node: BUS3#7
    assign VSX$_7_8 = VSX$_7[8] ; // node: BUS3#8
    assign VSX$_7_9 = VSX$_7[9] ; // node: BUS3#9
    assign VSX$_7_10 = VSX$_7[10] ; // node: BUS3#10
    assign VSX$_7_11 = VSX$_7[11] ; // node: BUS3#11
    assign VSX$_7_12 = VSX$_7[12] ; // node: BUS3#12
    assign VSX$_7_13 = VSX$_7[13] ; // node: BUS3#13
    assign VSX$_7_14 = VSX$_7[14] ; // node: BUS3#14
    assign VSX$_7_15 = VSX$_7[15] ; // node: BUS3#15
    assign VSX$_7_16 = VSX$_7[16] ; // node: BUS3#16
    assign VSX$_7_17 = VSX$_7[17] ; // node: BUS3#17
    assign VSX$_7_18 = VSX$_7[18] ; // node: BUS3#18
    assign VSX$_7_19 = VSX$_7[19] ; // node: BUS3#19
    assign VSX$_7_20 = VSX$_7[20] ; // node: BUS3#20
    assign VSX$_7_21 = VSX$_7[21] ; // node: BUS3#21
    assign VSX$_7_22 = VSX$_7[22] ; // node: BUS3#22
    assign VSX$_7_23 = VSX$_7[23] ; // node: BUS3#23
    assign VSX$_7_24 = VSX$_7[24] ; // node: BUS3#24
    assign VSX$_7_25 = VSX$_7[25] ; // node: BUS3#25
    assign VSX$_7_26 = VSX$_7[26] ; // node: BUS3#26
    assign VSX$_7_27 = VSX$_7[27] ; // node: BUS3#27
    assign VSX$_7_28 = VSX$_7[28] ; // node: BUS3#28
    assign VSX$_7_29 = VSX$_7[29] ; // node: BUS3#29
    assign VSX$_8_0 = VSX$_8[0] ; // node: BUS4#0
    assign VSX$_8_1 = VSX$_8[1] ; // node: BUS4#1
    assign VSX$_8_2 = VSX$_8[2] ; // node: BUS4#2
    assign VSX$_8_3 = VSX$_8[3] ; // node: BUS4#3
    assign VSX$_8_4 = VSX$_8[4] ; // node: BUS4#4
    assign VSX$_8_5 = VSX$_8[5] ; // node: BUS4#5
    assign VSX$_8_6 = VSX$_8[6] ; // node: BUS4#6
    assign VSX$_8_7 = VSX$_8[7] ; // node: BUS4#7
    assign VSX$_8_8 = VSX$_8[8] ; // node: BUS4#8
    assign VSX$_8_9 = VSX$_8[9] ; // node: BUS4#9
    assign VSX$_8_10 = VSX$_8[10] ; // node: BUS4#10
    assign VSX$_8_11 = VSX$_8[11] ; // node: BUS4#11
    assign VSX$_8_12 = VSX$_8[12] ; // node: BUS4#12
    assign VSX$_8_13 = VSX$_8[13] ; // node: BUS4#13
    assign VSX$_8_14 = VSX$_8[14] ; // node: BUS4#14
    assign VSX$_8_15 = VSX$_8[15] ; // node: BUS4#15
    assign VSX$_8_16 = VSX$_8[16] ; // node: BUS4#16
    assign VSX$_8_17 = VSX$_8[17] ; // node: BUS4#17
    assign VSX$_8_18 = VSX$_8[18] ; // node: BUS4#18
    assign VSX$_8_19 = VSX$_8[19] ; // node: BUS4#19
    assign VSX$_8_20 = VSX$_8[20] ; // node: BUS4#20
    assign VSX$_8_21 = VSX$_8[21] ; // node: BUS4#21
    assign VSX$_8_22 = VSX$_8[22] ; // node: BUS4#22
    assign VSX$_8_23 = VSX$_8[23] ; // node: BUS4#23
    assign VSX$_8_24 = VSX$_8[24] ; // node: BUS4#24
    assign VSX$_8_25 = VSX$_8[25] ; // node: BUS4#25
    assign VSX$_8_26 = VSX$_8[26] ; // node: BUS4#26
    assign VSX$_8_27 = VSX$_8[27] ; // node: BUS4#27
    assign VSX$_8_28 = VSX$_8[28] ; // node: BUS4#28
    assign VSX$_8_29 = VSX$_8[29] ; // node: BUS4#29
    assign VSX$_8_30 = VSX$_8[30] ; // node: BUS4#30
    assign VSX$_8_31 = VSX$_8[31] ; // node: BUS4#31
    assign VSX$_9_0 = VSX$_9[0] ; // node: BUS5#0
    assign VSX$_9_1 = VSX$_9[1] ; // node: BUS5#1
    assign VSX$_9_2 = VSX$_9[2] ; // node: BUS5#2
    assign VSX$_9_3 = VSX$_9[3] ; // node: BUS5#3
    assign VSX$_9_4 = VSX$_9[4] ; // node: BUS5#4
    assign VSX$_9_5 = VSX$_9[5] ; // node: BUS5#5
    assign VSX$_9_6 = VSX$_9[6] ; // node: BUS5#6
    assign VSX$_9_7 = VSX$_9[7] ; // node: BUS5#7
    assign VSX$_9_8 = VSX$_9[8] ; // node: BUS5#8
    assign VSX$_9_9 = VSX$_9[9] ; // node: BUS5#9
    assign VSX$_9_10 = VSX$_9[10] ; // node: BUS5#10
    assign VSX$_9_11 = VSX$_9[11] ; // node: BUS5#11
    assign VSX$_9_12 = VSX$_9[12] ; // node: BUS5#12
    assign VSX$_9_13 = VSX$_9[13] ; // node: BUS5#13
    assign VSX$_9_14 = VSX$_9[14] ; // node: BUS5#14
    assign VSX$_9_15 = VSX$_9[15] ; // node: BUS5#15
    assign VSX$_9_16 = VSX$_9[16] ; // node: BUS5#16
    assign VSX$_9_17 = VSX$_9[17] ; // node: BUS5#17
    assign VSX$_9_18 = VSX$_9[18] ; // node: BUS5#18
    assign VSX$_9_19 = VSX$_9[19] ; // node: BUS5#19
    assign VSX$_9_20 = VSX$_9[20] ; // node: BUS5#20
    assign VSX$_9_21 = VSX$_9[21] ; // node: BUS5#21
    assign VSX$_9_22 = VSX$_9[22] ; // node: BUS5#22
    assign VSX$_9_23 = VSX$_9[23] ; // node: BUS5#23
    assign VSX$_9_24 = VSX$_9[24] ; // node: BUS5#24
    assign VSX$_9_25 = VSX$_9[25] ; // node: BUS5#25
    assign VSX$_9_26 = VSX$_9[26] ; // node: BUS5#26
    assign VSX$_9_27 = VSX$_9[27] ; // node: BUS5#27
    assign VSX$_9_28 = VSX$_9[28] ; // node: BUS5#28
    assign VSX$_9_29 = VSX$_9[29] ; // node: BUS5#29
    assign VSX$_9_30 = VSX$_9[30] ; // node: BUS5#30
    assign VSX$_9_31 = VSX$_9[31] ; // node: BUS5#31
    assign VSX$_12_0 = VSX$_12[0] ; // node: BUS6#0
    assign VSX$_12_1 = VSX$_12[1] ; // node: BUS6#1
    assign VSX$_12_2 = VSX$_12[2] ; // node: BUS6#2
    assign VSX$_12_3 = VSX$_12[3] ; // node: BUS6#3
    assign VSX$_12_4 = VSX$_12[4] ; // node: BUS6#4
    assign VSX$_12_5 = VSX$_12[5] ; // node: BUS6#5
    assign VSX$_12_6 = VSX$_12[6] ; // node: BUS6#6
    assign VSX$_12_7 = VSX$_12[7] ; // node: BUS6#7
    assign VSX$_12_8 = VSX$_12[8] ; // node: BUS6#8
    assign VSX$_12_9 = VSX$_12[9] ; // node: BUS6#9
    assign VSX$_12_10 = VSX$_12[10] ; // node: BUS6#10
    assign VSX$_12_11 = VSX$_12[11] ; // node: BUS6#11
    assign VSX$_12_12 = VSX$_12[12] ; // node: BUS6#12
    assign VSX$_12_13 = VSX$_12[13] ; // node: BUS6#13
    assign VSX$_12_14 = VSX$_12[14] ; // node: BUS6#14
    assign VSX$_12_15 = VSX$_12[15] ; // node: BUS6#15
    assign VSX$_12_16 = VSX$_12[16] ; // node: BUS6#16
    assign VSX$_12_17 = VSX$_12[17] ; // node: BUS6#17
    assign VSX$_12_18 = VSX$_12[18] ; // node: BUS6#18
    assign VSX$_12_19 = VSX$_12[19] ; // node: BUS6#19
    assign VSX$_12_20 = VSX$_12[20] ; // node: BUS6#20
    assign VSX$_12_21 = VSX$_12[21] ; // node: BUS6#21
    assign VSX$_12_22 = VSX$_12[22] ; // node: BUS6#22
    assign VSX$_12_23 = VSX$_12[23] ; // node: BUS6#23
    assign VSX$_12_24 = VSX$_12[24] ; // node: BUS6#24
    assign VSX$_12_25 = VSX$_12[25] ; // node: BUS6#25
    assign VSX$_12_26 = VSX$_12[26] ; // node: BUS6#26
    assign VSX$_12_27 = VSX$_12[27] ; // node: BUS6#27
    assign VSX$_12_28 = VSX$_12[28] ; // node: BUS6#28
    assign VSX$_12_29 = VSX$_12[29] ; // node: BUS6#29
    assign VSX$_12_30 = VSX$_12[30] ; // node: BUS6#30
    assign VSX$_12_31 = VSX$_12[31] ; // node: BUS6#31
    assign VSX$_12_32 = VSX$_12[32] ; // node: BUS6#32
    assign VSX$_13_0 = VSX$_13[0] ; // node: BUS7#0
    assign VSX$_13_1 = VSX$_13[1] ; // node: BUS7#1
    assign VSX$_13_2 = VSX$_13[2] ; // node: BUS7#2
    assign VSX$_13_3 = VSX$_13[3] ; // node: BUS7#3
    assign VSX$_13_4 = VSX$_13[4] ; // node: BUS7#4
    assign VSX$_13_5 = VSX$_13[5] ; // node: BUS7#5
    assign VSX$_13_6 = VSX$_13[6] ; // node: BUS7#6
    assign VSX$_13_7 = VSX$_13[7] ; // node: BUS7#7
    assign VSX$_13_8 = VSX$_13[8] ; // node: BUS7#8
    assign VSX$_13_9 = VSX$_13[9] ; // node: BUS7#9
    assign VSX$_13_10 = VSX$_13[10] ; // node: BUS7#10
    assign VSX$_13_11 = VSX$_13[11] ; // node: BUS7#11
    assign VSX$_13_12 = VSX$_13[12] ; // node: BUS7#12
    assign VSX$_13_13 = VSX$_13[13] ; // node: BUS7#13
    assign VSX$_13_14 = VSX$_13[14] ; // node: BUS7#14
    assign VSX$_13_15 = VSX$_13[15] ; // node: BUS7#15
    assign VSX$_13_16 = VSX$_13[16] ; // node: BUS7#16
    assign VSX$_13_17 = VSX$_13[17] ; // node: BUS7#17
    assign VSX$_13_18 = VSX$_13[18] ; // node: BUS7#18
    assign VSX$_13_19 = VSX$_13[19] ; // node: BUS7#19
    assign VSX$_13_20 = VSX$_13[20] ; // node: BUS7#20
    assign VSX$_13_21 = VSX$_13[21] ; // node: BUS7#21
    assign VSX$_13_22 = VSX$_13[22] ; // node: BUS7#22
    assign VSX$_13_23 = VSX$_13[23] ; // node: BUS7#23
    assign VSX$_13_24 = VSX$_13[24] ; // node: BUS7#24
    assign VSX$_13_25 = VSX$_13[25] ; // node: BUS7#25
    assign VSX$_13_26 = VSX$_13[26] ; // node: BUS7#26
    assign VSX$_13_27 = VSX$_13[27] ; // node: BUS7#27
    assign VSX$_13_28 = VSX$_13[28] ; // node: BUS7#28
    assign VSX$_13_29 = VSX$_13[29] ; // node: BUS7#29
    assign VSX$_13_30 = VSX$_13[30] ; // node: BUS7#30
    assign VSX$_13_31 = VSX$_13[31] ; // node: BUS7#31
    assign VSX$_13_32 = VSX$_13[32] ; // node: BUS7#32
    assign VSX$_13_33 = VSX$_13[33] ; // node: BUS7#33
    assign VSX$_13_34 = VSX$_13[34] ; // node: BUS7#34
    assign VSX$_13_35 = VSX$_13[35] ; // node: BUS7#35
    assign VSX$_13_36 = VSX$_13[36] ; // node: BUS7#36
    assign VSX$_13_37 = VSX$_13[37] ; // node: BUS7#37
    assign VSX$_13_38 = VSX$_13[38] ; // node: BUS7#38
    assign VSX$_13_39 = VSX$_13[39] ; // node: BUS7#39
    assign VSX$_13_40 = VSX$_13[40] ; // node: BUS7#40
    assign VSX$_13_41 = VSX$_13[41] ; // node: BUS7#41
    assign VSX$_13_42 = VSX$_13[42] ; // node: BUS7#42
    assign VSX$_13_43 = VSX$_13[43] ; // node: BUS7#43
    assign VSX$_13_44 = VSX$_13[44] ; // node: BUS7#44
    assign VSX$_13_45 = VSX$_13[45] ; // node: BUS7#45
    assign VSX$_13_46 = VSX$_13[46] ; // node: BUS7#46
    assign VSX$_13_47 = VSX$_13[47] ; // node: BUS7#47
    assign VSX$_13_48 = VSX$_13[48] ; // node: BUS7#48
    assign VSX$_13_49 = VSX$_13[49] ; // node: BUS7#49
    assign VSX$_13_50 = VSX$_13[50] ; // node: BUS7#50
    assign VSX$_13_51 = VSX$_13[51] ; // node: BUS7#51
    assign VSX$_13_52 = VSX$_13[52] ; // node: BUS7#52
    assign VSX$_13_53 = VSX$_13[53] ; // node: BUS7#53
    assign VSX$_13_54 = VSX$_13[54] ; // node: BUS7#54
    assign VSX$_13_55 = VSX$_13[55] ; // node: BUS7#55
    assign VSX$_13_56 = VSX$_13[56] ; // node: BUS7#56
    assign VSX$_13_57 = VSX$_13[57] ; // node: BUS7#57
    assign VSX$_13_58 = VSX$_13[58] ; // node: BUS7#58
    assign VSX$_13_59 = VSX$_13[59] ; // node: BUS7#59
    assign VSX$_13_60 = VSX$_13[60] ; // node: BUS7#60
    assign VSX$_13_61 = VSX$_13[61] ; // node: BUS7#61
    assign VSX$_13_62 = VSX$_13[62] ; // node: BUS7#62
    assign VSX$_13_63 = VSX$_13[63] ; // node: BUS7#63
    assign VSX$_14_0 = VSX$_14[0] ; // node: BUS8#0
    assign VSX$_14_1 = VSX$_14[1] ; // node: BUS8#1
    assign VSX$_14_2 = VSX$_14[2] ; // node: BUS8#2
    assign VSX$_14_3 = VSX$_14[3] ; // node: BUS8#3
    assign VSX$_14_4 = VSX$_14[4] ; // node: BUS8#4
    assign VSX$_14_5 = VSX$_14[5] ; // node: BUS8#5
    assign VSX$_14_6 = VSX$_14[6] ; // node: BUS8#6
    assign VSX$_14_7 = VSX$_14[7] ; // node: BUS8#7
    assign VSX$_14_8 = VSX$_14[8] ; // node: BUS8#8
    assign VSX$_14_9 = VSX$_14[9] ; // node: BUS8#9
    assign VSX$_14_10 = VSX$_14[10] ; // node: BUS8#10
    assign VSX$_14_11 = VSX$_14[11] ; // node: BUS8#11
    assign VSX$_14_12 = VSX$_14[12] ; // node: BUS8#12
    assign VSX$_14_13 = VSX$_14[13] ; // node: BUS8#13
    assign VSX$_14_14 = VSX$_14[14] ; // node: BUS8#14
    assign VSX$_14_15 = VSX$_14[15] ; // node: BUS8#15
    assign VSX$_14_16 = VSX$_14[16] ; // node: BUS8#16
    assign VSX$_14_17 = VSX$_14[17] ; // node: BUS8#17
    assign VSX$_14_18 = VSX$_14[18] ; // node: BUS8#18
    assign VSX$_14_19 = VSX$_14[19] ; // node: BUS8#19
    assign VSX$_14_20 = VSX$_14[20] ; // node: BUS8#20
    assign VSX$_14_21 = VSX$_14[21] ; // node: BUS8#21
    assign VSX$_14_22 = VSX$_14[22] ; // node: BUS8#22
    assign VSX$_14_23 = VSX$_14[23] ; // node: BUS8#23
    assign VSX$_14_24 = VSX$_14[24] ; // node: BUS8#24
    assign VSX$_14_25 = VSX$_14[25] ; // node: BUS8#25
    assign VSX$_14_26 = VSX$_14[26] ; // node: BUS8#26
    assign VSX$_14_27 = VSX$_14[27] ; // node: BUS8#27
    assign VSX$_14_28 = VSX$_14[28] ; // node: BUS8#28
    assign VSX$_14_29 = VSX$_14[29] ; // node: BUS8#29
    assign VSX$_14_30 = VSX$_14[30] ; // node: BUS8#30
    assign VSX$_14_31 = VSX$_14[31] ; // node: BUS8#31
    assign VSX$_14_32 = VSX$_14[32] ; // node: BUS8#32
    assign VSX$_14_33 = VSX$_14[33] ; // node: BUS8#33
    assign VSX$_14_34 = VSX$_14[34] ; // node: BUS8#34
    assign VSX$_14_35 = VSX$_14[35] ; // node: BUS8#35
    assign VSX$_14_36 = VSX$_14[36] ; // node: BUS8#36
    assign VSX$_14_37 = VSX$_14[37] ; // node: BUS8#37
    assign VSX$_14_38 = VSX$_14[38] ; // node: BUS8#38
    assign VSX$_14_39 = VSX$_14[39] ; // node: BUS8#39
    assign VSX$_14_40 = VSX$_14[40] ; // node: BUS8#40
    assign VSX$_14_41 = VSX$_14[41] ; // node: BUS8#41
    assign VSX$_14_42 = VSX$_14[42] ; // node: BUS8#42
    assign VSX$_14_43 = VSX$_14[43] ; // node: BUS8#43
    assign VSX$_14_44 = VSX$_14[44] ; // node: BUS8#44
    assign VSX$_14_45 = VSX$_14[45] ; // node: BUS8#45
    assign VSX$_14_46 = VSX$_14[46] ; // node: BUS8#46
    assign VSX$_14_47 = VSX$_14[47] ; // node: BUS8#47
    assign VSX$_14_48 = VSX$_14[48] ; // node: BUS8#48
    assign VSX$_14_49 = VSX$_14[49] ; // node: BUS8#49
    assign VSX$_14_50 = VSX$_14[50] ; // node: BUS8#50
    assign VSX$_14_51 = VSX$_14[51] ; // node: BUS8#51
    assign VSX$_14_52 = VSX$_14[52] ; // node: BUS8#52
    assign VSX$_14_53 = VSX$_14[53] ; // node: BUS8#53
    assign VSX$_14_54 = VSX$_14[54] ; // node: BUS8#54
    assign VSX$_14_55 = VSX$_14[55] ; // node: BUS8#55
    assign VSX$_14_56 = VSX$_14[56] ; // node: BUS8#56
    assign VSX$_14_57 = VSX$_14[57] ; // node: BUS8#57
    assign VSX$_14_58 = VSX$_14[58] ; // node: BUS8#58
    assign VSX$_14_59 = VSX$_14[59] ; // node: BUS8#59
    assign VSX$_14_60 = VSX$_14[60] ; // node: BUS8#60
    assign VSX$_14_61 = VSX$_14[61] ; // node: BUS8#61
    assign VSX$_14_62 = VSX$_14[62] ; // node: BUS8#62
    assign VSX$_14_63 = VSX$_14[63] ; // node: BUS8#63
    assign VSX$_15_0 = VSX$_15[0] ; // node: BUS9#0
    assign VSX$_15_1 = VSX$_15[1] ; // node: BUS9#1
    assign VSX$_15_2 = VSX$_15[2] ; // node: BUS9#2
    assign VSX$_15_3 = VSX$_15[3] ; // node: BUS9#3
    assign VSX$_15_4 = VSX$_15[4] ; // node: BUS9#4
    assign VSX$_15_5 = VSX$_15[5] ; // node: BUS9#5
    assign VSX$_15_6 = VSX$_15[6] ; // node: BUS9#6
    assign VSX$_15_7 = VSX$_15[7] ; // node: BUS9#7
    assign VSX$_15_8 = VSX$_15[8] ; // node: BUS9#8
    assign VSX$_15_9 = VSX$_15[9] ; // node: BUS9#9
    assign VSX$_15_10 = VSX$_15[10] ; // node: BUS9#10
    assign VSX$_15_11 = VSX$_15[11] ; // node: BUS9#11
    assign VSX$_15_12 = VSX$_15[12] ; // node: BUS9#12
    assign VSX$_15_13 = VSX$_15[13] ; // node: BUS9#13
    assign VSX$_15_14 = VSX$_15[14] ; // node: BUS9#14
    assign VSX$_15_15 = VSX$_15[15] ; // node: BUS9#15
    assign VSX$_15_16 = VSX$_15[16] ; // node: BUS9#16
    assign VSX$_15_17 = VSX$_15[17] ; // node: BUS9#17
    assign VSX$_15_18 = VSX$_15[18] ; // node: BUS9#18
    assign VSX$_15_19 = VSX$_15[19] ; // node: BUS9#19
    assign VSX$_15_20 = VSX$_15[20] ; // node: BUS9#20
    assign VSX$_15_21 = VSX$_15[21] ; // node: BUS9#21
    assign VSX$_15_22 = VSX$_15[22] ; // node: BUS9#22
    assign VSX$_15_23 = VSX$_15[23] ; // node: BUS9#23
    assign VSX$_15_24 = VSX$_15[24] ; // node: BUS9#24
    assign VSX$_15_25 = VSX$_15[25] ; // node: BUS9#25
    assign VSX$_15_26 = VSX$_15[26] ; // node: BUS9#26
    assign VSX$_15_27 = VSX$_15[27] ; // node: BUS9#27
    assign VSX$_15_28 = VSX$_15[28] ; // node: BUS9#28
    assign VSX$_15_29 = VSX$_15[29] ; // node: BUS9#29
    assign VSX$_15_30 = VSX$_15[30] ; // node: BUS9#30
    assign VSX$_15_31 = VSX$_15[31] ; // node: BUS9#31
    assign VSX$_15_32 = VSX$_15[32] ; // node: BUS9#32
    assign VSX$_15_33 = VSX$_15[33] ; // node: BUS9#33
    assign VSX$_15_34 = VSX$_15[34] ; // node: BUS9#34
    assign VSX$_15_35 = VSX$_15[35] ; // node: BUS9#35
    assign VSX$_15_36 = VSX$_15[36] ; // node: BUS9#36
    assign VSX$_15_37 = VSX$_15[37] ; // node: BUS9#37
    assign VSX$_15_38 = VSX$_15[38] ; // node: BUS9#38
    assign VSX$_15_39 = VSX$_15[39] ; // node: BUS9#39
    assign VSX$_15_40 = VSX$_15[40] ; // node: BUS9#40
    assign VSX$_15_41 = VSX$_15[41] ; // node: BUS9#41
    assign VSX$_15_42 = VSX$_15[42] ; // node: BUS9#42
    assign VSX$_15_43 = VSX$_15[43] ; // node: BUS9#43
    assign VSX$_15_44 = VSX$_15[44] ; // node: BUS9#44
    assign VSX$_15_45 = VSX$_15[45] ; // node: BUS9#45
    assign VSX$_15_46 = VSX$_15[46] ; // node: BUS9#46
    assign VSX$_15_47 = VSX$_15[47] ; // node: BUS9#47
    assign VSX$_15_48 = VSX$_15[48] ; // node: BUS9#48
    assign VSX$_15_49 = VSX$_15[49] ; // node: BUS9#49
    assign VSX$_15_50 = VSX$_15[50] ; // node: BUS9#50
    assign VSX$_15_51 = VSX$_15[51] ; // node: BUS9#51
    assign VSX$_15_52 = VSX$_15[52] ; // node: BUS9#52
    assign VSX$_15_53 = VSX$_15[53] ; // node: BUS9#53
    assign VSX$_15_54 = VSX$_15[54] ; // node: BUS9#54
    assign VSX$_15_55 = VSX$_15[55] ; // node: BUS9#55
    assign VSX$_15_56 = VSX$_15[56] ; // node: BUS9#56
    assign VSX$_15_57 = VSX$_15[57] ; // node: BUS9#57
    assign VSX$_15_58 = VSX$_15[58] ; // node: BUS9#58
    assign VSX$_15_59 = VSX$_15[59] ; // node: BUS9#59
    assign VSX$_15_60 = VSX$_15[60] ; // node: BUS9#60
    assign VSX$_15_61 = VSX$_15[61] ; // node: BUS9#61
    assign VSX$_15_62 = VSX$_15[62] ; // node: BUS9#62
    assign VSX$_15_63 = VSX$_15[63] ; // node: BUS9#63
    assign VSX$_16_0 = VSX$_16[0] ; // node: BUS10#0
    assign VSX$_16_1 = VSX$_16[1] ; // node: BUS10#1
    assign VSX$_16_2 = VSX$_16[2] ; // node: BUS10#2
    assign VSX$_16_3 = VSX$_16[3] ; // node: BUS10#3
    assign VSX$_16_4 = VSX$_16[4] ; // node: BUS10#4
    assign VSX$_16_5 = VSX$_16[5] ; // node: BUS10#5
    assign VSX$_16_6 = VSX$_16[6] ; // node: BUS10#6
    assign VSX$_16_7 = VSX$_16[7] ; // node: BUS10#7
    assign VSX$_16_8 = VSX$_16[8] ; // node: BUS10#8
    assign VSX$_16_9 = VSX$_16[9] ; // node: BUS10#9
    assign VSX$_16_10 = VSX$_16[10] ; // node: BUS10#10
    assign VSX$_16_11 = VSX$_16[11] ; // node: BUS10#11
    assign VSX$_16_12 = VSX$_16[12] ; // node: BUS10#12
    assign VSX$_16_13 = VSX$_16[13] ; // node: BUS10#13
    assign VSX$_16_14 = VSX$_16[14] ; // node: BUS10#14
    assign VSX$_16_15 = VSX$_16[15] ; // node: BUS10#15
    assign VSX$_16_16 = VSX$_16[16] ; // node: BUS10#16
    assign VSX$_16_17 = VSX$_16[17] ; // node: BUS10#17
    assign VSX$_16_18 = VSX$_16[18] ; // node: BUS10#18
    assign VSX$_16_19 = VSX$_16[19] ; // node: BUS10#19
    assign VSX$_16_20 = VSX$_16[20] ; // node: BUS10#20
    assign VSX$_16_21 = VSX$_16[21] ; // node: BUS10#21
    assign VSX$_16_22 = VSX$_16[22] ; // node: BUS10#22
    assign VSX$_16_23 = VSX$_16[23] ; // node: BUS10#23
    assign VSX$_16_24 = VSX$_16[24] ; // node: BUS10#24
    assign VSX$_16_25 = VSX$_16[25] ; // node: BUS10#25
    assign VSX$_16_26 = VSX$_16[26] ; // node: BUS10#26
    assign VSX$_16_27 = VSX$_16[27] ; // node: BUS10#27
    assign VSX$_16_28 = VSX$_16[28] ; // node: BUS10#28
    assign VSX$_16_29 = VSX$_16[29] ; // node: BUS10#29
    assign VSX$_16_30 = VSX$_16[30] ; // node: BUS10#30
    assign VSX$_16_31 = VSX$_16[31] ; // node: BUS10#31
    assign VSX$_16_32 = VSX$_16[32] ; // node: BUS10#32
    assign VSX$_16_33 = VSX$_16[33] ; // node: BUS10#33
    assign VSX$_16_34 = VSX$_16[34] ; // node: BUS10#34
    assign VSX$_16_35 = VSX$_16[35] ; // node: BUS10#35
    assign VSX$_16_36 = VSX$_16[36] ; // node: BUS10#36
    assign VSX$_16_37 = VSX$_16[37] ; // node: BUS10#37
    assign VSX$_16_38 = VSX$_16[38] ; // node: BUS10#38
    assign VSX$_16_39 = VSX$_16[39] ; // node: BUS10#39
    assign VSX$_16_40 = VSX$_16[40] ; // node: BUS10#40
    assign VSX$_16_41 = VSX$_16[41] ; // node: BUS10#41
    assign VSX$_16_42 = VSX$_16[42] ; // node: BUS10#42
    assign VSX$_16_43 = VSX$_16[43] ; // node: BUS10#43
    assign VSX$_16_44 = VSX$_16[44] ; // node: BUS10#44
    assign VSX$_16_45 = VSX$_16[45] ; // node: BUS10#45
    assign VSX$_16_46 = VSX$_16[46] ; // node: BUS10#46
    assign VSX$_16_47 = VSX$_16[47] ; // node: BUS10#47
    assign VSX$_16_48 = VSX$_16[48] ; // node: BUS10#48
    assign VSX$_16_49 = VSX$_16[49] ; // node: BUS10#49
    assign VSX$_16_50 = VSX$_16[50] ; // node: BUS10#50
    assign VSX$_16_51 = VSX$_16[51] ; // node: BUS10#51
    assign VSX$_16_52 = VSX$_16[52] ; // node: BUS10#52
    assign VSX$_16_53 = VSX$_16[53] ; // node: BUS10#53
    assign VSX$_16_54 = VSX$_16[54] ; // node: BUS10#54
    assign VSX$_16_55 = VSX$_16[55] ; // node: BUS10#55
    assign VSX$_16_56 = VSX$_16[56] ; // node: BUS10#56
    assign VSX$_16_57 = VSX$_16[57] ; // node: BUS10#57
    assign VSX$_16_58 = VSX$_16[58] ; // node: BUS10#58
    assign VSX$_16_59 = VSX$_16[59] ; // node: BUS10#59
    assign VSX$_16_60 = VSX$_16[60] ; // node: BUS10#60
    assign VSX$_16_61 = VSX$_16[61] ; // node: BUS10#61
    assign VSX$_16_62 = VSX$_16[62] ; // node: BUS10#62
    assign VSX$_16_63 = VSX$_16[63] ; // node: BUS10#63
    assign VSX$_17_0 = VSX$_17[0] ; // node: BUS11#0
    assign VSX$_17_1 = VSX$_17[1] ; // node: BUS11#1
    assign VSX$_17_2 = VSX$_17[2] ; // node: BUS11#2
    assign VSX$_17_3 = VSX$_17[3] ; // node: BUS11#3
    assign VSX$_17_4 = VSX$_17[4] ; // node: BUS11#4
    assign VSX$_17_5 = VSX$_17[5] ; // node: BUS11#5
    assign VSX$_17_6 = VSX$_17[6] ; // node: BUS11#6
    assign VSX$_17_7 = VSX$_17[7] ; // node: BUS11#7
    assign VSX$_17_8 = VSX$_17[8] ; // node: BUS11#8
    assign VSX$_17_9 = VSX$_17[9] ; // node: BUS11#9
    assign VSX$_17_10 = VSX$_17[10] ; // node: BUS11#10
    assign VSX$_17_11 = VSX$_17[11] ; // node: BUS11#11
    assign VSX$_17_12 = VSX$_17[12] ; // node: BUS11#12
    assign VSX$_17_13 = VSX$_17[13] ; // node: BUS11#13
    assign VSX$_17_14 = VSX$_17[14] ; // node: BUS11#14
    assign VSX$_17_15 = VSX$_17[15] ; // node: BUS11#15
    assign VSX$_17_16 = VSX$_17[16] ; // node: BUS11#16
    assign VSX$_17_17 = VSX$_17[17] ; // node: BUS11#17
    assign VSX$_17_18 = VSX$_17[18] ; // node: BUS11#18
    assign VSX$_17_19 = VSX$_17[19] ; // node: BUS11#19
    assign VSX$_17_20 = VSX$_17[20] ; // node: BUS11#20
    assign VSX$_17_21 = VSX$_17[21] ; // node: BUS11#21
    assign VSX$_17_22 = VSX$_17[22] ; // node: BUS11#22
    assign VSX$_17_23 = VSX$_17[23] ; // node: BUS11#23
    assign VSX$_17_24 = VSX$_17[24] ; // node: BUS11#24
    assign VSX$_17_25 = VSX$_17[25] ; // node: BUS11#25
    assign VSX$_17_26 = VSX$_17[26] ; // node: BUS11#26
    assign VSX$_17_27 = VSX$_17[27] ; // node: BUS11#27
    assign VSX$_17_28 = VSX$_17[28] ; // node: BUS11#28
    assign VSX$_17_29 = VSX$_17[29] ; // node: BUS11#29
    assign VSX$_17_30 = VSX$_17[30] ; // node: BUS11#30
    assign VSX$_17_31 = VSX$_17[31] ; // node: BUS11#31
    assign VSX$_17_32 = VSX$_17[32] ; // node: BUS11#32
    assign VSX$_17_33 = VSX$_17[33] ; // node: BUS11#33
    assign VSX$_17_34 = VSX$_17[34] ; // node: BUS11#34
    assign VSX$_17_35 = VSX$_17[35] ; // node: BUS11#35
    assign VSX$_17_36 = VSX$_17[36] ; // node: BUS11#36
    assign VSX$_17_37 = VSX$_17[37] ; // node: BUS11#37
    assign VSX$_17_38 = VSX$_17[38] ; // node: BUS11#38
    assign VSX$_17_39 = VSX$_17[39] ; // node: BUS11#39
    assign VSX$_17_40 = VSX$_17[40] ; // node: BUS11#40
    assign VSX$_17_41 = VSX$_17[41] ; // node: BUS11#41
    assign VSX$_17_42 = VSX$_17[42] ; // node: BUS11#42
    assign VSX$_17_43 = VSX$_17[43] ; // node: BUS11#43
    assign VSX$_17_44 = VSX$_17[44] ; // node: BUS11#44
    assign VSX$_17_45 = VSX$_17[45] ; // node: BUS11#45
    assign VSX$_17_46 = VSX$_17[46] ; // node: BUS11#46
    assign VSX$_17_47 = VSX$_17[47] ; // node: BUS11#47
    assign VSX$_17_48 = VSX$_17[48] ; // node: BUS11#48
    assign VSX$_17_49 = VSX$_17[49] ; // node: BUS11#49
    assign VSX$_17_50 = VSX$_17[50] ; // node: BUS11#50
    assign VSX$_17_51 = VSX$_17[51] ; // node: BUS11#51
    assign VSX$_17_52 = VSX$_17[52] ; // node: BUS11#52
    assign VSX$_17_53 = VSX$_17[53] ; // node: BUS11#53
    assign VSX$_17_54 = VSX$_17[54] ; // node: BUS11#54
    assign VSX$_17_55 = VSX$_17[55] ; // node: BUS11#55
    assign VSX$_17_56 = VSX$_17[56] ; // node: BUS11#56
    assign VSX$_17_57 = VSX$_17[57] ; // node: BUS11#57
    assign VSX$_17_58 = VSX$_17[58] ; // node: BUS11#58
    assign VSX$_17_59 = VSX$_17[59] ; // node: BUS11#59
    assign VSX$_17_60 = VSX$_17[60] ; // node: BUS11#60
    assign VSX$_17_61 = VSX$_17[61] ; // node: BUS11#61
    assign VSX$_17_62 = VSX$_17[62] ; // node: BUS11#62
    assign VSX$_17_63 = VSX$_17[63] ; // node: BUS11#63
    assign VSX$_18_0 = VSX$_18[0] ; // node: BUS12#0
    assign VSX$_18_1 = VSX$_18[1] ; // node: BUS12#1
    assign VSX$_18_2 = VSX$_18[2] ; // node: BUS12#2
    assign VSX$_18_3 = VSX$_18[3] ; // node: BUS12#3
    assign VSX$_18_4 = VSX$_18[4] ; // node: BUS12#4
    assign VSX$_18_5 = VSX$_18[5] ; // node: BUS12#5
    assign VSX$_18_6 = VSX$_18[6] ; // node: BUS12#6
    assign VSX$_18_7 = VSX$_18[7] ; // node: BUS12#7
    assign VSX$_18_8 = VSX$_18[8] ; // node: BUS12#8
    assign VSX$_18_9 = VSX$_18[9] ; // node: BUS12#9
    assign VSX$_18_10 = VSX$_18[10] ; // node: BUS12#10
    assign VSX$_18_11 = VSX$_18[11] ; // node: BUS12#11
    assign VSX$_18_12 = VSX$_18[12] ; // node: BUS12#12
    assign VSX$_18_13 = VSX$_18[13] ; // node: BUS12#13
    assign VSX$_18_14 = VSX$_18[14] ; // node: BUS12#14
    assign VSX$_18_15 = VSX$_18[15] ; // node: BUS12#15
    assign VSX$_18_16 = VSX$_18[16] ; // node: BUS12#16
    assign VSX$_18_17 = VSX$_18[17] ; // node: BUS12#17
    assign VSX$_18_18 = VSX$_18[18] ; // node: BUS12#18
    assign VSX$_18_19 = VSX$_18[19] ; // node: BUS12#19
    assign VSX$_18_20 = VSX$_18[20] ; // node: BUS12#20
    assign VSX$_18_21 = VSX$_18[21] ; // node: BUS12#21
    assign VSX$_18_22 = VSX$_18[22] ; // node: BUS12#22
    assign VSX$_18_23 = VSX$_18[23] ; // node: BUS12#23
    assign VSX$_18_24 = VSX$_18[24] ; // node: BUS12#24
    assign VSX$_18_25 = VSX$_18[25] ; // node: BUS12#25
    assign VSX$_18_26 = VSX$_18[26] ; // node: BUS12#26
    assign VSX$_18_27 = VSX$_18[27] ; // node: BUS12#27
    assign VSX$_18_28 = VSX$_18[28] ; // node: BUS12#28
    assign VSX$_18_29 = VSX$_18[29] ; // node: BUS12#29
    assign VSX$_18_30 = VSX$_18[30] ; // node: BUS12#30
    assign VSX$_18_31 = VSX$_18[31] ; // node: BUS12#31
    assign VSX$_18_32 = VSX$_18[32] ; // node: BUS12#32
    assign VSX$_18_33 = VSX$_18[33] ; // node: BUS12#33
    assign VSX$_18_34 = VSX$_18[34] ; // node: BUS12#34
    assign VSX$_18_35 = VSX$_18[35] ; // node: BUS12#35
    assign VSX$_18_36 = VSX$_18[36] ; // node: BUS12#36
    assign VSX$_18_37 = VSX$_18[37] ; // node: BUS12#37
    assign VSX$_18_38 = VSX$_18[38] ; // node: BUS12#38
    assign VSX$_18_39 = VSX$_18[39] ; // node: BUS12#39
    assign VSX$_18_40 = VSX$_18[40] ; // node: BUS12#40
    assign VSX$_18_41 = VSX$_18[41] ; // node: BUS12#41
    assign VSX$_18_42 = VSX$_18[42] ; // node: BUS12#42
    assign VSX$_18_43 = VSX$_18[43] ; // node: BUS12#43
    assign VSX$_18_44 = VSX$_18[44] ; // node: BUS12#44
    assign VSX$_18_45 = VSX$_18[45] ; // node: BUS12#45
    assign VSX$_18_46 = VSX$_18[46] ; // node: BUS12#46
    assign VSX$_18_47 = VSX$_18[47] ; // node: BUS12#47
    assign VSX$_18_48 = VSX$_18[48] ; // node: BUS12#48
    assign VSX$_18_49 = VSX$_18[49] ; // node: BUS12#49
    assign VSX$_18_50 = VSX$_18[50] ; // node: BUS12#50
    assign VSX$_18_51 = VSX$_18[51] ; // node: BUS12#51
    assign VSX$_18_52 = VSX$_18[52] ; // node: BUS12#52
    assign VSX$_18_53 = VSX$_18[53] ; // node: BUS12#53
    assign VSX$_18_54 = VSX$_18[54] ; // node: BUS12#54
    assign VSX$_18_55 = VSX$_18[55] ; // node: BUS12#55
    assign VSX$_18_56 = VSX$_18[56] ; // node: BUS12#56
    assign VSX$_18_57 = VSX$_18[57] ; // node: BUS12#57
    assign VSX$_18_58 = VSX$_18[58] ; // node: BUS12#58
    assign VSX$_18_59 = VSX$_18[59] ; // node: BUS12#59
    assign VSX$_18_60 = VSX$_18[60] ; // node: BUS12#60
    assign VSX$_18_61 = VSX$_18[61] ; // node: BUS12#61
    assign VSX$_18_62 = VSX$_18[62] ; // node: BUS12#62
    assign VSX$_18_63 = VSX$_18[63] ; // node: BUS12#63
    assign VSX$_19_0 = VSX$_19[0] ; // node: BUS13#0
    assign VSX$_19_1 = VSX$_19[1] ; // node: BUS13#1
    assign VSX$_19_2 = VSX$_19[2] ; // node: BUS13#2
    assign VSX$_20_0 = VSX$_20[0] ; // node: BUS14#0
    assign VSX$_20_1 = VSX$_20[1] ; // node: BUS14#1
    assign VSX$_20_2 = VSX$_20[2] ; // node: BUS14#2
    assign VSX$_20_3 = VSX$_20[3] ; // node: BUS14#3
    assign VSX$_20_4 = VSX$_20[4] ; // node: BUS14#4
    assign VSX$_20_5 = VSX$_20[5] ; // node: BUS14#5
    assign VSX$_20_6 = VSX$_20[6] ; // node: BUS14#6
    assign VSX$_20_7 = VSX$_20[7] ; // node: BUS14#7
    assign VSX$_20_8 = VSX$_20[8] ; // node: BUS14#8
    assign VSX$_20_9 = VSX$_20[9] ; // node: BUS14#9
    assign VSX$_20_10 = VSX$_20[10] ; // node: BUS14#10
    assign VSX$_20_11 = VSX$_20[11] ; // node: BUS14#11
    assign VSX$_20_12 = VSX$_20[12] ; // node: BUS14#12
    assign VSX$_20_13 = VSX$_20[13] ; // node: BUS14#13
    assign VSX$_20_14 = VSX$_20[14] ; // node: BUS14#14
    assign VSX$_20_15 = VSX$_20[15] ; // node: BUS14#15
    assign VSX$_21_0 = VSX$_21[0] ; // node: BUS15#0
    assign VSX$_21_1 = VSX$_21[1] ; // node: BUS15#1
    assign VSX$_21_2 = VSX$_21[2] ; // node: BUS15#2
    assign VSX$_21_3 = VSX$_21[3] ; // node: BUS15#3
    assign VSX$_21_4 = VSX$_21[4] ; // node: BUS15#4
    assign VSX$_21_5 = VSX$_21[5] ; // node: BUS15#5
    assign VSX$_21_6 = VSX$_21[6] ; // node: BUS15#6
    assign VSX$_21_7 = VSX$_21[7] ; // node: BUS15#7
    assign VSX$_21_8 = VSX$_21[8] ; // node: BUS15#8
    assign VSX$_21_9 = VSX$_21[9] ; // node: BUS15#9
    assign VSX$_21_10 = VSX$_21[10] ; // node: BUS15#10
    assign VSX$_21_11 = VSX$_21[11] ; // node: BUS15#11
    assign VSX$_21_12 = VSX$_21[12] ; // node: BUS15#12
    assign VSX$_21_13 = VSX$_21[13] ; // node: BUS15#13
    assign VSX$_21_14 = VSX$_21[14] ; // node: BUS15#14
    assign VSX$_21_15 = VSX$_21[15] ; // node: BUS15#15
    assign VSX$_22_0 = VSX$_22[0] ; // node: BUS16#0
    assign VSX$_22_1 = VSX$_22[1] ; // node: BUS16#1
    assign VSX$_22_2 = VSX$_22[2] ; // node: BUS16#2
    assign VSX$_22_3 = VSX$_22[3] ; // node: BUS16#3
    assign VSX$_22_4 = VSX$_22[4] ; // node: BUS16#4
    assign VSX$_22_5 = VSX$_22[5] ; // node: BUS16#5
    assign VSX$_22_6 = VSX$_22[6] ; // node: BUS16#6
    assign VSX$_22_7 = VSX$_22[7] ; // node: BUS16#7
    assign VSX$_22_8 = VSX$_22[8] ; // node: BUS16#8
    assign VSX$_22_9 = VSX$_22[9] ; // node: BUS16#9
    assign VSX$_22_10 = VSX$_22[10] ; // node: BUS16#10
    assign VSX$_22_11 = VSX$_22[11] ; // node: BUS16#11
    assign VSX$_22_12 = VSX$_22[12] ; // node: BUS16#12
    assign VSX$_22_13 = VSX$_22[13] ; // node: BUS16#13
    assign VSX$_22_14 = VSX$_22[14] ; // node: BUS16#14
    assign VSX$_22_15 = VSX$_22[15] ; // node: BUS16#15
    assign VSX$_22_16 = VSX$_22[16] ; // node: BUS16#16
    assign VSX$_22_17 = VSX$_22[17] ; // node: BUS16#17
    assign VSX$_22_18 = VSX$_22[18] ; // node: BUS16#18
    assign VSX$_22_19 = VSX$_22[19] ; // node: BUS16#19
    assign VSX$_22_20 = VSX$_22[20] ; // node: BUS16#20
    assign VSX$_22_21 = VSX$_22[21] ; // node: BUS16#21
    assign VSX$_22_22 = VSX$_22[22] ; // node: BUS16#22
    assign VSX$_22_23 = VSX$_22[23] ; // node: BUS16#23
    assign VSX$_22_24 = VSX$_22[24] ; // node: BUS16#24
    assign VSX$_22_25 = VSX$_22[25] ; // node: BUS16#25
    assign VSX$_22_26 = VSX$_22[26] ; // node: BUS16#26
    assign VSX$_22_27 = VSX$_22[27] ; // node: BUS16#27
    assign VSX$_22_28 = VSX$_22[28] ; // node: BUS16#28
    assign VSX$_22_29 = VSX$_22[29] ; // node: BUS16#29
    assign VSX$_22_30 = VSX$_22[30] ; // node: BUS16#30
    assign VSX$_22_31 = VSX$_22[31] ; // node: BUS16#31
    assign VSX$_22_32 = VSX$_22[32] ; // node: BUS16#32
    assign VSX$_22_33 = VSX$_22[33] ; // node: BUS16#33
    assign VSX$_22_34 = VSX$_22[34] ; // node: BUS16#34
    assign VSX$_22_35 = VSX$_22[35] ; // node: BUS16#35
    assign VSX$_22_36 = VSX$_22[36] ; // node: BUS16#36
    assign VSX$_22_37 = VSX$_22[37] ; // node: BUS16#37
    assign VSX$_22_38 = VSX$_22[38] ; // node: BUS16#38
    assign VSX$_22_39 = VSX$_22[39] ; // node: BUS16#39
    assign VSX$_22_40 = VSX$_22[40] ; // node: BUS16#40
    assign VSX$_22_41 = VSX$_22[41] ; // node: BUS16#41
    assign VSX$_22_42 = VSX$_22[42] ; // node: BUS16#42
    assign VSX$_22_43 = VSX$_22[43] ; // node: BUS16#43
    assign VSX$_22_44 = VSX$_22[44] ; // node: BUS16#44
    assign VSX$_22_45 = VSX$_22[45] ; // node: BUS16#45
    assign VSX$_22_46 = VSX$_22[46] ; // node: BUS16#46
    assign VSX$_22_47 = VSX$_22[47] ; // node: BUS16#47
    assign VSX$_22_48 = VSX$_22[48] ; // node: BUS16#48
    assign VSX$_22_49 = VSX$_22[49] ; // node: BUS16#49
    assign VSX$_22_50 = VSX$_22[50] ; // node: BUS16#50
    assign VSX$_22_51 = VSX$_22[51] ; // node: BUS16#51
    assign VSX$_22_52 = VSX$_22[52] ; // node: BUS16#52
    assign VSX$_22_53 = VSX$_22[53] ; // node: BUS16#53
    assign VSX$_22_54 = VSX$_22[54] ; // node: BUS16#54
    assign VSX$_22_55 = VSX$_22[55] ; // node: BUS16#55
    assign VSX$_22_56 = VSX$_22[56] ; // node: BUS16#56
    assign VSX$_22_57 = VSX$_22[57] ; // node: BUS16#57
    assign VSX$_22_58 = VSX$_22[58] ; // node: BUS16#58
    assign VSX$_22_59 = VSX$_22[59] ; // node: BUS16#59
    assign VSX$_22_60 = VSX$_22[60] ; // node: BUS16#60
    assign VSX$_22_61 = VSX$_22[61] ; // node: BUS16#61
    assign VSX$_22_62 = VSX$_22[62] ; // node: BUS16#62
    assign VSX$_22_63 = VSX$_22[63] ; // node: BUS16#63
    fixed60_pll_phase40k_candidate  U1(.ac_input(VSX$_0), .clk(VSX$_1), .reset(VSX$_2), .phase_ok(VSX$_3), .locked(VSX$_4), .hold_60hz(VSX$_5), .pll_phase_5k(VSX$_6), .pll_phase_post_5k(VSX$_7), .pll_step_q(VSX$_8), .slow_error(VSX$_9), .input_valid(VSX$_10), .pll_update(VSX$_11), .theta_40k(VSX$_12), .base_step_40k(VSX$_13), .phase_error(VSX$_14), .correction_quotient(VSX$_15), .correction_remainder(VSX$_16), .correction_applied(VSX$_17), .correction_remaining(VSX$_18), .substep(VSX$_19), .held_ref(VSX$_20), .extrapolated_ref(VSX$_21), .anchor_residual(VSX$_22), .interval_complete(VSX$_23) ); // SIMetrix netlist ref: U5 

endmodule
