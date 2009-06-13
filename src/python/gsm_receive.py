#!/usr/bin/env python

from gnuradio import gr, gru, blks2
#, gsm
from gnuradio.eng_option import eng_option
from optparse import OptionParser
from os import sys

for extdir in ['../../debug/src/lib','../../debug/src/lib/.libs']:
    if extdir not in sys.path:
        sys.path.append(extdir)
import gsm                        

class tune(gr.feval_dd):
    def __init__(self, top_block):
        gr.feval_dd.__init__(self)
        self.top_block = top_block
        # self.center_freq = 0
    def eval(self, freq_offet):
        # self.center_freq = self.center_freq - freq_offet
        self.top_block.set_frequency(freq_offet)
        return freq_offet

class gsm_receiver_first_blood(gr.top_block):
    def __init__(self):
        gr.top_block.__init__(self)
        (options, args) = self._przetworz_opcje()
        self.tune_callback = tune(self)
        self.options    = options
        self.args       = args
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
        nazwa_pliku = self.options.inputfile
        zrodlo = gr.file_source(gr.sizeof_gr_complex, nazwa_pliku, False)
        return zrodlo
    
    def _ustaw_taktowanie(self):
        options = self.options
        clock_rate = 64e6
        self.clock_rate = clock_rate
        self.input_rate = clock_rate / options.decim
        self.gsm_symb_rate = 1625000.0 / 6.0
        self.sps = self.input_rate / self.gsm_symb_rate / self.options.osr

    def _ustaw_filtr(self):
        filter_cutoff   = 145e3	
        filter_t_width  = 10e3
        offset = 0
#        print "input_rate:", self.input_rate, "sample rate:", self.sps, " filter_cutoff:", filter_cutoff, " filter_t_width:", filter_t_width
        filter_taps     = gr.firdes.low_pass(1.0, self.input_rate, filter_cutoff, filter_t_width, gr.firdes.WIN_HAMMING)
        filtr          = gr.freq_xlating_fir_filter_ccf(1, filter_taps, offset, self.input_rate)
        return filtr

    def _ustaw_konwerter(self):
        v2s = gr.vector_to_stream(gr.sizeof_float, 142)
        return v2s
    
    def _ustaw_interpolator(self):
        interpolator = gr.fractional_interpolator_cc(0, self.sps) 
#	interpolator = blks2.rational_resampler_ccf(13, 6)
        return interpolator
    
    def _ustaw_odbiornik(self):
        odbiornik = gsm.receiver_cf(self.tune_callback, self.options.osr)
        return odbiornik
    
    def _przetworz_opcje(self):
        parser = OptionParser(option_class=eng_option)
        parser.add_option("-d", "--decim", type="int", default=128,
                          help="Set USRP decimation rate to DECIM [default=%default]")
        parser.add_option("-r", "--osr", type="int", default=4,
                          help="Oversampling ratio [default=%default]")
        parser.add_option("-I", "--inputfile", type="string", default="cfile",
                          help="Input filename")
        parser.add_option("-O", "--outputfile", type="string", default="cfile2.out",
                          help="Output filename")
        (options, args) = parser.parse_args ()
        return (options, args)
    
    def set_frequency(self, center_freq):
        self.filtr.set_center_freq(center_freq)

def main():
    try:
        gsm_receiver_first_blood().run()
    except KeyboardInterrupt:
        pass

if __name__ == '__main__':
    main()


