#pragma once
#include"..//external.h"
#include"StringClass.h"
#include"CustomButton.h"
#include"ItemCollection.h"

#define		CPOPUPMENU_CLASS		L"CustomPopUpMenu\0"

#define		MENU_ENTRY_PURE_TEXT		2
#define		MENU_ENTRY_ICON_TEXT		3

class CustomPopUpMenu;

class MenuEntry
	: public ClsObject<MenuEntry>,
	  public iCollectable<MenuEntry>
{
public:
	friend class CustomPopUpMenu;

	MenuEntry();
	MenuEntry(const MenuEntry& M)
	{
		this->backgroundcolor = M.backgroundcolor;
		this->highlightedcolor = M.highlightedcolor;
		this->iconID = M.iconID;
		this->pressedcolor = M.pressedcolor;
		this->squaresize = M.squaresize;
		this->textcolor = M.textcolor;
		this->itemHeight = M.itemHeight;
		this->entryStyle = M.entryStyle;
		this->ENTRY_ID = M.ENTRY_ID;
		iString _txt_ = M.text;
		this->text = _txt_;
	}
	~MenuEntry();

	//void MenuEntry::Release() { delete this; }

	void setID(int entryID);
	void setText(iString *_text);
	void setText(LPCTSTR _text);
	void setItemheight(int height);
	void setColors(COLORREF background, COLORREF textcolor, COLORREF highlighted, COLORREF pressed);
	void setIcon(int iconId, int squareSize);
	void setBackground(COLORREF background);

	int getExpectedItemHeight() {
		return (this->itemHeight >= squaresize) ? this->itemHeight : (this->squaresize + 4);
	}

	MenuEntry* MenuEntry::getInstance() { return this; }

	MenuEntry& operator= (const MenuEntry &M)
	{
		this->backgroundcolor = M.backgroundcolor;
		this->highlightedcolor = M.highlightedcolor;
		this->iconID = M.iconID;
		this->pressedcolor = M.pressedcolor;
		this->squaresize = M.squaresize;
		this->textcolor = M.textcolor;
		this->itemHeight = M.itemHeight;
		this->entryStyle = M.entryStyle;
		this->ENTRY_ID = M.ENTRY_ID;
		this->text = M.text;
		return *this;
	}

	bool operator== (const MenuEntry& entry) {
		return (this->ENTRY_ID == entry.ENTRY_ID) ? true : false;
	}

	void Clear()
	{
		this->text.Clear();
		this->backgroundcolor = RGB(240, 240, 240);
		this->textcolor = RGB(0, 0, 0);
		this->highlightedcolor = RGB(0, 120, 180);
		this->pressedcolor = RGB(0, 100, 150);
		this->iconID = 0;
		this->squaresize = 0;
		this->itemHeight = 22;
		this->entryStyle = MENU_ENTRY_PURE_TEXT;
		this->ENTRY_ID = 0;
	}

	const wchar_t* ToString() {
		return L"menu entry";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

private:
	iString text;
	COLORREF backgroundcolor;
	COLORREF textcolor;
	COLORREF highlightedcolor;
	COLORREF pressedcolor;
	int iconID;
	int squaresize;
	int itemHeight;
	int entryStyle;
	int ENTRY_ID;
};

class customPopUpMenuEventSink
{
public:
	virtual void onPopUpMenuButtonClicked(CTRLID Id) = 0;
	//virtual void onClose() = 0;
};

class CustomPopUpMenu
	: public ClsObject<CustomPopUpMenu>,
	  public customButtonEventSink
{
public:
	CustomPopUpMenu(HINSTANCE hInst);
	CustomPopUpMenu(HINSTANCE hInst, customPopUpMenuEventSink* _eventListener);
	~CustomPopUpMenu();

	void setEventListener(customPopUpMenuEventSink* _eventListener);
	void setControlFont(HFONT font);
	void setCloseExclusionArea(LPRECT area);

	void addMenuEntry(MenuEntry* entry);

	HRESULT Show(HWND _parent_, LPPOINT position, int width, COLORREF background);
	void Close();

	void CustomPopUpMenu::onCustomButtonClick(cObject sender, CTRLID ID) {
		UNREFERENCED_PARAMETER(sender);
		this->onButtonClick(ID);
	}

	const wchar_t* ToString() {
		return L"custom popup menu";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

private:
	HWND parent;
	HWND thisWnd;
	DWORD threadID;
	HHOOK mouseHook;
	HINSTANCE hInstance;
	HBRUSH hBackground;
	HFONT ctrlFont;
	RECT exclusionArea;
	BOOL inputCaptured;

	int numEntries;
	bool buttonWasClicked;
	bool useExclusionArea;

	itemCollection<MenuEntry> mEntries;

	customPopUpMenuEventSink* eventListener;

	HRESULT createContent();

	static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK MouseProc(int, WPARAM, LPARAM);

	int calculateItemHeigth();

	void onButtonClick(CTRLID ID);

	void setHooks(HINSTANCE);
};