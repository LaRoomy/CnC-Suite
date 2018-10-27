#include"LanguageDispatcher.h"
#include"StringTable de-DE.h"

TCHAR* LanguageDispatcher::getGermanString(int ID)
{
	switch (ID)
	{
		// UI - locations
	case UI_FILENAVIGATOR:
		return _UI_FILENAVIGATOR;
	case UI_APPPROPERTIES:
		return _UI_APPPROPERTIES;
	case UI_TABCONTROL:
		return _UI_TABCONTROL;

		// UI - strings
	case UI_IMPORT_FILE_CONTENT:
		return _UI_IMPORT_FILE_CONTENT;
	case UI_IMPORT_ACTION:
		return _UI_IMPORT_ACTION;
	case UI_DATAEXCHANGE_FINALIZE:
		return _UI_DATAEXCHANGE_FINALIZE;
	case UI_CANCEL:
		return _UI_CANCEL;
	case UI_DATAEXCHANGE_COMREADY:
		return _UI_DATAEXCHANGE_COMREADY;
	case UI_EDITCONTROL_SELECTIONRANGE:
		return _UI_EDITCONTROL_SELECTIONRANGE;
	case UI_EDITCONTROL_LINE:
		return _UI_EDITCONTROL_LINE;
	case UI_EDITCONTROL_COLUMN:
		return _UI_EDITCONTROL_COLUMN;

	case UI_TTP_SMPMNGR_ADDSAMPLE:
		return _UI_TTP_SMPMNGR_ADDSAMPLE;
	case UI_TTP_SMPMNGR_ADDFILTER:
		return _UI_TTP_SMPMNGR_ADDFILTER;
	case UI_TTP_SMPMNGR_CHANGESAMPLE:
		return _UI_TTP_SMPMNGR_CHANGESAMPLE;
	case UI_TTP_SMPMNGR_REMOVESAMPLE:
		return _UI_TTP_SMPMNGR_REMOVESAMPLE;
	case UI_TTP_SMPMNGR_COPYTOCLIPBOARD:
		return _UI_TTP_SMPMNGR_COPYTOCLIPBOARD;
	case UI_SMP_NEWFILTER_PLACEHOLDER:
		return _UI_SMP_NEWFILTER_PLACEHOLDER;
	case UI_SMP_TEMPLATEFOLDER:
		return _UI_SMP_TEMPLATEFOLDER;
	case UI_SMP_SAMPLEFOLDER:
		return _UI_SMP_SAMPLEFOLDER;
	case UI_SMP_REJECT:
		return _UI_SMP_REJECT;
	case UI_SMP_SAVESAMPLE:
		return _UI_SMP_SAVESAMPLE;
	case UI_SMP_CHANGEELEMENT:
		return _UI_SMP_CHANGEELEMENT;
	case UI_SMP_INSERTSAMPLE:
		return _UI_SMP_INSERTSAMPLE;
	case UI_SMP_CONFIRM_REMOVEFILTER:
		return _UI_SMP_CONFIRM_REMOVEFILTER;
	case UI_SMP_CONFIRM_DELETESAMPLE:
		return _UI_SMP_CONFIRM_DELETESAMPLE;
	case UI_SMP_CAPTIONCONFIRMATION:
		return _UI_SMP_CAPTIONCONFIRMATION;
	case UI_SMP_MAXLEVELREACHED:
		return _UI_SMP_MAXLEVELREACHED;
	case UI_SAMPLEMANAGER:
		return _UI_SAMPLEMANAGER;
	case UI_SMP_OVERRIDESAMPLE:
		return _UI_SMP_OVERRIDESAMPLE;

	case UI_GNRL_NAME:
		return _UI_GNRL_NAME;
	case UI_GNRL_DESCRIPTION:
		return _UI_GNRL_DESCRIPTION;
	case UI_GNRL_CONTENT:
		return _UI_GNRL_CONTENT;
	case UI_GNRL_NAMEINVALID:
		return _UI_GNRL_NAMEINVALID;
	case UI_GNRL_COPYTOCLIPBOARD:
		return _UI_GNRL_COPYTOCLIPBOARD;

	case UI_GNRL_ADDTAB:
		return _UI_GNRL_ADDTAB;
	case UI_GNRL_OPEN:
		return _UI_GNRL_OPEN;
	case UI_GNRL_SAVE:
		return _UI_GNRL_SAVE;
	case UI_GNRL_SAVEAS:
		return _UI_GNRL_SAVEAS;
	case UI_GNRL_SAVEALL:
		return _UI_GNRL_SAVEALL;
	case UI_GNRL_CUT:
		return _UI_GNRL_CUT;
	case UI_GNRL_COPY:
		return _UI_GNRL_COPY;
	case UI_GNRL_PASTE:
		return _UI_GNRL_PASTE;
	case UI_GNRL_DELETE_SELECTEDTEXT:
		return _UI_GNRL_DELETE_SELECTEDTEXT;
	case UI_GNRL_SELECTALL:
		return _UI_GNRL_SELECTALL;
	case UI_GNRL_SENDPROGRAM:
		return _UI_GNRL_SENDPROGRAM;
	case UI_GNRL_RECEIVEPROGRAM:
		return _UI_GNRL_RECEIVEPROGRAM;
	case UI_GNRL_IMPORT:
		return _UI_GNRL_IMPORT;
	case UI_GNRL_TEMPLATEMANAGER:
		return _UI_GNRL_TEMPLATEMANAGER;
	case UI_GNRL_APPLICATIONPROPERTIES:
		return _UI_GNRL_APPLICATIONPROPERTIES;
	case UI_GNRL_NEWCNCFILE:
		return _UI_GNRL_NEWCNCFILE;
	case UI_GNRL_NEWFOLDER:
		return _UI_GNRL_NEWFOLDER;
	case UI_GNRL_COPYFOLDER:
		return _UI_GNRL_COPYFOLDER;
	case UI_GNRL_DELETE:
		return _UI_GNRL_DELETE;
	case UI_GNRL_RENAME:
		return _UI_GNRL_RENAME;
	case UI_GNRL_CONVERTTOCNC3:
		return _UI_GNRL_CONVERTTOCNC3;
	case UI_GNRL_IMPORTCONTENT:
		return _UI_GNRL_IMPORTCONTENT;
	case UI_GNRL_COPYFILE:
		return _UI_GNRL_COPYFILE;
	case UI_GNRL_CHANGEROOTDIR:
		return _UI_GNRL_CHANGEROOTDIR;
	case UI_GNRL_PASTEFILE:
		return _UI_GNRL_PASTEFILE;
	case UI_GNRL_OPENFILE:
		return _UI_GNRL_OPENFILE;
	case UI_GNRL_KEEPOLDFILE:
		return _UI_GNRL_KEEPOLDFILE;
	case UI_GNRL_ADVANCED:
		return _UI_GNRL_ADVANCED;
	case UI_GNRL_PLAIN_SAVE:
		return _UI_GNRL_PLAIN_SAVE;
	case UI_GNRL_NEWFOLDER_PLACEHOLDER:
		return _UI_GNRL_NEWFOLDER_PLACEHOLDER;
	case UI_GNRL_NEWPROGRAM_PLACEHOLDER:
		return _UI_GNRL_NEWPROGRAM_PLACEHOLDER;
	case UI_GNRL_UNDO:
		return _UI_GNRL_UNDO;
	case UI_GNRL_REDO:
		return _UI_GNRL_REDO;
	case UI_GNRL_OPENINNEWTAB:
		return _UI_GNRL_OPENINNEWTAB;
	case UI_GNRL_PLAIN_OPEN:
		return _UI_GNRL_PLAIN_OPEN;
	case UI_GNRL_OPENHELP:
		return _UI_GNRL_OPENHELP;
	case UI_GNRL_VISITWEBSITE:
		return _UI_GNRL_VISITWEBSITE;
	case UI_GNRL_CLOSEAPP:
		return _UI_GNRL_CLOSEAPP;
	case UI_GNRL_NAVIGATEBACK:
		return _UI_GNRL_NAVIGATEBACK;
	case UI_GNRL_SHOWINWINEXPLORER:
		return _UI_GNRL_SHOWINWINEXPLORER;
	case UI_GNRL_CANCEL:
		return _UI_GNRL_CANCEL;
	case UI_GNRL_FINALIZE:
		return _UI_GNRL_FINALIZE;
	case UI_GNRL_INPUT:
		return _UI_GNRL_INPUT;
	case UI_GNRL_OUTPUT:
		return _UI_GNRL_OUTPUT;
	case UI_GNRL_CLOSE:
		return _UI_GNRL_CLOSE;


	case UI_NAV_SEARCHINROOT:
		return _UI_NAV_SEARCHINROOT;
	case UI_NAV_SELECTROOTFOLDER:
		return _UI_NAV_SELECTROOTFOLDER;
	case UI_NAV_EXPLORERBUTTON:
		return _UI_NAV_EXPLORERBUTTON;
	case UI_NAV_TEMPLATEBUTTON:
		return _UI_NAV_TEMPLATEBUTTON;


	case UI_EDITTOOLBAR_UPPERCASE:
		return _UI_EDITTOOLBAR_UPPERCASE;
	case UI_EDITTOOLBAR_LINENUMBERS:
		return _UI_EDITTOOLBAR_LINENUMBERS;
	case UI_EDITTOOLBAR_AUTOCOMPLETE:
		return _UI_EDITTOOLBAR_AUTOCOMPLETE;
	case UI_EDITTOOLBAR_TEXTCOLOR:
		return _UI_EDITTOOLBAR_TEXTCOLOR;
	case UI_EDITTOOLBAR_MARKCOLOR:
		return _UI_EDITTOOLBAR_MARKCOLOR;
	case UI_EDITTOOLBAR_CONVERTALL:
		return _UI_EDITTOOLBAR_CONVERTALL;
	case UI_EDITTOOLBAR_SEQUENCELINENBR:
		return _UI_EDITTOOLBAR_SEQUENCELINENBR;
	case UI_EDITTOOLBAR_CHECKFORERROR:
		return _UI_EDITTOOLBAR_CHECKFORERROR;
	case UI_EDITTOOLBAR_AUTOSYNTAX:
		return _UI_EDITTOOLBAR_AUTOSYNTAX;
	case UI_EDITTOOLBAR_TEXTPROPERTIES:
		return _UI_EDITTOOLBAR_TEXTPROPERTIES;
	case UI_EDITTOOLBAR_SHOWDOCPROPERTY:
		return _UI_EDITTOOLBAR_SHOWDOCPROPERTY;
	case UI_EDITTOOLBAR_HIDEDOCPROPERTY:
		return _UI_EDITTOOLBAR_HIDEDOCPROPERTY;

	case UI_TABCTRL_DOCUMENTPROPERTIES:
		return _UI_TABCTRL_DOCUMENTPROPERTIES;
	case UI_TABCTRL_NEWTAB:
		return _UI_TABCTRL_NEWTAB;
	case UI_TABCTRL_DESCPLACEHOLDER_1:
		return _UI_TABCTRL_DESCPLACEHOLDER_1;
	case UI_TABCTRL_DESCPLACEHOLDER_2:
		return _UI_TABCTRL_DESCPLACEHOLDER_2;
	case UI_TABCTRL_DESCPLACEHOLDER_3:
		return _UI_TABCTRL_DESCPLACEHOLDER_3;
	case UI_TABCTRL_TTBUTTONCOPYDESC:
		return _UI_TABCTRL_TTBUTTONCOPYDESC;
	case UI_TABCTRL_TTBUTTONINSERTDESC:
		return _UI_TABCTRL_TTBUTTONINSERTDESC;
	case UI_TABCTRL_TTBUTTONSETDIAMETER:
		return _UI_TABCTRL_TTBUTTONSETDIAMETER;
	case UI_TABCTRL_TTBUTTONDELETEDESC:
		return _UI_TABCTRL_TTBUTTONDELETEDESC;
	case UI_TABCTRL_FONTHEIGHT:
		return _UI_TABCTRL_FONTHEIGHT;
	case UI_TABCTRL_LINEOFFSET:
		return _UI_TABCTRL_LINEOFFSET;
	case UI_TABCTRL_FONTBOLD:
		return _UI_TABCTRL_FONTBOLD;
	case UI_TABCTRL_EXPANDPATHTOFILE:
		return _UI_TABCTRL_EXPANDPATHTOFILE;
	case UI_TABCTRL_ERASETABCONTENT:
		return _UI_TABCTRL_ERASETABCONTENT;
	case UI_TABCTRL_COPYCONTENTTONEWTAB:
		return _UI_TABCTRL_COPYCONTENTTONEWTAB;
	case UI_TABCTRL_SETCONTENTASTEMPLATE:
		return _UI_TABCTRL_SETCONTENTASTEMPLATE;

	case UI_PROPWND_SETTINGSBUTTON:
		return _UI_PROPWND_SETTINGSBUTTON;
	case UI_PROPWND_TABCTRLBUTTON:
		return _UI_PROPWND_TABCTRLBUTTON;
	case UI_PROPWND_FILENAVIGATOR:
		return _UI_PROPWND_FILENAVIGATOR;
	case UI_PROPWND_EDITOR:
		return _UI_PROPWND_EDITOR;
	case UI_PROPWND_EXCHANGE:
		return _UI_PROPWND_EXCHANGE;
	case UI_PROPWND_INFO:
		return _UI_PROPWND_INFO;

	case UI_PROPWND_LANGUAGE:
		return _UI_PROPWND_LANGUAGE;
	case UI_PROPWND_SYSTEMLANGUAGE:
		return _UI_PROPWND_SYSTEMLANGUAGE;
	case UI_PROPWND_GERMAN:
		return _UI_PROPWND_GERMAN;
	case UI_PROPWND_ENGLISH:
		return _UI_PROPWND_ENGLISH;
	case UI_PROPWND_DESIGN:
		return _UI_PROPWND_DESIGN;
	case UI_PROPWND_DSG_BLACK:
		return _UI_PROPWND_DSG_BLACK;
	case UI_PROPWND_DSG_GREY:
		return _UI_PROPWND_DSG_GREY;
	case UI_PROPWND_DSG_LIGHT:
		return _UI_PROPWND_DSG_LIGHT;
	case UI_PROPWND_DSG_GREEN:
		return _UI_PROPWND_DSG_GREEN;
	case UI_PROPWND_AUTOUPDATESEARCH:
		return _UI_PROPWND_AUTOUPDATESEARCH;
	case UI_PROPWND_ACTIONONLAUNCH:
		return _UI_PROPWND_ACTIONONLAUNCH;
	case UI_PROPWND_SAVEEXPLORERCOND:
		return _UI_PROPWND_SAVEEXPLORERCOND;
	case UI_PROPWND_SAVETABWNDCOND:
		return _UI_PROPWND_SAVETABWNDCOND;
	case UI_PROPWND_SAVEUNSAVEDCONTENT:
		return _UI_PROPWND_SAVEUNSAVEDCONTENT;
	case UI_PROPWND_UPDATING:
		return _UI_PROPWND_UPDATING;
	case UI_PROPWND_SEARCHFORUPDATESNOW:
		return _UI_PROPWND_SEARCHFORUPDATESNOW;
	case UI_PROPWND_APPUPTODATE:
		return _UI_PROPWND_APPUPTODATE;
	case UI_PROPWND_UPDATEAVAILABLE:
		return _UI_PROPWND_UPDATEAVAILABLE;
	case UI_PROPWND_ERRORNOINTERNETCON:
		return _UI_PROPWND_ERRORNOINTERNETCON;
	case UI_PROPWND_AUTOSAVE:
		return _UI_PROPWND_AUTOSAVE;
	case UI_PROPWND_OPENNEWFILEINNEWTAB:
		return _UI_PROPWND_OPENNEWFILEINNEWTAB;
	case UI_PROPWND_SHOWDOCPROPERTIES:
		return _UI_PROPWND_SHOWDOCPROPERTIES;
	case UI_PROPWND_DOCPROPTAGDESC:
		return _UI_PROPWND_DOCPROPTAGDESC;
	case UI_PROPWND_PROPDESCRIPTOR:
		return _UI_PROPWND_PROPDESCRIPTOR;
	case UI_PROPWND_ALWAYSEXPANDPATH:
		return _UI_PROPWND_ALWAYSEXPANDPATH;
	case UI_PROPWND_COLORSCHEME:
		return _UI_PROPWND_COLORSCHEME;
	case UI_PROPWND_NORMAL:
		return _UI_PROPWND_NORMAL;
	case UI_PROPWND_SEARCHFORERRORS:
		return _UI_PROPWND_SEARCHFORERRORS;
	case UI_PROPWND_AUTOCOMPLETESTRINGS:
		return _UI_PROPWND_AUTOCOMPLETESTRINGS;
	case UI_PROPWND_EDITACSTRINGS:
		return _UI_PROPWND_EDITACSTRINGS;
	case UI_PROPWND_ERRORCONTROL:
		return _UI_PROPWND_ERRORCONTROL;
	case UI_PROPWND_COMPORTNUMBER:
		return _UI_PROPWND_COMPORTNUMBER;
	case UI_PROPWND_BAUDRATE:
		return _UI_PROPWND_BAUDRATE;
	case UI_PROPWND_PARITY:
		return _UI_PROPWND_PARITY;
	case UI_PROPWND_DATABITS:
		return _UI_PROPWND_DATABITS;
	case UI_PROPWND_STOPBITS:
		return _UI_PROPWND_STOPBITS;
	case UI_PROPWND_NOPARITY:
		return _UI_PROPWND_NOPARITY;
	case UI_PROPWND_ODDPARITY:
		return _UI_PROPWND_ODDPARITY;
	case UI_PROPWND_EVENPARITY:
		return _UI_PROPWND_EVENPARITY;
	case UI_PROPWND_MARKPARITY:
		return _UI_PROPWND_MARKPARITY;
	case UI_PROPWND_SPACEPARITY:
		return _UI_PROPWND_SPACEPARITY;
	case UI_PROPWND_FLOWCTRL:
		return _UI_PROPWND_FLOWCTRL;
	case UI_PROPWND_FLOWCTRL_NONE:
		return _UI_PROPWND_FLOWCTRL_NONE;
	case UI_PROPWND_FLOWCTRL_XONXOFF:
		return _UI_PROPWND_FLOWCTRL_XONXOFF;
	case UI_PROPWND_FLOWCTRL_HARDWARE:
		return _UI_PROPWND_FLOWCTRL_HARDWARE;
	case UI_PROPWND_PERFORMPARITYCHECK:
		return _UI_PROPWND_PERFORMPARITYCHECK;
	case UI_PROPWND_ABORTONERROR:
		return _UI_PROPWND_ABORTONERROR;
	case UI_PROPWND_REPLACEPARITYERROR:
		return _UI_PROPWND_REPLACEPARITYERROR;
	case UI_PROPWND_DTRFLOWCONTROL:
		return _UI_PROPWND_DTRFLOWCONTROL;
	case UI_PROPWND_RTSFLOWCONTROL:
		return _UI_PROPWND_RTSFLOWCONTROL;
	case UI_PROPWND_CHARACTERS:
		return _UI_PROPWND_CHARACTERS;
	case UI_PROPWND_INTERFACECONFIG:
		return _UI_PROPWND_INTERFACECONFIG;
	case UI_PROPWND_MANAGECOLORSCHEMES:
		return _UI_PROPWND_MANAGECOLORSCHEMES;
	case UI_PROPWND_CHOOSECOLOR:
		return _UI_PROPWND_CHOOSECOLOR;
	case UI_PROPWND_NEWSCHEME:
		return _UI_PROPWND_NEWSCHEME;
	case UI_PROPWND_ANNOTATION:
		return _UI_PROPWND_ANNOTATION;
	case UI_PROPWND_DEFAULTTEXTCOLOR:
		return _UI_PROPWND_DEFAULTTEXTCOLOR;
	case UI_PROPWND_LINENUMBERCOLOR:
		return _UI_PROPWND_LINENUMBERCOLOR;
	case UI_PROPWND_BACKGROUNDCOLOR:
		return _UI_PROPWND_BACKGROUNDCOLOR;
	case UI_PROPWND_RESETTEXTCOLOR:
		return _UI_PROPWND_RESETTEXTCOLOR;
	case UI_PROPWND_RESETBACKGROUND:
		return _UI_PROPWND_RESETBACKGROUND;
	case UI_PROPWND_ADDAUTOCOMPL:
		return _UI_PROPWND_ADDAUTOCOMPL;
	case UI_PROPWND_DELETEAUTOCOMPL:
		return _UI_PROPWND_DELETEAUTOCOMPL;
	case UI_PROPWND_WHENINSERT:
		return _UI_PROPWND_WHENINSERT;
	case UI_PROPWND_APPEND:
		return _UI_PROPWND_APPEND;
	case UI_PROPWND_PREVIEW:
		return _UI_PROPWND_PREVIEW;
	case UI_PROPWND_OPENCREATEDFILE:
		return _UI_PROPWND_OPENCREATEDFILE;
	case UI_PROPWND_USEFILETAGSASTOOLTIP:
		return _UI_PROPWND_USEFILETAGSASTOOLTIP;
	case UI_PROPWND_SEARCHSETTINGS:
		return _UI_PROPWND_SEARCHSETTINGS;
	case UI_PROPWND_SRCH_OPENMULTIFILE:
		return _UI_PROPWND_SRCH_OPENMULTIFILE;
	case UI_PROPWND_SRCH_EXPANDPATH:
		return _UI_PROPWND_SRCH_EXPANDPATH;
	case UI_PROPWND_SRCH_CLOSEONOPEN:
		return _UI_PROPWND_SRCH_CLOSEONOPEN;
	case UI_PROPWND_SEARCHFORFILENAME:
		return _UI_PROPWND_SEARCHFORFILENAME;
	case UI_PROPWND_SEARCHFORFOLDERNAME:
		return _UI_PROPWND_SEARCHFORFOLDERNAME;
	case UI_PROPWND_SEARCH_IN:
		return _UI_PROPWND_SEARCH_IN;
	case UI_PROPWND_AUTOSYNTAX:
		return _UI_PROPWND_AUTOSYNTAX;
	case UI_PROPWND_SETAUTOSYNTAX:
		return _UI_PROPWND_SETAUTOSYNTAX;
	case UI_PROPWND_CONVERTSYNTAXONOPEN:
		return _UI_PROPWND_CONVERTSYNTAXONOPEN;
	case UI_PROPWND_LINENUMBERS:
		return _UI_PROPWND_LINENUMBERS;
	case UI_PROPWND_AUTONUMSTARTLINE:
		return _UI_PROPWND_AUTONUMSTARTLINE;
	case UI_PROPWND_NUMSTEPMAIN:
		return _UI_PROPWND_NUMSTEPMAIN;
	case UI_PROPWND_NUMLENGTHMAIN:
		return _UI_PROPWND_NUMLENGTHMAIN;
	case UI_PROPWND_MINIMUMLINENUMBER:
		return _UI_PROPWND_MINIMUMLINENUMBER;
	case UI_PROPWND_MAXIMUMLINENUMBER:
		return _UI_PROPWND_MAXIMUMLINENUMBER;

	case UI_PROPWND_ERASELINENRONBACKSPACE:
		return _UI_PROPWND_ERASELINENRONBACKSPACE;
	case UI_PROPWND_NOSPACEINLINENUMBER:
		return _UI_PROPWND_NOSPACEINLINENUMBER;
	case UI_PROPWND_USEDIFFERENTLINENUMBERINSUB:
		return _UI_PROPWND_USEDIFFERENTLINENUMBERINSUB;
	case UI_PROPWND_NUMLENGTHSUB:
		return _UI_PROPWND_NUMLENGTHSUB;
	case UI_PROPWND_NUMSTEPSUB:
		return _UI_PROPWND_NUMSTEPSUB;
	case UI_PROPWND_NEWEVENLINENUMBERRONTRIGGER:
		return _UI_PROPWND_NEWEVENLINENUMBERRONTRIGGER;
	case UI_PROPWND_ENDPROGDETECTION:
		return _UI_PROPWND_ENDPROGDETECTION;
	case UI_PROPWND_USEENDPROGDETECTION:
		return _UI_PROPWND_USEENDPROGDETECTION;
	case UI_PROPWND_ANNOTATIONANDVARIABLESTATEMENTS:
		return _UI_PROPWND_ANNOTATIONANDVARIABLESTATEMENTS;
	case UI_PROPWND_AUTOCOMPLETEBRACKETS:
		return _UI_PROPWND_AUTOCOMPLETEBRACKETS;
	case UI_PROPWND_INSERTSPACESINAUTOCMPLTBARCKET:
		return _UI_PROPWND_INSERTSPACESINAUTOCMPLTBARCKET;
	case UI_PROPWND_USEMULTILINEANNOTATIONS:
		return _UI_PROPWND_USEMULTILINEANNOTATIONS;
	case UI_PROPWND_SUBPROGDETECTION:
		return _UI_PROPWND_SUBPROGDETECTION;
	case UI_PROPWND_TRIGGER:
		return _UI_PROPWND_TRIGGER;
	case UI_PROPWND_PROGRAMRESTARTNECCESSARY:
		return _UI_PROPWND_PROGRAMRESTARTNECCESSARY;
	case UI_PROPWND_STANDARDTEMPLATE:
		return _UI_PROPWND_STANDARDTEMPLATE;
	case UI_PROPWND_INSERTSTANDARDTEMPLATE:
		return _UI_PROPWND_INSERTSTANDARDTEMPLATE;
	case UI_PROPWND_CREATEDEVICELISTING:
		return _UI_PROPWND_CREATEDEVICELISTING;
	case UI_PROPWND_SELECTDEVICE:
		return _UI_PROPWND_SELECTDEVICE;
	case UI_PROPWND_FOUNDDEVICES:
		return _UI_PROPWND_FOUNDDEVICES;
	case UI_PROPWND_NODEVICESELECTED:
		return _UI_PROPWND_NODEVICESELECTED;
	case UI_PROPWND_OUTPUTSETTINGS:
		return _UI_PROPWND_OUTPUTSETTINGS;
	case UI_PROPWND_OUTWINDOWSETTINGS:
		return _UI_PROPWND_OUTWINDOWSETTINGS;
	case UI_PROPWND_OUTFORMATSETTINGS:
		return _UI_PROPWND_OUTFORMATSETTINGS;
	case UI_PROPWND_ANCHOROUTPUTWINDOW:
		return _UI_PROPWND_ANCHOROUTPUTWINDOW;
	case UI_PROPWND_MONITORTRANSMISSON:
		return _UI_PROPWND_MONITORTRANSMISSON;
	case UI_PROPWND_VERBOSETRANSMISSION:
		return _UI_PROPWND_VERBOSETRANSMISSION;
	case UI_PROPWND_AUTOCLOSE:
		return _UI_PROPWND_AUTOCLOSE;
	case UI_PROPWND_RESETBUTTON:
		return _UI_PROPWND_RESETBUTTON;
	case UI_PROPWND_DISCREET:
		return _UI_PROPWND_DISCREET;
	case UI_PROPWND_DELETECURRENTSCHEME:
		return _UI_PROPWND_DELETECURRENTSCHEME;
	case UI_PROPWND_REMOVEAPOSTROPHECOMMENT:
		return _UI_PROPWND_REMOVEAPOSTROPHECOMMENT;
	case UI_PROPWND_REMOVEBRACKETCOMMENT:
		return _UI_PROPWND_REMOVEBRACKETCOMMENT;
	case UI_PROPWND_REMOVESPACES:
		return _UI_PROPWND_REMOVESPACES;



	case UI_EXPLORER_TOOLTIP_LOADROOTFOLDER:
		return _UI_EXPLORER_TOOLTIP_LOADROOTFOLDER;
	case UI_EXPLORER_TOOLTIP_NEWPROGRAMM:
		return _UI_EXPLORER_TOOLTIP_NEWPROGRAMM;
	case UI_EXPLORER_TOOLTIP_NEWFOLDER:
		return _UI_EXPLORER_TOOLTIP_NEWFOLDER;
	case UI_EXPLORER_TOOLTIP_COPYELEMENT:
		return _UI_EXPLORER_TOOLTIP_COPYELEMENT;
	case UI_EXPLORER_TOOLTIP_INSERTELEMENT:
		return _UI_EXPLORER_TOOLTIP_INSERTELEMENT;
	case UI_EXPLORER_TOOLTIP_DELETEELEMENT:
		return _UI_EXPLORER_TOOLTIP_DELETEELEMENT;
	case UI_EXPLORER_TOOLTIP_CONVERTTOCNC3:
		return _UI_EXPLORER_TOOLTIP_CONVERTTOCNC3;
	case UI_EXPLORER_TOOLTIP_IMPORTCONTENT:
		return _UI_EXPLORER_TOOLTIP_IMPORTCONTENT;
	case UI_EXPLORER_TOOLTIP_SEARCHFORPROGRAMM:
		return _UI_EXPLORER_TOOLTIP_SEARCHFORPROGRAMM;


	case UI_SSCREEN_LOADING:
		return _UI_SSCREEN_LOADING;
	case UI_SSCREEN_INITAPPDATA:
		return _UI_SSCREEN_INITAPPDATA;
	case UI_SSCREEN_VERIFYFILESYSTEM:
		return _UI_SSCREEN_VERIFYFILESYSTEM;
	case UI_SSCREEN_LOADEDITRESOURCES:
		return _UI_SSCREEN_LOADEDITRESOURCES;
	case UI_SSCREEN_INITMAINFRAME:
		return _UI_SSCREEN_INITMAINFRAME;

	case UI_STATUSBAR_READY_STATE:
		return _UI_STATUSBAR_READY_STATE;
	case UI_STATUSBAR_SENDINPROGRESS:
		return _UI_STATUSBAR_SENDINPROGRESS;
	case UI_STATUSBAR_RECEIVEINPROGRESS:
		return _UI_STATUSBAR_RECEIVEINPROGRESS;

	case UI_PROGRESS_COLLECTINGDATA:
		return _UI_PROGRESS_COLLECTINGDATA;
	case UI_PROGRESS_LOADINGDATA:
		return _UI_PROGRESS_LOADINGDATA;

	case UI_SEARCHCTRL_MAXSRCHRESULT:
		return _UI_SEARCHCTRL_MAXSRCHRESULT;

	case UI_RESETWND_CHOOSECOMPONENT:
		return _UI_RESETWND_CHOOSECOMPONENT;
	case UI_RESETWND_COMPARAMETER:
		return _UI_RESETWND_COMPARAMETER;
	case UI_RESETWND_COMMONSETTINGS:
		return _UI_RESETWND_COMMONSETTINGS;
	case UI_RESETWND_AUTOSYNTAXSETUP:
		return _UI_RESETWND_AUTOSYNTAXSETUP;
	case UI_RESETWND_USERTEXT:
		return _UI_RESETWND_USERTEXT;
	case UI_RESETWND_INTERNALSETUP:
		return _UI_RESETWND_INTERNALSETUP;
	case UI_RESETWND_AUTOCOMPLETESETUP:
		return _UI_RESETWND_AUTOCOMPLETESETUP;
	case UI_RESETWND_APPDATA:
		return _UI_RESETWND_APPDATA;
	case UI_RESETWND_SESSIONDATA:
		return _UI_RESETWND_SESSIONDATA;
	case UI_RESETWND_RESTARTMESSAGE:
		return _UI_RESETWND_RESTARTMESSAGE;

	case UI_PROPWND_SAVEHISTORY:
		return _UI_PROPWND_SAVEHISTORY;
	case UI_PROPWND_DELETEHISTORYNOW:
		return _UI_PROPWND_DELETEHISTORYNOW;

	case UI_GNRL_COPYRIGHT:
		return _UI_GNRL_COPYRIGHT;
	case UI_GNRL_RIGHTS:
		return _UI_GNRL_RIGHTS;
	case UI_GNRL_USERCANCELCONFIRM:
		return _UI_GNRL_USERCANCELCONFIRM;
	case UI_GNRL_NEWSUBFOLDER:
		return _UI_GNRL_NEWSUBFOLDER;
	case UI_GNRL_LINEENDFORMAT:
		return _UI_GNRL_LINEENDFORMAT;
	case UI_GNRL_HISTORY:
		return _UI_GNRL_HISTORY;


		// error messages
	case UI_XML_ERR_DOCUMENTFORMAT_INVALID:
		return _UI_XML_ERR_DOCUMENTFORMAT_INVALID;
	case UI_XML_ERR_UNEXPECTEDCLOSINGTAG:
		return _UI_XML_ERR_UNEXPECTEDCLOSINGTAG;
	case UI_XML_ERR_NOCLOSING_ELEMENT_P1:
		return _UI_XML_ERR_NOCLOSING_ELEMENT_P1;
	case UI_XML_ERR_NOCLOSING_ELEMENT_P2:
		return _UI_XML_ERR_NOCLOSING_ELEMENT_P2;

	case ERROR_MSG_FILETYPE_NOTSUPPORTED:
		return _ERROR_MSG_FILETYPE_NOTSUPPORTED;
	case ERROR_MSG_ROOTFOLDERINVALIDATED:
		return _ERROR_MSG_ROOTFOLDERINVALIDATED;
	case ERROR_MSG_ROOTFOLDERLOADINGFAILED:
		return _ERROR_MSG_ROOTFOLDERLOADINGFAILED;
	case ERROR_MSG_DRIVELOADINGNOTALLOWED:
		return _ERROR_MSG_DRIVELOADINGNOTALLOWED;


		// warning messages
	case WARNING_MSG_UPDATESEARCH_FAILED:
		return _WARNING_MSG_UPDATESEARCH_FAILED;
	case WARNING_MSG_SAVEALL_NOPATHFOUND:
		return _WARNING_MSG_SAVEALL_NOPATHFOUND;

		// info messages
	case INFO_MSG_FILEWASIMPORTED:
		return _INFO_MSG_FILEWASIMPORTED;
	case INFO_MSG_FILEWASCONVERTED:
		return _INFO_MSG_FILEWASCONVERTED;
	case INFO_MSG_UPDATEAVAILABLE:
		return _INFO_MSG_UPDATEAVAILABLE;
	case INFO_MSG_LASTTABNOCLOSE:
		return _INFO_MSG_LASTTABNOCLOSE;
	case INFO_MSG_MAXIMUMTABCOUNTREACHED:
		return _INFO_MSG_MAXIMUMTABCOUNTREACHED;
	case INFO_MSG_FILEISALREADYOPEN:
		return _INFO_MSG_FILEISALREADYOPEN;
	case INFO_MSG_CHANGESNEEDRESTART:
		return _INFO_MSG_CHANGESNEEDRESTART;
	case INFO_MSG_NOTARGETSELECTED:
		return _INFO_MSG_NOTARGETSELECTED;


		// dialog messages
	case DLG_MSG_FILENOTSAVEDSAVENOW:
		return _DLG_MSG_FILENOTSAVEDSAVENOW;
	case DLG_MSG_UNSAVEDCONTENTEXISTS:
		return _DLG_MSG_UNSAVEDCONTENTEXISTS;


	default:
		return _RESOURCE_INVALID;
	}
}