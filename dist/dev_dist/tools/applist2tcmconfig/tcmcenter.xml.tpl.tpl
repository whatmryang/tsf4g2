<?xml version="1.0" encoding="utf-8"?>
<tcmcenter CenterdAddr="1.1.1" TconndAddr="##TCONNDID##" BussinessID="##USER_ID##"
           AddrTemple="world:8.zone:8.function:8.instance:8"
		   ConfigSoName="../lib/config_test.so"
		   ConfigBaseDir="../conf/confcreated"><cluster WorkPath="/data/test/mmogxyz/" AutoTimeGap="30" OpTimeout="5">		<world>
			<zone>

			</zone>
		</world></cluster><HostTab><Host Name="localhost" InnerIP="127.0.0.1" /></HostTab><ClusterDeploy>		<world ID="##WORLDID##">
			<zone ID="2">
			</zone>
		</world></ClusterDeploy><TbusConf BussinessID="0" GCIMShmKey="##SHM_GCIM##" GRMShmKey="##SHM_GRM##" TbusdServicePort="##PORT_TBUSD##" ChannelSize="204800" ></TbusConf></tcmcenter>
