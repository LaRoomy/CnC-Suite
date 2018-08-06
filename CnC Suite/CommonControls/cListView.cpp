#include"cListView.h"
#include"..//HelperF.h"

cListView::cListView(HINSTANCE hInst)
	:hInstance(hInst),
	eventHandler(nullptr),
	selectedRowIndex(-1),
	eventblocker(false),
	listViewWnd(nullptr)
{
	this->setDefaultValues();
}

cListView::~cListView()
{
	DeleteObject(this->Properties.ctrlFont);
}

HRESULT cListView::Create(LPCTRLCREATIONSTRUCT creationals)
{
	HRESULT hr;

	if (creationals != nullptr)
	{
		this->parentWnd = creationals->parent;
		this->Properties.position.x = creationals->pos.x;
		this->Properties.position.y = creationals->pos.y;
		this->Properties.size.cx = creationals->size.cx;
		this->Properties.size.cy = creationals->size.cy;
		this->Properties.ctrlID = creationals->ctrlID;
	}

	hr = this->registerListViewHost();
	if (SUCCEEDED(hr))
	{
		this->hostWnd
			= CreateWindowEx(
				0,
				CLISTVIEWCLASS,
				nullptr,
				WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | this->Properties.additionalHostWndStyles,
				this->Properties.position.x,
				this->Properties.position.y,
				this->Properties.size.cx,
				this->Properties.size.cy,
				this->parentWnd,
				reinterpret_cast<HMENU>(this->Properties.ctrlID),
				this->hInstance,
				reinterpret_cast<LPVOID>(this)
			);

		hr = (this->hostWnd != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			RECT rc;
			GetClientRect(this->hostWnd, &rc);

			INITCOMMONCONTROLSEX icex;
			icex.dwICC = ICC_LISTVIEW_CLASSES;
			InitCommonControlsEx(&icex);

			this->listViewWnd
				= CreateWindowEx(
					0,
					WC_LISTVIEW,
					nullptr,
					WS_CHILD | WS_VISIBLE | this->Properties.additionalListviewStyles,
					this->Properties.padding,
					this->Properties.padding,
					rc.right - (2 * this->Properties.padding),
					rc.bottom - (2 * this->Properties.padding),
					this->hostWnd,
					nullptr,
					this->hInstance,
					nullptr
				);

			hr = (this->listViewWnd != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				if (this->Properties.additionalExtendedListviewStyles != 0)
				{
					ListView_SetExtendedListViewStyleEx(this->listViewWnd, 0, this->Properties.additionalExtendedListviewStyles);
				}
				hr = this->initListviewColumns();
				if (SUCCEEDED(hr))
				{
					ListView_SetBkColor(this->listViewWnd, this->Properties.background);
					ListView_SetTextBkColor(this->listViewWnd, this->Properties.background);
					ListView_SetTextColor(this->listViewWnd, this->Properties.textcolor);
				}
				SendMessage(
					this->listViewWnd,
					WM_SETFONT,
					reinterpret_cast<WPARAM>(this->Properties.ctrlFont),
					static_cast<LPARAM>(TRUE)
				);
			}
		}
	}
	return hr;
}

void cListView::setEventHandler(listViewEventSink * handler)
{
	this->eventHandler = handler;
}

void cListView::setColors(COLORREF bkgnd, COLORREF textc)
{
	this->Properties.background = bkgnd;
	this->Properties.textcolor = textc;
}

void cListView::setParent(HWND _parent)
{
	this->parentWnd = _parent;
}

void cListView::setColumnDefinitions(int numcolumns, itemCollection<iString>* columnnames, itemCollection<int>* columnwidths, int columntextalignment)
{
	this->Properties.numColumns = numcolumns;
	this->columnNames = *columnnames;
	this->columnWidths = *columnwidths;
	this->Properties.columnTextAlignment = columntextalignment;
}

void cListView::setPadding(int padding)
{
	this->Properties.padding = padding;
}

void cListView::setAdditonalHostWindowStyles(DWORD styles)
{
	this->Properties.additionalHostWndStyles = styles;
}

void cListView::setAdditonalListViewStyles(DWORD styles, DWORD extendedStyles)
{
	this->Properties.additionalListviewStyles = styles;
	this->Properties.additionalExtendedListviewStyles = extendedStyles;
}

