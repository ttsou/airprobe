function [viterbi_detector] = viterbi_generator(Lh, varargin)

 ###########################################################################
 #   Copyright (C) 2008 by Piotr Krysik                                    #
 #   pkrysik@stud.elka.pw.edu.pl                                           #
 #                                                                         #
 #   This program is free software; you can redistribute it and/or modify  #
 #   it under the terms of the GNU General Public License as published by  #
 #   the Free Software Foundation; either version 3 of the License, or     #
 #   (at your option) any later version.                                   #
 #                                                                         #
 #   This program is distributed in the hope that it will be useful,       #
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
 #   GNU General Public License for more details.                          #
 #                                                                         #
 #   You should have received a copy of the GNU General Public License     #
 #   along with this program; if not, write to the                         #
 #   Free Software Foundation, Inc.,                                       #
 #   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
 ###########################################################################
 
if(nargin!=1 && nargin !=3),
  printf("Bad number of arguments in viterbi_generator(...)");
  exit(1)
elseif(nargin==1)
  print_matrix_type="";
  print_path_metrics="";
elseif(nargin==3)
  %additional parameters for test
  print_matrix_type=varargin{1};
  print_path_metrics=varargin{2};
end

INCREMENT_NUM=2**(Lh-1);
BRANCH_NUM=2**(Lh+1);
PATHS_NUM=2**(Lh);
PREVIOUS = generate_previous(Lh);

[increment,  pm_candidates_imag, pm_candidates_real] = equations_gen(Lh);

function_interface_comment="\
/*\n\
** viterbi_detector:\n\
**           This part does the detection of received sequnece.\n\
**           Employed algorithm is viterbi Maximum Likehood Sequence Estimation.\n\
**           At this moment it gives hard decisions on the output, but\n\
**           it was designed with soft decisions in mind.\n\
**\n\
** SYNTAX:   void viterbi_detector(\n\
**                                  const gr_complex * input, \n\
**                                  unsigned int samples_num, \n\
**                                  gr_complex * rhh, \n\
**                                  unsigned int start_state, \n\
**                                  const unsigned int * stop_states, \n\
**                                  unsigned int stops_num, \n\
**                                  float * output)\n\
**\n\
** INPUT:    input:       Complex received signal afted matched filtering.\n\
**           samples_num: Number of samples in the input table.\n\
**           rhh:         The autocorrelation of the estimated channel \n\
**                        impulse response.\n\
**           start_state: Number of the start point. In GSM each burst \n\
**                        starts with sequence of three bits (0,0,0) which \n\
**                        indicates start point of the algorithm.\n\
**           stop_states: Table with numbers of possible stop states.\n\
**           stops_num:   Number of possible stop states\n\
**                     \n\
**\n\
** OUTPUT:   output:      Differentially decoded hard output of the algorithm: \n\
**                        -1 for logical \"0\" and 1 for logical \"1\"\n\
**\n\
** SUB_FUNC: none\n\
**\n\
** TEST(S):  Tested with real world normal burst.\n\
*/\n\n";

beginning=[ "\
#include <gnuradio/gr_complex.h>\n\
#define BURST_SIZE 148\n\
#define PATHS_NUM " int2str(PATHS_NUM) "\n\
\n\
void viterbi_detector(const gr_complex * input, unsigned int samples_num, gr_complex * rhh, unsigned int start_state, const unsigned int * stop_states, unsigned int stops_num, float * output)\n\
{\n\
   float increment[" int2str(INCREMENT_NUM) "];\n\
   float path_metrics1[" int2str(PATHS_NUM) "];\n\
   float path_metrics2[" int2str(PATHS_NUM) "];\n\
   float * new_path_metrics;\n\
   float * old_path_metrics;\n\
   float * tmp;\n\
   float trans_table[BURST_SIZE][" int2str(PATHS_NUM) "];\n\
   float pm_candidate1, pm_candidate2;\n\
   bool real_imag;\n\
   float input_symbol_real, input_symbol_imag;\n\
   unsigned int i, sample_nr;\n\n" ];



start_point_comment="\
/*\n\
* Setup first path metrics, so only state pointed by start_state is possible.\n\
* Start_state metric is equal to zero, the rest is written with some very low value,\n\
* which makes them practically impossible to occur.\n\
*/\n";

start_point=[ "\
   for(i=0; i<PATHS_NUM; i++){\n\
      path_metrics1[i]=(-10e30);\n\
   }\n\
   path_metrics1[start_state]=0;\n\n" ];



