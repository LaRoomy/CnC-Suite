#include"CustomPopUpMenu.h"
#include"..//HelperF.h"

CustomPopUpMenu::CustomPopUpMenu(HINSTANCE hInst)
	: eventListener(nullptr),
	hInstance(hInst),
	numEntries(0),
	hBackground(nullptr),
	ctrlFont(nullptr),
	buttonWasClicked(false),
	useExclusionArea(false),
	thisWnd(nullptr),
	parent(nullptr),
	inputCaptured(FALSE)
{
	this->setHooks(hInst);
}

CustomPopUpMenu::CustomPopUpMenu(HINSTANCE hInst, customPopUpMenuEventSink * _eventListener)
	: eventListener(_eventListener),
	hInstance(hInst),
	numEntries(0),
	hBackground(nullptr),
	ctrlFont(nullptr),
	buttonWasClicked(false)
{
	this->setHooks(hInst);
}

CustomPopUpMenu::~CustomPopUpMenu()
{
	for (int i = 0; i < this->mEntries.GetCount(); i++)
	{
		DestroyWindow(
			GetDlgItem(this->thisWnd, this->mEntries.GetAt(i).ENTRY_ID)
		);
	}
	if (this->hBackground != nullptr)
		DeleteObject(this->hBackground);
	if (this->ctrlFont != nullptr)
	{
		DeleteObject(this->ctrlFont);
		this->ctrlFont = nullptr;
	}
	UnhookWindowsHookEx(this->mouseHook);
}

void CustomPopUpMenu::setEventListener(customPopUpMenuEventSink * _eventListener)
{
	this->eventListener = _eventListener;
}

void CustomPopUpMenu::setControlFont(HFONT font)
{
	if (this->ctrlFont != nullptr)
		DeleteObject(this->ctrlFont);

	this->ctrlFont = font;
}

void CustomPopUpMenu::setCloseExclusionArea(LPRECT area)
{
	if (area != nullptr)
	{
		CopyRect(&this->exclusionArea, area);
		this->useExclusionArea = true;
	}
}

void CustomPopUpMenu::addMenuEntry(MenuEntry * entry)
{
	this->mEntries.AddItem(entry);
}

HRESULT CustomPopUpMenu::Show(HWND _parent_, LPPOINT position, int width, COLORREF background)
{
	HRESULT hr = S_OK;
	int x = 0, y = 0;
	this->parent = _parent_;

	if (position == nullptr)
	{
		POINT pt;
		// get mouse position
		hr = GetCursorPos(&pt) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			x = pt.x;
			y = pt.y;
		}
	}
	else
	{
		x = position->x;
		y = position->y;
	}
	if (SUCCEEDED(hr))
	{
		int absoluteHeight =
			this->mEntries.GetCount()
			* this->calculateItemHeigth();

		this->hBackground = CreateSolidBrush(background);

		WNDCLASSEX wcx;
		if (!GetClassInfoEx(this->hInstance, CPOPUPMENU_CLASS, &wcx))
		{
			wcx.cbSize = sizeof(WNDCLASSEX);
			wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
			wcx.lpfnWndProc = CustomPopUpMenu::wndProc;
			wcx.cbClsExtra = 0;
			wcx.cbWndExtra = sizeof(LONG_PTR);
			wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wcx.hInstance = this->hInstance;
			wcx.hIcon = nullptr;
			wcx.hbrBackground = this->hBackground;
			wcx.lpszClassName = CPOPUPMENU_CLASS;
			wcx.hIconSm = nullptr;
			wcx.lpszMenuName = nullptr;

			hr = (RegisterClassEx(&wcx) == 0) ? E_FAIL : S_OK;
		}
		if (SUCCEEDED(hr))
		{
			POINT pt_test;
			pt_test.x = x;
			pt_test.y = y;

			AsureWindowVisibilityFromPoint(&pt_test, width, absoluteHeight);

			x = pt_test.x;
			y = pt_test.y;

			this->thisWnd
				= CreateWindowEx(
					WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
					CPOPUPMENU_CLASS, nullptr,
					WS_POPUP | WS_BORDER,
					x, y, width, absoluteHeight,
					this->parent, nullptr, this->hInstance,
					reinterpret_cast<LPVOID>(this)
				);

			hr = (this->thisWnd != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				hr = this->createContent();
				if (SUCCEEDED(hr))
				{
					ShowWindow(this->thisWnd, SW_SHOW);
					UpdateWindow(this->thisWnd);

					SetFocus(this->parent);
				}
			}
		}
	}
	return hr;
}

void CustomPopUpMenu::Close()
{
	ReleaseCapture();
	this->inputCaptured = FALSE;

	DestroyWindow(this->thisWnd);
	this->Release();
}

HRESULT CustomPopUpMenu::createContent()
{
	HRESULT hr = S_OK;
	RECT rc;
	DWORD buttonMode = 0;
	POINT pt;
	SIZE sz;
	auto height = this->calculateItemHeigth();

	GetClientRect(this->thisWnd, &rc);

	for (int i = 0; i < this->mEntries.GetCount(); i++)
	{
		auto entry = this->mEntries.GetAt(i);

		pt.x = 0;
		pt.y = height *  i;
		sz.cx = rc.right;
		sz.cy = height;

		buttonMode = (entry.entryStyle == MENU_ENTRY_ICON_TEXT) ? BUTTONMODE_ICONTEXT : BUTTONMODE_TEXT;
		
		auto button = new CustomButton(this->thisWnd, buttonMode, &pt, &sz, entry.ENTRY_ID, this->hInstance);

		hr = (button != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			button->setEventListener(dynamic_cast<customButtonEventSink*>(this));
			button->setAlignment(BAL_LEFT);
			button->setConstraints(5, 10);
			button->setColors(entry.backgroundcolor, entry.highlightedcolor, entry.pressedcolor);
			button->setTextColor(entry.textcolor);
			if (entry.entryStyle == MENU_ENTRY_ICON_TEXT)
				button->setAppearance_IconText(entry.iconID, entry.squaresize, entry.text);
			else
				button->setAppearance_onlyText(entry.text, FALSE);
			if (this->ctrlFont != nullptr)
				button->setFont(this->ctrlFont);

			button->setBorder(TRUE, RGB(100, 100, 100));

			hr = button->Create();
			if (FAILED(hr))
				break;
		}
	}
	return hr;
}

