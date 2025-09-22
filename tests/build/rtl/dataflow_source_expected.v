`timescale 1ns/1ps

module dataflow_source_expected(
	input wire clk,
	input wire reset,
	output wire R_valid,
	input wire R_ready,
	output wire [7:0] R_data
);
	wire branch_valid;
	wire branch_ready;

	assign branch_valid = 1'b1;
	assign branch_ready = R_enable;

	reg R_valid_reg;
	reg [7:0] R_state;
	wire R_enable;

	assign R_valid = R_valid_reg;
	assign R_data = R_state;
	assign R_enable = (R_ready||!R_valid_reg);

	always @(posedge clk) begin
		if (reset) begin
			R_valid_reg <= 0;
			R_state <= 0;
		end else if (branch_valid&&(R_ready||!R_valid_reg)) begin
			R_state <= 0;
			R_valid_reg <= 1;
		end else begin
			if (R_ready) begin
				R_valid_reg <= 0;
			end
		end
	end

`ifndef SYNTHESIS
	wire [7:0] R;
	assign R = R_valid ? R_data : 8'bZ;

	initial begin
		$dumpfile("dump.vcd");
		$dumpvars(0, dataflow_source_expected);
	end
`endif
endmodule
