@echo off

set "ProjectDir=%CD%"

::生成debug
"C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv.exe" "%ProjectDir%\tcltapi.sln"  /Rebuild Debug /project "%ProjectDir%\tcltapi\tcltapi.vcproj" 

::生成release
"C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv.exe" "%ProjectDir%\tcltapi.sln"  /Rebuild Release /project "%ProjectDir%\tcltapi\tcltapi.vcproj"

echo. & pause