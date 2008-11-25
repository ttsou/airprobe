#!/usr/bin/env python

# import the GNU Radio code
import os
from gnuradio import gr, blks
import gsm
from gnuradio.blksimpl import gmsk

gsm_rate	= 1625000.0 / 6.0	# GSM samples / symbol

# most useful file Robert has
#file_name	= os.environ.get("GSMSPROOT") + "/resources/data/GSMSP_20070218_robert_dbsrx_941MHz_112.cfile"
#file_name	= os.environ.get("GSMSPROOT") + "/resources/data/GSMSP_20070204_robert_dbsrx_953.6MHz_64.cfile"
file_name	= "signal.data"
#decimation	= 64			# the decimation Robert used
#decimation	= 112			# the decimation Robert used
decimation	= 118			# the decimation Robert used

sps		= 64e6 / decimation	# samples / second with this decimation

filter_cutoff	= 150e3			# bandpass filter between +/- 150kHz
filter_t_width	= 50e3			# width of filter transition
offset		= 0			# Robert didn't need an offset


# create a GNURadio Flow Graph
fg = gr.flow_graph()

	# create a file source from Robert's file
source = gr.file_source(gr.sizeof_gr_complex, file_name)

# create a filter with the above constants
filter_taps = gr.firdes.low_pass(1.0, sps, filter_cutoff,
filter_t_width, gr.firdes.WIN_HAMMING)
filter = gr.freq_xlating_fir_filter_ccf(1, filter_taps, offset, sps)

# create the GMSK demod object
gd = gmsk.gmsk_demod(fg, sps / gsm_rate, gain_mu = 0.01,
omega_relative_limit = 0.005)

#diff = gr.diff_decoder_bb (2)

gsm = gsm.run_bb ()
#sqr = gr_example_b.square_bb ()

# create the vector sink
#sink = gr.vector_sink_b()

# connect all the blocks together
fg.connect(source, filter, gd, gsm)

# process the file
fg.run()

# dump the data
#print sink.data()

