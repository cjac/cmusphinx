# Microsoft Developer Studio Project File - Name="libsphinx2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libsphinx2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libsphinx2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libsphinx2.mak" CFG="libsphinx2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libsphinx2 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libsphinx2 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libsphinx2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\include" /I "..\..\..\src\libsphinx2\include" /I "..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "FAST8B" /D "LM_CACHE_PROFLE" /D FBSVQ_V8=1 /D UNPADDED_CEP=1 /D "_AFXDLL" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\libsphinx2.lib"

!ELSEIF  "$(CFG)" == "libsphinx2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\include ..\..\..\src\libsphinx2\include" /I "..\..\..\src\libsphinx2\include" /I "..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "FAST8B" /D "LM_CACHE_PROFLE" /D FBSVQ_V8=1 /D UNPADDED_CEP=1 /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\lib\libsphinx2-debug.lib"

!ENDIF 

# Begin Target

# Name "libsphinx2 - Win32 Release"
# Name "libsphinx2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="..\..\..\src\libsphinx2\add-table.c"
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\agc_emax.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\allphone.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\areadchar.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\areaddouble.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\areadfloat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\areadint.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\areadshort.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\awritechar.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\awritedouble.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\awritefloat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\awriteint.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\awriteshort.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\bisearch.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\blk_cdcn_norm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\cache_lm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\cdcn_init.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\cdcn_norm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\cdcn_update.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\cep_rw.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\CM_funcs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\dict.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\eht_quit.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\err.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\f2read.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\f2write.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\fbs_main.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\get_a_word.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\hash.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\hmm_tied_r.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\kb_main.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\lab.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\linklist.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\list.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\live_norm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\lm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\lm_3g.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\lmclass.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\logmsg.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\longio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\norm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\nxtarg.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\pconf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\peek_length.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\phone.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\prime.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\r_agc_noise.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\resfft.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\salloc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\sc_cbook_r.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\sc_vq.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\search.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\searchlat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\skipto.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\str2words.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\strcasecmp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\time_align.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\unlimit.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\util.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libsphinx2\uttproc.c

!IF  "$(CFG)" == "libsphinx2 - Win32 Release"

!ELSEIF  "$(CFG)" == "libsphinx2 - Win32 Debug"

# SUBTRACT CPP /I "..\..\..\include ..\..\..\src\libsphinx2\include"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
