#pragma once
#ifndef CNCSUITE_RESET_DIALOG_H_
#define CNCSUITE_RESET_DIALOG_H_
#include"external.h"
#include<CustomButton.h>
#include<CustomCheckbox.h>

#define		RDLG_CTRLID_EXITBUTTON			20
#define		RDLG_CTRLID_RESETBUTTON			21
#define		RDLG_CTRLID_CANCELBUTTON		22

#define		RDLG_CTRLID_COMPARAMETER_CBX		25
#define		RDLG_CTRLID_COMMONSETTINGS_CBX		26
#define		RDLG_CTRLID_AUTOSYNTAXSETUP_CBX		27
#define		RDLG_CTRLID_USERTEXT_CBX			28
#define		RDLG_CTRLID_INTERNALSETUP_CBX		29
#define		RDLG_CTRLID_AUTOCOMPLETESTRINGS_CBX	30
#define		RDLG_CTRLID_APPDATA_CBX				31
#define		RDLG_CTRLID_SESSIONDATA_CBX			32

typedef struct _CHECKSTATES {
	bool commonSettings;
	bool interfaceParameter;
	bool autosyntaxSetup;
	bool usertext;
	bool internalSetup;
	bool autocompleteSetup;
	bool appdataSetup;
	bool sessionData;
}CHECKSTATES, *LPCHECKSTATES;

class ResetDialog
	: public ObjectRelease<ResetDialog>,
	public customButtonEventSink,
	public customCheckboxEventSink
{
public:
	ResetDialog();
	~ResetDialog();

	HRESULT Create(HINSTANCE hInst, HWND hWndMain);
	void SetColors(COLORREF background, COLORREF text);

	void CloseDialog();

	static const TCHAR* RESET_DLG_CLASS_ID;

	// button events:
	void onCustomButtonClick(cObject sender, CTRLID ctrlID);
	// checkbox event:
	void onCustomCheckboxChecked(cObject sender, bool newState);

private:
	HINSTANCE hInstance;
	HWND resetDlgWnd;
	HWND MainWindow;

	bool displayRestartMessage;

	COLORREF backgroundColor;
	COLORREF textColor;

	HFONT font;

	CHECKSTATES cStates;

	HRESULT registerResetWnd();
	HRESULT createControls();

	static LRESULT CALLBACK resetProc(HWND resetWnd, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT onPaint(HWND hDlg);

	void executeReset();
};

#endif // !CNCSUITE_RESET_DIALOG_H_
