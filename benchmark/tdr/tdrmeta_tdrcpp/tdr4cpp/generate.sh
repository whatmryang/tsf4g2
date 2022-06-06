#!/bin/sh

if [ $# -ne 1 ]
	then
		echo "Usage: generate.sh  pathoftdrtools"
		exit 1
fi

echo $1

PATH=$1:${PATH}

tdr --xml2cpp -O . newovcs_desc.xml
tdr --xml2dr -o newovcs_desc.tdr newovcs_desc.xml
