#! /bin/sh

if [ $1"x" = x ]; then
	echo "./capture.sh <freq> [duration==10] [decim==112]"
	echo "Example: ./capture.sh 940.4M"
	exit 1
fi
FREQ=$1

DURATION=$2
if [ $2"x" = x ]; then
	DURATION=10
fi
DECIM=$3
if [ $3"x" = x ]; then
	DECIM=112
fi

USRP_PROG=usrp_rx_cfile.py
while :; do
	which "$USRP_PROG"
	if [ $? -eq 0 ]; then
		break
	fi
	USRP_PROG=/usr/share/gnuradio/usrp/usrp_rx_cfile.py
	which "$USRP_PROG"
	if [ $? -eq 0 ]; then
		break
	fi

	echo "ERROR: usrp_rx_cfile.py not found. Make sure it's in your PATH!"
	exit 1
done

FILE="capture_${FREQ}_${DECIM}.cfile"
samples=`expr 64000000 / $DECIM '*' $DURATION`
echo "Capturing for $DURATION seconds to $FILE ($samples samples)"
$USRP_PROG -d "$DECIM" -f "$FREQ" -N $samples $FILE

