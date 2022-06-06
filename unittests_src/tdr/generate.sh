#!/bin/sh

# Fisrt of all, you should set PATH to xml2c and xml2h properly

if [ $# -ne 1 ]
    then
	        echo "Usage: generate.sh  pathoftdrtools"
			        exit 1
					fi

					echo $1

					PATH=$1:${PATH}

#PATH=.:${HOME}/tsf4g/tools/:${PATH}
export PATH



tdr --xml2c  -f -o ov_cs.c testfile/ov_cs.xml
tdr --xml2h -f  testfile/ov_cs.xml
mv testfile/ov_cs.h ./

tdr --xml2h  testfile/tdr_database.xml
mv testfile/tdr_database.h ./ut_tdr_db.h

tdr --xml2h testfile/testMetalibParam.xml
mv testfile/testMetalibParam.h simpleMetalibV1.h


tdr --xml2h  testfile/tdr_dbms.xml
mv testfile/tdr_dbms.h ./tdr_dbms.h

tdr --xml2h  testfile/tdr_sort.xml
mv testfile/tdr_sort.h ./ut_tdr_sort.h


tdr --xml2h  testfile/test_defaultvalue.xml
mv testfile/test_defaultvalue.h  ./ut_test_defaultvalue.h

tdr --xml2h testfile/test_escape_characters.xml
mv testfile/test_escape_characters.h ./ut_test_escape_characters.h

tdr --xml2h testfile/test_json_inoutput.xml
mv testfile/test_json_inoutput.h ./ut_test_json_inoutput.h
