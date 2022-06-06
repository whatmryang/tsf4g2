#!/usr/bin/env python
# -*- coding: UTF-8 -*-
##============================================================================
## @Id:       $Id: tsm.py 10522 2010-07-20 02:22:08Z tomaswang $
## @Author:   $Author: tomaswang $
## @Date:     $Date: 2010-07-20 10:22:08 +0800 (星期二, 2010-07-20) $
## @Revision: $Revision: 10522 $
## @HeadURL:  $HeadURL: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/trunk/lib_src/tmng/tsm.py $
##----------------------------------------------------------------------------


import pprint
import struct
import tsm_dump


##----------------------------------------------------------------------------
## tsm class
##----------------------------------------------------------------------------
class tsm:

    _format_list = [
        ('prev', 'I', 4, '0x%08X'),
        ('next', 'I', 4, '0x%08X') ]
    _format_array = [
        ('iBegin',     'I', 4, '0x%08X'),
        ('iEnd',       'I', 4, '0x%08X'),
        ('iMaxHash',   'i', 4, '%d'),
        ('iSize',      'I', 4, '%d'),
        ('iMaxCap',    'i', 4, '%d'),
        ('iCurCap',    'i', 4, '%d'),
        ('stHeadFree', 'list'),
        ('stHeadUsed', 'list') ]
    _format_hash = [
        ('iBegin',       'I', 4, '0x%08X'),
        ('iEnd',         'I', 4, '0x%08X'),
        ('iNodeCount',   'i', 4, '%d'),
        ('iBacketCount', 'i', 4, '%d'),
        ('stNode',       'array'),
        ('stBucket',     'array') ]
    _format_index = [
        ('iMagic',    'I', 4, '0x%08X'),
        ('iLock',     'i', 4, '%d'),
        ('iCrc',      'i', 4, '%d'),
        ('iVersion',  'i', 4, '%d'),
        ('iSelfSize', 'I', 4, '%d'),
        ('stLib',     'array'),
        ('stUri',     'array'),
        ('stHash',    'hash') ]

    _format_data_ptr = [
        ('iRef',      'i', 4, '%d'),
        ('iShmId',    'i', 4, '%d'),
        ('iOffset',   'I', 4, '%d'),
        ('iSize',     'I', 4, '%d') ]
    _format_data_lib = [
        ('stData',       'data_ptr'),
        ('stTime',       'i',    4, '%d'),
        ('iBid',         'i',    4, '%d'),
        ('szLib',        '32s', 32, '%s'),
        ('iLibVer',      'i',    4, '%d'),
        ('iLibBuildVer', 'i',    4, '%d') ]
    _format_data_uri = [
        ('stData',       'data_ptr'),
        ('stTime',       'i',        4, '%d'),
        ('iBid',         'i',        4, '%d'),
        ('szLib',        '32s',     32, '%s'),
        ('iLibVer',      'i',        4, '%d'),
        ('iLibBuildVer', 'i',        4, '%d'),
        ('szMeta',       '32s',     32, '%s'),
        ('iMetaVer',     'i',        4, '%d'),
        ('szUri',        '1024s', 1024, '%s') ]
    _format_shm_head = [
        ('stData',       'data_ptr'),
        ('stTime',       'i',        4, '%d'),
        ('iBid',         'i',        4, '%d'),
        ('szLib',        '32s',     32, '%s'),
        ('iLibVer',      'i',        4, '%d'),
        ('iLibBuildVer', 'i',        4, '%d'),
        ('szMeta',       '32s',     32, '%s'),
        ('iMetaVer',     'i',        4, '%d'),
        ('szUri',        '1024s', 1024, '%s'),
        ('iIsUri',       'i',        4, '%d'),
        ('iMagic',       'i',        4, '0x%08X') ]

    def __init__(self):
        self.text = tsm_dump.dump(tsm_dump.k2i(tsm_dump.INDEX_KEY), 13783040)
        self.pp = pprint.PrettyPrinter(indent=2).pprint
        self.data = {}
        self.flush()

    def flush(self):
        self.head_len = self.unpack('index', self.text, self.data)

    def unpack(self, fmt, data, save, begin=0):
        b, e = begin, 0
        for i in getattr(self, '_format_' + fmt):
            if len(i) == 4:
                n, t, l, f = i
                e = b + l
                save[n] = f % struct.unpack(t, data[b:e])
                if l > 8:
                    save[n] = save[n].rstrip('\0')
                b = e
            elif len(i) == 2:
                n, f = i
                assert not save.has_key(n)
                save[n] = {}
                b = self.unpack(f, data, save[n], b)
            else:
                raise i
        return b

    def get_data(self, fmt, offset, text=None):
        if text is None:
            text = self.text
        tmp = {}
        off = self.unpack(fmt, text, tmp, offset)
        return tmp, offset + off

    def get_lib(self):
        ret = []
        info = self.data['stLib']
        begin = int(info['iBegin'], 16)
        end = int(info['iEnd'], 16)
        f = int(info['stHeadUsed']['next'], 16)
        while begin <= f < end:
            l, o = self.get_data('data_lib', f + 8)
            ret.append(l)
            l, o = self.get_data('list', f)
            f = int(l['next'], 16)
        return ret

    def get_uri(self):
        ret = []
        info = self.data['stUri']
        begin = int(info['iBegin'], 16)
        end = int(info['iEnd'], 16)
        f = int(info['stHeadUsed']['next'], 16)
        while begin <= f < end:
            l, o = self.get_data('data_uri', f + 8)
            ret.append(l)
            l, o = self.get_data('list', f)
            f = int(l['next'], 16)
        return ret


##----------------------------------------------------------------------------
## main
##----------------------------------------------------------------------------
if __name__ == "__main__":
    import sys
    obj = tsm()
    if len(sys.argv) == 1:
        obj.pp(obj.data)
        sys.exit(0)
    text = tsm_dump.dump(int(sys.argv[1]), int(sys.argv[2]))
    data = {}
    obj.unpack('shm_head', text, data)
    obj.pp(data)


##----------------------------------------------------------------------------
## END
##============================================================================
