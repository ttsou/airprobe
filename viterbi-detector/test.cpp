#include <stdio.h>
/*
** viterbi_detector:
**           This part does the detection of received sequnece.
**           Employed algorithm is viterbi Maximum Likehood Sequence Estimation.
**           At this moment it gives hard decisions on the output, but
**           it was designed with soft decisions in mind.
**
** SYNTAX:   void viterbi_detector(
**                                  const gr_complex * input, 
**                                  unsigned int samples_num, 
**                                  gr_complex * rhh, 
**                                  unsigned int start_state, 
**                                  const unsigned int * stop_states, 
**                                  unsigned int stops_num, 
**                                  float * output)
**
** INPUT:    input:       Complex received signal afted matched filtering.
**           samples_num: Number of samples in the input table.
**           rhh:         The autocorrelation of the estimated channel 
**                        impulse response.
**           start_state: Number of the start point. In GSM each burst 
**                        starts with sequence of three bits (0,0,0) which 
**                        indicates start point of the algorithm.
**           stop_states: Table with numbers of possible stop states.
**           stops_num:   Number of possible stop states
**                     
**
** OUTPUT:   output:      Differentially decoded hard output of the algorithm: 
**                        -1 for logical "0" and 1 for logical "1"
**
** SUB_FUNC: none
**
** TEST(S):  Tested with real world normal burst.
*/

#include <gnuradio/gr_complex.h>
#define BURST_SIZE 148
#define PATHS_NUM 16

