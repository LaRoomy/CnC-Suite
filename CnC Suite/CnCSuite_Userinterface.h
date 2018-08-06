#pragma once
// >> include the external dependencies
#include"external.h"
///////////////////////////////////////
#ifndef _CNCSUITE_USERINTERFACE_
#define _CNCSUITE_USERINTERFACE_

#define		GFWH_TVFRAME			1
#define		GFWH_EDITFRAME			2
#define		GFWH_CBOXFRAME			3

#define		MAXIMIZENORMAL			5

#define		UNI_FRAME_CLASS			L"UNIVERSALFRAMECLASS"

#define		PARAID_UI_SETTINGSBUTTON_TOUCHDOWNLOCK		10

typedef struct CustomFrameSize {
	BOOL valid;
	int nCmdShow;
	int TVFrameWidth;
	int EditFrameHeight;
}CSTMFRAME,*LPCSTMFRAME;

class CnCSuite_Userinterface {

public:
	virtual HRESULT Init(HWND Main, LPCSTMFRAME cFrameSize) = 0;
	virtual void Release() = 0;
	virtual LRESULT DefaultHandler(HWND, UINT, WPARAM, LPARAM) = 0;
	virtual BOOL IsReady() = 0;
	virtual void SetStatusbarInfo(int part, LPCTSTR text) = 0;
	virtual HWND GetFrameHandles(int RequestedFrame) = 0;
	virtual BOOL GetMaximizeRect(LPRECT rc) = 0;
	virtual HBRUSH GetBkgndBrush() = 0;
	virtual void setParameter(int parameterID, LONG_PTR value) = 0;
	virtual void onDpiChanged() = 0;
};

CnCSuite_Userinterface* CreateUI_Instance(HINSTANCE, LPAPPSTYLEINFO);


#endif //_CNCSUITE_USERINTERFACE_
