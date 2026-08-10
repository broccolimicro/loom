`timescale 1ns/1ps

module dataflow_add_expected(
	input wire clk,
	input wire reset,
	input wire A_valid,
	output wire A_ready,
	input wire [7:0] A_data,
	input wire B_valid,
	output wire B_ready,
	input wire [7:0] B_data,
	output wire S_valid,
	input wire S_ready,
	output wire [7:0] S_data
);
	wire branch_valid;
	wire branch_ready;

	assign branch_valid = A_valid&&B_valid;
	assign branch_ready = branch_valid&&S_enable;

	reg S_valid_reg;
	reg [7:0] S_state;
	wire S_enable;

	assign S_valid = S_valid_reg;
	assign S_data = S_state;
	assign S_enable = (S_ready||!S_valid_reg);

	assign B_ready = branch_ready;
	assign A_ready = branch_ready;

	always @(posedge clk) begin
		if (reset) begin
			S_valid_reg <= 0;
			S_state <= 0;
		end else if (branch_valid&&(S_ready||!S_valid_reg)) begin
			S_state <= A_data+B_data;
			S_valid_reg <= 1;
		end else begin
			if (S_ready) begin
				S_valid_reg <= 0;
			end
		end
	end

`ifndef SYNTHESIS
	wire [7:0] A;
	wire [7:0] B;
	wire [7:0] S;
	assign A = A_valid ? A_data : 8'bZ;
	assign B = B_valid ? B_data : 8'bZ;
	assign S = S_valid ? S_data : 8'bZ;

	initial begin
		$dumpfile("dump.vcd");
		$dumpvars(0, dataflow_add_expected);
	end
`endif
endmodule
