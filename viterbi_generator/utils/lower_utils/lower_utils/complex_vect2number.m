function [ number ] = complex_vect2number(vector)
 
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

% Change complex vector of values: 1,-1 or j,-j 
% into a integer number.

  number=0;
  [r,Lh] = size(vector);
  for l=1:Lh,
    position=2**(l-1);
    if(vector(l)==1),
      number=number+position*(1);
    elseif(vector(l)==-1),
      number=number+position*(-1);
    elseif(vector(l)==-j),
      number=number+position*(1);      
    elseif(vector(l)==j),
      number=number+position*(-1);      
    end    
  end
  if number < 0,
    number=floor(number/2);
  else
    number=ceil(number/2);
  end
