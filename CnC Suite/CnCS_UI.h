#pragma once
#include "CnCSuite_Userinterface.h"
///////////////////////////////////
#include<StringClass.h>
#include"HelperF.h"
///////////////////////////////////
#define		BDRAW_NORMAL		1
#define		BDRAW_MARKED		2
#define		BDRAW_PRESSED		3

#define		IS_BUTTON_ITEM		4
#define		IS_STATUSBAR_ITEM	5

#define		DRAW_ALL_AREAS		6
#define		DRAW_NCS_AREAS		7

#define		NS_SIZEAREA_SIZING	8
#define		WE_SIZEAREA_SIZING	9

typedef int ItemType;

typedef struct StaticObjects {

	HBRUSH backgroundBrush;
	HBRUSH titlebarBrush;
	HBRUSH sizeWndBrush;
	HBRUSH frameWndBrush;
	HBRUSH statusbarBrush;
	HBRUSH styleBrush;
	HBRUSH mainToolbarBrush;

	HFONT titlefont;
	HFONT statusbarfont;

	HPEN outlinePen;
	HPEN separatorPen;

	HCURSOR arrow;
	HCURSOR size_nwso;
	HCURSOR size_ns;
	HCURSOR size_we;

	HICON size_cross;
	HICON appSysIcon;
	HICON appSettingsIcon_norm;
	HICON appSettingsIcon_marked;
	HICON appSettingsIcon_pressed;
	HICON maximize_norm;
	HICON maximize_marked;
	HICON maximize_pressed;
	HICON maximize_norm_min;
	HICON maximize_marked_min;
	HICON maximize_pressed_min;
	HICON minimize_norm;
	HICON minimize_marked;
	HICON minimize_pressed;
	HICON close_norm;
	HICON close_mark;
	HICON close_pressed;
	HICON size_arrow;
	HICON size_arrow_disabled;
	HICON new_norm;
	HICON new_mark;
	HICON new_pressed;
	HICON open_norm;
	HICON open_mark;
	HICON open_pressed;
	HICON save_norm;
	HICON save_mark;
	HICON save_pressed;
	HICON saveall_norm;
	HICON saveall_mark;
	HICON saveall_pressed;
	HICON saveas_norm;
	HICON saveas_mark;
	HICON saveas_pressed;
	HICON copy_norm;
	HICON copy_mark;
	HICON copy_pressed;
	HICON dropdown_norm;
	HICON dropdown_mark;
	HICON dropdown_pressed;
	HICON paste_norm;
	HICON paste_mark;
	HICON paste_pressed;
	HICON cut_norm;
	HICON cut_mark;
	HICON cut_pressed;
	HICON selectall_norm;
	HICON selectall_mark;
	HICON selectall_pressed;
	HICON editdelete_norm;
	HICON editdelete_mark;
	HICON editdelete_pressed;
	HICON import_norm;
	HICON import_mark;
	HICON import_pressed;
	HICON export_norm;
	HICON export_mark;
	HICON export_pressed;
	HICON send_norm;
	HICON send_mark;
	HICON send_pressed;
	HICON receive_norm;
	HICON receive_mark;
	HICON receive_pressed;
	HICON help_norm;
	HICON help_mark;
	HICON help_pressed;
	HICON web_norm;
	HICON web_mark;
	HICON web_pressed;
	HICON quit_norm;
	HICON quit_mark;
	HICON quit_pressed;

}STATICOBJECTS,*LPSTATICOBJECTS;

typedef struct InternalParameter {

	BOOL MoveAreaActivated;
	BOOL SizeAreaActivated;
	BOOL WindowIsMaximized;
	BOOL WindowIsMinimized;
	BOOL DropdownbuttonIsVisible;
	BOOL dropdownbutton_active;
	BOOL ext_toolbar_exsisting;
	BOOL WindowSizeUserdefined;
	BOOL SizeChanged;
	BOOL FrameSizeAreaActivated;
	BOOL TimerActive;
	BOOL blockSettingsButtonNormalization;
	BOOL blockButtonAnimation;

	int DRAW_MODE;
	int LAST_TBBUTTON_ID;
	int ACTIVE_BUTTON_ID;

	HHOOK MouseHook;

	int InitialPos_X;
	int InitialPos_Y;
	int add_val_x;
	int add_val_y;
	int sizingStartHeight_Main;
	int sizingStartHeight_Editframe;

	RECT rc_multiU;

	HWND statusBar_WND;
	HWND extendedToolbar_WND;
	HWND TVFrame_WND;
	HWND EditFrame_WND;
	HWND CBoxFrame_WND;
	HWND SizeBeam_WND;

	int TVFrameWidth;
	int EditFrameHeight;

}IPARAM,*LPIPARAM;

typedef struct BTN_DrawInfo {

	HICON icon;
	HBITMAP bitmap;
	int cx;
	int cy;
	HBRUSH bkgnd;

}BUTTONDRAWINFO,*LPBUTTONDRAWINFO;

typedef struct _ENUMCHILDSIZEINFO {
	RECT rc_main_client;
	LONG_PTR ptrToClass;
}ENUMCHILDSIZEINFO, *LPENUMCHILDSIZEINFO;

typedef struct { int ul; int ur; int tb; int sc; }BMPID, *LPBMPID;

