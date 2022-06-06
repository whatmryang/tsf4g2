#!/bin/bash
PATH=$TSF4G_INSTALL_HOME/tools:${PATH}
export PATH

tmeta2tab -B ../cfg/tdr_database.tdr  --meta_name=UserInfo -o userdb.sql
