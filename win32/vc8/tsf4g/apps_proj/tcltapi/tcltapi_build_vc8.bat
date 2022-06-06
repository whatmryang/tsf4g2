@echo off

set "ProjectDir=%CD%"

@echo "%ProjectDir%"

::生成debug
"D:\Program Files\Microsoft Visual Studio 8\Common7\IDE\devenv.exe" "%ProjectDir%\tcltapi\tcltapi.sln"  /Rebuild Debug /project "%ProjectDir%\tcltapi\tcltapi.vcproj" 

::生成release
"D:\Program Files\Microsoft Visual Studio 8\Common7\IDE\devenv.exe" "%ProjectDir%\tcltapi\tcltapi.sln"  /Rebuild Release /project "%ProjectDir%\tcltapi\tcltapi.vcproj"

echo. & pause