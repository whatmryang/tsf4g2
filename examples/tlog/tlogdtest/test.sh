#!/bin/sh

#²âÊÔtlogdºÍtlogµÄÂß¼­
CWD=`pwd`
TLOGDD=../../../services/tlog_service/

cleanup()
{
	checkAndKill tlogdtest
	checkAndKill tlogd
	checkAndDel test_vec.log
	checkAndDel $TLOGDD/aaa.log
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


safeExit()
{
	cleanup
	exit $1
}

checkproc()
{
	#echo "Checking proc $1";
	#ps x
	N=`ps cx | grep $1 | grep -v grep | wc -l`
	return $N
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

cd $CWD
cd $TLOGDD
./tlogd --pid-file=tlogd.pid -D start
cd $CWD
./start_tlogdtest.sh
checkFileLines test_vec.log 9
checkFileLines $TLOGDD/aaa.log 9
safeExit 0
