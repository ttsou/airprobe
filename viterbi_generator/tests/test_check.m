#!/usr/bin/octave --silent
addpath("./tests/utils");
addpath("./utils");
addpath("./utils/lower_utils");
addpath("./utils/lower_utils/lower_utils");

arg_list=argv();
Lh=sscanf(arg_list{1}, "%d", "C");
PATHS_NUM=2**(Lh);
load tests/data/rhh.dat;
load tests/data/normal_burst.dat;
load test_result;

[r BURST_SIZE] = size(normal_burst);

[ SYMBOLS , PREVIOUS , NEXT , START , STOPS ] = viterbi_init(Lh);

path_metrics_test_result = path_metrics_test_result.';
[rx_burst METRIC] = viterbi_detector(SYMBOLS,NEXT,PREVIOUS,START,STOPS,normal_burst,Rhh);
[r,burst_size] = size(METRIC);
sum_check=0; %number of errors

%find branch metrics computation errors number (errors bigger than 1000)
for i=1:2:burst_size,
  check_matrix = (abs(METRIC(1:2:r,i)-path_metrics_test_result(:,i)));
  suma=sum( check_matrix > ones(PATHS_NUM,1)*1000);
  sum_check = sum_check + suma;
  
  check_matrix = (abs(METRIC(2:2:r,i+1)-path_metrics_test_result(:,i+1)));
  suma=sum( check_matrix > ones(PATHS_NUM,1)*1000);
  sum_check = sum_check + suma;
end

%compute expected number of errors (errors from the beginning)
q=2;
expected_result=0;
while(q<PATHS_NUM),
  expected_result=expected_result + (PATHS_NUM-q);
  q=q*2;
end

%check result
if(sum_check == expected_result),
  printf("Path metrics test: ok\n");
else
  sum_check
  printf("Path metrics test: failed\n");  
end

%check output of the viterbi_detector function
if(sum(output==rx_burst) == BURST_SIZE),
  printf("Final result test: ok\n");
else
  printf("Final result test: failed\n");  
end

