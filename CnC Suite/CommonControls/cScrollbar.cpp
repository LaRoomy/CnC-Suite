#include"cScrollbar.h"
#include<windowsx.h>
#include"..//HelperF.h"

CSScrollbar::CSScrollbar()
	: hInstance(nullptr),
	scrollBarWnd(nullptr),
	parentWnd(nullptr),
	Position({ 0,0 }),
	Size({ 0,0 }),
	iconSquareSize(0),
	plusIcon(nullptr),
	minusIcon(nullptr),
	plusIconHgl(nullptr),
	plusIconPrs(nullptr),
	minusIconHgl(nullptr),
	minusIconPrs(nullptr),
	plusIconDisabled(nullptr),
	minusIconDisabled(nullptr),
	autoHide(false),
	scrollBarType(CSScrollbar::SCROLLBARTYPE_VERTICAL),
	defaultThickness(16),
	scrollEvents(nullptr),
	enabledState(true),
	plusButtonHover(false),
	plusButtonPressed(false),
	minusButtonHover(false),
	minusButtonPressed(false),
	thumbHover(false),
	thumbPressed(false),
	mouseleaveScheduled(false),
	trackOffset(0),
	inputCaptured(false),
	trackStopBorderSize(100),
	scrollRange(0),
	scrollPosition(0)
{
	this->bkgndBrush = CreateSolidBrush(RGB(90, 90, 90));
	this->thumbBrush = CreateSolidBrush(RGB(130, 130, 130));
	this->thumbHglBrush = CreateSolidBrush(RGB(160, 160, 160));
	this->thumbPrsBrush = CreateSolidBrush(RGB(200, 200, 200));
}

CSScrollbar::CSScrollbar(HINSTANCE hInst)
	: hInstance(hInst),
	scrollBarWnd(nullptr),
	parentWnd(nullptr),
	Position({ 0,0 }),
	Size({ 0,0 }),
	iconSquareSize(0),
	plusIcon(nullptr),
	minusIcon(nullptr),
	plusIconHgl(nullptr),
	plusIconPrs(nullptr),
	minusIconHgl(nullptr),
	minusIconPrs(nullptr),
	plusIconDisabled(nullptr),
	minusIconDisabled(nullptr),
	autoHide(false),
	scrollBarType(CSScrollbar::SCROLLBARTYPE_VERTICAL),
	defaultThickness(16),
	scrollEvents(nullptr),
	enabledState(true),
	plusButtonHover(false),
	plusButtonPressed(false),
	minusButtonHover(false),
	minusButtonPressed(false),
	thumbHover(false),
	thumbPressed(false),
	mouseleaveScheduled(false),
	trackOffset(0),
	inputCaptured(false),
	trackStopBorderSize(100),
	scrollRange(0),
	scrollPosition(0)
{
	this->bkgndBrush = CreateSolidBrush(RGB(90, 90, 90));
	this->thumbBrush = CreateSolidBrush(RGB(130, 130, 130));
	this->thumbHglBrush = CreateSolidBrush(RGB(160, 160, 160));
	this->thumbPrsBrush = CreateSolidBrush(RGB(200, 200, 200));
}

CSScrollbar::~CSScrollbar()
{
	if(this->plusIcon != nullptr)
		DestroyIcon(this->plusIcon);
	if(this->minusIcon != nullptr)
		DestroyIcon(this->minusIcon);
	if (this->plusIconHgl != nullptr)
		DestroyIcon(this->plusIconHgl);
	if (this->plusIconPrs != nullptr)
		DestroyIcon(this->plusIconPrs);
	if (this->minusIconHgl != nullptr)
		DestroyIcon(this->minusIconHgl);
	if (this->minusIconPrs != nullptr)
		DestroyIcon(this->minusIconPrs);

	DeleteObject(this->bkgndBrush);
	DeleteObject(this->thumbBrush);
	DeleteObject(this->thumbHglBrush);
	DeleteObject(this->thumbPrsBrush);
}

const WCHAR* CSScrollbar::CSSCROLLBAR_CLASS_IDENTIFIER = L"CustomScrollbarClassID3859";

HRESULT CSScrollbar::Create(HWND Parent, LPPOINT pos, LPSIZE size)
{
	auto hr = (Parent != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		this->parentWnd = Parent;

		if ((pos == nullptr) || (size == nullptr))
		{
			this->calculateDefaultSize();
		}
		else
		{
			this->Position = *pos;
			this->Size = *size;
		}

		hr = this->registerScrollClass();
		if (SUCCEEDED(hr))
		{

			this->scrollBarWnd =
				CreateWindow(
					CSScrollbar::CSSCROLLBAR_CLASS_IDENTIFIER,
					nullptr,
					WS_CHILD | WS_CLIPSIBLINGS,
					this->Position.x,
					this->Position.y,
					this->Size.cx,
					this->Size.cy,
					Parent,
					nullptr,
					this->hInstance,
					reinterpret_cast<LPVOID>(this)
				);

			hr = (this->scrollBarWnd != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				this->updateThumbRect();

				ShowWindow(this->scrollBarWnd, SW_SHOW);
				UpdateWindow(this->scrollBarWnd);
			}
		}
	}
	return hr;
}

