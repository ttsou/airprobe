T = one_step
TGT = ${T}_tb

LIB="./" 

INCLUDE_DIR=./

VERILOG_SOURCES = ${TGT}.v \
${T}.v

SIMFILE = ${TGT}.vvp
VCDFILE = ${TGT}.vcd 

WAVERC = gtkwaverc
WAVECFG = ${TGT}.sav

ICARUS = iverilog
IFLAGS = -v

VVP = vvp
VFLAGS = -v
VFLAGSEXTRA = 

GTKWAVE = gtkwave 
GFLAGS = --save=${WAVECFG} --rcfile=${WAVERC}

all: compile simulate gtkwave

${TGT}.vvp: ${VERILOG_SOURCES}
	@ echo Compiling verilog files...
	@ ${ICARUS} ${IFLAGS}  -s ${TGT} -o ${SIMFILE} -y ${LIB} ${VERILOG_SOURCES} -I${INCLUDE_DIR} 

compile : ${TGT}.vvp

${TGT}.vcd: compile
	@ echo Simulating design...
	@ ${VVP} ${VFLAGS} ${SIMFILE} ${VFLAGSEXTRA}

simulate : ${TGT}.vcd

gtkwave : compile simulate
	@ echo Viewing waveforms in gtkwave...
	@ ${GTKWAVE} ${GFLAGS} ${VCDFILE} 

clean: 
	@ echo Cleaning up...
	@rm -f *.vvp *.vcd *~ *.log
	

