#!/usr/bin/env python
#


from gnuradio import gr, gr_unittest
import gsm

class qa_gsm (gr_unittest.TestCase):

    def setUp (self):
        self.fg = gr.top_block ()

    def tearDown (self):
        self.fg = None

    def test_001_burst_cf (self):
#        src_data = map(complex,(-3, 4, -5.5, 2, 3))
        src_data = (-3, 4, -5.5, 2, 3)
        print src_data
        expected_result = (9, 16, 30.25, 4, 9)
        print expected_result
        src = gr.vector_source_c (src_data)
        burst = gsm.burst_cf ()
        dst = gr.vector_sink_f ()
        self.fg.connect (src, burst)
        self.fg.connect (burst, dst)
        self.fg.run ()
        result_data = dst.data ()
        self.assertFloatTuplesAlmostEqual (expected_result, result_data, 5)
        
if __name__ == '__main__':
    gr_unittest.main ()
