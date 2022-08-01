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
!define VERSION "1.3.8"

Name "CnC Suite ${VERSION}"
OutFile "installer_output\CnC Suite USRINST x64 ${VERSION}.exe"             ;MARK: x86/x64

InstallDir "$LOCALAPPDATA\CnC Suite"                                        ;MARK: x86/x64 ($PROGRAMFILES64 for x64)
; NOTE: The program directory should be for the user: C:\Users\hans-\AppData\Local\CnC Suite\... or
    ;   C:\Users\hans-\AppData\Local\Programs\CnC Suite\...



VIProductVersion 1.3.8.0
VIAddVersionKey /LANG=0 "ProductName" "CnC Suite Installer x64"
VIAddVersionKey /LANG=0 "Comments" "Windows 10/8/7"
VIAddVersionKey /LANG=0 "CompanyName" "LaroomySoft"
VIAddVersionKey /LANG=0 "FileDescription" "This file installs CnC Suite on your Computer"
VIAddVersionKey /LANG=0 "LegalCopyright" "(C) LaroomySoft"
VIAddVersionKey /LANG=0 "ProductVersion" "1.3.8"
VIAddVersionKey /LANG=0 "FileVersion" "1.3.8"

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
!insertmacro MUI_PAGE_LICENSE "license.rtf"
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller:
!insertmacro MUI_UNPAGE_WELCOME
;!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_COMPONENTS     ;let the user select if he wants to delete the created userfolder (appdata + projects)
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
; end pages region -----------------------------------------------;
; ----------------------------------------------------------------;
; ----------------------------------------------------------------;
; language region ------------------------------------------------;

!insertmacro MUI_LANGUAGE "English"

; end language region --------------------------------------------;
; ----------------------------------------------------------------;
; ----------------------------------------------------------------;
; variable region ------------------------------------------------;

    Var day
    Var month
    Var year
    Var weekday
    Var hours
    Var minute
    Var seconds

; end variable region --------------------------------------------;


