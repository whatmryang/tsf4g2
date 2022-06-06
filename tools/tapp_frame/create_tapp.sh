#!/bin/sh
usage()
{
      echo "Usage: "
      echo "  $0  TappName [ParentDir]"
      echo " "
      echo "Example: "
      echo "  $0  tqosproxy /home/test/ "
}

if [ $# -lt 1 ]; then
  echo "paramter list uncorrect"
  usage
  exit 1
fi

PARENT_DIR=.
if [ $# -gt 1 ];  then
    PARENT_DIR=$2;
fi

if  test "$1" == "--help" || test "$1" == "-h" ; then
	usage
	exit 0
fi	


#rm -rf ${1}
PROJ_DIR=${PARENT_DIR}/${1}
mkdir -p  ${PROJ_DIR}


APP_BIG=`echo $1 | tr a-z A-Z`
	  
sed -e "s/TEMPLE_ORI/$1/g" -e "s/TEMPLE_BIG/$APP_BIG/g" ./temple/temple.c > ${PROJ_DIR}/${1}.c
sed -e "s/TEMPLE_ORI/$1/g" -e "s/TEMPLE_BIG/$APP_BIG/g" ./temple/temple_conf_desc.xml > ${PROJ_DIR}/${1}_conf_desc.xml
sed -e "s/TEMPLE_ORI/$1/g" -e "s/TEMPLE_BIG/$APP_BIG/g" ./temple/temple.xml > ${PROJ_DIR}/${1}.xml
cp ./temple/Makefile ${PROJ_DIR}/ 
sed -e "s/TEMPLE_ORI/$1/g" -e "s/TEMPLE_BIG/$APP_BIG/g" ./temple/conv.sh > ${PROJ_DIR}/conv.sh
sed -e "s/TEMPLE_ORI/$1/g" -e "s/TEMPLE_BIG/$APP_BIG/g" ./temple/Makefile.real > ${PROJ_DIR}/Makefile.real
sed -e "s/TEMPLE_ORI/$1/g" -e "s/TEMPLE_BIG/$APP_BIG/g" ./temple/version.h > ${PROJ_DIR}/version.h
sed -e "s/TEMPLE_ORI/$1/g" -e "s/TEMPLE_BIG/$APP_BIG/g" ./temple/start_temple.sh > ${PROJ_DIR}/start_${1}.sh
sed -e "s/TEMPLE_ORI/$1/g" -e "s/TEMPLE_BIG/$APP_BIG/g" ./temple/stop_temple.sh > ${PROJ_DIR}/stop_${1}.sh
chmod +x ${PROJ_DIR}/*.sh
