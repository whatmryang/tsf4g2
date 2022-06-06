#!/bin/bash

TSM="./tsm --bid=12"
DATA_HEAD="tsm-data://12/tsmtest:1.2.3/conf"
META_HEAD="tsm-meta://12/tormsvr:14:11/TORMConnectDBInfo:10/";

ipc -r

$TSM loadmeta --file=../../lib_src/tmng/xml/conf.tdr
$TSM loadmeta --file=../../lib_src/tmng/xml/conf.tdr

$TSM loadmeta --file=../../lib_src/tmng/xml/ctrl.tdr
$TSM loadmeta --file=../../lib_src/tmng/xml/ctrl.tdr

$TSM loadmeta --file=../../lib_src/tmng/xml/conf.tdr
$TSM loadmeta --file=../../lib_src/tmng/xml/ctrl.tdr

./tsm list --uri="tsm-meta://12/*"

$TSM loaddata --file=a --meta-uri=${META_HEAD} --uri="${DATA_HEAD}/hello/asdf"
$TSM loaddata --file=a --meta-uri=${META_HEAD} --uri="${DATA_HEAD}/hello/hk"
$TSM loaddata --file=a --meta-uri=${META_HEAD} --uri="${DATA_HEAD}/world/asdf"
$TSM loaddata --file=a --meta-uri=${META_HEAD} --uri="${DATA_HEAD}/world/hk"

./tsm list --uri="tsm-data://12/*"

ipc

./tsm destroy

