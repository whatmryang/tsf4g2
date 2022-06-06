#!/bin/sh

LogName=$1

MakePath=../apps/clientupdatesystem/

cd $MakePath; make

if [ $? != 0 ]; then
  echo "make failed.exit"
  exit 1
fi

cd -
echo "make done"





