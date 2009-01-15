A5/1 Circuit, expressed in Verilog

The following is equivalent to the purported A5/1 
pedagogical C earlier published.

For non-commercial, non-governmental use only.

It comes with a testbench and sample output.

/*
a5.v
Purported A5/1 circuit, expressed in Verilog. 
13 May 99 
David Honig
honig@sprynet.com

Derived from Briceno, Goldberg, Wagner's Pedagogical C Code
of May 99.

To load key: assert Startloading, load data starting on next clock,
bitwise (1 delay + 64 key + 22 frame clocks).

Then wait for Doneloading to be asserted (100 more clocks).  Then
harvest your bits.

A testbench and sample output is appended as comments.

This synthesizes to about 150 LCs and runs at 80 Mhz on
the smaller Altera CPLDs e.g., 10K30.

*/



module a5(Clk, Reset_n,
        Bitout,
        Keybit,
        Startloading,
        Doneloading);


input Clk, Reset_n;
output Bitout;          // output keystream
        reg Bitout;
input   Keybit;         // input keybits 64 + 22
input   Startloading;   // initial keyload
output  Doneloading;    // signal done of keyloading
        reg Doneloading;


// internal state; lsb is leftmost 
reg [18:0] lfsr_1;
reg [21:0] lfsr_2;
reg [22:0] lfsr_3;

reg [1:0] State;        // FSM control
reg [6:0] Counter;      // for counting steps
reg [2:0] Phase;        // for sequencing phases

wire hi_1, hi_2, hi_3;
assign hi_1 = lfsr_1[18];
assign hi_2 = lfsr_2[21];
assign hi_3 = lfsr_3[22];


wire mid1, mid2, mid3;
assign mid1=lfsr_1[8];  
assign mid2=lfsr_2[10];  
assign mid3=lfsr_3[10];  


wire maj; 
assign maj=majority(mid1, mid2, mid3);

wire newbit1, newbit2, newbit3;
assign newbit1= ( lfsr_1[13] ^ lfsr_1[16] ^ lfsr_1[17] ^ lfsr_1[18] );
assign newbit2= ( lfsr_2[20] ^ lfsr_2[21] ) ;
assign newbit3= ( lfsr_3[7]  ^ lfsr_3[20] ^ lfsr_3[21] ^ lfsr_3[22] );


parameter IDLE=0;
parameter KEYING=1;
parameter RUNNING=2;


always @(posedge Clk or negedge Reset_n) begin
if (!Reset_n) begin: resetting

        $display("a5 reset");
        Doneloading <=0;
        Bitout <=0;
        {lfsr_1, lfsr_2, lfsr_3} <= 64'h 0;
        {State, Counter, Phase} <=0;

        end // reset
else begin
        case (State)

           IDLE: begin: reset_but_no_key

                if (Startloading) begin: startloadingkey
                        // $display("Loading key starts at %0d ", $time);
                        State <= KEYING; 
                        {lfsr_1, lfsr_2, lfsr_3} <= 64'h 0;
                        Phase <=0; Counter<=0;
                        end // if
                end // idle

           KEYING: begin

                case (Phase) 

                        0: begin: load64andclock
                        
                                clockallwithkey;

                                // $display("Loading key bit %0b  %0d at %0d   %0x", Keybit, Counter, $time, lfsr_1);
                                if (Counter==63) begin
                                        Counter <=0; 
                                        Phase <= Phase +1; 
                                        $display(" ");

                                        end
                                else Counter <=Counter+1; 
                                end

                        1: begin: load22andclock

                                // $display("Loading frame bit %0b at %0d %0d   %0x", Keybit, Counter, $time, lfsr_1);
                                clockallwithkey;

                                if (Counter==21) begin
                                        Counter <=0; 
                                        Phase <= Phase +1; 
                                        end
                                else Counter <=Counter+1;  
                                end

                        2: begin: clock100

                                majclock;

                                if (Counter ==100) begin
                                        $display("Done keying, now running %0d\n", $time);
                                        State <= RUNNING; 
                                        end
                                else Counter <= Counter+1; 
                                end
                endcase // Phase
                end // keying

           RUNNING: begin

                Doneloading <=1;        // when done loading
                Bitout <= hi_1 ^ hi_2 ^ hi_3; 
                majclock;

                end // running
        endcase // State
        end // else not resetting
