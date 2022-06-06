#!/bin/sh
echo "正在检测tsf4g目录规范"


checkdir()
{
	dirToCheck=$1
	if [ ! -d $dirToCheck ];then
		echo "error::目录不存在$dirToCheck"
		#exit -1
	fi
}

checkfile()
{
	dirToCheck=$1
	if [ ! -f $dirToCheck ];then
		echo "error::文件不存在$dirToCheck"
		#exit -1
	fi
}

checkapp()
{
	APPNAME=$1
	echo "正在检测目录规范"
	checkdir apps/$APPNAME

	appDirList="bin cfg tools examples src"
	for appDir in $appDirList
	do 
		checkdir apps/$APPNAME/$appDir
	done 
	
	checkfile apps/$APPNAME/Makefile
	
	echo "正在检测文档规范"
	checkdir docs/apps/$APPNAME
	
	NOTNAME=`echo $APPNAME | sed 's:^t:tsf4g_:g'`

	echo "正在检查发布脚本规范"
	checkfile dist/$NOTNAME.dist 
	checkfile dist/$NOTNAME.install
	checkfile dist/$NOTNAME.make
	
	echo "正在检测api规范"
	checkdir include/apps/$APPNAMEapi
}

checklib()
{
	LIBNAME=$1
	echo "正在检测源码目录"
	checkdir lib_src/$LIBNAME

	echo "正在检测头文件"
	checkdir include/$LIBNAME
	
	echo "正在检测例子"
	checkdir lib/examples/$LIBNAME
	
	echo "正在检测文档"
	checkdir docs/$LIBNAME
	checkdir docs/$LIBNAME/dist
	checkdir docs/$LIBNAME/dev
}

checkService()
{
	NAME=$1
	echo "正在检测源码目录"
	checkdir services_src/${NAME}_service
	echo "正在检测二进制目录"
	checkdir services/${NAME}_service
	echo "正在检测文档"
	checkdir docs/$NAME
}

checkTool()
{
	NAME=$1
	echo "正在检测源码目录"
	checkdir tools_src/$NAME
	echo "正在检测二进制目录"
	checkfile tools/$NAME
	echo "正在检测文档"
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

echo "正在检查app目录规范"
appList="tnamesvr tqos torm tagent tcenterd tcm tconnd tmart tdir tdirty trecver tsender sdm mail clientupdatesystem"
for app in $appList
do
	checkapp $app
done 

echo "正在检测lib目录规范"
libList="comm pal taa tapp tbus tdr tlog tloghelp  tmng tsec"
for lib in $libList
do
	checklib $lib
done

echo "正在检测service目录规范"
serviceList="tlog tbus"
for service in $serviceList
do
	checkService $service
done

echo "正在检测tools目录规范"
toolList="tmng tdr tbusmgr trelaymgr"
for tool in $toolList
do
	checkTool $tool
done
