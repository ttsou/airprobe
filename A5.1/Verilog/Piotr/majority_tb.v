//                              -*- Mode: Verilog -*-
// Filename        : majority_tb.v
// Description     : 
// Author          : piotr
// Created On      : Mon Jan 19 16:41:43 2009
// Last Modified By: .
// Last Modified On: .
// Update Count    : 0
// Status          : Unknown, Use with caution!

module majority_tb();
   reg	R1_clk_bit, R2_clk_bit, R3_clk_bit;
   wire R1_clk_out, R2_clk_out,	R3_clk_out;
   
   majority majority_inst(
			  .R1_clk_bit(R1_clk_bit),
			  .R2_clk_bit(R2_clk_bit),
			  .R3_clk_bit(R3_clk_bit),
			  .R1_clk_out(R1_clk_out),
			  .R2_clk_out(R2_clk_out),
			  .R3_clk_out(R3_clk_out)
			  );

  initial
   begin
      R1_clk_bit = 0;
      R2_clk_bit = 0;
      R3_clk_bit = 0;
      
      #10 R1_clk_bit = 1;
      R2_clk_bit = 0;
      R3_clk_bit = 0;
      
      #10 R1_clk_bit = 0;
      R2_clk_bit = 1;
      R3_clk_bit = 0;      

      #10 R1_clk_bit = 1;
      R2_clk_bit = 1;
      R3_clk_bit = 0;      

      #10 R1_clk_bit = 0;
      R2_clk_bit = 0;
      R3_clk_bit = 1;

      #10 R1_clk_bit = 1;
      R2_clk_bit = 0;
      R3_clk_bit = 1;      
      
      #10 R1_clk_bit = 0;
      R2_clk_bit = 1;
      R3_clk_bit = 1;
      
      #10 R1_clk_bit = 1;
      R2_clk_bit = 1;
      R3_clk_bit = 1;      
   end
  
   initial
   begin    
      $dumpvars;
      $dumpfile("majority_tb.vcd");
      $dumpon;
      #90 $dumpoff;
   end  
   
endmodule