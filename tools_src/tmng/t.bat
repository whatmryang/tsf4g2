tsm destroy

tsm --bid=12 loadmeta --file=..\..\lib_src\tmng\xml\conf.tdr
tsm --bid=12 loadmeta --file=..\..\lib_src\tmng\xml\conf.tdr

tsm --bid=12 loadmeta --file=..\..\lib_src\tmng\xml\ctrl.tdr
tsm --bid=12 loadmeta --file=..\..\lib_src\tmng\xml\ctrl.tdr

tsm --bid=12 loadmeta --file=..\..\lib_src\tmng\xml\conf.tdr
tsm --bid=12 loadmeta --file=..\..\lib_src\tmng\xml\ctrl.tdr

tsm list --uri="tsm-meta://12/*"

tsm --bid=12 loaddata --file=a --meta-uri="tsm-meta://12/tormsvr:14:11/TORMConnectDBInfo:10/" --uri="tsm-data://12/tsmtest:1.2.3/conf/hello/asdf"
tsm --bid=12 loaddata --file=a --meta-uri="tsm-meta://12/tormsvr:14:11/TORMConnectDBInfo:10/" --uri="tsm-data://12/tsmtest:1.2.3/conf/hello/hk"
tsm --bid=12 loaddata --file=a --meta-uri="tsm-meta://12/tormsvr:14:11/TORMConnectDBInfo:10/" --uri="tsm-data://12/tsmtest:1.2.3/conf/world/asdf"
tsm --bid=12 loaddata --file=a --meta-uri="tsm-meta://12/tormsvr:14:11/TORMConnectDBInfo:10/" --uri="tsm-data://12/tsmtest:1.2.3/conf/world/hk"

tsm list --uri="tsm-data://12/*"

tsm destroy

