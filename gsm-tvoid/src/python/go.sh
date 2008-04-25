#! /bin/sh

echo "go.sh <file.cfile> [decim==112]"

DECIM=$2
FILE=$1
if [ $FILE"x" = x ]; then
	FILE="../../../resource/data/GSMSP_940.8Mhz_118.cfile"
	DECIM=118
fi

if [ $DECIM"x" = x ]; then
	DECIM=112
fi

./gsm_scan.py -SN -pd -d "$DECIM" -I "$FILE" | ../../../gsmdecode/src/gsmdecode -i
