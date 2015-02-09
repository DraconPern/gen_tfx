; Pimpscript file v1.3.0
; Copyright (C) 1999 Nullsoft Inc.
; You can comment by starting lines with ; or #.

; Variables you can use in many of the strings:
;   $PROGRAMFILES (usually c:\program files)
;   $WINDIR (usually c:\windows)
;   $SYSDIR (usually c:\windows\system)
;   $DESKTOP (the desktop directory for the current user)
;   $INSTDIR (whatever the install directory ends up being)
;   $VISDIR  (visualization plug-in directory. DO NOT USE IN DefaultDir)
;   $DSPDIR  (dsp plug-in directory. DO NOT USE IN DefaultDir)

; Name specifies what the installer will use for its name
Name TransparentFX 1.11
Text TransparentFX enables transparency for Winamp's windows.  Visit www.draconpern.com for updates.
OutFile TransparentFX111.exe
SetOutPath Plugins

AddFile F:\myfiles\Windows\gen_tfx\Release\gen_tfx.dll
AddFile F:\myfiles\Windows\gen_tfx\TransparentFX.txt
ExecFile "$WINDIR\notepad.exe" $INSTDIR\Plugins\TransparentFX.txt

