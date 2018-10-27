#include "history.h"
#include "BasicFPO.h"
#include "HelperF.h"
#include "AppPath.h"

HistoryItem::HistoryItem()
	: historyAction(HistoryItem::NOT_DEFINED), langID(LANG_ENGLISH)
{
}

HistoryItem::HistoryItem(const HistoryItem & hi)
	: historyAction(HistoryItem::NOT_DEFINED)
{
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
}

HistoryItem::~HistoryItem()
{}

void HistoryItem::Clear()
{
	this->path.Clear();
	this->displayName.Clear();
	this->historyAction = HistoryItem::NOT_DEFINED;
}

void HistoryItem::FromString(const wchar_t* stringRepresentation)
{
	if (stringRepresentation != nullptr)
	{
		auto maxChar = (DWORD)_lengthOfString(stringRepresentation);
		DWORD i = 0, j = 0;
		TCHAR hAction[65] = { 0 };
		TCHAR dateTime[32] = { 0 };

		// at first count the length of the path
		while (stringRepresentation[i] != L'|')
		{
			if (stringRepresentation[i] == L'\0')
				return;
			if (i == maxChar)
				return;

			i++;
		}

		// get the path as a segment
		CHARSCOPE cs;
		cs.startChar = 0;
		cs.endChar = i - 1;

		iString sR(stringRepresentation);
		this->path = sR.GetSegment(&cs);

		i++;

		while (stringRepresentation[i] != L'|')
		{
			if (stringRepresentation[i] == L'\0')
				return;
			if (i == maxChar)
				return;

			hAction[j] = stringRepresentation[i];

			i++;
			j++;
		}

		this->historyAction = (HistoryAction)_wtoi(hAction);

		i++;
		j = 0;

		while (stringRepresentation[i] != L'|')
		{
			if (stringRepresentation[i] == L'\0')
				break;
			if (i == maxChar)
				break;

			dateTime[j] = stringRepresentation[i];

			i++;
			j++;
		}
		
		this->lastOpenedTime.FromString(dateTime);
		this->setTimes();
		this->setDisplayName();
	}
}

void HistoryItem::setTimes()
{
	auto bfpo = CreateBasicFPO();
	if (bfpo != nullptr)
	{
		SYSTEMTIME crTime, lwTime;

		auto result = 
			bfpo->GetFileTimes(
				this->path.GetData(),
				&crTime,
				nullptr,
				&lwTime
			);
		if (result)
		{
			this->creationTime.SetTime(&crTime);
			this->lastWriteTime.SetTime(&lwTime);
		}
		SafeRelease(&bfpo);
	}
}

void HistoryItem::setDisplayName()
{
	this->displayName.Clear();

	TCHAR* fName = nullptr;

	if (GetFilenameOutOfPath(
		this->path.GetData(),
		&fName, TRUE)
		== TRUE)
	{
		this->displayName = fName;
		SafeDeleteArray(&fName);
	}
}

void HistoryItem::generateStringRepresentation()
{
	this->representation.Replace(this->path);
	this->representation += L"|";
	this->representation += (DWORD)this->historyAction;
	this->representation += L"|";
	this->representation += this->lastOpenedTime.ToString();
}

History::History()
	:filePath(nullptr), isReady(false), canDelete(true)
{
	this->historyList.SetEventHandler(
		dynamic_cast<IConversionProtocol*>(this)
	);
	this->historyList.SetEndOfLineFormat(
		EditorContentManager::ENDOFLINE_FORMAT_CRLF
	);
}

History::~History()
{
	SafeDeleteArray(&this->filePath);
}

void History::Clear()
{
	this->historyList.Clear();
	this->isReady = false;
	this->canDelete = true;
}

int History::GetItemCount()
{
	auto cItems = this->historyList.GetLineCount();
	if (cItems == 1)
		return 0;
	else
		return cItems;
}

HistoryItem & History::GetHistoryItemAt(int index)
{
	this->curItem.Clear();
	this->curItem.SetLangID(
		(LANGID)getCurrentAppLanguage()
	);
	this->curItem.FromString(
		this->historyList.GetLine(index)
	);
	return curItem;
}

