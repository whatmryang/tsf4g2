#!/bin/sh

if [ $# -ne 1 ]
	then
		echo "Usage: generate.sh  pathoftdrtools"
		exit 1
fi

echo $1

PATH=$1:${PATH}

if [ ! -d "../../include/tlog/" ]
then
	mkdir ../../include/tlog/
fi

tdr --xml2h tlogdatadef.xml tlog_priority_def.xml tlogfilterdef.xml tlogfiledef.xml tlognetdef.xml tlogvecdef.xml tloganydef.xml tlogbindef.xml tlog_category_def.xml tlogconf.xml tlogmemdef.xml
mv tlog_priority_def.h ../../include/tlog/tlog_priority_def.h

tdr --xml2c -o tlogconf.c tlogdatadef.xml tlog_priority_def.xml tlogfilterdef.xml tlogfiledef.xml tlognetdef.xml tlogvecdef.xml tloganydef.xml tlogbindef.xml tlog_category_def.xml tlogconf.xml tlogmemdef.xml

tdr --xml2c -o tlog_priority_def.h tlog_priority_def.xml

#tdr -C tlogdef.xml -o tlogdef.c


