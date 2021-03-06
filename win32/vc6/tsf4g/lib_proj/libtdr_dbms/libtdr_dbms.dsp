# Microsoft Developer Studio Project File - Name="libtdr_dbms" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libtdr_dbms - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libtdr_dbms.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libtdr_dbms.mak" CFG="libtdr_dbms - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libtdr_dbms - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libtdr_dbms - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libtdr_dbms - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../lib"
# PROP Intermediate_Dir "..\..\..\..\..\tsf4g_output\libraries\tdr\libtdr_dbms\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../../deps/scew" /I "../../../../deps/expat/lib" /I "../../../../deps/mysql/include" /I "../../../../include" /I "../../../../libraries/tdr" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "TSF4G_STATIC" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libtdr_dbms - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../lib"
# PROP Intermediate_Dir "..\..\..\..\..\tsf4g_output\libraries\tdr\libtdr_dbms\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../../deps/scew" /I "../../../../deps/expat/lib" /I "../../../../deps/mysql/include" /I "../../../../include" /I "../../../../libraries/tdr" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "TSF4G_STATIC" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../../../lib\libtdr_dbms_d.lib"

!ENDIF 

# Begin Target

# Name "libtdr_dbms - Win32 Release"
# Name "libtdr_dbms - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_auxtools.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_ctypes_info.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_data_initialize.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_data_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_data_sort.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\dbms\tdr_dbdriver_mysql.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_error.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_error_i.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_iostream_i.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_md5.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_metalib_init.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_metalib_manage_i.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_net.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_os.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\dbms\tdr_sql.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_strptime.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_auxtools.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_ctypes_info_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\dbms\tdr_dbdriver_mysql.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_define_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_error_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_iostream_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_md5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_metalib_kernel_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_metalib_manage_i.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\libraries\tdr\tdr_os.h
# End Source File
# End Group
# End Target
# End Project