bool History::ToFile(LPCTSTR path)
{
	auto res = (path != nullptr) ? true : false;
	if (res)
	{
		res =
			(CopyStringToPtr(path, &this->filePath) == TRUE)
			? true : false;
		if (res)
		{
			// start the buffer creation ...
			this->canDelete = false;
			this->historyList.PrepareContentForSubsequentUsage();

			// ->> continue in the textbuffer-ready handler ...
		}
	}
	return res;
}

bool History::FromFile(LPCTSTR path)
{
	auto bfpo = CreateBasicFPO();
	auto res =
		(bfpo != nullptr)
		? true : false;

	if(res)
	{
		res =
			(bfpo->CheckForFileExist(path) == TRUE)
			? true : false;

		if (res)
		{
			TCHAR* content = nullptr;

			res =
				(bfpo->LoadBufferFmFileAsUtf8(&content, path) == TRUE)
				? true : false;

			if (res)
			{
				this->historyList.Clear();
				this->historyList.SetContentWithoutCopy(content);
				// NOTE: do not delete the content buffer(because it is used by the historyList Object)
			}
		}
		SafeRelease(&bfpo);
	}
	return res;
}

void History::CollectionComplete(cObject sender)
{
	//reinterpret_cast<EditorContentManager*>(sender)
	//	->InsertLineAt(
	//		L"CNCSUITE:HISTORY FORMAT:UTF8",
	//		0
	//	);
	this->isReady = true;
}

void History::TextBufferReady(cObject sender)
{
	if (this->filePath != nullptr)
	{
		auto data =
			reinterpret_cast<EditorContentManager*>(sender)
				->GetContent();

		if (data != nullptr)
		{
			auto bfpo = CreateBasicFPO();
			if (bfpo != nullptr)
			{
				bfpo->SaveBufferToFileAsUtf8(data, this->filePath);
				
				SafeRelease(&bfpo);
			}
		}
		SafeDeleteArray(&this->filePath);
	}
	this->canDelete = true;
}

UIHistory::UIHistory()
	: hHistoryEvent(nullptr),
	hInstance(nullptr),
	historyWindow(nullptr),
	parent(nullptr),
	windowID(0),
	currentIndexPos(0),
	textColor(RGB(255,255,255)),
	backgroundColor(RGB(0,0,0)),
	backgroundBrush(nullptr),
	outlineBrush(nullptr),
	outlinePen(nullptr),
	trackingEvent(false),
	isWndOpen(false),
	currentSelectedIndex(-1),
	canReset(false),
	itemBrush(nullptr),
	textAccentColor(RGB(220,220,220)),
	selectedItemBrush(nullptr),
	outlineColor(RGB(255,255,255)),
	verticalScrollbar(nullptr),
	redrawScrollbar(false)
{
	//                    ||
	//                   _||_
	//                   \  /
	// DO NOT DELETE!!!!! \/		???? do delete!

	//AppPath appPath;
	//auto historyPath =
	//	appPath.Get(PATHID_FILE_HISTORY);

	//this->historyData.FromFile(
	//	historyPath.GetData()
	//);


	// TEMP!!!!		//////////////////////////////////////////

	//HistoryItem item;
	//item.SetLangID(
	//	getCurrentAppLanguage()
	//);
	//item.SetActionType(HistoryItem::FILE_OPENED);
	//item.SetItemPath(L"C:\\Users\\hans-\\Documents\\CnC Suite\\Projects\\Welland\\Deckeleinsätze\\Deckeleinsatz(A.44385-9a)\\Welland Deckeleinsatz ( 1 ).cnc3");
	//item.SetLastOpenedTime();

	//HistoryItem item2;
	//item2.SetLangID(
	//	getCurrentAppLanguage()
	//);
	//item2.SetActionType(HistoryItem::FILE_OPENED);
	//item2.SetItemPath(L"C:\\this\\is\\another\\test.cnc3");
	//item2.SetLastOpenedTime();

	//for (int i = 0; i < 30; i++)
	//{
	//	if ((i % 2) == 0)
	//		this->historyData.AddHistoryItem(item);
	//	else
	//		this->historyData.AddHistoryItem(item2);
	//}

	AppPath appPath;
	auto path = appPath.Get(PATHID_FILE_HISTORY);

	auto hFound =
		this->historyData.FromFile(
			path.GetData()
		);

	if (hFound)
	{
		// race conditions ?? access violations ??

		int ix = 0;

		while (!this->historyData.IsReady()) {
			Sleep(20);
			ix++;

			if (ix > 500)// 20 x 500 = 10000ms timeout!
				break;
		}
		this->currentIndexPos = this->historyData.GetItemCount() - 1;
	}

	//this->historyData.ToFile(
	//	path.GetData()
	//);

	// END TEMP !!! ///////////////////////////////////////////



	this->nameFont = CreateScaledFont(20, FW_MEDIUM, APPLICATION_PRIMARY_FONT);
	this->dateTimeFont = CreateScaledFont(14, FW_MEDIUM, APPLICATION_PRIMARY_FONT);
	this->pathFont = CreateScaledFont(13, FW_MEDIUM, APPLICATION_PRIMARY_FONT);

	SetRect(&this->selectedArea, 0, 0, 0, 0);
}

