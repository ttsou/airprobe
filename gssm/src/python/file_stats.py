#!/usr/bin/env python

# $Id: file_stats.py,v 1.1 2007-07-07 16:26:33 jl Exp $

from gnuradio import gr, usrp, db_dbs_rx, blks
from gnuradio.blksimpl import gmsk
import gssm
import sys

#sps		= 1000e3

usrp_rate	= 64e6
decim_rate	= 112
sps		= usrp_rate / decim_rate

gsm_rate	= 1625000.0 / 6.0

#xcf		= 150e3
#xtw		= 50e3
#xm		= -31127.933289

file_name	= "signal.data"


class gssm_graph(gr.flow_graph):
	def __init__(self, fname):
		gr.flow_graph.__init__(self)

		src = gr.file_source(gr.sizeof_gr_complex, fname)
		self.gs = gs = gssm.sink(sps)
		self.connect(src, gs)

def main():
	fname = file_name
	if len(sys.argv) == 2:
		fname = sys.argv[1]
	try:
		gg = gssm_graph(fname)
		gg.run()
		gg.gs.stats()

	except KeyboardInterrupt:
		pass

if __name__ == '__main__':
	main()
