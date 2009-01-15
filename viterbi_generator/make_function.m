#!/usr/bin/octave --silent

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
 
addpath("./utils");
addpath("./utils/lower_utils");
addpath("./utils/lower_utils/lower_utils");
arg_list=argv();
Lh=sscanf(arg_list{1}, "%d", "C");
license_statement="\
/***************************************************************************\n\
 *   Copyright (C) 2008 by Piotr Krysik                                    *\n\
 *   pkrysik@stud.elka.pw.edu.pl                                           *\n\
 *                                                                         *\n\
 *   This program is free software; you can redistribute it and/or modify  *\n\
 *   it under the terms of the GNU General Public License as published by  *\n\
 *   the Free Software Foundation; either version 2 of the License, or     *\n\
 *   (at your option) any later version.                                   *\n\
 *                                                                         *\n\
 *   This program is distributed in the hope that it will be useful,       *\n\
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *\n\
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *\n\
 *   GNU General Public License for more details.                          *\n\
 *                                                                         *\n\
 *   You should have received a copy of the GNU General Public License     *\n\
 *   along with this program; if not, write to the                         *\n\
 *   Free Software Foundation, Inc.,                                       *\n\
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *\n\
 ***************************************************************************/\n\n";
f_name="viterbi_detector";
viterbi_detector_file = fopen([f_name '.cpp'],'w');
viterbi_detector = viterbi_generator(Lh);
fprintf(viterbi_detector_file, "%s", license_statement);
fprintf(viterbi_detector_file, "%s", viterbi_detector);