UIHistory::~UIHistory()
{
	DeleteObject(this->backgroundBrush);
	DeleteObject(this->outlineBrush);
	DeleteObject(this->outlinePen);
	DeleteObject(this->nameFont);
	DeleteObject(this->dateTimeFont);
	DeleteObject(this->pathFont);
	DeleteObject(this->itemBrush);
	DeleteObject(this->selectedItemBrush);
}

const WCHAR* UIHistory::HISTORY_WINDOW_CLASS = L"CNCSUITEHISTORYWNDCLASS";

HRESULT UIHistory::ShowHistoryWindow(LPCTRLCREATIONSTRUCT pccs)
{
	if (this->isWndOpen)
	{
		return S_OK;
	}

	auto hr =
		(pccs != nullptr)
		? S_OK : E_INVALIDARG;

	if (SUCCEEDED(hr))
	{
		this->hInstance = pccs->hInst;
		this->windowID = pccs->ctrlID;
		this->parent = pccs->parent;

		GetApplicationStyleInformation(&this->styleInfo);

		hr = this->registerClass();
		if (SUCCEEDED(hr))
		{
			this->historyWindow =
				CreateWindow(
					UIHistory::HISTORY_WINDOW_CLASS,
					nullptr,
					WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
					pccs->pos.x,
					pccs->pos.y,
					pccs->size.cx,
					pccs->size.cy,
					pccs->parent,
					IDTOHMENU(pccs->ctrlID),
					pccs->hInst,
					reinterpret_cast<LPVOID>(this)
				);

			hr =
				(this->historyWindow != nullptr)
				? S_OK : E_HANDLE;

			if (SUCCEEDED(hr))
			{
				hr = this->createControls();
				if (SUCCEEDED(hr))
				{
					hr = this->createListing();
					if (SUCCEEDED(hr))
					{
						ShowWindow(this->historyWindow, SW_SHOW);
						UpdateWindow(this->historyWindow);
						this->isWndOpen = true;
					}
				}
			}
		}
	}
	return hr;
}

void UIHistory::CloseHistoryWindow()
{
	DestroyWindow(this->historyWindow);
	this->historyWindow = nullptr;
	this->hInstance = nullptr;
	this->windowID = 0;
	this->parent = nullptr;
	this->selectedArea = { 0,0,0,0 };
	this->canReset = false;
	this->currentIndexPos = this->historyData.GetItemCount() - 1;
	this->currentSelectedIndex = -1;
	this->mousePosY = -1;
	this->verticalScrollbar = nullptr;

	// cancel leave event ??
}

void UIHistory::Save()
{
	AppPath appPath;

	auto fPath =
		appPath.Get(PATHID_FILE_HISTORY);

	if (fPath.succeeded())
	{
		this->historyData.ToFile(
			fPath.GetData()
		);
	}
}

