#!/usr/bin/env python
# TODO:
#	* Adjust offset by PPM
#	* Auto-tune offset (add option to enable)
#	* Add status info to window (frequency, offset, etc)
#	* Put direct frequency tuning back
#	* Add rate-limited file reads (throttle?)
#	* Make console only version
#	* Reset burst_stats on retune
#	* Add better option checking

import sys

#nasty hack for testing
for extdir in ['../lib','../lib/.libs']:
	if extdir not in sys.path:
		sys.path.append(extdir)

from gnuradio import gr, gru, blks
from gnuradio import usrp
from gnuradio import eng_notation
from gnuradio.eng_option import eng_option
from gnuradio.wxgui import stdgui, fftsink, waterfallsink, scopesink, form, slider
from optparse import OptionParser
from math import pi
import wx
import gsm


class burst_callback(gr.feval_ll):
	def __init__(self, fg):
		gr.feval_ll.__init__(self)
		self.fg = fg

	def eval(self, x):
		try:
			#print "burst_callback: ", x, "\n";
			if gsm.BURST_CB_ADJ_OFFSET == x:
				#TODO: rework so this will work on file input
				last_offset = self.fg.burst.last_freq_offset()
				if last_offset < 200.0:
					return 0

				self.fg.offset -= last_offset
				print "burst_callback: ADJ_OFFSET:", last_offset, " ARFCN: ", self.fg.arfcn, "\n";
				self.fg.set_channel(self.fg.arfcn)

			elif gsm.BURST_CB_TUNE == x:
				self.fg.set_channel(self.fg.burst.next_arfcn)

			return 0

		except Exception, e:
			print "burst_callback: Exception: ", e


def pick_subdevice(u):
	if u.db[0][0].dbid() >= 0:
		return (0, 0)
	if u.db[1][0].dbid() >= 0:
		return (1, 0)
	return (0, 0)

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


class app_flow_graph(stdgui.gui_flow_graph):
	def __init__(self, frame, panel, vbox, argv):
		stdgui.gui_flow_graph.__init__(self)

		#testing
		self.status_msg = "Started."
		
		self.frame = frame
		self.panel = panel
		
		parser = OptionParser(option_class=eng_option)

		#view options
		parser.add_option("-S", "--scopes", type="string", default="I",
							help="Select scopes to display. (N)one, (I)nput,(F)ilter,(d)emod,(c)locked,(b)urst [default=%default]")
		parser.add_option("-p", "--print-console", type="string", default="s",
							help="What to print on console. [default=%default]\n" +
							"(n)othing, (e)verything, (s)tatus, (a)ll Types, (k)nown, (u)nknown, \n" +
							"TS(0), (F)CCH, (S)CH, (N)ormal, (D)ummy\n" +
							"Usefull (b)its, All TS (B)its, (C)orrelation bits, he(x) burst data, \n" +
							"(d)ecoded hex for gsmdecode")
				

		#decoder options
		parser.add_option("-D", "--decoder", type="string", default="f",
							help="Select decoder block to use. (c)omplex,(f)loat [default=%default]")
		parser.add_option("-d", "--decim", type="int", default=112,
							help="Set fgpa decimation rate to DECIM [default=%default]")
		parser.add_option("-o", "--offset", type="eng_float", default=0.0,
							help="Tuning offset frequency")
		parser.add_option("-C", "--clock-offset", type="eng_float", default=0.0,
							help="Sample clock offset frequency")
		parser.add_option("-E", "--equalizer", type="string", default="none",
							help="Type of equalizer to use.  none, fixed-dfe [default=%default]")
		parser.add_option("-t", "--timing", type="string", default="cq",
							help="Type of timing techniques to use. [default=%default] \n" +
							"(n)one, (c)orrelation track, (q)uarter bit, (f)ull04 ")

		#file options
		parser.add_option("-I", "--inputfile", type="string", default=None,
							help="Select a capture file to read")
		parser.add_option("-O", "--outputfile", type="string", default=None,
							help="Filename to save burst output")
		parser.add_option("-l", "--fileloop", action="store_true", dest="fileloop", default=False,
							help="Continuously loop data from input file")
		
		#usrp options
		parser.add_option("-R", "--rx-subdev-spec", type="subdev", default=None,
							help="Select USRP Rx side A or B (default=first one with a daughterboard)")
		#FIXME: gain not working?
		parser.add_option("-g", "--gain", type="eng_float", default=None,
							help="Set gain in dB (default is midpoint)")
		parser.add_option("-c", "--channel", type="int", default=None,
							help="Tune to GSM ARFCN.  Overrides --freq")
		parser.add_option("-r", "--region", type="string", default="u",
							help="Frequency bands to use for channels.  (u)s or (e)urope [default=%default]")


		(options, args) = parser.parse_args()
		if (len(args) != 0) or (not (options.channel or options.inputfile)):
			parser.print_help()
			sys.exit(1)

		self.options = options
		self.scopes = options.scopes
		self.region = options.region
		self.channel = options.channel
		self.offset = options.offset
		
		self.print_status = options.print_console.count('s')
		
		if options.print_console.count('e'):
			self.print_status = 1
			
