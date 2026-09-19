// inverter_controller.v
//
// SOGI core (bit-exact per project spec sections E/F) running at 40kHz
// sample rate with a /16 decimation to the 2.5kHz SOGI/PLL update rate.
//
// Three DEBUG-ONLY mirror outputs are added at the bottom of the update
// block for SIMPLIS analog observation (view -> Bus Ripper -> bus_to_voltage
// -> Voltage Probe), 1 count = 1 mV:
//     sogi_alpha_view = va_new >>> 12   (Q12 -> integer counts)
//     sogi_beta_view  = vb_new >>> 12   (Q12 -> integer counts)
//     sogi_input_view = v_in_count      (already integer counts)
//
// These mirror registers are pure observation taps. They are written from
// va_new / vb_new / v_in_count AFTER those values have been computed by
// the unmodified SOGI difference equations below; they do not feed back
// into va_n1/va_n2/vb_n1/vb_n2/vin_n1/vin_n2 or any other state, and
// nothing downstream (PLL, current loop, PWM — none of which exist in
// this module yet) reads them.
//
// SOGI coefficients, the Q30_Round definition, and the difference
// equations are exactly as specified and must not be altered here.

module inverter_controller(
    input                      clk,        // 40 kHz sample clock
    input                      rst,        // synchronous, active high
    input       [11:0]         adc_grid,   // AN1 raw ADC count, unsigned 0..4095

    // ---- DEBUG MIRROR ONLY — not part of any control path ----
    output reg signed [15:0]   sogi_alpha_view,
    output reg signed [15:0]   sogi_beta_view,
    output reg signed [15:0]   sogi_input_view
);

    // -----------------------------------------------------------------
    // Fixed 60 Hz SOGI coefficients (Q30), per spec section E.
    // Declared at 64-bit width so every multiply below is performed at
    // full 64-bit precision without relying on implicit context-based
    // sign extension of narrower operands.
    // -----------------------------------------------------------------
    localparam signed [63:0] SOGI_A1  = -64'sd1919669515;
    localparam signed [63:0] SOGI_A2  =  64'sd867878708;
    localparam signed [63:0] SOGI_B0  =  64'sd102931558;
    localparam signed [63:0] SOGI_QB0 =  64'sd7760857;
    localparam signed [63:0] SOGI_QB1 =  64'sd15521713;
    localparam signed [63:0] SOGI_QB2 =  64'sd7760857;

    localparam signed [15:0] GRID_ADC_CENTER = 16'sd1986;

    localparam [3:0] DECIM_MAX = 4'd15; // 40kHz / 16 = 2.5kHz update

    // -----------------------------------------------------------------
    // State (Q12), per spec section E state list.
    // -----------------------------------------------------------------
    reg signed [31:0] vin_n1, vin_n2;
    reg signed [31:0] va_n1,  va_n2;
    reg signed [31:0] vb_n1,  vb_n2;

    reg        [3:0]  decim_cnt;

    // -----------------------------------------------------------------
    // Per-tick working variables (blocking, computed fresh each decim tick)
    // -----------------------------------------------------------------
    reg signed [15:0] v_in_count;
    reg signed [31:0] v_in_count_ext;
    reg signed [31:0] v_in_q;

    reg signed [63:0] v_in_q_64, vin_n1_64, vin_n2_64;
    reg signed [63:0] va_n1_64,  va_n2_64;
    reg signed [63:0] vb_n1_64,  vb_n2_64;

    reg signed [63:0] acc_va, acc_vb;
    reg signed [31:0] va_new, vb_new;

    // -----------------------------------------------------------------
    // Q30_Round, exactly per spec section E:
    //   x >= 0 :  (x + (1<<29)) >>> 30
    //   x <  0 : -(((-x) + (1<<29)) >>> 30)
    // -----------------------------------------------------------------
    function signed [31:0] q30_round;
        input signed [63:0] x;
        reg   signed [63:0] xa;
        begin
            if (x >= 0)
                q30_round = (x + 64'sd536870912) >>> 30;
            else begin
                xa = -x;
                q30_round = -((xa + 64'sd536870912) >>> 30);
            end
        end
    endfunction

    initial begin
        decim_cnt        = 4'd0;
        vin_n1            = 32'sd0;
        vin_n2            = 32'sd0;
        va_n1             = 32'sd0;
        va_n2             = 32'sd0;
        vb_n1             = 32'sd0;
        vb_n2             = 32'sd0;
        sogi_alpha_view   = 16'sd0;
        sogi_beta_view    = 16'sd0;
        sogi_input_view   = 16'sd0;
    end

    always @(posedge clk) begin
        if (rst) begin
            decim_cnt        <= 4'd0;
            vin_n1            <= 32'sd0;
            vin_n2            <= 32'sd0;
            va_n1             <= 32'sd0;
            va_n2             <= 32'sd0;
            vb_n1             <= 32'sd0;
            vb_n2             <= 32'sd0;
            sogi_alpha_view   <= 16'sd0;
            sogi_beta_view    <= 16'sd0;
            sogi_input_view   <= 16'sd0;
        end
        else if (decim_cnt == DECIM_MAX) begin
            decim_cnt <= 4'd0;

            // ---- grid input, this tick ----
            v_in_count     = $signed({4'b0000, adc_grid}) - GRID_ADC_CENTER;
            v_in_count_ext = {{16{v_in_count[15]}}, v_in_count};
            v_in_q         = v_in_count_ext <<< 12;

            // ---- 64-bit sign-extended operands for the multiplies ----
            v_in_q_64 = {{32{v_in_q[31]}}, v_in_q};
            vin_n1_64 = {{32{vin_n1[31]}}, vin_n1};
            vin_n2_64 = {{32{vin_n2[31]}}, vin_n2};
            va_n1_64  = {{32{va_n1[31]}},  va_n1};
            va_n2_64  = {{32{va_n2[31]}},  va_n2};
            vb_n1_64  = {{32{vb_n1[31]}},  vb_n1};
            vb_n2_64  = {{32{vb_n2[31]}},  vb_n2};

            // ---- SOGI difference equations, exactly per spec section E ----
            acc_va = (SOGI_B0  * v_in_q_64) - (SOGI_B0  * vin_n2_64)
                   - (SOGI_A1  * va_n1_64)  - (SOGI_A2  * va_n2_64);

            acc_vb = (SOGI_QB0 * v_in_q_64) + (SOGI_QB1 * vin_n1_64) + (SOGI_QB2 * vin_n2_64)
                   - (SOGI_A1  * vb_n1_64)  - (SOGI_A2  * vb_n2_64);

            va_new = q30_round(acc_va);
            vb_new = q30_round(acc_vb);

            // ---- state update order, exactly per spec section E ----
            vin_n2 <= vin_n1;
            vin_n1 <= v_in_q;

            va_n2 <= va_n1;
            va_n1 <= va_new;

            vb_n2 <= vb_n1;
            vb_n1 <= vb_new;

            // ============ DEBUG MIRROR ONLY (analog observation) ============
            // 1 count = 1 mV at the SIMPLIS bus_to_voltage wrapper.
            sogi_alpha_view <= va_new >>> 12;
            sogi_beta_view  <= vb_new >>> 12;
            sogi_input_view <= v_in_count;
            // ==================================================================
        end
        else begin
            decim_cnt <= decim_cnt + 4'd1;
        end
    end

endmodule
