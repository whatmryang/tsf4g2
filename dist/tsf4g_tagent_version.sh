#!/bin/sh
PATH=$PATH:.
export PATH

usage()
{
      echo "Usage: "
      echo "  $0  LabelName"
      echo " "
      echo "Example: "
      echo "  $0  tsf4g_base_01_0001 "
      echo "  $0  tsf4g_tagent_01_0001 "
}

if [ $# -ne 1 ]; then
  echo "paramter list uncorrect"
  usage
  exit 1
fi

#trap ' echo "Label=$1,MajorStr=$MajorStr,MinorStr=$MinorStr,RevStr=$RevStr,Major=$Major,Minor=$Minor,Rev=$Rev,Build =$Build" ' DEBUG


RevStr=`echo $1|sed -n -e "s/.*_\([^a-zA-Z]*\)_\(.*\)_\(.*\)/\3/p"`

if [ x$RevStr =  x"" ]; then
 MajorStr=`echo $1|sed -n -e "s/.*_\([^a-zA-Z]*\)_\(.*\)/\1/p"`
 MinorStr=`echo $1|sed -n -e "s/.*_\([^a-zA-Z]*\)_\(.*\)/\2/p"`
else
 MajorStr=`echo $1|sed -n -e "s/.*_\([^a-zA-Z]*\)_\(.*\)_\(.*\)/\1/p"`
 MinorStr=`echo $1|sed -n -e "s/.*_\([^a-zA-Z]*\)_\(.*\)_\(.*\)/\2/p"`
fi

Major=`echo $MajorStr|sed -n -e "s/^0*\([^0].*\)/\1/p"`
Minor=`echo $MinorStr|sed -n -e "s/^0*\([^0].*\)/\1/p"`
Rev=`echo $RevStr|sed -n -e "s/^0*\([^0].*\)/\1/p"`

if [ x$Major = x"" ]; then
  Major=1
fi

if [ x$Minor = x"" ]; then
  Minor=0
fi

if [ x$Rev = x"" ]; then
  Rev=0
fi

Build=`date +%Y%m%d`

version_file = "$HOME/tsf4g/apps/agent/src/version.h"
#replace major macro defintion in version.h
sed -i  -e "s/^\(#define\\s*MAJOR\\s*\)[0-9]*/\1$Major/g" 	$version_file

#replace minor macro definition in version.h
sed -i  -e "s/^\(#define\\s*MINOR\\s*\)[0-9]*/\1$Minor/g" 	$version_file

#replace rev macro definition in version.h
sed -i  -e "s/^\(#define\\s*REV\\s*\)[0-9]*/\1$Rev/g" 		$version_file

#replace build macro definiton in version.h
sed -i  -e "s/^\(#define\\s*BUILD\\s*\)[0-9]*/\1$Build/g" 	$version_file

echo "done"
