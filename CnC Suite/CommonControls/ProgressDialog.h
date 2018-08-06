#pragma once
#include"..\external.h"
#include<ctrlHelper.h>
#include<StringClass.h>
#include"..\Graphics\workingIndicator.h"

#define		WM_STEPPROGRESS		WM_USER  + 1
#define		WM_STEPANIMATION	WM_USER  + 2

typedef DWORD ProgressType;

class ProgressDialog
{
public:
	ProgressDialog();
	~ProgressDialog();

	void setDisplayMessage(iString& dMsg)
	{
		this->displayMessage = dMsg;
	}
	void setType(ProgressType _type)
	{
		this->type = _type;
	}
	// The Range is limited to 1.000.000 steps
	// the default is 376
	void setAbsoluteRange(DWORD _range)
	{
		if (_range > ProgressDialog::MAXIMUMPROGRESS_RANGE)
			this->absoluteRange = ProgressDialog::MAXIMUMPROGRESS_RANGE;
		else
			this->absoluteRange = _range;
	}
	void setBackgroundColor(COLORREF color)
	{
		this->cBackground = color;
		this->backgroundBrush = CreateSolidBrush(color);
	}
	void setDialogSizeParameter(int x, int y, int cx, int cy);

	static ProgressDialog* getOpenInstance();

	static const ProgressType PROGRESSTYPE_PROGRESS = 0x01;
	static const ProgressType PROGRESSTYPE_WORKER = 0x02;
	static const ProgressType PROGRESSTYPE_POPUP = 0x04;
	static const ProgressType PROGRESSTYPE_CHILD = 0x08;
	static const ProgressType PROGRESSTYPE_BORDER = 0x10;
	static const ProgressType PROGRESSTYPE_CLOSEONFULLPROGRESS = 0x20;
	static const ProgressType PROGRESSTYPE_ANIMATE = 0x40;

	static const DWORD MAXIMUMPROGRESS_RANGE = 1000000;
	static const TCHAR* PROGRESSCLASS;
	static const CTRLID PROGRESSBAR_ID = 123;

	HRESULT Create(HWND owner, HINSTANCE hInst);

	// always call close to destroy the window -> Close() enables the owner - otherwise the owner remains disabled
	void Close();
	void Update();
	// when finished -> Close(); will be called
	void FinishAndClose();

	void Release() {
		delete this;
	}

	// returns false if the current range has reached the absolute range
	bool stepRange();
	// returns false if the current range is higher than the requested range
	bool setCurrentRange(DWORD _range);

	void disableOwnerUntilFinish(bool dsbl) { this->disableOwner = dsbl; }

private:
	HINSTANCE hInstance;
	ProgressType type;
	POINT position;
	SIZE wndSize;
	HWND hDlg;
	HWND hBar;
	HWND ownerWindow;
	HFONT dlgFont;
	DWORD absoluteRange;
	DWORD currentRange;
	DWORD animationPosition;
	iString displayMessage;
	COLORREF cBackground;
	HBRUSH backgroundBrush;
	bool disableOwner;

	workingIndicator* pWI;

	static LRESULT CALLBACK progressProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK barProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
	static DWORD __stdcall animateProc(LPVOID);
	static DWORD __stdcall increaseProc(LPVOID);

	void _updateDialog();
	void _updateProgressBar();

	HRESULT _registerProgressClass();
	HRESULT _createContent();

	LRESULT onPaint(HWND);
	LRESULT onBarPaint(HWND);
	LRESULT onStepProgress();
	LRESULT onStepAnimation();
};