end // always











function majority;
input a,b,c;

begin
        case({a,b,c}) // synopsys parallel_case
          3'b 000: majority=0;
          3'b 001: majority=0;
          3'b 010: majority=0;
          3'b 011: majority=1;

          3'b 100: majority=0;
          3'b 101: majority=1;
          3'b 110: majority=1;
          3'b 111: majority=1;
        endcase
end
endfunction


task clock1;
begin
        lfsr_1 <= ( lfsr_1 << 1 ) | newbit1; 
end
endtask

task clock2;
begin
        lfsr_2 <= (lfsr_2 << 1) | newbit2; 
end
endtask

task clock3;
begin
        lfsr_3 <= (lfsr_3 << 1) | newbit3; 
end
endtask

task clockall;
begin
        clock1;
        clock2; 
        clock3;
end
endtask

task clockallwithkey;
begin
        lfsr_1 <= ( lfsr_1 << 1 ) |  newbit1 ^ Keybit; 
        lfsr_2 <= ( lfsr_2 << 1 ) |  newbit2 ^ Keybit; 
        lfsr_3 <= ( lfsr_3 << 1 ) |  newbit3 ^ Keybit; 
end
endtask

task majclock;
begin
        if (mid1 == maj) clock1;
        if (mid2 == maj) clock2;
        if (mid3 == maj) clock3;
end
endtask


endmodule














/**************** CUT HERE FOR TESTBENCH test_a5.v **************************

module test_a5;


reg Clk, Reset_n;
wire Bitout;            // output keystream

reg     Keybit;         // input keybits 64 + 22
reg     Startloading;   // initial keyload
wire    Doneloading;    // signal done of keyloading


reg [0:7] key [7:0]; 
reg [22:0] frame; 



a5 dut(Clk, Reset_n,
        Bitout,
        Keybit,
        Startloading,
        Doneloading);


always @(Clk) #5 Clk <= ~Clk;

integer i,j;


initial begin
        #5
        key[0]= 8'h 12;
        key[1]= 8'h 23;
        key[2]= 8'h 45;
        key[3]= 8'h 67;

        key[4]= 8'h 89;
        key[5]= 8'h AB;
        key[6]= 8'h CD; 
        key[7]= 8'h EF;

        frame <= 22'h 134;
        Clk <=0;
        Reset_n <=1; 
        Startloading <=0;
        Keybit <=0;
        
        #10 Reset_n <=0; 
        #10 Reset_n <=1; 

        // key setup 
        
        #100
         Startloading <=1; $display("Starting to key %0d", $time); 
        for (i=0; i<8; i=i+1) begin
                for (j=0; j<8; j=j+1) begin
                        #10 Startloading <=0; 
                            Keybit <= key[i] >> j;
                        end // j
                end // i

        for (i=0; i<22; i=i+1) begin
                #10 Keybit <= frame[i]; 
                end

        wait(Doneloading); $display("Done keying %0d", $time); 

        $write("\nBits out: \n"); 
        repeat (32) #10 $write("%b", Bitout);


        $display("\nknown good=\n%b", 32'h 534EAA58);
        
        #1000 $display("\nSim done."); $finish;
end // init 

endmodule



************************* END OF TESTBENCH ************************************/


/**** SAMPLE OUTPUT
 
a5 reset
a5 reset
Starting to key 125
 
Done keying, now running 2000

Done keying 2010

Bits out: 
01010011010011101010101001011000
known good=
01010011010011101010101001011000

Sim done.
*********/


// eof

