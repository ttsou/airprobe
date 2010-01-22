#!/usr/bin/env python
# TODO:
#	* Add status info to window (frequency, offset, etc)
#	* Put direct frequency tuning back
#	* Add rate-limited file reads (throttle?)
#	* Make console only version
#	* Reset burst_stats on retune
#	* Add better option checking
#	* Wideband (multi-channel) processing (usrp and/or file input)
#	* Automatic beacon scan (quick scan RSSI, then check for BCCH)
#	* AGC
#Piotr:
#	I've removed stuff for visualisation which isn't needed to get
#	output bits and which causes that the program hangs when it finish processing.
#	This version works with GNU Radio 3.2.x

import sys

#nasty hack for testing
for extdir in ['../lib','../lib/.libs']:
	if extdir not in sys.path:
		sys.path.append(extdir)

from gnuradio import gr, gru, blks2
from gnuradio import usrp
from gnuradio import eng_notation
from gnuradio.eng_option import eng_option
from gnuradio.wxgui import stdgui2, fftsink2, waterfallsink2, scopesink2, form, slider
from optparse import OptionParser
from math import pi
import wx
import gsm

####################
class burst_callback(gr.feval_ll):
	def __init__(self, fg):
		gr.feval_ll.__init__(self)
		self.fg = fg
		self.offset_mean_num = 10		#number of FCCH offsets to average
		self.offset_vals = []
		
####################
	def eval(self, x):
		#print "burst_callback: eval(",x,")\n";
		try:
			if gsm.BURST_CB_SYNC_OFFSET == x:
				#print "burst_callback: SYNC_OFFSET\n";
				if self.fg.options.tuning.count("o"):
					last_offset = self.fg.burst.last_freq_offset()
					self.fg.offset -= last_offset
					#print "burst_callback: SYNC_OFFSET:", last_offset, " ARFCN: ", self.fg.channel, "\n";
					self.fg.set_channel(self.fg.channel)

			elif gsm.BURST_CB_ADJ_OFFSET == x:
				last_offset = self.fg.burst.last_freq_offset()

				self.offset_vals.append(last_offset)
				count =  len(self.offset_vals)
				#print "burst_callback: ADJ_OFFSET:", last_offset, ", count=",count,"\n";

				if count >= self.offset_mean_num:
					sum = 0.0
					while len(self.offset_vals):
						sum += self.offset_vals.pop(0)

					self.fg.mean_offset = sum / self.offset_mean_num

					#print "burst_callback: mean offset:", self.fg.mean_offset, "\n";

					#retune if greater than 100 Hz
					if abs(self.fg.mean_offset) > 100.0:	
						#print "burst_callback: mean offset adjust:", self.fg.mean_offset, "\n";
						if self.fg.options.tuning.count("o"):	
							#print "burst_callback: tuning.\n";
							self.fg.offset -= self.fg.mean_offset
							self.fg.set_channel(self.fg.channel)

			elif gsm.BURST_CB_TUNE == x:
				#print "burst_callback: BURST_CB_TUNE: ARFCN: ", self.fg.burst.next_arfcn, "\n";
				if self.fg.options.tuning.count("h"):
					#print "burst_callback: tuning.\n";
					self.fg.set_channel(self.fg.burst.next_arfcn)

			return 0

		except Exception, e:
			print >> sys.stderr, "burst_callback: Exception: ", e


####################
def pick_subdevice(u):
	if u.db(0, 0).dbid() >= 0:
		return (0, 0)
	if u.db(1, 0).dbid() >= 0:
		return (1, 0)
	return (0, 0)