LRESULT CustomPopUpMenu::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CustomPopUpMenu* menu = nullptr;

	if (msg == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		menu = reinterpret_cast<CustomPopUpMenu*>(pcr->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(menu));
		return 1;
	}
	else
	{
		menu = reinterpret_cast<CustomPopUpMenu*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (menu != nullptr)
		{
			switch (msg)
			{
			case WM_DESTROY:
				return 0;
			default:
				return DefWindowProc(hWnd, msg, wParam, lParam);
			}
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

LRESULT CustomPopUpMenu::MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	else
	{
		LPMOUSEHOOKSTRUCT pmh = reinterpret_cast<LPMOUSEHOOKSTRUCT>(lParam);
		if (pmh != nullptr)
		{
			HWND refWnd = FindWindow(CPOPUPMENU_CLASS, nullptr);
			if (refWnd)
			{
				auto _this = reinterpret_cast<CustomPopUpMenu*>(GetWindowLongPtr(refWnd, GWLP_USERDATA));
				if (_this != nullptr)
				{
					bool exe = true;

					RECT rc;
					GetWindowRect(refWnd, &rc);

					if (_this->useExclusionArea && isPointInRect(pmh->pt.x, pmh->pt.y, &_this->exclusionArea))
					{
						// if the exclusion rect is set and the mouse is inside of the exclusion-rect -> prevent further execution
						exe = false;
					}

					if(exe)
					{
						if ((pmh->pt.x < rc.left) || (pmh->pt.x > rc.right) || (pmh->pt.y < rc.top) || (pmh->pt.y > rc.bottom))
						{
							if (!_this->inputCaptured)
							{
								SetCapture(_this->thisWnd);
								_this->inputCaptured = TRUE;
							}


							SHORT keystate1, keystate2;
							keystate1 = GetAsyncKeyState(VK_LBUTTON);
							keystate2 = GetAsyncKeyState(VK_RBUTTON);

							if ((keystate1 & 0x8000) || (keystate2 & 0x8000))
							{
								if (!_this->buttonWasClicked)
									_this->Close();
							}
						}
						else
						{
							if (_this->inputCaptured)
							{
								ReleaseCapture();
								_this->inputCaptured = FALSE;
							}
						}
					}
				}
			}
		}
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
}

int CustomPopUpMenu::calculateItemHeigth()
{
	int height = 22;
	int newHeight = 0;

	for (int i = 0; i < this->mEntries.GetCount(); i++)
	{
		auto entry = this->mEntries.GetAt(i);
		newHeight = entry.getExpectedItemHeight();
		if (newHeight > height)
			height = newHeight;
	}
	return height;
}

void CustomPopUpMenu::onButtonClick(CTRLID ID)
{
	ReleaseCapture();
	this->inputCaptured = FALSE;
	DestroyWindow(this->thisWnd);

	if (this->eventListener != nullptr)
	{
		this->buttonWasClicked = true;
		this->eventListener->onPopUpMenuButtonClicked(ID);
	}
	this->Release();
}

void CustomPopUpMenu::setHooks(HINSTANCE hInst)
{
	this->threadID = GetCurrentThreadId();
	this->mouseHook = SetWindowsHookEx(WH_MOUSE, CustomPopUpMenu::MouseProc, hInst, this->threadID);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MenuEntry::MenuEntry()
	: iconID(0),
	squaresize(0),
	backgroundcolor(RGB(240,240,240)),
	textcolor(RGB(0,0,0)),
	highlightedcolor(RGB(0, 120, 180)),
	pressedcolor(RGB(0, 100, 150)),
	itemHeight(22),
	entryStyle(MENU_ENTRY_PURE_TEXT),
	ENTRY_ID(0)
{}

MenuEntry::~MenuEntry()
{
	//if (this->font != nullptr)
	//{
	//	DeleteObject(this->font);
	//	this->font = nullptr;
	//}
}

void MenuEntry::setID(int entryID)
{
	this->ENTRY_ID = entryID;
}

void MenuEntry::setText(iString* _text)
{
	this->text = *_text;
}

void MenuEntry::setText(LPCTSTR text_in)
{
	iString Text_(text_in);
	this->setText(&Text_);
}

void MenuEntry::setItemheight(int height)
{
	this->itemHeight = height;
}

void MenuEntry::setColors(COLORREF background, COLORREF Textcolor, COLORREF highlighted, COLORREF pressed)
{
	this->backgroundcolor = background;
	this->textcolor = Textcolor;
	this->highlightedcolor = highlighted;
	this->pressedcolor = pressed;
}

void MenuEntry::setIcon(int iconId, int squareSize)
{
	this->iconID = iconId;
	this->squaresize = squareSize;
	this->entryStyle = MENU_ENTRY_ICON_TEXT;	
}

void MenuEntry::setBackground(COLORREF background)
{
	this->backgroundcolor = background;
}