void UIHistory::SetColors(COLORREF BackgroundColor, COLORREF ItemColor, COLORREF SelectedItemColor, COLORREF TextColor, COLORREF AccentTextColor, COLORREF OutlineColor)
{
	this->textColor = TextColor;
	this->textAccentColor = AccentTextColor;
	this->backgroundColor = BackgroundColor;
	this->outlineColor = OutlineColor;

	DeleteObject(this->backgroundBrush);
	this->backgroundBrush = CreateSolidBrush(BackgroundColor);
	DeleteObject(this->outlineBrush);
	this->outlineBrush = CreateSolidBrush(OutlineColor);
	DeleteObject(this->outlinePen);
	this->outlinePen = CreatePen(PS_SOLID, 1, OutlineColor);
	DeleteObject(this->itemBrush);
	this->itemBrush = CreateSolidBrush(ItemColor);
	DeleteObject(this->selectedItemBrush);
	this->selectedItemBrush = CreateSolidBrush(SelectedItemColor);
}

void UIHistory::OnFilesystemModification(LPFILESYSTEMOBJECT fso)
{
	if (fso != nullptr)
	{
		switch (fso->FileSystemOperation)
		{
		case FSO_ITEM_RENAMED:
			this->onItemRenamed(fso);
			break;
		case FSO_ITEM_MOVED:
			break;
		case FSO_ITEM_CREATED:
			break;
		case FSO_ITEM_DELETED:
			break;
		default:
			break;
		}
		this->Save();
	}
}

void UIHistory::onCustomButtonClick(cObject sender, CTRLID ID)
{
	switch (ID)
	{
	case BID_QUIT:
		this->onQuit();
		break;
	default:
		break;
	}
}

void UIHistory::OnThumbTrack(cObject sender, int absoluteTrackPos)
{
	auto sBar =
		reinterpret_cast<CSScrollbar*>(sender);

	sBar->UpdateThumb(absoluteTrackPos);

	auto pos = sBar->GetScrollPosition();
	if (pos >= 0)
	{
		auto cItems = this->historyData.GetItemCount();

		auto index =
			((int)(pos / this->GetLineSize(0)) - 1);

		if (index < cItems && index >= 0)
		{
			index = (cItems - 1) - index;
			if (index >= 0)
			{
				this->currentIndexPos = index;
				this->Update();
			}
		}
	}
}

HRESULT UIHistory::registerClass()
{
	HRESULT hr = S_OK;
	WNDCLASSEX wcex;

	if (GetClassInfoEx(this->hInstance, UIHistory::HISTORY_WINDOW_CLASS, &wcex) == 0)
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = UIHistory::historyProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = this->hInstance;
		wcex.hIcon = nullptr;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = UIHistory::HISTORY_WINDOW_CLASS;
		wcex.hIconSm = nullptr;

		hr =
			(RegisterClassEx(&wcex) != 0)
			? S_OK : E_FAIL;
	}
	return hr;
}