#	   if (options.inputfile and ( options.freq or options.rx_subdev_spec or options.gain)):
#		   print "datafile option cannot be used with USRP options."
#		   sys.exit(1)


		#adjust or caclulate sample clock
		clock_rate = 64e6
		if options.clock_offset:
			clock_rate = 64e6 + options.clock_offset
		elif options.channel:		
			f = get_freq_from_arfcn(options.channel,options.region)
			if f:
				percent_offset = options.offset / get_freq_from_arfcn(options.channel,options.region)
			else:
				percent_offset = 0.0
				
			clock_rate += clock_rate * percent_offset
			print "% offset = ", percent_offset, "clock = ", clock_rate
			
		#set the default input rate, we will check with the USRP if it is being used
		input_rate = clock_rate / options.decim
		gsm_symb_rate = 1625000.0 / 6.0
		sps = input_rate/gsm_symb_rate

		# Build the flowgraph
		# Setup our input source
		if options.inputfile:
			self.using_usrp = False
			print "Reading data from: " + options.inputfile
			self.source = gr.file_source(gr.sizeof_gr_complex, options.inputfile, options.fileloop)
		else:
			self.using_usrp = True
			self.u = usrp.source_c(decim_rate=options.decim)
			if options.rx_subdev_spec is None:
				options.rx_subdev_spec = pick_subdevice(self.u)
			self.u.set_mux(usrp.determine_rx_mux_value(self.u, options.rx_subdev_spec))

			# determine the daughterboard subdevice
			self.subdev = usrp.selected_subdev(self.u, options.rx_subdev_spec)
			input_rate = self.u.adc_freq() / self.u.decim_rate()

			# set initial values
			if options.gain is None:
				# if no gain was specified, use the mid-point in dB
				g = self.subdev.gain_range()
				options.gain = float(g[0]+g[1])/2

			self.set_gain(options.gain)

		# configure the processing blocks
		# configure channel filter
		filter_cutoff	= 145e3		#135,417Hz is GSM bandwidth 
		filter_t_width	= 10e3

		#Only DSP adjust for offset on datafile, adjust tuner for USRP
		#TODO: see if we can change this offset at runtime based on freq detection
		if options.inputfile:
			offset = self.offset
		else:
			offset = 0.0

		filter_taps = gr.firdes.low_pass(1.0, input_rate, filter_cutoff, filter_t_width, gr.firdes.WIN_HAMMING)
		self.filter = gr.freq_xlating_fir_filter_ccf(1, filter_taps, offset, input_rate)

		# Connect the blocks
		if self.scopes.count("I"):
			self.input_fft_scope = fftsink.fft_sink_c (self, panel, fft_size=1024, sample_rate=input_rate)

		if options.inputfile:
			self.connect(self.source, self.filter)
			if self.scopes.count("I"):
				self.connect(self.source, self.input_fft_scope)
		else:
			self.connect(self.u, self.filter)
			if self.scopes.count("I"):
				self.connect(self.u, self.input_fft_scope)

		#create a tuner callback
		self.burst_cb = burst_callback(self)
		
		# Setup flow based on decoder selection
		if options.decoder.count("c"):
			self.burst = gsm.burst_cf(self.burst_cb,input_rate)
			self.connect(self.filter, self.burst)

		elif options.decoder.count("f"):
			# configure demodulator
			# adjust the phase gain for sampling rate
			self.demod = gr.quadrature_demod_cf(sps);
			
			#configure clock recovery
			gain_mu = 0.01
			gain_omega = .25 * gain_mu * gain_mu		# critically damped
			self.clocker = gr.clock_recovery_mm_ff(	sps, 
													gain_omega,
													0.5,			#mu
													gain_mu,
													0.3)			#omega_relative_limit, 

			self.burst = gsm.burst_ff(self.burst_cb)
			self.connect(self.filter, self.demod, self.clocker, self.burst)

			if self.scopes.count("d"):
				self.demod_scope = scopesink.scope_sink_f(self, panel, sample_rate=input_rate)
				self.connect(self.demod, self.demod_scope)

			if self.scopes.count("c"):
				self.clocked_scope = scopesink.scope_sink_f(self, panel, sample_rate=gsm_symb_rate,v_scale=1)
				self.connect(self.clocker, self.clocked_scope)

		elif options.decoder.count("F"):
			#configure clock recovery
			gain_mu = 0.01
			gain_omega = .25 * gain_mu * gain_mu		# critically damped
			self.clocker = gr.clock_recovery_mm_cc(	sps, 
													gain_omega,
													0.5,			#mu
													gain_mu,
													0.3)			#omega_relative_limit, 


			# configure demodulator
			self.demod = gr.quadrature_demod_cf(1);
			
			self.burst = gsm.burst_ff()
			self.connect(self.filter, self.clocker, self.demod, self.burst)

			if self.scopes.count("d"):
				self.demod_scope = scopesink.scope_sink_f(self, panel, sample_rate=input_rate)
				self.connect(self.demod, self.demod_scope)

			if self.scopes.count("c"):
				self.clocked_scope = scopesink.scope_sink_f(self, panel, sample_rate=gsm_symb_rate,v_scale=1)
				self.connect(self.clocker, self.clocked_scope)


		# setup decoder parameters
		# equalizer
		eq_types = {'none': gsm.EQ_NONE, 'fixed-dfe': gsm.EQ_FIXED_DFE}
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

		#console print options
		popts = 0
		
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
		
		if options.print_console.count('d'):
			popts |= gsm.PRINT_GSM_DECODE
		
		if options.print_console.count('C'):
			popts |= gsm.PRINT_BITS | gsm.PRINT_CORR_BITS

		if options.print_console.count('x'):
			popts |= gsm.PRINT_BITS | gsm.PRINT_HEX
		
		if options.print_console.count('B'):
			popts |= gsm.PRINT_BITS | gsm.PRINT_ALL_BITS
		
		elif options.print_console.count('b'):
			popts |= gsm.PRINT_BITS

		if options.print_console.count('d'):
			popts |= gsm.PRINT_GSM_DECODE
		
		#TODO: should warn if PRINT_GSM_DECODE is combined with other flags (will corrupt output for gsmdecode)
		
		self.burst.d_print_options = popts	
		
		##########################
		#set burst tuning callback
		#self.burst_cb = gsm_tuner()
		#self.burst.set_tuner_callback(self.burst_cb)
		
		# connect the primary path after source
		self.v2s = gr.vector_to_stream(gr.sizeof_float,142)		#burst output is 142 (USEFUL_BITS)
		self.connect(self.burst, self.v2s)

		# create and connect the scopes that apply to all decoders
		if self.scopes.count("F"):
			self.filter_fft_scope = fftsink.fft_sink_c (self, panel, fft_size=1024, sample_rate=input_rate)
			self.connect(self.filter, self.filter_fft_scope)

		#Connect output sinks
		if self.scopes.count("b"):
			self.burst_scope = scopesink.scope_sink_f(self, panel, sample_rate=gsm_symb_rate,v_scale=1)
			self.connect(self.v2s, self.burst_scope)
		elif not options.outputfile:
			self.burst_sink = gr.null_sink(gr.sizeof_float)
			self.connect(self.v2s, self.burst_sink)
							
		# setup & connect output file
		if options.outputfile:
			self.filesink = gr.file_sink(gr.sizeof_float, options.outputfile)
			self.connect(self.v2s, self.filesink)

		
		self._build_gui(vbox)
		
		self.set_channel(self.channel)

		self.t1 = wx.Timer(self.frame)
		self.t1.Start(5000,0)
		self.frame.Bind(wx.EVT_TIMER, self.on_tick)

		#bind the idle routing for message_queue processing	
		self.frame.Bind(wx.EVT_IDLE, self.on_idle)


	def _set_status_msg(self, msg):
		self.frame.GetStatusBar().SetStatusText(msg, 0)

	def _build_gui(self, vbox):

		if self.scopes.count("I"):
			vbox.Add(self.input_fft_scope.win, 5, wx.EXPAND)

		if self.scopes.count("F"):
			vbox.Add(self.filter_fft_scope.win, 5, wx.EXPAND)

		if self.scopes.count("d"):
			vbox.Add(self.demod_scope.win, 5, wx.EXPAND)

		if self.scopes.count("c"):
			vbox.Add(self.clocked_scope.win, 5, wx.EXPAND)
 
		if self.scopes.count("b"):
			vbox.Add(self.burst_scope.win, 5, wx.EXPAND)
		
		# add control area at the bottom
		if self.using_usrp:

			def _form_set_freq(kv):
				return self.set_freq(kv['freq'])

			self.usrpform = usrpform = form.form()
			hbox = wx.BoxSizer(wx.HORIZONTAL)

			hbox.Add((5,0), 0, 0)
			g = self.subdev.gain_range()
			usrpform['gain'] = form.slider_field(parent=self.panel, sizer=hbox, label="Gain",
											   weight=3,
											   min=int(g[0]), max=int(g[1]),
											   callback=self.set_gain)

			hbox.Add((5,0), 0, 0)
			usrpform['chan'] = form.slider_field(	parent=self.panel, sizer=hbox, label="Channel",
												weight=3,
												min=0, max=1023,
												value=self.channel,
												callback=self.set_channel)

			vbox.Add(hbox, 0, wx.EXPAND)
		

	def set_freq(self, freq):

		if not self.using_usrp:
			return False
	
		freq = freq - self.offset

		r = self.u.tune(0, self.subdev, freq)

		if r:
			self.status_msg = '%f' % (freq/1e6)
			return True
		else:
			self.status_msg = "Failed to set frequency (%f)" % (freq/1e6)
			return False

	def set_gain(self, gain):

		if not self.using_usrp:
			return False

		self.subdev.set_gain(gain)

	def set_channel(self, chan):

		self.arfcn = chan
		
		if not self.using_usrp:
			return False
		
		freq = get_freq_from_arfcn(chan,self.region)

		if freq:
			self.set_freq(freq)
		else:
			self.status_msg = "Invalid Channel"

	def print_stats(self):

		n_total = self.burst.d_total_count
		n_unknown = self.burst.d_unknown_count
		n_known = n_total - n_unknown
		
		print "======== STATS ========="
		print 'freq_offset:    ',self.offset
		print 'mean_offset:    ',self.burst.mean_freq_offset()
		print 'sync_loss_count:',self.burst.d_sync_loss_count
		print 'total_bursts:   ',n_total
		print 'fcch_count:     ',self.burst.d_fcch_count
		print 'part_sch_count: ',self.burst.d_part_sch_count
		print 'sch_count:      ',self.burst.d_sch_count
		print 'normal_count:   ',self.burst.d_normal_count
		print 'dummy_count:    ',self.burst.d_dummy_count
		print 'unknown_count:  ',self.burst.d_unknown_count
		print 'known_count:    ',n_known
		if n_total:
			print '%known:         ', 100.0 * n_known / n_total
		print ""		
				
	def on_tick(self, evt):
		#if option.autotune
			#tune offset
			
		if self.print_status:
			self.print_stats()

	def on_idle(self, event):
		self._set_status_msg(self.status_msg)
		#print "Idle.\n";

def main ():
	app = stdgui.stdapp(app_flow_graph, "GSM Scanner", nstatus=1)
	app.MainLoop()


if __name__ == '__main__':
	main ()
