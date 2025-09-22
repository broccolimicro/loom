`timescale 1ns/1ps

module dataflow_sink_expected(
	input wire clk,
	input wire reset,
	input wire L_valid,
	output wire L_ready,
	input wire [7:0] L_data
);
	wire branch_valid;
	wire branch_ready;

	assign branch_valid = L_valid;
	assign branch_ready = branch_valid;

	assign L_ready = branch_ready;

`ifndef SYNTHESIS
	wire [7:0] L;
	assign L = L_valid ? L_data : 8'bZ;

	initial begin
		$dumpfile("dump.vcd");
		$dumpvars(0, dataflow_sink_expected);
	end
`endif
endmodule
