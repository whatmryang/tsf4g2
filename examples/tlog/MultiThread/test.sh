#!/bin/sh

cleanup()
{
	./stop_MultiThread.sh
	checkAndDel thread0_0.log
	checkAndDel thread1_0.log
	checkAndDel thread2_0.log
}

safeExit()
{
	cleanup
	exit $1
}

checkproc()
{
	sleep 1;
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

	if [ $n -ne $Lines ];then
		echo "Unexpected file content in $FileName,Expected $Lines,But Found $n"
		safeExit -1
	fi
	return 0
}


cleanup
startAndTest ./start_MultiThread.sh
sleep 1
checkFileLines thread0_0.log 1
checkFileLines thread1_0.log 1
checkFileLines thread2_0.log 1

safeExit 0
