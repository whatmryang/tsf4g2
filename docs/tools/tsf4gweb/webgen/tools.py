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
            raw_input('�������� TenioPath ���ô���')
            quit()
    else:
        raw_input('�����û������� TenioPath Ϊ Tenio CC ��Ŀ¼')
        quit()
    return path
