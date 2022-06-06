#!/bin/sh


#########################set the macros ########################################
TABLECNT=0
TOTAL=100
MYSQL=mysql
DBNAME=sdms


#########################do the proper operation here###########################

while [ $TABLECNT -lt $TOTAL ]
  do
 	echo "truncate userdata_${TABLECNT}" | $MYSQL -usdms -psdms $DBNAME 
  TABLECNT=`expr $TABLECNT + 1`
done

