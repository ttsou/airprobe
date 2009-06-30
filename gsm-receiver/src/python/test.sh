#!/bin/sh

./gsm_receive.py -I cfile > receiver-test.out 2> /dev/null
echo " 01 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b\n 15 06 21 00 01 00 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b 2b" > receiver-comp.out
diff receiver-test.out receiver-comp.out > receiver-test-diff.out 
test_result=`cat receiver-test-diff.out`

rm receiver-test.out receiver-test-diff.out receiver-comp.out

if [ "x$test_result" = "x" ]; then
    echo "Test: passed"
    exit 0
else
    echo "Test: failed"
    exit 1
fi