void cListView::setFont(HFONT font)
{
	if ((this->Properties.ctrlFont != nullptr) && (font != nullptr))
	{
		DeleteObject(this->Properties.ctrlFont);
		this->Properties.ctrlFont = font;

		if (this->listViewWnd != nullptr)
		{
			SendMessage(
				this->listViewWnd,
				WM_SETFONT,
				reinterpret_cast<WPARAM>(this->Properties.ctrlFont),
				static_cast<LPARAM>(TRUE)
			);
		}
	}
}

listViewItem cListView::getItemAtIndex(int index)
{
	auto nItems = this->lvItems.GetCount();
	if (nItems > 0)
	{
		if ((index >= 0) && (index < nItems))
			return this->lvItems.GetAt(index);
	}
	return listViewItem(0);
}

void cListView::setSelectedRow(int index)
{
	if (index < this->lvItems.GetCount())
	{
		if (index == LV_SELECTHIGHEST)
			index = this->lvItems.GetCount() - 1;

		UINT state;

		if (index < 0)
			state = 0;
		else
			state = LVIS_FOCUSED | LVIS_SELECTED;

		ListView_SetItemState(
			this->listViewWnd,
			index,
			state,
			0x000F);

		this->selectedRowIndex = index;
	}
}

void cListView::addItem(listViewItem& item)
{
	this->lvItems.AddItem(item);
	this->insertLastItemToView();
}

void cListView::updateItem(int index, listViewItem& item)
{
	this->lvItems.ReplaceAt(index, item);
	this->reloadData(true);
}

void cListView::deleteItemAt(int index)
{
	this->lvItems.RemoveAt(index);
	this->reloadData(false);
}

void cListView::deleteSelection()
{
	// handle multiple selection ??

	if (this->selectedRowIndex >= 0)
	{
		this->deleteItemAt(this->selectedRowIndex);
		this->selectedRowIndex = -1;
	}
}

void cListView::reloadData(bool keepSelection)
{
	int selIndeX = this->selectedRowIndex;

	ListView_DeleteAllItems(this->listViewWnd);

	auto cnt = this->lvItems.GetCount();
	for (int i = 0; i < cnt; i++)
	{
		this->insertItemFromIndex(i);
	}

	// VERIFY!!!!  ???

	if (keepSelection)
	{
		this->activateEventBlocker();
		this->setSelectedRow(selIndeX);
	}
}

void cListView::clear()
{
	this->lvItems.Clear();
	ListView_DeleteAllItems(this->listViewWnd);
}

void cListView::activateEventBlocker()
{
	this->eventblocker = true;
}

void cListView::deactivateEventBlocker()
{
	this->eventblocker = false;
}

HRESULT cListView::registerListViewHost()
{
	HRESULT hr = S_OK;
	WNDCLASSEX wcx;

	if (GetClassInfoEx(this->hInstance, CLISTVIEWCLASS, &wcx) == 0)
	{
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = sizeof(LONG_PTR);
		wcx.hbrBackground = nullptr;
		wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcx.hInstance = this->hInstance;
		wcx.lpfnWndProc = cListView::lvHostProc;
		wcx.hIcon = nullptr;
		wcx.hIconSm = nullptr;
		wcx.lpszClassName = CLISTVIEWCLASS;
		wcx.lpszMenuName = nullptr;

		hr = (RegisterClassEx(&wcx) == 0) ? E_FAIL : S_OK;
	}
	return hr;
}

HRESULT cListView::initListviewColumns()
{
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for (int i = 0; i < this->Properties.numColumns; i++)
	{
		lvc.iSubItem = i;
		lvc.pszText = nullptr;

		CopyStringToPtr(this->columnNames.GetAt(i).getContentReference(), &lvc.pszText);
		
		// set specific column width
		if (this->columnWidths.GetCount() > i)
			lvc.cx = this->columnWidths.GetAt(i);
		else
			lvc.cx = 100;

		lvc.fmt = this->Properties.columnTextAlignment;

		if (ListView_InsertColumn(this->listViewWnd, this->Properties.numColumns, &lvc) == -1)
			return E_FAIL;

		delete[] lvc.pszText;
	}
	return S_OK;
}

