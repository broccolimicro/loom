`timescale 1ns/1ps

module dataflow_split_expected(
	input wire clk,
	input wire reset,
	input wire Cc_valid,
	output wire Cc_ready,
	input wire Cc_data,
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
	wire [1:0] branch_valid;
	wire [1:0] branch_ready;

	assign branch_valid[0] = Cc_valid&&L_valid&&Cc_data==0;
	assign branch_valid[1] = Cc_valid&&L_valid&&Cc_data==1;
	assign branch_ready[0] = branch_valid[0]&&R0_enable;
	assign branch_ready[1] = branch_valid[1]&&R1_enable;

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
	
	assign Cc_ready = branch_ready[0]||branch_ready[1];
	assign L_ready = branch_ready[0]||branch_ready[1];

	always @(posedge clk) begin
		if (reset) begin
			R0_valid_reg <= 0;
			R0_state <= 0;
			R1_valid_reg <= 0;
			R1_state <= 0;
		end else if (branch_valid[0]&&(R0_ready||!R0_valid_reg)) begin
			R0_state <= L_data;
			R0_valid_reg <= 1;
			if (R1_ready) begin
				R1_valid_reg <= 0;
			end
		end else if (branch_valid[1]&&(R1_ready||!R1_valid_reg)) begin
			R1_state <= L_data;
			R1_valid_reg <= 1;
			if (R0_ready) begin
				R0_valid_reg <= 0;
			end
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
	wire Cc;
	wire [7:0] L;
	wire [7:0] R0;
	wire [7:0] R1;
	assign Cc = Cc_valid ? Cc_data : 1'bZ;
	assign L = L_valid ? L_data : 8'bZ;
	assign R0 = R0_valid ? R0_data : 8'bZ;
	assign R1 = R1_valid ? R1_data : 8'bZ;

	initial begin
		$dumpfile("dump.vcd");
		$dumpvars(0, dataflow_split_expected);
	end
`endif
endmodule
