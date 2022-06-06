#!/bin/sh


#########################set the macros ########################################
TABLECNT=0
TOTAL=100
MYSQLADMIN=mysqladmin
MYSQL=mysql
DBNAME=sdms


#########################do the proper operation here###########################
$MYSQLADMIN create -uroot $DBNAME

while [ $TABLECNT -lt $TOTAL ]
  do
  cat userdata.sql | sed "s/TABLEIDX/$TABLECNT/" | $MYSQL -uroot $DBNAME
  TABLECNT=`expr $TABLECNT + 1`
done

