@echo off

set "ProjectDir=%CD%"

::����debug
"C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv.exe" "%ProjectDir%\tcltapi.sln"  /Rebuild Debug /project "%ProjectDir%\tcltapi\tcltapi.vcproj" 

::����release
"C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv.exe" "%ProjectDir%\tcltapi.sln"  /Rebuild Release /project "%ProjectDir%\tcltapi\tcltapi.vcproj"

echo. & pause