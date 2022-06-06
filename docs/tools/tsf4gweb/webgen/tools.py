#coding=gbk

import os

def assertTenioPath():
    path = os.getenv('TenioPath')
    if path:
        if not (
            os.path.isdir(path) and
            os.path.isdir(path+r'\TenioSource') and
            os.path.isdir(path+r'\TenioTools') and
            os.path.isdir(path+r'\Document')
            ):
            raw_input('环境变量 TenioPath 设置错误')
            quit()
    else:
        raw_input('请设置环境变量 TenioPath 为 Tenio CC 根目录')
        quit()
    return path
