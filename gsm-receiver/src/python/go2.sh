#! /bin/sh

#echo "go.sh <file_dl.cfile> <file_ul.cfile> [decim==112]"

KEY=$5
CONFIGURATION=$4
DECIM=$3
FILE_UL=$2
FILE=$1

if [ $DECIM"x" = x ]; then
	DECIM=48
fi

if [ $CONFIGURATION"x" = x ]; then
#	CONFIGURATION="0C"
	CONFIGURATION="7P"
fi

if [ "$KEY""x" = x ]; then
	KEY="00 00 00 00 00 00 00 00"
fi

# Use GSMTAP with WireShark instead of gmsdecode !

#./gsm_receive.py  -d "$DECIM" -I "$FILE" -c "$CONFIGURATION" -k "$KEY" | ../../../gsmdecode/src/gsmdecode -i

./gsm_receive_multi.py  -d "$DECIM" -I "$FILE" -U "$FILE_UL" -c "$CONFIGURATION" -k "$KEY"
