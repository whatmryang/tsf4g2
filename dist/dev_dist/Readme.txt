tsf4g_dev发布方法

1. 首先需要安装Python和tsf4g_dev_deps(一台机器只需要安装一次，如果已经安装好了，可以不用安装这两个包，直接进入第三步)
2. 安装tsf4g_dev_deps，注意，需要使用root安装
3. 安装tsf4g_dev,注意，要用使用者用户安装
4. 进入 ~/tsf4g_dev_install/ 运行setup.sh
5. 进入 mmog_mng运行tcmconsole
6. 在console顺序运行如下命令：
   refreshbuscfg *.*.*.*
   createcfg *.*.*.*
   pushcfg *.*.*.*(由于bus管道大小问题，这步可能会有个别文件失败，重新pushcfg 失败的文件即可)
   start *.*.*.*
   stop *.*.*.*
7. 日志已经集中了，放在~/tsf4g_dev_install/mmog_mng/log/下