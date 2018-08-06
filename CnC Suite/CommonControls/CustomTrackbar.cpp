#include"CustomTrackbar.h"
#include"..//HelperF.h"

customTrackbar::customTrackbar()
	:trackRange(200),
	trackMax(100),
	trackMin(-100),
	trackPosition(0),
	imageSquareSize(0),
	normalTrackbuttonImageResId(0),
	trackingTrackbuttonImageResId(0),
	trackbarType(customTrackbar::TRACKBARTYPE_HORIZONTAL),
	_parent(nullptr),
	_trackbar(nullptr),
	_eventSink(nullptr),
	isTracking(false),
	normalImage(nullptr),
	trackingImage(nullptr),
	fontHeight(0),
	trackbarThickness(1),
	cursorOffset(0),
	_drawBorder(false),
	ctrlID(0)
{
	this->backgroundColor = RGB(240, 240, 240);
	this->barColor = RGB(40, 40, 40);
	this->textColor = RGB(0, 0, 0);
	this->controlFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Calibri\0");
}

customTrackbar::~customTrackbar()
{
	DestroyIcon(this->normalImage);
	DestroyIcon(this->trackingImage);
	DeleteObject(this->controlFont);
}

const TCHAR* customTrackbar::customTrackbarClass = L"CUSTOMTRACKBARCLASS_";

HRESULT customTrackbar::create(HINSTANCE hInst, HWND parent, LPPOINT pos, LPSIZE sz, DWORD controlID)
{
	HRESULT hr;

	this->hInstance = hInst;
	this->_parent = parent;
	this->ctrlID = controlID;

	hr = this->_registerTrackbarClass();
	if (SUCCEEDED(hr))
	{
		hr = ((this->normalTrackbuttonImageResId == 0) || (this->trackingTrackbuttonImageResId == 0))
			? E_FAIL : S_OK;

		if (SUCCEEDED(hr))
		{
			this->normalImage =
				(HICON)LoadImage(
					hInst,
					MAKEINTRESOURCE(this->normalTrackbuttonImageResId),
					IMAGE_ICON,
					this->imageSquareSize,
					this->imageSquareSize,
					LR_DEFAULTCOLOR
				);
			this->trackingImage =
				(HICON)LoadImage(
					hInst,
					MAKEINTRESOURCE(this->trackingTrackbuttonImageResId),
					IMAGE_ICON,
					this->imageSquareSize,
					this->imageSquareSize,
					LR_DEFAULTCOLOR
				);
			hr = ((this->normalImage != nullptr) && (this->trackingImage != nullptr))
				? S_OK : E_FAIL;

			if (SUCCEEDED(hr))
			{
				DWORD styles = WS_CHILD | WS_VISIBLE;
				if (this->_drawBorder)
					styles |= WS_BORDER;

				this->_trackbar =
					CreateWindow(
						customTrackbar::customTrackbarClass,
						nullptr,
						styles,
						pos->x, pos->y,
						sz->cx, sz->cy,
						parent,
						IDTOHMENU(controlID),
						hInst,
						reinterpret_cast<LPVOID>(this)
					);

				hr = (this->_trackbar != nullptr) ? S_OK : E_FAIL;

				if (SUCCEEDED(hr))
				{
					// ...
				}
			}
		}
	}
	return hr;
}

void customTrackbar::setEventHandler(trackBarEventSink * eventSink)
{
	this->_eventSink = eventSink;
}

void customTrackbar::setTrackRange(int lowerMin, int upperMax)
{
	if(upperMax > 0)
		this->trackMax = upperMax;

	if(lowerMin <= 0)
		this->trackMin = lowerMin;

	this->trackRange = upperMax + (-lowerMin);
}

void customTrackbar::setTrackPosition(int pos)
{
	if ((pos <= this->trackMax) && (pos >= this->trackMin))
	{
		this->trackPosition = pos;	
	}
	else
	{
		if (pos < this->trackMin)
			this->trackPosition = this->trackMin;
		else if (pos > this->trackMax)
			this->trackPosition = this->trackMax;
	}
	this->UpdateTrackbar();
}

void customTrackbar::setText(const iString & text)
{
	this->_text = text;
}

void customTrackbar::setColors(COLORREF background, COLORREF bar, COLORREF text)
{
	this->backgroundColor = background;
	this->barColor = bar;
	this->textColor = text;
}

void customTrackbar::setTrackbuttonImages(int normal, int tracking, int squareSize)
{
	this->normalTrackbuttonImageResId = normal;
	this->trackingTrackbuttonImageResId = tracking;
	this->imageSquareSize = squareSize;
}

