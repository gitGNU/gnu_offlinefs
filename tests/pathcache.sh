#!/bin/bash

# Customization
maxdepth=1900
step=50
count=20

# Commandline parsing
usage(){
    echo "Usage: $0 [-c] <path>"
    exit 1
}

while getopts c opt; do
    case $opt in
	c)
	    do_create=true;;
	?)
	    usage
    esac
done

if [ $OPTIND -ne $# ]; then
    usage
fi

basepath=${!OPTIND}

if [ ! -d $basepath ]; then
    echo "Path doesn't exist or not a directory"
    exit
fi

# Actual benchmarking

genpath(){
    n=$1

    echo -n "."

    while [ $((n--)) -ne 0 ];do
	echo -n "/_"
    done
}

if [ $do_create ]; then
    if [ -e $basepath/"_" ]; then
	echo "Removing \"./_\" ... "
	time rm -Rf $basepath/_
    fi

    echo "Creating directory tree..."
    time mkdir -p $basepath/`genpath $maxdepth`
fi

for ((n=step; n<=maxdepth; n+=step)); do
    echo -n "stat (depth = $n)... "
    path=$basepath/`genpath $n`
    (time stat $path > /dev/null) 2>&1 |grep real
done

path=$basepath/`genpath $maxdepth`
mean=0

for ((n=0; n < count; n++)); do
    echo -n "stat (depth = $maxdepth)... "
    t=`(time stat $path > /dev/null) 2>&1 |grep real |sed 's/.*m\([0-9.]*\)s/\1/'`
    echo "$t"s
    mean=`echo "$mean+$t" |bc -q `
done

mean=`echo "scale=4; $mean/$count" |bc -q `

echo "Mean for depth=$maxdepth: " $mean
