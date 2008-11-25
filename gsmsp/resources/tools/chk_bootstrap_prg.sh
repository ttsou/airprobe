#! /bin/sh

DIE=0
(autoconf --version) < /dev/null > /dev/null 2>&1 || {
    echo
        echo "You must have autoconf installed."
	DIE=1
}

# libtool --version check not done...
(libtoolize --version) < /dev/null > /dev/null 2>&1 || {
    echo
        echo "You must have libtool installed."
        DIE=1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
    echo
        echo "You must have automake installed."
        DIE=1
}

if test "$DIE" -eq 1; then
    exit 1
fi