HRESULT UIHistory::createControls()
{
	RECT rc;
	GetClientRect(this->historyWindow, &rc);

	POINT pt;
	pt.x = rc.right - DPIScale(25);
	pt.y = 0;
	SIZE sz;
	sz.cx = DPIScale(25);
	sz.cy = DPIScale(25);

	auto closeButton =
		new CustomButton(this->historyWindow, BUTTONMODE_ICON, &pt, &sz, BID_QUIT, this->hInstance);

	auto hr =
		(closeButton != nullptr)
		? S_OK : E_POINTER;

	if (SUCCEEDED(hr))
	{
		closeButton->setAppearance_onlyIcon(IDI_CLOSEBUTTON_MARKED, DPIScale(24));
		closeButton->setColors(this->backgroundColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
		closeButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
		closeButton->setBorder(TRUE, this->styleInfo.OutlineColor);
		
		hr = closeButton->Create();
		if (SUCCEEDED(hr))
		{
			this->verticalScrollbar = new CSScrollbar(this->hInstance);

			hr =
				(this->verticalScrollbar != nullptr)
				? S_OK : E_NOINTERFACE;

			if (SUCCEEDED(hr))
			{
				RECT rcList;
				this->getListArea(&rcList);
				pt.x = rcList.right;
				pt.y = rcList.top;
				sz.cx = DPIScale(16);
				sz.cy = rcList.bottom - rcList.top;

				this->verticalScrollbar->SetScrollbarType(
					CSScrollbar::SCROLLBARTYPE_VERTICAL
				);
				this->verticalScrollbar->SetEventHandler(
					dynamic_cast<IScrollEventProtocol*>(this)
				);
				this->verticalScrollbar->SetScrollRange(
					this->getContentHeight()
					- rcList.bottom
				);
				this->verticalScrollbar->SetDefaultThickness(
					DPIScale(16)
				);
				this->verticalScrollbar->SetImages(IDI_SYS_SCROLLARROW_DOWN, IDI_SYS_SCROLLARROW_UP, 16);
				this->verticalScrollbar->SetStyleImages(IDI_SYS_SCROLLARROW_DOWN_HGL, IDI_SYS_SCROLLARROW_DOWN_PRS, IDI_SYS_SCROLLARROW_UP_HGL, IDI_SYS_SCROLLARROW_UP_PRS);
				this->verticalScrollbar->SetDisabledImages(IDI_SYS_SCROLLARROW_DOWN_DSBL, IDI_SYS_SCROLLARROW_UP_DSBL);
				//this->verticalScrollbar->SetColors(this->styleInfo.TabColor, )

				hr = this->verticalScrollbar->Create(this->historyWindow, &pt, &sz);
				if (SUCCEEDED(hr))
				{
					// ...
				}
			}
		}
	}
	return hr;
}

HRESULT UIHistory::createListing()
{
	return S_OK;
}

LRESULT UIHistory::historyProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UIHistory* pUIhistory = nullptr;

	if (message == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		if (pcr != nullptr)
		{
			pUIhistory = reinterpret_cast<UIHistory*>(pcr->lpCreateParams);
			if (pUIhistory != nullptr)
			{
				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pUIhistory);
			}
		}
		return 1;
	}
	else
	{
		pUIhistory =
			reinterpret_cast<UIHistory*>(
				GetWindowLongPtr(hWnd, GWLP_USERDATA)
				);
		if (pUIhistory != nullptr)
		{
			switch (message)
			{
			case WM_PAINT:
				return pUIhistory->onPaint(hWnd);
			case WM_SIZE:
				return pUIhistory->onSize();
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_MOUSEMOVE:
				return pUIhistory->onMouseMove(lParam);
			case WM_MOUSEWHEEL:
				return pUIhistory->onMouseWheel(wParam);
			case WM_LBUTTONUP:
				return pUIhistory->onLButtonUp(lParam);
			case WM_LBUTTONDOWN:
				return pUIhistory->onLButtonDown(lParam);
			case WM_MOUSELEAVE:
				return pUIhistory->onMouseLeave();
			case WM_DESTROY:
				return pUIhistory->onDestroy();
			default:
				break;
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

LRESULT UIHistory::onPaint(HWND hWnd)
{
	HDC hdc;
	RECT rc;
	PAINTSTRUCT ps;
	int curHeight = 0;
	auto curIndex = this->currentIndexPos;
	auto itemHeight = this->getItemHeight();

	hdc = BeginPaint(hWnd, &ps);
	if (hdc)
	{
		HDC offScreenDC = CreateCompatibleDC(hdc);
		if (offScreenDC)
		{
			GetClientRect(this->historyWindow, &rc);
			//rc.right -= DPIScale(16);

			HBITMAP offScreenBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			if (offScreenBmp)
			{
				RECT itemRect, rcListArea;
				this->getListArea(&rcListArea);
				curHeight = rcListArea.top;

				SetRect(
					&itemRect,
					0, curHeight,
					rcListArea.right,
					itemHeight + curHeight
				);

				auto originBmp =
					SelectObject(offScreenDC, offScreenBmp);

				// erase the bitmap
				FillRect(offScreenDC, &rc, this->backgroundBrush);

				// draw items
				while (curHeight < rcListArea.bottom)// - itemHeight)) // only rc.bottom
				{
					itemRect.top = curHeight;
					itemRect.bottom = curHeight + itemHeight;

					if (!this->drawHistoryItem(offScreenDC, curIndex, &itemRect))
						break;

					curIndex--;
					curHeight += itemHeight;
				}

				BitBlt(hdc, rc.left, rc.top, rc.right, rc.bottom, offScreenDC, 0, 0, SRCCOPY);

				SelectObject(offScreenDC, originBmp);
				DeleteObject(offScreenBmp);
			}
			DeleteDC(offScreenDC);
		}
		EndPaint(hWnd, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT UIHistory::onMouseMove(LPARAM lParam)
{
	this->mousePosY = GET_Y_LPARAM(lParam);

	if (!this->trackingEvent)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = this->historyWindow;

		TrackMouseEvent(&tme);
		this->trackingEvent = true;
	}

	if (!isPointInRect(2, this->mousePosY, &this->selectedArea))
	{
		RECT rcList;
		this->getListArea(&rcList);

		if (this->isOverValidItem(this->mousePosY))
		{
			RedrawWindow(this->historyWindow, &rcList, nullptr, RDW_NOERASE | RDW_INVALIDATE | RDW_UPDATENOW);
			this->canReset = true;
		}
		else
		{
			if (this->canReset)
			{
				this->selectedArea = { 0,0,0,0 };
				this->currentSelectedIndex = -1;

				RedrawWindow(this->historyWindow, &rcList, nullptr, RDW_NOERASE | RDW_INVALIDATE | RDW_UPDATENOW);
				this->canReset = false;
			}
		}
	}

	return static_cast<LRESULT>(0);
}

LRESULT UIHistory::onMouseWheel(WPARAM wParam)
{
	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	if (zDelta > 0)
	{
		// wheel minus
		if (this->itemUP())
		{
			auto pos = this->verticalScrollbar->GetScrollPosition();

			this->verticalScrollbar->SetScrollPosition(
				pos
				- this->GetLineSize(0), true
			);
		}
		else
		{
			this->verticalScrollbar->SetScrollPosition(0);
			this->redrawScrollbar = true;
		}
	}
	else if (zDelta < 0)
	{
		// wheel plus
		if (this->itemDOWN())
		{
			auto pos = this->verticalScrollbar->GetScrollPosition();

			this->verticalScrollbar->SetScrollPosition(
				pos
				+ this->GetLineSize(0), true
			);
		}
		else
			this->redrawScrollbar = true;
	}

	if (this->redrawScrollbar)
	{
		this->verticalScrollbar->SetScrollPosition(
			this->verticalScrollbar->GetScrollPosition(),
			true
		);
	}
	return static_cast<LRESULT>(0);
}

LRESULT UIHistory::onLButtonDown(LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	return static_cast<LRESULT>(0);
}

LRESULT UIHistory::onLButtonUp(LPARAM lParam)
{
	if (this->hHistoryEvent != nullptr)
	{
		auto yPos =
			GET_Y_LPARAM(lParam);

		auto index =
			this->itemIndexFromPosition(yPos);

		if ((index >= 0) && (index < this->historyData.GetItemCount()))
		{
			auto item =
				this->historyData.GetHistoryItemAt(index);

				this->hHistoryEvent->OnEntryClicked(
					reinterpret_cast<cObject>(this),
					&item
				);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT UIHistory::onMouseLeave()
{
	this->mousePosY = -1;
	this->trackingEvent = false;
	this->eraseSelectedArea();

	return static_cast<LRESULT>(0);
}

LRESULT UIHistory::onDestroy()
{
	this->isWndOpen = false;

	if (this->hHistoryEvent != nullptr)
	{
		this->hHistoryEvent->OnWindowClosed(
			reinterpret_cast<cObject>(this)
		);
	}
	return static_cast<LRESULT>(0);
}

LRESULT UIHistory::onSize()
{
	RECT rcList, rc;
	this->getListArea(&rcList);
	GetClientRect(this->historyWindow, &rc);

	if (this->verticalScrollbar != nullptr)
	{
		this->verticalScrollbar->Move(
			rcList.right,
			rcList.top,
			DPIScale(16),
			rcList.bottom - rcList.top
		);
	}

	auto cls_btn = GetDlgItem(this->historyWindow, BID_QUIT);
	if (cls_btn != nullptr)
	{
		MoveWindow(
			cls_btn,
			rc.right - DPIScale(25),
			0,
			DPIScale(25),
			DPIScale(25),
			TRUE
		);
	}

	return static_cast<LRESULT>(0);
}

void UIHistory::onQuit()
{
	this->CloseHistoryWindow();

	if (this->hHistoryEvent != nullptr)
	{
		this->hHistoryEvent->OnWindowClosed(reinterpret_cast<cObject>(this));
	}
}

void UIHistory::onItemRenamed(LPFILESYSTEMOBJECT fso)
{
	auto cItems = this->historyData.GetItemCount();
	if (cItems > 0)
	{
		for (int i = 0; i < cItems; i++)
		{
			auto item = this->historyData.GetHistoryItemAt(i);
			auto path = item.GetItemPath();

			if (path.Equals(
				fso->OldPath.GetPathData()
				))
			{
				item.SetItemPath(
					fso->Path.GetPathData()
				);
				this->historyData.ReplaceHistoryItemAt(item, i);
			}
		}
	}
}

bool UIHistory::drawHistoryItem(HDC hdc, int itemIndex, LPRECT itemRect)
{
	//auto maxItems = this->historyData.GetItemCount();
	//if (itemIndex == maxItems)
	//	return false;

	if (itemIndex < 0)
		return false;


	auto item = this->historyData.GetHistoryItemAt(itemIndex);

	// if this item is selected -> draw selected color
	if((this->mousePosY < itemRect->bottom) && (this->mousePosY > itemRect->top))
	{
		this->currentSelectedIndex = itemIndex;

		FillRect(hdc, itemRect, this->selectedItemBrush);

		// save selected rect
		CopyRect(&this->selectedArea, itemRect);		
	}
	else
	{
		FillRect(hdc, itemRect, this->itemBrush);
	}

	// draw the frame
	auto originPen =
		SelectObject(hdc, this->outlinePen);

	MoveToEx(hdc, 0, 0, nullptr);
	LineTo(hdc, itemRect->right - 1, 0);
	LineTo(hdc, itemRect->right - 1, itemRect->bottom - 1);
	LineTo(hdc, 0, itemRect->bottom - 1);
	LineTo(hdc, 0, 0);

	SelectObject(hdc, originPen);

	// draw the item info
	auto originFont =
		SelectObject(hdc, this->nameFont);

	SIZE sz;
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, this->textColor);

	GetTextExtentPoint32(
		hdc,
		item
		.GetDisplayName()
		.GetData(),
		item
		.GetDisplayName()
		.GetLength(),
		&sz
	);

	TextOut(
		hdc,
		DPIScale(10),
		DPIScale(2) + itemRect->top,
		item.GetDisplayName().GetData(),
		item.GetDisplayName().GetLength()
	);

	if (sz.cx < (itemRect->right - DPIScale(100)))
	{
		iString completeDate(
			item
			.GetLastOpenedTime()
			.SimpleDateAsString()
		);
		completeDate += L"  ";
		completeDate +=
			item
			.GetLastOpenedTime()
			.SimpleTimeAsString();

		SelectObject(hdc, this->dateTimeFont);

		TextOut(
			hdc,
			itemRect->right - DPIScale(90),
			DPIScale(6) + itemRect->top,
			completeDate.GetData(),
			completeDate.GetLength()
		);
	}

	SelectObject(hdc, this->pathFont);

	SetTextColor(
		hdc,
		this->textAccentColor
	);

	TextOut(
		hdc,
		DPIScale(15),
		DPIScale(24) + itemRect->top,
		item.GetItemPath().GetData(),
		item.GetItemPath().GetLength()
	);

	SelectObject(hdc, originFont);

	return true;
}

void UIHistory::getListArea(LPRECT prc)
{
	// retrieve the window rect excluding the area used by the other controls	
	GetClientRect(this->historyWindow, prc);

	prc->top = DPIScale(25);			// tool area
	prc->right -= DPIScale(16);			// scrollbar
}

int UIHistory::getItemHeight()
{
	return DPIScale(40);
}

int UIHistory::getContentHeight()
{
	RECT rcList;
	this->getListArea(&rcList);

	return
		this->historyData.GetItemCount()
		* this->getItemHeight()
		+ ((rcList.bottom - rcList.top) / 4);
}

int UIHistory::itemIndexFromPosition(int Ypos)
{
	RECT rcList;
	this->getListArea(&rcList);

	// get the position without the upper tool-area
	auto relativePos = Ypos - rcList.top;
	// get the index-position of the mouse within the visible items
	auto relativeIndex =
		((int)(relativePos /
			this->getItemHeight())
		);

	//old:   !
	// add the relative index to the index of the first visible item-index
	//return this->currentIndexPos + relativeIndex;

	// sub the relative index from the currentPos
	return this->currentIndexPos - relativeIndex;
}

void UIHistory::eraseSelectedArea()
{
	RedrawWindow(this->historyWindow, &this->selectedArea, nullptr, RDW_NOERASE | RDW_INVALIDATE);
	SetRect(&this->selectedArea, 0, 0, 0, 0);
}

bool UIHistory::isOverValidItem(int pos_y)
{
	RECT rcListArea;
	this->getListArea(&rcListArea);

	auto itemCount = this->historyData.GetItemCount();
	auto itemHeight = this->getItemHeight();

	if ((pos_y > ((itemCount * itemHeight) + rcListArea.top))
		|| (pos_y < rcListArea.top))
		return false;
	else
		return true;
}

bool UIHistory::itemUP()
{
	auto nItems = this->historyData.GetItemCount();
	if (nItems > 0)
	{
		if (this->currentIndexPos < (nItems - 1))
		{
			this->currentIndexPos++;
			RedrawWindow(this->historyWindow, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW | RDW_NOCHILDREN);
			reinterpret_cast<CustomButton*>(
				CUSTOMBUTTONCLASS_FROMID(this->historyWindow, BID_QUIT)
				)->Update();
			return true;
		}		
	}
	return false;
}

bool UIHistory::itemDOWN()
{
	if (this->currentIndexPos > 0)
	{
		RECT rcList;
		this->getListArea(&rcList);

		auto remHeight =
			this->getItemHeight()
			* this->currentIndexPos;

		auto availableHeight = rcList.bottom - rcList.top;

		if (remHeight > (availableHeight - availableHeight / 4))
		{
			this->currentIndexPos--;
			RedrawWindow(this->historyWindow, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW | RDW_NOCHILDREN);
			reinterpret_cast<CustomButton*>(
				CUSTOMBUTTONCLASS_FROMID(this->historyWindow, BID_QUIT)
				)->Update();

			return true;
		}		
	}
	return false;
}

void UIHistory::updatePage()
{
	auto pos = this->verticalScrollbar->GetScrollPosition();
	if (pos >= 0)
	{
		auto cItems = this->historyData.GetItemCount();

		auto index =
			((int)(pos / this->GetLineSize(0)) - 1);

		_BELOW_ZERO_SETTOZERO(index);

		if (index < cItems)
		{
			index = (cItems - 1) - index;
			if (index >= 0)
			{
				this->currentIndexPos = index;
				this->Update();
			}
		}
	}
}

void UIHistory::UpdateScrollbar()
{
	if (this->verticalScrollbar != nullptr)
	{
		RECT rcList;
		this->getListArea(&rcList);

		this->verticalScrollbar->SetScrollRange(
			this->getContentHeight()
			- rcList.bottom
		);
	}
}
