<tlogdconf version="11">
    <Threading>0 </Threading>
    <EnableViewer>0 </EnableViewer>
    <MaxFD>10240 </MaxFD>
    <MaxScanOnce>0 </MaxScanOnce>
    <ScanRatioOnce>100 </ScanRatioOnce>
    <ScanRatio>0 </ScanRatio>
    <TDRList type="TDRList">
        <Count>1 </Count>
        <TDRs type="TDR">
            <Name>default </Name>
            <Path> </Path>
        </TDRs>
    </TDRList>
    <PDUList type="PDUList">
        <Count>1 </Count>
        <PDUs type="PDU">
            <Name>default </Name>
            <Size>0 </Size>
            <LenParsertype>PDULENPARSERID_BY_NONE </LenParsertype>
            <LenParser type="PDULenParser">
                <NoneParser type="PDULenNoneParser">
                    <MaxPkgLen> 16384 </MaxPkgLen>
                </NoneParser>
            </LenParser>
        </PDUs>
    </PDUList>
    <ListenerList type="ListenerList">
        <Count>1 </Count>
        <Listeners type="Listener">
            <Name>default </Name>
            <Url>udp://127.0.0.1:##TLOGDPORT##</Url>
            <SendBuff>0 </SendBuff>
            <RecvBuff> 32768 </RecvBuff>
            <MaxIdle>0 </MaxIdle>
            <LogBin>0 </LogBin>
            <TransLimit type="TransLimit">
                <PkgHiMark>0 </PkgHiMark>
                <PkgLoMark>0 </PkgLoMark>
                <Period>0 </Period>
                <Decay>0 </Decay>
            </TransLimit>
            <ConnLimit type="ConnLimit">
                <Backlog>0 </Backlog>
                <MaxConn>0 </MaxConn>
                <Permit>0 </Permit>
                <HighMark>0 </HighMark>
                <LowMark>0 </LowMark>
                <Period>0 </Period>
            </ConnLimit>
            <FailWait>0 </FailWait>
            <FailOp>0 </FailOp>
        </Listeners>
    </ListenerList>
    <SerializerList type="SerializerList">
        <Count>1 </Count>
        <Serializers type="Serializer">
            <Name>default </Name>
            <Logger type="TLOGDevAny">
                <Type> FILE </Type>
					<Device>
						<File>
							<Pattern>../../../../mmog_run/log/tsf4g_dev.log </Pattern>
							<BuffSize> 1024000 </BuffSize>
							<SizeLimit> 51200000 </SizeLimit>
							<Precision> 1 </Precision>
							<MaxRotate> 1 </MaxRotate>
							<SyncTime> 0 </SyncTime>
							<NoFindLatest> 0 </NoFindLatest>
						</File>
					</Device>
            </Logger>
            <DropHead>0 </DropHead>
        </Serializers>
    </SerializerList>
    <NetTransList type="NetTransList">
        <Count>1 </Count>
        <NetTrans type="NetTrans">
            <Name>default </Name>
            <LoadRatio>0 </LoadRatio>
            <MaxFD>10240 </MaxFD>
            <UseTimeStamp>0 </UseTimeStamp>
            <Res>0 </Res>
            <PDU>default </PDU>
            <Listener>default </Listener>
            <Serializer>default </Serializer>
        </NetTrans>
    </NetTransList>
</tlogdconf>
