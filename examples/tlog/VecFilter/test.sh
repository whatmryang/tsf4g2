#!/bin/sh

cleanup()
{
	checkAndKill VecFilter
	checkAndDel test_vec1.log
	checkAndDel test_vec2.log
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

	if [ $n -ne $Lines ];then
		echo "Unexpected file content in $FileName,Expected $Lines,But Found $n"
		safeExit -1
	fi
	return 0
}


cleanup
startAndTest ./VecFilter
sleep 1
checkFileLines test_vec1.log 2
checkFileLines test_vec2.log 2


safeExit 0
