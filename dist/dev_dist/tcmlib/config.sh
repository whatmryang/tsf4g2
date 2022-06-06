#!/bin/sh
N=`echo $APPID | grep '[0-9]\+.[0-9]\+.2.[0-9]\+'| wc -l`
if [ $N -eq 1 ];then
    echo "$APPID creating config"
    cat ${LIBPATH}/martsvr.xml|
    sed "s:##GAMESVRD##:${gamesvrd5}:g"|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/martsvr.xml
    cat $OUTPUT_PATH/martsvr.xml
fi
N=`echo $APPID | grep '[0-9]\+.[0-9]\+.3.[0-9]\+'| wc -l`
if [ $N -eq 1 ];then
    echo "$APPID creating config"
    cat ${LIBPATH}/tnamesvr.xml|
    sed "s:##GAMESVRD##:${gamesvrd5}:g"|
    sed "s:##TORMSVR##:${tormsvr8}:g"|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/tnamesvr.xml
    cat $OUTPUT_PATH/tnamesvr.xml
fi
N=`echo $APPID | grep '[0-9]\+.[0-9]\+.4.[0-9]\+'| wc -l`
if [ $N -eq 1 ];then
    echo "$APPID creating config"
    cat ${LIBPATH}/tconnd.xml|
    sed "s:##GAMESVRD##:${gamesvrd5}:g"|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/tconnd.xml
    cat $OUTPUT_PATH/tconnd.xml
fi
N=`echo $APPID | grep '[0-9]\+.[0-9]\+.5.[0-9]\+'| wc -l`
if [ $N -eq 1 ];then
    echo "$APPID creating config"
    cat ${LIBPATH}/gamesvrd.xml|
    sed "s:##VERSIONSERVER##:${versionserver11}:g"|
    sed "s:##TDIR_SVRD##:${tdir_svrd14}:g"|
    sed "s:##TORMSVR##:${tormsvr8}:g"|
    sed "s:##TMARTSVR##:${tmartsvr2}:g"|
    sed "s:##TCONND##:${tconnd4}:g"|
    sed "s:##TNAMESVR##:${tnamesvr3}:g"|
    sed "s:##MAILSVR##:${mailsvr9}:g"|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/gamesvrd.xml
    cat $OUTPUT_PATH/gamesvrd.xml
fi
N=`echo $APPID | grep '[0-9]\+.[0-9]\+.6.[0-9]\+'| wc -l`
if [ $N -eq 1 ];then
    echo "$APPID creating config"
    cat ${LIBPATH}/client.xml|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/client.xml
    cat $OUTPUT_PATH/client.xml
fi
N=`echo $APPID | grep '[0-9]\+.[0-9]\+.7.[0-9]\+'| wc -l`
if [ $N -eq 1 ];then
    echo "$APPID creating config"
    cat ${LIBPATH}/client.xml|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/client.xml
    cat $OUTPUT_PATH/client.xml
fi
N=`echo $APPID | grep '[0-9]\+.[0-9]\+.8.[0-9]\+'| wc -l`
if [ $N -eq 1 ];then
    echo "$APPID creating config"
    cat ${LIBPATH}/tormsvr.xml|
    sed "s:##MAILSVR##:${mailsvr9}:g"|
    sed "s:##GAMESVRD##:${gamesvrd5}:g"|
    sed "s:##TNAMESVR##:${tnamesvr3}:g"|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/tormsvr.xml
    cat $OUTPUT_PATH/tormsvr.xml
fi
N=`echo $APPID | grep '[0-9]\+.[0-9]\+.9.[0-9]\+'| wc -l`
if [ $N -eq 1 ];then
    echo "$APPID creating config"
    cat ${LIBPATH}/mailsvr.xml|
    sed "s:##GAMESVRD##:${gamesvrd5}:g"|
    sed "s:##TORMSVR##:${tormsvr8}:g"|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/mailsvr.xml
    cat $OUTPUT_PATH/mailsvr.xml
