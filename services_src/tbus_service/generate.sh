#!/bin/sh

# Fisrt of all, you should set PATH to tdr tool properly

if [ $# -ne 1 ] 
	then
		echo "Usage: generate.sh  pathoftdrtools"
		exit 1
fi


PATH=$1:${PATH}
export PATH

cp ../../lib_src/tbus/tbus_macros.xml  ./
tdr --xml2h tbus_macros.xml tbusd_desc.xml 

tdr --xml2c tbus_macros.xml tbusd_desc.xml  -o tbusd_desc.c

#generate control protocol
tdr --xml2h tbusd_ctrl_proto.xml  
tdr --xml2c tbusd_ctrl_proto.xml  -o tbusd_ctrl_proto.c

Build=`date +%Y%m%d`
VERSION_FILE=./version.h

#replace build macro definiton in version.h
sed -i  -e "s/^\(#define\\s*BUILD\\s*\)[0-9]*/\1$Build/g"  ${VERSION_FILE}