void viterbi_detector(const gr_complex * input, unsigned int samples_num, gr_complex * rhh, unsigned int start_state, const unsigned int * stop_states, unsigned int stops_num, float * output)
{
   float increment[8];
   float path_metrics1[16];
   float path_metrics2[16];
   float * new_path_metrics;
   float * old_path_metrics;
   float * tmp;
   float trans_table[BURST_SIZE][16];
   float pm_candidate1, pm_candidate2;
   bool real_imag;
   float input_symbol_real, input_symbol_imag;
   unsigned int i, sample_nr;

/*
* Setup first path metrics, so only state pointed by start_state is possible.
* Start_state metric is equal to zero, the rest is written with some very low value,
* which makes them practically impossible to occur.
*/
   for(i=0; i<PATHS_NUM; i++){
      path_metrics1[i]=(-10e30);
   }
   path_metrics1[start_state]=0;

/*
* Compute Increment - a table of values which does not change for subsequent input samples.
* Increment is table of reference levels for computation of branch metrics:
*    branch metric = (+/-)received_sample (+/-) reference_level
*/
   increment[0] = -rhh[1].imag() -rhh[2].real() -rhh[3].imag() +rhh[4].real();
   increment[1] = rhh[1].imag() -rhh[2].real() -rhh[3].imag() +rhh[4].real();
   increment[2] = -rhh[1].imag() +rhh[2].real() -rhh[3].imag() +rhh[4].real();
   increment[3] = rhh[1].imag() +rhh[2].real() -rhh[3].imag() +rhh[4].real();
   increment[4] = -rhh[1].imag() -rhh[2].real() +rhh[3].imag() +rhh[4].real();
   increment[5] = rhh[1].imag() -rhh[2].real() +rhh[3].imag() +rhh[4].real();
   increment[6] = -rhh[1].imag() +rhh[2].real() +rhh[3].imag() +rhh[4].real();
   increment[7] = rhh[1].imag() +rhh[2].real() +rhh[3].imag() +rhh[4].real();


/*
* Computation of path metrics and decisions (Add-Compare-Select).
* It's composed of two parts: one for odd input samples (imaginary numbers)
* and one for even samples (real numbers).
* Each part is composed of independent (parallelisable) statements like  
* this one:
*      pm_candidate1 = old_path_metrics[0] - input_symbol_real - increment[7];
*      pm_candidate2 = old_path_metrics[8] - input_symbol_real + increment[0];
*      if(pm_candidate1 > pm_candidate2){
*         new_path_metrics[0] = pm_candidate1;
*         trans_table[sample_nr][0] = -1.0;
*      }
*      else{
*         new_path_metrics[0] = pm_candidate2;
*         trans_table[sample_nr][0] = 1.0;
*      }
* This is very good point for optimisations (SIMD or OpenMP) as it's most time 
* consuming part of this function. 
*/
   printf("# name: path_metrics_test_result\n# type: matrix\n# rows: 148\n# columns: 16\n");
   sample_nr=0;
   old_path_metrics=path_metrics1;
   new_path_metrics=path_metrics2;
   while(sample_nr<samples_num){
      //Processing imag states
      real_imag=1;
      input_symbol_imag = input[sample_nr].imag();

      pm_candidate1 = old_path_metrics[0] + input_symbol_imag - increment[2];
      pm_candidate2 = old_path_metrics[8] + input_symbol_imag + increment[5];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[0] = pm_candidate1;
         trans_table[sample_nr][0] = -1.0;
      }
      else{
         new_path_metrics[0] = pm_candidate2;
         trans_table[sample_nr][0] = 1.0;
      }

      pm_candidate1 = old_path_metrics[0] - input_symbol_imag + increment[2];
      pm_candidate2 = old_path_metrics[8] - input_symbol_imag - increment[5];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[1] = pm_candidate1;
         trans_table[sample_nr][1] = -1.0;
      }
      else{
         new_path_metrics[1] = pm_candidate2;
         trans_table[sample_nr][1] = 1.0;
      }

      pm_candidate1 = old_path_metrics[1] + input_symbol_imag - increment[3];
      pm_candidate2 = old_path_metrics[9] + input_symbol_imag + increment[4];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[2] = pm_candidate1;
         trans_table[sample_nr][2] = -1.0;
      }
      else{
         new_path_metrics[2] = pm_candidate2;
         trans_table[sample_nr][2] = 1.0;
      }

      pm_candidate1 = old_path_metrics[1] - input_symbol_imag + increment[3];
      pm_candidate2 = old_path_metrics[9] - input_symbol_imag - increment[4];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[3] = pm_candidate1;
         trans_table[sample_nr][3] = -1.0;
      }
      else{
         new_path_metrics[3] = pm_candidate2;
         trans_table[sample_nr][3] = 1.0;
      }

      pm_candidate1 = old_path_metrics[2] + input_symbol_imag - increment[0];
      pm_candidate2 = old_path_metrics[10] + input_symbol_imag + increment[7];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[4] = pm_candidate1;
         trans_table[sample_nr][4] = -1.0;
      }
      else{
         new_path_metrics[4] = pm_candidate2;
         trans_table[sample_nr][4] = 1.0;
      }

      pm_candidate1 = old_path_metrics[2] - input_symbol_imag + increment[0];
      pm_candidate2 = old_path_metrics[10] - input_symbol_imag - increment[7];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[5] = pm_candidate1;
         trans_table[sample_nr][5] = -1.0;
      }
      else{
         new_path_metrics[5] = pm_candidate2;
         trans_table[sample_nr][5] = 1.0;
      }

      pm_candidate1 = old_path_metrics[3] + input_symbol_imag - increment[1];
      pm_candidate2 = old_path_metrics[11] + input_symbol_imag + increment[6];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[6] = pm_candidate1;
         trans_table[sample_nr][6] = -1.0;
      }
      else{
         new_path_metrics[6] = pm_candidate2;
         trans_table[sample_nr][6] = 1.0;
      }

      pm_candidate1 = old_path_metrics[3] - input_symbol_imag + increment[1];
      pm_candidate2 = old_path_metrics[11] - input_symbol_imag - increment[6];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[7] = pm_candidate1;
         trans_table[sample_nr][7] = -1.0;
      }
      else{
         new_path_metrics[7] = pm_candidate2;
         trans_table[sample_nr][7] = 1.0;
      }

      pm_candidate1 = old_path_metrics[4] + input_symbol_imag - increment[6];
      pm_candidate2 = old_path_metrics[12] + input_symbol_imag + increment[1];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[8] = pm_candidate1;
         trans_table[sample_nr][8] = -1.0;
      }
      else{
         new_path_metrics[8] = pm_candidate2;
         trans_table[sample_nr][8] = 1.0;
      }

      pm_candidate1 = old_path_metrics[4] - input_symbol_imag + increment[6];
      pm_candidate2 = old_path_metrics[12] - input_symbol_imag - increment[1];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[9] = pm_candidate1;
         trans_table[sample_nr][9] = -1.0;
      }
      else{
         new_path_metrics[9] = pm_candidate2;
         trans_table[sample_nr][9] = 1.0;
      }

      pm_candidate1 = old_path_metrics[5] + input_symbol_imag - increment[7];
      pm_candidate2 = old_path_metrics[13] + input_symbol_imag + increment[0];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[10] = pm_candidate1;
         trans_table[sample_nr][10] = -1.0;
      }
      else{
         new_path_metrics[10] = pm_candidate2;
         trans_table[sample_nr][10] = 1.0;
      }

      pm_candidate1 = old_path_metrics[5] - input_symbol_imag + increment[7];
      pm_candidate2 = old_path_metrics[13] - input_symbol_imag - increment[0];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[11] = pm_candidate1;
         trans_table[sample_nr][11] = -1.0;
      }
      else{
         new_path_metrics[11] = pm_candidate2;
         trans_table[sample_nr][11] = 1.0;
      }

      pm_candidate1 = old_path_metrics[6] + input_symbol_imag - increment[4];
      pm_candidate2 = old_path_metrics[14] + input_symbol_imag + increment[3];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[12] = pm_candidate1;
         trans_table[sample_nr][12] = -1.0;
      }
      else{
         new_path_metrics[12] = pm_candidate2;
         trans_table[sample_nr][12] = 1.0;
      }

      pm_candidate1 = old_path_metrics[6] - input_symbol_imag + increment[4];
      pm_candidate2 = old_path_metrics[14] - input_symbol_imag - increment[3];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[13] = pm_candidate1;
         trans_table[sample_nr][13] = -1.0;
      }
      else{
         new_path_metrics[13] = pm_candidate2;
         trans_table[sample_nr][13] = 1.0;
      }

      pm_candidate1 = old_path_metrics[7] + input_symbol_imag - increment[5];
      pm_candidate2 = old_path_metrics[15] + input_symbol_imag + increment[2];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[14] = pm_candidate1;
         trans_table[sample_nr][14] = -1.0;
      }
      else{
         new_path_metrics[14] = pm_candidate2;
         trans_table[sample_nr][14] = 1.0;
      }

      pm_candidate1 = old_path_metrics[7] - input_symbol_imag + increment[5];
      pm_candidate2 = old_path_metrics[15] - input_symbol_imag - increment[2];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[15] = pm_candidate1;
         trans_table[sample_nr][15] = -1.0;
      }
      else{
         new_path_metrics[15] = pm_candidate2;
         trans_table[sample_nr][15] = 1.0;
      }

      for(i=0; i<16; i++){
         printf(" %0.6f", new_path_metrics[i]);
      }
      printf("\n");

      tmp=old_path_metrics;
      old_path_metrics=new_path_metrics;
      new_path_metrics=tmp;

      sample_nr++;
      if(sample_nr==samples_num)
         break;

      //Processing real states
      real_imag=0;
      input_symbol_real = input[sample_nr].real();

      pm_candidate1 = old_path_metrics[0] - input_symbol_real - increment[7];
      pm_candidate2 = old_path_metrics[8] - input_symbol_real + increment[0];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[0] = pm_candidate1;
         trans_table[sample_nr][0] = -1.0;
      }
      else{
         new_path_metrics[0] = pm_candidate2;
         trans_table[sample_nr][0] = 1.0;
      }

      pm_candidate1 = old_path_metrics[0] + input_symbol_real + increment[7];
      pm_candidate2 = old_path_metrics[8] + input_symbol_real - increment[0];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[1] = pm_candidate1;
         trans_table[sample_nr][1] = -1.0;
      }
      else{
         new_path_metrics[1] = pm_candidate2;
         trans_table[sample_nr][1] = 1.0;
      }

      pm_candidate1 = old_path_metrics[1] - input_symbol_real - increment[6];
      pm_candidate2 = old_path_metrics[9] - input_symbol_real + increment[1];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[2] = pm_candidate1;
         trans_table[sample_nr][2] = -1.0;
      }
      else{
         new_path_metrics[2] = pm_candidate2;
         trans_table[sample_nr][2] = 1.0;
      }

      pm_candidate1 = old_path_metrics[1] + input_symbol_real + increment[6];
      pm_candidate2 = old_path_metrics[9] + input_symbol_real - increment[1];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[3] = pm_candidate1;
         trans_table[sample_nr][3] = -1.0;
      }
      else{
         new_path_metrics[3] = pm_candidate2;
         trans_table[sample_nr][3] = 1.0;
      }

      pm_candidate1 = old_path_metrics[2] - input_symbol_real - increment[5];
      pm_candidate2 = old_path_metrics[10] - input_symbol_real + increment[2];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[4] = pm_candidate1;
         trans_table[sample_nr][4] = -1.0;
      }
      else{
         new_path_metrics[4] = pm_candidate2;
         trans_table[sample_nr][4] = 1.0;
      }

      pm_candidate1 = old_path_metrics[2] + input_symbol_real + increment[5];
      pm_candidate2 = old_path_metrics[10] + input_symbol_real - increment[2];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[5] = pm_candidate1;
         trans_table[sample_nr][5] = -1.0;
      }
      else{
         new_path_metrics[5] = pm_candidate2;
         trans_table[sample_nr][5] = 1.0;
      }

      pm_candidate1 = old_path_metrics[3] - input_symbol_real - increment[4];
      pm_candidate2 = old_path_metrics[11] - input_symbol_real + increment[3];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[6] = pm_candidate1;
         trans_table[sample_nr][6] = -1.0;
      }
      else{
         new_path_metrics[6] = pm_candidate2;
         trans_table[sample_nr][6] = 1.0;
      }

      pm_candidate1 = old_path_metrics[3] + input_symbol_real + increment[4];
      pm_candidate2 = old_path_metrics[11] + input_symbol_real - increment[3];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[7] = pm_candidate1;
         trans_table[sample_nr][7] = -1.0;
      }
      else{
         new_path_metrics[7] = pm_candidate2;
         trans_table[sample_nr][7] = 1.0;
      }

      pm_candidate1 = old_path_metrics[4] - input_symbol_real - increment[3];
      pm_candidate2 = old_path_metrics[12] - input_symbol_real + increment[4];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[8] = pm_candidate1;
         trans_table[sample_nr][8] = -1.0;
      }
      else{
         new_path_metrics[8] = pm_candidate2;
         trans_table[sample_nr][8] = 1.0;
      }

      pm_candidate1 = old_path_metrics[4] + input_symbol_real + increment[3];
      pm_candidate2 = old_path_metrics[12] + input_symbol_real - increment[4];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[9] = pm_candidate1;
         trans_table[sample_nr][9] = -1.0;
      }
      else{
         new_path_metrics[9] = pm_candidate2;
         trans_table[sample_nr][9] = 1.0;
      }

      pm_candidate1 = old_path_metrics[5] - input_symbol_real - increment[2];
      pm_candidate2 = old_path_metrics[13] - input_symbol_real + increment[5];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[10] = pm_candidate1;
         trans_table[sample_nr][10] = -1.0;
      }
      else{
         new_path_metrics[10] = pm_candidate2;
         trans_table[sample_nr][10] = 1.0;
      }

      pm_candidate1 = old_path_metrics[5] + input_symbol_real + increment[2];
      pm_candidate2 = old_path_metrics[13] + input_symbol_real - increment[5];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[11] = pm_candidate1;
         trans_table[sample_nr][11] = -1.0;
      }
      else{
         new_path_metrics[11] = pm_candidate2;
         trans_table[sample_nr][11] = 1.0;
      }

      pm_candidate1 = old_path_metrics[6] - input_symbol_real - increment[1];
      pm_candidate2 = old_path_metrics[14] - input_symbol_real + increment[6];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[12] = pm_candidate1;
         trans_table[sample_nr][12] = -1.0;
      }
      else{
         new_path_metrics[12] = pm_candidate2;
         trans_table[sample_nr][12] = 1.0;
      }

      pm_candidate1 = old_path_metrics[6] + input_symbol_real + increment[1];
      pm_candidate2 = old_path_metrics[14] + input_symbol_real - increment[6];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[13] = pm_candidate1;
         trans_table[sample_nr][13] = -1.0;
      }
      else{
         new_path_metrics[13] = pm_candidate2;
         trans_table[sample_nr][13] = 1.0;
      }

      pm_candidate1 = old_path_metrics[7] - input_symbol_real - increment[0];
      pm_candidate2 = old_path_metrics[15] - input_symbol_real + increment[7];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[14] = pm_candidate1;
         trans_table[sample_nr][14] = -1.0;
      }
      else{
         new_path_metrics[14] = pm_candidate2;
         trans_table[sample_nr][14] = 1.0;
      }

      pm_candidate1 = old_path_metrics[7] + input_symbol_real + increment[0];
      pm_candidate2 = old_path_metrics[15] + input_symbol_real - increment[7];
      if(pm_candidate1 > pm_candidate2){
         new_path_metrics[15] = pm_candidate1;
         trans_table[sample_nr][15] = -1.0;
      }
      else{
         new_path_metrics[15] = pm_candidate2;
         trans_table[sample_nr][15] = 1.0;
      }

      for(i=0; i<16; i++){
         printf(" %0.6f", new_path_metrics[i]);
      }
      printf("\n");

      tmp=old_path_metrics;
      old_path_metrics=new_path_metrics;
      new_path_metrics=tmp;

      sample_nr++;
   }

