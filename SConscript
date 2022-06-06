Import("buildenv")


SConscript("deps/SConscript")

if buildenv._isWindows:
    SConscript('deps/scew/scew/SConscript')
    SConscript('deps/expat/lib/SConscript')
    SConscript('deps/CUnit/CUnit/Sources/SConscript')

SConscript('lib_src/tbus_i/SConscript')
SConscript('lib_src/SConscript')
if buildenv._isWindows and (buildenv.vcVariant == "vc6" or buildenv.vcVariant == "vc7"):
    print "no gtest"
else:
    SConscript('lib_src/gtest/SConscript')
    SConscript('test_lib/SConscript')
    SConscript('test_services/SConscript')

SConscript('lib_src/cutest/SConscript')

SConscript('tools_src/tdr_tools/tdr/SConscript')

SConscript('tools_src/tmng/SConscript')
SConscript('tools_src/SConscript')
SConscript('tools/SConscript')

SConscript("benchmark/SConscript")
SConscript("examples/SConscript")

if buildenv._isWindows and (buildenv.vcVariant == "vc6" or buildenv.vcVariant == "vc7"):
    print "Nothing more"
else:
    SConscript('tools_src/tdr_tools/tdr2xml/SConscript')

    if not (buildenv._isWindows and buildenv._linkType == 'MT'):
        SConscript('tools_src/tdr_tools/tmeta2tab/SConscript')

    SConscript('tools_src/tdr_tools/tdrio/SConscript')
    SConscript('tools_src/tbusmgr/SConscript')
    SConscript('tools_src/trelaymgr/SConscript')

    SConscript('services_src/tlog_service/SConscript')
    SConscript('services_src/tbus_service/SConscript')

    SConscript('tools_src/tbusconf/SConscript')

    SConscript("unittests_src/tdr/SConscript")
    SConscript("unittests_src/tlog/SConscript")
    SConscript("unittests_src/comm/SConscript")

    SConscript('examples/tdr/net_pack_unpack/SConscript')
    SConscript('examples/tdr/data_input_output/SConscript')

    SConscript('examples/comm/tmempool/SConscript')
    SConscript('examples/tbus/intermachines/SConscript')

    SConscript('examples/tlog/SConscript')
    SConscript('examples/tlog/tlogtdr/SConscript')
    SConscript('examples/tlog/tlogHex/SConscript')
    SConscript("examples/tbus_tdr_tsec/tbustestserver/SConscript")

    SConscript('examples/tapp/multiThread/SConscript')
    SConscript('examples/tapp/multiThreadFull/SConscript')
    SConscript('examples/tapp/helloworld/SConscript')
    SConscript('examples/tapp/helloworld_tsm/SConscript')
    SConscript('examples/tapp/service/SConscript')

SConscript('docs/SConscript')
SConscript('dist/SConscript')

if buildenv.isWin32():
    buildenv.InstallDir(buildenv.getInstallBase()+'/win32_pdb/',"win32_pdb")
elif buildenv.isWin64():
    buildenv.InstallDir(buildenv.getInstallBase()+'/win64_pdb/',"win64_pdb")

SConscript('include/SConscript')
