#!/bin/sh

cleanup()
{
	checkAndKill HelloWorld
	checkAndDel test_file.log
}

safeExit()
{
	cleanup
	exit $1
}

checkproc()
{
	echo "Checking proc $1";
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

cleanup
startAndTest ./HelloWorld

n=`cat test_file.log | wc -l`

if [ $n -ne 3 ];then
	echo "Failed to write file"
	safeExit -1
fi

safeExit 0
