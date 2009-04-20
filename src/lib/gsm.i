/* -*- c++ -*- */

%feature("autodoc", "1");		// generate python docstrings

%include "exception.i"
%import "gnuradio.i"			// the common stuff

%include "gsm_constants.h"

%{
#include "gnuradio_swig_bug_workaround.h"	// mandatory bug fix
#include "gsm_receiver_cf.h"
#include <stdexcept>
#include "gsm_constants.h"
%}

// ----------------------------------------------------------------

/*
 * First arg is the package prefix.
 * Second arg is the name of the class minus the prefix.
 *
 * This does some behind-the-scenes magic so we can
 * access howto_square_ff from python as howto.square_ff
 */
GR_SWIG_BLOCK_MAGIC(gsm,receiver_cf);

gsm_receiver_cf_sptr gsm_make_receiver_cf ( gr_feval_dd *tuner, int osr);

class gsm_receiver_cf : public gr_block
{
private:
  gsm_receiver_cf ( gr_feval_dd *tuner, int osr);
};

// ----------------------------------------------------------------