LRESULT cListView::lvHostProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	cListView* clv = nullptr;

	if (message == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		clv = reinterpret_cast<cListView*>(pcr->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(clv));
		return 1;
	}
	else
	{
		clv = reinterpret_cast<cListView*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (clv != nullptr)
		{
			switch (message)
			{
			case WM_NOTIFY:
				return clv->onNotify(lParam);
			case WM_DESTROY:
				SafeRelease(&clv);
				return 0;
			case WM_PAINT:
				return clv->onPaint(hWnd);
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			default:
				break;
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

LRESULT cListView::onPaint(HWND hWnd)
{
	HDC hdc;
	RECT rc;
	PAINTSTRUCT ps;

	GetClientRect(hWnd, &rc);

	hdc = BeginPaint(hWnd, &ps);
	if (hdc)
	{
		HBRUSH brush = CreateSolidBrush(this->Properties.background);
		if (brush)
		{
			FillRect(hdc, &rc, brush);

			DeleteObject(brush);
		}
		EndPaint(hWnd, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT cListView::onNotify(LPARAM lParam)
{
	int result = 0;
	NMLVDISPINFO* dispInfo = nullptr;
	LPNMITEMACTIVATE itemActivate = nullptr;
	LPNMLISTVIEW plistView = nullptr;

	switch (((LPNMHDR)lParam)->code)
	{
	case LVN_GETDISPINFO:
		dispInfo = reinterpret_cast<NMLVDISPINFO*>(lParam);
		if (dispInfo != nullptr)
		{
			auto lItem = this->lvItems.GetAt(dispInfo->item.iItem);
			if (lItem.isValid())
			{
				StringCbCopy(dispInfo->item.pszText, sizeof(TCHAR)* STRSAFE_MAX_CCH, lItem.getDataInRow(dispInfo->item.iSubItem).GetData());
			}
		}
		break;
	case NM_CLICK:
		itemActivate = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
		if (itemActivate != nullptr)
		{
			//this->selectedRowIndex = itemActivate->iItem;

			//if (this->eventHandler != nullptr)
			//{
			//	this->eventHandler->onListviewItemSelected(
			//		reinterpret_cast<cObject>(this),
			//		this->selectedRowIndex
			//	);
			//}

			//show_integer(itemActivate->iItem, itemActivate->iSubItem);
		}
		break;
	case LVN_ITEMCHANGED:
		plistView = reinterpret_cast<LPNMLISTVIEW>(lParam);
		if (plistView != nullptr)
		{
			if (plistView->uNewState & LVIS_SELECTED)
			{
				this->selectedRowIndex = plistView->iItem;

				if (!this->eventblocker)
				{
					if (this->eventHandler != nullptr)
					{
						this->eventHandler->onListviewItemSelected(
							reinterpret_cast<cObject>(this),
							this->selectedRowIndex
						);
					}
				}
				else
				{
					this->eventblocker = false;
				}
			}
			else
			{
				// item unselected
				if (this->selectedRowIndex == plistView->iItem)
				{
					this->selectedRowIndex = -1;

					if (!this->eventblocker)
					{
						if (this->eventHandler != nullptr)
						{
							this->eventHandler->onListviewItemSelected(
								reinterpret_cast<cObject>(this),
								this->selectedRowIndex
							);
						}
					}
					else
					{
						this->eventblocker = false;
					}
				}
			}
		}
		break;
	default:
		break;
	}
	return static_cast<LRESULT>(result);
}

void cListView::setDefaultValues()
{
	// TODO ???
	this->Properties.padding = 0;


	this->Properties.additionalExtendedListviewStyles = 0;
	this->Properties.additionalHostWndStyles = 0;
	this->Properties.additionalListviewStyles = 0;
	this->Properties.ctrlFont =
		CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, APPLICATION_PRIMARY_FONT);
}

void cListView::insertLastItemToView()
{
	LVITEM lvi;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.iSubItem = 0;
	lvi.iItem = this->lvItems.GetCount() - 1;

	ListView_InsertItem(this->listViewWnd, &lvi);
}

void cListView::insertItemFromIndex(int index)
{
	LVITEM lvi;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.iSubItem = 0;
	lvi.iItem = index;

	ListView_InsertItem(this->listViewWnd, &lvi);
}
