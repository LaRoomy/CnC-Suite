#pragma once
// FILEKEY >> Default
#define		DATAKEY_EDITOR_UPPERCASE				L"EditProperties_Uppercase"//type: bool //default: true
#define		DATAKEY_EDITOR_AUTOCOMPLETE				L"EditProperties_Autocomplete"//type: bool //default: true
#define		DATAKEY_EDITOR_AUTONUM					L"EditProperties_Autonum"//type: bool //default: true
#define		DATAKEY_EDITOR_TEXTCOLOR				L"EditProperties_Textcolor"//type: bool //default: true
#define		DATAKEY_EDITOR_FOCUSMARK				L"EditProperties_FocusMark"//type: bool //default: true

#define		DATAKEY_EDITOR_FONT_FAMILY_INDEX		L"fontfamilycomboindex"//type: int //default: 0
#define		DATAKEY_EDITOR_FONT_HEIGHT				L"charheightcomboindex"//type: int //default: 275
#define		DATAKEY_EDITOR_LINEOFFSET				L"richeditlineoffset"//type: int //default: 72
#define		DATAKEY_EDITOR_BOLDSTYLE				L"richeditboldstyle"//type: bool //default: true

#define		DATAKEY_SETTINGS_DESIGN					L"designcombo"//type: int //default: 0
#define		DATAKEY_SETTINGS_LANGUAGE				L"langcombo"//type: int //default: 0
#define		DATAKEY_SETTINGS_COLORSCHEME			L"colorschemecombo"//type: int //default: 0
#define		DATAKEY_SETTINGS_COLORSCHEME_USAGE		L"useXmlColorScheme"//type: bool //default: false
#define		DATAKEY_SETTINGS_COLORSCHEME_DEFINDEX	L"colorscheme_defIndex"//type: int //default: 1
#define		DATAKEY_SETTINGS_COLORSCHEME_CUR_NAME	L"selectedSchemeName"//type: string
#define		DATAKEY_SETTINGS_SEARCHFORUPDATES		L"updatecheckbox"//type: bool //default: true
#define		DATAKEY_SETTINGS_SAVE_EXPLORER_COND		L"saveexplorercbx"//type: bool //default: true
#define		DATAKEY_SETTINGS_SAVE_TABWND_COND		L"savetabwndcondcbx"//type: bool //default: true
#define		DATAKEY_SETTINGS_SAVE_UNSAVED_CONTENT	L"saveunsavedcbx"//type: bool //default: false
#define		DATAKEY_SETTINGS_AUTOSAVE				L"autosavecbx"//type: bool //default: false
#define		DATAKEY_SETTINGS_OPEN_IN_NEW_TAB		L"newtabcbx"//type: bool //default: true
#define		DATAKEY_SETTINGS_SHOW_DESCWND			L"showpropwndcbx"//type: bool //default: true
#define		DATAKEY_SETTINGS_ALWAYS_EXPAND_PATH		L"alwaysexpandpathcbx"//type: bool //default: true
#define		DATAKEY_SETTINGS_SEARCH_FOR_ERRORS		L"searchforerrorcbx"//type: bool //defautl: false
#define		DATAKEY_SETTINGS_SINGLESESSIONRECOVER	L"singlesessionrecover"//type: unsigned int //default: 0
#define		DATAKEY_SETTINGS_OPENCREATEDFILE		L"opencreatedfilecbx"//type: bool //default: true
#define		DATAKEY_SETTINGS_USEFILETAGSASTOOLTIP	L"usefiletagsastooltipcbx"//type: bool //default: true
#define		DATAKEY_SETTINGS_CURRENTCOMPORT			L"currentCOMPort"//type: int //default: 1
#define		DATAKEY_SETTINGS_INSERTDEFAULTTEXT		L"insertDefaultText"//type: bool //default: true

