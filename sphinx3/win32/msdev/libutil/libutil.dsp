# Microsoft Developer Studio Project File - Name="libutil" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libutil - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libutil.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libutil.mak" CFG="libutil - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libutil - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libutil - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libutil - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\lib\Release"
# PROP Intermediate_Dir "..\..\..\lib\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\src\libutil" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "AD_BACKEND_WIN32" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy  Release\libutil.lib  ..\..\..\lib\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "libutil - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\lib\Debug"
# PROP Intermediate_Dir "..\..\..\lib\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\src\libutil" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "AD_BACKEND_WIN32" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy  Debug\libutil.lib  ..\..\..\lib\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "libutil - Win32 Release"
# Name "libutil - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\libutil\bitvec.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\case.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\ckd_alloc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\cmd_ln.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\err.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\filename.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\glist.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\hash.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\heap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\profile.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\str2words.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\unlimit.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\libutil\bitvec.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\case.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\ckd_alloc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\cmd_ln.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\err.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\filename.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\glist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\hash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\heap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\io.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\libutil.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\prim_type.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\profile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\str2words.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libutil\unlimit.h
# End Source File
# End Group
# End Target
# End Project
