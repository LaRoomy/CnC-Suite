#include"CustomButton.h"
#include"..//HelperF.h"

CustomButton::CustomButton(HWND Parent, DWORD _buttonMode, LPPOINT position, LPSIZE size, int ctrlID, HINSTANCE hInst)
	: eventSink(nullptr)
{
	this->initButtonProperty();
	this->buttonMode = _buttonMode;
	this->subclassID = static_cast<UINT_PTR>(ctrlID);

	this->hInstance = hInst;
	this->buttonProperty.alignment = BAL_CENTER;
	this->buttonProperty.parent = Parent;
	this->buttonProperty.ctrlID = ctrlID;
	this->buttonProperty.padding = 5;
	this->buttonProperty.spacing = 5;
	this->buttonProperty.x = position->x;
	this->buttonProperty.y = position->y;
	this->buttonProperty.width = size->cx;
	this->buttonProperty.height = size->cy;
	this->buttonProperty.font = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Segoe UI\0");
	this->buttonProperty.textColor = RGB(0, 0, 0);
	this->buttonProperty.highlightTextColor = RGB(127, 127, 127);
	this->buttonProperty.disabledTextColor = RGB(150, 150, 150);

	this->buttonProperty.Bkgnd = CreateSolidBrush(RGB(240, 240, 240));
	this->buttonProperty.mouseover = CreateSolidBrush(RGB(180, 240, 250));
	this->buttonProperty.pressed = CreateSolidBrush(RGB(160, 220, 230));
	this->buttonProperty.border = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
	this->buttonProperty.disabled = CreateSolidBrush(RGB(100, 100, 100));

	this->buttonControl.touchdownlock_active = false;
	this->buttonControl.isEnabled = TRUE;
	this->buttonControl.sizeToContext = FALSE;
	this->buttonControl.parentIsComposited = FALSE;
	this->buttonControl.mouseAnimation = true;
	this->buttonControl.mouseIsOver = FALSE;
	this->buttonControl.buttonIsPressed = FALSE;
	this->buttonControl.isTouchedDown = FALSE;
	this->buttonControl.textHighlightOnSelection = FALSE;

	this->buttonProperty.borderParts.bottom = true;
	this->buttonProperty.borderParts.top = true;
	this->buttonProperty.borderParts.left = true;
	this->buttonProperty.borderParts.right = true;

	this->threadId = GetCurrentThreadId();
}

CustomButton::~CustomButton()
{
	if (this->buttonProperty.mouseHook != nullptr)
	{
		UnhookWindowsHookEx(this->buttonProperty.mouseHook);
	}
	if (this->buttonProperty.button != nullptr)
	{
		RemoveWindowSubclass(this->buttonProperty.button, CustomButton::buttonSub, this->subclassID);
	}

	this->eventSink = nullptr;

	DeleteObject(this->buttonProperty.Bkgnd);
	DeleteObject(this->buttonProperty.mouseover);
	DeleteObject(this->buttonProperty.pressed);
	DeleteObject(this->buttonProperty.disabled);
	DeleteObject(this->buttonProperty.border);
	DeleteObject(this->buttonProperty.font);
	this->buttonProperty.font = nullptr;
}

HRESULT CustomButton::setBorder(BOOL drawBorder, COLORREF borderColor)
{
	DeleteObject(this->buttonProperty.border);
	this->buttonProperty.border = CreatePen(PS_SOLID, 1, borderColor);
	this->buttonControl.drawBorder = drawBorder;

	if (this->buttonProperty.button)
		RedrawWindow(this->buttonProperty.button, nullptr, nullptr, RDW_INVALIDATE);

	return S_OK;
}

HRESULT CustomButton::setAppearance_onlyText(iString& text, BOOL applySizeToContext)
{
	this->buttonProperty.text = text;
	this->buttonControl.sizeToContext = applySizeToContext;
	return S_OK;
}

HRESULT CustomButton::setAppearance_onlyIcon(int Icon, int sqareSize)
{
	this->buttonProperty.iconID = Icon;
	this->buttonProperty.iconSqareSize = sqareSize;

	// redraw ??

	return S_OK;
}

HRESULT CustomButton::setAppearance_IconText(int Icon, int sqareSize, iString& text)
{
	this->buttonProperty.iconID = Icon;
	this->buttonProperty.iconSqareSize = sqareSize;
	this->buttonProperty.text = text;

	// redraw ??

	return S_OK;
}