####################
def get_freq_from_arfcn(chan,region):

	#P/E/R-GSM 900
	if chan >= 0 and chan <= 124:
		freq = 890 + 0.2*chan + 45

	#GSM 850
	elif chan >= 128 and chan <= 251:
		freq = 824.2 + 0.2*(chan - 128) + 45
		
	#GSM 450
	elif chan >= 259 and chan <= 293:
		freq = 450.6 + 0.2*(chan - 259) + 10
		
	#GSM 480
	elif chan >= 306 and chan <= 340:
		freq = 479 + 0.2*(chan - 306) + 10
		
	#DCS 1800
	elif region is "e" and chan >= 512 and chan <= 885:
		freq = 1710.2 + 0.2*(chan - 512) + 95
		
	#DCS 1900
	elif region is "u" and chan >= 512 and chan <= 810:
		freq = 1850.2 + 0.2*(chan - 512) + 80

	#E/R-GSM 900
	elif chan >= 955 and chan <= 1023:
		freq = 890 + 0.2*(chan - 1024) + 45

	else:
		freq = 0

	return freq * 1e6

def get_arfcn_from_freq(freq,region):
	freq = freq / 1e6
	# GSM 450
	if freq <= 450.6 + 0.2*(293 - 259) + 10:
		arfcn = ((freq - (450.6 + 10)) / 0.2) + 259
	# GSM 480
	elif freq <= 479 + 0.2*(340 - 306) + 10:
		arfcn = ((freq - (479 + 10)) / 0.2) + 306
	# GSM 850
	elif freq <= 824.2 + 0.2*(251 - 128) + 45:
		arfcn = ((freq - (824.2 + 45)) / 0.2) + 128
	#E/R-GSM 900
	elif freq <= 890 + 0.2*(1023 - 1024) + 45:
		arfcn = ((freq - (890 + 45)) / -0.2) + 955
	# GSM 900
	elif freq <= 890 + 0.2*124 + 45:
		arfcn = (freq - (890 + 45)) / 0.2
	else:
		if region is "u":
			if freq > 1850.2 + 0.2*(810 - 512) + 80:
				arfcn = 0;
			else:
				arfcn = (freq - (1850.2 + 80) / 0.2) + 512
		elif region is "e":
			if freq > 1710.2 + 0.2*(885 - 512) + 95:
				arfcn = 0;
			else:
				arfcn = (freq - (1710.2 + 95) / 0.2) + 512
		else:
			arfcn = 0

	return arfcn

####################
class tvoid_receiver(gr.top_block):

	def __init__(self):
		gr.top_block.__init__(self)

		#self.frame = frame
		#self.panel = panel
		self.parse_options()
		self.setup_flowgraph()
		self.setup_print_options()
		#self._build_gui(vbox)

		#some non-gui wxwindows handlers
		#self.t1 = wx.Timer(self.frame)
		#self.t1.Start(5000,0)
		#self.frame.Bind(wx.EVT_TIMER, self.on_tick)

		#bind the idle routing for message_queue processing	
		#self.frame.Bind(wx.EVT_IDLE, self.on_idle)
		
		#tune
		self.set_channel(self.channel)
	
		#giddyup
#		self.status_msg = "Started."


