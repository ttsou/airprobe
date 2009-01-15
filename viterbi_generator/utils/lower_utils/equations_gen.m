function [increment, pm_candidates_imag, pm_candidates_real]= equations_gen(Lh)

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

INCREMENT_NUM=2**(Lh-1);

SYMBOLS = make_symbols(Lh);
PREVIOUS = generate_previous(Lh);

load tests/data/rhh.dat

[INCREMENT NUMBERS] = generate_increment(SYMBOLS,PREVIOUS);

function_real="real";
function_imag="imag";
sign="";
fction="";
equation_num="";
increment =  { };
equation="";

for equation_num=1:INCREMENT_NUM,
  for part_num=1:Lh,
    coefficient=INCREMENT(equation_num,part_num);
    if(coefficient==1 || coefficient==-1),
      fction=function_real;
    else
      fction=function_imag;
      coefficient=coefficient*j;
    end
 
    if(coefficient==1 && part_num != 1),
      sign="+";
    elseif(coefficient==-1),
      sign="-";
    else 
      sign="";
    end
 
    equation = [equation " " sign "rhh[" int2str(part_num) "]." fction "()" ];
  end
  
  increment(equation_num) = ["increment[" int2str(equation_num-1) "] =" equation];
  equation="";
end


BRANCH_NUM=2**(Lh+1);


%make path metrics
pm_candidates_real={};
pm_candidates_imag={};
for symbol_num=1:BRANCH_NUM,
  
  inc1_num=NUMBERS(PREVIOUS(symbol_num,1),symbol_num);
  inc2_num=NUMBERS(PREVIOUS(symbol_num,2),symbol_num);
  
  symbol=SYMBOLS(symbol_num,1);
  
  if(inc1_num<0)
    inc1_sign="+";
    inc1_num=-inc1_num;
  else
    inc1_sign="-";
  end

  if(inc2_num<0)
    inc2_sign="+";
    inc2_num=-inc2_num;
  else
    inc2_sign="-";
  end

  inc1_num=inc1_num-1;
  inc2_num=inc2_num-1;

  if(symbol==1 || symbol==-1),
    fction=function_real;
  else
    fction=function_imag;
    symbol=symbol*(-j);
  end
  
  if(symbol==1),
    symbol_sign="+";
  else
    symbol_sign="-";
  end  

  branch_metric1=[" " symbol_sign " input_symbol_" fction " " inc1_sign " increment[" int2str(inc1_num) "]" ];
  branch_metric2=[" " symbol_sign " input_symbol_" fction " " inc2_sign " increment[" int2str(inc2_num) "]" ];
  
  num = ceil((symbol_num)/2);
  prev1_num = ceil((PREVIOUS(symbol_num,1))/2)-1;
  prev2_num = ceil((PREVIOUS(symbol_num,2))/2)-1;  
  
  if(mod(symbol_num,2)==1),
    pm_candidates_imag(num, 1)=[ "pm_candidate1 = old_path_metrics[" int2str(prev1_num) "]" branch_metric1 ];
    pm_candidates_imag(num, 2)=[ "pm_candidate2 = old_path_metrics[" int2str(prev2_num) "]" branch_metric2 ];
  else
    pm_candidates_real(num, 1)=[ "pm_candidate1 = old_path_metrics[" int2str(prev1_num) "]" branch_metric1 ];
    pm_candidates_real(num, 2)=[ "pm_candidate2 = old_path_metrics[" int2str(prev2_num) "]" branch_metric2 ];
  end
end

