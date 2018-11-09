#pragma once
#ifndef _CNCSUITE_TABCONTROL_
#define _CNCSUITE_TABCONTROL_

#define		TCI_OPENREQUEST			10
#define		TCI_NOINITACTION		11
#define		TCI_ISFIRSTUSE			12

#define		TCP_EMPTYAREA			12
#define		TCP_LASTSESSION			13
#define		TCP_NEWTAB				14

#define		TOS_SAVE				20
#define		TOS_SAVEALL				21

#define		FILE_OVERWRITTEN		25

#include"external.h"
#include"EditControl.h"
#include"Async.h"
#include"FileSystem.h"
#include"CnC3FileManager.h"

typedef struct _TCSTARTUPINFO {
	int mode;
	BOOL useXMLColorScheme;
	BOOL restoreSession;
	TCHAR* PathToFile;
	TCHAR* WorkingDirectory;
}TCSTARTUPINFO, *LPTCSTARTUPINFO;

typedef struct _TCPROPERTY {
	int StartupAction;
	BOOL OpenNewPathInNewTab;
	BOOL AutoSave;
	EDITSTYLECOLORS esc;
	EDITCONTROLPROPERTIES ecp;
}TCPROPERTY, *LPTCPROPERTY;

class CnCSuite_Tabcontrol {
public:
	virtual HRESULT Init(HWND Frame, LPTCSTARTUPINFO tc_info) = 0;
	virtual BOOL GetProperty(LPTCPROPERTY TabControlProp) = 0;
	virtual void GetCurrentCnC3File(CnC3File& file) = 0;
	virtual BOOL GetCurrentTabContent(TCHAR** content) = 0;
	virtual BOOL GetCurrentSelectedText(LPSELECTIONINFO selInfo) = 0;
	virtual LPCTSTR GetCurrentFilename() = 0;
	virtual void UserRequest_AddNewTab() = 0;
	virtual void UserRequest_Open(LPTSTR path, BOOL ForceOpenInNewTab, BOOL setFocus) = 0;
	virtual void UserRequest_Open(const CnC3File& file, bool forceOpenInNewTab, bool setFocus) = 0;
	virtual BOOL UserRequest_SaveAs(TCHAR** path) = 0;
	virtual BOOL UserRequest_Save(DWORD mode) = 0;
	virtual void UserRequest_Import(LPTSTR content) = 0;
	virtual BOOL UserRequest_CloseApp() = 0;
	virtual void UserRequest_InsertText(LPTSTR text) = 0;
	virtual void UserRequest_SetNewColorScheme(BOOL setUserdefinedColor) = 0;
	virtual HWND GetFrameHandle() = 0;
	virtual	HWND GetCurrentVisibleEditboxHandle() = 0;
	virtual void FileSystemChanged(LPFILESYSTEMOBJECT fso) = 0;
	virtual void GetCurrentTabDataAsStringCollection(itemCollection<iString>& data) = 0;
	virtual void ICommand(WPARAM wParam, LPARAM lParam) = 0;
	virtual void UserAction_Copy() = 0;
	virtual void UserAction_Cut() = 0;
	virtual void UserAction_Paste() = 0;
	virtual void UserAction_SelectAll() = 0;
	virtual void UserAction_Clear() = 0;
	virtual void UserAction_Keyboard(WPARAM vKey) = 0;
	virtual BOOL GetDescriptionNames(LPDESCRIPTIONINFO dInfo) = 0;
	virtual void SetDescriptionNames(LPDESCRIPTIONINFO dInfo) = 0;
	virtual void SetFocusOnCurrentVisibleEditbox() = 0;
	virtual void GetEditControlProperties(UINT_PTR editstylecolors, LONG_PTR editcontrolproperties) = 0;
	virtual void UpdateAutocompleteData() = 0;
	virtual void UpdateAutosyntaxSettings() = 0;
	virtual void SaveTabCondition() = 0;
	virtual void RestoreTabCondition() = 0;
	virtual void onDpiChanged() = 0;
	virtual void Release() = 0;
};

CnCSuite_Tabcontrol* CreateTabcontrol(HINSTANCE hInst, HWND MainWindow);

inline void focusToEdit()
{
	Sleep(100);

	auto tabCtrl =
		reinterpret_cast<CnCSuite_Tabcontrol*>(
			getComponentHandle(COMP_ID_TAB_CONTROL)
		);

	if (tabCtrl != nullptr)
	{
		tabCtrl->SetFocusOnCurrentVisibleEditbox();
	}
}

inline void updateFocusRect()
{
	Sleep(1000);

	auto tabCtrl =
		reinterpret_cast<CnCSuite_Tabcontrol*>(
			getComponentHandle(COMP_ID_TAB_CONTROL)
			);

	if (tabCtrl != nullptr)
	{
		SendMessage(
			tabCtrl->GetCurrentVisibleEditboxHandle(),
			WM_UPDATEFOCUSRECT,
			0, 0
		);
	}
}


#endif// _CNCS_TABCONTROL_INCLUDED_
