#!/bin/sh

if test "$1" == "start"; then
	
    echo "set up bus conf ......"
    ./setupbus.sh > setup.output
    echo "finished to set up bus conf."
	cd send
	rm -f *.log*
	./tbusd --id 0.0.0.1 --conf-file tbusd.xml --tlogconf tbusdlog.xml --bus-key 5000 restart -D
	./test &
	
	
	cd ../trsf
	rm -f *.log*
	./tbusd --id 0.0.0.2 --conf-file tbusd.xml --tlogconf tbusdlog.xml --bus-key 6000 restart -D
	sleep 1
	./test &
	
	
	cd ../recv
	rm -f *.log*
	./tbusd --id 0.0.0.3 --conf-file tbusd.xml --tlogconf tbusdlog.xml --bus-key 7000 restart -D
	sleep 1
	./test &
	
	cd ../
	
	
elif test "$1" == "stop"; then


#	TBUSPID=`ps --User $USER | grep "tbusd" | awk '{ print $1}'`
#	for PID in $TBUSPID
#	do
#		kill -9 $PID
#	done
#	ps --User $USER | grep "tbusd"
	cd send
	./tbusd --id 0.0.0.1 stop
	
	cd ../trsf
	./tbusd --id 0.0.0.2 stop
	
	cd ../recv
	./tbusd --id 0.0.0.3 stop
	
	cd ../
	
elif test "$1" == "list"; then
	echo "###send bus conf###################"
	../../../tools/tbusmgr --conf-file send/tbus_mgr.xml 
	../../../tools/trelaymgr --conf-file send/trelay_mgr.xml 

	echo "###trsf bus conf###################" 
	../../../tools/tbusmgr --conf-file trsf/tbus_mgr.xml 
	../../../tools/trelaymgr --conf-file trsf/trelay_mgr.xml

	echo "###recv bus conf###################"
	../../../tools/tbusmgr --conf-file recv/tbus_mgr.xml 
	../../../tools/trelaymgr --conf-file recv/trelay_mgr.xml 
else
    echo "Usage: $0 start|stop|list"

fi

