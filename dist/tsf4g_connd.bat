@echo "正在安装TSF4G tconnd 客户端API"

@set "INSTALL_DIR=%1"
@if "%INSTALL_DIR%"=="" set "INSTALL_DIR=C:\program% files\tencent\tsf4g"

@echo "%INSTALL_DIR%"

@if not exist "%INSTALL_DIR%\." mkdir "%INSTALL_DIR%"
@if not exist "%INSTALL_DIR%\include\." mkdir "%INSTALL_DIR%\include"
@if not exist "%INSTALL_DIR%\win32_lib\." mkdir "%INSTALL_DIR%\win32_lib"

@xcopy .\include "%INSTALL_DIR%\include" /s
@xcopy .\win32_lib "%INSTALL_DIR%\win32_lib" /s


@echo "安装完毕"

echo. & pause