; INSTALLER SECTION **********************************************;
Section "Installer Section"

    ;select current user
    SetShellVarContext current

    ;SetOverwrite on (the default is 'on' so this is not necessary, but maybe set it to 'ifnewer' ?)
    SetOverwrite on

    ;Create files and directories
    SetOutPath "$INSTDIR"
    SetOutPath "$INSTDIR\bin"

    ;    Program files
    File "CnC Suite.VisualElementsManifest.xml"
    File "exe_user_x64\CnC Suite.exe"                        ;MARK: x64 only !
    
    ; NOTE: Write the dependency dll's direct in the installation directory
    ; ************************************************************************************** ;

    ;Visual Studio distributed dll's - Path: C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.24.28127\x64
    File "DLLs\Microsoft.VC142.CRT\concrt140.dll"
    File "DLLs\Microsoft.VC142.CRT\msvcp140.dll"
    File "DLLs\Microsoft.VC142.CRT\msvcp140_1.dll"
    File "DLLs\Microsoft.VC142.CRT\msvcp140_2.dll"
    File "DLLs\Microsoft.VC142.CRT\msvcp140_codecvt_ids.dll"
    File "DLLs\Microsoft.VC142.CRT\vccorlib140.dll"
    File "DLLs\Microsoft.VC142.CRT\vcruntime140.dll"
    File "DLLs\Microsoft.VC142.CRT\vcruntime140_1.dll"


    ;Other dependency files - not all are necessary but it's easier to copy the hole folder
    ;Path: C:\Program Files (x86)\Windows Kits\10\Redist\10.0.18362.0\ucrt\DLLs\...
    File "DLLs\x64\api-ms-win-core-console-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-console-l1-2-0.dll"
    File "DLLs\x64\api-ms-win-core-datetime-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-debug-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-errorhandling-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-file-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-file-l1-2-0.dll"
    File "DLLs\x64\api-ms-win-core-file-l2-1-0.dll"
    File "DLLs\x64\api-ms-win-core-handle-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-heap-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-interlocked-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-libraryloader-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-localization-l1-2-0.dll"
    File "DLLs\x64\api-ms-win-core-memory-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-namedpipe-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-processenvironment-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-processthreads-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-processthreads-l1-1-1.dll"
    File "DLLs\x64\api-ms-win-core-profile-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-rtlsupport-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-string-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-synch-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-synch-l1-2-0.dll"
    File "DLLs\x64\api-ms-win-core-sysinfo-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-timezone-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-core-util-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-conio-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-convert-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-environment-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-filesystem-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-heap-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-locale-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-math-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-multibyte-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-private-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-process-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-runtime-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-stdio-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-string-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-time-l1-1-0.dll"
    File "DLLs\x64\api-ms-win-crt-utility-l1-1-0.dll"
    File "DLLs\x64\ucrtbase.dll"


    ;Write the assets-folder
    SetOutPath "$INSTDIR\bin\assets"
    File "logo_sq70.png"
    File "logo_sq150.png"

    ;   Write the fonts-folder (deprecated - this is not used anymore - the application uses the systemfont consolas!)
    ;SetOutPath "$INSTDIR\fonts"
    ;File "Code New Roman.otf"
    ;File "Courier Prime Code.ttf"
    ;File "Courier Prime Sans.ttf"
    ;File "Inconsolata-Bold.ttf"
    ;File "ProFontWindowsEdit.ttf"
    ;File "PTM55FT.ttf"
    ;File "SVBasicManual.ttf"
    ;File "VeraMono.ttf"

    ;Write the image folder
    SetOutPath "$INSTDIR\image"
    File "Cnc_Suite_mIcon.ico"
    File "cnc3_file_ico.ico"

    ;TODO: write the manual to the application folder!!!! (if it is ready)

    ;Create uninstaller
    WriteUninstaller "$INSTDIR\bin\Uninstall.exe"

    ;Create startmenu-entry
    CreateShortCut "$STARTMENU\CnC Suite.lnk" "$INSTDIR\bin\CnC Suite.exe" ;NOTE: The $STARTMENU variable depends on the SetShellVarContext setting
  
    ;Create desktop shortcut
    CreateShortCut "$DESKTOP\CnC Suite.lnk" "$INSTDIR\bin\CnC Suite.exe"

    ;Write registry (Admin level required - only necessary for Admin Installation (deprecated))
    ;------------------------------------------------------------------------------------------->
    ;Filetype association
    ;WriteRegStr HKCR ".cnc3" "" "cnc.file"
    ;WriteRegStr HKCR ".cnc3\DefaultIcon" "" "$\"$INSTDIR\image\cnc3_file_ico.ico$\""
    ;WriteRegStr HKCR ".cnc3\PersistentHandler" "" "{5e941d80-bf96-11cd-b579-08002b30bfeb}"

    ;WriteRegStr HKCR "cnc.file\DefaultIcon" "" "$\"$INSTDIR\image\cnc3_file_ico.ico$\""
    ;WriteRegStr HKCR "cnc.file\shell\edit\command" "" "$\"$INSTDIR\CnC Suite.exe$\" $\"%1$\""
    ;WriteRegStr HKCR "cnc.file\shell\open\command" "" "$\"$INSTDIR\CnC Suite.exe$\" $\"%1$\""
    ;------------------------------------------------------------------------------------------->
    ${GetTime} "" "L" $day $month $year $weekday $hours $minute $seconds


    ;Register Uninstaller:
    ${If} ${RunningX64}
       SetRegView 64
    ${EndIf}

    ;Write registry (Only for current user (user-installation))
    WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "DisplayName" "CnC Suite"
    WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "DisplayIcon" "$INSTDIR\image\Cnc_Suite_mIcon.ico"   
    WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "UninstallString" "$INSTDIR\bin\Uninstall.exe"
    ;WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "QuietUninstallString" "'$INSTDIR\bin\Uninstall.exe' /SILENT"
    WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "DisplayVersion" "${VERSION}"
    WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "Publisher" "LaroomySoft"
    WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "URLInfoAbout" "https://www.cnc-suite.de"    
    WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "URLUpdateInfo" "https://www.cnc-suite.de"    
    WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "InstallLocation" "$INSTDIR"
    WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "InstallDate" "$hours:$minute - $day/$month/$year"

    ;File-type association for the current user
    ;ProgID:
    WriteRegStr HKCU "Software\Classes\cnc3file" "" "CnC Suite File"
    WriteRegStr HKCU "Software\Classes\cnc3file\DefaultIcon" "" "$\"$INSTDIR\image\cnc3_file_ico.ico$\""
    WriteRegStr HKCU "Software\Classes\cnc3file\shell\open\command" "" "$\"$INSTDIR\bin\CnC Suite.exe$\" $\"%1$\""
    WriteRegStr HKCU "Software\Classes\cnc3file\shell\edit\command" "" "$\"$INSTDIR\bin\CnC Suite.exe$\" $\"%1$\""
    ;FileType:
    WriteRegStr HKCU "Software\Classes\.cnc3" "" "cnc3file"
    WriteRegStr HKCU "Software\Classes\.cnc3" "Content Type" "Application/CnC Suite"
    WriteRegNone HKCU "Software\Classes\.cnc3\OpenWithProgids" "cnc3file" 0

    WriteRegStr HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Extensions" "cnc3" "$\"$INSTDIR\bin\CnC Suite.exe$\"";TODO: delete in uninstaller

    ;Set Application Registry entry (not necessary - only for use by installer and cnc-suite.exe)
    WriteRegStr HKCU "SOFTWARE\CnC_Suite\CurrentVersion" "" "1.0.3"

    SetRegView 32

