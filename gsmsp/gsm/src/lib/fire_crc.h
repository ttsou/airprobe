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

#include <vector>
#include <math.h>

#ifndef INCLUDED_FIRE_CRC_H
#define INCLUDED_FIRE_CRC_H

/*!
 * \brief 
 * \ingroup 
 */

class fire_crc 
{
protected:
 
  unsigned int d_crc_size;
  unsigned int d_data_size;
  unsigned int d_syn_start;
  std::vector<int> d_syndrome_reg;

public:

  fire_crc (unsigned int crc_size, unsigned int data_size);
  ~fire_crc (); 
  int check_crc (const unsigned char *input_bits,
	         unsigned char *control_bits);
  int syndrome_shift (unsigned int bit);
  int rem (const int x, const int y);
  
};

#endif