fi
N=`echo $APPID | grep '[0-9]\+.[0-9]\+.10.[0-9]\+'| wc -l`
if [ $N -eq 1 ];then
    echo "$APPID creating config"
    cat ${LIBPATH}/tconnd_cus.xml|
    sed "s:##VERSIONSERVER##:${versionserver11}:g"|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/tconnd_cus.xml
    cat $OUTPUT_PATH/tconnd_cus.xml
fi
N=`echo $APPID | grep '[0-9]\+.[0-9]\+.11.[0-9]\+'| wc -l`
if [ $N -eq 1 ];then
    echo "$APPID creating config"
    cat ${LIBPATH}/versionserver.xml|
    sed "s:##TCONND_CUS##:${tconnd10}:g"|
    sed "s:##GAMESVRD##:${gamesvrd5}:g"|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/versionserver.xml
    cat $OUTPUT_PATH/versionserver.xml
fi
N=`echo $APPID | grep '[0-9]\+.[0-9]\+.12.[0-9]\+'| wc -l`
if [ $N -eq 1 ];then
    echo "$APPID creating config"
    cat ${LIBPATH}/tconnd_dir.xml|
    sed "s:##TDIR_SVRD##:${tdir_svrd14}:g"|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/tconnd_dir.xml
    cat $OUTPUT_PATH/tconnd_dir.xml
fi
N=`echo $APPID | grep '[0-9]\+.[0-9]\+.13.[0-9]\+'| wc -l`
if [ $N -eq 1 ];then
    echo "$APPID creating config"
    cat ${LIBPATH}/tconnd_udp.xml|
    sed "s:##TDIR_SVRD##:${tdir_svrd14}:g"|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/tconnd_udp.xml
    cat $OUTPUT_PATH/tconnd_udp.xml
fi
N=`echo $APPID | grep '[0-9]\+.[0-9]\+.14.[0-9]\+'| wc -l`
if [ $N -eq 1 ];then
    echo "$APPID creating config"
    cat ${LIBPATH}/tdir_data_level3.xml|
    sed "s:##TCONND_DIR##:${tconnd12}:g"|
    sed "s:##GAMESVRD##:${gamesvrd5}:g"|
    sed "s:##TCONND_DIR_UDP##:${tconnd13}:g"|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/tdir_data_level3.xml
    cat $OUTPUT_PATH/tdir_data_level3.xml
    cat ${LIBPATH}/tdir.cfg|
    sed "s:##TCONND_DIR##:${tconnd12}:g"|
    sed "s:##GAMESVRD##:${gamesvrd5}:g"|
    sed "s:##TCONND_DIR_UDP##:${tconnd13}:g"|
    sed "s:##MYNAME##:${APPID}:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g"|
    sed "s:##SHM_GRM##:2105:g"|
    sed "s:##SHM_NAMESVRTASKQUEUE##:2102:g"|
    sed "s:##SHM_TORMSHM##:2101:g"|
    sed "s:##SHM_NAMESVRCACHESHM##:2103:g"|
    sed "s:##SHM_GCIM##:2104:g"|
    sed "s:##PORT_TBUSD##:10104:g"|
    sed "s:##PORT_TCONND1PORT##:10102:g"|
    sed "s:##PORT_TCONND2PORT##:10103:g"|
    sed "s:##PORT_ZONETCONNDPORT##:10100:g"|
    sed "s:##PORT_TCUSPORT##:10101:g"|
    sed "s:##XYZ_DB##:xyz_db_11:g"|
    sed "s:##IP_ZONECONND##:172.25.40.107:g"|
    sed "s:##IP_CUSCONND##:172.25.40.107:g"|
    sed "s:##IP_DIRCONND##:172.25.40.107:g"|
    sed "s:##IP_UDPCONND##:172.25.40.107:g" > $OUTPUT_PATH/tdir.cfg
    cat $OUTPUT_PATH/tdir.cfg
fi
