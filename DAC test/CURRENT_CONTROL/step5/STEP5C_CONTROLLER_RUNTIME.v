`timescale 1ns/1ps
// Pure integer controller. reset_sample is replay bookkeeping, applied before
// processing the same sample (preserves independent Step 5B vector cases).
module STEP5C_CONTROLLER_RUNTIME(
    input clk_40k, input reset_sample, input boost_mode,
    input signed [31:0] e, input signed [31:0] Boost_PWM_counts,
    output reg signed [63:0] acc, output reg signed [63:0] raw,
    output reg signed [31:0] limited, finalDuty, effective_correction,
    output reg signed [31:0] x1, x2, y1, y2
);
    initial begin
        acc=0; raw=0; limited=0; finalDuty=0; effective_correction=0;
        x1=0; x2=0; y1=0; y2=0;
    end
    always @(posedge clk_40k) begin
        if (reset_sample) begin x1=0; x2=0; y1=0; y2=0; end
        if (boost_mode) begin
            acc = 64'sd26000 * $signed(e)
                - 64'sd32000 * $signed(x1)
                + 64'sd9640 * $signed(x2)
                + 64'sd30010 * $signed(y1)
                + 64'sd2668 * $signed(y2);
            raw = $signed(acc) >>> 15;
            if (raw > 64'sd2500) limited=32'sd2500;
            else if (raw < -64'sd2500) limited=-32'sd2500;
            else limited=raw;
            finalDuty=Boost_PWM_counts+limited;
            if (finalDuty>32'sd10625) finalDuty=32'sd10625;
            else if (finalDuty<0) finalDuty=0;
            effective_correction=finalDuty-Boost_PWM_counts;
            x2=x1; x1=e; y2=y1; y1=effective_correction;
        end else begin
            acc=0; raw=0; limited=0; finalDuty=0; effective_correction=0;
            x1=0; x2=0; y1=0; y2=0;
        end
    end
endmodule
