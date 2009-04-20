#!/usr/bin/env python
#!/usr/bin/env python

from gnuradio import gr, gru, blks2
from gnuradio import usrp
#from gnuradio import gsm
from gnuradio.eng_option import eng_option
from optparse import OptionParser
from os import sys
#"""
for extdir in ['../../debug/src/lib','../../debug/src/lib/.libs']:
    if extdir not in sys.path:
        sys.path.append(extdir)
import gsm                        
#"""
def pick_subdevice(u):
    if u.db[0][0].dbid() >= 0:
        return (0, 0)
    if u.db[1][0].dbid() >= 0:
        return (1, 0)
    return (0, 0)

class gsm_receiver_first_blood(gr.top_block):
    def __init__(self):
        gr.top_block.__init__(self)
        ( self.options, self.args) = self._przetworz_opcje()
        self._ustaw_taktowanie()
        self.zrodlo = self._ustaw_zrodlo()
        self.filtr = self._ustaw_filtr()
        self.interpolator = self._ustaw_interpolator()
        self.odbiornik = self._ustaw_odbiornik()
        self.konwerter = self._ustaw_konwerter()
        self.ujscie = self._ustaw_ujscie()
    
        self.connect(self.zrodlo, self.filtr,  self.interpolator, self.odbiornik, self.konwerter, self.ujscie)
#       self.connect(self.zrodlo, self.ujscie)
  
    def _ustaw_ujscie(self):
        nazwa_pliku_wy = self.options.outputfile
        ujscie = gr.file_sink(gr.sizeof_float, nazwa_pliku_wy)
        return ujscie
    
    def _ustaw_zrodlo(self):	
        options = self.options
        fusb_block_size = gr.prefs().get_long('fusb', 'block_size', 4096)
        fusb_nblocks    = gr.prefs().get_long('fusb', 'nblocks', 16)
        self.usrp = usrp.source_c(decim_rate=options.decim, fusb_block_size=fusb_block_size, fusb_nblocks=fusb_nblocks)
        
        if options.rx_subdev_spec is None:
            options.rx_subdev_spec = pick_subdevice(self.usrp)
        
        self.usrp.set_mux(usrp.determine_rx_mux_value(self.usrp, options.rx_subdev_spec))
        # determine the daughterboard subdevice
        self.subdev = usrp.selected_subdev(self.usrp, options.rx_subdev_spec)
        input_rate = self.usrp.adc_freq() / self.usrp.decim_rate()

        # set initial values
        if options.gain is None:
            # if no gain was specified, use the mid-point in dB
            g = self.subdev.gain_range()
            options.gain = float(g[0]+g[1])/2

        r = self.usrp.tune(0, self.subdev, options.freq)
        self.subdev.set_gain(options.gain)
        return self.usrp
    
    def _ustaw_taktowanie(self):
        options = self.options
        clock_rate = 64e6
        self.clock_rate = clock_rate
        self.input_rate = clock_rate / options.decim
        self.gsm_symb_rate = 1625000.0 / 6.0
        self.sps = self.input_rate / self.gsm_symb_rate

    def _ustaw_filtr(self):
        filter_cutoff   = 145e3	
        filter_t_width  = 10e3
        offset = 0
        print "input_rate:", self.input_rate, "sample rate:", self.sps, " filter_cutoff:", filter_cutoff, " filter_t_width:", filter_t_width
        filter_taps     = gr.firdes.low_pass(1.0, self.input_rate, filter_cutoff, filter_t_width, gr.firdes.WIN_HAMMING)
        filtr          = gr.freq_xlating_fir_filter_ccf(1, filter_taps, offset, self.input_rate)
        return filtr

    def _ustaw_konwerter(self):
        v2s = gr.vector_to_stream(gr.sizeof_float, 142)
        return v2s
    
    def _ustaw_interpolator(self):
        interpolator = gr.fractional_interpolator_cc(0, self.sps) 
        return interpolator
    
    def _ustaw_odbiornik(self):
        odbiornik = gsm.receiver_cf(1)
        return odbiornik
    
    def _przetworz_opcje(self):
        parser = OptionParser(option_class=eng_option)
        parser.add_option("-d", "--decim", type="int", default=128,
                                    help="Set USRP decimation rate to DECIM [default=%default]")
        parser.add_option("-I", "--inputfile", type="string", default="cfile",
                                    help="Input filename")
        parser.add_option("-O", "--outputfile", type="string", default="cfile2.out",
                                    help="Output filename")
        parser.add_option("-R", "--rx-subdev-spec", type="subdev", default=None,
                                    help="Select USRP Rx side A or B (default=first one with a daughterboard)")   
        parser.add_option("-f", "--freq", type="eng_float", default="950.4M",
                                    help="set frequency to FREQ", metavar="FREQ")
        parser.add_option("-g", "--gain", type="eng_float", default=None,
                                    help="Set gain in dB (default is midpoint)")
        (options, args) = parser.parse_args ()
        return (options, args)

def main():
    try:
        gsm_receiver_first_blood().run()
    except KeyboardInterrupt:
        pass

if __name__ == '__main__':
    main()


