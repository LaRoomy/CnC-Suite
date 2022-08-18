; include_region--------------------------------------------------;
Unicode true
!include "LogicLib.nsh"
!include "MUI2.nsh"
!include "x64.nsh"
; end include_region----------------------------------------------;
; ----------------------------------------------------------------;
; ----------------------------------------------------------------;
; general_region--------------------------------------------------;
!define VERSION "1.3.8"
!define REDISTRIBUTABLE_FILENAME "redistributables\vc_redist.x86.exe"    ;MARK x86/x64

Name "CnC Suite ${VERSION}"
OutFile "installer_output\CnC Suite Installer x86 ${VERSION}.exe"       ;MARK: x86/x64

InstallDir "$PROGRAMFILES\CnC Suite"                                    ;MARK: x86/x64 ($PROGRAMFILES64 for x64)

RequestExecutionLevel admin

VIProductVersion 1.0.3.0
VIAddVersionKey /LANG=0 "ProductName" "CnC Suite Installer"
VIAddVersionKey /LANG=0 "Comments" "Windows 10/8/7"
VIAddVersionKey /LANG=0 "CompanyName" "Laroomy Designs"
VIAddVersionKey /LANG=0 "FileDescription" "This file installs CnC Suite on your Computer"
VIAddVersionKey /LANG=0 "LegalCopyright" "(C) Laroomy Designs"
VIAddVersionKey /LANG=0 "ProductVersion" "1.3.8"
VIAddVersionKey /LANG=0 "FileVersion" "1.3.8"

;Var finishbuttontext

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

    ;no variables yet!

; end variable region --------------------------------------------;

;TODO's:
; - Write Registry, don't forget to write the version to hklm to check it in installer
;
; - Check for previous version and remove it or tell the user that it is already installed


; INSTALLER SECTION **********************************************;
Section "Installer Section"

    ;select current user
    SetShellVarContext current

    ;SetOverwrite on (the default is 'on' so this is not necessary, but maybe set it to 'ifnewer' ?)
    SetOverwrite on

    ;Create the temporary installer for the redistributable
    SetOutPath "$LOCALAPPDATA"
    File "${REDISTRIBUTABLE_FILENAME}"
    
    ; -> execute the external installer
    ExecWait '"$LOCALAPPDATA\${REDISTRIBUTABLE_FILENAME}"  "/quiet" "/norestart"' ;"/install" "/silent" "/norestart" ;/install /passive /silent /norestart
    ;IfErrors redistributable_error

    ;Create files and directories
    SetOutPath "$INSTDIR"

    File "buildOutput\out_x86\CnC Suite.exe"                            ;MARK: x86/x64
    File "CnC Suite.VisualElementsManifest.xml"

    SetOutPath "$INSTDIR\assets"

    File "logo_sq70.png"
    File "logo_sq150.png"

    SetOutPath "$INSTDIR\fonts"

    File "Code New Roman.otf"
    File "Courier Prime Code.ttf"
    File "Courier Prime Sans.ttf"
    File "Inconsolata-Bold.ttf"
    File "ProFontWindowsEdit.ttf"
    File "PTM55FT.ttf"
    File "SVBasicManual.ttf"
    File "VeraMono.ttf"

    SetOutPath "$INSTDIR\image"

    File "Cnc_Suite_mIcon.ico"
    File "cnc3_file_ico.ico"

    ;Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ;Create startmenu-entry
    CreateShortCut "$SMPROGRAMS\CnC Suite.lnk" "$INSTDIR\CnC Suite.exe"
    ; the following section is disabled because in this installer we don't want to create a folder in the start-menu
    ; old: CreateDirectory "$SMPROGRAMS\CnC Suite"
    ; old: CreateShortCut "$SMPROGRAMS\CnC Suite\CnC Suite.lnk" "$INSTDIR\CnC Suite.exe"

    ;Create desktop shortcut
    CreateShortCut "$DESKTOP\CnC Suite.lnk" "$INSTDIR\CnC Suite.exe"

    ;Write registry
    ;------------------------------------------------------------------------------------------->
    ;Filetype association
    WriteRegStr HKCR ".cnc3" "" "cnc.file"
    WriteRegStr HKCR ".cnc3\DefaultIcon" "" "$\"$INSTDIR\image\cnc3_file_ico.ico$\""
    WriteRegStr HKCR ".cnc3\PersistentHandler" "" "{5e941d80-bf96-11cd-b579-08002b30bfeb}"

    WriteRegStr HKCR "cnc.file\DefaultIcon" "" "$\"$INSTDIR\image\cnc3_file_ico.ico$\""
    WriteRegStr HKCR "cnc.file\shell\edit\command" "" "$\"$INSTDIR\CnC Suite.exe$\" $\"%1$\""
    WriteRegStr HKCR "cnc.file\shell\open\command" "" "$\"$INSTDIR\CnC Suite.exe$\" $\"%1$\""
    ;------------------------------------------------------------------------------------------->
    ;Register Uninstaller:
    ${If} ${RunningX64}
       SetRegView 64
    ${EndIf}

    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "DisplayName" "CnC Suite"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "DisplayIcon" "$INSTDIR\image\Cnc_Suite_mIcon.ico"   
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "Publisher" "Laroomy Designs"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite" "URLInfoAbout" "http://www.cnc-suite.de"    
    

    ;Set Application Registry entry
    WriteRegStr HKLM "SOFTWARE\CnC_Suite\CurrentVersion" "" "1.0.3"

    SetRegView 32

    goto finalize

