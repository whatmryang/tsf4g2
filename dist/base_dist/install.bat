@echo "正在安装TSF4G base组件"

@set "INSTALL_DIR=%1"
@if "%INSTALL_DIR%"=="" set "INSTALL_DIR=C:\program% files\tencent\tsf4g"

@echo "%INSTALL_DIR%"
rem set "INSTALL_DIR=C:\tsf4g_install"
rem makedir for install
rem rmdir %INSTALL_DIR% /S /Q
@if not exist "%INSTALL_DIR%\." mkdir "%INSTALL_DIR%"
@if not exist "%INSTALL_DIR%\include\." mkdir "%INSTALL_DIR%\include"
@if not exist "%INSTALL_DIR%\lib\." mkdir "%INSTALL_DIR%\lib"
@if not exist "%INSTALL_DIR%\examples\." mkdir "%INSTALL_DIR%\examples"
@if not exist "%INSTALL_DIR%\tools\." mkdir "%INSTALL_DIR%\tools"
@if not exist "%INSTALL_DIR%\services\." mkdir "%INSTALL_DIR%\services"
@if not exist "%INSTALL_DIR%\services\tbus_service." mkdir "%INSTALL_DIR%\services\tbus_service"

@copy README "%INSTALL_DIR%\README"
@xcopy .\include "%INSTALL_DIR%\include" /s /q
@xcopy .\lib "%INSTALL_DIR%\lib" /s /q
@xcopy .\examples "%INSTALL_DIR%\examples" /s /q
@xcopy .\tools "%INSTALL_DIR%\tools" /s /q

@xcopy .\services\tbus_service "%INSTALL_DIR%\services\tbus_service" /s /q
@echo "安装完成"
@pause