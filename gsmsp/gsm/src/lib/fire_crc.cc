/* -*- c++ -*- */
/*
 * Copyright 2005 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "fire_crc.h"
#include <math.h>
#include <iostream>

fire_crc::fire_crc (unsigned int crc_size, unsigned int data_size)
  : d_crc_size(crc_size),
    d_data_size(data_size),
    d_syn_start(0),
    d_syndrome_reg()
{
    // Initialise syndrome
}

fire_crc::~fire_crc()
{
}

int 
fire_crc::rem(const int x, const int y)
{
    return (x % y);
}

int
fire_crc::check_crc(const unsigned char *input_bits,
                          unsigned char *control_data)
{ 
    int j,error_count = 0, error_index = 0, success_flag = 0, syn_index = 0;
    d_syn_start = 0;
    // reset the syndrome register
    d_syndrome_reg.clear();
    d_syndrome_reg.insert(d_syndrome_reg.begin(),40,0);

    // shift in the data bits
    for (unsigned int i=0; i < d_data_size; i++) {
        error_count = syndrome_shift(input_bits[i]);
        control_data[i] = input_bits[i];
    }

    // shift in the crc bits
    for (unsigned int i=0; i < d_crc_size; i++) {
        error_count = syndrome_shift(1-input_bits[i+d_data_size]);
    }
   
    // Find position of error burst
    if (error_count == 0) {
        error_index = 0;
    }
    else {
        error_index = 1;
        error_count = syndrome_shift(0);
        error_index += 1;
        while (error_index < (d_data_size + d_crc_size) ) {
            error_count = syndrome_shift(0);
            error_index += 1;
            if ( error_count == 0 ) break;
        }
    }

    // Test for correctable errors
    //printf("error_index %d\n",error_index);
    if (error_index == 224) success_flag = 0;
    else {

        // correct index depending on the position of the error
        if (error_index == 0) syn_index = error_index;
        else syn_index = error_index - 1;
        
        // error burst lies within data bits
        if (error_index < 184) {
            //printf("error < bit 184,%d\n",error_index);
            j = error_index;
            while ( j < (error_index+12) ) {
                if (j < 184) {
                    control_data[j] = control_data[j] ^ 
                       d_syndrome_reg[rem(d_syn_start+39-j+syn_index,40)];
                }               
                else break;
                j = j + 1;
            }
        }
        else if ( error_index > 212 ) {
            //printf("error > bit 212,%d\n",error_index);
            j = 0;
            while ( j < (error_index - 212) ) {
                control_data[j] = control_data[j] ^ 
                      d_syndrome_reg[rem(d_syn_start+39-j-224+syn_index,40)];
                j = j + 1;
            }
        }
        // for 183 < error_index < 213 error in parity alone so ignore
        success_flag = 1;
    }
    return success_flag;
}    

int 
fire_crc::syndrome_shift(unsigned int bit)
{
    int error_count = 0;
    if (d_syn_start == 0) d_syn_start = 39;
    else d_syn_start -= 1;

    std::vector<int> temp_syndrome_reg = d_syndrome_reg;

    temp_syndrome_reg[rem(d_syn_start+3,40)] = 
                       d_syndrome_reg[rem(d_syn_start+3,40)] ^
                       d_syndrome_reg[d_syn_start];
    temp_syndrome_reg[rem(d_syn_start+17,40)] = 
                       d_syndrome_reg[rem(d_syn_start+17,40)] ^
                       d_syndrome_reg[d_syn_start];
    temp_syndrome_reg[rem(d_syn_start+23,40)] = 
                       d_syndrome_reg[rem(d_syn_start+23,40)] ^
                       d_syndrome_reg[d_syn_start];
    temp_syndrome_reg[rem(d_syn_start+26,40)] = 
                       d_syndrome_reg[rem(d_syn_start+26,40)] ^
                       d_syndrome_reg[d_syn_start];

    temp_syndrome_reg[rem(d_syn_start+4,40)] = 
                       d_syndrome_reg[rem(d_syn_start+4,40)] ^ bit;
    temp_syndrome_reg[rem(d_syn_start+6,40)] = 
                       d_syndrome_reg[rem(d_syn_start+6,40)] ^ bit;
    temp_syndrome_reg[rem(d_syn_start+10,40)] = 
                       d_syndrome_reg[rem(d_syn_start+10,40)] ^ bit;
    temp_syndrome_reg[rem(d_syn_start+16,40)] = 
                       d_syndrome_reg[rem(d_syn_start+16,40)] ^ bit;
    temp_syndrome_reg[rem(d_syn_start+27,40)] = 
                       d_syndrome_reg[rem(d_syn_start+27,40)] ^ bit;
    temp_syndrome_reg[rem(d_syn_start+29,40)] = 
                       d_syndrome_reg[rem(d_syn_start+29,40)] ^ bit;
    temp_syndrome_reg[rem(d_syn_start+33,40)] = 
                       d_syndrome_reg[rem(d_syn_start+33,40)] ^ bit;
    temp_syndrome_reg[rem(d_syn_start+39,40)] = 
                       d_syndrome_reg[rem(d_syn_start+39,40)] ^ bit;

    temp_syndrome_reg[d_syn_start] = d_syndrome_reg[d_syn_start] ^ bit;

    d_syndrome_reg = temp_syndrome_reg;

    for (unsigned int i = 0; i < 28; i++) {
       error_count = error_count + d_syndrome_reg[rem(d_syn_start+i,40)];
    }
    return error_count;
}
 

