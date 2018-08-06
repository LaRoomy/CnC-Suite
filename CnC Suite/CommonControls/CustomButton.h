#pragma once
#include"..//external.h"
#include<StringClass.h>
#include<ctrlHelper.h>

#define		BUTTONMODE_TEXT			1
#define		BUTTONMODE_ICON			2
#define		BUTTONMODE_ICONTEXT		3
#define		BUTTONMODE_BITMAP		4

#define		BAL_LEFT				10
#define		BAL_RIGHT				11
#define		BAL_CENTER				12

#define		CUSTOMBUTTONCLASS_FROMID(hwnd,id)	(LRESULT)(SNDMSG(GetDlgItem(((HWND)hwnd),((int)id)),((UINT)WM_GETWNDINSTANCE), ((WPARAM)0), ((LPARAM)0)))

typedef struct _BUTTONPROPERTY
{
	POINT ptOffset;
	int x;
	int y;
	int width;
	int height;
	CTRLID ctrlID;
	int iconID;
	int disabledIcon;
	int iconSqareSize;
	HICON icon;
	int bitmapID;
	SIZE szBitmap;
	HBITMAP bitmap;
	HFONT font;
	iString text;
	iString tooltip;
	HWND button;
	HWND parent;
	HWND hwndToolTip;
	COLORREF textColor;
	COLORREF highlightTextColor;
	COLORREF disabledTextColor;
	DWORD alignment;
	int padding;
	int spacing;

	PARTIALBORDER borderParts;

	HBRUSH Bkgnd;
	HBRUSH mouseover;
	HBRUSH pressed;
	HBRUSH disabled;
	HPEN border;

	HHOOK mouseHook;

}BUTTONPROPERTY, *LPBUTTONPROPERTY;

typedef struct _BUTTONCONTROL
{
	BOOL buttonIsPressed;
	BOOL mouseIsOver;
	BOOL isTouchedDown;
	BOOL drawBorder;
	BOOL isEnabled;
	BOOL sizeToContext;
	BOOL parentIsComposited;
	BOOL textHighlightOnSelection;

	bool mouseAnimation;
	bool touchdownlock_active;

}BUTTONCONTROL, *LPBUTTONCONTROL;

class customButtonEventSink
{
public:
	virtual void onCustomButtonClick(cObject sender, CTRLID ID_button) = 0;
};

class CustomButton
{
public:
	CustomButton(HWND Parent, DWORD _buttonMode, LPPOINT position, LPSIZE size, int ctrlID, HINSTANCE hInst);
	~CustomButton();

	HRESULT setBorder(BOOL drawBorder, COLORREF borderColor);
	HRESULT setAppearance_onlyText(iString& text, BOOL applySizeToContext);
	HRESULT setAppearance_onlyIcon(int Icon, int sqareSize);
	HRESULT setAppearance_IconText(int Icon, int sqareSize, iString& text);
	HRESULT setAppearance_Bitmap(int bitmap, int width, int height);
	HRESULT setColors(COLORREF normalBackground, COLORREF onMouseOverBkgnd, COLORREF onPressedBkgnd);
	HRESULT setFont(HFONT font);
	HRESULT setTextColor(COLORREF);
	HRESULT setTooltip(iString& tooltip);
	// the disabled-icon must have the same sqaresize like the normal icon!
	HRESULT setDisabledIcon(int icon);
	void setDisabledColors(COLORREF bkgndDisabled, COLORREF textDisabled);
	void setDisabledColor(COLORREF disabledBackground);
	// this works only for Icon-text or plain-text mode!
	HRESULT setAlignment(DWORD align);
	void setConstraints(int padding, int spacing);
	void setTouchdownLock(bool holdButtonOnTouchdown);
	void buttonDownRelease();
	void setButtonDown();
	void setEnabledState(bool state);
	void workWithCompositedWindows(bool wwcw);
	void setContentOffset(int x_offset, int y_offset);
	void setPartialBorderParts(bool left, bool top, bool right, bool bottom);
	void setMouseAnimation(bool animate);
	void setTextHighlight(BOOL highlight, COLORREF highlightColor);
	
	CTRLID getID();

	DWORD getThreadId() { return this->threadId; }
	BOOL isCompositionControl() { return this->buttonControl.parentIsComposited; }

	HRESULT setEventListener(customButtonEventSink* _eventSink);

	HRESULT Create();

	void Update();
	void updateDimensions(LPPOINT pos, LPSIZE size);

	void Release() { delete this; }
	HWND getHandle() { return this->buttonProperty.button; }

private:
	UINT_PTR subclassID;
	DWORD threadId;
	DWORD buttonMode;
	HINSTANCE hInstance;
	BUTTONPROPERTY buttonProperty;
	BUTTONCONTROL buttonControl;

	customButtonEventSink* eventSink;

	static LRESULT CALLBACK buttonSub(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
	static LRESULT CALLBACK mouseProc(int, WPARAM, LPARAM);

	LRESULT onMouseMove(HWND);
	LRESULT onLButtonDown(HWND);
	LRESULT onLButtonUp(HWND);
	LRESULT onMouseLeave(HWND);
	LRESULT onPaint();
	LRESULT onSize();

	void initButtonProperty();

	void drawText(HDC, LPRECT);
	void drawIcon(HDC, LPRECT, BOOL);
	void drawIconText(HDC, LPRECT, DWORD);

	void ResetControlParameters();
};