;redistributable_error:
    ;MessageBox MB_OK|MB_ICONSTOP "Error installing the required components for CnC Suite.$\nInstallation will be canceled."
    ;Delete "$LOCALAPPDATA\${REDISTRIBUTABLE_FILENAME}"
    ;Quit

finalize:
    ;clean up
    Delete "$LOCALAPPDATA\${REDISTRIBUTABLE_FILENAME}"

SectionEnd
; end INSTALLER SECTION ********************************************;
; ******************************************************************;
; UNINSTALLER SECTION **********************************************;
Section "un.Application" uninst_app

    ;Delete files and folders
    ;----------------------------------------------------------------------->
    Delete "$INSTDIR\image\Cnc_Suite_mIcon.ico"
    Delete "$INSTDIR\image\cnc3_file_ico.ico"

    RMDir "$INSTDIR\image"

    Delete "$INSTDIR\fonts\Code New Roman.otf"
    Delete "$INSTDIR\fonts\Courier Prime Code.ttf"
    Delete "$INSTDIR\fonts\Courier Prime Sans.ttf"
    Delete "$INSTDIR\fonts\Inconsolata-Bold.ttf"
    Delete "$INSTDIR\fonts\ProFontWindowsEdit.ttf"
    Delete "$INSTDIR\fonts\PTM55FT.ttf"
    Delete "$INSTDIR\fonts\SVBasicManual.ttf"
    Delete "$INSTDIR\fonts\VeraMono.ttf"

    RMDir "$INSTDIR\fonts"

    Delete "$INSTDIR\assets\logo_sq70.png"
    Delete "$INSTDIR\assets\logo_sq150.png"

    RMDir "$INSTDIR\assets"

    Delete "$INSTDIR\Uninstall.exe"
    Delete "$INSTDIR\CnC Suite.exe"
    Delete "$INSTDIR\CnC Suite.VisualElementsManifest.xml"

    RMDir "$INSTDIR"

    ;Delete startmenu-entries
    Delete "$SMPROGRAMS\CnC Suite\CnC Suite.lnk"
    RMDir "$SMPROGRAMS\CnC Suite"

    ;Delete desktop-shortcut
    Delete "$DESKTOP\CnC Suite.lnk"

    ;Remove Registry Entries
    ;------------------------------------------------------------------------->
    ;Delete Filetype associations
    DeleteRegKey HKCR ".cnc3"
    DeleteRegKey HKCR "cnc.file"

    ${If} ${RunningX64}
       SetRegView 64
    ${EndIf}

    ;Delete uninstaller key
    DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CnC_Suite"
    DeleteRegKey HKLM "SOFTWARE\CnC_Suite\CurrentVersion"

    SetRegView 32
SectionEnd

;Remove section for the application data -------------------------------------->
Section "un.Application Data" uninst_appdata
    RMDir /r "$DOCUMENTS\CnC Suite\AppData"         ;in case of old-installation
    RMDir /r "$LOCALAPPDATA\CnC Suite\AppData"      ;in case of new-installation
SectionEnd

;Remove section for the user data ---------------------------------------------------------->
Section "un.User Data" uninst_userdata
    ;old!
    RMDir /r "$DOCUMENTS\CnC Suite\UserStyles"
    RMDir /r "$DOCUMENTS\CnC Suite\Samples"
    ;new!
    RMDir /r "$LOCALAPPDATA\CnC Suite\UserStyles"
    RMDir /r "$LOCALAPPDATA\CnC Suite\Samples"

SectionEnd

;Remove section for the projects ---------------------------------------------------------------------------->
Section "un.Projects" uninst_projects
    RMDir /r "$DOCUMENTS\CnC Suite\Projects"

    ${If} ${SectionIsSelected} ${uninst_appdata}
        ${If} ${SectionIsSelected} ${uninst_userdata}
            ${If} ${SectionIsSelected} ${uninst_projects}
                RMDir "$DOCUMENTS\CnC Suite"
            ${EndIf}
        ${EndIf}
    ${EndIf}
SectionEnd

LangString UNINSTAPP_DESC ${LANG_ENGLISH} "Removes the Application and all it's Subfiles from the System."
LangString UNINSTAPPDATA_DESC ${LANG_ENGLISH} "Removes the Application Data and Settings from the User-Directory."
LangString UNINSTUSERDATA_DESC ${LANG_ENGLISH} "Removes the user-defined App-Data from the User-Directory."
LangString UNINSTPROJ_DESC ${LANG_ENGLISH} "Removes the Projects Folder and all it's Content from the User-Directory."


!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${uninst_app} $(UNINSTAPP_DESC)
    !insertmacro MUI_DESCRIPTION_TEXT ${uninst_appdata} $(UNINSTAPPDATA_DESC)
    !insertmacro MUI_DESCRIPTION_TEXT ${uninst_userdata} $(UNINSTUSERDATA_DESC)
    !insertmacro MUI_DESCRIPTION_TEXT ${uninst_projects} $(UNINSTPROJ_DESC)
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END

; end UNINSTALLER SECTION *************************************************************************************;