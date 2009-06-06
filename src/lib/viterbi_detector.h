/***************************************************************************
 *   Copyright (C) 2008 by Piotr Krysik                                    *
 *   pkrysik@stud.elka.pw.edu.pl                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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

void viterbi_detector(const gr_complex * input, unsigned int samples_num, gr_complex * rhh, unsigned int start_state, const unsigned int * stop_states, unsigned int stops_num, float * output);
