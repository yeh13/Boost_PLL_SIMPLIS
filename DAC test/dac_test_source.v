// dac_test_source.v
//
// Minimal Verilog-HDL (VSXA) test module.
// On every rising edge of clk, outputs the next value in a fixed
// 7-step signed 16-bit sequence:
//     0, 100, 500, 1000, -500, -1000, 0, (repeat)
//
// Drive clk from a SIMPLIS pulse/clock source connected to this VSXA
// instance's clk pin (that connection is automatically bridged to an
// analog node by VSXA since the source is a non-Verilog device).
//
// Verilog-2001 syntax only, per project constraint J.1.

module dac_test_source(
    input  clk,
    output reg signed [15:0] test_value
);

    reg [2:0] step;

    initial begin
        step       = 3'd0;
        test_value = 16'sd0;
    end

    always @(posedge clk) begin
        case (step)
            3'd0: test_value <= 16'sd0;
            3'd1: test_value <= 16'sd100;
            3'd2: test_value <= 16'sd500;
            3'd3: test_value <= 16'sd1000;
            3'd4: test_value <= -16'sd500;
            3'd5: test_value <= -16'sd1000;
            3'd6: test_value <= 16'sd0;
            default: test_value <= 16'sd0;
        endcase

        if (step == 3'd6)
            step <= 3'd0;
        else
            step <= step + 3'd1;
    end

endmodule
