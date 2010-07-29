#! /bin/sh

#echo "go.sh <file.cfile> [decim==174]"

KEY=$4
CONFIGURATION=$3
DECIM=$2
FILE=$1

if [ $DECIM"x" = x ]; then
	DECIM=174
fi

if [ $CONFIGURATION"x" = x ]; then
	CONFIGURATION=""
fi

if [ "$KEY""x" = x ]; then
	KEY="00 00 00 00 00 00 00 00"
fi

# Use GSMTAP with WireShark instead of gmsdecode !

#./gsm_receive100.py  -d "$DECIM" -I "$FILE" -c "$CONFIGURATION" -k "$KEY" | ../../../gsmdecode/src/gsmdecode -i

./gsm_receive100.py  -d "$DECIM" -I "$FILE" -c "$CONFIGURATION" -k "$KEY"
