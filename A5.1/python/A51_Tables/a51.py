from BitVector import BitVector
"""
Module with implementations of:
lsfr - Linear Feedback Shift Register
a51 - algorithm A5/1
"""

class lsfr:
    def __init__(self, length, taps, middle_bit, name=""):
        """
        length - length of the register in bits
        taps - feedback taps, for clocking the shift register.
               These correspond to the primitive polynomial
               Example polynomials from A5/1:
               x**19 + x**5 + x**2 + x + 1, x**22 + x + 1,
               or x**23 + x**15 + x**2 + x + 1
        middle_bit - middle bit of each of the three shift registers, for clock control
        name - name of LSFR - for print()
        """
        self.taps = taps
        self.length = length
        self.name = name
        self.value = BitVector(size = length);
        self.clk_bit_nr = length-middle_bit-1

    def mix(self, liczba):
        """Read value from LSB to MSB and add each bit to LSFR's feedback"""
        for key_bit in reversed(liczba):
            bit = key_bit
            self.clock(bit)

    def clock(self, bit=False):
        """Clock LSFR. Can add value of bit to feedback."""
        for tap in self.taps:
            bit_nr = self.length - tap - 1
            bit = bit ^ self.value[bit_nr]
        self.value << 1
        self.value[self.length-1] = bit

    def out(self):
        """Clock LSFR. Can add value of bit to loopback."""
        return self.value[0]

    def clk_bit(self):
        """Return clocking bit."""
        return self.value[self.clk_bit_nr]

    def set_value(self, value):
        """Set internal state of LSFR."""
        self.value = BitVector(size=self.length, intVal=value)

    def __str__(self):
        return "%s:%X" %  (self.name, self.value.intValue())


class a51:
    def __init__(self):
        self.key_size = 64
        self.fn_size = 22
        self.r1 = lsfr(19, [13, 16, 17, 18], 8, "R1")
        self.r2 = lsfr(22, [20, 21], 10, "R2")
        self.r3 = lsfr(23, [7, 20, 21, 22], 10, "R3")
        self.r1_state_mask = ((1 << self.r1.length) -1 )
        self.r2_state_mask = ((1 << self.r2.length) -1 )
        self.r3_state_mask = ((1 << self.r3.length) -1 )

    def initialize(self, key, fn):
        """
        Initialize algorithm with:

        key - value of Kc - integer value of 64 bits
        fn - fame number - integer value of 22 bits
        """
        key_vect = BitVector(size=self.key_size, intVal=key)
        fn_vect = BitVector(size=self.fn_size, intVal=fn)

        self.r1.mix(key_vect)
        self.r2.mix(key_vect)
        self.r3.mix(key_vect)

        self.r1.mix(fn_vect)
        self.r2.mix(fn_vect)
        self.r3.mix(fn_vect)

        for i in xrange(0, 100):
            self._clock_regs()

    def _clock_regs(self):
        """Clock all LSFR's according to majority rule."""
        maj = self._majority()
        if self.r1.clk_bit() == maj:
            self.r1.clock()
        if self.r2.clk_bit() == maj:
            self.r2.clock()
        if self.r3.clk_bit() == maj:
            self.r3.clock()

    def get_output(self):
        """
        Generate 228 keystream bits:

        AtoB - 114 bits of keystream for the  A->B direction.  Store it, MSB first
        BtoA - 114 bits of keystream for the  B->A direction.  Store it, MSB first
        """
        AtoB = self.gen_block(114)
        BtoA = self.gen_block(114)
        return (AtoB, BtoA)

    def _out_bit(self):
        out_bit = self.r1.out() ^ self.r2.out() ^ self.r3.out()
        return out_bit

    def gen_block(self,size):
        out = BitVector(size=0)
        for i in xrange(0, size):
            self._clock_regs()
            out =   out + BitVector(intVal = int(self._out_bit()))
        return out.intValue()

    def _majority(self):
        sum = self.r1.clk_bit()+self.r2.clk_bit()+self.r3.clk_bit()
        if sum >= 2:
            return True
        else:
            return False

    def get_state(self):
        state = self.r3.value + self.r2.value + self.r1.value
        return state.intValue()

    def set_state(self,state):
        r1_value = state & self.r1_state_mask
        state = state >> self.r1.length
        r2_value = state & self.r2_state_mask
        state = state >> self.r2.length
        r3_value = state & self.r3_state_mask
        state = state >> self.r3.length
        self.r1.set_value(r1_value)
        self.r2.set_value(r2_value)
        self.r3.set_value(r3_value)

def test():
    key = 0xEFCDAB8967452312 #== 0x48C4A2E691D5B3F7 from A5/1 pedagogical implementation in C
    fn =   0x000134
    known_good_AtoB = 0x14D3AA960BFA0546ADB861569CA30# == 0x534EAA582FE8151AB6E1855A728C00 from A5/1 pedagogical implementation in C
    known_good_BtoA = 0x093F4D68D757ED949B4CBE41B7C6B#== 0x24FD35A35D5FB6526D32F906DF1AC0 from A5/1 pedagogical implementation in C
    alg = a51()
    alg.initialize(key, fn)
    alg.set_state(alg.get_state())
    (AtoB, BtoA) = alg.get_output()
    failed = False
    if (AtoB != known_good_AtoB) or (BtoA != known_good_BtoA):
        failed = True

    print "Key: %x" % key
    print "Fame number: %x" % fn
    print "Known good keystream from A to B: 0x%029x" % known_good_AtoB
    print "Observed keystream from A to B  : 0x%029x" % AtoB
    print "Known good keystream from B to A: 0x%029x" % known_good_BtoA
    print "Observed keystream from B to A  : 0x%029x" % BtoA
    if failed != True:
        print "\nResult: everything looks ok."
    else:
        print "\nResult: test failed!"

if __name__ == '__main__':
    test()