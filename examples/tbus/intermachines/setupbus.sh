#!/bin/sh

../../../tools/tbusmgr --conf-file send/tbus_mgr.xml --write
../../../tools/tbusmgr --conf-file trsf/tbus_mgr.xml --write
../../../tools/tbusmgr --conf-file recv/tbus_mgr.xml --write

../../../tools/trelaymgr --conf-file send/trelay_mgr.xml --write
../../../tools/trelaymgr --conf-file trsf/trelay_mgr.xml --write
../../../tools/trelaymgr --conf-file recv/trelay_mgr.xml --write

cd send
rm -f *.log*
cp ../../../../services/tbus_service/tbusd ./


cd ../trsf
rm -f *.log*
cp ../../../../services/tbus_service/tbusd ./


cd ../recv
rm -f *.log*
cp ../../../../services/tbus_service/tbusd ./

cd ../