####################
	def parse_options(self):
		parser = OptionParser(option_class=eng_option)

		#view options
		parser.add_option("-S", "--scopes", type="string", default="I",
							help="Select scopes to display. (N)one, (I)nput,(F)ilter,(d)emod,(c)locked,(b)urst [default=%default]")
		parser.add_option("-p", "--print-console", type="string", default="s",
							help="What to print on console. [default=%default]\n" +
							"(n)othing, (e)verything, (s)tatus, (a)ll Types, (k)nown, (u)nknown, \n" +
							"TS(0), (F)CCH, (S)CH, (N)ormal, (D)ummy\n" +
							"Usefull (b)its, All TS (B)its, (C)orrelation bits, he(x) raw burst data, \n" +
							"(d)ecoded hex for gsmdecode")
				

		#decoder options
		parser.add_option("-D", "--decoder", type="string", default="c",
							help="Select decoder block to use. (c)omplex,(f)loat [default=%default]")
		parser.add_option("-E", "--equalizer", type="string", default="none",
							help="Type of equalizer to use.  none, fixed-dfe, fixed-linear [default=%default]")
		parser.add_option("-t", "--timing", type="string", default="cn",
							help="Type of timing techniques to use. [default=%default] \n" +
							"(n)one, (c)orrelation track, (q)uarter bit, (f)ull04 ")

		#tuning options
		parser.add_option("-T", "--tuning", type="string", default="oh",
							help="Type of tuning to perform. [default=%default] \n" +
							"(n)one, (o)ffset adjustment, (h)opping ")
		parser.add_option("-o", "--offset", type="eng_float", default=0.0,
							help="Tuning offset frequency")
		
		#file options
		parser.add_option("-I", "--inputfile", type="string", default=None,
							help="Select a capture file to read")
		parser.add_option("-O", "--outputfile", type="string", default=None,
							help="Filename to save burst output")
		parser.add_option("-l", "--fileloop", action="store_true", dest="fileloop", default=False,
							help="Continuously loop data from input file")
		
		#usrp options
		parser.add_option("-d", "--decim", type="int", default=112,
							help="Set USRP decimation rate to DECIM [default=%default]")
		parser.add_option("-R", "--rx-subdev-spec", type="subdev", default=None,
							help="Select USRP Rx side A or B (default=first one with a daughterboard)")
		parser.add_option("-A", "--antenna", default=None,
							help="Select Rx Antenna (only on RFX-series boards)")
		parser.add_option("--fusb-block-size", type="int", default=0,
							help="Set USRP blocksize")
		parser.add_option("--fusb-nblocks", type="int", default=0,
							help="Set USRP block buffers")
		parser.add_option("--realtime",action="store_true", dest="realtime",
							help="Use realtime scheduling.")
		parser.add_option("-F", "--clock-frequency", type="int", default=64e6,
							help="USRP FPGA master clock frequency")
		parser.add_option("-C", "--clock-offset", type="eng_float", default=0.0,
							help="Sample clock offset frequency")

		#FIXME: gain not working?
		parser.add_option("-g", "--gain", type="eng_float", default=None,
							help="Set gain in dB (default is midpoint)")
		parser.add_option("-c", "--channel", type="int", default=1,
							help="Tune to GSM ARFCN.")
		parser.add_option("-r", "--region", type="string", default="u",
							help="Frequency bands to use for channels.  (u)s or (e)urope [default=%default]")

		#testing options
		parser.add_option("--test-hop-speed",action="store_true", dest="test_hop_speed",
							help="Test hopping speed.")
		parser.add_option("--hopgood", type="int", default=658,
							help="Good ARFCN [default=%default]")
		parser.add_option("--hopbad", type="int", default=655,
							help="Emtpy ARFCN [default=%default]")

		(options, args) = parser.parse_args()
		if (len(args) != 0):
			parser.print_help()
			sys.exit(1)

#	   if (options.inputfile and ( options.freq or options.rx_subdev_spec or options.gain)):
#		   print "datafile option cannot be used with USRP options."
#		   sys.exit(1)

		if options.test_hop_speed:
			options.tuning = 'h'	#hopping only, no offset
			options.channel = options.hopgood

		self.options = options
		self.scopes = options.scopes
		self.region = options.region
		self.channel = options.channel
		self.offset = options.offset

