`timescale 1ns/1ps

module dataflow_merge_expected(
	input wire clk,
	input wire reset,
	input wire Cc_valid,
	output wire Cc_ready,
	input wire Cc_data,
	input wire L0_valid,
	output wire L0_ready,
	input wire [7:0] L0_data,
	input wire L1_valid,
	output wire L1_ready,
	input wire [7:0] L1_data,
	output wire R_valid,
	input wire R_ready,
	output wire [7:0] R_data
);
	wire [1:0] branch_valid;
	wire [1:0] branch_ready;

	assign branch_valid[0] = Cc_valid&&L0_valid&&Cc_data==0;
	assign branch_valid[1] = Cc_valid&&L1_valid&&Cc_data==1;
	assign branch_ready[0] = branch_valid[0]&&R_enable;
	assign branch_ready[1] = branch_valid[1]&&R_enable;

	reg R_valid_reg;
	reg [7:0] R_state;
	wire R_enable;

	assign R_valid = R_valid_reg;
	assign R_data = R_state;
	assign R_enable = (R_ready||!R_valid_reg);

	assign Cc_ready = branch_ready[0]||branch_ready[1];
	assign L0_ready = branch_ready[0];
	assign L1_ready = branch_ready[1];

	always @(posedge clk) begin
		if (reset) begin
			R_valid_reg <= 0;
			R_state <= 0;
		end else if (branch_valid[0]&&(R_ready||!R_valid_reg)) begin
			R_state <= L0_data;
			R_valid_reg <= 1;
		end else if (branch_valid[1]&&(R_ready||!R_valid_reg)) begin
			R_state <= L1_data;
			R_valid_reg <= 1;
		end else begin
			if (R_ready) begin
				R_valid_reg <= 0;
			end
		end
	end

`ifndef SYNTHESIS
	wire Cc;
	wire [7:0] L0;
	wire [7:0] L1;
	wire [7:0] R;
	assign Cc = Cc_valid ? Cc_data : 1'bZ;
	assign L0 = L0_valid ? L0_data : 8'bZ;
	assign L1 = L1_valid ? L1_data : 8'bZ;
	assign R = R_valid ? R_data : 8'bZ;

	initial begin
		$dumpfile("dump.vcd");
		$dumpvars(0, dataflow_merge_expected);
	end
`endif
endmodule
