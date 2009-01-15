function [ SYMBOLS ] = make_symbols(Lh)

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
 
BRANCH_NUM=2**(Lh+1);
SYMBOLS=ones(BRANCH_NUM, Lh);

for column=1:Lh,
  for i=1:(2**column),
    SYMBOLS(i:(2**(column+1)):BRANCH_NUM,column)=-1;
  end
 
end

SYMBOLS(1:2:BRANCH_NUM, 1:2:Lh) = SYMBOLS(1:2:BRANCH_NUM, 1:2:Lh).*(-j);
SYMBOLS(2:2:BRANCH_NUM, 2:2:Lh) = SYMBOLS(2:2:BRANCH_NUM, 2:2:Lh).*(-j); 

