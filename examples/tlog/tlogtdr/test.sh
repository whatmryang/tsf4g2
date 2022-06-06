#!/bin/sh

cleanup()
{
	checkAndKill tlogtest 
	sleep 1
	checkAndDel tlogtest.log
	checkAndDel tlogtest.error 
}

safeExit()
{
	cleanup
	exit $1
}

checkproc()
{
	#echo "Checking proc $1";
	sleep 1;
	#ps x	
	N=`ps cx | grep $1 | grep -v grep | wc -l`
	return $N
}

checkAndKill()
{
	checkproc $1
	if [ $? -ne 0 ];then
		killall $1
	fi
}

checkAndDel()
{
	if [ -f $1 ];then
		rm $1
	fi
	
	if [ -d $1 ];then
		rm -rf $1
	fi
}

startAndTest()
{
	$1
	if [ $? -ne 0 ];then
		echo "Failed to start $1"
		safeExit -1
	fi
}

checkFileLines()
{
	FileName=$1
	Lines=$2
	n=`cat $FileName | wc -l`
	cat $FileName
	if [ $n -ne $Lines ];then
		echo "Unexpected file content in $FileName,Expected $Lines,But Found $n"
		safeExit -1
	fi
	return 0
}


cleanup
startAndTest ./start_tlogtest.sh
sleep 1
checkFileLines tlogtest.log 11 
checkFileLines tlogtest.error 3


safeExit 0
