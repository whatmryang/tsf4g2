@echo off

set "ProjectDir=%CD%"



"C:\Program Files\Microsoft Visual Studio\COMMON\MSDev98\Bin\MSDEV.EXE"  %ProjectDir%\tcltapi.dsp /MAKE "tcltapi - all" /REBUILD
@copy %ProjectDir%\Debug\vc60.pdb   %ProjectDir%\..\..\..\..\..\..\win32_lib\vc6\debug\tcltapi_d_vc60.pdb
@copy %ProjectDir%\Release\vc60.pdb %ProjectDir%\..\..\..\..\..\..\win32_lib\vc6\release\tcltapi_vc60.pdb

echo. & pause