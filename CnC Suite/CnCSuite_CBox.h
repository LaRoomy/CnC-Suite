#pragma once
#ifndef _CNCSUITE_CODEBOX_
#define _CNCSUITE_CODEBOX_

#include"external.h"

#define		CBOX_DISPLAY_ERROR				0
#define		CBOX_DISPLAY_WARNING			1
#define		CBOX_DISPLAY_INFORMATION		2

class CnCSuite_CBox {
public:
	virtual void Release() = 0;
	virtual HRESULT Init(HWND CBoxFrame) = 0;
	virtual void DisplayNotification(int type, LPCTSTR code, LPCTSTR text, LPCTSTR location) = 0;
	virtual void RemoveError(int type, LPCTSTR code, LPCTSTR location) = 0;
	virtual void Clear() = 0;
	virtual BOOL RequestErrorValidity() = 0;
	virtual void onDpiChanged() = 0;
	virtual void ResizeWindow() = 0;
};

CnCSuite_CBox* CreateCBoxInstance(HINSTANCE hInst, HWND MainWindow);

#endif // _CNCSUITE_CODEBOX_
