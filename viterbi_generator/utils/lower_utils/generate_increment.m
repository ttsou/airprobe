function [INCREMENT NUMBERS] = generate_increment(SYMBOLS,PREVIOUS)%,Rhh)

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
 
MSK_STATES_NUM=4; % 1,-1,j,-j
[POSIBLE_SEQ_NUM,Lh]=size(SYMBOLS); 
INCREMENT_NUM=2**(Lh-1);
%Lh - channel memory length
%POSIBLE_SEQ_NUM - number of posible sequences of MSK symbols (1,-1,j,-j) for given Lh

%INCREMENT=zeros(2,Lh,POSIBLE_SEQ_NUM/8);

% Rhh IS STORED AS:
% [ Rhh(1) Rhh(2) Rhh(3) ... Rhh(Lh) ]
INCREMENT=zeros(INCREMENT_NUM, Lh);

for n=1:POSIBLE_SEQ_NUM,
  % ONLY TWO LEGAL PREVIOUS STATES EXIST, SO THE LOOP IS UNROLLED
  m=PREVIOUS(n,1);
  vector=[conj(SYMBOLS(n,1))* SYMBOLS(m,:)];
  number=complex_vect2number(vector);
  if(number>0)
    INCREMENT(number,:)=vector;
  end

  NUMBERS(m,n)=number;

  m=PREVIOUS(n,2);
  vector=[conj(SYMBOLS(n,1))* SYMBOLS(m,:)];
  number=complex_vect2number(vector);
  if(number>0)
    INCREMENT(number,:)=vector;
  end  
  NUMBERS(m,n)=number;  
end

%test_inc=zeros(2**(Lh+1),2**(Lh+1));
%for n=1:2**(Lh+1),
%  for m=1:2**(Lh+1),
%    number=NUMBERS(m,n);

%    if(number!=0)
%      sign=1;
%      if(number<0),
%        sign=-1;
%        number=-number;
%      end
%      test_inc(m,n)=real(sign*INCREMENT(number,:)*Rhh(2:Lh+1).');
%    end
%  end
%end
%INCREMENT
%test_inc
%NUMBERS
