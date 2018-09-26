#pragma once
#ifndef _CSCROLLBAR_H_
#define _CSCROLLBAR_H_

#include<Windows.h>
#include"..//cObject.h"

constexpr auto SCROLLPOS_MAX = -100;
#define	SCROLLPOS_MIN	-101

typedef int ScrollBarType;

// Protocol for the user-generated events on the scrollbar
// For an example of the implementation look at the end of cScrollbar.h
__interface IScrollEventProtocol
{
public:
	void OnLinePlus(cObject sender, int currentPos);
	void OnLineMinus(cObject sender, int currentPos);
	void OnPagePlus(cObject sender, int currentPos);
	void OnPageMinus(cObject sender, int currentPos);
	void OnMouseWheelPlus(cObject sender, int currentPos);
	void OnMouseWheelMinus(cObject sender, int currentPos);
	void OnThumbTrack(cObject sender, int absoluteTrackPosition);// the absolute trackposition is the middle-position of the thumb (the thumb has already moved)
};
// NOTE:
// when the user interacts with the scrollbar, the scrollbar calls the methods of the IScrollEventProtocol Interface
// ! -> the scrollbar does not scroll itself, the subscriber of the interface must call 'SetScrollPosition(...)' when the events will be called
// 
// To make it easier to implement the functionality of content-scrolling, the component-class should inherit from the IScrollable class
class IScrollable
{
public:
	virtual int GetLineSize(ScrollBarType type) = 0;

	int GetPageSize(HWND hWnd)
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		return rc.bottom;
	}

	double GetPageCount(HWND hWnd, int contentHeight)
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		return ((double)contentHeight / (double)rc.bottom);
	}

	int GetPageScrollHeight(HWND hWnd, int contentHeight)
	{
		auto pCount = this->GetPageCount(hWnd, contentHeight);
		if (pCount > 1)
		{
			pCount -= 1;

			if (pCount > 1)
			{
				auto psh = (double)contentHeight / pCount;
				return (int)psh;
			}
		}
		return contentHeight;
	}
};

