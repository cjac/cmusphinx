# Microsoft Developer Studio Generated NMAKE File, Based on Sphinx.dsp
!IF "$(CFG)" == ""
CFG=Sphinx - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Sphinx - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Sphinx - Win32 Release" && "$(CFG)" != "Sphinx - Win32 Debug" && "$(CFG)" != "Sphinx - Win32 Unicode Debug" && "$(CFG)" != "Sphinx - Win32 Unicode Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Sphinx.mak" CFG="Sphinx - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Sphinx - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Sphinx - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Sphinx - Win32 Unicode Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Sphinx - Win32 Unicode Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Sphinx - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Sphinx.ocx" "$(OUTDIR)\Sphinx.bsc" ".\Release\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\Sphinx.obj"
	-@erase "$(INTDIR)\Sphinx.pch"
	-@erase "$(INTDIR)\Sphinx.res"
	-@erase "$(INTDIR)\Sphinx.sbr"
	-@erase "$(INTDIR)\Sphinx.tlb"
	-@erase "$(INTDIR)\SphinxArgfileUpdate.obj"
	-@erase "$(INTDIR)\SphinxArgfileUpdate.sbr"
	-@erase "$(INTDIR)\SphinxBuildLm.obj"
	-@erase "$(INTDIR)\SphinxBuildLm.sbr"
	-@erase "$(INTDIR)\SphinxConf.obj"
	-@erase "$(INTDIR)\SphinxConf.sbr"
	-@erase "$(INTDIR)\SphinxCtl.obj"
	-@erase "$(INTDIR)\SphinxCtl.sbr"
	-@erase "$(INTDIR)\SphinxPpg.obj"
	-@erase "$(INTDIR)\SphinxPpg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Sphinx.bsc"
	-@erase "$(OUTDIR)\Sphinx.exp"
	-@erase "$(OUTDIR)\Sphinx.lib"
	-@erase "$(OUTDIR)\Sphinx.ocx"
	-@erase "$(OUTDIR)\Sphinx.pdb"
	-@erase ".\Release\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G6 /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Sphinx.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Sphinx.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sphinx.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Sphinx.sbr" \
	"$(INTDIR)\SphinxArgfileUpdate.sbr" \
	"$(INTDIR)\SphinxBuildLm.sbr" \
	"$(INTDIR)\SphinxConf.sbr" \
	"$(INTDIR)\SphinxCtl.sbr" \
	"$(INTDIR)\SphinxPpg.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\Sphinx.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=..\..\sphinx2\src\build\win32\lib\libsphinx2.lib winmm.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\Sphinx.pdb" /debug /machine:I386 /def:".\Sphinx.def" /out:"$(OUTDIR)\Sphinx.ocx" /implib:"$(OUTDIR)\Sphinx.lib" /libpath:"..\fbs8-6-97a\lib" 
DEF_FILE= \
	".\Sphinx.def"
LINK32_OBJS= \
	"$(INTDIR)\Sphinx.obj" \
	"$(INTDIR)\SphinxArgfileUpdate.obj" \
	"$(INTDIR)\SphinxBuildLm.obj" \
	"$(INTDIR)\SphinxConf.obj" \
	"$(INTDIR)\SphinxCtl.obj" \
	"$(INTDIR)\SphinxPpg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Sphinx.res"

"$(OUTDIR)\Sphinx.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\Release
TargetPath=.\Release\Sphinx.ocx
InputPath=.\Release\Sphinx.ocx
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
<< 
	

