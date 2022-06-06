#!/bin/sh

mysql -u root -e "drop database if exists xyz_db_12"
mysql -u root -e "create database xyz_db_12"
cd ../../mmog_run/etc/tormsvr/
mysql -u root xyz_db_12 < /data/mickey/tsf4g_dev_install/mmog_deps/tsf4g_install/apps/../../../mmog_run/etc/tormsvr/role_info.sql
mysql -u root xyz_db_12 < /data/mickey/tsf4g_dev_install/mmog_deps/tsf4g_install/apps/../../../mmog_run/etc/tormsvr/mail_info.sql
mysql -u root xyz_db_12 < /data/mickey/tsf4g_dev_install/mmog_deps/tsf4g_install/apps/../../../mmog_run/etc/tormsvr/xyz_create_table.sql
mysql -u root xyz_db_12 < /data/mickey/tsf4g_dev_install/mmog_deps/tsf4g_install/apps/../../../mmog_run/etc/tormsvr/role_name.sql
