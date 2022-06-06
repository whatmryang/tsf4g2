#!/bin/sh
echo "���ڼ��tsf4gĿ¼�淶"


checkdir()
{
	dirToCheck=$1
	if [ ! -d $dirToCheck ];then
		echo "error::Ŀ¼������$dirToCheck"
		#exit -1
	fi
}

checkfile()
{
	dirToCheck=$1
	if [ ! -f $dirToCheck ];then
		echo "error::�ļ�������$dirToCheck"
		#exit -1
	fi
}

checkapp()
{
	APPNAME=$1
	echo "���ڼ��Ŀ¼�淶"
	checkdir apps/$APPNAME

	appDirList="bin cfg tools examples src"
	for appDir in $appDirList
	do 
		checkdir apps/$APPNAME/$appDir
	done 
	
	checkfile apps/$APPNAME/Makefile
	
	echo "���ڼ���ĵ��淶"
	checkdir docs/apps/$APPNAME
	
	NOTNAME=`echo $APPNAME | sed 's:^t:tsf4g_:g'`

	echo "���ڼ�鷢���ű��淶"
	checkfile dist/$NOTNAME.dist 
	checkfile dist/$NOTNAME.install
	checkfile dist/$NOTNAME.make
	
	echo "���ڼ��api�淶"
	checkdir include/apps/$APPNAMEapi
}

checklib()
{
	LIBNAME=$1
	echo "���ڼ��Դ��Ŀ¼"
	checkdir lib_src/$LIBNAME

	echo "���ڼ��ͷ�ļ�"
	checkdir include/$LIBNAME
	
	echo "���ڼ������"
	checkdir lib/examples/$LIBNAME
	
	echo "���ڼ���ĵ�"
	checkdir docs/$LIBNAME
	checkdir docs/$LIBNAME/dist
	checkdir docs/$LIBNAME/dev
}

checkService()
{
	NAME=$1
	echo "���ڼ��Դ��Ŀ¼"
	checkdir services_src/${NAME}_service
	echo "���ڼ�������Ŀ¼"
	checkdir services/${NAME}_service
	echo "���ڼ���ĵ�"
	checkdir docs/$NAME
}

checkTool()
{
	NAME=$1
	echo "���ڼ��Դ��Ŀ¼"
	checkdir tools_src/$NAME
	echo "���ڼ�������Ŀ¼"
	checkfile tools/$NAME
	echo "���ڼ���ĵ�"
	checkdir docs/$NAME
}

usage()
{
	echo "$0 app appName
	check app rules
$0 lib libName
	check lib rules
$0 service serviceName
	check service rules
$0 all 
	check all above
"
}

case $1 in
	app)
		checkapp $2
		exit 0
	;;
	lib)
		checklib $2
		exit 0
	;;
	service)
		checkservice $2
		exit 0
	;;
	all)
	;;
	*)
	usage
	exit -1
esac 

dirList="deps build dev_rules apps dist docs include lib lib_src services services_src tools tools_src unittests unittests_src win32 win32_lib"
for dir in $dirList
do
	checkdir $dir
done

echo "���ڼ��appĿ¼�淶"
appList="tnamesvr tqos torm tagent tcenterd tcm tconnd tmart tdir tdirty trecver tsender sdm mail clientupdatesystem"
for app in $appList
do
	checkapp $app
done 

echo "���ڼ��libĿ¼�淶"
libList="comm pal taa tapp tbus tdr tlog tloghelp  tmng tsec"
for lib in $libList
do
	checklib $lib
done

echo "���ڼ��serviceĿ¼�淶"
serviceList="tlog tbus"
for service in $serviceList
do
	checkService $service
done

echo "���ڼ��toolsĿ¼�淶"
toolList="tmng tdr tbusmgr trelaymgr"
for tool in $toolList
do
	checkTool $tool
done