!ELSEIF  "$(CFG)" == "Sphinx - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Sphinx.ocx" "$(OUTDIR)\Sphinx.tlb" "$(OUTDIR)\Sphinx.bsc" ".\Debug\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\Sphinx.obj"
	-@erase "$(INTDIR)\Sphinx.pch"
	-@erase "$(INTDIR)\Sphinx.res"
	-@erase "$(INTDIR)\Sphinx.sbr"
	-@erase "$(INTDIR)\Sphinx.tlb"
	-@erase "$(INTDIR)\SphinxArgfileUpdate.obj"
	-@erase "$(INTDIR)\SphinxArgfileUpdate.sbr"
	-@erase "$(INTDIR)\SphinxBuildLm.obj"
	-@erase "$(INTDIR)\SphinxBuildLm.sbr"
	-@erase "$(INTDIR)\SphinxConf.obj"
	-@erase "$(INTDIR)\SphinxConf.sbr"
	-@erase "$(INTDIR)\SphinxCtl.obj"
	-@erase "$(INTDIR)\SphinxCtl.sbr"
	-@erase "$(INTDIR)\SphinxPpg.obj"
	-@erase "$(INTDIR)\SphinxPpg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Sphinx.bsc"
	-@erase "$(OUTDIR)\Sphinx.exp"
	-@erase "$(OUTDIR)\Sphinx.ilk"
	-@erase "$(OUTDIR)\Sphinx.lib"
	-@erase "$(OUTDIR)\Sphinx.ocx"
	-@erase "$(OUTDIR)\Sphinx.pdb"
	-@erase ".\Debug\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\fbs8-6-97\include" /I "..\fbs8-6-97\src\libfbs\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D int32=int /D int16=short /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Sphinx.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Sphinx.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sphinx.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Sphinx.sbr" \
	"$(INTDIR)\SphinxArgfileUpdate.sbr" \
	"$(INTDIR)\SphinxBuildLm.sbr" \
	"$(INTDIR)\SphinxConf.sbr" \
	"$(INTDIR)\SphinxCtl.sbr" \
	"$(INTDIR)\SphinxPpg.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\Sphinx.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=..\fbs8-6-97\libdbg\libfbs.lib ..\fbs8-6-97\libdbg\libfbsvq.lib ..\fbs8-6-97\libdbg\libfe.lib ..\fbs8-6-97\libdbg\libcdcn.lib ..\fbs8-6-97\libdbg\libIO.lib ..\fbs8-6-97\libdbg\libad.lib ..\fbs8-6-97\libdbg\libcommon.lib winmm.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\Sphinx.pdb" /debug /machine:I386 /def:".\Sphinx.def" /out:"$(OUTDIR)\Sphinx.ocx" /implib:"$(OUTDIR)\Sphinx.lib" /pdbtype:sept /libpath:"..\fbs8-6-97a\lib" 
DEF_FILE= \
	".\Sphinx.def"
LINK32_OBJS= \
	"$(INTDIR)\Sphinx.obj" \
	"$(INTDIR)\SphinxArgfileUpdate.obj" \
	"$(INTDIR)\SphinxBuildLm.obj" \
	"$(INTDIR)\SphinxConf.obj" \
	"$(INTDIR)\SphinxCtl.obj" \
	"$(INTDIR)\SphinxPpg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Sphinx.res"

"$(OUTDIR)\Sphinx.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\Debug
TargetPath=.\Debug\Sphinx.ocx
InputPath=.\Debug\Sphinx.ocx
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
<< 
	

!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Debug"

OUTDIR=.\DebugU
INTDIR=.\DebugU
# Begin Custom Macros
OutDir=.\DebugU
# End Custom Macros

ALL : "$(OUTDIR)\Sphinx.ocx" "$(OUTDIR)\Sphinx.tlb" ".\DebugU\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\Sphinx.obj"
	-@erase "$(INTDIR)\Sphinx.pch"
	-@erase "$(INTDIR)\Sphinx.res"
	-@erase "$(INTDIR)\Sphinx.tlb"
	-@erase "$(INTDIR)\SphinxArgfileUpdate.obj"
	-@erase "$(INTDIR)\SphinxBuildLm.obj"
	-@erase "$(INTDIR)\SphinxConf.obj"
	-@erase "$(INTDIR)\SphinxCtl.obj"
	-@erase "$(INTDIR)\SphinxPpg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Sphinx.exp"
	-@erase "$(OUTDIR)\Sphinx.ilk"
	-@erase "$(OUTDIR)\Sphinx.lib"
	-@erase "$(OUTDIR)\Sphinx.ocx"
	-@erase "$(OUTDIR)\Sphinx.pdb"
	-@erase ".\DebugU\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "u:\projects\Communicator\src\fbs8-6-97\include" /I "u:\projects\Communicator\src\fbs8-6-97\src\libfbs\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /Fp"$(INTDIR)\Sphinx.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Sphinx.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sphinx.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=libfbs.lib libfbsvq.lib libfe.lib libio.lib libcommon.lib libad.lib libcdcn.lib winmm.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\Sphinx.pdb" /debug /machine:I386 /def:".\Sphinx.def" /out:"$(OUTDIR)\Sphinx.ocx" /implib:"$(OUTDIR)\Sphinx.lib" /pdbtype:sept /libpath:"..\fbs8-6-97\lib" 
DEF_FILE= \
	".\Sphinx.def"
