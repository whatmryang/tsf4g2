import os
import shutil
from package_with_softlink import build_package
from package_with_softlink import find_base_dir

basedir = find_base_dir()
os.chdir(basedir)

os.system("scons -u --enable-debug=0 --tsf4g-publish=1")
if ('posix' == os.name):
    build_package()
