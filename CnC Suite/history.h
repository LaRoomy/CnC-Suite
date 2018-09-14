#pragma once
#ifndef _CNCSUITE_HISTORY_H_
#define _CNCSUITE_HISTORY_H_

#include<Windows.h>
#include"cObject.h"
#include<StringClass.h>
#include<ItemCollection.h>
#include<ctrlHelper.h>
#include<CustomButton.h>

#include "DateTime.h"
#include"EditorContentManager.h"
#include"CnC Suite.h"

constexpr auto BID_QUIT = 10;

typedef uintX HistoryAction;

class HistoryItem
	: public ClsObject<HistoryItem>,
	public iCollectable<HistoryItem>
{
public:
	HistoryItem();
	HistoryItem(const HistoryItem& hi);
	~HistoryItem();

	static const HistoryAction NOT_DEFINED	= 0;
	static const HistoryAction FILE_OPENED	= 0x0001;
	static const HistoryAction FILE_CHANGED	= 0x0002;
	static const HistoryAction FILE_DELETED	= 0x0004;
	static const HistoryAction FILE_MOVED	= 0x0008;
	static const HistoryAction FILE_COPIED	= 0x0010;
	static const HistoryAction FILE_RENAMED = 0x0020;
	static const HistoryAction FILE_CREATED = 0x0040;

	HistoryItem& operator= (const HistoryItem& hi) {
		this->Clear();
		this->historyAction = hi.GetActionType();
		this->path.Replace(
			hi.GetItemPath()
		);
		this->displayName.Replace(
			hi.GetDisplayName()
		);
		this->creationTime = hi.GetCreationTime();
		this->lastWriteTime = hi.GetLastWriteTime();
		this->lastOpenedTime = hi.GetLastOpenedTime();
		this->langID = hi.langID;
		return *this;
	}

	void Clear();

	void SetLangID(LANGID lID) {
		this->langID = lID;
		this->creationTime.SetLangID(lID);
		this->lastWriteTime.SetLangID(lID);
		this->lastOpenedTime.SetLangID(lID);
	}

	const wchar_t* ToString() {
		this->generateStringRepresentation();
		return this->representation.GetData();
	}

	void FromString(const wchar_t* stringRepresentation);

	void SetActionType(HistoryAction hiac) {
		this->historyAction = hiac;
	}
	const HistoryAction GetActionType() const {
		return this->historyAction;
	}

	void SetItemPath(LPCTSTR _path_) {
		this->path.Replace(_path_);
		this->setDisplayName();
		this->setTimes();
	}

	//LPCTSTR GetItemPath() const {
	//	return path.GetData();
	//}

	const iString& GetItemPath() const {
		return this->path;
	}
	const iString& GetDisplayName() const {
		return this->displayName;
	}
	const DateTime GetCreationTime() const {
		return this->creationTime;
	}
	const DateTime GetLastWriteTime() const {
		return this->lastWriteTime;
	}
	const DateTime GetLastOpenedTime() const {
		return this->lastOpenedTime;
	}

	// Sets the current time as the last time the file was opened (must be invoked when the file-action is executed and saved)
	void SetLastOpenedTime() {
		this->lastOpenedTime.Clear();
		this->lastOpenedTime.SetLangID(this->langID);
		this->lastOpenedTime.FromLocalTime();
	}

private:
	iString path;
	iString displayName;
	iString representation;
	uintX historyAction;
	DateTime creationTime;
	DateTime lastWriteTime;
	DateTime lastOpenedTime;

	LANGID langID;

	void setTimes();
	void setDisplayName();
	void generateStringRepresentation();
};

__interface IHistroyEventProtocoll {
public:
	void OnEntryClicked(cObject sender, HistoryItem* item);
	void OnWindowClosed(cObject sender);
};

class History
	: public ClsObject<History>,
	public IConversionProtocol
{
public:
	History();
	~History();

	void Clear();

	bool IsReady() {
		return this->isReady;
	}

	bool CanDelete() {
		return this->canDelete;
	}

	int GetItemCount();

	void AddHistoryItem(HistoryItem& item) {
		this->historyList.AddLine(
			item.ToString()
		);
	}

	HistoryItem& GetHistoryItemAt(int index);

	bool ToFile(LPCTSTR path);
	bool FromFile(LPCTSTR path);

	//ClsObject Base:
	const wchar_t* ToString() {
		return L"not_impl_history_cls";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

	//IConversionProtocoll Base
	void CollectionComplete(cObject sender);
	void TextBufferReady(cObject sender);
	void ConversionError(cObject sender) {
		UNREFERENCED_PARAMETER(sender);
	}

private:
	bool isReady;
	bool canDelete;
	EditorContentManager historyList;
	LPTSTR filePath;
	HistoryItem curItem;

};

class UIHistory
	: public ClsObject<UIHistory>, public customButtonEventSink
{
public:
	UIHistory();
	~UIHistory();

	static const WCHAR* HISTORY_WINDOW_CLASS;

	HRESULT ShowHistoryWindow(LPCTRLCREATIONSTRUCT pccs);
	void CloseHistoryWindow();

	void SetColors(COLORREF BackgroundColor,COLORREF ItemColor, COLORREF SelectedItemColor, COLORREF TextColor, COLORREF AccentTextColor, COLORREF OutlineColor);

	void SetEventHandler(IHistroyEventProtocoll* hEvent) {
		this->hHistoryEvent = hEvent;
	}

	//ClsObject Base:
	const wchar_t* ToString() {
		return L"not_impl_uihistory_cls";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

	//button event Base:
	void onCustomButtonClick(cObject sender, CTRLID ID);

private:
	HINSTANCE hInstance;
	HWND historyWindow;
	CTRLID windowID;
	HWND parent;

	int currentIndexPos;
	int currentSelectedIndex;
	int mousePosY;

	bool trackingEvent;
	bool isWndOpen;
	bool canReset;

	RECT selectedArea;

	HBRUSH backgroundBrush;
	HBRUSH outlineBrush;
	HBRUSH itemBrush;
	HBRUSH selectedItemBrush;
	HPEN outlinePen;
	HFONT nameFont;
	HFONT dateTimeFont;
	HFONT pathFont;

	COLORREF textColor;
	COLORREF textAccentColor;
	COLORREF backgroundColor;
	COLORREF outlineColor;

	APPSTYLEINFO styleInfo;

	IHistroyEventProtocoll* hHistoryEvent;

	History historyData;

	HRESULT registerClass();
	HRESULT createControls();
	HRESULT createListing();

	static LRESULT CALLBACK historyProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT onPaint(HWND hWnd);
	LRESULT onMouseMove(LPARAM lParam);
	LRESULT onMouseWheel(LPARAM lParam);
	LRESULT onLButtonDown(LPARAM lParam);
	LRESULT onLButtonUp(LPARAM lParam);
	LRESULT onMouseLeave();
	LRESULT onDestroy();

	void onQuit();

	bool drawHistoryItem(HDC hdc, int itemIndex, LPRECT itemRect);

	void getListArea(LPRECT prc);
	int getItemHeight();
	int itemIndexFromPosition(int Ypos);
	void eraseSelectedArea();
	bool isOverValidItem(int pos_y);
};


#endif // !_CNCSUITE_HISTORY_H_
