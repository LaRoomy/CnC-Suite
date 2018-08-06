#include "ComboBox.h"
#include"ctrlHelper.h"
#include"..\HelperF.h"

comboBox::comboBox(HINSTANCE hInstance, HWND parent, DWORD Type, DWORD ID, int x, int y, int cx, int cy)
	:hInst(hInstance),
	Parent(parent),
	selectedIndex(-1),
	Id(ID),
	eventHandler(nullptr)
{
	this->Success = TRUE;
	this->ctrlFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Segoe UI\0");

	this->Combo = 		
		CreateWindow(
			WC_COMBOBOX,
			NULL,
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_OVERLAPPED | CBS_HASSTRINGS | this->GetStylesFromType(Type),
			x, y, cx, cy,
			Parent,
			IDTOHMENU(ID),
			hInstance,
			NULL
		);
	
	if (!this->Combo)this->Success = FALSE;
	else
	{
		SendMessage(this->Combo, WM_SETFONT, reinterpret_cast<WPARAM>(this->ctrlFont), static_cast<LPARAM>(TRUE));

		if (!SetWindowSubclass(this->Combo, comboBox::comboSub, 0, reinterpret_cast<DWORD_PTR>(this)))
			this->Success = FALSE;
		else
		{
			this->Items =
				new itemCollection<iString>(
				((CollectionEvents<iString>*)this)
					);
		}
	}
}

void comboBox::setFont(HFONT font)
{
	DeleteObject(this->ctrlFont);
	this->ctrlFont = font;
	SendMessage(this->Combo, WM_SETFONT, reinterpret_cast<WPARAM>(this->ctrlFont), static_cast<LPARAM>(TRUE));
}

void comboBox::AddItem(const TCHAR * item)
{
	SendMessage(this->Combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item));
}

void comboBox::RemoveItem(int index)
{
	UNREFERENCED_PARAMETER(index);

	SendMessage(this->Combo, CB_RESETCONTENT, 0, 0);

	for (int i = 0; i < this->Items->GetCount(); i++)
	{
		iString string = this->Items->GetAt(i);
		//auto str = string.getContentReference();
		SendMessage(this->Combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(string.getContentReference()));
	}
}

LRESULT comboBox::comboSub(HWND combo, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR refData)
{
	UNREFERENCED_PARAMETER(uId);

	comboBox* pCombo = reinterpret_cast<comboBox*>(refData);
	if (pCombo != NULL)
	{
		switch (msg)
		{
		case WM_GETWNDINSTANCE:
			return reinterpret_cast<LRESULT>(pCombo);
		case WM_COMMAND:
			if (pCombo->eventHandler != nullptr)
			{
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					pCombo->selectedIndex
						= static_cast<int>(
							SendMessage(
								combo,
								CB_GETCURSEL,
								0, 0
							)
						);
					__try
					{
						pCombo->eventHandler->onComboBoxSelectionChanged(
							reinterpret_cast<cObject>(pCombo),
							pCombo->selectedIndex
						);
					}
					__except (
						GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
						? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
					{
						return DefSubclassProc(combo, msg, wParam, lParam);
					}
				}
			}
			return DefSubclassProc(combo, msg, wParam, lParam);
		case WM_DESTROY:
			pCombo->Release();
			return 0;
		default:
			break;
		}
	}
	return DefSubclassProc(combo, msg, wParam, lParam);
}

DWORD comboBox::GetStylesFromType(DWORD type)
{
	DWORD flags = 0;

	if (type == COMBOTYPE_SIMPLE)
	{
		flags = CBS_SIMPLE;
	}
	else if (type == COMBOTYPE_DROPDOWN)
	{
		flags = CBS_DROPDOWN;
	}
	else if (type == COMBOTYPE_DROPDOWNLIST)
	{
		flags = CBS_DROPDOWNLIST;
	}
	return flags;
}

