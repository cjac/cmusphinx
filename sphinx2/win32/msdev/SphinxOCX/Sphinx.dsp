# Microsoft Developer Studio Project File - Name="Sphinx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Sphinx - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Sphinx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Sphinx.mak" CFG="Sphinx - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Sphinx - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Sphinx - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Communicator/Sphinx", WHAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Sphinx - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Ext "ocx"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I "..\..\..\src\libsphinx2\include" /I "..\..\..\include" /D "NDEBUG" /D "_SIMPLERESULT" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "AD_BACKEND_WIN32" /FR /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /nodefaultlib
# Begin Custom Build - z
OutDir=.\Release
TargetPath=.\Release\Sphinx.ocx
InputPath=.\Release\Sphinx.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32  /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=link /lib /out:libsphinx2.lib *.obj
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Sphinx - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Ext "ocx"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\src\libsphinx2\include" /I "..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "AD_BACKEND_WIN32" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\fbs8-6-97a\lib"
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\Debug
TargetPath=.\Debug\Sphinx.ocx
InputPath=.\Debug\Sphinx.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "Sphinx - Win32 Release"
# Name "Sphinx - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Sphinx.cpp
# End Source File
# Begin Source File

SOURCE=.\Sphinx.def
# End Source File
# Begin Source File

SOURCE=.\Sphinx.odl
# End Source File
# Begin Source File

SOURCE=.\Sphinx.rc
# End Source File
# Begin Source File

SOURCE=.\SphinxConf.cpp
# End Source File
# Begin Source File

SOURCE=.\SphinxCtl.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\include\ad.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\include\basic_types.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\cont_ad.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\dict.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\err.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\fbs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\hash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\include\kb.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\lm_3g.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\lmclass.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\include\msd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\posixwin32.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\s2types.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\include\search_const.h
# End Source File
# Begin Source File

SOURCE=.\Sphinx.h
# End Source File
# Begin Source File

SOURCE=.\SphinxArgfileUpdate.h
# End Source File
# Begin Source File

SOURCE=.\SphinxBuildLm.h
# End Source File
# Begin Source File

SOURCE=.\SphinxConf.h
# End Source File
# Begin Source File

SOURCE=.\SphinxCtl.h
# End Source File
# Begin Source File

SOURCE=.\SphinxPpg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Sphinx.ico
# End Source File
# Begin Source File

SOURCE=.\Debug\Sphinx.tlb
# End Source File
# Begin Source File

SOURCE=.\SphinxCtl.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
