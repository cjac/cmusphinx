; example2.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install makensisw.exe into a directory that the user selects,

;--------------------------------

; The name of the installer
Name "Meeting Recorder"

; The file to write
OutFile "sphinx3.4_win32.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Carnegie Mellon University\Sphinx3.4\"

Var DATADIR

;--------------------------------

; Pages

Page directory

PageEx directory
	PageCallbacks CheckSettings "" VerifyDD
PageExEnd

Page instfiles

;--------------------------------

; The stuff to install
Section "Sphinx Install"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File ".\msdev\programs\decode\Debug\decode.exe"
  File ".\msdev\programs\gausubvq\Debug\gausubvq.exe"
  File ".\msdev\programs\livedecode\Debug\livedecode.exe"
  File ".\msdev\programs\livepretend\Debug\livepretend.exe"


SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

;  CreateDirectory "$SMPROGRAMS\Sphinx"
  CreateShortCut "$SMPROGRAMS\Sphinx.lnk" "$INSTDIR\sphinx.exe" "" "$INSTDIR\sphinx.exe" 0
  
SectionEnd

;check for an old installation
Function CheckSettings

  CreateDirectory $INSTDIR
  ifFileExists "$INSTDIR\settings.txt" has_settings no_settings
  has_settings:
    FileOpen $0 "$INSTDIR\settings.txt" r
    FileRead $0 $R0
    FileClose $0

    ;strip newline
    StrLen $R1 $R0
    IntOp $R1 $R1 - 1
    StrCpy $DATADIR $R0 $R1

    Goto out
  no_settings:
    StrCpy $DATADIR "$INSTDIR"

  out:
FunctionEnd

Function VerifyDD
  StrCmp $DATADIR "" p_abort
  ifFileExists "$DATADIR\*.*" f_end
  

  p_abort:
  MessageBox MB_OK "$DATADIR\*.*"
  Abort
  f_end:
FunctionEnd

;--------------------------------

