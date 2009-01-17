from myhdl import *

def Lfsr(clk, reset, enable, reg, taps):
    
    @always(clk.posedge)
    def shiftRegister():
        if enable:
            output = 0
        
            for tap in taps:
                output ^= reg[tap]
        
            reg.next = ((reg << 1) + (output & 1)) % reg.max

    return shiftRegister

def printLfsr(clk, reg):

    @always(clk.negedge)
    def doPrint():
        print "LFSR value %s" % bin(reg, len(reg))

    return doPrint

def ClkDriver(clk, period=20):

    lowTime = int(period/2)
    highTime = period - lowTime

    @instance
    def driveClk():
        while True:
            yield delay(lowTime)
            clk.next = 1
            yield delay(highTime)
            clk.next = 0

    return driveClk

def ResetGen(reset, time):
    @instance
    def driveReset():
        reset.next = 0
        yield delay(time)
        reset.next = 1

    return driveReset

def Lfsr_tb():
    clk = Signal(0)
    reset = Signal(0)
    oscillator = ClkDriver(clk)
    reg = Signal(intbv(23)[19:])
    lfsr = Lfsr(clk, reg, [13, 16, 17, 18], reset)
    printer = printLfsr(clk, reg)
    return instances()

def muxReg(clk, reset, reg, R1, R2, R3):
    return loadReg

def a5(clk, reset, reg):

    R1 = Signal(intbv(0)[19:])
    R2 = Signal(intbv(0)[22:])
    R3 = Signal(intbv(0)[23:])

    en1 = Signal(intbv(1)[1:])
    en2 = Signal(intbv(1)[1:])
    en3 = Signal(intbv(1)[1:])

    @always(clk.posedge, reset.negedge)
    def loadReg():
        if reset == 0:
            print "Reset"
            R1.next = reg[64:45]
            R2.next = reg[45:23]
            R3.next = reg[23:0]
        else:
            reg.next[64:45] = R1
            reg.next[45:23] = R2
            reg.next[23:0] = R3

    lfsr1 = Lfsr(clk, reset, en1, R1, [13, 16, 17, 18])
    lfsr2 = Lfsr(clk, reset, en2, R2, [20, 21])
    lfsr3 = Lfsr(clk, reset, en3, R3, [7, 20, 21, 22])
    
    @always_comb
    def clock():
        if R1[8] + R2[10] + R3[10] > 2:
            en1.next = R1[8]
            en2.next = R2[10]
            en3.next = R3[10]
        else:
            en1.next = not R1[8]
            en2.next = not R2[10]
            en3.next = not R3[10]


    return instances()

def a5_tb():
    clk = Signal(intbv(0)[1:])
    reset = Signal(intbv(0)[1:])
    oscillator = ClkDriver(clk)
    resetgen = ResetGen(reset, 35)
    reg = Signal(intbv(0xdeadbeefcafebabe)[64:])
    lfsr = a5(clk, reset, reg)
    printer = printLfsr(clk, reg)
    return instances()





def sayhello():
    clk = Signal(0)

    oscillator = ClkDriver(clk)

    hello = Hello(clk)

    return instances()

#inst = a5_tb()
#sim = Simulation(inst)
#sim.run()
a5_inst = toVHDL(a5_tb)
