`timescale 1ns/1ps
`include "fixed60_pll_phase40k_debug_wrapper.v"

module fixed60_pll_phase40k_debug_tb;
    reg [11:0] ac_input;
    reg clk, reset;
    wire direct_phase_ok;
    wire wrapped_phase_ok;
    wire direct_locked;
    wire wrapped_locked;
    wire direct_hold_60hz;
    wire wrapped_hold_60hz;
    wire [29:0] direct_pll_phase_5k;
    wire [29:0] wrapped_pll_phase_5k;
    wire [29:0] direct_pll_phase_post_5k;
    wire [29:0] wrapped_pll_phase_post_5k;
    wire signed [31:0] direct_pll_step_q;
    wire signed [31:0] wrapped_pll_step_q;
    wire signed [31:0] direct_slow_error;
    wire signed [31:0] wrapped_slow_error;
    wire direct_input_valid;
    wire wrapped_input_valid;
    wire direct_pll_update;
    wire wrapped_pll_update;
    wire [32:0] direct_theta_40k;
    wire [32:0] wrapped_theta_40k;
    wire signed [63:0] direct_base_step_40k;
    wire signed [63:0] wrapped_base_step_40k;
    wire signed [63:0] direct_phase_error;
    wire signed [63:0] wrapped_phase_error;
    wire signed [63:0] direct_correction_quotient;
    wire signed [63:0] wrapped_correction_quotient;
    wire signed [63:0] direct_correction_remainder;
    wire signed [63:0] wrapped_correction_remainder;
    wire signed [63:0] direct_correction_applied;
    wire signed [63:0] wrapped_correction_applied;
    wire signed [63:0] direct_correction_remaining;
    wire signed [63:0] wrapped_correction_remaining;
    wire [2:0] direct_substep;
    wire [2:0] wrapped_substep;
    wire signed [15:0] direct_held_ref;
    wire signed [15:0] wrapped_held_ref;
    wire signed [15:0] direct_extrapolated_ref;
    wire signed [15:0] wrapped_extrapolated_ref;
    wire signed [63:0] direct_anchor_residual;
    wire signed [63:0] wrapped_anchor_residual;
    wire direct_interval_complete;
    wire wrapped_interval_complete;
    wire [1535:0] debug_values;
    wire debug_values_known;
    wire [645:0] direct_outputs;
    wire [645:0] wrapped_outputs;
    integer sample_index, failed_checks, checked_samples;
    real frequency_hz, sample_time, adc_sample;
    assign direct_outputs = {direct_phase_ok, direct_locked, direct_hold_60hz, direct_pll_phase_5k, direct_pll_phase_post_5k, direct_pll_step_q, direct_slow_error, direct_input_valid, direct_pll_update, direct_theta_40k, direct_base_step_40k, direct_phase_error, direct_correction_quotient, direct_correction_remainder, direct_correction_applied, direct_correction_remaining, direct_substep, direct_held_ref, direct_extrapolated_ref, direct_anchor_residual, direct_interval_complete};
    assign wrapped_outputs = {wrapped_phase_ok, wrapped_locked, wrapped_hold_60hz, wrapped_pll_phase_5k, wrapped_pll_phase_post_5k, wrapped_pll_step_q, wrapped_slow_error, wrapped_input_valid, wrapped_pll_update, wrapped_theta_40k, wrapped_base_step_40k, wrapped_phase_error, wrapped_correction_quotient, wrapped_correction_remainder, wrapped_correction_applied, wrapped_correction_remaining, wrapped_substep, wrapped_held_ref, wrapped_extrapolated_ref, wrapped_anchor_residual, wrapped_interval_complete};

    fixed60_pll_phase40k_candidate direct(
        .ac_input(ac_input),
        .clk(clk),
        .reset(reset),
        .phase_ok(direct_phase_ok),
        .locked(direct_locked),
        .hold_60hz(direct_hold_60hz),
        .pll_phase_5k(direct_pll_phase_5k),
        .pll_phase_post_5k(direct_pll_phase_post_5k),
        .pll_step_q(direct_pll_step_q),
        .slow_error(direct_slow_error),
        .input_valid(direct_input_valid),
        .pll_update(direct_pll_update),
        .theta_40k(direct_theta_40k),
        .base_step_40k(direct_base_step_40k),
        .phase_error(direct_phase_error),
        .correction_quotient(direct_correction_quotient),
        .correction_remainder(direct_correction_remainder),
        .correction_applied(direct_correction_applied),
        .correction_remaining(direct_correction_remaining),
        .substep(direct_substep),
        .held_ref(direct_held_ref),
        .extrapolated_ref(direct_extrapolated_ref),
        .anchor_residual(direct_anchor_residual),
        .interval_complete(direct_interval_complete)
    );
    fixed60_pll_phase40k_debug_wrapper wrapped(
        .ac_input(ac_input),
        .clk(clk),
        .reset(reset),
        .phase_ok(wrapped_phase_ok),
        .locked(wrapped_locked),
        .hold_60hz(wrapped_hold_60hz),
        .pll_phase_5k(wrapped_pll_phase_5k),
        .pll_phase_post_5k(wrapped_pll_phase_post_5k),
        .pll_step_q(wrapped_pll_step_q),
        .slow_error(wrapped_slow_error),
        .input_valid(wrapped_input_valid),
        .pll_update(wrapped_pll_update),
        .theta_40k(wrapped_theta_40k),
        .base_step_40k(wrapped_base_step_40k),
        .phase_error(wrapped_phase_error),
        .correction_quotient(wrapped_correction_quotient),
        .correction_remainder(wrapped_correction_remainder),
        .correction_applied(wrapped_correction_applied),
        .correction_remaining(wrapped_correction_remaining),
        .substep(wrapped_substep),
        .held_ref(wrapped_held_ref),
        .extrapolated_ref(wrapped_extrapolated_ref),
        .anchor_residual(wrapped_anchor_residual),
        .interval_complete(wrapped_interval_complete),
        .debug_values(debug_values),
        .debug_values_known(debug_values_known)
    );

    task check_outputs;
        begin
            if (direct_outputs !== wrapped_outputs) begin
                $display("FAIL control outputs at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[0 +: 64]) !== ac_input) begin
                $display("FAIL debug lane 0 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[64 +: 64]) !== direct.x) begin
                $display("FAIL debug lane 1 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[128 +: 64]) !== direct.va) begin
                $display("FAIL debug lane 2 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[192 +: 64]) !== direct.vb) begin
                $display("FAIL debug lane 3 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[256 +: 64]) !== direct.vq) begin
                $display("FAIL debug lane 4 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[320 +: 64]) !== $signed(direct_slow_error)) begin
                $display("FAIL debug lane 5 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[384 +: 64]) !== direct_phase_error) begin
                $display("FAIL debug lane 6 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[448 +: 64]) !== $signed(direct_pll_step_q)) begin
                $display("FAIL debug lane 7 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[512 +: 64]) !== direct_pll_phase_5k) begin
                $display("FAIL debug lane 8 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[576 +: 64]) !== direct_pll_phase_post_5k) begin
                $display("FAIL debug lane 9 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[640 +: 64]) !== direct_theta_40k) begin
                $display("FAIL debug lane 10 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[704 +: 64]) !== direct_base_step_40k) begin
                $display("FAIL debug lane 11 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[768 +: 64]) !== direct_substep) begin
                $display("FAIL debug lane 12 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[832 +: 64]) !== direct_anchor_residual) begin
                $display("FAIL debug lane 13 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[896 +: 64]) !== direct_correction_quotient) begin
                $display("FAIL debug lane 14 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[960 +: 64]) !== direct_correction_remainder) begin
                $display("FAIL debug lane 15 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[1024 +: 64]) !== direct_correction_applied) begin
                $display("FAIL debug lane 16 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[1088 +: 64]) !== direct_correction_remaining) begin
                $display("FAIL debug lane 17 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[1152 +: 64]) !== direct.raw) begin
                $display("FAIL debug lane 18 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[1216 +: 64]) !== direct.err) begin
                $display("FAIL debug lane 19 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[1280 +: 64]) !== direct.offset) begin
                $display("FAIL debug lane 20 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[1344 +: 64]) !== $signed(direct_held_ref)) begin
                $display("FAIL debug lane 21 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[1408 +: 64]) !== $signed(direct_extrapolated_ref)) begin
                $display("FAIL debug lane 22 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            if ($signed(debug_values[1472 +: 64]) !== direct.delivery_increment) begin
                $display("FAIL debug lane 23 at sample %0d", sample_index);
                failed_checks = failed_checks + 1;
            end
            checked_samples = checked_samples + 1;
        end
    endtask

    initial begin
        clk = 0;
        reset = 0;
        ac_input = 2048;
        failed_checks = 0;
        checked_samples = 0;
        sample_index = 0;
        #1 reset = 1;
        #1;
        check_outputs;
        reset = 0;
        for (sample_index = 0; sample_index < 120000; sample_index = sample_index + 1) begin
            frequency_hz = 60.0;
            if (sample_index >= 40000 && sample_index < 60000) frequency_hz = 59.0;
            if (sample_index >= 60000 && sample_index < 80000) frequency_hz = 61.0;
            sample_time = sample_index / 40000.0;
            adc_sample = 2048.0 + 1860.0 * $sin(6.283185307179586 * frequency_hz * sample_time);
            if (sample_index >= 80000 && sample_index < 88000) adc_sample = 2048;
            ac_input = $rtoi(adc_sample);
            #12499 clk = 1;
            #1;
            check_outputs;
            #12499 clk = 0;
            #1;
        end
        reset = 1;
        #1;
        check_outputs;
        if (failed_checks == 0)
            $display("PASS: %0d comparisons of all 646 existing output bits and 24 debug lanes", checked_samples);
        else
            $display("FAIL: %0d checks", failed_checks);
        $finish;
    end
endmodule

