#pragma once
#include"..//external.h"
#include<ctrlHelper.h>
#include<StringClass.h>
#include<ItemCollection.h>
#include"..//HelperF.h"

#define		CLISTVIEWCLASS		L"cListViewClass"

#define		LV_DESELECT			-1
#define		LV_SELECTHIGHEST	-28

typedef struct _LISTVIEWPROPERTIES {

	POINT position;
	SIZE size;

	CTRLID ctrlID;
	int numColumns;

	int padding;
	int columnTextAlignment;

	COLORREF background;
	COLORREF textcolor;

	DWORD additionalHostWndStyles;
	DWORD additionalListviewStyles;
	DWORD additionalExtendedListviewStyles;

	HFONT ctrlFont;

}LISTVIEWPROPERTIES, *LPLISTVIEWPROPERTIES;

class listViewEventSink
{
public:
	virtual void onListviewItemSelected(cObject sender, int selectedRowIndex) = 0;
};

class listViewItem
	: public ClsObject<listViewItem>,
	public iCollectable<listViewItem>
	
{
public:
	listViewItem() {
		this->numItemsInRow = 1;
		this->initalize();
	}
	listViewItem(int itemsInRow)
	:numItemsInRow(itemsInRow) {
		if (itemsInRow > 0)
		{
			this->initalize();
		}
		else
			this->rowItem = nullptr;
	}
	listViewItem(const listViewItem& item)
	{
		this->duplicateFrom(item);
	}
	~listViewItem()
	{
		this->Clear();
	}

	iString getDataInRow(int rowIndex) const
	{
		if (rowIndex < this->rowItem->GetCount())
		{
			return this->rowItem->GetAt(rowIndex);
		}
		else
		{
			iString err(L"-error-:type:invalid_argument:-section-:class:listviewitem:getdatainrow");
			return err;
		}
	}

	bool isValid()
	{
		if (this->rowItem != nullptr)
			return true;
		else
			return false;
	}

	bool setDataInRow(int rowIndex, const TCHAR* data)
	{
		iString _data_(data);
		return this->setDataInRow(rowIndex, _data_);
	}

	bool setDataInRow(int rowIndex, iString& data)
	{
		if (this->rowItem == nullptr)return false;
		else
		{
			auto cnt = this->rowItem->GetCount();

			if (rowIndex >= cnt)return false;
			else
			{
				this->rowItem->ReplaceAt(rowIndex, data);
				return true;
			}
		}
	}

	listViewItem& operator= (const listViewItem& item)
	{
		this->Replace(item);
		return *this;
	}
	bool operator== (const listViewItem& item) {

		auto nItems = item.getRowCount();
		for (int i = 0; i < nItems; i++)
		{
			if (this->getDataInRow(i) != item.getDataInRow(i))
			{
				return false;
			}
		}
		return true;
	}

	listViewItem* listViewItem::getInstance() { return this; }
	
	void Replace(const listViewItem& item)
	{
		this->Clear();
		this->duplicateFrom(item);
	}
	
	void Clear()
	{
		if (this->rowItem != nullptr)
		{
			this->rowItem->Release();
		}
		this->numItemsInRow = 0;
	}

	void ClearData()
	{
		if (this->rowItem != nullptr)
		{
			iString placeholder("");

			for (int i = 0; i < this->numItemsInRow; i++)
			{
				this->rowItem->ReplaceAt(i, placeholder);
			}
		}
	}

	int getRowCount() const { return this->numItemsInRow; }

	const wchar_t* ToString() {
		return L"listviewitem";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

private:
	int numItemsInRow;

	itemCollection<iString>* rowItem;

	void duplicateFrom(const listViewItem& item)
	{
		this->numItemsInRow = item.numItemsInRow;
		this->rowItem = itemCollection<iString>::Copy(item.rowItem);
	}
	void initalize()
	{
		this->rowItem = new itemCollection<iString>();

		for (int i = 0; i < this->numItemsInRow; i++)
		{
			iString placeholder("---");
			this->rowItem->AddItem(placeholder);
		}
	}
};

class cListView
	: public ClsObject<cListView>
{
public:
	cListView(HINSTANCE hInst);
	~cListView();

	HRESULT Create(LPCTRLCREATIONSTRUCT creationals);

	void setEventHandler(listViewEventSink* handler);
	void setColors(COLORREF bkgnd, COLORREF text);
	void setParent(HWND _parent);
	// possible alignment parameters -> LVCFMT_LEFT / LVCFMT_CENTER / LVCFMT_RIGHT
	void setColumnDefinitions(int numcolumns, itemCollection<iString>* columnnames, itemCollection<int>* columnwidths, int columntextalignment);
	void setPadding(int padding);
	void setAdditonalHostWindowStyles(DWORD styles);
	void setAdditonalListViewStyles(DWORD styles, DWORD extendedStyles);
	void setFont(HFONT font);

	CTRLID getCtrlID() { return this->Properties.ctrlID; }
	int getSelectedRowIndex() { return this->selectedRowIndex; }
	HWND getListViewHandle() { return this->listViewWnd; }
	listViewItem getItemAtIndex(int index);

	void setSelectedRow(int index);
	void setFocus() { SetFocus(this->listViewWnd); }

	void addItem(listViewItem& item);
	void updateItem(int index, listViewItem& item);
	void deleteItemAt(int index);
	void deleteSelection();

	void reloadData(bool keepSelection);
	void clear();

	void activateEventBlocker();
	void deactivateEventBlocker();

	const wchar_t* ToString() {
		return L"customlistview";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

private:
	HINSTANCE hInstance;
	HWND listViewWnd;
	HWND hostWnd;
	HWND parentWnd;

	bool eventblocker;

	int selectedRowIndex;

	itemCollection<listViewItem> lvItems;
	itemCollection<iString> columnNames;
	itemCollection<int> columnWidths;

	LISTVIEWPROPERTIES Properties;

	listViewEventSink* eventHandler;

	HRESULT registerListViewHost();
	HRESULT initListviewColumns();

	static LRESULT CALLBACK lvHostProc(HWND, UINT, WPARAM, LPARAM);

	LRESULT onPaint(HWND);
	LRESULT onNotify(LPARAM);

	void setDefaultValues();

	void insertLastItemToView();
	void insertItemFromIndex(int);
};
