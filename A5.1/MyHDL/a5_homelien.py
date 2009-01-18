#!/usr/bin/env python
# -*- coding: utf-8 -*-
# v2, oystein@homelien.no, 2009

AVOID_CHAINING	= True # hack

SIMULATE	= False
SIMULATE	= True # comment out this line to generate verilog

from myhdl import *

def chain_xor(out, signals):
	# build chain for taps (this is a typical myhdl hack..)
	chain = []
	def xor(o, i1, i2, pos):
		@always_comb
		def c():
			o.next = i1 ^ i2[pos]
		return c
	inp = Signal(bool(0))
	for sig, pos in signals:
		s = Signal(bool(0))
		x2 = xor(s, inp, sig, pos)
		chain += [x2]
		inp = s

	@always_comb
	def output():
		out.next = s

	return chain, output


def lfsr_hw(clk, i, majority, length, taps, clockbit, name=""):
	print "lfsr %s: len %s taps %s, clockbit %s" % (name, length, taps, clockbit)

	lfsr = Signal(intbv(0)[length:])

	bit = Signal(bool(0))
	if not AVOID_CHAINING:
		# this stopped working in some myhdl version.  need to complain on the mailing list
		chain = chain_xor(bit, [(i, 0)] + [(lfsr, tap) for tap in taps]);
	else:
		chain = ()

	@always(clk.posedge)
	def shift():
		if i[2]:
			# reset
			lfsr.next = 0
		elif i[1] or (majority == lfsr[clockbit]): # i[1] set if clocking forced (for init)
			# calculate tap bits
			lfsr.next = concat(lfsr[length-1:0], bit)
			#print "shift, contents: %08x, bit %d" % (lfsr, bit)

	clockreg = Signal(bool(0))
	outputreg = Signal(bool(0))

	@always_comb
	def extract_bits():
		clockreg.next = lfsr[clockbit]
		outputreg.next = lfsr[length-1]

	return (chain, shift, extract_bits), lfsr, clockreg, outputreg, bit

def a5_homelien(clk, i, o):
	"""
	input[0] is initialization data
	input[1] is True to force clocking (while initializing)
	input[2] is reset (clocked, active high)
	"""

	majority = Signal(bool(0))
	r1, r1_reg, r1_clk, r1_out, r1_bit = lfsr_hw(clk, i, majority, 19, [13, 16, 17, 18], 8, "R1")
	r2, r2_reg, r2_clk, r2_out, r2_bit = lfsr_hw(clk, i, majority, 22, [20, 21], 10, "R2")
	r3, r3_reg, r3_clk, r3_out, r3_bit = lfsr_hw(clk, i, majority, 23, [7, 20, 21, 22], 10, "R3")
	@always_comb
	def xors():
		# this should be done automatically with the chain function from the lists above
		# but perhaps need to complain on myhdl list to get it working again
		r1_bit.next = i[0] ^ r1_reg[13] ^ r1_reg[16] ^ r1_reg[17] ^ r1_reg[18]
		r2_bit.next = i[0] ^ r2_reg[20] ^ r2_reg[21]
		r3_bit.next = i[0] ^ r3_reg[7] ^ r3_reg[20] ^ r3_reg[21] ^ r3_reg[22]

	@always_comb
	def calc_stuff():
		majority.next = (r1_clk + r2_clk + r3_clk) >= 2
		o.next = r1_out ^ r2_out ^ r3_out

	return (r1, r2, r3, calc_stuff, xors)
	
def test():
	clk = Signal(bool(0))
	input = Signal(intbv(0)[3:]) # for initialization
	output = Signal(bool(0))

	algorithm = a5_homelien(clk, input, output)

	@instance
	def test():
		key = 0xEFCDAB8967452312 #== 0x48C4A2E691D5B3F7 from A5/1 pedagogical implementation in C
		fn =   0x000134
		known_good_AtoB = 0x14D3AA960BFA0546ADB861569CA30# == 0x534EAA582FE8151AB6E1855A728C00 from A5/1 pedagogical implementation in C
		known_good_BtoA = 0x093F4D68D757ED949B4CBE41B7C6B#== 0x24FD35A35D5FB6526D32F906DF1AC0 from A5/1 pedagogical implementation in C

		phases = (
			("clock in dummy key %s" % hex(key),			2,	intbv(key)[64:]),
			("reset",										4,	intbv(key)[1:]),
			("clock in key %s" % hex(key),					2,	intbv(key)[64:]),
			("clock in fn %s" % hex(fn),					2,	intbv(fn)[22:]),
			("100 fuzzing clocks",							0,	intbv(0)[100:]),
			("A->B (expected: %s)" % hex(known_good_AtoB),	0,	intbv(0)[114:]),
			("B->A (expected: %s)" % hex(known_good_BtoA),	0,	intbv(0)[114:]),
		)

		for description, inputor, data in phases:
			print "- %s" % description
			result = intbv(0)[len(data):]

			for count in range(len(data)):
				input.next = inputor + data[count]

				yield delay(10)
				clk.next = not clk
				yield delay(10)
				clk.next = not clk
				
				#print "time %8d: r1 %08x r2 %08x r3 %08x maj %d input %x out %d%d%d" % (count, r1_reg, r2_reg, r3_reg, majority, input, r1_out, r2_out, r3_out)
				count += 1

				result = (result << 1) + output
			print "  result: %s" % hex(result)


	return test, algorithm

if SIMULATE:		
	Simulation(test()).run()
else:
	clk = Signal(bool(0))
	input = Signal(intbv(0)[3:])
	output = Signal(bool(0))

	toVerilog(a5_homelien, clk, input, output)

	print ".v file generated"

