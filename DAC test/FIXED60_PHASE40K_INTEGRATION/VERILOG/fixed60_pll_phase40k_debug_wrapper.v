`include "fixed60_pll_phase40k_candidate.v"

module fixed60_pll_phase40k_debug_wrapper(
    input wire [11:0] ac_input,
    input wire clk,
    input wire reset,
    output wire phase_ok,
    output wire locked,
    output wire hold_60hz,
    output wire [29:0] pll_phase_5k,
    output wire [29:0] pll_phase_post_5k,
    output wire signed [31:0] pll_step_q,
    output wire signed [31:0] slow_error,
    output wire input_valid,
    output wire pll_update,
    output wire [32:0] theta_40k,
    output wire signed [63:0] base_step_40k,
    output wire signed [63:0] phase_error,
    output wire signed [63:0] correction_quotient,
    output wire signed [63:0] correction_remainder,
    output wire signed [63:0] correction_applied,
    output wire signed [63:0] correction_remaining,
    output wire [2:0] substep,
    output wire signed [15:0] held_ref,
    output wire signed [15:0] extrapolated_ref,
    output wire signed [63:0] anchor_residual,
    output wire interval_complete,
    output wire [1535:0] debug_values,
    output wire debug_values_known
);
    fixed60_pll_phase40k_candidate core(
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

    assign debug_values[0 +: 64] = ac_input;
    assign debug_values[64 +: 64] = core.x;
    assign debug_values[128 +: 64] = core.va;
    assign debug_values[192 +: 64] = core.vb;
    assign debug_values[256 +: 64] = core.vq;
    assign debug_values[320 +: 64] = $signed(slow_error);
    assign debug_values[384 +: 64] = phase_error;
    assign debug_values[448 +: 64] = $signed(pll_step_q);
    assign debug_values[512 +: 64] = pll_phase_5k;
    assign debug_values[576 +: 64] = pll_phase_post_5k;
    assign debug_values[640 +: 64] = theta_40k;
    assign debug_values[704 +: 64] = base_step_40k;
    assign debug_values[768 +: 64] = substep;
    assign debug_values[832 +: 64] = anchor_residual;
    assign debug_values[896 +: 64] = correction_quotient;
    assign debug_values[960 +: 64] = correction_remainder;
    assign debug_values[1024 +: 64] = correction_applied;
    assign debug_values[1088 +: 64] = correction_remaining;
    assign debug_values[1152 +: 64] = core.raw;
    assign debug_values[1216 +: 64] = core.err;
    assign debug_values[1280 +: 64] = core.offset;
    assign debug_values[1344 +: 64] = $signed(held_ref);
    assign debug_values[1408 +: 64] = $signed(extrapolated_ref);
    assign debug_values[1472 +: 64] = core.delivery_increment;
    assign debug_values_known = (^debug_values !== 1'bx);
endmodule

