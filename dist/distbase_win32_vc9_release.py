import os

os.chdir("../")

os.system("scons -c")
pdbFiles = os.listdir(".\win32_pdb")
pdbFiles.remove('.svn');
for file in pdbFiles:
    os.remove("./win32_pdb/" +file)
os.system("scons --vcversion=vc9 --enable-debug=1 --no-tag=1 --tsf4g-publish=1")
os.system("scons --vcversion=vc9 --enable-debug=0 --clean-release=0")
os.system("scons --vcversion=vc9 --enable-debug=0 --clean-release=0 --tsf4g-publish=1")
