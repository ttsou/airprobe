//                              -*- Mode: Verilog -*-
// Filename        : majority.v
// Description     : Majority function
// Author          : piotr
// Created On      : Sun Jan 18 23:16:04 2009
// Last Modified By: .
// Last Modified On: .
// Update Count    : 0
// Status          : Unknown, Use with caution!

module majority(
		R1_clk_bit,
		R2_clk_bit,
		R3_clk_bit,
		R1_clk_out,
		R2_clk_out,
		R3_clk_out,
		);

   input R1_clk_bit;   
   input R2_clk_bit;   
   input R3_clk_bit;   
   output R1_clk_out;
   output R2_clk_out;   
   output R3_clk_out;

   wire   majority_bit;

   assign majority_bit = ( R1_clk_bit & R2_clk_bit ) ^ ( R2_clk_bit & R3_clk_bit ) ^ ( R1_clk_bit & R3_clk_bit );

   assign R1_clk_out = ! majority_bit ^ R1_clk_bit;
   assign R2_clk_out = ! majority_bit ^ R2_clk_bit;
   assign R3_clk_out = ! majority_bit ^ R3_clk_bit;
   
endmodule