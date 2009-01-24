//                              -*- Mode: Verilog -*-
// Filename        : one_step_tb.v
// Description     : Testbench for one step of A5/1 module
// Author          : piotr
// Created On      : Mon Jan 19 15:16:40 2009
// Last Modified By: .
// Last Modified On: .
// Update Count    : 0
// Status          : Unknown, Use with caution!
`define InternalStateWidth 64

module one_step_tb();
   reg clk, enable;
   reg [`InternalStateWidth:1] in_state;
   wire[`InternalStateWidth:1] out_state;

   one_step one_step_inst(
			  .clk(clk),
			  .enable(enable),
			  .in_state(in_state),
			  .out_state(out_state)
			  );
   always
   begin
      #5 clk = !clk;
   end

   initial
   begin
      clk = 1'b1;
      enable = 1'b1;
      in_state = `InternalStateWidth'd1;
      #5  in_state = `InternalStateWidth'b1110010110111111000110110101111000000111001010000110010010100010;
      #10 in_state = `InternalStateWidth'b1110010110111111000110101011110000001110010110001100100101000101;
      #10 in_state = `InternalStateWidth'b1100101101111110001101001011110000001110010110011001001010001010;
      #10 in_state = `InternalStateWidth'b1100101101111110001101010111100000011100101110110010010100010101;
      #10 in_state = `InternalStateWidth'b1001011011111100011010010111100000011100101111100100101000101011;
      #10 in_state = `InternalStateWidth'b1001011011111100011010001111000000111001011111001001010001010110;
      #10 in_state = `InternalStateWidth'b0010110111111000110100111110000001110010111111001001010001010110;
      
      #20 $finish;
   end
  
   initial
   begin    
      $dumpvars;
      $dumpfile("one_step_tb.vcd");
      $dumpon;
      #90 $dumpoff;
   end   
endmodule