LINK32_OBJS= \
	"$(INTDIR)\Sphinx.obj" \
	"$(INTDIR)\SphinxArgfileUpdate.obj" \
	"$(INTDIR)\SphinxBuildLm.obj" \
	"$(INTDIR)\SphinxConf.obj" \
	"$(INTDIR)\SphinxCtl.obj" \
	"$(INTDIR)\SphinxPpg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Sphinx.res"

"$(OUTDIR)\Sphinx.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\DebugU
TargetPath=.\DebugU\Sphinx.ocx
InputPath=.\DebugU\Sphinx.ocx
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
<< 
	

!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Release"

OUTDIR=.\ReleaseU
INTDIR=.\ReleaseU
# Begin Custom Macros
OutDir=.\ReleaseU
# End Custom Macros

ALL : "$(OUTDIR)\Sphinx.ocx" "$(OUTDIR)\Sphinx.tlb" ".\ReleaseU\regsvr32.trg"


CLEAN :
	-@erase "$(INTDIR)\Sphinx.obj"
	-@erase "$(INTDIR)\Sphinx.pch"
	-@erase "$(INTDIR)\Sphinx.res"
	-@erase "$(INTDIR)\Sphinx.tlb"
	-@erase "$(INTDIR)\SphinxArgfileUpdate.obj"
	-@erase "$(INTDIR)\SphinxBuildLm.obj"
	-@erase "$(INTDIR)\SphinxConf.obj"
	-@erase "$(INTDIR)\SphinxCtl.obj"
	-@erase "$(INTDIR)\SphinxPpg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Sphinx.exp"
	-@erase "$(OUTDIR)\Sphinx.lib"
	-@erase "$(OUTDIR)\Sphinx.ocx"
	-@erase ".\ReleaseU\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\fbs8-6-97\include" /I "..\fbs8-6-97\src\libfbs\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /D int32=int /D int16=short /Fp"$(INTDIR)\Sphinx.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Sphinx.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Sphinx.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=libfbs.lib libfbsvq.lib libfe.lib libio.lib libcommon.lib libad.lib libcdcn.lib winmm.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\Sphinx.pdb" /machine:I386 /def:".\Sphinx.def" /out:"$(OUTDIR)\Sphinx.ocx" /implib:"$(OUTDIR)\Sphinx.lib" 
DEF_FILE= \
	".\Sphinx.def"
LINK32_OBJS= \
	"$(INTDIR)\Sphinx.obj" \
	"$(INTDIR)\SphinxArgfileUpdate.obj" \
	"$(INTDIR)\SphinxBuildLm.obj" \
	"$(INTDIR)\SphinxConf.obj" \
	"$(INTDIR)\SphinxCtl.obj" \
	"$(INTDIR)\SphinxPpg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Sphinx.res"

"$(OUTDIR)\Sphinx.ocx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\ReleaseU
TargetPath=.\ReleaseU\Sphinx.ocx
InputPath=.\ReleaseU\Sphinx.ocx
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
<< 
	

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Sphinx.dep")
!INCLUDE "Sphinx.dep"
!ELSE 
!MESSAGE Warning: cannot find "Sphinx.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Sphinx - Win32 Release" || "$(CFG)" == "Sphinx - Win32 Debug" || "$(CFG)" == "Sphinx - Win32 Unicode Debug" || "$(CFG)" == "Sphinx - Win32 Unicode Release"
SOURCE=.\Sphinx.cpp

!IF  "$(CFG)" == "Sphinx - Win32 Release"


"$(INTDIR)\Sphinx.obj"	"$(INTDIR)\Sphinx.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Debug"


"$(INTDIR)\Sphinx.obj"	"$(INTDIR)\Sphinx.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Debug"


"$(INTDIR)\Sphinx.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Release"


"$(INTDIR)\Sphinx.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ENDIF 

SOURCE=.\Sphinx.odl

!IF  "$(CFG)" == "Sphinx - Win32 Release"

MTL_SWITCHES=/nologo /D "NDEBUG" /tlb "$(OUTDIR)\Sphinx.tlb" /mktyplib203 /o "NUL" /win32 

