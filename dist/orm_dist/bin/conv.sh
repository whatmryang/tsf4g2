#!/bin/sh
PATH=$TSF4G_INSTALL_HOME/tools:${PATH}
export PATH

tdr --xml2dr -o ../cfg/tdr_database.tdr ../cfg/tdr_database.xml
