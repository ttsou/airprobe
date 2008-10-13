// $Id: gssm.i,v 1.2 2007-07-07 16:31:42 jl Exp $

%include "exception.i"
%import "gnuradio.i"

%{
#include "gnuradio_swig_bug_workaround.h"
#include "gssm_sink.h"
#include <stdexcept>
%}


GR_SWIG_BLOCK_MAGIC(gssm, sink);
gssm_sink_sptr gssm_make_sink(double);

class gssm_sink : public gr_sync_block {

public:
        int     d_search_fc_count;
        int     d_found_fc_count;
        int     d_valid_s;
        int     d_invalid_s;

        void stats();

private:
	gssm_sink(double);
};
