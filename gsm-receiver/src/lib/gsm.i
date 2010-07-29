/* -*- c++ -*- */
/*
 * @file
 * @author Piotr Krysik <pkrysik@stud.elka.pw.edu.pl>
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

%feature("autodoc", "1");		// generate python docstrings

%include "exception.i"
%import "gnuradio.i"			// the common stuff

/* %include "gsm_constants.h" */

%{
#include "gnuradio_swig_bug_workaround.h"	// mandatory bug fix
#include "gsm_receiver_cf.h"
#include <stdexcept>
/* #include "gsm_constants.h" */
%}

// ----------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(gsm,receiver_cf);

gsm_receiver_cf_sptr gsm_make_receiver_cf ( gr_feval_dd *tuner, gr_feval_dd *synchronizer, int osr, std::string key, std::string configuration);

class gsm_receiver_cf : public gr_block
{
private:
  gsm_receiver_cf ( gr_feval_dd *tuner, gr_feval_dd *synchronizer, int osr);
};

// ----------------------------------------------------------------
