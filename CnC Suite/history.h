#pragma once
#ifndef _CNCSUITE_HISTORY_H_
#define _CNCSUITE_HISTORY_H_

#include<Windows.h>
#include"cObject.h"
#include<StringClass.h>
#include<ItemCollection.h>
#include<ctrlHelper.h>
#include<CustomButton.h>
#include<cScrollbar.h>

#include "DateTime.h"
#include"EditorContentManager.h"
#include"CnC Suite.h"
#include"FileSystem.h"

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
	bool operator== (const HistoryItem& hi) {
		return
			(this->path.Equals(hi.path)
				&& this->historyAction == hi.historyAction)
			? true : false;
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

__interface IHistroyEventProtocol {
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
	void ReplaceHistoryItemAt(HistoryItem& item, int index) {
		this->historyList.ReplaceLine(
			item.ToString(),
			index
		);
	}

	HistoryItem& GetHistoryItemAt(int index);

	// Return-Values: (TRUE == Equal) (FALSE == Not Equal) (2 == the path is a segment of the itemPath)
	// (-1 == Error!)
	BOOL CompareHistoryItemPathAt(int index, LPCTSTR path);

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
	: public ClsObject<UIHistory>, public customButtonEventSink, public IScrollEventProtocol, public IScrollable
{
public:
	UIHistory();
	~UIHistory();

	static const WCHAR* HISTORY_WINDOW_CLASS;

	HRESULT ShowHistoryWindow(LPCTRLCREATIONSTRUCT pccs);
	void CloseHistoryWindow();

	void AddToHistory(HistoryItem& item) {
		this->historyData.AddHistoryItem(item);
		this->UpdateScrollbar();
	}
	void ClearCompleteHistory() {
		this->historyData.Clear();
		if (this->isWndOpen)
			this->Update();
	}
	void Save();
	void Update() {
		RedrawWindow(this->historyWindow, nullptr, nullptr, RDW_NOCHILDREN | RDW_NOERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}
	int GetEntryCount() {
		return this->historyData.GetItemCount();
	}

	void SetColors(COLORREF BackgroundColor,COLORREF ItemColor, COLORREF SelectedItemColor, COLORREF TextColor, COLORREF AccentTextColor, COLORREF OutlineColor);

	void SetEventHandler(IHistroyEventProtocol* hEvent) {
		this->hHistoryEvent = hEvent;
	}

	void OnFilesystemModification(LPFILESYSTEMOBJECT fso);

	//ClsObject Base:
	const wchar_t* ToString() {
		return L"not_impl_uihistory_cls";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

	//button event Base:
	void onCustomButtonClick(cObject sender, CTRLID ID);

	//scrollbar event Base
	int GetLineSize(ScrollBarType type){
		UNREFERENCED_PARAMETER(type);
		return this->getItemHeight();
	}
	
	void OnLinePlus(cObject sender, int currentPos) {
		reinterpret_cast<CSScrollbar*>(sender)
			->SetScrollPosition(
				currentPos
				+ this->GetLineSize(0),
				true
			);
		// Scoll the Window-Content !
		this->itemDOWN();
	}
	void OnLineMinus(cObject sender, int currentPos) {
		reinterpret_cast<CSScrollbar*>(sender)
			->SetScrollPosition(
				currentPos
				- this->GetLineSize(0),
				true
			);
		// Scoll the Window-Content !
		this->itemUP();
	}
	void OnPagePlus(cObject sender, int currentPos) {
		reinterpret_cast<CSScrollbar*>(sender)
			->SetScrollPosition(
				currentPos
				+ this->GetPageScrollHeight(
					this->historyWindow,
					this->getContentHeight()
				),
				true
			);
		// Scoll the Window-Content !
		this->updatePage();
	}
	void OnPageMinus(cObject sender, int currentPos) {
		reinterpret_cast<CSScrollbar*>(sender)
			->SetScrollPosition(
				currentPos
				- this->GetPageScrollHeight(
					this->historyWindow,
					this->getContentHeight()
				),
				true
			);
		// Scoll the Window-Content !
		this->updatePage();
	}
	void OnMouseWheelPlus(cObject sender, int currentPos) {
		reinterpret_cast<CSScrollbar*>(sender)
			->SetScrollPosition(
				currentPos
				+ this->GetLineSize(0),
				true
			);
		// Scoll the Window-Content !
		this->itemDOWN();
	}
	void OnMouseWheelMinus(cObject sender, int currentPos) {
		reinterpret_cast<CSScrollbar*>(sender)
			->SetScrollPosition(
				currentPos
				- this->GetLineSize(0),
				true
			);
		// Scoll the Window-Content !
		this->itemUP();
	}
	void OnThumbTrack(cObject sender, int absoluteTrackPos);

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
	bool redrawScrollbar;

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

	IHistroyEventProtocol* hHistoryEvent;

	History historyData;
	CSScrollbar* verticalScrollbar;

	HRESULT registerClass();
	HRESULT createControls();
	HRESULT createListing();

	static LRESULT CALLBACK historyProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT onPaint(HWND hWnd);
	LRESULT onMouseMove(LPARAM lParam);
	LRESULT onMouseWheel(WPARAM wParam);
	LRESULT onLButtonDown(LPARAM lParam);
	LRESULT onLButtonUp(LPARAM lParam);
	LRESULT onMouseLeave();
	LRESULT onDestroy();
	LRESULT onSize();

	void onQuit();
	void onItemRenamed(LPFILESYSTEMOBJECT fso);

	bool drawHistoryItem(HDC hdc, int itemIndex, LPRECT itemRect);

	void getListArea(LPRECT prc);
	int getItemHeight();
	int getContentHeight();
	int itemIndexFromPosition(int Ypos);
	void eraseSelectedArea();
	bool isOverValidItem(int pos_y);

	bool itemUP();
	bool itemDOWN();
	void updatePage();

	void UpdateScrollbar();
};


#endif // !_CNCSUITE_HISTORY_H_
