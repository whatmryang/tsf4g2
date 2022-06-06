#!/bin/sh
#Create tables here.

mysql -uroot -e "DROP DATABASE if EXISTS tlog"
mysql -uroot -e "CREATE DATABASE tlog"
mysql -uroot tlog < tlogdb.sql