HRESULT CustomButton::setAppearance_Bitmap(int bitmap, int width, int height)
{
	UNREFERENCED_PARAMETER(bitmap);
	UNREFERENCED_PARAMETER(width);
	UNREFERENCED_PARAMETER(height);
	return E_NOTIMPL;
}

HRESULT CustomButton::setColors(COLORREF normalBackground, COLORREF onMouseOverBkgnd, COLORREF onPressedBkgnd)
{
	DeleteObject(this->buttonProperty.Bkgnd);
	DeleteObject(this->buttonProperty.mouseover);
	DeleteObject(this->buttonProperty.pressed);

	this->buttonProperty.Bkgnd = CreateSolidBrush(normalBackground);
	this->buttonProperty.mouseover = CreateSolidBrush(onMouseOverBkgnd);
	this->buttonProperty.pressed = CreateSolidBrush(onPressedBkgnd);

	if (this->buttonProperty.button)
		RedrawWindow(this->buttonProperty.button, nullptr, nullptr, RDW_INVALIDATE);

	return S_OK;
}

HRESULT CustomButton::setFont(HFONT font)
{
	if (font != nullptr)
	{
		DeleteObject(this->buttonProperty.font);
		this->buttonProperty.font = font;

		if (this->buttonProperty.button)
			RedrawWindow(this->buttonProperty.button, nullptr, nullptr, RDW_INVALIDATE);

		return S_OK;
	}
	else
		return E_FAIL;
}

HRESULT CustomButton::setTextColor(COLORREF color)
{
	this->buttonProperty.textColor = color;

	if (this->buttonProperty.button)
		RedrawWindow(this->buttonProperty.button, nullptr, nullptr, RDW_INVALIDATE);

	return S_OK;
}

HRESULT CustomButton::setTooltip(iString& tooltip)
{
	HRESULT hr;

	this->buttonProperty.tooltip = tooltip;

	hr = ((this->buttonProperty.parent == NULL) || (this->buttonProperty.button == NULL))
		? E_FAIL : S_OK;

	if (SUCCEEDED(hr))
	{
		this->buttonProperty.hwndToolTip = CreateWindowEx(
			0,
			TOOLTIPS_CLASS,
			NULL,
			WS_POPUP | TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			this->buttonProperty.parent, NULL, this->hInstance, NULL);

		hr = (this->buttonProperty.hwndToolTip != NULL) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			TOOLINFO ti = { 0 };
			ti.cbSize = sizeof(ti);
			ti.hwnd = this->buttonProperty.parent;
			ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
			ti.uId = (UINT_PTR)this->buttonProperty.button;
			ti.lpszText = tooltip.getContentReference();

			SendMessage(this->buttonProperty.hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
		}
	}
	return hr;
}

HRESULT CustomButton::setDisabledIcon(int icon)
{
	this->buttonProperty.disabledIcon = icon;
	return S_OK;
}

void CustomButton::setDisabledColor(COLORREF disabledBackground)
{
	if (this->buttonProperty.disabled != nullptr)
		DeleteObject(this->buttonProperty.disabled);

	this->buttonProperty.disabled = CreateSolidBrush(disabledBackground);
}

void CustomButton::setDisabledColors(COLORREF bkgndDisabled, COLORREF textDisabled)
{
	if (this->buttonProperty.disabled != nullptr)
		DeleteObject(this->buttonProperty.disabled);

	this->buttonProperty.disabled = CreateSolidBrush(bkgndDisabled);
	this->buttonProperty.disabledTextColor = textDisabled;
}

HRESULT CustomButton::setAlignment(DWORD align)
{
	if ((align != BAL_CENTER) && (align != BAL_RIGHT) && (align != BAL_LEFT))
	{
		return E_INVALIDARG;
	}
	else
	{
		this->buttonProperty.alignment = align;
		return S_OK;
	}
}

void CustomButton::setConstraints(int padding, int spacing)
{
	this->buttonProperty.padding = padding;
	this->buttonProperty.spacing = spacing;
}

void CustomButton::setTouchdownLock(bool holdButtonOnTouchdown)
{
	this->buttonControl.touchdownlock_active = holdButtonOnTouchdown;

	if (!holdButtonOnTouchdown)
		this->buttonDownRelease();
}

