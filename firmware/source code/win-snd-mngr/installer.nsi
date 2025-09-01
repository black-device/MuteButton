!define _NAME "Mute Button"
!define _PATH "BlackDevice\Mute-button"
!define _EXE "mute-btn-systray_1.0.0.exe"

Name "${_NAME}"

; define name of installer
OutFile "${_NAME}-installer.exe"

; define installation directory
InstallDir "$PROGRAMFILES\${_PATH}"

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\${_PATH}" "Install_Dir"


RequestExecutionLevel admin

!include "nsProcess.nsh"

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------


; start default section
Section "${_NAME}"
    SetRegView 64 ; NSIS is a 32-bit installer. We force to write in 64 bit reg section

    ; set the installation directory as the destination for the following actions
    SetOutPath $INSTDIR

    ; Put files there (you can add more File lines too)
    ; MAKE SURE YOU PUT ALL THE FILES HERE IN THE UNINSTALLER TOO
    File "${_EXE}"

    ; Write the installation path into the registry
    WriteRegStr HKLM "Software\${_PATH}" "Install_Dir" "$INSTDIR"

    ; Add registry entry to run the app on start up
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "${_NAME}" "$INSTDIR\${_EXE}"

    ; Write the uninstall keys for Windows
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${_NAME}" "DisplayName" "${_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${_NAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${_NAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${_NAME}" "NoRepair" 1
    WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd


Section "Start Menu Shortcuts (required)"
    SectionIn RO

    CreateDirectory "$SMPROGRAMS\${_PATH}"
    CreateShortcut "$SMPROGRAMS\${_PATH}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
    CreateShortcut "$SMPROGRAMS\${_PATH}\${_NAME}.lnk" "$INSTDIR\${_EXE}" "" "$INSTDIR\${_EXE}" 0
SectionEnd


; uninstaller section
Section "Uninstall"
    SetRegView 64 ; NSIS is a 32-bit installer. We force to write in 64 bit reg section

    ; kill the kill the process before uninstall it
    ${nsProcess::KillProcess} "${_EXE}" $R4
    Sleep 4000

    ; Remove registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${_NAME}"
    DeleteRegKey HKLM "Software\${_PATH}"
    DeleteRegValue HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "${_NAME}"

    ; Remove files and uninstaller
    ; MAKE SURE NOT TO USE A WILDCARD. IF A
    ; USER CHOOSES A STUPID INSTALL DIRECTORY,
    ; YOU'LL WIPE OUT OTHER FILES TOO
    Delete "$INSTDIR\${_EXE}"
    Delete "$INSTDIR\uninstall.exe"

    ; Remove shortcuts, if any
    Delete "$SMPROGRAMS\${_PATH}\*.*"

    ; Remove directories used (only deletes empty dirs)
    RMDir "$SMPROGRAMS\${_PATH}"
    RMDir "$INSTDIR"

SectionEnd

; this function auto-runs after installation is fine
Function .onInstSuccess
  Exec "$INSTDIR\${_EXE}"
FunctionEnd
