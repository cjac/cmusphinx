# Microsoft Developer Studio Project File - Name="libs3decoder" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libs3decoder - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libs3decoder.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libs3decoder.mak" CFG="libs3decoder - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libs3decoder - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libs3decoder - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libs3decoder - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\src\\" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "AD_BACKEND_WIN32" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libs3decoder - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\src\\" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "AD_BACKEND_WIN32" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libs3decoder - Win32 Release"
# Name "libs3decoder - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\agc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\approx_cont_mgau.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\approx_cont_mgau.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\args.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\ascr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\beam.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\bio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\cmn.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\cmn_prior.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\cont_mgau.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\corpus.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\dict.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\dict2pid.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\fe_interface.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\fe_sigproc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\feat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\fillpen.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\flat_fwd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\gs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\gs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\hmm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\interp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\kb.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\kbcore.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\lextree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\live_decode.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\lm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\lmclass.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\lmclass.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\logs3.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\mdef.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\misc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\mllr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\ms_gauden.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\ms_mllr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\ms_senone.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\s3_align.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\s3_dag.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\subvq.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\tmat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\utt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\vector.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\vithist.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\wid.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\agc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\args.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\ascr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\beam.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\bio.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\cmn.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\cmn_prior.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\cont_mgau.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\corpus.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\dict.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\dict2pid.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\fe.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\fe_internal.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\feat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\fillpen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\flat_fwd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\hmm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\hyp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\interp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\kb.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\kbcore.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\lextree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\live2.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\lm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\logs3.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\mdef.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\misc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\mllr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\ms_gauden.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\ms_mllr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\ms_senone.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\s3_align.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\s3_dag.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\s3types.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\search.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\subvq.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\tmat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\utt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\vector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\vithist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\wid.h
# End Source File
# End Group
# End Target
# End Project
