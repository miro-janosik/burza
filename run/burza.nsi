; This script is based on example2.nsi, 
;
;--------------------------------

; The name of the installer
Name "Burza"

; The file to write
OutFile "..\Burza_1_0.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Burza

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Burza" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

; The /SOLID lzma compressor created the smallest installer
SetCompressor /SOLID lzma

Icon "data/burza.ico"
UninstallIcon "data/burza.ico"


;--------------------------------

; Pages

; Page components
Page directory
Page instfiles
 
UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Burza"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put files there
  ; When adding new files - do not forget to add them to uninstall part too
  
  File "Burza.exe"
  File "burza.xml"
  File "*.dll"

  File "vcredist_2008_x86.exe"
  File "vcredist_2005_sp1_x86.exe"

  ; doc directory
  SetOutPath $INSTDIR\documents

  ; data directory
  SetOutPath $INSTDIR\data

  File "data\logo_mc.gif"
  File "data\burza.glade"
  File "data\burza.ico"
  File "data\*.txt"

  SetOutPath $INSTDIR\lib\gtk-2.0\2.10.0\loaders
  File lib\gtk-2.0\2.10.0\loaders\*.*

  SetOutPath $INSTDIR\etc\gtk-2.0
  File etc\gtk-2.0\*.*

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Burza "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Burza" "DisplayName" "Burza"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Burza" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Burza" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Burza" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  ; install vs redistributable
  SetDetailsPrint both
  DetailPrint ""
  DetailPrint "Please wait few seconds while installing VS 2005 SP1 redistributable ..."
  SetDetailsPrint none
  ExecWait '"$INSTDIR\vcredist_2005_sp1_x86.exe" /q:a /c:"msiexec /i vcredist.msi /qn /l*v %temp%\vcredist_x86.log"'

  SetDetailsPrint both
  DetailPrint ""
  DetailPrint "Please wait few seconds while installing VS 2008 redistributable ..."
  SetDetailsPrint none
  ExecWait '"$INSTDIR\vcredist_2008_x86.exe" /q'

  SetDetailsPrint both
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcut"

  CreateShortCut "$SMPROGRAMS\Burza.lnk" "$INSTDIR\Burza.exe" "" "$INSTDIR\data\burza.ico" 0
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Desktop Shortcut"

  CreateShortCut "$DESKTOP\Burza.lnk" "$INSTDIR\Burza.exe" "" "$INSTDIR\data\burza.ico" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

  ; uninstall vs redists ...
  ; ... but - its not really necessary to uninstal such upgrade, is it ?
  ; ExecWait '"$INSTDIR\vcredist_2008_x86.exe" /qu'
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Burza"
  DeleteRegKey HKLM SOFTWARE\Burza

  ; Remove files and uninstaller
  Delete $INSTDIR\*.exe
  Delete $INSTDIR\*.dll
  Delete $INSTDIR\*.xml

  ; files created by program
  Delete $INSTDIR\burza.log

  ; etc a lib directories
  Delete $INSTDIR\etc\gtk-2.0\*.*
  RMDir $INSTDIR\etc
  Delete $INSTDIR\lib\gtk-2.0\2.10.0\loaders\*.*
  RMDir $INSTDIR\lib


  ; doc directory
  ; SetOutPath $INSTDIR\documents
  ; data directory
  ; SetOutPath $INSTDIR\data

  Delete $INSTDIR\data\*.gif
  Delete $INSTDIR\data\*.glade
  Delete $INSTDIR\data\*.ico

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Burza.lnk"
  Delete "$DESKTOP\Burza.lnk"

  ; Do not remove RMDir "$INSTDIR"

SectionEnd