/*
* Find the best from the stop states by comparing their path metrics.
* Not every stop state is always possible, so we are searching in
* a subset of them.
*/
   unsigned int best_stop_state;
   float stop_state_metric, max_stop_state_metric;
   best_stop_state = stop_states[0];
   max_stop_state_metric = old_path_metrics[best_stop_state];
   for(i=1; i< stops_num; i++){
      stop_state_metric = old_path_metrics[stop_states[i]];
      if(stop_state_metric > max_stop_state_metric){
         max_stop_state_metric = stop_state_metric;
         best_stop_state = stop_states[i];
      }
   }

/*
* This table was generated with hope that it gives a litle speedup during
* traceback stage. 
* Received bit is related to the number of state in the trellis.
* I've numbered states so their parity (number of ones) is related
* to a received bit. 
*/
   static const unsigned int parity_table[PATHS_NUM] = { 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,  };

/*
* Table of previous states in the trellis diagram.
* For GMSK modulation every state has two previous states.
* Example:
*   previous_state_nr1 = prev_table[current_state_nr][0]
*   previous_state_nr2 = prev_table[current_state_nr][1]
*/
   static const unsigned int prev_table[PATHS_NUM][2] = { {0,8}, {0,8}, {1,9}, {1,9}, {2,10}, {2,10}, {3,11}, {3,11}, {4,12}, {4,12}, {5,13}, {5,13}, {6,14}, {6,14}, {7,15}, {7,15},  };

