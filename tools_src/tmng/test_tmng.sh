#!/bin/bash
##############################################################################

STEP_FLAG=1
BACK_FLAG=1

##############################################################################
# step 1: 进入脚本目录
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && {
    OLD_PATH_SAVE=`pwd`
    cd `dirname $0`
    BACK_FLAG=$((BACK_FLAG+1))
}

STEP_FLAG=$((STEP_FLAG+1))
##############################################################################
# step 2: 检查测试临时目录
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && {
    TEST_PATH_NAME="`basename $0`_test"
    [ -e "$TEST_PATH_NAME" ] && {
        echo "Error: path ($TEST_PATH_NAME) alread exist..."
    } || {
        BACK_FLAG=$((BACK_FLAG+1))
    }
}

STEP_FLAG=$((STEP_FLAG+1))
##############################################################################
# step 3: 建立测试临时目录结构并进入临时目录
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && {
    mkdir -p $TEST_PATH_NAME/tdr_{old,new}
    cd $TEST_PATH_NAME
    BACK_FLAG=$((BACK_FLAG+1))
}

STEP_FLAG=$((STEP_FLAG+1))
##############################################################################
# step 4: 检查 tmng 的执行权限
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && {
    TMNG="../../../tools/tmng"
    [ -f $TMNG -a -x $TMNG ] && {
        TMNG="$TMNG --conf-mibkey 34210 --conf-metabasekey 34220"
        BACK_FLAG=$((BACK_FLAG+1))
    } || {
        echo "Error: tmng ($TMNG) not found or not found x flag..."
    }
}

STEP_FLAG=$((STEP_FLAG+1))
##############################################################################
# step 5: 拷贝测试用的临时 tdr文件
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && {
    TDR_FILES=`find ../../.. -type f -name '*.tdr' | grep -v '/.svn/'`
    for F in $TDR_FILES
    do
        N=${F#../../..}
        N=${N//\//_}
        cp $F tdr_old/$N
    done
    BACK_FLAG=$((BACK_FLAG+1))
}

STEP_FLAG=$((STEP_FLAG+1))
##############################################################################
# step 6: 初始化 tmng 共享内
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && {
    $TMNG --create metabase
    $TMNG --create mib
    BACK_FLAG=$((BACK_FLAG+1))
}

STEP_FLAG=$((STEP_FLAG+1))
##############################################################################
# step 7: 加 tdr到 tmng 中，并导出生成新的 tdr文件
#-----------------------------------------------------------------------------
exit

[ $BACK_FLAG -eq $STEP_FLAG ] && {
    for F in tdr_old/*
    do
        $TMNG --loadmeta $F
    done
    BACK_FLAG=$((BACK_FLAG+1))
}

STEP_FLAG=$((STEP_FLAG+1))
##############################################################################
# step 8: 比较新老 tdr文件
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && {
    echo "step: $STEP_FLAG" 
}

##############################################################################
# step -: 恢复初始环境
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && {
    BACK_FLAG=$((BACK_FLAG-1))
} || {
    echo "total steps: $STEP_FLAG, -run- steps: $BACK_FLAG"
}

STEP_FLAG=$((STEP_FLAG-1))
##############################################################################
# step -7: 
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && { BACK_FLAG=$((BACK_FLAG-1))
    $TMNG --destroy mib
    $TMNG --destory metabase
}

STEP_FLAG=$((STEP_FLAG-1))
##############################################################################
# step -6: 
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && { BACK_FLAG=$((BACK_FLAG-1))
}

STEP_FLAG=$((STEP_FLAG-1))
##############################################################################
# step -5: 
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && { BACK_FLAG=$((BACK_FLAG-1))
}

STEP_FLAG=$((STEP_FLAG-1))
##############################################################################
# step -4: 
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && { BACK_FLAG=$((BACK_FLAG-1))
    cd ..
    rm -rf "$TEST_PATH_NAME"
}

STEP_FLAG=$((STEP_FLAG-1))
##############################################################################
# step -3: 
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && { BACK_FLAG=$((BACK_FLAG-1))
}

STEP_FLAG=$((STEP_FLAG-1))
##############################################################################
# step -2: 
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && { BACK_FLAG=$((BACK_FLAG-1))
}

STEP_FLAG=$((STEP_FLAG-1))
##############################################################################
# step -1: 返回到初始目录
#-----------------------------------------------------------------------------

[ $BACK_FLAG -eq $STEP_FLAG ] && { BACK_FLAG=$((BACK_FLAG-1))
    cd $OLD_PATH_SAVE
}

#-----------------------------------------------------------------------------
# THE END
##############################################################################
