#!/bin/sh

if [ $# -ne 1 ] 
	then
		echo "Usage: generate.sh  pathoftdrtools"
		exit 1
fi

echo $1

PATH=$1:${PATH}

tdr --xml2c tdr_sort.xml -o tdr_sort.c 
