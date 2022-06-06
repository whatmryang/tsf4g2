#!/usr/local/bin/python

import os
import shutil
shutil.move('libtdr.a','../../lib/libtdr.a')
shutil.copy('libtdr.so','../../lib/libtdr.so')
shutil.move('libtdr.so','/usr/local/lib/libtdr.so')
