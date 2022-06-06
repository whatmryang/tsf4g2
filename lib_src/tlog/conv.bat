
..\..\tools\tdr --xml2h tlogdatadef.xml tlog_priority_def.xml tlogfilterdef.xml tlogfiledef.xml tlognetdef.xml tlogvecdef.xml tloganydef.xml tlogbindef.xml tlog_category_def.xml tlogconf.xml
move tlogdatadef.h ../../include/tlog/tlogdatadef.h
move tlogfilterdef.h ../../include/tlog/tlogfilterdef.h
move tlogfiledef.h ../../include/tlog/tlogfiledef.h
move tlognetdef.h ../../include/tlog/tlognetdef.h
move tlogvecdef.h ../../include/tlog/tlogvecdef.h
move tloganydef.h ../../include/tlog/tloganydef.h
move tlogbindef.h ../../include/tlog/tlogbindef.h
move tlog_priority_def.h ../../include/tlog/tlog_priority_def.h
move tlog_category_def.h ../../include/tlog/tlog_category_def.h
move tlogconf.h ../../include/tlog/tlogconf.h

..\..\tools\tdr.exe --xml2c -o tlogconf.c tlogdatadef.xml tlog_priority_def.xml tlogfilterdef.xml tlogfiledef.xml tlognetdef.xml tlogvecdef.xml tloganydef.xml tlogbindef.xml tlog_category_def.xml tlogconf.xml

rem tdr -C tlogdef.xml -o tlogdef.c


