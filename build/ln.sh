#!/bin/sh

# ln dynamic lib 

if [ $# -ne 3   ]
    then
	        echo "Usage: $0 libpath target linkname  "
	        exit 1
	fi


					
cd $1;
ln -s -f $2 $3




