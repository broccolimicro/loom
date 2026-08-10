`timescale 1ns/1ps

module dataflow_copy_expected(
	input wire clk,
	input wire reset,
	input wire L_valid,
	output wire L_ready,
	input wire [7:0] L_data,
	output wire R0_valid,
	input wire R0_ready,
	output wire [7:0] R0_data,
	output wire R1_valid,
	input wire R1_ready,
	output wire [7:0] R1_data
);
	wire [7:0] l_data;
	assign l_data = L_data;

	wire branch_valid;
	wire branch_ready;

	assign branch_valid = L_valid;
	assign branch_ready = branch_valid&&R0_enable&&R1_enable;

	reg R0_valid_reg;
	reg [7:0] R0_state;
	wire R0_enable;
	reg R1_valid_reg;
	reg [7:0] R1_state;
	wire R1_enable;

	assign R0_valid = R0_valid_reg;
	assign R0_data = R0_state;
	assign R0_enable = (R0_ready||!R0_valid_reg);
	assign R1_valid = R1_valid_reg;
	assign R1_data = R1_state;
	assign R1_enable = (R1_ready||!R1_valid_reg);

	assign L_ready = branch_ready;

	always @(posedge clk) begin
		if (reset) begin
			R0_valid_reg <= 0;
			R0_state <= 0;
			R1_valid_reg <= 0;
			R1_state <= 0;
		end else if (branch_valid&&(R0_ready||!R0_valid_reg)&&(R1_ready||!R1_valid_reg)) begin
			R0_state <= l_data;
			R0_valid_reg <= 1;
			R1_state <= l_data;
			R1_valid_reg <= 1;
		end else begin
			if (R0_ready) begin
				R0_valid_reg <= 0;
			end

			if (R1_ready) begin
				R1_valid_reg <= 0;
			end
		end
	end

`ifndef SYNTHESIS
	wire [7:0] L;
	wire [7:0] R0;
	wire [7:0] R1;
	assign L = L_valid ? L_data : 8'bZ;
	assign R0 = R0_valid ? R0_data : 8'bZ;
	assign R1 = R1_valid ? R1_data : 8'bZ;

	initial begin
		$dumpfile("dump.vcd");
		$dumpvars(0, dataflow_copy_expected);
	end
`endif
endmodule
