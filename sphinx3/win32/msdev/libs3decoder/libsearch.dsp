# Microsoft Developer Studio Project File - Name="libsearch" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libsearch - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libsearch.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libsearch.mak" CFG="libsearch - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libsearch - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libsearch - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "libsearch - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\..\..\lib\Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\lib\Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "AD_BACKEND_WIN32" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libsearch - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\..\..\lib\Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\lib\Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "AD_BACKEND_WIN32" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libsearch - Win32 Release"
# Name "libsearch - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\ascr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\blkarray_list.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\dag.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\flat_fwd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\fsg_history.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\fsg_lextree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\fsg_psubtree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\fsg_search.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\gmm_wrap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\kb.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\kbcore.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\lextree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_align.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_allphone.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_debug.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_flat_fwd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_fsg.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_output.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_time_switch_tree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_word_switch_tree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\vithist.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\whmm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\word_fsg.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\word_graph.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\word_ugprob.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\ascr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\blkarray_list.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\dag.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\flat_fwd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\fsg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\fsg_history.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\fsg_lextree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\fsg_psubtree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\fsg_search.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\gmm_wrap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\hyp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\kb.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\kbcore.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\lextree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\search.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_debug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_flat_fwd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_fsg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_output.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_time_switch_tree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_word_switch_tree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\vithist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\whmm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\word_fsg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\word_graph.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\word_ugprob.h
# End Source File
# End Group
# End Target
# End Project
