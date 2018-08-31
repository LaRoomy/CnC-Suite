#pragma once
#include"external.h"
#include<CustomButton.h>
#include<StringClass.h>

#define		SPLASHSCREENCLASS		L"SPLASHSCREENWNDCLASS"

#define		TIMERID_15MSEC		20

#define		BITMAP_SIZE_CX		400
#define		BITMAP_SIZE_CY		314

typedef struct _GRAPHICOUTPUTINFO {

	int currentPosition[3];
	iString outputString;

}GRAPHICOUTPUTINFO, *LPGRAPHICOUTPUTINFO;

class splashScreen
	: public ClsObject<splashScreen>
{
public:
	splashScreen(HINSTANCE hInst);
	~splashScreen();

	HRESULT Start();
	HRESULT Stop();

	const wchar_t* ToString() {
		return L"splash screen";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}


private:
	HINSTANCE hInstance;
	HWND sScreen;
	BOOL canceledByUser;
	DWORD exitButtonState;
	HHOOK mouseHook;

	HBRUSH runnerBrush;

	HICON exitIcon;
	HICON exitIconMarked;
	HICON exitIconPressed;
	HFONT font;

	GRAPHICOUTPUTINFO gInfo;

	HRESULT create();
	HRESULT createControls();
	HRESULT registerSCClass();

	static LRESULT CALLBACK splashProc(HWND, UINT, WPARAM, LPARAM);
	static DWORD WINAPI	asyncProc(LPVOID);
	static LRESULT CALLBACK mouseProc(int nCode, WPARAM wParam, LPARAM lParam);

	bool startAsync();
	DWORD run();

	LRESULT onPaint(HWND);
	LRESULT onTimer(HWND, WPARAM);
	LRESULT onCommand(HWND, WPARAM, LPARAM);
	LRESULT onLButtonDown(LPARAM lParam);

	void onExitButtonClick();

	void cleanUp();
	void setOutputString(LPARAM);
	HICON getCurrentExitIcon();
};
