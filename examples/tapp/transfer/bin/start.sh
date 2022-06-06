#!/bin/sh
./transfer \
--conf-file ./conf/transfer.xml \
--log-level 1000 \
--tlogconf ./conf/tlogconf.xml \
--rundata_timer ./conf/rundata_timer.xml \
--log-file ./log/transfer \
--bus-key 34324 \
--id 2.2.2.2 \
restart
