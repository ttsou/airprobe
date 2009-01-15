function [ PREVIOUS ] = generate_previous(Lh)

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
 
SYMBOLS_NUM = 2**(Lh+1);
n=1;
for i=1:4:SYMBOLS_NUM,
    PREVIOUS(i,1) = n+1;
    PREVIOUS(i,2) = n+SYMBOLS_NUM/2+1;
    PREVIOUS(i+1,1) = n;
    PREVIOUS(i+1,2) = n+SYMBOLS_NUM/2;
    PREVIOUS(i+2,1) = n+1;
    PREVIOUS(i+2,2) = n+SYMBOLS_NUM/2+1;
    PREVIOUS(i+3,1) = n;
    PREVIOUS(i+3,2) = n+SYMBOLS_NUM/2;    
    n=n+2;
end
