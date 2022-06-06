#!/bin/sh
PATH=$PATH:.
export PATH

usage()
{
      echo "Usage: "
      echo "  $0 ProjectName LabelName"
      echo " "
      echo "Example: "
      echo "  $0 base tsf4g_base_01_0001 "
      echo "  $0 tconnd tsf4g_tconnd_01_0001 "
}

CreateLabelDir()
{
    echo "--create label dir begin--"
    svn list https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/tags/$LabelName/ > /dev/null 2> /dev/null
    if [ $? == 0 ]; then
        echo "  Warning: label dir '$LabelName' exists!"
        echo -n "  Do you want to overwrite it first? [yes/no] "

        read input
        case "$input" in
            yes|Yes|Y|y)
            svn del -m "remove old tags" https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/tags/$LabelName/ > /dev/null 2> /dev/null
            if [ $? != 0 ]; then
                echo "Error: unnable to remove label dir '$LabelName'"
                exit 1
            fi
            echo "  lable '$LabelName' was removed"
            svn mkdir -m "tag for publish" https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/tags/$LabelName/ > /dev/null 2> /dev/null
            if [ $? != 0 ]; then
                echo "Error: unnable to create label '$LabelName'"
                exit 1
            fi
            echo "  label dir '$LabelName' was created"
            break
            ;;
            *)
            echo "...Label was aborted"
            echo ""
            exit 1
            break
            ;;
        esac
    else
        svn mkdir -m "tag for publish" https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/tags/$LabelName/ > /dev/null 2> /dev/null
        if [ $? != 0 ]; then
            echo "Error: unnable to create label dir '$LabelName'"
            exit 1
        fi
        echo "  label dir '$LabelName' was created"
    fi
    echo "--create label dir end--"
}

LabelProject()
{
      LabelFiles=`cat project.cfg|grep -v "#"|grep -i "$1.label"|sed -n -e "s/^[ ]*$1.label[ ]*=//p" `

      echo "--label $1 begin--"
      for item in $LabelFiles
      do
        svn copy -m "tag for publish" https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/$item https://sz-svn4.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/tags/$LabelName/ > /dev/null 2> /dev/null
        if [ $? != 0 ]; then
            echo "Error: unnable to create label '$LabelName'"
            exit 1
        fi
      done
      echo "--labeling $1 done--"
}

if [ $# -ne 2 ]; then
  echo "paramter list uncorrect"
  usage
  exit 1
fi

ProjectName=$1
LabelName=$2
LogName=~/$2.log
if [ -e $LogName ]; then
  rm $LogName
fi

echo "LabelName is $2"|tee -a $LogName

BasePath=`echo $0|sed -n -e ' s/\(.*\)\/[^\/]*$/\1/p' `

#echo $BasePath
if [ $BasePath != "." ]; then
  cd $BasePath
fi

solist=`find ../lib/ -name *.so`
dependencyflay=0

if [ "$solist" != "" ]; then
  echo ""
  echo ""
  echo "--check so dependency begin--"
  for so in $solist
  do
      id=`whoami`
      dependency=`ldd $so|grep $id`
      if [ "$dependency" != "" ]; then
        echo "find $so unproper dependency:$dependency"
        $dependencyflay=1
      fi
  done
  echo "--check so dependency done--"
fi

if [ "$dependencyflag" = 1 ];then
  echo "Please remove unproper so dependency first,label fail!exit~~"
  exit 1
fi

ProjectList=`cat $BasePath/project.cfg|grep -v "#"|grep -i "project"|sed -n -e 's/^[ ]*project[ ]*=//p' `
ProjectNameFound=0

for var in $ProjectList
do
   if [ $ProjectName = $var ]; then
      ProjectNameFound=1
      break
   fi
done

if [ $ProjectNameFound != 0 ]; then
  #step1:generate version.h file
  VersionSh=`cat $BasePath/project.cfg|grep -v "#"|grep -i "$ProjectName.version"|sed -n -e "s/^[ ]*$ProjectName.version[ ]*=//p" `
  if [ "$VersionSh" != "" ]; then
    ./$VersionSh $2
	if [ $? != 0 ];then
      echo "label version info failed\n"
	  exit 1
	fi
  fi
  #step1.1
  CreateLabelDir
  #step2:attach label
  LabelProject $ProjectName
  if [ $ProjectName != "base" ]; then
    LabelProject "base"
  fi
else
  echo "$ProjectName not found in $BasePath/projec.cfg,label $ProjectName fail!,exit"
  exit 1
fi

#step3:call make file
MakeSh=`cat $BasePath/project.cfg|grep -v "#"|grep -i "$ProjectName.make"|sed -n -e "s/^[ ]*$ProjectName.make[ ]*=//p" `
if [ "$MakeSh" != "" ]; then
  ./$MakeSh|tee -a $LogName
  if [ $? != 0 ];then
    echo "make $ProjectName failed\n"
	exit 2
  fi
fi

echo "Do you want to release $LabelName Now?[yes/no]"

#step4:release project
releaseflag=0
read input

case "$input" in
     yes|Yes|Y|y)
        releaseflag=1
        break
        ;;
     no|No|N|n)
        releaseflag=0
        break
        ;;
     *)
        releaseflag=0
        ;;
esac

if [ "$releaseflag" != 0 ]; then
     ./release.sh $1 $2|tee -a $LogName
fi
