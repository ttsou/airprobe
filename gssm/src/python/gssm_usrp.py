#!/usr/bin/env python

# $Id: gssm_usrp.py,v 1.2 2007-07-07 16:31:44 jl Exp $

from gnuradio import gr, usrp, db_dbs_rx, blks2
from gnuradio.blks2impl import gmsk
from usrpm import usrp_dbid
import gssm
import sys

# constant
gsm_rate	= 1625000.0 / 6.0

# script constant
decim		= 112
gain		= 70

# bts channel
c0		= 875.4e6

# experimental constant
default_usrp_offset = 12e3

class gssm_flow_graph(gr.top_block):
	def __init__(self, usrp_offset):
		gr.top_block.__init__(self)

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

		xcf = 150e3
		xtw = 50e3
		xt = gr.firdes.low_pass(1.0, sps, xcf, xtw,
		   gr.firdes.WIN_HAMMING)
		xf = gr.fir_filter_ccf(1, xt)

		g = gssm.sink(sps)

		self.connect(u, xf, g)

def main():
	if len(sys.argv) == 2:
		uo = float(sys.argv[1])
	else:
		uo = default_usrp_offset
	g = gssm_flow_graph(uo)
	g.run()

if __name__ == '__main__':
	main()
