#pragma once
#include"..\\external.h"
#include"ItemCollection.h"
#include"StringClass.h"

#define		COMBOTYPE_SIMPLE		1
#define		COMBOTYPE_DROPDOWN		2
#define		COMBOTYPE_DROPDOWNLIST	3

class comboBoxEventSink
{
public:
	virtual void onComboBoxSelectionChanged(cObject sender, int selIndex) = 0;
};

class comboBox
	: public CollectionEvents<iString>
{
public:
	comboBox(HINSTANCE hInstance, HWND parent,DWORD Type, DWORD ID, int x, int y, int cx, int cy);
	~comboBox()
	{
		SafeRelease(&this->Items);

		DeleteObject(this->ctrlFont);
	}

	itemCollection<iString>* Items;

	// event handler //////////////////////////////////////////////
	void comboBox::OnItemAdded(const iString& item, int itemIndex)
	{
		UNREFERENCED_PARAMETER(itemIndex);

		auto stringItem = item;
		this->AddItem(stringItem.GetData());
	}
	void comboBox::OnItemRemoved(int removedAt)
	{
		this->RemoveItem(removedAt);
	}
	void comboBox::OnItemsCleared()
	{
		SendMessage(this->Combo, CB_RESETCONTENT, 0, 0);
	}
	////////////////////////////////////////////////////////////////
	// properties >>
	int getSelectedIndex() { return this->selectedIndex; }
	void setSelectedIndex(int index)
	{
		ComboBox_SetCurSel(this->Combo, index);
		this->selectedIndex = index;
	}
	DWORD getCtrlId() { return this->Id; }
	////////////////////////////////////////////////////////////////

	void setEventHandler(comboBoxEventSink* handler) { this->eventHandler = handler; }

	void setFont(HFONT font);

	HWND GetHandle() { return this->Combo; }
	BOOL Succeded() { return this->Success; }
	void Release() { delete this; }

private:
	HINSTANCE hInst;
	HWND Combo;
	HWND Parent;
	DWORD Id;
	BOOL Success;
	HFONT ctrlFont;

	comboBoxEventSink* eventHandler;

	int selectedIndex;

	void AddItem(const TCHAR* item);
	void RemoveItem(int index);

	static LRESULT CALLBACK comboSub(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);

	DWORD GetStylesFromType(DWORD);
};