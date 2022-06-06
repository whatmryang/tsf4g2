#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH

tdr --xml2dr -o tlogd_tconnddef.tdr ../../lib_src/tlog/tlogdatadef.xml ../../lib_src/tlog/tlog_priority_def.xml ../../lib_src/tlog/tlogfilterdef.xml ../../lib_src/tlog/tlogfiledef.xml ../../lib_src/tlog/tlognetdef.xml ../../lib_src/tlog/tlogvecdef.xml ../../lib_src/tlog/tloganydef.xml ../../lib_src/tlog/tlogbindef.xml tlogd_tconnddef.xml
tdr --xml2h  -O . -o tlogd_tconnddef.h ../../lib_src/tlog/tlogdatadef.xml ../../lib_src/tlog/tlog_priority_def.xml ../../lib_src/tlog/tlogfilterdef.xml ../../lib_src/tlog/tlogfiledef.xml ../../lib_src/tlog/tlognetdef.xml ../../lib_src/tlog/tlogvecdef.xml ../../lib_src/tlog/tloganydef.xml ../../lib_src/tlog/tlogbindef.xml tlogd_tconnddef.xml
tdr --xml2c  -o tlogd_tconnddef.c ../../lib_src/tlog/tlogdatadef.xml ../../lib_src/tlog/tlog_priority_def.xml ../../lib_src/tlog/tlogfilterdef.xml ../../lib_src/tlog/tlogfiledef.xml ../../lib_src/tlog/tlognetdef.xml ../../lib_src/tlog/tlogvecdef.xml ../../lib_src/tlog/tloganydef.xml ../../lib_src/tlog/tlogbindef.xml tlogd_tconnddef.xml

rm -f tlogdatadef.h tlog_priority_def.h tlogfilterdef.h tlogfiledef.h tlognetdef.h tlogvecdef.h tloganydef.h tlogbindef.h