####################
	def	setup_print_options(self):
		options = self.options

		self.print_status = options.print_console.count('s')

		if options.print_console.count('e'):
			self.print_status = 1

		#console print options
		popts = 0

		if options.print_console.count('d'):
			popts |= gsm.PRINT_GSM_DECODE

		if options.print_console.count('s'):
			popts |= gsm.PRINT_STATE

		if options.print_console.count('e'):
			popts |= gsm.PRINT_EVERYTHING

		if options.print_console.count('a'):
			popts |= gsm.PRINT_ALL_TYPES

		if options.print_console.count('k'):
			popts |= gsm.PRINT_KNOWN

		if options.print_console.count('u'):
			popts |= gsm.PRINT_UNKNOWN

		if options.print_console.count('0'):
			popts |= gsm.PRINT_TS0

		if options.print_console.count('F'):
			popts |= gsm.PRINT_FCCH

		if options.print_console.count('S'):
			popts |= gsm.PRINT_SCH

		if options.print_console.count('N'):
			popts |= gsm.PRINT_NORMAL

		if options.print_console.count('D'):
			popts |= gsm.PRINT_DUMMY

		if options.print_console.count('x'):
			popts |= gsm.PRINT_BITS | gsm.PRINT_HEX

		if options.print_console.count('B'):
			popts |= gsm.PRINT_BITS | gsm.PRINT_ALL_BITS

		elif options.print_console.count('b'):
			popts |= gsm.PRINT_BITS

		elif options.print_console.count('C'):
			popts |= gsm.PRINT_BITS | gsm.PRINT_CORR_BITS

		#print  "Print flags: 0x%8.8x\n" %(popts)
		
		self.burst.d_print_options = popts	


####################
	def setup_usrp(self):
		options = self.options
		
		#set resonable defaults if no user prefs set
		if options.realtime:
			if options.fusb_block_size == 0:
				options.fusb_block_size = gr.prefs().get_long('fusb', 'rt_block_size', 1024)
			if options.fusb_nblocks == 0:
				options.fusb_nblocks    = gr.prefs().get_long('fusb', 'rt_nblocks', 16)
		else:
			if options.fusb_block_size == 0:
				options.fusb_block_size = gr.prefs().get_long('fusb', 'block_size', 4096)
			if options.fusb_nblocks == 0:
				options.fusb_nblocks    = gr.prefs().get_long('fusb', 'nblocks', 16)
		
		print >> sys.stderr, "fusb_block_size =", options.fusb_block_size
		print >> sys.stderr, "fusb_nblocks    =", options.fusb_nblocks

			
		self.ursp = usrp.source_c(decim_rate=options.decim,fusb_block_size=options.fusb_block_size,fusb_nblocks=options.fusb_nblocks)
		self.ursp.set_fpga_master_clock_freq(options.clock_frequency)

		if options.rx_subdev_spec is None:
			options.rx_subdev_spec = pick_subdevice(self.ursp)
		
		self.ursp.set_mux(usrp.determine_rx_mux_value(self.ursp, options.rx_subdev_spec))

		# determine the daughterboard subdevice
		self.subdev = usrp.selected_subdev(self.ursp, options.rx_subdev_spec)
		input_rate = self.ursp.adc_freq() / self.ursp.decim_rate()

		if options.antenna is not None:
			print >> sys.stderr, "USRP antenna %s" % (options.antenna,)
			self.subdev.select_rx_antenna(options.antenna)

		# set initial values
		if options.gain is None:
			# if no gain was specified, use the mid-point in dB
			g = self.subdev.gain_range()
			options.gain = float(g[0]+g[1])/2

		self.set_gain(options.gain)
	
		self.source = self.ursp

####################
	def setup_timing(self):
		options = self.options
		clock_rate = options.clock_frequency

		if options.clock_offset:
			clock_rate = clock_rate + options.clock_offset
		elif options.channel:		
			#calculate actual clock rate based on frequency offset (assumes shared clock for sampling and tuning) 
			f = get_freq_from_arfcn(options.channel,options.region)
			if f:
				percent_offset = options.offset / get_freq_from_arfcn(options.channel,options.region)
			else:
				percent_offset = 0.0
				
			clock_rate += clock_rate * percent_offset
			print >> sys.stderr, "% offset = ", percent_offset, "clock = ", clock_rate
			
		self.clock_rate = clock_rate
		self.input_rate = clock_rate / options.decim		#TODO: what about usrp value?
		self.gsm_symb_rate = 1625000.0 / 6.0
		self.sps = self.input_rate / self.gsm_symb_rate
	