void CustomButton::buttonDownRelease()
{
	this->buttonControl.isTouchedDown = FALSE;

	if (this->buttonProperty.button != nullptr)
		RedrawWindow(this->buttonProperty.button, NULL, NULL, RDW_NOERASE | RDW_NOCHILDREN | RDW_INVALIDATE);
}

void CustomButton::setButtonDown()
{
	this->buttonControl.isTouchedDown = TRUE;

	if (this->buttonProperty.button != nullptr)
		RedrawWindow(this->buttonProperty.button, NULL, NULL, RDW_NOERASE | RDW_NOCHILDREN | RDW_INVALIDATE);
}

void CustomButton::setEnabledState(bool state)
{
	if (state)
		this->buttonControl.isEnabled = TRUE;
	else
	{
		this->buttonControl.isEnabled = FALSE;
		this->ResetControlParameters();
	}

	if (this->buttonProperty.button != nullptr)
	{
		RedrawWindow(this->buttonProperty.button, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
	}
}

void CustomButton::workWithCompositedWindows(bool wwcw)
{
	this->buttonControl.parentIsComposited = wwcw ? TRUE : FALSE;

	if (!wwcw)
	{
		if (this->buttonProperty.mouseHook != nullptr)
		{
			UnhookWindowsHookEx(this->buttonProperty.mouseHook);
		}
	}
}

void CustomButton::setContentOffset(int x_offset, int y_offset)
{
	// only implemented for buttonmode-text at the moment!

	this->buttonProperty.ptOffset.x = x_offset;
	this->buttonProperty.ptOffset.y = y_offset;

	// redraw??
}

void CustomButton::setPartialBorderParts(bool left, bool top, bool right, bool bottom)
{
	this->buttonProperty.borderParts.left = left;
	this->buttonProperty.borderParts.top = top;
	this->buttonProperty.borderParts.right = right;
	this->buttonProperty.borderParts.bottom = bottom;
}

void CustomButton::setMouseAnimation(bool animate)
{
	this->buttonControl.mouseAnimation = animate;

	//this->buttonControl.mouseIsOver = FALSE;
	//this->buttonControl.buttonIsPressed = FALSE;
	if(this->buttonProperty.button != nullptr)
		RedrawWindow(this->buttonProperty.button, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
}

void CustomButton::setTextHighlight(BOOL highlight, COLORREF highlightColor)
{
	this->buttonProperty.highlightTextColor = highlightColor;
	this->buttonControl.textHighlightOnSelection = highlight;

	if (this->buttonProperty.button != nullptr)
		RedrawWindow(this->buttonProperty.button, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
}

CTRLID CustomButton::getID()
{
	return this->buttonProperty.ctrlID;
}

HRESULT CustomButton::setEventListener(customButtonEventSink * _eventSink)
{
	if (_eventSink != nullptr)
	{
		this->eventSink = _eventSink;
		return S_OK;
	}
	else
		return E_FAIL;
}

HRESULT CustomButton::Create()
{
	HRESULT hr;

	hr = (this->buttonProperty.button == nullptr) ? S_OK : E_FAIL;
	if(SUCCEEDED(hr))
	{
		this->buttonProperty.button = CreateWindow(
			L"BUTTON",
			nullptr,
			WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_CLIPSIBLINGS,
			this->buttonProperty.x, this->buttonProperty.y,
			this->buttonProperty.width,
			this->buttonProperty.height,
			this->buttonProperty.parent,
			(HMENU)this->buttonProperty.ctrlID,
			this->hInstance, nullptr);

		hr = (this->buttonProperty.button) ? S_OK : E_FAIL;
		if(SUCCEEDED(hr))
		{
			hr = SetWindowSubclass(this->buttonProperty.button, CustomButton::buttonSub, this->subclassID, reinterpret_cast<DWORD_PTR>(this)) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				if (this->buttonControl.parentIsComposited)
				{
					this->buttonProperty.mouseHook =
						SetWindowsHookEx(
							WH_MOUSE,
							CustomButton::mouseProc,
							this->hInstance,
							GetCurrentThreadId()
						);

					hr = (this->buttonProperty.mouseHook != nullptr) ? S_OK : E_FAIL;
				}
				if (SUCCEEDED(hr))
				{
					if ((this->buttonProperty.tooltip.GetLength() > 0) && (this->buttonProperty.hwndToolTip == NULL))
					{
						iString ttip(this->buttonProperty.tooltip.GetData());

						hr = this->setTooltip(ttip);
					}


					// ...
				}
			}
		}
	}
	return hr;
}

void CustomButton::Update()
{
	RedrawWindow(this->buttonProperty.button, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
}

void CustomButton::updateDimensions(LPPOINT pos, LPSIZE size)
{
	this->buttonProperty.x = pos->x;
	this->buttonProperty.y = pos->y;
	this->buttonProperty.width = size->cx;
	this->buttonProperty.height = size->cy;

	if (this->buttonProperty.button != nullptr)
	{
		MoveWindow(
			this->buttonProperty.button,
			pos->x,
			pos->y,
			size->cx,
			size->cy,
			TRUE
		);
	}
}

LRESULT CustomButton::buttonSub(HWND button, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR refData)
{
	UNREFERENCED_PARAMETER(uID);

	auto cButton = reinterpret_cast<CustomButton*>(refData);
	if (cButton != nullptr)
	{
		switch (message)
		{
		case WM_ERASEBKGND:
			return static_cast<LRESULT>(TRUE);
		case WM_PAINT:
			return cButton->onPaint();
		case WM_SIZE:
			return cButton->onSize();
		case WM_MOUSEMOVE:
			return cButton->onMouseMove(button);
		case WM_MOUSELEAVE:
			return cButton->onMouseLeave(button);
		case WM_LBUTTONDOWN:
			return cButton->onLButtonDown(button);
		case WM_LBUTTONUP:
			return cButton->onLButtonUp(button);
		case WM_GETWNDINSTANCE:
			return reinterpret_cast<LRESULT>(cButton);
		case WM_DESTROY:
			SafeRelease(&cButton);
			return 0;
		default:
			break;
		}
	}
	return DefSubclassProc(button, message, wParam, lParam);
}

LRESULT CustomButton::mouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	else
	{
		auto mhs = reinterpret_cast<LPMOUSEHOOKSTRUCT>(lParam);
		if (mhs != nullptr)
		{
			static HWND mouseON = nullptr;

			if (mouseON == nullptr)
			{
				TCHAR classNameBuffer[256] = { 0 };

				auto c = GetClassName(mhs->hwnd, classNameBuffer, 255);
				if (c > 0)
				{
					if (CompareStringsNoCase(L"Button", classNameBuffer))
					{
						auto instance = SendMessage(mhs->hwnd, WM_GETWNDINSTANCE, 0, 0);
						if (instance != 0)
						{
							CustomButton* _this = reinterpret_cast<CustomButton*>(instance);
							if (_this != nullptr)
							{
								if (_this->isCompositionControl())
								{
									mouseON = mhs->hwnd;

									switch (wParam)
									{
									case WM_LBUTTONDOWN:
										_this->onLButtonDown(mhs->hwnd);
										break;
									case WM_LBUTTONUP:
										_this->onLButtonUp(mhs->hwnd);
										break;
									case WM_MOUSEMOVE:
										_this->onMouseMove(mhs->hwnd);
										break;
									default:
										break;
									}
								}
							}
						}
					}
				}
			}

			if ((mhs->hwnd != mouseON) && (mouseON != nullptr))
			{
				auto instance = SendMessage(mouseON, WM_GETWNDINSTANCE, 0, 0);
				if (instance != 0)
				{
					CustomButton* _this = reinterpret_cast<CustomButton*>(instance);
					if (_this != nullptr)
					{
						if (_this->isCompositionControl())
						{
							_this->onMouseLeave(mouseON);
						}
					}
				}
				mouseON = nullptr;
			}
		}
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
}

LRESULT CustomButton::onMouseMove(HWND button)
{
	if (this->buttonControl.isEnabled)
	{
		if (!this->buttonControl.mouseIsOver)
		{
			this->buttonControl.mouseIsOver = TRUE;

			if(this->buttonControl.mouseAnimation)
				RedrawWindow(button, NULL, NULL, RDW_NOERASE | RDW_INVALIDATE);

			if (this->buttonControl.parentIsComposited)
			{
				// use the hook!!!
			}
			else
			{
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = this->buttonProperty.button;
				tme.dwHoverTime = HOVER_DEFAULT;

				TrackMouseEvent(&tme);
			}
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CustomButton::onLButtonDown(HWND button)
{
	if (this->buttonControl.isEnabled)
	{
		if (!this->buttonControl.buttonIsPressed)
		{
			this->buttonControl.buttonIsPressed = TRUE;

			if(this->buttonControl.mouseAnimation)
				RedrawWindow(button, NULL, NULL, RDW_NOERASE | RDW_INVALIDATE);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CustomButton::onLButtonUp(HWND button)
{
	if (this->buttonControl.isEnabled)
	{
		this->buttonControl.buttonIsPressed = FALSE;

		if (this->buttonControl.touchdownlock_active)
		{
			this->buttonControl.isTouchedDown = TRUE;
		}

		if (this->eventSink != nullptr)
		{
			__try
			{
				this->eventSink->onCustomButtonClick(
					reinterpret_cast<cObject>(this), 
					this->buttonProperty.ctrlID
				);
			}
			__except (
				GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
				? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
			{
				return static_cast<LRESULT>(0);
			}
		}

		RedrawWindow(button, NULL, NULL, RDW_NOERASE | RDW_INVALIDATE);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CustomButton::onMouseLeave(HWND button)
{
	if (this->buttonControl.isEnabled)
	{
		if (this->buttonControl.mouseIsOver || this->buttonControl.buttonIsPressed)
		{
			this->buttonControl.mouseIsOver = FALSE;
			this->buttonControl.buttonIsPressed = FALSE;

			RedrawWindow(button, NULL, NULL, RDW_NOERASE | RDW_INVALIDATE);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CustomButton::onPaint()
{
	HDC hdc;
	RECT rc;
	HGDIOBJ origin;
	PAINTSTRUCT ps;

	hdc = BeginPaint(this->buttonProperty.button, &ps);
	if (hdc)
	{
		GetClientRect(this->buttonProperty.button, &rc);

		if (this->buttonControl.isEnabled)
		{
			if (this->buttonControl.mouseAnimation)
			{
				if (this->buttonControl.mouseIsOver && !this->buttonControl.buttonIsPressed)
				{
					if (!this->buttonControl.isTouchedDown)
						FillRect(hdc, &rc, this->buttonProperty.mouseover);
				}
				else if (this->buttonControl.buttonIsPressed)
				{
					FillRect(hdc, &rc, this->buttonProperty.pressed);
				}
				else
				{
					if ((this->buttonControl.touchdownlock_active) && (this->buttonControl.isTouchedDown))
						FillRect(hdc, &rc, this->buttonProperty.pressed);
					else
						FillRect(hdc, &rc, this->buttonProperty.Bkgnd);
				}
			}
			else
			{
				FillRect(hdc, &rc, this->buttonProperty.Bkgnd);
			}
		}
		else
		{
			FillRect(hdc, &rc, this->buttonProperty.disabled);
		}

		// draw border
		if (this->buttonControl.drawBorder)
		{
			origin = SelectObject(hdc, this->buttonProperty.border);

			if (this->buttonProperty.borderParts.top)
			{
				MoveToEx(hdc, 0, 0, nullptr);
				LineTo(hdc, rc.right - 1, 0);
			}
			if (this->buttonProperty.borderParts.right)
			{
				MoveToEx(hdc, rc.right - 1, 0, nullptr);
				LineTo(hdc, rc.right - 1, rc.bottom - 1);
			}
			if (this->buttonProperty.borderParts.bottom)
			{
				MoveToEx(hdc, rc.right - 1, rc.bottom - 1, nullptr);
				LineTo(hdc, 0, rc.bottom - 1);
			}
			if (this->buttonProperty.borderParts.left)
			{
				MoveToEx(hdc, 0, rc.bottom - 1, nullptr);
				LineTo(hdc, 0, 0);
			}
			SelectObject(hdc, origin);
		}

		// set text / icon / bitmap
		switch (this->buttonMode)
		{
		case BUTTONMODE_TEXT:
			this->drawText(hdc, &rc);
			break;
		case BUTTONMODE_BITMAP:
			break;
		case BUTTONMODE_ICON:
			this->drawIcon(hdc, nullptr, TRUE);
			break;
		case BUTTONMODE_ICONTEXT:
			this->drawIconText(hdc, nullptr, this->buttonProperty.alignment);
			break;
		default:
			break;
		}

		EndPaint(this->buttonProperty.button, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CustomButton::onSize()
{
	RECT rc;
	GetClientRect(this->buttonProperty.button, &rc);

	this->buttonProperty.width = rc.right;
	this->buttonProperty.height = rc.bottom;

	return static_cast<LRESULT>(0);
}

void CustomButton::initButtonProperty()
{
	this->buttonProperty.bitmap = nullptr;
	this->buttonProperty.icon = nullptr;
	this->buttonProperty.button = nullptr;
	this->buttonProperty.parent = nullptr;
	this->buttonProperty.mouseHook = nullptr;
	

	this->buttonProperty.ptOffset.x = 0;
	this->buttonProperty.ptOffset.y = 0;
}

void CustomButton::drawText(HDC hdc, LPRECT rc)
{
	POINT pos = { 0,0 };
	SIZE szText;
	int outputLength = this->buttonProperty.text.GetLength();

	HGDIOBJ original = SelectObject(hdc, this->buttonProperty.font);
	SetBkMode(hdc, TRANSPARENT);

	if (this->buttonControl.isEnabled)
	{
		if (this->buttonControl.mouseAnimation)
		{
			if (this->buttonControl.textHighlightOnSelection)
			{
				if (this->buttonControl.mouseIsOver)
				{
					if (!this->buttonControl.isTouchedDown)
					{
						SetTextColor(hdc, this->buttonProperty.highlightTextColor);
					}
					else
						SetTextColor(hdc, this->buttonProperty.textColor);
				}
				else
					SetTextColor(hdc, this->buttonProperty.textColor);
			}
			else
				SetTextColor(hdc, this->buttonProperty.textColor);
		}
		else
			SetTextColor(hdc, this->buttonProperty.textColor);
	}
	else
		SetTextColor(hdc, this->buttonProperty.disabledTextColor);

	GetTextExtentPoint32(hdc, this->buttonProperty.text.GetData(), this->buttonProperty.text.GetLength(), &szText);

	// check if the buttontext is to wide for the buttonsize
	if (szText.cx > (this->buttonProperty.width - (this->buttonProperty.padding*2)))
	{
		// reduce the text-counter
		while (szText.cx > (this->buttonProperty.width - (this->buttonProperty.padding*2)))
		{
			outputLength--;
			GetTextExtentPoint32(hdc, this->buttonProperty.text.GetData(), outputLength, &szText);
		}
		// if the tooltip is empty -> set buttontext as tooltip
		if (this->buttonProperty.tooltip.GetLength() == 0)
		{
			this->setTooltip(this->buttonProperty.text);
		}
	}

	pos.y = (rc->bottom - szText.cy) / 2;

	// offset y
	if (this->buttonProperty.ptOffset.y != 0)
		pos.y += this->buttonProperty.ptOffset.y;

	switch (this->buttonProperty.alignment)
	{
	case BAL_LEFT:
		pos.x = this->buttonProperty.padding;
		break;
	case BAL_CENTER:
		pos.x = (rc->right - szText.cx) / 2;
		break;
	case BAL_RIGHT:
		pos.x = rc->right - (szText.cx - this->buttonProperty.padding);
		break;
	default:
		pos.x = 0;
		break;
	}
	// offset x
	if (this->buttonProperty.ptOffset.x != 0)
		pos.x += this->buttonProperty.ptOffset.x;

	TextOut(
		hdc,
		pos.x,
		pos.y,
		this->buttonProperty.text.GetData(),
		outputLength
	);
	SelectObject(hdc, original);
}

void CustomButton::drawIcon(HDC hdc, LPRECT rcDraw, BOOL centerImage)
{
	HICON icon =
		(HICON)LoadImage(
			this->hInstance,
			this->buttonControl.isEnabled ? MAKEINTRESOURCE(this->buttonProperty.iconID) : MAKEINTRESOURCE(this->buttonProperty.disabledIcon),
			IMAGE_ICON,
			this->buttonProperty.iconSqareSize,
			this->buttonProperty.iconSqareSize,
			LR_DEFAULTCOLOR);

	if (icon != nullptr)
	{
		POINT pos = { 0,0 };

		if (centerImage)
		{
			// calculate center
			pos.x = (this->buttonProperty.width - this->buttonProperty.iconSqareSize) / 2;
			pos.y = (this->buttonProperty.height - this->buttonProperty.iconSqareSize) / 2;
		}
		else
		{
			if (rcDraw != nullptr)
			{
				pos.x = rcDraw->left;
				pos.y = rcDraw->top;
			}
		}

		DrawIconEx(hdc, pos.x, pos.y, icon, this->buttonProperty.iconSqareSize, this->buttonProperty.iconSqareSize, 0, nullptr, DI_NORMAL);

		DestroyIcon(icon);
	}
}

void CustomButton::drawIconText(HDC hdc, LPRECT rcDraw, DWORD alignment)
{
	HICON icon =
		(HICON)LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(this->buttonProperty.iconID),
			IMAGE_ICON,
			this->buttonProperty.iconSqareSize,
			this->buttonProperty.iconSqareSize,
			LR_DEFAULTCOLOR);

	if (icon != nullptr)
	{
		POINT pos = { 0,0 };
		SIZE szText;

		HGDIOBJ original = SelectObject(hdc, this->buttonProperty.font);
		SetBkMode(hdc, TRANSPARENT);

		if (this->buttonControl.isEnabled)
		{
			if (this->buttonControl.mouseAnimation)
			{
				if (this->buttonControl.textHighlightOnSelection)
				{
					if (this->buttonControl.mouseIsOver)
					{
						if (!this->buttonControl.isTouchedDown)
						{
							SetTextColor(hdc, this->buttonProperty.highlightTextColor);
						}
						else
							SetTextColor(hdc, this->buttonProperty.textColor);
					}
					else
						SetTextColor(hdc, this->buttonProperty.textColor);
				}
				else
					SetTextColor(hdc, this->buttonProperty.textColor);
			}
			else
				SetTextColor(hdc, this->buttonProperty.textColor);
		}
		else
			SetTextColor(hdc, this->buttonProperty.disabledTextColor);

		GetTextExtentPoint32(hdc, this->buttonProperty.text.GetData(), this->buttonProperty.text.GetLength(), &szText);

		int centersizeY = (this->buttonProperty.iconSqareSize - szText.cy) / 2;
		int contentLength = this->buttonProperty.iconSqareSize + 5 + szText.cx;
		int contentHeight;

		if (this->buttonProperty.iconSqareSize > szText.cy)contentHeight = this->buttonProperty.iconSqareSize;
		else contentHeight = szText.cy;

		if (alignment == BAL_CENTER)
		{
			// calculate center
			pos.x = (this->buttonProperty.width - contentLength) / 2;
			pos.y = (this->buttonProperty.height - this->buttonProperty.iconSqareSize) / 2;
		}
		else if (alignment == BAL_LEFT)
		{
			if (rcDraw != nullptr)
			{
				pos.x = rcDraw->left;
				pos.y = rcDraw->top;
			}
			else
			{
				pos.x = this->buttonProperty.padding;
				pos.y = (this->buttonProperty.height - this->buttonProperty.iconSqareSize) / 2;
			}
		}
		else if (alignment == BAL_RIGHT)
		{
			// TODO: implement!

			//showText(L"right-alignment not implemented.");
		}
		else
		{
			if (rcDraw != nullptr)
			{
				pos.x = rcDraw->left;
				pos.y = rcDraw->top;
			}
		}

		TextOut(
			hdc,
			pos.x + this->buttonProperty.iconSqareSize + this->buttonProperty.spacing,
			pos.y + centersizeY,
			this->buttonProperty.text.GetData(),
			this->buttonProperty.text.GetLength()	);

		SelectObject(hdc, original);

		if(this->buttonControl.isEnabled)
			DrawIconEx(
				hdc,
				pos.x, pos.y,
				icon,
				this->buttonProperty.iconSqareSize,
				this->buttonProperty.iconSqareSize,
				0, nullptr,
				DI_NORMAL
			);
		else
		{
			HICON disabledIcon
				= (HICON)LoadImage(
					this->hInstance,
					MAKEINTRESOURCE(this->buttonProperty.disabledIcon),
					IMAGE_ICON,
					this->buttonProperty.iconSqareSize,
					this->buttonProperty.iconSqareSize,
					LR_DEFAULTCOLOR);

			if (disabledIcon)
			{
				DrawIconEx(
					hdc,
					pos.x, pos.y,
					disabledIcon,
					this->buttonProperty.iconSqareSize,
					this->buttonProperty.iconSqareSize,
					0, nullptr,
					DI_NORMAL
				);
			}
		}
		DestroyIcon(icon);
	}
}

void CustomButton::ResetControlParameters()
{
	this->buttonControl.buttonIsPressed = FALSE;
	this->buttonControl.mouseIsOver = FALSE;

	//RedrawWindow(this->buttonProperty.button, NULL, NULL, RDW_NOERASE | RDW_NOCHILDREN | RDW_INVALIDATE);
}