class CSScrollbar
	: public ClsObject<CSScrollbar>
{
public:
	CSScrollbar();
	CSScrollbar(HINSTANCE hInst);
	~CSScrollbar();

	// Creates the VerticalScrollbar
	// REMARKS:
	//	If the pos and/or the size parameter is nullptr, the scrollbar calculates the position and size itself.
	//	Verical Bar: By default it is right-aligned and fills the parent in Y-direction, the default-width is 16
	//  Horizontal Bar: By default it is bottom-aligned and fills the parent in X-direction, the default height is 16
	HRESULT Create(HWND Parent, LPPOINT pos, LPSIZE size);
	HRESULT Create(HINSTANCE hInst, HWND Parent, LPPOINT pos, LPSIZE size);

	// Sets the Instance parameter if the object was created with the default constructor
	void SetInstance(HINSTANCE hInst) {
		this->hInstance = hInst;
	}

	// Sets the range of the scrollbar - this value should not be negative
	bool SetScrollRange(int range);

	// sets the current position of the thumb
	// NOTE: This value must be inside the range, otherwise it will be set to nearest endpoint
	//       The redraw of the scrollbar must be explicitly specified - otherwise it won't be updated
	void SetScrollPosition(int position);
	void SetScrollPosition(int position, bool redraw);

	// Sets the automatic hide function
	// NOTE: this is disabled by default and must be explicitly enabled
	void SetAutoHide(bool AutoHide) {
		this->autoHide = AutoHide;
	}

	// Shows or hides the VerticalScrollbar - if the scrollbar is set to hidden state, autohide will also be set to false
	// otherwise it will be automatically shown if the scrollbar enters the enabled state
	void SetShowState(bool state);

	// Sets the invisible border on the right/bottom and the left/top side of the window, the default is 100
	// If the thumb is tracked and the cursor overruns this border, the tracking will be suspended until the cursor moves back into the valid area
	void SetTrackStopBorderSize(int tsbSize) {
		this->trackStopBorderSize = tsbSize;
	}

	// Hook up the event-handler for the scrollbar-events
	void SetEventHandler(IScrollEventProtocol* handler) {
		this->scrollEvents = handler;
	}

	// Sets the type of the scrollbar
	void SetScrollbarType(ScrollBarType type);

	// Sets the images for the plus & minus arrow
	// NOTE:
	//   The squaresize should be equal to the thickness of the scrollbar
	//   The plus image increases the scrollposition, the minus image decreases the scrollposition
	void SetImages(int plusImageID, int minusImageID, int squareSize);
	void SetStyleImages(int plusImageHighlightID, int plusImagePressedID, int minusImageHighlightID, int minusImagePressedID);
	void SetDisabledImages(int plusImageDisabledID, int minusImageDisabledID);

	// Sets the colors for the scrollbar
	void SetColors(COLORREF background, COLORREF thumbColor, COLORREF thumbHighlightColor, COLORREF thumbPressedColor);

	// Sets the default thickness of the bar - the inital value is 16 and thats the minimum value, the maximum is 32
	void SetDefaultThickness(int thickness);

	// Resizes the scrollbar to it's default position
	// NOTE: If the scrollbar is set to a custom position, this method erases the custom parameter and sets the bar to its default position
	//     >> To Prevent this, call the method 'Move(...)' instead
	void Resize();

	// Updates only the thumb of the scrollbar
	// NOTE: This method also calls 'SetScrollPosition(...)' but does not redraw the entire window, only the thumb
	void UpdateThumb(int newScrollPosition);

	int GetScrollPosition() { return this->scrollPosition; }
	int GetScrollRange() { return this->scrollRange; }

	// Moves the scrollbar
	void Move(int x, int y, int cx, int cy);

	bool IsVisible() {
		return IsWindowVisible(this->scrollBarWnd) ? true : false;
	}

	static const ScrollBarType SCROLLBARTYPE_VERTICAL = 1;
	static const ScrollBarType SCROLLBARTYPE_HORIZONTAL = 2;

	static const WCHAR* CSSCROLLBAR_CLASS_IDENTIFIER;

	const wchar_t* ToString() {
		return L"scrollbar";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

private:
	HINSTANCE hInstance;
	HWND scrollBarWnd;
	HWND parentWnd;

	POINT Position;
	SIZE Size;
	RECT ThumbRect;

	DWORD scrollBarType;

	int scrollRange;
	int scrollPosition; // the scrollposition is the absolute value inside the range! The draw-position must be calculated

	int defaultThickness;
	int iconSquareSize;
	int trackStopBorderSize;

	int trackOffset;

	HICON plusIcon;
	HICON minusIcon;

	HICON plusIconHgl;
	HICON plusIconPrs;

	HICON minusIconHgl;
	HICON minusIconPrs;

	HICON plusIconDisabled;
	HICON minusIconDisabled;

	HBRUSH bkgndBrush;
	HBRUSH thumbBrush;
	HBRUSH thumbHglBrush;
	HBRUSH thumbPrsBrush;

	bool autoHide;
	bool enabledState;

	bool plusButtonHover;
	bool plusButtonPressed;
	bool minusButtonHover;
	bool minusButtonPressed;
	bool thumbHover;
	bool thumbPressed;

	bool mouseleaveScheduled;
	bool inputCaptured;

	IScrollEventProtocol* scrollEvents;

	HRESULT registerScrollClass();

	static LRESULT CALLBACK scrollBarProc(HWND scrollBar, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT onPaint(HWND scrollBar);
	LRESULT onMouseWheel(WPARAM wParam);
	LRESULT __fastcall onMousemove(WPARAM wParam, LPARAM lParam);
	LRESULT onMouseLeave();
	LRESULT onLButtonDown(LPARAM lParam);
	LRESULT onLButtonUp(LPARAM lParam);
	LRESULT onLButtonDblClick(LPARAM lParam);
	LRESULT onSize();

	void __fastcall drawVerticalBar(HDC hdc, HWND sbar);
	void __fastcall drawHorizontalBar(HDC hdc, HWND sbar);

	// if the height is greater than the range, hide scrollbar!

	bool calculateDefaultSize();

	int __fastcall calculateHalfThumbLength(int windowlength);// if the return-value is -1 the scrollbar should be disabled
	int __fastcall calculateRelativeThumbPosition(int windowlength);
	int __fastcall calculateAbsoluteThumbPosition(int x, int y);
	int calculateTrackOffset(int x, int y);

	void updateThumbRect();
	void getThumbActivationRect(LPRECT prc);
	void getButtonRects(LPRECT plusButton, LPRECT minusButton);
	void resetControlParameter();
	void disableMouseLeaveNotification();
};

#endif // !_CSCROLLBAR_H_

//// scrollbar definitions in a derived class:
//
//class DerivedClass : 	public IScrollEventProtocol,public IScrollable
//{
	//	int GetLineSize(){ return 24; } 
	//
	//void OnLinePlus(cObject sender, int currentPos) {
	//	reinterpret_cast<CSScrollbar*>(sender)
	//		->SetScrollPosition(
	//			currentPos
	//			+ this->GetLineSize(),
	//			true
	//		);
	//		// Scoll the Window-Content !
	//}
	//void OnLineMinus(cObject sender, int currentPos) {
	//	reinterpret_cast<CSScrollbar*>(sender)
	//		->SetScrollPosition(
	//			currentPos
	//			- this->GetLineSize(),
	//			true
	//		);
	//		// Scoll the Window-Content !
	//}
	//void OnPagePlus(cObject sender, int currentPos) {
	//	reinterpret_cast<CSScrollbar*>(sender)
	//		->SetScrollPosition(
	//			currentPos
	//			+ this->GetPageScrollHeight(this->Window, this->contentHeight),
	//			true
	//		);
	//		// Scoll the Window-Content !
	//}
	//void OnPageMinus(cObject sender, int currentPos) {
	//	reinterpret_cast<CSScrollbar*>(sender)
	//		->SetScrollPosition(
	//			currentPos
	//			- this->GetPageScrollHeight(this->Window, this->contentHeight),
	//			true
	//		);
	//		// Scoll the Window-Content !
	//}
	//void OnMouseWheelPlus(cObject sender, int currentPos) {
	//	reinterpret_cast<CSScrollbar*>(sender)
	//		->SetScrollPosition(
	//			currentPos
	//			+ (3 * this->GetLineSize()),
	//			true
	//		);
	//		// Scoll the Window-Content !
	//}
	//void OnMouseWheelMinus(cObject sender, int currentPos) {
	//	reinterpret_cast<CSScrollbar*>(sender)
	//		->SetScrollPosition(
	//			currentPos
	//			- (3 * this->GetLineSize()),
	//			true
	//		);
	//		// Scoll the Window-Content !
	//}
	//void OnThumbTrack(cObject sender, int absoluteTrackPos) {
	//	reinterpret_cast<CSScrollbar*>(sender)
	//		->UpdateThumb(absoluteTrackPos);

	//		// Scoll the Window-Content !
	//};
//};