/*
* Traceback and differential decoding of received sequence.
* Decisions stored in trans_table are used to restore best path in the trellis.
*/
   sample_nr=samples_num;
   unsigned int state_nr=best_stop_state;
   unsigned int decision;
   bool out_bit=0;

   while(sample_nr>0){
      sample_nr--;
      decision = (trans_table[sample_nr][state_nr]>0);

      if(decision != out_bit)
         output[sample_nr]=-trans_table[sample_nr][state_nr];
      else
         output[sample_nr]=trans_table[sample_nr][state_nr];

      out_bit = out_bit ^ real_imag ^ parity_table[state_nr];
      state_nr = prev_table[state_nr][decision];
      real_imag = !real_imag;
   }
}
int main()
{
   gr_complex rhh[5];
   gr_complex input[BURST_SIZE];
   float output[BURST_SIZE];
   float path_metrics[16];
   unsigned int i;

   rhh[0] = gr_complex(6681134.2347451737,0.0000000000);
   rhh[1] = gr_complex(4315167.0637422213,-860367.8252039659);
   rhh[2] = gr_complex(935284.8973972955,-371780.5098627206);
   rhh[3] = gr_complex(-51462.6871850645,50393.6755332753);
   rhh[4] = gr_complex(-7439.8044847587,8028.1598971039);
   input[0] = gr_complex(7977179.5758423340,-2764222.4174382309);
   input[1] = gr_complex(6856794.8374158992,4865869.9717015224);
   input[2] = gr_complex(9811374.7624817826,5183732.8837238941);
   input[3] = gr_complex(6679440.9549223864,-4428971.3826953433);
   input[4] = gr_complex(-4696701.8298753574,-5933567.0446173064);
   input[5] = gr_complex(-6541783.9135319907,4653341.9806263465);
   input[6] = gr_complex(2084689.1360897610,10178136.3338905703);
   input[7] = gr_complex(6147479.6545689385,8874496.3001975510);
   input[8] = gr_complex(5904918.0852984637,8800690.9255027324);
   input[9] = gr_complex(6748883.3597641187,8777701.9989523701);
   input[10] = gr_complex(6778221.8454960939,8045655.9501006966);
   input[11] = gr_complex(6364128.8660152592,8022191.0277904849);
   input[12] = gr_complex(8722069.2811547928,4776436.4937707158);
   input[13] = gr_complex(10932320.6634155307,-1558217.3467115229);
   input[14] = gr_complex(8712591.8296174519,-1858753.9812126879);
   input[15] = gr_complex(2379782.5548789091,2932506.6217711852);
   input[16] = gr_complex(-5444416.7791099474,5946987.8123247186);
   input[17] = gr_complex(-5424576.3013667949,8041132.5497372206);
   input[18] = gr_complex(4060723.5737396451,5559136.6484692302);
   input[19] = gr_complex(9867910.7169802599,-1816506.5368241861);
   input[20] = gr_complex(9116570.9774469398,-6509021.2122100489);
   input[21] = gr_complex(5005755.5616738033,-8815847.5133575164);
   input[22] = gr_complex(-2234914.8963576080,-10346435.2929649707);
   input[23] = gr_complex(-7202255.9594532764,-9713796.9845253937);
   input[24] = gr_complex(-9190470.4906416610,-4700539.2048025727);
   input[25] = gr_complex(-10344677.5631833095,3164735.0763911339);
   input[26] = gr_complex(-8144676.0358214928,3689244.8448072611);
   input[27] = gr_complex(-2187512.8785404381,-2405086.8007661770);
   input[28] = gr_complex(4192422.7355999672,-2302450.1948127761);
   input[29] = gr_complex(7246362.6484694732,4298058.7434391864);
   input[30] = gr_complex(6556902.8967612814,8229181.3319309540);
   input[31] = gr_complex(6142925.0144970221,8026586.9547691522);
   input[32] = gr_complex(8129663.3146255473,4033126.4267871310);
   input[33] = gr_complex(5840297.4751724107,-4620492.6748427916);
   input[34] = gr_complex(-1914675.1103647740,-10020347.7468968201);
   input[35] = gr_complex(-2940115.4495635680,-8372239.2818623800);
   input[36] = gr_complex(4008522.1739267618,-6826920.4027512996);
   input[37] = gr_complex(8135500.9140075976,-7047328.2304275325);
   input[38] = gr_complex(8405763.8657652065,-6931698.9773707138);
   input[39] = gr_complex(8929684.6084167603,-6179537.9078161716);
   input[40] = gr_complex(8180060.2347036134,-2186703.0895569432);
   input[41] = gr_complex(3091335.4318671110,4054954.8339853790);
   input[42] = gr_complex(-4295697.1578283813,7927174.1333465036);
   input[43] = gr_complex(-8473978.8917866647,8708279.0942377765);
   input[44] = gr_complex(-8307511.0194505267,8912587.7116757408);
   input[45] = gr_complex(-3924398.3662469061,9788935.7640386801);
   input[46] = gr_complex(3578818.1363314060,9475507.7755236719);
   input[47] = gr_complex(7878887.0608754344,8305521.6216092538);
   input[48] = gr_complex(7661073.4441334298,8193166.0088053150);
   input[49] = gr_complex(7711201.6565008871,8476321.8633535057);
   input[50] = gr_complex(8106392.3937723571,8574168.6601282097);
   input[51] = gr_complex(4338938.6736003347,8161180.2535219444);
   input[52] = gr_complex(-2892764.5596533259,7908598.1890047276);
   input[53] = gr_complex(-7361426.2291188771,8354406.9966570400);
   input[54] = gr_complex(-7399748.1998513071,9027400.1381105371);
   input[55] = gr_complex(-2985443.1269021751,10343848.6311076693);
   input[56] = gr_complex(4056164.2682814910,10098505.0940024592);
   input[57] = gr_complex(3662304.4839048819,7612823.1212739553);
   input[58] = gr_complex(-3973366.8165788800,7937078.9175711880);
   input[59] = gr_complex(-4235798.1452934938,10334281.0868006106);
   input[60] = gr_complex(3036808.0689534992,9898067.6488185041);
   input[61] = gr_complex(7473543.5356334411,7807926.3775777007);
   input[62] = gr_complex(9079322.0398128927,3406424.4956655651);
   input[63] = gr_complex(9858644.6258316785,-3257437.0358347511);
   input[64] = gr_complex(8880394.8906779401,-7679278.0091405679);
   input[65] = gr_complex(3363837.0022190632,-9429173.7162515502);
   input[66] = gr_complex(-5976744.0335793961,-5385450.6034509651);
   input[67] = gr_complex(-9557981.0088688023,3050312.8082751348);
   input[68] = gr_complex(-7022882.0701328497,3640309.4189949362);
   input[69] = gr_complex(-6956342.7005542396,-3448204.4347535190);
   input[70] = gr_complex(-7352343.4183843154,-7661573.7851171279);
   input[71] = gr_complex(-2991151.1712345490,-7200411.9186981265);
   input[72] = gr_complex(2885118.0445274930,-2984883.7704812000);
   input[73] = gr_complex(7032828.1064425148,3581754.7941750460);
   input[74] = gr_complex(10088763.7540586796,3662867.8467899580);
   input[75] = gr_complex(10564181.4445688296,-3225930.1012457302);
   input[76] = gr_complex(8350222.2591632884,-3732983.7871769890);
   input[77] = gr_complex(7833973.7142023239,2987993.6910825032);
   input[78] = gr_complex(9706408.6739433222,3052094.8092914429);
   input[79] = gr_complex(9745006.6094854400,-3935102.0540104648);
   input[80] = gr_complex(7816404.5637982106,-8252538.7267344501);
   input[81] = gr_complex(2986277.6633892348,-9487975.5299777929);
   input[82] = gr_complex(-5695845.6292771958,-5169515.8120368905);
   input[83] = gr_complex(-10054151.1938444097,3411496.4582903101);
   input[84] = gr_complex(-8051379.2657383084,2941255.2196879322);
   input[85] = gr_complex(-3549464.5857489500,-3756761.7516137050);
   input[86] = gr_complex(2758228.0974008231,-3356159.6927887732);
   input[87] = gr_complex(3030904.7997136060,2423714.1793336859);
   input[88] = gr_complex(-4312484.6648249906,6565622.5327912308);
   input[89] = gr_complex(-8292628.1023570709,7907719.3416422373);
   input[90] = gr_complex(-7526932.9695930025,8014555.7987554707);
   input[91] = gr_complex(-7845877.0589824449,7739528.8884838661);
   input[92] = gr_complex(-8621562.5281414296,7418568.3737750174);
   input[93] = gr_complex(-8417158.0598517433,7181224.2163500283);
   input[94] = gr_complex(-8512688.8578708004,7987622.6814399734);
   input[95] = gr_complex(-4394195.2836061195,9949034.4722546078);
   input[96] = gr_complex(5266129.2915991358,6195329.6856880505);
   input[97] = gr_complex(10194253.6950446907,-2784736.7088111811);
   input[98] = gr_complex(8824963.8092368357,-7786593.4880886609);
   input[99] = gr_complex(3396738.8080786392,-9691138.5840958729);
   input[100] = gr_complex(-5722550.5051566781,-5556953.7173151653);
   input[101] = gr_complex(-5714701.2187169204,5472711.9084113399);
   input[102] = gr_complex(3332017.2621497451,9704527.7971289307);
   input[103] = gr_complex(7342514.5288135549,7753950.5122191338);
   input[104] = gr_complex(6818048.9879954616,8189460.3032278512);
   input[105] = gr_complex(3394201.9642735380,7945062.7654534997);
   input[106] = gr_complex(-4167885.5119835120,6895554.1659005154);
   input[107] = gr_complex(-8619065.5479442235,6801211.7162556173);
   input[108] = gr_complex(-7427938.6338890027,2908143.9905314222);
   input[109] = gr_complex(-6703266.9973377967,-4149571.8933691182);
   input[110] = gr_complex(-7397979.4083190663,-8091927.8271739502);
   input[111] = gr_complex(-8023643.3861837359,-8148580.2711619306);
   input[112] = gr_complex(-9593984.7424030975,-3401373.7567658830);
   input[113] = gr_complex(-9949905.4782167133,3693487.8505923129);
   input[114] = gr_complex(-7643884.0559280366,3256209.4725131588);
   input[115] = gr_complex(-2697566.6721224338,-3206105.0871117339);
   input[116] = gr_complex(3377896.5469162161,-3637995.9373872238);
   input[117] = gr_complex(7053276.0410971297,2764169.1462154472);
   input[118] = gr_complex(8628643.1609179415,3396969.9720182270);
   input[119] = gr_complex(8591483.6933623832,-3571890.2316942490);
   input[120] = gr_complex(7631932.3620410990,-7892583.3345261374);
   input[121] = gr_complex(3523270.0499849520,-9235294.6299457569);
   input[122] = gr_complex(-5210049.1886767643,-5470018.0261554671);
   input[123] = gr_complex(-5729032.9836538611,4597120.7055899110);
   input[124] = gr_complex(4722045.6474561440,5593554.0462966477);
   input[125] = gr_complex(9943005.2302960120,-3058974.8321727011);
   input[126] = gr_complex(7338276.2722858489,-3095524.7280114950);
   input[127] = gr_complex(6924101.7894357592,4679671.4725222811);
   input[128] = gr_complex(9394305.4008450378,4311128.9848157102);
   input[129] = gr_complex(9835558.7182324342,-3667900.0077023208);
   input[130] = gr_complex(8223108.7120001484,-8579057.2975662407);
   input[131] = gr_complex(2845955.6588687100,-9157587.9910129588);
   input[132] = gr_complex(-5965686.3459505001,-4635464.3736052224);
   input[133] = gr_complex(-9515672.9792508632,3206974.9185256958);
   input[134] = gr_complex(-8344649.1418864401,7073463.4248667136);
   input[135] = gr_complex(-8254731.1990081621,7048210.7404652704);
   input[136] = gr_complex(-7504645.1065991390,3469527.9779098351);
   input[137] = gr_complex(-6905884.6544481078,-4071500.7763075172);
   input[138] = gr_complex(-7265828.0211983416,-8724546.7090917714);
   input[139] = gr_complex(-3159001.5446096850,-7599002.0958460663);
   input[140] = gr_complex(4820872.7249604221,-7688968.4836491412);
   input[141] = gr_complex(4538964.4842799734,-10655536.8874594998);
   input[142] = gr_complex(-4627802.4763165191,-6342339.7226147177);
   input[143] = gr_complex(-5229789.9845531741,4793436.5098953182);
   input[144] = gr_complex(3023641.9008085518,9426626.5553139690);
   input[145] = gr_complex(3151170.4858403988,7510483.1565964483);
   input[146] = gr_complex(-3112381.9596595298,6480539.7998312227);
   input[147] = gr_complex(-3526072.3028905988,4305425.6301188134);
   unsigned int stop_states[1] = { 4,  };

   viterbi_detector(input, BURST_SIZE, rhh, 3, stop_states, 1, output);
      printf("# name: output\n# type: matrix\n# rows: 1\n# columns: 148\n");
      for(i=0; i<BURST_SIZE ; i++){
         printf(" %d\n", output[i]>0);
      }
      printf("\n");

}
