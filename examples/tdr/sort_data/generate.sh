#!/bin/sh

if [ $# -ne 1 ] 
	then
		echo "Usage: generate.sh  pathoftdrtools"
		exit 1
fi

echo $1

PATH=$1:${PATH}

tdr --xml2h tdr_sort.xml 
tdr --xml2c -o tdr_sort.c tdr_sort.xml 