HRESULT CSScrollbar::Create(HINSTANCE hInst, HWND Parent, LPPOINT pos, LPSIZE size)
{
	this->hInstance = hInst;
	return this->Create(Parent, pos, size);
}

bool CSScrollbar::SetScrollRange(int range)
{
	if (range < 0)
		return false;
	else
	{
		this->scrollRange = range;

		if (this->scrollPosition > range)
			this->scrollPosition = range;

		if (this->scrollBarWnd != nullptr)
		{
			this->updateThumbRect();

			if (this->scrollBarWnd != nullptr)
			{
				RedrawWindow(this->scrollBarWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW);
			}
		}
		return true;
	}
}

void CSScrollbar::SetScrollPosition(int position)
{
	if (position == SCROLLPOS_MIN)
		this->scrollPosition = 0;
	else if (position == SCROLLPOS_MAX)
		this->scrollPosition = this->scrollRange;
	else
	{
		if (position < 0)
			position = 0;
		if (position > this->scrollRange)
			position = this->scrollRange;

		this->scrollPosition = position;

		if (this->scrollBarWnd != nullptr)
		{
			this->updateThumbRect();
		}
	}
}

void CSScrollbar::SetScrollPosition(int position, bool redraw)
{
	if (position == SCROLLPOS_MIN)
		this->scrollPosition = 0;
	else if (position == SCROLLPOS_MAX)
		this->scrollPosition = this->scrollRange;
	else
	{
		if (position < 0)
			position = 0;
		if (position > this->scrollRange)
			position = this->scrollRange;

		this->scrollPosition = position;

		if (this->scrollBarWnd != nullptr)
		{
			this->updateThumbRect();

			if (redraw)
			{
				RedrawWindow(this->scrollBarWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW);
			}
		}
	}
}

void CSScrollbar::SetShowState(bool state)
{
	if (state)
		ShowWindow(this->scrollBarWnd, SW_SHOW);
	else
	{
		this->autoHide = false;
		ShowWindow(this->scrollBarWnd, SW_HIDE);
	}
}

void CSScrollbar::SetScrollbarType(ScrollBarType type)
{
	if ((type == 1) || (type == 2))
	{
		this->scrollBarType = type;
	}
}

void CSScrollbar::SetImages(int plusImageID, int minusImageID, int squareSize)
{
	if (this->plusIcon != nullptr)
		DestroyIcon(this->plusIcon);
	if (this->minusIcon != nullptr)
		DestroyIcon(this->minusIcon);

	this->iconSquareSize = squareSize;

	this->plusIcon = (HICON)
		LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(plusImageID),
			IMAGE_ICON,
			squareSize,
			squareSize,
			LR_DEFAULTCOLOR
		);
	this->minusIcon = (HICON)
		LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(minusImageID),
			IMAGE_ICON,
			squareSize,
			squareSize,
			LR_DEFAULTCOLOR
		);
}

void CSScrollbar::SetStyleImages(int plusImageHighlightID, int plusImagePressedID, int minusImageHighlightID, int minusImagePressedID)
{
	if (this->plusIconHgl != nullptr)
		DestroyIcon(this->plusIconHgl);
	if (this->plusIconPrs != nullptr)
		DestroyIcon(this->plusIconPrs);
	if (this->minusIconHgl != nullptr)
		DestroyIcon(this->minusIconHgl);
	if (this->minusIconPrs != nullptr)
		DestroyIcon(this->minusIconPrs);

	this->plusIconHgl = (HICON)
		LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(plusImageHighlightID),
			IMAGE_ICON,
			this->iconSquareSize,
			this->iconSquareSize,
			LR_DEFAULTCOLOR
		);
	this->plusIconPrs = (HICON)
		LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(plusImagePressedID),
			IMAGE_ICON,
			this->iconSquareSize,
			this->iconSquareSize,
			LR_DEFAULTCOLOR
		);
	this->minusIconHgl = (HICON)
		LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(minusImageHighlightID),
			IMAGE_ICON,
			this->iconSquareSize,
			this->iconSquareSize,
			LR_DEFAULTCOLOR
		);
	this->minusIconPrs = (HICON)
		LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(minusImagePressedID),
			IMAGE_ICON,
			this->iconSquareSize,
			this->iconSquareSize,
			LR_DEFAULTCOLOR
		);
}

