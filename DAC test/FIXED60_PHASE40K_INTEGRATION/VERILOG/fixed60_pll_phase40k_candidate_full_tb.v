`timescale 1ns / 1ps

module fixed60_pll_phase40k_candidate_full_tb;
    reg [11:0] ac_input;
    reg clk;
    reg reset;

    wire phase_ok, locked, hold_60hz;
    wire [29:0] pll_phase_5k, pll_phase_post_5k;
    wire signed [31:0] pll_step_q, slow_error;
    wire input_valid, pll_update;
    wire [32:0] theta_40k;
    wire signed [63:0] base_step_40k;
    wire signed [63:0] phase_error;
    wire signed [63:0] correction_quotient;
    wire signed [63:0] correction_remainder;
    wire signed [63:0] correction_applied;
    wire signed [63:0] correction_remaining;
    wire [2:0] substep;
    wire signed [15:0] held_ref, extrapolated_ref;
    wire signed [63:0] anchor_residual;
    wire interval_complete;

    fixed60_pll_phase40k_candidate dut (
        .ac_input(ac_input),
        .clk(clk),
        .reset(reset),
        .phase_ok(phase_ok),
        .locked(locked),
        .hold_60hz(hold_60hz),
        .pll_phase_5k(pll_phase_5k),
        .pll_phase_post_5k(pll_phase_post_5k),
        .pll_step_q(pll_step_q),
        .slow_error(slow_error),
        .input_valid(input_valid),
        .pll_update(pll_update),
        .theta_40k(theta_40k),
        .base_step_40k(base_step_40k),
        .phase_error(phase_error),
        .correction_quotient(correction_quotient),
        .correction_remainder(correction_remainder),
        .correction_applied(correction_applied),
        .correction_remaining(correction_remaining),
        .substep(substep),
        .held_ref(held_ref),
        .extrapolated_ref(extrapolated_ref),
        .anchor_residual(anchor_residual),
        .interval_complete(interval_complete)
    );

    localparam real ADC_FS = 40000.0;
    localparam int ADC_CENTER = 12'd2048;
    localparam int ADC_AMP = 12'd1860;
    localparam integer SAMPLE_COUNT = 40000 * 2; // 2 seconds, short deterministic trace

    integer sample_idx;
    integer file_id;

    function automatic integer sin_q12_input;
        input integer idx;
        real t;
        real x;
        begin
            t = idx / ADC_FS;
            x = ADC_CENTER + ADC_AMP * $sin(2.0 * 3.141592653589793 * 60.0 * t);
            sin_q12_input = $rtoi(x);
        end
    endfunction

    initial begin
        clk = 1'b0;
        reset = 1'b1;
        ac_input = 12'd2048;
        sample_idx = 0;

        file_id = $fopen("fixed60_pll_phase40k_candidate_full_trace.csv", "w");
        $fwrite(file_id, "sample,ac_input,phase_ok,locked,hold_60hz,pll_phase_5k,pll_phase_post_5k,pll_step_q,theta_40k,base_step_40k,phase_error,correction_quotient,correction_remainder,correction_applied,substep,held_ref,extrapolated_ref\n");

        repeat (4) @(posedge clk);
        reset = 1'b0;

        while (sample_idx < SAMPLE_COUNT) begin
            ac_input = sin_q12_input(sample_idx);
            @(posedge clk);
            sample_idx = sample_idx + 1;

            // This is a verification scaffold for the full candidate.
            // Runtime comparison against the MATLAB golden vector is pending actual HDL execution.
            $fwrite(file_id, "%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d\n",
                sample_idx,
                ac_input,
                phase_ok,
                locked,
                hold_60hz,
                pll_phase_5k,
                pll_phase_post_5k,
                pll_step_q,
                theta_40k,
                base_step_40k,
                phase_error,
                correction_quotient,
                correction_remainder,
                correction_applied,
                substep,
                held_ref,
                extrapolated_ref);
        end

        $fclose(file_id);
        $display("full candidate verification scaffold generated; simulator execution pending");
        $finish;
    end

    always #12500 clk = ~clk; // 40 kHz clock = 25 us period
endmodule
