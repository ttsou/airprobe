#!/usr/bin/env python

# $Id: gssm_stats.py,v 1.2 2007-07-07 16:31:44 jl Exp $

from gnuradio import gr, usrp, db_dbs_rx, blks
from gnuradio.blksimpl import gmsk
import usrp_dbid
import gssm
import sys
import time
import thread

# constant
gsm_rate	= 1625000.0 / 6.0

# script constant
decim		= 112
gain		= 32

# bts channel
c0		= 874e6

# experimental constant
default_usrp_offset = 4e3

# filter constants
xcf = 150e3
xtw = 50e3

def display_stats(gs):
	while 1:
		print "%d:\t%d:%d" % \
		   (gs.d_found_fc_count, gs.d_valid_s, gs.d_invalid_s)
		time.sleep(1)


class gssm_flow_graph(gr.flow_graph):
	def __init__(self, usrp_offset):
		gr.flow_graph.__init__(self)

		print "decim = %d, gain = %d, offset = %.2f" % \
		   (decim, gain, usrp_offset)
		print "filter center %.2f, filter width %.2f" % \
		   (xcf, xtw)

		u = usrp.source_c(decim_rate = decim)
		s = usrp.pick_subdev(u, (usrp_dbid.DBS_RX,))
		u.set_mux(usrp.determine_rx_mux_value(u, s))
		subdev = usrp.selected_subdev(u, s)

		if subdev.dbid() != usrp_dbid.DBS_RX:
			raise Exception('dbs daughterboard not detected!')

		subdev.set_gain(gain)

		sps = u.adc_freq() / u.decim_rate()
		if sps < 2 * gsm_rate:
			raise Exception('sample rate too low')

		u.tune(0, subdev, c0 + usrp_offset)

		xt = gr.firdes.low_pass(1.0, sps, xcf, xtw,
		   gr.firdes.WIN_HAMMING)
		xf = gr.fir_filter_ccf(1, xt)

		self.gs = gs = gssm.sink(sps)

		self.connect(u, xf, gs)

def main():
	if len(sys.argv) == 2:
		uo = float(sys.argv[1])
	else:
		uo = default_usrp_offset
	g = gssm_flow_graph(uo)
	# thread.start_new_thread(display_stats, (g.gs,))
	# g.run()
	g.start()
	time.sleep(10.0)
	g.stop()
	g.gs.stats()

if __name__ == '__main__':
	main()

