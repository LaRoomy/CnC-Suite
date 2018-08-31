#pragma once
#ifndef _ICONSOLE_H_
#define _ICONSOLE_H_

//#include"..//external.h"

#include<Windows.h>

#include"..//Resource.h"

#include<cScrollbar.h>
#include"..//cObject.h"
#include"..//EditorContentManager.h"

typedef int ConsoleType;

typedef struct _CARETPROPERTY {
	RECT cArea;
	bool isVisible;
}CARETPROPERTY, *LPCARETPROPERTY;

// launches events when the user interact with the console
__interface IConsoleInputEvent {
public:
	// Return true to insert the default command-entry at the beginning of the next line.
	// Additional text could be inserted in the console before this functions comes back
	bool UserPressedReturn(cObject sender);
	bool UserEnteredCommand(cObject sender, LPCWSTR command);
};

class CSConsole
	: public ClsObject<CSConsole>,
	public IScrollEventProtocol,
	public IScrollable
{
public:
	CSConsole();
	CSConsole(HINSTANCE hInst);
	~CSConsole();

	HRESULT Create(HWND Parent, LPPOINT pos, LPSIZE size, DWORD CtrlID);
	// Sets the default colors for front- and background
	void SetColors(COLORREF background, COLORREF defaultText);
	// Adds a line to the console and prints it in defaultcolor
	bool AddLine(LPCWSTR data);
	// Adds a line to the console and prints it in the specified color
	bool AddLine(LPCWSTR data, COLORREF lineColor);
	// Adds an empty line
	bool AddEmptyLine();
	// Adds a character at the end of the current line
	bool AddCharacterToLine(WCHAR data);
	// Updates all parameter and redraws the console
	void Update();
	// Updates the caret (and erases the old one)
	void UpdateCaret() {
		this->eraseCaret();
		this->updateCaretRect();
		this->drawCaret();
	}
	// Erases the console
	void Clear();
	// This function should be used to move the console instead of using the window handle
	void Move(int x, int y, int cx, int cy);

	// Derived Method from the IScrollable-class
	// Retrieves the size of a line in pixel (Intended for internal use)
	int GetLineSize(ScrollBarType type);

	// Retrievess the height of the pure text-content
	int GetContentHeight();

	// Retrieves a handle to the console-window - USE WITH CAUTION!
	HWND GetConsoleHandle() {
		return this->consoleWindow;
	}

	// Sets the line height
	// -> the minimum is 10px
	// -> the default is 24px
	void SetLineHeight(int lHgt);

	// Sets the property of the text, the values must be in an appropriate ratio to the lineheight
	// -> the default is:  height = 18px ; weight = FW_MEDIUM ; faceName = Consolas
	void SetFontProperty(int height, int weight, LPCTSTR faceName);

	// Sets the control-font directly
	void SetFontDirect(HFONT font);

	void SetConsoleType(ConsoleType type) {
		this->cType = type;
	}

	// Sets the default entry before the cursor when a new line is inserted
	void SetCommandEntry(LPCWSTR entry);

	// Sets the startpoint of text from the left
	void SetLeftMargin(int marginLeft);

	// Sets the width of the caret-beam
	void SetCaretWidth(int cWidth);

	// Sets the auto-scroll property - if enabled the window scrolls automatically to the last line when a new line is inserted!
	void SetAutoScroll(bool autoscroll);

	// Sets additional windows styles for the console-Window - must be called before the 'create' method
	void SetAdditonalWindowStyles(DWORD styles) {
		this->additionalStyles = styles;
	}

	void RegisterForUserEvents(IConsoleInputEvent* eventHandler) {
		this->inputEvents = eventHandler;
	}

	// This method prevents all input and update operations - it must be called if the user want's to close the host-window with an 'exit' command
	void Freeze(bool enable) {
		this->freezer = enable;
	}

	const wchar_t* ToString() {
		return L"custom console X";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

	static const WCHAR* CONSOLE_CLASS_ID;

	static const ConsoleType CONSOLETYPE_READONLY = 0x01;
	static const ConsoleType CONSOLETYPE_ACCEPTINPUT = 0x02;
	static const ConsoleType CONSOLETYPE_SCROLLABLE = 0x04;
	static const ConsoleType CONSOLETYPE_MULTICOLOR = 0x08;

	// scrollbar events:
	void OnLinePlus(cObject sender, int currentPos){
		reinterpret_cast<CSScrollbar*>(sender)
			->SetScrollPosition(
				currentPos
				+ this->GetLineSize(CSScrollbar::SCROLLBARTYPE_VERTICAL),
				true
			);
		this->Update();
	}
	void OnLineMinus(cObject sender, int currentPos){
		reinterpret_cast<CSScrollbar*>(sender)
			->SetScrollPosition(
				currentPos
				- this->GetLineSize(CSScrollbar::SCROLLBARTYPE_VERTICAL),
				true
			);
		this->Update();
	}
	void OnPagePlus(cObject sender, int currentPos){
		reinterpret_cast<CSScrollbar*>(sender)
			->SetScrollPosition(
				currentPos
				+ this->GetPageScrollHeight(this->consoleWindow, this->contentHeight),
				true
			);
		this->Update();
	}
	void OnPageMinus(cObject sender, int currentPos){
		reinterpret_cast<CSScrollbar*>(sender)
			->SetScrollPosition(
				currentPos
				- this->GetPageScrollHeight(this->consoleWindow, this->contentHeight),
				true
			);
		this->Update();
	}
	void OnMouseWheelPlus(cObject sender, int currentPos) {
		reinterpret_cast<CSScrollbar*>(sender)
			->SetScrollPosition(
				currentPos
				+ (2 * this->GetLineSize(CSScrollbar::SCROLLBARTYPE_VERTICAL)),
				true
			);
		this->Update();
	}
	void OnMouseWheelMinus(cObject sender, int currentPos) {
		reinterpret_cast<CSScrollbar*>(sender)
			->SetScrollPosition(
				currentPos
				- (2 * this->GetLineSize(CSScrollbar::SCROLLBARTYPE_VERTICAL)),
				true
			);
		this->Update();
	}
	void OnThumbTrack(cObject sender, int absoluteTrackPos) {
		reinterpret_cast<CSScrollbar*>(sender)
			->UpdateThumb(absoluteTrackPos);
		this->Update();
	};

private:
	HINSTANCE hInstance;
	HWND consoleWindow;
	HWND parentWindow;

	DWORD additionalStyles;

	POINT consolePosition;
	SIZE consoleSize;

	WCHAR* commandEntry;

	static const DWORD CRT_BLINKTIMER_ID = 4134;

	int lineHeight;
	int contentHeight;
	int margin_left;
	int caretWidth;
	DWORD ctrlID;
	ConsoleType cType;

	bool hasFocus;// only set caret if the console has the focus!
	bool autoScroll;
	bool freezer;

	IConsoleInputEvent *inputEvents;
	CSScrollbar* VerticalScrollbar;
	EditorContentManager ContentManager;
	CARETPROPERTY CaretProperty;

	itemCollection<COLORREF> lineColor;

	COLORREF backgroundColor;
	COLORREF defaultTextColor;

	HFONT ctrlFont;
	HBRUSH backgroundBrush;
	HBRUSH caretBrush;

	HRESULT registerConsoleClass();
	
	static LRESULT CALLBACK consoleProc(HWND hWndConsole, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT onPaint(HWND console);
	LRESULT onSize();
	LRESULT onMouseWheel(WPARAM wParam);
	LRESULT onLButtonDown();
	LRESULT onTimer(WPARAM wParam);
	LRESULT onChar(WPARAM wParam);
	LRESULT onSetFocus();
	LRESULT onKillFocus();

	void onBackspace();
	void onLinefeed();

	void draw(HDC hdc);

	int getFirstVisibleLineIndex(int* invisibleHeight);
	void setDefaultParameter();
	void eraseCaret();
	void drawCaret();
	void updateCaretRect();
	void updateContentHeight();
	void controlTimer(bool enable);
	COLORREF getLineColor(int index);
};

#endif // !_ICONSOLE_H_