// FILEKEY >> Autosyntax
#define		DATAKEY_SETTINGS_AUTOSYNTAX_ISON				L"as_isoncbx"//type: bool //default: false
#define		DATAKEY_SETTINGS_AUTOSYNTAX_AUTONUMSTARTLINE	L"as_autonumstartline"//type: int //default: 2
#define		DATAKEY_SETTINGS_AUTOSYNTAX_AUTONUMSTEPMAIN		L"as_numstepmain"//type: int //default: 1
#define		DATAKEY_SETTINGS_AUTOSYNTAX_NUMLENGTHMAIN		L"as_numlenghtmain"//type: int //default: 1
#define		DATAKEY_SETTINGS_AUTOSYNTAX_MINLINENUMBER		L"as_minlinenumber"//type: int //default: 1
#define		DATAKEY_SETTINGS_AUTOSYNTAX_MAXLINENUMBER		L"as_maxlinenumber"//type: int //default: 9999
#define		DATAKEY_SETTINGS_AUTOSYNTAX_ERASELINENUMBER		L"as_eraselinenumber"//type: bool //default: false
#define		DATAKEY_SETTINGS_AUTOSYNTAX_NOSPACEINLINENUMBER	L"as_nospaceinlinenumber"//type: bool //default: false
#define		DATAKEY_SETTINGS_AUTOSYNTAX_USEDIFFNUMLENINSUB	L"as_usedifferentnumlengthinsub"//type: bool //default: false
#define		DATAKEY_SETTINGS_AUTOSYNTAX_NUMLENGTHSUB		L"as_numlengthsub"//type: int //default: 1
#define		DATAKEY_SETTINGS_AUTOSYNTAX_NUMSTEPSUB			L"as_numstepsub"//type: int //default: 1
#define		DATAKEY_SETTINGS_AUTOSYNTAX_NEWEVENLINEONTRIGG	L"as_newevenlinenumberontrigger"//type: bool //default: false
#define		DATAKEY_SETTINGS_AUTOSYNTAX_SUBPROGTRIGGEREDIT	L"as_subprogtriggeredit"//type: string //default: SUBPROG_TRIGGER_DEFAULTSTRING
#define		DATAKEY_SETTINGS_AUTOSYNTAX_EVENLINETRIGGEREDIT	L"as_evenlinenumbertriggeredit"//type: string //default: NEWLINE_TRIGGER_DEFAULTSTRING
#define		DATAKEY_SETTINGS_AUTOSYNTAX_USEENDPROGDETECTION	L"as_useendprogdetection"//type: bool //default: true
#define		DATAKEY_SETTINGS_AUTOSYNTAX_ENDPROGTRIGGEREDIT	L"as_endprogtriggeredit"//type: string //default: ENDPROG_TRIGGER_DEFAULTSTRING
#define		DATAKEY_SETTINGS_AUTOSYNTAX_AUTOCOMPBRACKETS	L"as_autocompletebrackets"//type: bool //default: true
#define		DATAKEY_SETTINGS_AUTOSYNTAX_INSERTSPACESINBRAC	L"as_insertspacesinautocompletebrackets"//type: bool //default: false
#define		DATAKEY_SETTINGS_AUTOSYNTAX_USEMULTILINEANNOTNS	L"as_usemultilineannotations"//type: bool //default: false

// FILEKEY >> User-strings
#define		DATAKEY_USERSTRINGS_DEFAULTTABINSERTTEXT		L"us_defaulttabinserttext"//type: string

// FILEKEY >> Extended Settings
#define		DATAKEY_EXSETTINGS_EXCHANGEWND_ANCHORWND			L"exs_anchorexchangewindow"//type: bool //default: true
#define		DATAKEY_EXSETTINGS_EXCHANGEWND_MONITORTRANSMISSION	L"exs_monitortransmission"//type: bool //default: true
#define		DATAKEY_EXSETTINGS_EXCHANGEWND_VERBOSETRANSMISSION	L"exs_verbosetransmission"//type: bool //default: false
#define		DATAKEY_EXSETTINGS_EXCHANGEWND_AUTOCLOSE			L"exs_autoclosewnd"//type: bool //default: true
#define		DATAKEY_EXSETTINGS_EXCHANGEWND_LINEENDFORMAT		L"exs_seriallineendformatindex"//type: int //default: 0
#define		DATAKEY_EXSETTINGS_EXPORT_LINEENDFORMAT				L"exs_exportlineendformatindex"//type: int //default: 0
#define		DATAKEY_EXSETTINGS_EXPORT_REMOVEAPOSTROPHECOMMENT	L"exs_removeapostrophecomment"//type: bool //default: true
#define		DATAKEY_EXSETTINGS_EXPORT_REMOVEBRACKETCOMMENT		L"exs_removebracketcomment"//type: bool //default: false
#define		DATAKEY_EXSETTINGS_EXPORT_REMOVESPACES				L"exs_removespaces"//type: bool //default: false

// FILEKEY >> Internal Settings
#define		DATAKEY_INTSET_BLOCKDRIVELOADING					L"ins_blockdriveloading"//type: bool //default: true
#define		DATAKEY_INTSET_EDITOR_FOCUSRECTOFFSET_TOP			L"ins_top_focusrectoffset"//type: int //default: 1
#define		DATAKEY_INTSET_EDITOR_FOCUSRECTOFFSET_BOTTOM		L"ins_bottom_focusrectoffset"//type: int //default: 5


// ! NOTE: the filekey is not only the accessor for the settings, it is also the filename - so the canonical name must conform to the windows-filename conventions!
#define		FILEKEY_DEFAULT							L"cncsuitesettings"
#define		FILEKEY_AUTOSYNTAX_SETTINGS				L"autosyntax"
#define		FILEKEY_USER_STRINGS					L"userData"
#define		FILEKEY_EXTENDED_SETTINGS				L"extendedsettings"
#define		FILEKEY_INTERNAL_SETTINGS				L"internal"