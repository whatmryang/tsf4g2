#!/bin/sh
if test "$#" == "0" ; then
    echo "Usage: $0 loop-count"
    exit
fi
echo "./tdrnetpro -B newovcs_desc.tdr -m DispContent -l $1 newovcs_data.xml"
./tdrnetpro -B newovcs_desc.tdr -m DispContent -l $1 newovcs_data.xml
