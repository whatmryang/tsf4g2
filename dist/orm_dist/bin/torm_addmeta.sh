#!/bin/sh

PATH=$TSF4G_INSTALL_HOME/tools:${PATH}
export PATH

tmng  --create metabase
tmng  --create mib
tmng  --loadmeta ../cfg/tdr_database.xml
