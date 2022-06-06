/** @file $RCSfile: userdata.sql,v $
  sql script for creating database and table for sdms server
  $Id: userdata.sql,v 1.1.1.1 2008/05/28 07:34:59 kent Exp $
@author $Author: kent $
@date $Date: 2008/05/28 07:34:59 $
@version $Revision: 1.1.1.1 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
@note If DB not exist, invoke this command at mysql console first: "CREATE DATABASE sdms ;"
*/

CREATE TABLE userdata_TABLEIDX
(
  uin             INT UNSIGNED NOT NULL,
  data_type       SMALLINT UNSIGNED NOT NULL,
  status          TINYINT UNSIGNED NOT NULL DEFAULT '0',
  pwd_type        TINYINT UNSIGNED NOT NULL DEFAULT '0',
  passwd          CHAR(32) DEFAULT NULL,
  static_ip       CHAR(32) DEFAULT NULL,
  keyvalue        BIGINT UNSIGNED DEFAULT '0',
  day_plus_amt    BIGINT UNSIGNED DEFAULT '0',
  day_minus_amt   BIGINT UNSIGNED DEFAULT '0',
  mtime           DATETIME NOT NULL DEFAULT '1970-01-01 08:00:00',
  single_plus     BIGINT UNSIGNED DEFAULT '0',
  day_plus        BIGINT UNSIGNED DEFAULT '0',
  single_minus    BIGINT UNSIGNED DEFAULT '0',
  day_minus       BIGINT UNSIGNED DEFAULT '0',
  last_plus_amt   BIGINT UNSIGNED DEFAULT '0',
  last_plus_time  DATETIME NOT NULL DEFAULT '1970-01-01 08:00:00',
  last_minus_amt  BIGINT UNSIGNED DEFAULT '0',
  last_minus_time DATETIME NOT NULL DEFAULT '1970-01-01 08:00:00',
  activate_time   DATETIME NOT NULL DEFAULT '1970-01-01 08:00:00',
  remark          CHAR(32) DEFAULT NULL,
  PRIMARY KEY ( uin, data_type )
) TYPE = INNODB ;