void customTrackbar::setType(int type)
{
	this->trackbarType = type;
}

void customTrackbar::setBarAppearence(int barThickness, bool drawMiddleMarker, bool drawScale, int scaleStep)
{
	this->trackbarThickness = barThickness;

	// these values are not implemented yet
	UNREFERENCED_PARAMETER(drawMiddleMarker);
	UNREFERENCED_PARAMETER(drawScale);
	UNREFERENCED_PARAMETER(scaleStep);
}

void customTrackbar::setBorder(bool drawBorder)
{
	this->_drawBorder = drawBorder;
}

void customTrackbar::setFont(HFONT font)
{
	if (font != nullptr)
	{
		if (this->controlFont != nullptr)
			DeleteObject(this->controlFont);

		this->controlFont = font;
	}
}

void customTrackbar::UpdateTrackbar()
{
	RedrawWindow(this->_trackbar, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
}

HRESULT customTrackbar::_registerTrackbarClass()
{
	HRESULT hr = S_OK;
	WNDCLASSEX wcx;

	if (!GetClassInfoEx(this->hInstance, customTrackbar::customTrackbarClass, &wcx))
	{
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = sizeof(LONG_PTR);
		wcx.lpfnWndProc = customTrackbar::trackbarProc;
		wcx.hInstance = this->hInstance;
		wcx.hIcon = nullptr;
		wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcx.hbrBackground = nullptr;
		wcx.lpszClassName = customTrackbar::customTrackbarClass;
		wcx.lpszMenuName = nullptr;
		wcx.hIconSm = nullptr;

		hr = (RegisterClassEx(&wcx) != 0) ? S_OK : E_FAIL;
	}
	return hr;
}

LRESULT customTrackbar::trackbarProc(HWND trackWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	customTrackbar* ctb = nullptr;

	if (message == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		if (pcr != nullptr)
		{
			ctb = reinterpret_cast<customTrackbar*>(pcr->lpCreateParams);
			if (ctb != nullptr)
			{
				SetWindowLongPtr(trackWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctb));
			}
		}
		return 1;
	}
	else
	{
		ctb = reinterpret_cast<customTrackbar*>(GetWindowLongPtr(trackWnd, GWLP_USERDATA));

		if (ctb != nullptr)
		{
			switch (message)
			{
			case WM_MOUSEMOVE:
				return ctb->onMousemove(lParam);
			case WM_MOUSELEAVE:
				return ctb->onMouseLeave();
			case WM_LBUTTONDOWN:
				return ctb->onLButtondown(lParam);
			case WM_LBUTTONUP:
				return ctb->onLButtonup(lParam);
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_PAINT:
				return ctb->onPaint(trackWnd);
			case WM_DESTROY:
				SafeRelease(&ctb);
			default:
				break;
			}
		}
		return DefWindowProc(trackWnd, message, wParam, lParam);
	}
}