increment_comment="\
/*\n\
* Compute Increment - a table of values which does not change for subsequent input samples.\n\
* Increment is table of reference levels for computation of branch metrics:\n\
*    branch metric = (+/-)received_sample (+/-) reference_level\n\
*/\n";

increment_block="";
for i=1:INCREMENT_NUM,
  increment_block = [increment_block sprintf("   %s;\n",increment(i))];
end



path_metrics_comment="\n\n\
/*\n\
* Computation of path metrics and decisions (Add-Compare-Select).\n\
* It's composed of two parts: one for odd input samples (imaginary numbers)\n\
* and one for even samples (real numbers).\n\
* Each part is composed of independent (parallelisable) statements like  \n\
* this one:\n\
*      pm_candidate1 = old_path_metrics[0] - input_symbol_real - increment[7];\n\
*      pm_candidate2 = old_path_metrics[8] - input_symbol_real + increment[0];\n\
*      if(pm_candidate1 > pm_candidate2){\n\
*         new_path_metrics[0] = pm_candidate1;\n\
*         trans_table[sample_nr][0] = -1.0;\n\
*      }\n\
*      else{\n\
*         new_path_metrics[0] = pm_candidate2;\n\
*         trans_table[sample_nr][0] = 1.0;\n\
*      }\n\
* This is very good point for optimisations (SIMD or OpenMP) as it's most time \n\
* consuming part of this function. \n\
*/\n";

path_metrics_block="   sample_nr=0;\n";

path_metrics_block=[path_metrics_block "   old_path_metrics=path_metrics1;\n"];
path_metrics_block=[path_metrics_block "   new_path_metrics=path_metrics2;\n"];

path_metrics_block=[path_metrics_block "\
   while(sample_nr<samples_num){\n\
      //Processing imag states\n\
      real_imag=1;\n\
      input_symbol_imag = input[sample_nr].imag();\n"];


    for i=1:PATHS_NUM,
      path_metrics_block=[path_metrics_block sprintf("\n      %s;\n      %s;\n", pm_candidates_imag(i, 1), pm_candidates_imag(i, 2)) ];

      path_metrics_block=[path_metrics_block "\
      if(pm_candidate1 > pm_candidate2){\n\
         new_path_metrics[" int2str(i-1) "] = pm_candidate1;\n\
         trans_table[sample_nr][" int2str(i-1) "] = -1.0;\n\
      }\n\
      else{\n\
         new_path_metrics[" int2str(i-1) "] = pm_candidate2;\n\
         trans_table[sample_nr][" int2str(i-1) "] = 1.0;\n\
      }\n"];
    end

    path_metrics_block=[path_metrics_block print_path_metrics];
    %change new metrics into old metrics
    path_metrics_block=[path_metrics_block "      tmp=old_path_metrics;\n"];
    path_metrics_block=[path_metrics_block "      old_path_metrics=new_path_metrics;\n"];
    path_metrics_block=[path_metrics_block "      new_path_metrics=tmp;\n\n"];

    path_metrics_block=[path_metrics_block "      sample_nr++;\n"];
    path_metrics_block=[path_metrics_block "      if(sample_nr==samples_num)\n         break;\n\n"];
    path_metrics_block=[path_metrics_block "      //Processing real states\n"];
    path_metrics_block=[path_metrics_block "      real_imag=0;\n"];
    path_metrics_block=[path_metrics_block "      input_symbol_real = input[sample_nr].real();\n"];
    for i=1:PATHS_NUM,
      path_metrics_block=[path_metrics_block sprintf("\n      %s;\n      %s;\n", pm_candidates_real(i, 1), pm_candidates_real(i, 2)) ];
      path_metrics_block=[path_metrics_block "\
      if(pm_candidate1 > pm_candidate2){\n\
         new_path_metrics[" int2str(i-1) "] = pm_candidate1;\n\
         trans_table[sample_nr][" int2str(i-1) "] = -1.0;\n\
      }\n\
      else{\n\
         new_path_metrics[" int2str(i-1) "] = pm_candidate2;\n\
         trans_table[sample_nr][" int2str(i-1) "] = 1.0;\n\
      }\n"];
    end
    path_metrics_block=[path_metrics_block print_path_metrics];
    %change new metrics into old metrics
    path_metrics_block=[path_metrics_block "      tmp=old_path_metrics;\n"];
    path_metrics_block=[path_metrics_block "      old_path_metrics=new_path_metrics;\n"];
    path_metrics_block=[path_metrics_block "      new_path_metrics=tmp;\n"];

    path_metrics_block=[path_metrics_block "\n      sample_nr++;\n"];

