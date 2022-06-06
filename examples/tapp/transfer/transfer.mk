#TSF4G_INSTALL_HOME=/usr/local/tsf4g_release/
TSF4G_INSTALL_HOME=/data/flyma/tsf4g/_trunk

TSF4G_TOOLS=${TSF4G_INSTALL_HOME}/tools
TSF4G_INCLUDE=${TSF4G_INSTALL_HOME}/include
TSF4G_LIB=${TSF4G_INSTALL_HOME}/lib

CC=gcc
COMMLIBS=-Wl,-Bstatic -lscew -lexpat -Wl,-Bdynamic -lpthread
CFLAGES=-g -O2 -fno-strict-aliasing -Wall -Werror -Wextra -pipe -D_NEW_LIC -D_GNU_SOURCE -D_REENTRANT
