#pragma once
#include"..//external.h"

#define		WKI_CLASS		L"WORKINGINDICATOR_CLASS\0"

#define		STOP_THREAD_IMMEDIATELY		1

#define		WINDOWMODE_CHILD			2
#define		WINDOWMODE_POPUP			3

#define		ANIMATIONMODE_LINEAR			5
#define		ANIMATIONMODE_CIRCLE			6
#define		ANIMATIONMODE_LINEAR_EXTENDED	7	// animationtime not available in this mode

#define		ID_EXWORKING_TIMER			10

#define		FINISH_SEQUENCE				0xFFFFFF

typedef struct _WIGRAPHICOUTPUTINFO {

	int currentPosition[5];
	int calculatedValue[5];

}WIGRAPHICOUTPUTINFO, *LPWIGRAPHICOUTPUTINFO;


// Recall that the maximum number off circling balls depends on the window-with and the gap (spacing) programmed in the section

class workingIndicator
	: public ObjectRelease<workingIndicator>
{
public:
	workingIndicator(HINSTANCE hInst, int indicationIcon);
	workingIndicator(HINSTANCE hInst);
	~workingIndicator();

	//void workingIndicator::Release() { delete this; } //done in objectRelease

	// ! watch out: do not destroy the host window !
	// -> call the kill method on the object instead
	HRESULT showAsPopUp(HWND parent_, LPRECT rc, COLORREF bkgnd);
	// ! watch out: do not destroy the host window !
	// -> call the kill method on the object instead
	HRESULT showAsChild(HWND parent_, LPRECT rc, COLORREF bkgnd);
	void kill();

	// the maximum of possible moving items depends on the window-width an the spacing (width / spacing = max items)
	// the animationtime depends on the systemspeed
	void setAnimationProperty(DWORD Mode, int numOfMovingItems, DWORD _animationTime, DWORD _spacing);

	//extended functions - do not combine with normal functions >>

	// height is always 5!
	HRESULT showExtendedAnimation(HWND _parent, LPPOINT pos, int width);
	void setExtendedAnimationProperty(DWORD Mode, COLORREF _background, int iconID);
	// set the object-reference to nullptr after killing the worker!
	void killEx(DWORD delay_ms);

	void setFinishNotificationTarget(HWND msgReceiver);

private:
	HWND parent;
	HWND workingWnd;
	HWND notifyTarget;
	HINSTANCE hInstance;
	COLORREF background;
	COLORREF moverColor;
	HBRUSH bkgndBrush;
	HBRUSH moverBrush;
	HICON iIcon;
	DWORD animationMode;
	DWORD windowMode;
	DWORD animationTime;
	DWORD spacing;
	DWORD delay;

	WIGRAPHICOUTPUTINFO gInfo;

	int numMover;
	int stopThread;

	bool animationStarted;

	static LRESULT CALLBACK	WNDProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK WndSubProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
	static DWORD WINAPI indicationProc(LPVOID);
	static DWORD WINAPI killdelayProc(LPVOID);

	void startIndicationThread();
	void startDelayThread();

	LRESULT onPaint(HWND);
	LRESULT onAnimate(HWND, LPARAM);
	LRESULT onExTimer(HWND, WPARAM);

	int calculateHeigtFromLength(int, int);
	void animateEx(HWND, HDC);
	void animateExOpt(HWND, HDC);
};


inline void killWorkingIndicatorPopUp()
{
	HWND inst = FindWindow(WKI_CLASS, nullptr);
	if (inst != nullptr)
	{
		auto wki = reinterpret_cast<workingIndicator*>(
			GetWindowLongPtr(inst, GWLP_USERDATA));
		if (wki != nullptr)
		{
			wki->kill();
		}
	}
}

inline void killWorkingIndicatorChild(HWND parent, bool delay)
{
	HWND inst = GetDlgItem(parent, ID_WORKINGINDICATOR);
	if (inst != nullptr)
	{
		auto wki = reinterpret_cast<workingIndicator*>(
			GetWindowLongPtr(inst, GWLP_USERDATA));

		if (wki != nullptr)
		{
			wki->kill();

			if (delay)
			{
				Sleep(10);
			}
		}
	}
}

inline workingIndicator* getWorkingIndicatorInstance(HWND in_opt_parent )
{
	HWND inst = FindWindow(WKI_CLASS, nullptr);
	if (inst != nullptr)
	{
		return reinterpret_cast<workingIndicator*>(
			GetWindowLongPtr(inst, GWLP_USERDATA));
	}
	else
	{
		if (in_opt_parent != nullptr)
		{
			inst = GetDlgItem(in_opt_parent, ID_WORKINGINDICATOR);
			if (inst != nullptr)
			{
				return reinterpret_cast<workingIndicator*>(
					GetWindowLongPtr(inst, GWLP_USERDATA));
			}
			else
				return nullptr;
		}
		else
			return nullptr;
	}
}
