rem ..\..\..\services\tbus_service\tbusd.exe /install
copy  ..\..\..\services\tbus_service\tbusd.exe .\
tbusd.exe  --conf-file=tbusd.xml --tlogconf=tbusdlog.xml  -D start
pause