#!/bin/sh

PATH=$TSF4G_INSTALL_HOME/tools:${PATH}
export PATH

tmng --destroy metabase
tmng --destroy mib
