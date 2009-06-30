#!/usr/bin/octave --silent

addpath("./tests/utils");
addpath("./utils");
addpath("./utils/lower_utils");
addpath("./utils/lower_utils/lower_utils");
arg_list=argv();
Lh=sscanf(arg_list{1}, "%d", "C");

INCREMENT_NUM=2**(Lh-1);
BRANCH_NUM=2**(Lh+1);
PATHS_NUM=2**(Lh);

load tests/data/rhh.dat;
load tests/data/normal_burst.dat;

[r BURST_SIZE] = size(normal_burst);

SYMBOLS = make_symbols(Lh);
START = make_start(Lh,SYMBOLS);
STOPS = make_stops(Lh,SYMBOLS);

[increment,  pm_candidates_imag, pm_candidates_real] = equations_gen(Lh);

f_name="test";
test_file = fopen([f_name '.cpp'],'w');

print_matrix_type=sprintf("   printf(\"# name: path_metrics_test_result\\n# type: matrix\\n# rows: %d\\n# columns: %d\\n\");\n", BURST_SIZE, PATHS_NUM);;

print_path_metrics=[ "\n\
      for(i=0; i<" int2str(PATHS_NUM) "; i++){\n\
         printf(\" %0.6f\", new_path_metrics[i]);\n\
      }\n\
      printf(\"\\n\");\n\n" ];

viterbi_detector = viterbi_generator(Lh,  print_matrix_type, print_path_metrics);

fprintf(test_file, "\
#include <stdio.h>\n", BURST_SIZE);

fprintf(test_file, "%s", viterbi_detector);

fprintf(test_file, [ "\
int main()\n\
{\n\
   gr_complex rhh[" int2str(Lh+1) "];\n\
   gr_complex input[BURST_SIZE];\n\
   float output[BURST_SIZE];\n\
   float path_metrics[" int2str(PATHS_NUM) "];\n\
   unsigned int i;\n\n" ]);

for i=1:Lh+1,
  fprintf(test_file,"   rhh[%d] = gr_complex(%0.10f,%0.10f);\n",i-1, real(Rhh(i)), imag(Rhh(i)));
end

for i=1:BURST_SIZE,
  fprintf(test_file,"   input[%d] = gr_complex(%0.10f,%0.10f);\n",i-1, real(normal_burst(i)), imag(normal_burst(i)));
end

[c STOPS_NUM] = size(STOPS);

stop_states=["   unsigned int stop_states[" int2str(STOPS_NUM) "] = { "];
for i=1:STOPS_NUM,
  stop_states=[ stop_states int2str(ceil(STOPS(i)/2)-1) ", "];
end
stop_states=[ stop_states " };\n\n" ];

fprintf(test_file, stop_states);

fprintf(test_file, ["   viterbi_detector(input, BURST_SIZE, rhh, " int2str(ceil(START/2)-1) ", stop_states, " int2str(STOPS_NUM) ", output);" ]);

print_output=[ "\n\
      printf(\"# name: output\\n# type: matrix\\n# rows: 1\\n# columns: " int2str(BURST_SIZE) "\\n\");\n\
      for(i=0; i<BURST_SIZE ; i++){\n\
         printf(\" %d\\n\", output[i]>0);\n\
      }\n\
      printf(\"\\n\");\n" ];

fprintf(test_file, "%s", print_output);

fprintf(test_file, "\n}\n");


