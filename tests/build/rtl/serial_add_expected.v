`timescale 1ns/1ps

module serial_add_expected(
	input wire clk,
	input wire reset,
	input wire Ac_valid,
	output wire Ac_ready,
	input wire Ac_data,
	input wire Bc_valid,
	output wire Bc_ready,
	input wire Bc_data,
	input wire Bd_valid,
	output wire Bd_ready,
	input wire [3:0] Bd_data,
	input wire Ad_valid,
	output wire Ad_ready,
	input wire [3:0] Ad_data,
	output wire Sc_valid,
	input wire Sc_ready,
	output wire Sc_data,
	output wire Sd_valid,
	input wire Sd_ready,
	output wire [3:0] Sd_data
);
	wire [4:0] s_data;
	assign s_data = Ad_data + Bd_data + ci_data;
	
	reg ci_data;

	wire [4:0] branch_valid;
	wire [4:0] branch_ready;

	assign branch_valid[0] = Ac_valid&&Bc_valid&&Ad_valid&&Bd_valid&&Ac_data==1'b0&&Bc_data==1'b0;
	assign branch_valid[1] = Ac_valid&&Bc_valid&&Ad_valid&&Bd_valid&&Ac_data==1'b0&&Bc_data==1'b1;
	assign branch_valid[2] = Ac_valid&&Bc_valid&&Ad_valid&&Bd_valid&&Ac_data==1'b1&&Bc_data==1'b0;
	assign branch_valid[3] = Ac_valid&&Bc_valid&&Ad_valid&&Bd_valid&&Ac_data==1'b1&&Bc_data==1'b1&&s_data[4]!=ci_data;
	assign branch_valid[4] = Ac_valid&&Bc_valid&&Ad_valid&&Bd_valid&&Ac_data==1'b1&&Bc_data==1'b1&&s_data[4]==ci_data;
	assign branch_ready[0] = branch_valid[0]&&Sc_enable&&Sd_enable;
	assign branch_ready[1] = branch_valid[1]&&Sc_enable&&Sd_enable;
	assign branch_ready[2] = branch_valid[2]&&Sc_enable&&Sd_enable;
	assign branch_ready[3] = branch_valid[3]&&Sc_enable&&Sd_enable;
	assign branch_ready[4] = branch_valid[4]&&Sc_enable&&Sd_enable;

	reg Sc_valid_reg;
	reg Sc_state;
	wire Sc_enable;
	reg Sd_valid_reg;
	reg [3:0] Sd_state;
	wire Sd_enable;
	assign Sc_valid = Sc_valid_reg;
	assign Sc_data = Sc_state;
	assign Sc_enable = (Sc_ready||!Sc_valid_reg);
	assign Sd_valid = Sd_valid_reg;
	assign Sd_data = Sd_state;
	assign Sd_enable = (Sd_ready||!Sd_valid_reg);

	assign Ac_ready = (branch_ready[0]||branch_ready[1]||branch_ready[4]);
	assign Bc_ready = (branch_ready[0]||branch_ready[2]||branch_ready[4]);
	assign Bd_ready = (branch_ready[0]||branch_ready[2]||branch_ready[4]);
	assign Ad_ready = (branch_ready[0]||branch_ready[1]||branch_ready[4]);

`ifndef SYNTHESIS
	wire Ac;
	wire Bc;
	wire Sc;
	wire [3:0] Ad;
	wire [3:0] Bd;
	wire [3:0] Sd;
	assign Ac = Ac_valid ? Ac_data : 1'bZ;
	assign Ad = Ad_valid ? Ad_data : 4'bZ;
	assign Bc = Bc_valid ? Bc_data : 1'bZ;
	assign Bd = Bd_valid ? Bd_data : 4'bZ;
	assign Sc = Sc_valid ? Sc_data : 1'bZ;
	assign Sd = Sd_valid ? Sd_data : 4'bZ;
`endif

	always @(posedge clk) begin
		if (reset) begin
			Sc_valid_reg <= 0;
			Sc_state <= 0;
			Sd_valid_reg <= 0;
			Sd_state <= 0;
			ci_data <= 0;
		end else if (branch_valid[0]&&(Sc_ready||!Sc_valid_reg)&&(Sd_ready||!Sd_valid_reg)) begin
			Sc_state <= 0;
			Sc_valid_reg <= 1;
			Sd_state <= s_data[3:0];
			Sd_valid_reg <= 1;
			ci_data <= s_data[4];
		end else if (branch_valid[1]&&(Sc_ready||!Sc_valid_reg)&&(Sd_ready||!Sd_valid_reg)) begin
			Sc_state <= 0;
			Sc_valid_reg <= 1;
			Sd_state <= s_data[3:0];
			Sd_valid_reg <= 1;
			ci_data = s_data[4];
		end else if (branch_valid[2]&&(Sc_ready||!Sc_valid_reg)&&(Sd_ready||!Sd_valid_reg)) begin
			Sc_state <= 0;
			Sc_valid_reg <= 1;
			Sd_state <= s_data[3:0];
			Sd_valid_reg <= 1;
			ci_data <= s_data[4];
		end else if (branch_valid[3]&&(Sc_ready||!Sc_valid_reg)&&(Sd_ready||!Sd_valid_reg)) begin
			Sc_state <= 0;
			Sc_valid_reg <= 1;
			Sd_state <= s_data[3:0];
			Sd_valid_reg <= 1;
			ci_data <= s_data[4];
		end else if (branch_valid[4]&&(Sc_ready||!Sc_valid_reg)&&(Sd_ready||!Sd_valid_reg)) begin
			Sc_state <= 1;
			Sc_valid_reg <= 1;
			Sd_state <= s_data[3:0];
			Sd_valid_reg <= 1;
			ci_data <= 0;
		end else begin
			if (Sc_ready) begin
				Sc_valid_reg <= 0;
			end

			if (Sd_ready) begin
				Sd_valid_reg <= 0;
			end
		end
	end

`ifndef SYNTHESIS
	initial begin
		$dumpfile("dump.vcd");
		$dumpvars(0, serial_add_expected);
	end
`endif
endmodule