void CSScrollbar::SetDisabledImages(int plusImageDisabledID, int minusImageDisabledID)
{
	if (this->plusIconDisabled != nullptr)
		DestroyIcon(this->plusIconDisabled);
	if (this->minusIconDisabled != nullptr)
		DestroyIcon(this->minusIconDisabled);

	this->plusIconDisabled = (HICON)
		LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(plusImageDisabledID),
			IMAGE_ICON,
			this->iconSquareSize,
			this->iconSquareSize,
			LR_DEFAULTCOLOR
		);
	this->minusIconDisabled = (HICON)
		LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(minusImageDisabledID),
			IMAGE_ICON,
			this->iconSquareSize,
			this->iconSquareSize,
			LR_DEFAULTCOLOR
		);
}

void CSScrollbar::SetColors(COLORREF background, COLORREF thumbColor, COLORREF thumbHighlightColor, COLORREF thumbPressedColor)
{
	DeleteObject(this->bkgndBrush);
	DeleteObject(this->thumbBrush);
	DeleteObject(this->thumbHglBrush);
	DeleteObject(this->thumbPrsBrush);

	this->bkgndBrush = CreateSolidBrush(background);
	this->thumbBrush = CreateSolidBrush(thumbColor);
	this->thumbHglBrush = CreateSolidBrush(thumbHighlightColor);
	this->thumbPrsBrush = CreateSolidBrush(thumbPressedColor);
}

void CSScrollbar::SetDefaultThickness(int thickness)
{
	if ((thickness >= 16) && (thickness <= 32))
		this->defaultThickness = thickness;
}

void CSScrollbar::Resize()
{
	if (this->scrollBarWnd != nullptr)
	{
		if (this->calculateDefaultSize())
		{
			// resize the scrollbar-window, but do not redraw
			MoveWindow(this->scrollBarWnd, this->Position.x, this->Position.y, this->Size.cx, this->Size.cy, FALSE);
			// update the thumbrect before redraw
			this->updateThumbRect();
			// now do redraw
			RedrawWindow(this->scrollBarWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW);
		}
	}
}

void CSScrollbar::UpdateThumb(int newScrollPosition)
{
	HDC hdc = GetDC(this->scrollBarWnd);
	if (hdc)
	{
		// erase the old thumb
		FillRect(hdc, &this->ThumbRect, this->bkgndBrush);

		this->SetScrollPosition(newScrollPosition, false);

		// draw the new thumb
		if (this->thumbHover)
			FillRect(hdc, &this->ThumbRect, this->thumbHglBrush);
		else if(this->thumbPressed)
			FillRect(hdc, &this->ThumbRect, this->thumbPrsBrush);
		else
			FillRect(hdc, &this->ThumbRect, this->thumbBrush);

		ReleaseDC(this->scrollBarWnd, hdc);
	}
}

void CSScrollbar::Move(int x, int y, int cx, int cy)
{
	if (MoveWindow(this->scrollBarWnd, x, y, cx, cy, TRUE))
	{
		this->Position.x = x;
		this->Position.y = y;
		this->Size.cx = cx;
		this->Size.cy = cy;
	}
}

//int CSScrollbar::GetRealScrollPos()
//{
//	RECT rc;
//	GetClientRect(this->scrollBarWnd, &rc);
//
//	auto thumbHalf = this->calculateHalfThumbLength(
//		(this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
//		? rc.bottom : rc.right
//	);
//	if (thumbHalf >= 0)
//	{
//		auto thumbMidPoint = ((this->ThumbRect.bottom - this->ThumbRect.top) / 2) + this->ThumbRect.top;
//		auto thumbMinimum = ((this->ThumbRect.bottom - this->ThumbRect.top) / 2) + this->iconSquareSize;
//
//		return (thumbMidPoint - thumbMinimum);
//	}
//	return -1;
//}

