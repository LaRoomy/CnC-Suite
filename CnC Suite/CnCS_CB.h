#pragma once
#include"CnCSuite_CBox.h"

#define			ERM_REMOVE			1
#define			ERM_ADDERROR		2

typedef struct _LVDISPSTRUCT {
	int type;
	TCHAR* code;
	TCHAR* definition;
	TCHAR* location;
}LVDISPSTRUCT, *LPLVDISPSTRUCT;

class CnCS_CB : public CnCSuite_CBox
{
public:		CnCS_CB(HINSTANCE, HWND);
			~CnCS_CB();

			void CnCS_CB::Release() { delete this; }
			HRESULT CnCS_CB::Init(HWND CBoxFrame) { return this->_init(CBoxFrame); }
			void CnCS_CB::DisplayNotification(int type, LPCTSTR code, LPCTSTR text, LPCTSTR location) { this->_display(type, code, text, location); }
			void CnCS_CB::RemoveError(int type, LPCTSTR code, LPCTSTR location) { this->_remove(type, code, location); }
			void CnCS_CB::Clear() { ListView_DeleteAllItems(this->Listview); }
			BOOL CnCS_CB::RequestErrorValidity() { return this->_requestErrorValidity(); }
			void CnCS_CB::onDpiChanged() { this->onDPIChanged(); }

			// ??? temp ???
			void CnCS_CB::ResizeWindow() { this->OnSize(); }
private:
	HWND CBFrame;
	HWND Listview;
	HWND Main;
	HINSTANCE hInstance;
	DWORD Ecount;
	BOOL TimerActive;

	LPLVDISPSTRUCT lvd;

	HFONT lvFont;

	HRESULT _init(HWND);
	HRESULT InitColumns();
	HRESULT InitImageList();

	static LRESULT CALLBACK CBoxSub(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);

	void _display(int, LPCTSTR, LPCTSTR, LPCTSTR);
	void _remove(int, LPCTSTR, LPCTSTR);

	LRESULT OnSize();
	LRESULT OnNotify(LPARAM);
	LRESULT OnSetAppStyle(LPARAM);
	LRESULT OnValidateError(LPARAM);

	BOOL ErrorStorageControl(int, int, LPCTSTR, LPCTSTR, LPCTSTR);
	BOOL ValidateError(int, LPCTSTR, LPCTSTR, DWORD*);
	BOOL _requestErrorValidity();

	HWND GetErrorSource(LPTSTR);

	void onDPIChanged();
	void _createDpiDependendResources();
};

CnCSuite_CBox* CreateCBoxInstance(HINSTANCE hInst, HWND MainWindow)
{
	if ((hInst == NULL) || (MainWindow == NULL))
		return (CnCSuite_CBox*)NULL;
	else
		return new CnCS_CB(hInst, MainWindow);
}