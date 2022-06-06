#!/bin/sh

LINETMP="<tr><td>NAME</td> <td>MKRES</td> <td>TESTRES</td></tr>"
#testlib libname

makelib()
{
	NAME=$1
	DIR=`pwd`
	cd ../lib_src/$NAME
	make
	if [ $? -ne 0 ];then
		MKRES="ERROR"
	else
		cd $DIR
		cd ../lib/examples/$NAME
		make
		if [ $? -ne 0 ];then
			MKRES="ERROR"
		else
			MKRES="OK"
		fi	
		cd $DIR
	fi
	cd $DIR
}

maketestlib()
{
	NAME=$1
	DIR=`pwd`
	
	cd ../lib/examples/$NAME
	make test
	if [ $? -ne 0 ];then
		MKTSTRES="ERROR"
	else
		MKTSTRES="OK"
	fi	
	cd $DIR

}

#testlib libname

testlib()
{
	NAME=$1
	makelib $1
	maketestlib $1
	LINETMP="<tr><td>NAME</td> <td>MKRES</td> <td>TESTRES</td></tr>"
	RESULT=`echo $LINETMP | sed "s:NAME:$1:g" | sed "s:MKRES:$MKRES:g" | sed "s:TESTRES:$MKTSTRES:g"`
}

makeapp()
{
	NAME=$1
	DIR=`pwd`
	cd ../apps/$NAME
	make
	if [ $? -ne 0 ];then
		MKRES="ERROR"
	else
		MKRES="OK"
	fi
	cd $DIR
}

maketestapp()
{
	NAME=$1
	DIR=`pwd`
	
	cd ../apps/$NAME
	make test
	if [ $? -ne 0 ];then
		MKTSTRES="ERROR"
	else
		MKTSTRES="OK"
	fi	
	cd $DIR

}

testapp()
{
	NAME=$1
	makeapp $1
	maketestapp $1
	LINETMP="<tr><td>NAME</td> <td>MKRES</td> <td>TESTRES</td></tr>"
	RESULT=`echo $LINETMP | sed "s:NAME:$1:g" | sed "s:MKRES:$MKRES:g" | sed "s:TESTRES:$MKTSTRES:g"`
}




libList="tlog"
for lib in $libList
do 
	testlib $lib
	TABLERES=${TABLERES}${RESULT}
	
done


appList="tnamesvr tqos"
for app in $appList
do
	testapp $app
	TABLERES=${TABLERES}${RESULT}
done

DATE=`date +%Y%m%d`
cat tmplate.html | 
	sed "s:##DATE##:$DATE:g" |
	#sed "s:##SVN_INFO##:$SVNINFO:g"  |
	sed "s:##TESTRES##:$TABLERES:g" > result.html