LRESULT CSScrollbar::onMouseWheel(WPARAM wParam)
{
	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	if (zDelta > 0)
	{
		if (this->scrollEvents != nullptr)
		{
			this->scrollEvents->OnMouseWheelMinus(
				reinterpret_cast<cObject>(this),
				this->scrollPosition
			);
		}
	}
	else if (zDelta < 0)
	{
		if (this->scrollEvents != nullptr) {
			this->scrollEvents->OnMouseWheelPlus(
				reinterpret_cast<cObject>(this),
				this->scrollPosition
			);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CSScrollbar::onMousemove(WPARAM wParam, LPARAM lParam)
{
	auto xPos = GET_X_LPARAM(lParam);
	auto yPos = GET_Y_LPARAM(lParam);

	bool redraw = false;
	RECT thumbActivationRect;
	this->getThumbActivationRect(&thumbActivationRect);

	if (this->inputCaptured)
	{
		// a thumb-track was started and the input was captured, so track the mouse on the whole screen
		POINT screenPos;
		RECT rcWnd, rcCtrl;

		GetCursorPos(&screenPos);
		GetWindowRect(this->scrollBarWnd, &rcWnd);

		SetRect(
			&rcCtrl,
			(this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL) ? rcWnd.left - this->trackStopBorderSize : rcWnd.left,
			(this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL) ? rcWnd.top : rcWnd.top - this->trackStopBorderSize,
			(this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL) ? rcWnd.right + this->trackStopBorderSize : rcWnd.right,
			(this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL) ? rcWnd.bottom : rcWnd.bottom + this->trackStopBorderSize
		);

		if (isPointInRect(screenPos.x, screenPos.y, &rcCtrl))
		{
			if (this->scrollEvents != nullptr)
			{
				screenPos.y -= rcWnd.top;
				screenPos.x -= rcWnd.left;

				_BELOW_ZERO_SETTOZERO(screenPos.x);
				_BELOW_ZERO_SETTOZERO(screenPos.y);
				
				this->scrollEvents->OnThumbTrack(
					reinterpret_cast<cObject>(this),
					this->calculateAbsoluteThumbPosition(
						screenPos.x + this->trackOffset,
						screenPos.y + this->trackOffset)
				);
			}
		}
	}
	else
	{
		// track mouseleave:
		if ((!this->mouseleaveScheduled) && (!this->inputCaptured))
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = this->scrollBarWnd;
			TrackMouseEvent(&tme);

			this->mouseleaveScheduled = true;
		}

		// check if the mouse is over the thumb
		if (isPointInRect(xPos, yPos, &thumbActivationRect))
		{
			if (wParam & MK_LBUTTON)
			{
				if (!this->thumbPressed)
				{
					this->thumbPressed = true;
					this->thumbHover = false;

					// calculate the offset to the middle of the thumb
					this->trackOffset =
						this->calculateTrackOffset(xPos, yPos);

					this->disableMouseLeaveNotification();
					SetCapture(this->scrollBarWnd);
					this->inputCaptured = true;

					redraw = true;
				}
			}
			else
			{
				if (!this->thumbHover)
				{
					this->thumbHover = true;
					this->thumbPressed = false;

					redraw = true;
				}
			}
		}
		else
		{
			if (this->thumbHover || this->thumbPressed)
			{
				this->thumbHover = false;
				this->thumbPressed = false;

				redraw = true;
			}

			// check if the mouse is over the buttons
			RECT pls, mns;
			this->getButtonRects(&pls, &mns);

			if (isPointInRect(xPos, yPos, &pls))
			{
				if (wParam & MK_LBUTTON)
				{
					if (!this->plusButtonPressed)
					{
						this->plusButtonPressed = true;
						this->plusButtonHover = false;

						redraw = true;
					}
				}
				else
				{
					if (!this->plusButtonHover)
					{
						this->plusButtonHover = true;
						this->plusButtonPressed = false;

						redraw = true;
					}
				}
			}
			else
			{
				if (this->plusButtonHover || this->plusButtonPressed)
				{
					this->plusButtonPressed = false;
					this->plusButtonHover = false;

					redraw = true;
				}

				if (isPointInRect(xPos, yPos, &mns))
				{
					if (wParam & MK_LBUTTON)
					{
						if (!this->minusButtonPressed)
						{
							this->minusButtonPressed = true;
							this->minusButtonHover = false;

							redraw = true;
						}
					}
					else
					{
						if (!this->minusButtonHover)
						{
							this->minusButtonHover = true;
							this->minusButtonPressed = false;

							redraw = true;
						}
					}
				}
				else
				{
					if (this->minusButtonHover || this->minusButtonPressed)
					{
						this->minusButtonPressed = false;
						this->minusButtonHover = false;

						redraw = true;
					}
				}
			}
		}
	}
	if (redraw)
	{
		RedrawWindow(this->scrollBarWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CSScrollbar::onMouseLeave()
{
	bool redraw = false;
	this->mouseleaveScheduled = false;

	if (this->plusButtonHover)
	{
		this->plusButtonHover = false;
		redraw = true;
	}
	if (this->plusButtonPressed)
	{
		this->plusButtonPressed = false;
		redraw = true;
	}
	if (this->minusButtonHover)
	{
		this->minusButtonHover = false;
		redraw = true;
	}
	if (this->minusButtonPressed)
	{
		this->minusButtonPressed = false;
		redraw = true;
	}
	if (this->thumbPressed)
	{
		this->thumbPressed = false;
		redraw = true;
	}
	if (this->thumbHover)
	{
		this->thumbHover = false;
		redraw = true;
	}
	if (redraw)
	{
		RedrawWindow(this->scrollBarWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CSScrollbar::onLButtonDown(LPARAM lParam)
{
	auto xPos = GET_X_LPARAM(lParam);
	auto yPos = GET_Y_LPARAM(lParam);

	bool redraw = false;
	RECT thumbActivationRect;
	this->getThumbActivationRect(&thumbActivationRect);

	// check if the mouse is over the thumb
	if (isPointInRect(xPos, yPos, &thumbActivationRect))
	{
		if (!this->thumbPressed)
		{
			this->thumbPressed = true;
			this->thumbHover = false;

			// calculate the offset to the middle of the thumb
			this->trackOffset =
				this->calculateTrackOffset(xPos, yPos);

			this->disableMouseLeaveNotification();
			SetCapture(this->scrollBarWnd);
			this->inputCaptured = true;

			redraw = true;
		}
	}
	else
	{
		// check if the mouse is over the buttons
		RECT pls, mns;
		this->getButtonRects(&pls, &mns);

		if (isPointInRect(xPos, yPos, &pls))
		{
			if (!this->plusButtonPressed)
			{
				this->plusButtonPressed = true;
				this->plusButtonHover = false;

				if (this->scrollEvents != nullptr)
				{
					this->scrollEvents->OnLinePlus(
						reinterpret_cast<cObject>(this),
						this->scrollPosition
					);
				}
				redraw = true;
			}		
		}
		else
		{
			if (isPointInRect(xPos, yPos, &mns))
			{
				if (!this->minusButtonPressed)
				{
					this->minusButtonPressed = true;
					this->minusButtonHover = false;

					if (this->scrollEvents != nullptr)
					{
						this->scrollEvents->OnLineMinus(
							reinterpret_cast<cObject>(this),
							this->scrollPosition
						);
					}
					redraw = true;
				}
			}
			else
			{
				// check if the mouse is in the gap between the thumb and the buttons

				if (this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
				{
					if ((yPos < pls.top) && (yPos > thumbActivationRect.bottom))
					{
						// page plus
						if (this->scrollEvents != nullptr)
						{
							this->scrollEvents->OnPagePlus(
								reinterpret_cast<cObject>(this),
								this->scrollPosition
							);
						}
					}
					else if ((yPos < thumbActivationRect.top) && (yPos > mns.bottom))
					{
						// page minus
						if (this->scrollEvents != nullptr)
						{
							this->scrollEvents->OnPageMinus(
								reinterpret_cast<cObject>(this),
								this->scrollPosition
							);
						}
					}
				}
				else
				{
					if ((xPos > thumbActivationRect.right) && (xPos < pls.left))
					{
						// page plus
						if (this->scrollEvents != nullptr)
						{
							this->scrollEvents->OnPagePlus(
								reinterpret_cast<cObject>(this),
								scrollPosition
							);
						}
					}
					else if ((xPos < thumbActivationRect.left) && (xPos > mns.right))
					{
						// page minus
						if (this->scrollEvents != nullptr)
						{
							this->scrollEvents->OnPageMinus(
								reinterpret_cast<cObject>(this),
								this->scrollPosition
							);
						}
					}
				}
			}
		}
	}
	if (redraw)
	{
		RedrawWindow(this->scrollBarWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CSScrollbar::onLButtonUp(LPARAM lParam)
{
	auto xPos = GET_X_LPARAM(lParam);
	auto yPos = GET_Y_LPARAM(lParam);

	this->trackOffset = 0;
	bool redraw = false;
	RECT thumbActivationRect;
	this->getThumbActivationRect(&thumbActivationRect);

	if (this->inputCaptured)
	{
		RECT rcWnd;
		POINT screenPos;

		GetCursorPos(&screenPos);
		GetWindowRect(this->scrollBarWnd, &rcWnd);

		// if the cursor is outside of the window, reset control parameter and redraw
		if(!isPointInRect(screenPos.x, screenPos.y, &rcWnd))
		{
			this->resetControlParameter();
			redraw = true;
		}
		ReleaseCapture();
		this->inputCaptured = false;
	}

	// check if the mouse is over the thumb
	if (isPointInRect(xPos, yPos, &thumbActivationRect))
	{
		this->thumbPressed = false;
		this->thumbHover = true;

		redraw = true;
	}
	else
	{
		// check if the mouse is over the buttons
		RECT pls, mns;
		this->getButtonRects(&pls, &mns);

		if (isPointInRect(xPos, yPos, &pls))
		{
			this->plusButtonPressed = false;
			this->plusButtonHover = true;

			redraw = true;
		}
		else
		{
			if (isPointInRect(xPos, yPos, &mns))
			{
				this->minusButtonPressed = false;
				this->minusButtonHover = true;

				redraw = true;			
			}
		}
	}
	if (redraw)
	{
		RedrawWindow(this->scrollBarWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CSScrollbar::onLButtonDblClick(LPARAM lParam)
{
	auto xPos = GET_X_LPARAM(lParam);
	auto yPos = GET_Y_LPARAM(lParam);

	// check if the mouse is over the buttons
	RECT pls, mns;
	this->getButtonRects(&pls, &mns);

	bool redraw = false;

	if (isPointInRect(xPos, yPos, &pls))
	{
		if (!this->plusButtonPressed)
		{
			this->plusButtonPressed = true;
			this->plusButtonHover = false;

			if (this->scrollEvents != nullptr)
			{
				this->scrollEvents->OnLinePlus(
					reinterpret_cast<cObject>(this),
					this->scrollPosition
				);
			}
			redraw = true;
		}
	}
	else
	{
		if (isPointInRect(xPos, yPos, &mns))
		{
			if (!this->minusButtonPressed)
			{
				this->minusButtonPressed = true;
				this->minusButtonHover = false;

				if (this->scrollEvents != nullptr)
				{
					this->scrollEvents->OnLineMinus(
						reinterpret_cast<cObject>(this),
						this->scrollPosition
					);
				}
				redraw = true;
			}
		}
	}
	if (redraw)
	{
		RedrawWindow(this->scrollBarWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CSScrollbar::onSize()
{
	this->updateThumbRect();
	RedrawWindow(this->scrollBarWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW);

	return static_cast<LRESULT>(0);
}

void CSScrollbar::drawVerticalBar(HDC hdc, HWND sbar)
{
	RECT rc;
	GetClientRect(sbar, &rc);

	if(this->enabledState)
	{
		// draw enabled:

		// erase the area
		FillRect(hdc, &rc, this->bkgndBrush);

		//draw the arrow images
		if (this->minusButtonHover)
			DrawIconEx(hdc, 0, 0, this->minusIconHgl, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);
		else if (this->minusButtonPressed)
			DrawIconEx(hdc, 0, 0, this->minusIconPrs, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);
		else
			DrawIconEx(hdc, 0, 0, this->minusIcon, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);

		if (this->plusButtonHover)
			DrawIconEx(hdc, 0, rc.bottom - this->iconSquareSize, this->plusIconHgl, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);
		else if (this->plusButtonPressed)
			DrawIconEx(hdc, 0, rc.bottom - this->iconSquareSize, this->plusIconPrs, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);
		else
			DrawIconEx(hdc, 0, rc.bottom - this->iconSquareSize, this->plusIcon, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);

		if(this->thumbHover)
			FillRect(hdc, &this->ThumbRect, this->thumbHglBrush);
		else if(this->thumbPressed)
			FillRect(hdc, &this->ThumbRect, this->thumbPrsBrush);
		else
			FillRect(hdc, &this->ThumbRect, this->thumbBrush);
	}
	else
	{
		// draw disabled:

		// erase the area
		FillRect(hdc, &rc, this->bkgndBrush);

		// draw the arrow images
		DrawIconEx(hdc, 0, 0, this->minusIconDisabled, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);
		DrawIconEx(hdc, 0, rc.bottom - this->iconSquareSize, this->plusIconDisabled, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);
	}
}

void CSScrollbar::drawHorizontalBar(HDC hdc, HWND sbar)
{
	RECT rc;
	GetClientRect(sbar, &rc);

	if (this->enabledState)
	{
		// draw enabled:

		// erase the area
		FillRect(hdc, &rc, this->bkgndBrush);

		//draw the arrow images
		if (this->minusButtonHover)
			DrawIconEx(hdc, 0, 0, this->minusIconHgl, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);
		else if (this->minusButtonPressed)
			DrawIconEx(hdc, 0, 0, this->minusIconPrs, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);
		else
			DrawIconEx(hdc, 0, 0, this->minusIcon, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);

		if (this->plusButtonHover)
			DrawIconEx(hdc, rc.right - this->iconSquareSize, 0, this->plusIconHgl, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);
		else if (this->plusButtonPressed)
			DrawIconEx(hdc, rc.right - this->iconSquareSize, 0, this->plusIconPrs, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);
		else
			DrawIconEx(hdc, rc.right - this->iconSquareSize, 0, this->plusIcon, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);

		if (this->thumbHover)
			FillRect(hdc, &this->ThumbRect, this->thumbHglBrush);
		else if (this->thumbPressed)
			FillRect(hdc, &this->ThumbRect, this->thumbPrsBrush);
		else
			FillRect(hdc, &this->ThumbRect, this->thumbBrush);
	}
	else
	{
		// draw disabled:

		// erase the area
		FillRect(hdc, &rc, this->bkgndBrush);

		// draw the arrow images
		DrawIconEx(hdc, 0, 0, this->minusIconDisabled, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);
		DrawIconEx(hdc, rc.right - this->iconSquareSize, 0, this->plusIconDisabled, this->iconSquareSize, this->iconSquareSize, 0, nullptr, DI_NORMAL);
	}
}

HRESULT CSScrollbar::registerScrollClass()
{
	HRESULT hr = S_OK;
	WNDCLASSEX wcx;

	if (GetClassInfoEx(this->hInstance, CSScrollbar::CSSCROLLBAR_CLASS_IDENTIFIER, &wcx) == 0)
	{
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wcx.lpfnWndProc = CSScrollbar::scrollBarProc;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = sizeof(LONG_PTR);
		wcx.hInstance = this->hInstance;
		wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcx.hIcon = nullptr;
		wcx.hIconSm = nullptr;
		wcx.lpszClassName = CSScrollbar::CSSCROLLBAR_CLASS_IDENTIFIER;
		wcx.lpszMenuName = nullptr;
		wcx.hbrBackground = nullptr;

		hr = (RegisterClassEx(&wcx) == 0)
			? E_FAIL : S_OK;
	}
	return hr;
}

LRESULT CSScrollbar::scrollBarProc(HWND scrollBar, UINT message, WPARAM wParam, LPARAM lParam)
{
	CSScrollbar* pCSSbar = nullptr;

	if (message == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		if (pcr != nullptr)
		{
			pCSSbar = reinterpret_cast<CSScrollbar*>(pcr->lpCreateParams);
			if (pCSSbar != nullptr)
			{
				SetWindowLongPtr(scrollBar, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCSSbar));
			}
		}
		return static_cast<LRESULT>(1);
	}
	else
	{
		pCSSbar = reinterpret_cast<CSScrollbar*>(GetWindowLongPtr(scrollBar, GWLP_USERDATA));
		if (pCSSbar != nullptr)
		{
			switch (message)
			{
			case WM_PAINT:
				return pCSSbar->onPaint(scrollBar);
			case WM_SIZE:
				return pCSSbar->onSize();
			case WM_MOUSEWHEEL:
				return pCSSbar->onMouseWheel(wParam);
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_MOUSEMOVE:
				return pCSSbar->onMousemove(wParam, lParam);
			case WM_MOUSELEAVE:
				return pCSSbar->onMouseLeave();
			case WM_LBUTTONDOWN:
				return pCSSbar->onLButtonDown(lParam);
			case WM_LBUTTONUP:
				return pCSSbar->onLButtonUp(lParam);
			case WM_LBUTTONDBLCLK:
				return pCSSbar->onLButtonDblClick(lParam);
			case WM_DESTROY:
				pCSSbar->Release();
				return 0;
			default:
				break;
			}
		}
		return DefWindowProc(scrollBar, message, wParam, lParam);
	}
}

LRESULT CSScrollbar::onPaint(HWND scrollBar)
{
	HDC hdc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(scrollBar, &ps);
	if (hdc)
	{
		if (this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
			this->drawVerticalBar(hdc, scrollBar);
		else
			this->drawHorizontalBar(hdc, scrollBar);

		EndPaint(scrollBar, &ps);
	}
	return static_cast<LRESULT>(0);
}

bool CSScrollbar::calculateDefaultSize()
{
	if (this->parentWnd != nullptr)
	{
		RECT rc;
		GetClientRect(this->parentWnd, &rc);

		if (this->scrollBarType == CSScrollbar::SCROLLBARTYPE_HORIZONTAL)
		{
			this->Position.x = 0;
			this->Position.y = rc.bottom - this->defaultThickness;

			this->Size.cx = rc.right;
			this->Size.cy = this->defaultThickness;
		}
		else if (this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
		{
			this->Position.x = rc.right - this->defaultThickness;
			this->Position.y = 0;

			this->Size.cx = this->defaultThickness;
			this->Size.cy = rc.bottom;
		}
		return true;
	}
	return false;
}

int CSScrollbar::calculateHalfThumbLength(int windowlength)
{
	auto corrected_windowlength = windowlength - (2 * this->iconSquareSize);

	if (corrected_windowlength > 0)
	{
		auto scalarVal = (double)(this->scrollRange + windowlength) / (double)corrected_windowlength;
		if (scalarVal > 1)
		{
			// if the range is larger than the windowlength, the scalar-value is above 1

			auto tmbwidth = (double)corrected_windowlength / scalarVal;		// calculate the absolute thumbwidth
			tmbwidth = tmbwidth / 2;								// make it a half for subsequent usage
			return (int)tmbwidth;
		}
	}
	return -1;
}

int CSScrollbar::calculateRelativeThumbPosition(int windowlength)
{
	auto corrected_windowlength = windowlength - (2 * this->iconSquareSize); // correct the window-length by subtract the length of the plus + minus buttons

	if (corrected_windowlength > 0)
	{
		auto scalarVal = (double)(this->scrollRange + windowlength) / (double)corrected_windowlength;	// calc the value to scale the thumb
		if (scalarVal > 1)
		{
			auto tmbwidth = (double)corrected_windowlength / scalarVal;		// calculate the thumbwidth

			scalarVal = (double)this->scrollRange / ((double)corrected_windowlength - tmbwidth);	// re-calculate the scale-value with the final length, over which the thumb can be moved

			auto relativePosition =
				(((double)this->scrollPosition / scalarVal)			// scale the current scroll position
				+ (tmbwidth / 2))									// add the half of the thumb
				+ this->iconSquareSize;								// add the length of the minus button (upper button!)

			return (int)relativePosition;
		}
	}
	return -1;
}

int CSScrollbar::calculateAbsoluteThumbPosition(int x, int y)
{
	RECT rc;
	GetClientRect(this->scrollBarWnd, &rc);

	double windowlength =
		(this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
		? rc.bottom : rc.right;											// get the windowlength

	windowlength = windowlength - (2 * this->iconSquareSize);			// correct the windowlength by subtracting the plus + minus buttons

	if (windowlength > 0)
	{
		auto scalarVal = (double)(this->scrollRange + rc.bottom) / windowlength;		// calc the value to get thumb-length
		if (scalarVal > 1)
		{
			auto tmbwidth = windowlength / scalarVal;					// calc the thumbwidth

			double posVal =
				(this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
				? (y -= (int)((tmbwidth / 2.0) + this->iconSquareSize))
				: (x -= (int)((tmbwidth / 2.0) + this->iconSquareSize));

			if (posVal >= 0)
			{
				scalarVal = (double)(this->scrollRange) / (windowlength - tmbwidth);	// re-calculate the scale-value with the final length, over which the thumb can be moved

				double absolutePosition = posVal * scalarVal;			// calc the absolute position

				return (int)absolutePosition;
			}
		}
	}
	return -1;
}

int CSScrollbar::calculateTrackOffset(int x, int y)
{
	int thumbVal =
		(this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
		? ((this->ThumbRect.bottom - this->ThumbRect.top) / 2)
		: ((this->ThumbRect.right - this->ThumbRect.left) / 2);

	if (thumbVal > 0)
	{
		thumbVal =
			(this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
			? (thumbVal + this->ThumbRect.top)
			: (thumbVal + this->ThumbRect.left);

		return (this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
			? (thumbVal - y)
			: (thumbVal - x);
	}
	return 0;
}

void CSScrollbar::updateThumbRect()
{
	RECT rc;
	GetClientRect(this->scrollBarWnd, &rc);

	// calculate the thumblength -> if it is -1 -> scrollbar disabled
	auto thumbHalfLength = this->calculateHalfThumbLength(
		(this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
		? rc.bottom : rc.right
	);

	// calculate the relative position -> if it is -1 -> scrollbar disabled
	auto relativeThumbPosition = this->calculateRelativeThumbPosition(
		(this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
		? rc.bottom : rc.right
	);


	if ((thumbHalfLength < 0) || (relativeThumbPosition < 0))
	{
		// disable
		this->enabledState = false;

		if (this->autoHide)
		{
			ShowWindow(this->scrollBarWnd, SW_HIDE);
		}
	}
	else
	{
		if (!this->enabledState)
		{
			// enable
			this->enabledState = true;

			if (this->autoHide)
			{
				ShowWindow(this->scrollBarWnd, SW_SHOW);
			}
		}

		if (this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
		{
			SetRect(
				&this->ThumbRect,
				this->defaultThickness / 4,
				relativeThumbPosition - thumbHalfLength,
				this->defaultThickness - (this->defaultThickness / 4),
				relativeThumbPosition + thumbHalfLength
			);
		}
		else
		{
			SetRect(
				&this->ThumbRect,
				
				relativeThumbPosition - thumbHalfLength,
				this->defaultThickness / 4,
				relativeThumbPosition + thumbHalfLength,
				this->defaultThickness - (this->defaultThickness / 4)				
			);
		}
	}
}

void CSScrollbar::getThumbActivationRect(LPRECT prc)
{
	if (this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
	{
		prc->top = this->ThumbRect.top;
		prc->bottom = this->ThumbRect.bottom;
		prc->left = 0;
		prc->right = this->defaultThickness;
	}
	else
	{
		prc->right = this->ThumbRect.right;
		prc->left = this->ThumbRect.left;
		prc->top = 0;
		prc->bottom = this->defaultThickness;
	}
}

void CSScrollbar::getButtonRects(LPRECT plusButton, LPRECT minusButton)
{
	RECT rc;
	GetClientRect(this->scrollBarWnd, &rc);

	if (this->scrollBarType == CSScrollbar::SCROLLBARTYPE_VERTICAL)
	{
		SetRect(minusButton, 0, 0, rc.right, this->iconSquareSize);
		SetRect(plusButton, 0, rc.bottom - this->iconSquareSize, rc.right, rc.bottom);
	}
	else
	{
		SetRect(plusButton, rc.right - this->iconSquareSize, 0, rc.right, rc.bottom);
		SetRect(minusButton, 0, 0, this->iconSquareSize, rc.bottom);		
	}
}

void CSScrollbar::resetControlParameter()
{
	if (this->mouseleaveScheduled)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_CANCEL;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = this->scrollBarWnd;
		TrackMouseEvent(&tme);

		this->mouseleaveScheduled = false;
	}
	this->plusButtonHover = false;
	this->plusButtonPressed = false;
	this->minusButtonHover = false;
	this->minusButtonPressed = false;
	this->thumbPressed = false;
	this->thumbHover = false;

	this->trackOffset = 0;
}

void CSScrollbar::disableMouseLeaveNotification()
{
	if (this->mouseleaveScheduled)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_CANCEL;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = this->scrollBarWnd;
		TrackMouseEvent(&tme);

		this->mouseleaveScheduled = false;
	}
}
