/* -*- c++ -*- */

%feature("autodoc", "1");		// generate python docstrings

%include "exception.i"
%import "gnuradio.i"			// the common stuff

%{
#include "gnuradio_swig_bug_workaround.h"	// mandatory bug fix
#include "gsm_run_bb.h"
#include <stdexcept>
%}

// ----------------------------------------------------------------

/*
 * First arg is the package prefix.
 * Second arg is the name of the class minus the prefix.
 *
 * This does some behind-the-scenes magic so we can
 * access gsm_ruN_bb from python as gsm.run_bb
 */
GR_SWIG_BLOCK_MAGIC(gsm,run_bb);

gsm_run_bb_sptr gsm_make_run_bb ();

class gsm_run_bb : public gr_block
{
private:
  gsm_run_bb ();
};