LRESULT customTrackbar::onPaint(HWND track)
{
	HDC hdc;
	RECT rc;
	PAINTSTRUCT ps;

	GetClientRect(track, &rc);

	hdc = BeginPaint(track, &ps);
	if (hdc)
	{
		HDC hdcMem = CreateCompatibleDC(hdc);
		if (hdcMem)
		{
			HBITMAP wndImage = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			if (wndImage)
			{
				HGDIOBJ origin = SelectObject(hdcMem, wndImage);

				HBRUSH bkgnd = CreateSolidBrush(this->backgroundColor);
				if (bkgnd)
				{
					FillRect(hdcMem, &rc, bkgnd);

					HBRUSH barBrush = CreateSolidBrush(this->barColor);
					if (barBrush)
					{
						SIZE textSize;
						textSize.cx = 0;
						textSize.cy = 0;

						int textLen = this->_text.GetLength();
						if (textLen > 0)
						{
							HGDIOBJ originFont = SelectObject(hdcMem, this->controlFont);

							SetBkMode(hdcMem, TRANSPARENT);
							SetTextColor(hdcMem, this->textColor);

							GetTextExtentPoint32(hdcMem, this->_text.GetData(), this->_text.GetLength(), &textSize);

							TextOut(
								hdcMem,
								(rc.right / 2) - (textSize.cx / 2),
								2,
								this->_text.GetData(),
								this->_text.GetLength()
							);

							SelectObject(hdcMem, originFont);

							// this is only set if the textcontent is valid
							this->fontHeight = textSize.cy; // reduces the absolute track position in a vertical trackbar
						}

						RECT barRect;
						POINT iconOutputPos;

						if (this->trackbarType == customTrackbar::TRACKBARTYPE_VERTICAL)
						{
							iconOutputPos.x = (rc.right / 2) - (this->imageSquareSize / 2);
							iconOutputPos.y = this->_getAbsoluteTrackPosition() - (this->imageSquareSize / 2);
							
							SetRect(
								&barRect,
								(rc.right / 2) - this->trackbarThickness,
								(this->imageSquareSize / 2) + 5 + this->fontHeight,
								(rc.right / 2) + this->trackbarThickness,
								rc.bottom - ((this->imageSquareSize / 2) + 5)
							);
						}
						else
						{
							iconOutputPos.x = this->_getAbsoluteTrackPosition() - (this->imageSquareSize / 2);
							iconOutputPos.y = (rc.bottom / 2) - (this->imageSquareSize / 2);

							SetRect(
								&barRect,
								(this->imageSquareSize / 2) + 5,
								(rc.bottom / 2) - this->trackbarThickness,
								rc.right - ((this->imageSquareSize / 2) + 5),
								(rc.bottom / 2) + this->trackbarThickness
							);
						}
						FillRect(hdcMem, &barRect, barBrush);

						DrawIconEx(
							hdcMem,
							iconOutputPos.x,
							iconOutputPos.y,
							this->isTracking ? this->trackingImage : this->normalImage,
							this->imageSquareSize,
							this->imageSquareSize,
							0,
							nullptr,
							DI_NORMAL
						);
						BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

						DeleteObject(barBrush);
					}
					DeleteObject(bkgnd);
				}
				SelectObject(hdcMem, origin);
				DeleteObject(wndImage);
			}
			DeleteDC(hdcMem);
		}
		EndPaint(track, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT customTrackbar::onMousemove(LPARAM lParam)
{
	int xPos = GET_X_LPARAM(lParam);
	int yPos = GET_Y_LPARAM(lParam);

	if (this->isTracking)
	{
		this->setTrackPositionFromCursor(
			(this->trackbarType == customTrackbar::TRACKBARTYPE_HORIZONTAL)
			? xPos : yPos
		);
		this->UpdateTrackbar();

		if (this->_eventSink != nullptr)
		{
			this->_eventSink->trackBar_Tracking(
				reinterpret_cast<cObject>(this),
				this->trackPosition
			);
		}
	}

	return static_cast<LRESULT>(0);
}

LRESULT customTrackbar::onLButtondown(LPARAM lParam)
{
	int xPos = GET_X_LPARAM(lParam);
	int yPos = GET_Y_LPARAM(lParam);

	auto absPos = this->_getAbsoluteTrackPosition();

	RECT rc;
	RECT trackerPos;
	GetClientRect(this->_trackbar, &rc);

	if (this->trackbarType == customTrackbar::TRACKBARTYPE_VERTICAL)
	{
		SetRect(
			&trackerPos,
			(rc.right / 2) - (this->imageSquareSize / 2),
			absPos - (this->imageSquareSize / 2),
			(rc.right / 2) + (this->imageSquareSize / 2),
			absPos + (this->imageSquareSize / 2)
		);
	}
	else
	{
		SetRect(
			&trackerPos,
			absPos - (this->imageSquareSize / 2),
			(rc.bottom / 2) - (this->imageSquareSize / 2),
			absPos + (this->imageSquareSize / 2),
			(rc.bottom / 2) + (this->imageSquareSize / 2)
		);
	}

	if (isPointInRect(xPos, yPos, &trackerPos))
	{
		if (!this->isTracking)
		{
			// get the cursor offset
			this->cursorOffset =
				(this->trackbarType == customTrackbar::TRACKBARTYPE_HORIZONTAL)
				? (absPos - xPos)
				: (absPos - yPos);

			// make sure to detect mouseleave!!
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.dwHoverTime = 0;
			tme.hwndTrack = this->_trackbar;
			TrackMouseEvent(&tme);
			
			this->isTracking = true;
			this->UpdateTrackbar();

			if (this->_eventSink != nullptr)
			{
				this->_eventSink->trackBar_TrackStarted(
					reinterpret_cast<cObject>(this),
					this->trackPosition
				);
			}
		}
	}
	else
	{
		this->setTrackPositionFromCursor(
			(this->trackbarType == customTrackbar::TRACKBARTYPE_HORIZONTAL)
			? xPos : yPos
		);
		this->UpdateTrackbar();

		if (this->_eventSink != nullptr)
		{
			this->_eventSink->trackBar_TrackEnd(
				reinterpret_cast<cObject>(this),
				this->trackPosition
			);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT customTrackbar::onLButtonup(LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	//int xPos = GET_X_LPARAM(lParam);
	//int yPos = GET_Y_LPARAM(lParam);

	if (this->isTracking)
	{
		this->cursorOffset = 0;
		this->isTracking = false;

		this->UpdateTrackbar();

		if (this->_eventSink != nullptr)
		{
			this->_eventSink->trackBar_TrackEnd(
				reinterpret_cast<cObject>(this),
				this->trackPosition
			);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT customTrackbar::onMouseLeave()
{
	if (this->isTracking)
	{
		this->cursorOffset = 0;
		this->isTracking = false;

		this->UpdateTrackbar();

		if (this->_eventSink != nullptr)
		{
			this->_eventSink->trackBar_TrackEnd(
				reinterpret_cast<cObject>(this),
				this->trackPosition
			);
		}
	}
	return static_cast<LRESULT>(0);
}

int customTrackbar::_getAbsoluteTrackPosition()
{
	RECT rc;
	GetClientRect(this->_trackbar, &rc);

	int len = (this->trackbarType == customTrackbar::TRACKBARTYPE_HORIZONTAL)
		? rc.right : (rc.bottom - this->fontHeight);

	double trackbar_effective_length =
		len - (this->imageSquareSize + 10);

	double pixelPerTrackpoint = 0;

	if (this->trackRange != 0)// prevent division by zero
		pixelPerTrackpoint = trackbar_effective_length / this->trackRange;
	else
		return 0;

	//new calculation:

	double value = 0;

	if (this->trackbarType == customTrackbar::TRACKBARTYPE_VERTICAL)
	{
		value =
			((rc.bottom - (this->imageSquareSize / 2)) - 5)
			- (this->trackPosition * pixelPerTrackpoint);

		value += (this->trackMin * pixelPerTrackpoint);// if trackMin is not zero ->adapt the position!
	}
	else if (this->trackbarType == customTrackbar::TRACKBARTYPE_HORIZONTAL)
	{
		value =
			((this->imageSquareSize / 2) + 5)
			+ (this->trackPosition * pixelPerTrackpoint);

		value -= (this->trackMin * pixelPerTrackpoint);// if trackMin is not zero ->adapt the position!
	}
	else
		return 0;

	return (int)value;

	
	// old calculation:

	//double middle = (-(this->trackMin))*pixelPerTrackpoint;
	//double currentTrackPos = pixelPerTrackpoint * this->trackPosition;
	//
	//middle += currentTrackPos;
	//middle += ((double)((this->imageSquareSize / 2) + 5));

	//if (this->trackbarType == customTrackbar::TRACKBARTYPE_VERTICAL)
	//{
	//	middle += ((double)this->fontHeight);

	//	middle = rc.bottom - middle;// ????
	//}
	//return (int)middle;
}

void customTrackbar::setTrackPositionFromCursor(int xyCursor)
{
	xyCursor += this->cursorOffset;

	RECT rc;
	GetClientRect(this->_trackbar, &rc);

	int len = (this->trackbarType == customTrackbar::TRACKBARTYPE_HORIZONTAL)
		? rc.right : (rc.bottom - this->fontHeight);

	double trackbar_effective_length =
		len - (this->imageSquareSize + 10);

	double cursorRelativePosition =
		xyCursor - ((this->imageSquareSize / 2) + 5);

	if (this->trackbarType == customTrackbar::TRACKBARTYPE_VERTICAL)
		cursorRelativePosition -= this->fontHeight;

	double trackPointPerPixel = 0;

	if (trackbar_effective_length != 0)// prevent divison by zero
	{
		trackPointPerPixel =
			this->trackRange / trackbar_effective_length;
	}
	else
		return;

	if (this->trackbarType == customTrackbar::TRACKBARTYPE_VERTICAL)
	{
		cursorRelativePosition =
			(rc.bottom - (this->imageSquareSize + 10 + this->fontHeight + cursorRelativePosition)
				);
	}

	double tPos = cursorRelativePosition * trackPointPerPixel;

	this->trackPosition = (int)(tPos + this->trackMin);

	if (this->trackPosition > this->trackMax)
		this->trackPosition = this->trackMax;
	else if (this->trackPosition < this->trackMin)
		this->trackPosition = this->trackMin;
}