####################
	def setup_filter(self):
		#test w/o filter (for buffer latency)
		#self.filter = self.source
		#return
		
		options = self.options

		# configure channel filter
		filter_cutoff	= 145e3		#135,417Hz is GSM bandwidth 
		filter_t_width	= 10e3

		#Only DSP adjust for offset on datafile, adjust tuner for USRP
		#TODO: see if we can change this offset at runtime based on freq detection
		if options.inputfile:
			offset = self.offset
		else:
			offset = 0.0

		filter_taps = gr.firdes.low_pass(1.0, self.input_rate, filter_cutoff, filter_t_width, gr.firdes.WIN_HAMMING)
		self.filter = gr.freq_xlating_fir_filter_ccf(1, filter_taps, offset, self.input_rate)

		self.connect(self.source, self.filter)

####################
	def setup_f_flowgraph(self):
		# configure demodulator
		# adjust the phase gain for sampling rate
		self.demod = gr.quadrature_demod_cf(self.sps);
		
		#configure clock recovery
		gain_mu = 0.01
		gain_omega = .25 * gain_mu * gain_mu		# critically damped
		self.clocker = gr.clock_recovery_mm_ff(	self.sps, 
												gain_omega,
												0.5,			#mu
												gain_mu,
												0.5)			#omega_relative_limit, 

		self.burst = gsm.burst_ff(self.burst_cb)
		self.connect(self.filter, self.demod, self.clocker, self.burst)
	
####################
	def setup_c_flowgraph(self):
			#use the sink version if burst scope not selected
# 			if self.scopes.count("b"):
# 				self.burst = gsm.burst_cf(self.burst_cb,self.input_rate)
# 			else:
# 				self.burst = gsm.burst_sink_c(self.burst_cb,self.input_rate)

			self.burst = gsm.burst_cf(self.burst_cb,self.input_rate)
			
			self.connect(self.filter, self.burst)	

####################
	
####################
	def configure_burst_decoder(self):
		options = self.options
		
		# equalizer
		eq_types = {'none': gsm.EQ_NONE, 'fixed-dfe': gsm.EQ_FIXED_DFE, 'fixed-linear': gsm.EQ_FIXED_LINEAR }
		self.burst.d_equalizer_type = eq_types[options.equalizer]

		# timing
		topts = 0

		if options.timing.count('c'):
			topts |= gsm.CLK_CORR_TRACK
		
		if options.timing.count('q'):
			topts |= gsm.QB_QUARTER
		
		elif options.timing.count('f'):
			topts |= gsm.QB_FULL04
		
		self.burst.d_clock_options = topts

		#test modes
		testopts = 0
		
		if options.test_hop_speed:
			testopts |= gsm.OPT_TEST_HOP_SPEED
			self.burst.d_hop_good_arfcn = options.hopgood
			self.burst.d_hop_bad_arfcn = options.hopbad
			
			print "!!!!! Enabling Hop Speed Testing (good=%d, bad=%d) !!!!!" % (options.hopgood,options.hopbad)

		self.burst.d_test_options = testopts
		#print "Test Options: 0x%8.8x" % (self.burst.d_test_options)
		
		

####################
	def setup_flowgraph(self):

		options = self.options
		
		# Attempt to enable realtime scheduling
		if options.realtime:
			r = gr.enable_realtime_scheduling()
			if r == gr.RT_OK:
				options.realtime = True
				print >> sys.stderr, "Realtime scheduling ENABLED"
			else:
				options.realtime = False
				print >> sys.stderr, "Realtime scheduling FAILED"

		self.setup_timing()
		
		# Setup our input source
		if options.inputfile:
			self.using_usrp = False
			print >> sys.stderr, "Reading data from: " + options.inputfile
			self.source = gr.file_source(gr.sizeof_gr_complex, options.inputfile, options.fileloop)
		else:
			self.using_usrp = True
			self.setup_usrp()

		self.setup_filter()

		#create a tuner callback
		self.mean_offset = 0.0		#this is set by tuner callback
		self.burst_cb = burst_callback(self)

		# Setup flow based on decoder selection
	#	if options.decoder.count("c"):
		self.setup_c_flowgraph()
	#	elif options.decoder.count("f"):
	#		self.setup_f_flowgraph()

		self.configure_burst_decoder()
		
		#Hookup a vector-stream converter if we want burst output
		if self.scopes.count("b") or options.outputfile:
			self.v2s = gr.vector_to_stream(gr.sizeof_float,142)		#burst output is 142 (USEFUL_BITS)
			self.connect(self.burst, self.v2s)
		else:
			self.burst_sink = gr.null_sink(gr.sizeof_float)
			self.v2s = gr.vector_to_stream(gr.sizeof_float,142)		#burst output is 142 (USEFUL_BITS)
			self.connect(self.burst, self.v2s)
			self.connect(self.v2s, self.burst_sink)


		#Output file
		if options.outputfile:
			self.filesink = gr.file_sink(gr.sizeof_float, options.outputfile)
			self.connect(self.v2s, self.filesink)

