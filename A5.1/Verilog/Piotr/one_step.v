//                              -*- Mode: Verilog -*-
// Filename        : one_step.v
// Description     : One step of the pipelined A5/1 algorithm
// Author          : piotr
// Created On      : Fri Jan 16 19:21:34 2009
// Last Modified By: .
// Last Modified On: .
// Update Count    : 0
// Status          : Work in progress

module one_step(
		clk,
		enable,
		in_state,
		out_state,
		keystream_bit
		);

   parameter StateWidth = 64;
   parameter R1Width = 19;
   parameter R2Width = 22;
   parameter R3Width = 23;
   
   input     clk, enable;
   input [StateWidth:1] in_state;
   output keystream_bit;
   output [StateWidth:1] out_state;

   wire [R1Width:1] 	 R1in;
   wire [R2Width:1] 	 R2in;
   wire [R3Width:1] 	 R3in;

   wire 		 R1_feedback;
   wire 		 R2_feedback;
   wire 		 R3_feedback;
   
   wire 		 R1_clk_out;
   wire 		 R2_clk_out;
   wire 		 R3_clk_out;

   reg [R1Width:1] 	 R1;
   reg [R2Width:1] 	 R2;
   reg [R3Width:1] 	 R3;
   
   assign 		 R1in = in_state[R1Width:1];
   assign 		 R2in = in_state[R2Width+R1Width:R1Width+1];
   assign 		 R3in = in_state[StateWidth:R1Width+R2Width+1];

   assign 		 R1_feedback = R1in[19] ^ R1in[18] ^ R1in[17] ^ R1in[14];
   assign 		 R2_feedback = R2in[22] ^ R2in[21];
   assign 		 R3_feedback = R3in[23] ^ R3in[22] ^ R3in[21] ^ R3in[7];

   assign 		 out_state = {R3,R2,R1};

   assign 		 keystream_bit = R1[R1Width] ^ R2[R2Width] ^ R3[R3Width];
      
   majority majority_instance(
		.R1_clk_bit(R1in[9]),
		.R2_clk_bit(R2in[11]),
		.R3_clk_bit(R3in[11]),
		.R1_clk_out(R1_clk_out),
		.R2_clk_out(R2_clk_out),
		.R3_clk_out(R3_clk_out)
		);
   
   always @(posedge clk)
     begin
	if(enable == 1'b1)
	  begin
	     if(R1_clk_out == 1'b1)
	       R1 <= {R1in[R1Width-1:1], R1_feedback};
	     else
	       R1 <= R1in;
	     
	     if(R2_clk_out == 1'b1)
	       R2 <= {R2in[R2Width-1:1], R2_feedback};
	     else
	       R2 <= R2in;
	     
	     if(R3_clk_out == 1'b1)
	       R3 <= {R3in[R3Width-1:1], R3_feedback};
	     else
	       R3 <= R3in;
	  end
     end
endmodule
