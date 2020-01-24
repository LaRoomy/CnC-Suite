
; IMPORTANT:        CnC Suite test component!
;_________________________________________________________________________________________________
;THIS IS FOR SINGLE TESTING PURPOSES IN CASE OF PROBLEMS WITH THE USER-INSTALLER!!!!!!!!!!!!!!!!!!
;_________________________________________________________________________________________________

; include_region--------------------------------------------------;
Unicode true
!include "LogicLib.nsh"
!include "MUI2.nsh"
!include "x64.nsh"
!include "FileFunc.nsh"
; end include_region----------------------------------------------;
; ----------------------------------------------------------------;
; ----------------------------------------------------------------;
; general_region--------------------------------------------------;
!define VERSION "1.2.1"

Name "USBCon Install ${VERSION}"
OutFile "USBCon Install ${VERSION}.exe"             ;MARK: x86/x64

InstallDir "$LOCALAPPDATA\testinstaller"                                        ;MARK: x86/x64 ($PROGRAMFILES64 for x64)
; NOTE: The program directory should be for the user: C:\Users\hans-\AppData\Local\CnC Suite\... or
    ;   C:\Users\hans-\AppData\Local\Programs\CnC Suite\...



VIProductVersion 1.3.8.0
VIAddVersionKey /LANG=0 "ProductName" "USBCon Installer"
VIAddVersionKey /LANG=0 "Comments" "Windows 10"
VIAddVersionKey /LANG=0 "CompanyName" "LaroomySoft"
VIAddVersionKey /LANG=0 "FileDescription" "This file installs USBCon on your Computer"
VIAddVersionKey /LANG=0 "LegalCopyright" "(C) LaroomySoft"
VIAddVersionKey /LANG=0 "ProductVersion" "1.2.1"
VIAddVersionKey /LANG=0 "FileVersion" "1.2.1"

RequestExecutionLevel user

; end general_region----------------------------------------------;
; ----------------------------------------------------------------;
; ----------------------------------------------------------------;
; interface_configuration_region ---------------------------------;
BrandingText "Laroomy Designs"

!define MUI_ICON "orange-install.ico"
!define MUI_UNICON "orange-uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "headerimage_left.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "headerimage_left.bmp"
!define MUI_BGCOLOR 0xD2D2D2
!define MUI_ABORTWARNING

!define MUI_WELCOMEFINISHPAGE_BITMAP "welcomeimage.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "welcomeimage_uninstall.bmp"    ;recommended size 164 x 314

; end interface_configuration_region -----------------------------;
; ----------------------------------------------------------------;
; ----------------------------------------------------------------;
; pages region ---------------------------------------------------;

; TODO: define the ui-strings for the pages and make it language-dependent !!


;!insertmacro ${GetTime} "" "L" $day $month $year $weekday $hours $minute $seconds

; Installer:
!insertmacro MUI_PAGE_WELCOME
;!insertmacro MUI_PAGE_LICENSE "license.rtf"
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller:
!insertmacro MUI_UNPAGE_WELCOME
;!insertmacro MUI_UNPAGE_CONFIRM
;!insertmacro MUI_UNPAGE_COMPONENTS     ;let the user select if he wants to delete the created userfolder (appdata + projects)
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
; end pages region -----------------------------------------------;
; ----------------------------------------------------------------;
; ----------------------------------------------------------------;
; language region ------------------------------------------------;

!insertmacro MUI_LANGUAGE "English"



; INSTALLER SECTION **********************************************;
Section "Installer Section"

    ;select current user
    SetShellVarContext current

    ;SetOverwrite on (the default is 'on' so this is not necessary, but maybe set it to 'ifnewer' ?)
    SetOverwrite on

    ;Create files and directories
    SetOutPath "$INSTDIR"
    SetOutPath "$INSTDIR\bin"

    ;Program file(s)
    File "USBCon.exe" 

    ;Create uninstaller
    WriteUninstaller "$INSTDIR\bin\Uninstall.exe"

    ;Create startmenu-entry
    CreateShortCut "$STARTMENU\USBCon.lnk" "$INSTDIR\bin\USBCon.exe"

    DetailPrint "Path to StartmenuEntry:"
    DetailPrint AppData=$APPDATA
  
    ;Create desktop shortcut
    CreateShortCut "$DESKTOP\USBCon.lnk" "$INSTDIR\bin\USBCon.exe"

SectionEnd
; end INSTALLER SECTION ********************************************;
; ******************************************************************;
; UNINSTALLER SECTION **********************************************;
Section "un.Application" uninst_app
    ;select the current user
    SetShellVarContext current

    ;make sure the ....
    SetOutPath $DESKTOP

    ;Delete files and folders
    ;----------------------------------------------------------------------->

    ;RMDir /r /REBOOTOK "$INSTDIR\bin"
    ; Remove the Installation-Directory (DO NOT USE /r !!! - See: https://nsis.sourceforge.io/Reference/RMDir for more info)

    ;Delete "$INSTDIR\bin\USBCon.exe"
    ;Delete "$INSTDIR\bin\Uninstall.exe"

    RMDir /r /REBOOTOK "$LOCALAPPDATA\testinstaller\bin"

    RMDir "$LOCALAPPDATA\testinstaller"

    ;Delete startmenu-entry
    Delete "$STARTMENU\USBCon.lnk"

    ;Delete desktop-shortcut
    Delete "$DESKTOP\USBCon.lnk"

    ;Remove Registry Entries
    ;------------------------------------------------------------------------->

SectionEnd

; end UNINSTALLER SECTION *************************************************************************************;