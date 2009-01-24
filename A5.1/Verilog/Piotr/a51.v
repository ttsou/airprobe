//                              -*- Mode: Verilog -*-
// Filename        : a51.v
// Description     : A5/1 algorithm (without mixing of the Kc and Fn)
// Author          : piotr
// Created On      : Mon Jan 19 20:55:38 2009
// Last Modified By: .
// Last Modified On: .
// Update Count    : 0
// Status          : Unknown, Use with caution!


module a51(
	   clk,
	   enable,
	   in_state,
	   keystream
	   );

   parameter StateWidth = 64;
   
   input clk, enable;
   input [StateWidth:1] in_state;   
   output [StateWidth:1] keystream;

   
   wire [StateWidth:1] step1_out;
   one_step step1(.clk(clk), .enable(enable), .in_state(in_state), .out_state(step1_out));

   wire [StateWidth:1] step2_out;
   one_step step2(.clk(clk), .enable(enable), .in_state(step1_out), .out_state(step2_out));

   wire [StateWidth:1] step3_out;
   one_step step3(.clk(clk), .enable(enable), .in_state(step2_out), .out_state(step3_out));

   wire [StateWidth:1] step4_out;
   one_step step4(.clk(clk), .enable(enable), .in_state(step3_out), .out_state(step4_out));

   wire [StateWidth:1] step5_out;
   one_step step5(.clk(clk), .enable(enable), .in_state(step4_out), .out_state(step5_out));

   wire [StateWidth:1] step6_out;
   one_step step6(.clk(clk), .enable(enable), .in_state(step5_out), .out_state(step6_out));

   wire [StateWidth:1] step7_out;
   one_step step7(.clk(clk), .enable(enable), .in_state(step6_out), .out_state(step7_out));

   wire [StateWidth:1] step8_out;
   one_step step8(.clk(clk), .enable(enable), .in_state(step7_out), .out_state(step8_out));

   wire [StateWidth:1] step9_out;
   one_step step9(.clk(clk), .enable(enable), .in_state(step8_out), .out_state(step9_out));

   wire [StateWidth:1] step10_out;
   one_step step10(.clk(clk), .enable(enable), .in_state(step9_out), .out_state(step10_out));

   wire [StateWidth:1] step11_out;
   one_step step11(.clk(clk), .enable(enable), .in_state(step10_out), .out_state(step11_out));

   wire [StateWidth:1] step12_out;
   one_step step12(.clk(clk), .enable(enable), .in_state(step11_out), .out_state(step12_out));

   wire [StateWidth:1] step13_out;
   one_step step13(.clk(clk), .enable(enable), .in_state(step12_out), .out_state(step13_out));

   wire [StateWidth:1] step14_out;
   one_step step14(.clk(clk), .enable(enable), .in_state(step13_out), .out_state(step14_out));

   wire [StateWidth:1] step15_out;
   one_step step15(.clk(clk), .enable(enable), .in_state(step14_out), .out_state(step15_out));

   wire [StateWidth:1] step16_out;
   one_step step16(.clk(clk), .enable(enable), .in_state(step15_out), .out_state(step16_out));

   wire [StateWidth:1] step17_out;
   one_step step17(.clk(clk), .enable(enable), .in_state(step16_out), .out_state(step17_out));

   wire [StateWidth:1] step18_out;
   one_step step18(.clk(clk), .enable(enable), .in_state(step17_out), .out_state(step18_out));

   wire [StateWidth:1] step19_out;
   one_step step19(.clk(clk), .enable(enable), .in_state(step18_out), .out_state(step19_out));

   wire [StateWidth:1] step20_out;
   one_step step20(.clk(clk), .enable(enable), .in_state(step19_out), .out_state(step20_out));

   wire [StateWidth:1] step21_out;
   one_step step21(.clk(clk), .enable(enable), .in_state(step20_out), .out_state(step21_out));

   wire [StateWidth:1] step22_out;
   one_step step22(.clk(clk), .enable(enable), .in_state(step21_out), .out_state(step22_out));

   wire [StateWidth:1] step23_out;
   one_step step23(.clk(clk), .enable(enable), .in_state(step22_out), .out_state(step23_out));

   wire [StateWidth:1] step24_out;
   one_step step24(.clk(clk), .enable(enable), .in_state(step23_out), .out_state(step24_out));

   wire [StateWidth:1] step25_out;
   one_step step25(.clk(clk), .enable(enable), .in_state(step24_out), .out_state(step25_out));

   wire [StateWidth:1] step26_out;
   one_step step26(.clk(clk), .enable(enable), .in_state(step25_out), .out_state(step26_out));

   wire [StateWidth:1] step27_out;
   one_step step27(.clk(clk), .enable(enable), .in_state(step26_out), .out_state(step27_out));

   wire [StateWidth:1] step28_out;
   one_step step28(.clk(clk), .enable(enable), .in_state(step27_out), .out_state(step28_out));

   wire [StateWidth:1] step29_out;
   one_step step29(.clk(clk), .enable(enable), .in_state(step28_out), .out_state(step29_out));

   wire [StateWidth:1] step30_out;
   one_step step30(.clk(clk), .enable(enable), .in_state(step29_out), .out_state(step30_out));

   wire [StateWidth:1] step31_out;
   one_step step31(.clk(clk), .enable(enable), .in_state(step30_out), .out_state(step31_out));

   wire [StateWidth:1] step32_out;
   one_step step32(.clk(clk), .enable(enable), .in_state(step31_out), .out_state(step32_out));

   wire [StateWidth:1] step33_out;
   one_step step33(.clk(clk), .enable(enable), .in_state(step32_out), .out_state(step33_out));

   wire [StateWidth:1] step34_out;
   one_step step34(.clk(clk), .enable(enable), .in_state(step33_out), .out_state(step34_out));

   wire [StateWidth:1] step35_out;
   one_step step35(.clk(clk), .enable(enable), .in_state(step34_out), .out_state(step35_out));

   wire [StateWidth:1] step36_out;
   one_step step36(.clk(clk), .enable(enable), .in_state(step35_out), .out_state(step36_out));

   wire [StateWidth:1] step37_out;
   one_step step37(.clk(clk), .enable(enable), .in_state(step36_out), .out_state(step37_out));

   wire [StateWidth:1] step38_out;
   one_step step38(.clk(clk), .enable(enable), .in_state(step37_out), .out_state(step38_out));

   wire [StateWidth:1] step39_out;
   one_step step39(.clk(clk), .enable(enable), .in_state(step38_out), .out_state(step39_out));

   wire [StateWidth:1] step40_out;
   one_step step40(.clk(clk), .enable(enable), .in_state(step39_out), .out_state(step40_out));

   wire [StateWidth:1] step41_out;
   one_step step41(.clk(clk), .enable(enable), .in_state(step40_out), .out_state(step41_out));

   wire [StateWidth:1] step42_out;
   one_step step42(.clk(clk), .enable(enable), .in_state(step41_out), .out_state(step42_out));

   wire [StateWidth:1] step43_out;
   one_step step43(.clk(clk), .enable(enable), .in_state(step42_out), .out_state(step43_out));

   wire [StateWidth:1] step44_out;
   one_step step44(.clk(clk), .enable(enable), .in_state(step43_out), .out_state(step44_out));

   wire [StateWidth:1] step45_out;
   one_step step45(.clk(clk), .enable(enable), .in_state(step44_out), .out_state(step45_out));

   wire [StateWidth:1] step46_out;
   one_step step46(.clk(clk), .enable(enable), .in_state(step45_out), .out_state(step46_out));

   wire [StateWidth:1] step47_out;
   one_step step47(.clk(clk), .enable(enable), .in_state(step46_out), .out_state(step47_out));

   wire [StateWidth:1] step48_out;
   one_step step48(.clk(clk), .enable(enable), .in_state(step47_out), .out_state(step48_out));

   wire [StateWidth:1] step49_out;
   one_step step49(.clk(clk), .enable(enable), .in_state(step48_out), .out_state(step49_out));

   wire [StateWidth:1] step50_out;
   one_step step50(.clk(clk), .enable(enable), .in_state(step49_out), .out_state(step50_out));

   wire [StateWidth:1] step51_out;
   one_step step51(.clk(clk), .enable(enable), .in_state(step50_out), .out_state(step51_out));

   wire [StateWidth:1] step52_out;
   one_step step52(.clk(clk), .enable(enable), .in_state(step51_out), .out_state(step52_out));

   wire [StateWidth:1] step53_out;
   one_step step53(.clk(clk), .enable(enable), .in_state(step52_out), .out_state(step53_out));

   wire [StateWidth:1] step54_out;
   one_step step54(.clk(clk), .enable(enable), .in_state(step53_out), .out_state(step54_out));

   wire [StateWidth:1] step55_out;
   one_step step55(.clk(clk), .enable(enable), .in_state(step54_out), .out_state(step55_out));

   wire [StateWidth:1] step56_out;
   one_step step56(.clk(clk), .enable(enable), .in_state(step55_out), .out_state(step56_out));

   wire [StateWidth:1] step57_out;
   one_step step57(.clk(clk), .enable(enable), .in_state(step56_out), .out_state(step57_out));

   wire [StateWidth:1] step58_out;
   one_step step58(.clk(clk), .enable(enable), .in_state(step57_out), .out_state(step58_out));

   wire [StateWidth:1] step59_out;
   one_step step59(.clk(clk), .enable(enable), .in_state(step58_out), .out_state(step59_out));

   wire [StateWidth:1] step60_out;
   one_step step60(.clk(clk), .enable(enable), .in_state(step59_out), .out_state(step60_out));

   wire [StateWidth:1] step61_out;
   one_step step61(.clk(clk), .enable(enable), .in_state(step60_out), .out_state(step61_out));

   wire [StateWidth:1] step62_out;
   one_step step62(.clk(clk), .enable(enable), .in_state(step61_out), .out_state(step62_out));

   wire [StateWidth:1] step63_out;
   one_step step63(.clk(clk), .enable(enable), .in_state(step62_out), .out_state(step63_out));

   wire [StateWidth:1] step64_out;
   one_step step64(.clk(clk), .enable(enable), .in_state(step63_out), .out_state(step64_out));

   wire [StateWidth:1] step65_out;
   one_step step65(.clk(clk), .enable(enable), .in_state(step64_out), .out_state(step65_out));

   wire [StateWidth:1] step66_out;
   one_step step66(.clk(clk), .enable(enable), .in_state(step65_out), .out_state(step66_out));

   wire [StateWidth:1] step67_out;
   one_step step67(.clk(clk), .enable(enable), .in_state(step66_out), .out_state(step67_out));

   wire [StateWidth:1] step68_out;
   one_step step68(.clk(clk), .enable(enable), .in_state(step67_out), .out_state(step68_out));

   wire [StateWidth:1] step69_out;
   one_step step69(.clk(clk), .enable(enable), .in_state(step68_out), .out_state(step69_out));

   wire [StateWidth:1] step70_out;
   one_step step70(.clk(clk), .enable(enable), .in_state(step69_out), .out_state(step70_out));

   wire [StateWidth:1] step71_out;
   one_step step71(.clk(clk), .enable(enable), .in_state(step70_out), .out_state(step71_out));

   wire [StateWidth:1] step72_out;
   one_step step72(.clk(clk), .enable(enable), .in_state(step71_out), .out_state(step72_out));

   wire [StateWidth:1] step73_out;
   one_step step73(.clk(clk), .enable(enable), .in_state(step72_out), .out_state(step73_out));

   wire [StateWidth:1] step74_out;
   one_step step74(.clk(clk), .enable(enable), .in_state(step73_out), .out_state(step74_out));

   wire [StateWidth:1] step75_out;
   one_step step75(.clk(clk), .enable(enable), .in_state(step74_out), .out_state(step75_out));

   wire [StateWidth:1] step76_out;
   one_step step76(.clk(clk), .enable(enable), .in_state(step75_out), .out_state(step76_out));

   wire [StateWidth:1] step77_out;
   one_step step77(.clk(clk), .enable(enable), .in_state(step76_out), .out_state(step77_out));

   wire [StateWidth:1] step78_out;
   one_step step78(.clk(clk), .enable(enable), .in_state(step77_out), .out_state(step78_out));

   wire [StateWidth:1] step79_out;
   one_step step79(.clk(clk), .enable(enable), .in_state(step78_out), .out_state(step79_out));

   wire [StateWidth:1] step80_out;
   one_step step80(.clk(clk), .enable(enable), .in_state(step79_out), .out_state(step80_out));

   wire [StateWidth:1] step81_out;
   one_step step81(.clk(clk), .enable(enable), .in_state(step80_out), .out_state(step81_out));

   wire [StateWidth:1] step82_out;
   one_step step82(.clk(clk), .enable(enable), .in_state(step81_out), .out_state(step82_out));

   wire [StateWidth:1] step83_out;
   one_step step83(.clk(clk), .enable(enable), .in_state(step82_out), .out_state(step83_out));

   wire [StateWidth:1] step84_out;
   one_step step84(.clk(clk), .enable(enable), .in_state(step83_out), .out_state(step84_out));

   wire [StateWidth:1] step85_out;
   one_step step85(.clk(clk), .enable(enable), .in_state(step84_out), .out_state(step85_out));

   wire [StateWidth:1] step86_out;
   one_step step86(.clk(clk), .enable(enable), .in_state(step85_out), .out_state(step86_out));

   wire [StateWidth:1] step87_out;
   one_step step87(.clk(clk), .enable(enable), .in_state(step86_out), .out_state(step87_out));

   wire [StateWidth:1] step88_out;
   one_step step88(.clk(clk), .enable(enable), .in_state(step87_out), .out_state(step88_out));

   wire [StateWidth:1] step89_out;
   one_step step89(.clk(clk), .enable(enable), .in_state(step88_out), .out_state(step89_out));

   wire [StateWidth:1] step90_out;
   one_step step90(.clk(clk), .enable(enable), .in_state(step89_out), .out_state(step90_out));

   wire [StateWidth:1] step91_out;
   one_step step91(.clk(clk), .enable(enable), .in_state(step90_out), .out_state(step91_out));

   wire [StateWidth:1] step92_out;
   one_step step92(.clk(clk), .enable(enable), .in_state(step91_out), .out_state(step92_out));

   wire [StateWidth:1] step93_out;
   one_step step93(.clk(clk), .enable(enable), .in_state(step92_out), .out_state(step93_out));

   wire [StateWidth:1] step94_out;
   one_step step94(.clk(clk), .enable(enable), .in_state(step93_out), .out_state(step94_out));

   wire [StateWidth:1] step95_out;
   one_step step95(.clk(clk), .enable(enable), .in_state(step94_out), .out_state(step95_out));

   wire [StateWidth:1] step96_out;
   one_step step96(.clk(clk), .enable(enable), .in_state(step95_out), .out_state(step96_out));

   wire [StateWidth:1] step97_out;
   one_step step97(.clk(clk), .enable(enable), .in_state(step96_out), .out_state(step97_out));

   wire [StateWidth:1] step98_out;
   one_step step98(.clk(clk), .enable(enable), .in_state(step97_out), .out_state(step98_out));

   wire [StateWidth:1] step99_out;
   one_step step99(.clk(clk), .enable(enable), .in_state(step98_out), .out_state(step99_out));

   wire [StateWidth:1] step100_out;
   one_step step100(.clk(clk), .enable(enable), .in_state(step99_out), .out_state(step100_out));

   
endmodule