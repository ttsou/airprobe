#! /bin/sh

echo "go.sh <file.cfile> [decim==112]"

DECIM=$2
FILE=$1

if [ $DECIM"x" = x ]; then
	DECIM=112
fi

./gsm_receive.py  -d "$DECIM" -I "$FILE" | ../../../gsmdecode/src/gsmdecode -i
