#!/bin/sh

usage()
{
      echo "Usage: "
      echo "  $0 UID"
      echo " "
      echo "Example: "
      echo "  $0  12"
      echo "  $0  13 "
}

UNINSTALL=`pwd`/uninstall.sh

echo "Detecting last installation"
if [ -f $UNINSTALL ];then
	echo "Found last installation deleting..."
	$UNINSTALL
fi
echo "Delete done"

echo "#!/bin/sh" > $UNINSTALL
chmod +x $UNINSTALL
INSTALL_LOG=INSTALL_`date +%Y%m%d`.log
echo "rm $INSTALL_LOG ">>  $UNINSTALL
echo "Starting install" > ${INSTALL_LOG}

echo "Getting user ID..."
## Create user id
if [ -z "$1" ];then
	#usage
	#exit -1
	if [ -d /tmp/tsf4g_dev_inst ];then
		files=`ls /tmp/tsf4g_dev_inst`
		for inst in ${files}
		do
			rm /tmp/tsf4g_dev_inst/${inst}
			USER_ID=${inst}
			break
		done
	
		if [ -z $USER_ID ];then
			echo "Error not enough id"
			exit 1
		fi
	else
		mkdir -p /tmp/tsf4g_dev_inst
		chmod 777 /tmp/tsf4g_dev_inst
		LIST="2 3 4 5 6 7 8 9 10 11 12 13 14"
		for inst in ${LIST}
		do
			touch /tmp/tsf4g_dev_inst/${inst}
			chmod 777 /tmp/tsf4g_dev_inst/${inst}
		done
		USER_ID=1
	fi
else
	USER_ID=${1}
fi


echo "touch /tmp/tsf4g_dev_inst/${USER_ID}" >> $UNINSTALL

echo "USER_ID=${USER_ID}" >> ${INSTALL_LOG}

###end create user id

if [ ${USER_ID} -ge 15 ];then
	echo "Error too much users. You should not install more than 15 instances on one machine"
	exit -1
fi 

echo "Succ user ID:$USER_ID"

echo "Createing dir variables"

TSF4G_DEV_HOME=`pwd`
TSF4G_HOME=${TSF4G_DEV_HOME}/mmog_deps/tsf4g_install
TSF4G_APP=${TSF4G_HOME}/apps/
TSF4G_TOOLS=${TSF4G_HOME}/mmog_mng/

TCONND_ID=1.3.${USER_ID}
TCMCENTER_ID=1.2.${USER_ID}
TLOGD_ID=1.4.${USER_ID}

TCONNDPORT=`awk "BEGIN{print (8000+${USER_ID})}"`
TLOGDPORT=`awk "BEGIN{print (8300+${USER_ID})}"`

echo "TSF4G_DEV_HOME=${TSF4G_DEV_HOME}" >> ${INSTALL_LOG}
echo "Creating tlog configurations"

cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/tlogd.xml.tpl | 
 sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_HOME}/services/tlog_service/tlogd.xml 
 
echo "Creating process log configurations"
cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/gamesvr/gamesvrd_log.xml | 
 sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_DEV_HOME}/mmog_run/etc/gamesvr/gamesvrd_log.xml 

cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/gamesvr/martsvr_log.xml | 
 sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_DEV_HOME}/mmog_run/etc/martsvr/martsvr_log.xml
 
cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/gamesvr/tconnd_dir_log.xml | 
 sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_DEV_HOME}/mmog_run/etc/tdir/tconnd_dir_log.xml

 cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/gamesvr/mailsvr_log.xml | 
 sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_DEV_HOME}/mmog_run/etc/mailsvr/mailsvr_log.xml 
 
 cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/gamesvr/tconnd_cus_log.xml | 
 sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_DEV_HOME}/mmog_run/etc/tcus/tconnd_cus_log.xml
 
 cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/gamesvr/tconnd_log.xml | 
 sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_DEV_HOME}/mmog_run/etc/tconnd/tconnd_log.xml 
 
 cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/gamesvr/tconnd_udp_log.xml | 
 sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_DEV_HOME}/mmog_run/etc/tdir/tconnd_udp_log.xml
 
 cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/gamesvr/tnamesvr_log.xml | 
 sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_DEV_HOME}/mmog_run/etc/tnamesvr/tnamesvr_log.xml
 
 cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/gamesvr/tdir_svrd_log.xml | 
 sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_DEV_HOME}/mmog_run/etc/tdir/tdir_svrd_log.xml 
 
 cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/gamesvr/tormsvr_log.xml | 
 sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_DEV_HOME}/mmog_run/etc/tormsvr/tormsvr_log.xml

echo "Creating tcm configurations"

cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/tcm/tconnd.xml | 
	sed "s:##TCONNDPORT##:${TCONNDPORT}:g"|
	sed "s:##TCMID##:${TCMCENTER_ID}:g" > ${TSF4G_APP}/tcm/conf/tconnd.xml 
  
cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/tcm/tconnd_log.xml | 
	sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_APP}/tcm/conf/tconnd_log.xml 
  
cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/tcm/tcmconsole_log.xml | 
	sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_APP}/tcm/conf/tcmconsole_log.xml 
  
cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/tcm/tcmcenter_log.xml | 
	sed "s:##TLOGDPORT##:${TLOGDPORT}:g" > ${TSF4G_APP}/tcm/conf/tcmcenter_log.xml 
  
cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/tcm/tcmconsole.xml | 
	sed "s:##TCONNDPORT##:${TCONNDPORT}:g" > ${TSF4G_APP}/tcm/conf/tcmconsole.xml
     
cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/tcm/tconnd_start | 
	sed "s:##TCONNDID##:${TCONND_ID}:g" > ${TSF4G_APP}/tcm/bin/tconnd_start
 
cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/tcm/tconnd_stop | 
	sed "s:##TCONNDID##:${TCONND_ID}:g" > ${TSF4G_APP}/tcm/bin/tconnd_stop

cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/tcm/tcmcenter_start | 
	sed "s:##TCMID##:${TCMCENTER_ID}:g" |
	sed "s:##USER_ID##:${USER_ID}:g" > ${TSF4G_APP}/tcm/bin/tcmcenter_start
 
cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/tcm/tcmcenter_stop | 
	sed "s:##USER_ID##:${USER_ID}:g" |
	sed "s:##TCMID##:${TCMCENTER_ID}:g" > ${TSF4G_APP}/tcm/bin/tcmcenter_stop

echo "cd ${TSF4G_HOME}/services/tlog_service" >> ${UNINSTALL}
echo "./tlogd --pid-file=tlogd.pid stop" >> ${UNINSTALL}
ln -s /usr/local/tsf4g_release/apps/tagent/log/tagent.log ${TSF4G_DEV_HOME}/tagent.log

echo "Installing tcm"

cat ${TSF4G_DEV_HOME}/mmog_mng/applist2tcmconfig/tcmcenter.xml.tpl.tpl | 
	sed "s:##TCONNDID##:${TCONND_ID}:g" |
	sed "s:##WORLDID##:${USER_ID}:g"  |
	sed "s:##USER_ID##:${USER_ID}:g"	> ${TSF4G_DEV_HOME}/mmog_mng/applist2tcmconfig/tcmcenter.xml.tpl
  
cat ${TSF4G_DEV_HOME}/mmog_deps/defConfig/tmng.conf.tpl |
	sed "s:##METABASEKEY##:${METABASEKEY}:g"  > ${TSF4G_DEV_HOME}/mmog_run/etc/tormsvr/tmng.conf

cat  ${TSF4G_DEV_HOME}/mmog_mng/applist2tcmconfig/archtecture.xml |
	sed "s:##TSF4GAPP##:${TSF4G_APP}:g"  >  ${TSF4G_DEV_HOME}/mmog_mng/applist2tcmconfig/applist.xml.tmp 


# Checking centerd Process
N=`ps aux | grep tcenterd | grep -v grep | wc -l`
echo "Checking tcenterd"
if [ $N -eq 1 ] ;then 
	echo "OK..."
else
	echo "Tcenterd not found ,Pls check tsf4g_dev_deps install"
	exit 1
fi	

echo "Checking tagent"
N=`ps aux | grep tagent | grep -v grep | wc -l`
if [ $N -eq 0 ] ;then 
	echo "tagent not found, Pls check tsf4g_dev_deps install"
	exit 1

else
	echo "Ok..."
fi	

echo "Checking for python modules"

#import sys
#from xml.dom import minidom
#import os
#import copy

python -c 'import sys'
if [ $? -ne 0 ]; then
	echo "Python Module Error"
	exit 1
fi

python -c 'from xml.dom import minidom'
if [ $? -ne 0 ]; then
	echo "Python Module Error"
	exit 1
fi

python -c 'import os'
if [ $? -ne 0 ]; then
	echo "Python Module Error"
	exit 1
fi

python -c 'import copy'
if [ $? -ne 0 ]; then
	echo "Python Module Error"
	exit 1
fi
      
echo "Creating tcmcenter.xml"   
cd ${TSF4G_DEV_HOME}/mmog_mng/applist2tcmconfig/
python config.py  ${USER_ID}  
chmod 777 ../../mmog_deps/tsf4g_install/apps/tcm/lib/config.sh
sh shminit.sh &> /dev/null
sh chmod.sh   > /dev/null
sh dbinit.sh  > /dev/null


echo "Reloading tcm config"
cd ${TSF4G_DEV_HOME}/mmog_mng/
chmod 777 tcmreload
cp applist2tcmconfig/tcmcenter.xml ../mmog_deps/tsf4g_install/apps/tcm/conf/
source startmng
echo "cd ${TSF4G_DEV_HOME}/mmog_mng/" >> ${UNINSTALL}
echo "source ${TSF4G_DEV_HOME}/mmog_mng/stopmng " >> ${UNINSTALL}

echo "Creating dev env "
cd ${TSF4G_DEV_HOME}/mmog_dev
./configure --with-TSF4G_INSTALL_HOME=${TSF4G_HOME} --with-XYZ_INSTALL_HOME=${TSF4G_DEV_HOME}/mmog_run > /dev/null

echo "Congraturations Installation success"
cd ${TSF4G_DEV_HOME}