class CnCS_UI : public CnCSuite_Userinterface {

public:
	CnCS_UI(HINSTANCE, LPAPPSTYLEINFO);
	~CnCS_UI();

	HRESULT CnCS_UI::Init(HWND hWnd, LPCSTMFRAME cFrameSize) { return this->Init_UI(hWnd, cFrameSize); }
	void CnCS_UI::Release() { delete this; }
	LRESULT CnCS_UI::DefaultHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) { return this->_DefaultHandler(hWnd, message, wParam, lParam); }
	void CnCS_UI::SetStatusbarInfo(int part, LPCTSTR text) { this->ChangeStatusbar(part, text); }
	HWND CnCS_UI::GetFrameHandles(int RequestedFrame) { return this->Get_Frame_Handles(RequestedFrame); }
	BOOL CnCS_UI::GetMaximizeRect(LPRECT rc) { return this->Get_MaximizeRect(rc); }
	HBRUSH CnCS_UI::GetBkgndBrush() { return this->pObj->backgroundBrush; }
	void CnCS_UI::onDpiChanged() { this->dpiChanged(); }
	BOOL CnCS_UI::IsReady() { return (this->MainWindow != nullptr) ? TRUE : FALSE; }
	void CnCS_UI::setParameter(int parameterID, LONG_PTR value) { this->setParameterFromExtern(parameterID, value); }

private:
	BOOL startup_result;
	HWND MainWindow;
	HINSTANCE hInstance;

	iString statusbarTextHolder[3];

	LPSTATICOBJECTS pObj;
	LPIPARAM iParam;
	APPSTYLEINFO styleInfo;

	HRESULT Init_UI(HWND,LPCSTMFRAME);
	HRESULT RegisterChildClasses();

	BOOL Init_Childs();
	BOOL CreateStatusBar();
	BOOL CreateTVFrame();
	BOOL CreateEditFrame();
	BOOL CreateCBoxFrame();
	BOOL CreateTBButtons();
	BOOL CreateExtendedToolbar();
	BOOL CreateSizeBeam(LPRECT,int);

	LRESULT _DefaultHandler(HWND, UINT, WPARAM, LPARAM);

	static LRESULT CALLBACK ButtonSub(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
	static LRESULT CALLBACK StatusbarSub(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
	static LRESULT CALLBACK ExtendedToolbarSub(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
	static LRESULT CALLBACK SizeWndProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK FrameWndProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK MouseProc(int, WPARAM, LPARAM);

	LRESULT OnMinMaxInfo(LPARAM);
	LRESULT OnPaint(HWND);
	LRESULT OnSize(HWND, WPARAM, LPARAM);
	LRESULT OnMove(HWND);
	LRESULT OnTimer(HWND, WPARAM);
	LRESULT OnDrawItem(HWND, LPARAM);
	LRESULT OnMouseMove(HWND, WPARAM, LPARAM);
	LRESULT OnNormalize(HWND, WPARAM);
	LRESULT OnLButtondown(HWND, WPARAM, LPARAM);
	LRESULT OnLButtonUp(HWND, LPARAM);
	LRESULT OnEraseBkgnd(WPARAM);
	LRESULT OnSysCommand(HWND, WPARAM, LPARAM);
	LRESULT OnSetAppStyle(LPARAM);
	LRESULT OnProcessNewInstance();
	LRESULT OnBlockInput(WPARAM wParam);

	void OnRestoreWindow(HWND, LPARAM);
	void OnRestoreStatusbar();
	void OnRestoreToolbar();
	void OnRestoreFrames();
	void OnRestoreFloatingWnd();
	void OnMaximized();
	void Global_Tracking(LPMOUSEHOOKSTRUCT,CnCS_UI*);
	void HandleSpecialActions(int);
	void ChangeStatusbar(int, LPCTSTR);
	void DrawAll(HWND);
	void DrawNeccessaryAreas(HWND);
	void WindowChanged(HWND);
	void SetNewFrameAlignment();
	void dpiChanged();

	BOOL MakeButton(HWND, int, int, int, int, DWORD);
	BOOL MakeTooltip(HWND, int);
	BOOL DrawButtonWithIcon(HWND, LPDRAWITEMSTRUCT, HBRUSH, HICON, int, int);
	BOOL DrawButtonWithBitmap(HWND, LPDRAWITEMSTRUCT, HBITMAP, int, int);
	BOOL DrawButtonFromID(HWND, LPDRAWITEMSTRUCT, int, int);
	BOOL GetButtonDrawInfo(int, int, LPBUTTONDRAWINFO);
	BOOL DrawStatusbar(LPDRAWITEMSTRUCT);

	BOOL SwitchButtonActivationStatus(int, int);

	int WhatIsItem(int);
	void SavePreSizingParameter(LPRECT main, LPRECT editframe);
	void AdaptEditframeHeight();
	HWND GetButtonHandle(int);
	HWND Get_Frame_Handles(int);
	BOOL Get_MaximizeRect(LPRECT);
	void ControlFloatingWndShowState(int);
	void setParameterFromExtern(int pID, LONG_PTR val);

	void _createDpiDependedResources(bool cleanOld);
};

CnCSuite_Userinterface* CreateUI_Instance(HINSTANCE hInst, LPAPPSTYLEINFO pStyleInfo) { return new CnCS_UI(hInst, pStyleInfo); }