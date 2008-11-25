#! /bin/sh

# set GSMSPROOT
if [ x"${GSMSPROOT}" = x ]; then
	export GSMSPROOT=`pwd`
	echo "WARN: Setting GSMSPROOT to ${GSMSPROOT}"
	cd "$cwd"
fi

if [ ! -d "${GSMSPROOT}" ]; then
	echo "ERR: GSMSPROOT directory \"$GSMSPROOT\" does not exist."
	exit -1
fi

PYVERSION=`python -V 2>&1 | cut -f2 -d" " | cut -f1-2 -d.`

if [ x"${PYTHONPATH}" = x -o x`echo "$PYTHONPATH" | grep site-packages` = x ]; then
	export PYTHONPATH=$PYTHONPATH:/usr/lib/python${PYVERSION}/site-packages
	export PYTHONPATH=$PYTHONPATH:/usr/local/lib/python${PYVERSION}/site-packages
fi

for block in gsm; do
	dir="${GSMSPROOT}/${block}/src/lib"
	if [ ! -d "${dir}" ]; then
		echo "Ignoring block $block, $dir does not exist..."
	fi
	echo "Adding block $block..."
	export PYTHONPATH=${GSMSPROOT}/${block}/src/lib:$PYTHONPATH
	export PYTHONPATH=${GSMSPROOT}/${block}/src/lib/.libs:$PYTHONPATH
done

echo "      GSMSPROOT=${GSMSPROOT}"
echo "     PYTHONPATH=${PYTHONPATH}"
if [ ! -f ${GSMSPROOT}/signal.data ]; then
	ln -sf ${GSMSPROOT}/resources/data/GSMSP_940.8Mhz_118.cfile ${GSMSPROOT}/signal.data
fi

if [ x"$1" = x ]; then
	echo "           EXEC=${GSMSPROOT}/python/gsm_run.py"
	${GSMSPROOT}/python/gsm_run.py
else
	$1
fi