SectionEnd
; end INSTALLER SECTION ********************************************;
; ******************************************************************;
; UNINSTALLER SECTION **********************************************;
Section "un.Application" uninst_app

    ;select the current user
    SetShellVarContext current

    ;Delete files and folders
    ;----------------------------------------------------------------------->

    ; Remove image-folder (and content):
    RMDir /r /REBOOTOK "$LOCALAPPDATA\CnC Suite\image"
    ; Remove fonts-folder (and content):
    RMDir /r /REBOOTOK "$LOCALAPPDATA\CnC Suite\fonts"
    ; Remove bin-folder (and content)
    RMDir /r /REBOOTOK "$LOCALAPPDATA\CnC Suite\bin"
    ; Remove manual
    RMDir /r /REBOOTOK "$LOCALAPPDATA\CnC Suite\Manual"

    ; Remove the Installation-Directory (DO NOT USE \r !!! - See: https://nsis.sourceforge.io/Reference/RMDir for more info)
    ; This is done later, because the user could have selected the choice to keep some data...
    ;RMDir "$INSTDIR"

    ;Delete startmenu-entry
    Delete "$STARTMENU\CnC Suite.lnk"

    ;Delete desktop-shortcut
    Delete "$DESKTOP\CnC Suite.lnk"

    ;Remove Registry Entries
    ;------------------------------------------------------------------------->

    ${If} ${RunningX64}
       SetRegView 64
    ${EndIf}

    ;Remove file Association
    DeleteRegKey HKCU "Software\Classes\cnc3file"
    DeleteRegKey HKCU "Software\Classes\.cnc3"
    DeleteRegKey HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Extensions\cnc3"
    ;Delete uninstaller key
    DeleteRegKey HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite"
    DeleteRegKey HKCU "SOFTWARE\CnC_Suite"

    SetRegView 32
SectionEnd

;Remove section for the application data -------------------------------------->
Section "un.Application Data" uninst_appdata
    ;select the current user:
    SetShellVarContext current

    ;remove:
    RMDir /r "$DOCUMENTS\CnC Suite\AppData"         ;in case of old-installation
    RMDir /r "$LOCALAPPDATA\CnC Suite\AppData"      ;in case of new-installation

    RMDir /r "$LOCALAPPDATA\CnC Suite\CtrlData"     ;remove ctrl-data folder
    RMDir /r "$LOCALAPPDATA\CnC Suite\control"      ; -||-
SectionEnd

;Remove section for the user data ---------------------------------------------------------->
Section "un.User Data" uninst_userdata
    ;select the current user:
    SetShellVarContext current

    ;old!
    RMDir /r "$DOCUMENTS\CnC Suite\UserStyles"
    RMDir /r "$DOCUMENTS\CnC Suite\Templates"
    ;new!
    RMDir /r "$LOCALAPPDATA\CnC Suite\UserStyles"
    RMDir /r "$LOCALAPPDATA\CnC Suite\Templates"; this folder remains in the documents/CnC Suite folder

    ;if the previous sections are completely executed, the folder must be empty, if so, delete it
    RMDir   "$LOCALAPPDATA\CnC Suite"
SectionEnd

;Remove section for the projects ---------------------------------------------------------------------------->
Section /o "un.Projects and Templates" uninst_projects    ; /o == unselected
    ;select the current user:
    SetShellVarContext current

    RMDir /r "$DOCUMENTS\CnC Suite\Projects"

    ${If} ${SectionIsSelected} ${uninst_userdata}
        ${If} ${SectionIsSelected} ${uninst_projects}
            RMDir "$DOCUMENTS\CnC Suite"                ; Remove the containing folder only if no data is in it!
        ${EndIf}
    ${EndIf}
SectionEnd

LangString UNINSTAPP_DESC ${LANG_ENGLISH} "Removes the Application and all it's Subfiles from the System."
LangString UNINSTAPPDATA_DESC ${LANG_ENGLISH} "Removes the Application Data and Settings from the User-Directory."
LangString UNINSTUSERDATA_DESC ${LANG_ENGLISH} "Removes the user-defined App-Data from the User-Directory."
LangString UNINSTPROJ_DESC ${LANG_ENGLISH} "Removes the Projects- and Templates-Folder and all Content from the User-Directory."


!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${uninst_app} $(UNINSTAPP_DESC)
    !insertmacro MUI_DESCRIPTION_TEXT ${uninst_appdata} $(UNINSTAPPDATA_DESC)
    !insertmacro MUI_DESCRIPTION_TEXT ${uninst_userdata} $(UNINSTUSERDATA_DESC)
    !insertmacro MUI_DESCRIPTION_TEXT ${uninst_projects} $(UNINSTPROJ_DESC)
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END

; end UNINSTALLER SECTION *************************************************************************************;