path_metrics_block=[path_metrics_block "   }\n\n"];



find_best_stop_comment="\
/*\n\
* Find the best from the stop states by comparing their path metrics.\n\
* Not every stop state is always possible, so we are searching in\n\
* a subset of them.\n\
*/\n";

find_best_stop=[                "   unsigned int best_stop_state;\n"];
find_best_stop=[ find_best_stop "   float stop_state_metric, max_stop_state_metric;\n" ];
find_best_stop=[ find_best_stop "   best_stop_state = stop_states[0];\n"];
find_best_stop=[ find_best_stop "   max_stop_state_metric = old_path_metrics[best_stop_state];\n"];
find_best_stop=[ find_best_stop "   for(i=1; i< stops_num; i++){\n"];
find_best_stop=[ find_best_stop "      stop_state_metric = old_path_metrics[stop_states[i]];\n"];
find_best_stop=[ find_best_stop "      if(stop_state_metric > max_stop_state_metric){\n"];
find_best_stop=[ find_best_stop "         max_stop_state_metric = stop_state_metric;\n"];
find_best_stop=[ find_best_stop "         best_stop_state = stop_states[i];\n"];
find_best_stop=[ find_best_stop "      }\n"];
find_best_stop=[ find_best_stop "   }\n\n"];



parity_table_comment="\
/*\n\
* This table was generated with hope that it gives a litle speedup during\n\
* traceback stage. \n\
* Received bit is related to the number of state in the trellis.\n\
* I've numbered states so their parity (number of ones) is related\n\
* to a received bit. \n\
*/\n";

parity_table=["   static const unsigned int parity_table[PATHS_NUM] = { "];
for i=1:PATHS_NUM/4,
  parity_table=[   parity_table   "0, 1, 1, 0, " ]; %int2str(even_parity(i-1))
end
parity_table=[   parity_table " };\n\n" ];



prev_table_comment="\
/*\n\
* Table of previous states in the trellis diagram.\n\
* For GMSK modulation every state has two previous states.\n\
* Example:\n\
*   previous_state_nr1 = prev_table[current_state_nr][0]\n\
*   previous_state_nr2 = prev_table[current_state_nr][1]\n\
*/\n";

prev=ceil(PREVIOUS(1:2:2**(Lh+1),:)/2);
prev_table=["   static const unsigned int prev_table[PATHS_NUM][2] = { "];
for i=1:PATHS_NUM,
  prev_table=[   prev_table "{" int2str(prev(i,1)-1) ","  int2str(prev(i,2)-1)  "}, " ];
end
prev_table=[   prev_table " };\n\n" ];



traceback_comment="\
/*\n\
* Traceback and differential decoding of received sequence.\n\
* Decisions stored in trans_table are used to restore best path in the trellis.\n\
*/\n";

traceback = [          "   sample_nr=samples_num;\n"];
traceback = [traceback "   unsigned int state_nr=best_stop_state;\n"];
traceback = [traceback "   unsigned int decision;\n"];
traceback = [traceback "   bool out_bit=0;\n\n"];
traceback = [traceback "   while(sample_nr>0){\n" ];
traceback = [traceback "      sample_nr--;\n" ];
traceback = [traceback "      decision = (trans_table[sample_nr][state_nr]>0);\n\n" ];

traceback = [traceback "      if(decision != out_bit)\n"];
traceback = [traceback "         output[sample_nr]=-trans_table[sample_nr][state_nr];\n" ];
traceback = [traceback "      else\n" ];
traceback = [traceback "         output[sample_nr]=trans_table[sample_nr][state_nr];\n\n" ];

traceback = [traceback "      out_bit = out_bit ^ real_imag ^ parity_table[state_nr];\n" ];
traceback = [traceback "      state_nr = prev_table[state_nr][decision];\n" ];
traceback = [traceback "      real_imag = !real_imag;\n"];
traceback = [traceback "   }\n" ];



end_of_viterbi_detector = [ "}\n" ];

viterbi_detector=[function_interface_comment \
                  beginning \
                  start_point_comment start_point \
                  increment_comment increment_block \
                  path_metrics_comment print_matrix_type path_metrics_block \
                  find_best_stop_comment find_best_stop \
                  parity_table_comment parity_table \
                  prev_table_comment prev_table \
                  traceback_comment traceback \
                  end_of_viterbi_detector];


