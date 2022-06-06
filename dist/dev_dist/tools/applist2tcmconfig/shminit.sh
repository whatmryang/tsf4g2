#!/bin/sh
cd /data/mickey/tsf4g_dev_install/mmog_deps/tsf4g_install/apps/
echo "[METABASE]" > tmng.conf
echo "    Key=2201" >> tmng.conf
echo "    Count	= 100" >> tmng.conf
echo "    Size	= 10000000" >> tmng.conf
../tools/tmng --destroy mib
../tools/tmng --create metabase
../tools/tmng --loadmeta ../../../mmog_run/etc/tormsvr/xyz_database.tdr 
../tools/tmng --loadmeta ../../../mmog_run/etc/tormsvr/mail_db.tdr
../tools/tmng --loadmeta ../../../mmog_run/etc/tormsvr/namesvr_db.tdr
