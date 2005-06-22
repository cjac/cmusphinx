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
CPP=xicl6.exe
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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\src\libs3decoder" /I "..\..\..\src" /I "..\..\..\src\libutil" /I "..\..\..\src\libs3decoder\libAPI\\" /I "..\..\..\src\libs3decoder\libam\\" /I "..\..\..\src\libs3decoder\libcep_feat\\" /I "..\..\..\src\libs3decoder\libcommon\\" /I "..\..\..\src\libs3decoder\libdict\\" /I "..\..\..\src\libs3decoder\liblm\\" /I "..\..\..\src\libs3decoder\libsearch\\" /I "..\..\..\src\libs3decoder\libam" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "AD_BACKEND_WIN32" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\src\libs3decoder\libAPI\\" /I "..\..\..\src\libs3decoder\libam\\" /I "..\..\..\src\libs3decoder\libcep_feat\\" /I "..\..\..\src\libs3decoder\libcommon\\" /I "..\..\..\src\libs3decoder\libdict\\" /I "..\..\..\src\libs3decoder\liblm\\" /I "..\..\..\src\libs3decoder\libsearch\\" /I "..\..\..\src\libs3decoder\libam" /I "..\..\..\src" /I "..\..\..\src\libutil" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "AD_BACKEND_WIN32" /YX /FD /GZ /c
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

# Name "libs3decoder - Win32 Release"
# Name "libs3decoder - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcep_feat\agc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\approx_cont_mgau.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\approx_cont_mgau.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\ascr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcommon\bio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcommon\stat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\cb2mllr_io.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libep\classify.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcep_feat\cmn.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcep_feat\cmn_prior.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\cont_mgau.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcommon\corpus.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libdict\dict.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libdict\dict2pid.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libep\endptr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\fast_algo_struct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcep_feat\fe.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcep_feat\fe_interface.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcep_feat\fe_sigproc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcep_feat\feat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\liblm\fillpen.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\gs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\gs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\hmm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\interp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\dag.c
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

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_fsm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_time_switch_tree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_word_switch_tree.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_debug.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\gmm_wrap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libAPI\live_decode_API.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libAPI\live_decode_args.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\liblm\lm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\liblm\lmclass.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\liblm\lmclass.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcommon\logs3.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\adaptor.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\mdef.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcommon\misc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\mllr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\ms_gauden.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\ms_mllr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\ms_senone.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\subvq.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\tmat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libAPI\utt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcommon\vector.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\vithist.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libdict\wid.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcep_feat\agc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\ascr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcommon\bio.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcommon\stat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libep\classify.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcep_feat\cmn.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcep_feat\cmn_prior.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\cont_mgau.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcommon\corpus.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libdict\dict.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libdict\dict2pid.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libep\endptr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\fast_algo_struct.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcep_feat\fe.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcep_feat\fe_internal.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcep_feat\feat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\liblm\fillpen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\flat_fwd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\hmm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\hyp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\interp.h
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

SOURCE=..\..\..\src\libs3decoder\libsearch\srch.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_align.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_allphone.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_fsm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_time_switch_tree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_word_switch_tree.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\srch_debug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\gmm_wrap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\live2.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libAPI\live_decode_API.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libAPI\live_decode_args.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\liblm\lm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcommon\logs3.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\mdef.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcommon\misc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\mllr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\ms_gauden.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\ms_mllr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\ms_senone.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcommon\s3types.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\search.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\subvq.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libam\tmat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\utt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libcommon\vector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libsearch\vithist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs3decoder\libdict\wid.h
# End Source File
# End Group
# End Target
# End Project
