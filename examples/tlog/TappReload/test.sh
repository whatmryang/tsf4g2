#!/bin/sh

cleanup()
{
	./stop_example.sh
	checkAndDel example.log
	checkAndDel example.error
	checkAndDel example_tbus.log
	checkAndDel tbus.log
	checkAndDel example_rundata.log
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
		cat $FileName 
		safeExit -1
	fi
	return 0
}


cleanup
cp example_log_filter.xml example_log.xml
startAndTest ./start_example.sh
sleep 1
checkFileLines example.log 6
checkFileLines example.error 0

cp example_log_filter_on.xml example_log.xml
startAndTest ./reload_example.sh
sleep 1
checkFileLines example.log 16
checkFileLines example.error 7


cp example_log_filter_off.xml example_log.xml
startAndTest ./reload_example.sh
sleep 1
checkFileLines example.log 31
checkFileLines example.error 14

#cat example.log
safeExit 0