"$(OUTDIR)\Sphinx.tlb" : $(SOURCE) "$(OUTDIR)"
	$(MTL) @<<
  $(MTL_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Debug"

MTL_SWITCHES=/nologo /D "_DEBUG" /tlb "$(OUTDIR)\Sphinx.tlb" /mktyplib203 /o "NUL" /win32 

"$(OUTDIR)\Sphinx.tlb" : $(SOURCE) "$(OUTDIR)"
	$(MTL) @<<
  $(MTL_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Debug"

MTL_SWITCHES=/nologo /D "_DEBUG" /tlb "$(OUTDIR)\Sphinx.tlb" /mktyplib203 /o "NUL" /win32 

"$(OUTDIR)\Sphinx.tlb" : $(SOURCE) "$(OUTDIR)"
	$(MTL) @<<
  $(MTL_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Release"

MTL_SWITCHES=/nologo /D "NDEBUG" /tlb "$(OUTDIR)\Sphinx.tlb" /mktyplib203 /o "NUL" /win32 

"$(OUTDIR)\Sphinx.tlb" : $(SOURCE) "$(OUTDIR)"
	$(MTL) @<<
  $(MTL_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\Sphinx.rc

!IF  "$(CFG)" == "Sphinx - Win32 Release"


"$(INTDIR)\Sphinx.res" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.tlb"
	$(RSC) /l 0x409 /fo"$(INTDIR)\Sphinx.res" /i "Release" /d "NDEBUG" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Debug"


"$(INTDIR)\Sphinx.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\Sphinx.res" /i "Debug" /d "_DEBUG" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Debug"


"$(INTDIR)\Sphinx.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\Sphinx.res" /i "DebugU" /d "_DEBUG" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Release"


"$(INTDIR)\Sphinx.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\Sphinx.res" /i "ReleaseU" /d "NDEBUG" /d "_AFXDLL" $(SOURCE)


!ENDIF 

SOURCE=.\SphinxArgfileUpdate.cpp

!IF  "$(CFG)" == "Sphinx - Win32 Release"


"$(INTDIR)\SphinxArgfileUpdate.obj"	"$(INTDIR)\SphinxArgfileUpdate.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Debug"


"$(INTDIR)\SphinxArgfileUpdate.obj"	"$(INTDIR)\SphinxArgfileUpdate.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Debug"


"$(INTDIR)\SphinxArgfileUpdate.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Release"


"$(INTDIR)\SphinxArgfileUpdate.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ENDIF 

SOURCE=.\SphinxBuildLm.cpp

!IF  "$(CFG)" == "Sphinx - Win32 Release"


"$(INTDIR)\SphinxBuildLm.obj"	"$(INTDIR)\SphinxBuildLm.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Debug"


"$(INTDIR)\SphinxBuildLm.obj"	"$(INTDIR)\SphinxBuildLm.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Debug"


"$(INTDIR)\SphinxBuildLm.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Release"


"$(INTDIR)\SphinxBuildLm.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ENDIF 

SOURCE=.\SphinxConf.cpp

!IF  "$(CFG)" == "Sphinx - Win32 Release"


"$(INTDIR)\SphinxConf.obj"	"$(INTDIR)\SphinxConf.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Debug"


"$(INTDIR)\SphinxConf.obj"	"$(INTDIR)\SphinxConf.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Debug"


"$(INTDIR)\SphinxConf.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Release"


"$(INTDIR)\SphinxConf.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ENDIF 

SOURCE=.\SphinxCtl.cpp

!IF  "$(CFG)" == "Sphinx - Win32 Release"


"$(INTDIR)\SphinxCtl.obj"	"$(INTDIR)\SphinxCtl.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Debug"


"$(INTDIR)\SphinxCtl.obj"	"$(INTDIR)\SphinxCtl.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Debug"


"$(INTDIR)\SphinxCtl.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Release"


"$(INTDIR)\SphinxCtl.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ENDIF 

SOURCE=.\SphinxPpg.cpp

!IF  "$(CFG)" == "Sphinx - Win32 Release"


"$(INTDIR)\SphinxPpg.obj"	"$(INTDIR)\SphinxPpg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Debug"


"$(INTDIR)\SphinxPpg.obj"	"$(INTDIR)\SphinxPpg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Debug"


"$(INTDIR)\SphinxPpg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Release"


"$(INTDIR)\SphinxPpg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Sphinx.pch"


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "Sphinx - Win32 Release"

CPP_SWITCHES=/nologo /G6 /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Sphinx.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\Sphinx.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\fbs8-6-97\include" /I "..\fbs8-6-97\src\libfbs\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D int32=int /D int16=short /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Sphinx.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\Sphinx.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "u:\projects\Communicator\src\fbs8-6-97\include" /I "u:\projects\Communicator\src\fbs8-6-97\src\libfbs\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /Fp"$(INTDIR)\Sphinx.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\Sphinx.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Sphinx - Win32 Unicode Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "..\fbs8-6-97\include" /I "..\fbs8-6-97\src\libfbs\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /D int32=int /D int16=short /Fp"$(INTDIR)\Sphinx.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\Sphinx.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

