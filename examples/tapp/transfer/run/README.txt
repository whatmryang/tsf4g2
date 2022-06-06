本目录下有三个目录，分别对应 client tconnd servers,
与 ../bin 共同使用搭建一个的测试环境, 用来演示transfer框架

搭建步骤如下:

step 1 配置共享内存
    tbusmgr -C ./tbusmgr.xml -W

step 2 启动模拟的 dirtysvr 服务程序
    cd ./dirtysvr/ && ./start_dirtysvr.sh

step 3 启动 transfer 服务程序
    cd ../bin/ && ./start.sh

step 4 启动 tconnd 接入程序
    cd ./tconnd/ && ./starttconnd.sh

step 5 启动模拟 client 程序
    cd ./client/ && ./startclient.sh
