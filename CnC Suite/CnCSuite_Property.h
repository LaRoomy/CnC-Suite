#pragma once
#ifndef _CNCSUITE_PROPERTY_
#define	_CNCSUITE_PROPERTY_

#define		COMMON_SETTINGS_PAGE	0
#define		APPLICATION_ISFIRSTUSE	3
#define		CS_PROPERTYWINDOW_CLASS L"PROPWINDOWCLASS"

#include"external.h"

class CnCSuite_Property {
public:
	virtual void Release() = 0;
	virtual BOOL VerifyApplicationFileSystem() = 0;
	virtual BOOL AcquireData() = 0;
	virtual BOOL CheckForUpdates() = 0;
	virtual HRESULT StringToSplashScreen(LPTSTR string_) = 0;
	//WATCH OUT: this is a core-reference, NOT a copy!!!
	virtual LPVOID GetAutocompleteStrings(int* count) = 0;
	virtual LanguageID GetUserLanguage() = 0;
	virtual HRESULT CreatePropertyWindow(HWND Main, HINSTANCE hInst) = 0;
	virtual BOOL isPropWindowOpen(HWND *hwndOut) = 0;
	virtual void onDpiChanged() = 0;
};

CnCSuite_Property* CreateCnCSuiteProperty();

#endif // _CNCSUITE_PROPERTY_


//////// Settings:
// Programm:
// design (color)
// language
// search for updates automatically
// action on program-start
// -> save treeview expand image
// -> save tabs and content
// -> save unsaved content??

// Tabcontrol:
// autosave
// open new file in new tab
// show document-window
// descriptions for cnc3 - tags

// FileNavigator
// (always) expand path to opened item
// save navigation image

// data-exchange
// setup (included in dataexchange.h)

// editor
// make all the editor-setting accessible here too?????
// search for errors automatically

// userdefined colors xml scheme??

// autocomplete strings???



