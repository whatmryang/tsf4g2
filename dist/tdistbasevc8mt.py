import os

#os.chdir("../")
os.system("scons -u --enable-debug=1 --vcLinkType=MT --no-tag=1 --tsf4g-publish=1 --publish-modules=base")
os.system("scons -u --enable-debug=0 --vcLinkType=MT --tsf4g-publish=1 --publish-modules=base  --clean-release=0")
