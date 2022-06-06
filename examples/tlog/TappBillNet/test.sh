#!/bin/sh

cleanup()
{
	checkAndKill TappTlog
	checkAndDel TappTlog.log
	checkAndDel TappTlog.error
	checkAndDel TappTlog_rundata.log
	checkAndDel TappTlog_tbus.log
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

#Testing default log
cleanup
checkAndDel TappTlog_log.xml
startAndTest ./start_TappTlog.sh
sleep 1
checkFileLines TappTlog.log 30
checkFileLines TappTlog.error 14

#Testing filter mod
cleanup
cp test_filter.xml TappTlog_log.xml
startAndTest ./start_TappTlog.sh
sleep 1
checkFileLines TappTlog.log 4
checkFileLines TappTlog.error 2

#Testing priority
cleanup
cp test_priority.xml TappTlog_log.xml
startAndTest ./start_TappTlog.sh
sleep 1
checkFileLines TappTlog.log 0
checkFileLines TappTlog.error 0

#Testing --logfile option
cleanup
cp test_priority.xml TappTlog_log.xml
startAndTest "./TappTlog  --conf-file TappTlog.xml --log-level 600 --log-file TappTlog start"
sleep 1
checkFileLines TappTlog.log 30
checkFileLines TappTlog.error 14

safeExit 0
