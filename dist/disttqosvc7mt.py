import os

#os.chdir("../")
os.system("scons -u --enable-debug=1  --vcversion=vc7 --vcLinkType=MT --no-tag=1 --tsf4g-publish=1 --publish-modules=tqos")
os.system("scons -u --enable-debug=0  --vcversion=vc7 --vcLinkType=MT --tsf4g-publish=1 --publish-modules=tqos  --clean-release=0")