#		self.setup_scopes()			

####################		
#	def set_status_msg(self, msg):
#		self.frame.GetStatusBar().SetStatusText(msg, 0)

####################
####################
	def set_freq(self, freq):
		#TODO: for wideband processing, determine if the desired freq is within our current sample range.
		#		If so, use the frequency translator to tune.  Tune the USRP otherwise.
		#		Maybe have a flag to force tuning the USRP?
		
		if not self.using_usrp:
			#if reading from file just adjust for offset in the freq translator
			if self.print_status:
				print >> sys.stderr, "Setting filter center freq to offset: ", self.offset, "\n"
			
			self.filter.set_center_freq(self.offset) 
			return True
	
		freq = freq - self.offset

		r = self.ursp.tune(0, self.subdev, freq)

		if r:
			self.status_msg = '%f' % (freq/1e6)
			return True
		else:
			self.status_msg = "Failed to set frequency (%f)" % (freq/1e6)
			return False

####################
	def set_gain(self, gain):

		if not self.using_usrp:
			return False

		self.subdev.set_gain(gain)

####################
	def set_channel(self, chan):

		self.chan = chan
		
		freq = get_freq_from_arfcn(chan,self.region)

		if freq:
			self.set_freq(freq)
		else:
			self.status_msg = "Invalid Channel"

####################
	def print_stats(self):
		out = sys.stderr
		
		n_total = self.burst.d_total_count
		n_unknown = self.burst.d_unknown_count
		n_known = n_total - n_unknown
		
		print >> out, "======== STATS ========="
		print >> out, 'freq_offset:    ',self.offset
		print >> out, 'mean_offset:    ',self.mean_offset
		print >> out, 'sync_loss_count:',self.burst.d_sync_loss_count
		print >> out, 'total_bursts:   ',n_total
		print >> out, 'fcch_count:     ',self.burst.d_fcch_count
		print >> out, 'part_sch_count: ',self.burst.d_part_sch_count
		print >> out, 'sch_count:      ',self.burst.d_sch_count
		print >> out, 'normal_count:   ',self.burst.d_normal_count
		print >> out, 'dummy_count:    ',self.burst.d_dummy_count
		print >> out, 'unknown_count:  ',self.burst.d_unknown_count
		print >> out, 'known_count:    ',n_known
		if n_total:
			print >> out, '%known:         ', 100.0 * n_known / n_total

		#timing
		if self.options.decoder.count("c"):
			omega = self.burst.get_omega()
		else:
			omega = self.clocker.omega()

		percent_sps = omega / self.sps 
		print >> out, 'omega:          %f (%f / %f)' % (omega,self.sps,percent_sps)
		
		print >> out, ""		
				

####################
def main():
    try:
	tvoid_receiver().run()
    except KeyboardInterrupt:
	pass
                        

#	app = gsm_receiver_first_blood().run()stdgui2.stdapp(app_flow_graph, "GSM Scanner", nstatus=1)
#	app.MainLoop()

####################
if __name__ == '__main__':
	main()
