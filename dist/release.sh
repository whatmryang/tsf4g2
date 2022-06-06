#!/bin/sh
PATH=$PATH:.
export PATH


usage()
{
      echo "Usage: "
      echo "  $0 ProjectName LabelName"
      echo " "
      echo "Example: "
      echo "  $0 base tsf4g_base_01_0001"
      echo "  $0 tconnd tsf4g_tconnd_01_0001 "
}

ReleaseProject()
{

  ReleaseScript=`cat project.cfg|grep -v "#"|grep -i "$1.release"|sed -n -e "s/^[ ]*$1.release[ ]*=//p" `
  echo "$ReleaseScript"
  if [ "$ReleaseScript" != "" ];then
    echo ""
    echo "-----------------------------------tar $1 begin-----------------------------"
    $ReleaseScript $2 
    echo "-----------------------------------tar $1 done-----------------------------"
  fi
  
}

if [ $# != 2 ]; then
   echo "parameter list  uncorrect"
   usage
   exit 1
fi

ProjectName=$1

BasePath=`echo $0|sed -n -e ' s/\(.*\)\/[^\/]*$/\1/p' `

if [ "$BasePathi" != "." ]; then
  cd $BasePath
fi

ProjectList=`cat project.cfg|grep -v "#"|grep -i "project"|sed -n -e 's/^[ ]*project[ ]*=//p' `

ProjectNameFound=0

for var in $ProjectList
do 
   if [ $ProjectName = $var ]; then
      ProjectNameFound=1
      break
   fi
done

if [ $ProjectNameFound != 0 ]; then
  ReleaseProject $ProjectName $2
else
  echo "$ProjectName not found in $BasePath/project.cfg,Release $ProjectName fail!,exit"
  exit 1
fi
