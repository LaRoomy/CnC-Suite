#include"CustomCheckbox.h"

CustomCheckbox::CustomCheckbox(HINSTANCE hInst, HWND parent, LPPOINT position, LPSIZE size, int ctrlID)
	:hInstance(hInst),
	Parent(parent),
	eventHandler(nullptr)
{
	SecureZeroMemory(&this->cbxControl, sizeof(CHECKBOXCONTROL));

	this->cbxProperty.ctrlID = ctrlID;
	this->cbxProperty.x = position->x;
	this->cbxProperty.y = position->y;
	this->cbxProperty.width = size->cx;
	this->cbxProperty.height = size->cy;
	this->cbxProperty.alignment = CBX_ALIGN_LEFT;
	this->cbxProperty.iconID_checked = DEFAULT_IMAGE_CHECKED;
	this->cbxProperty.iconID_unchecked = DEFAULT_IMAGE_UNCHECKED;
	this->cbxProperty.iconID_checked_disabled = DEFAULT_IMAGE_CHECKED_DISABLED;
	this->cbxProperty.iconID_unchecked_disabled = DEFAULT_IMAGE_UNCHECKED_DISABLED;
	this->cbxProperty.iconSquareSize = DEFAULT_IMAGE_SIZE;
	this->cbxProperty.background = RGB(240, 240, 240);
	this->cbxProperty.textcolor = RGB(0, 0, 0);
	this->cbxProperty.ctrlFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Segoe UI\0");
	this->cbxProperty.spacing = 10;
	this->cbxProperty.padding = 5;

	this->cbxControl.isEnabled = true;
	this->cbxControl.interactionMode = CBX_INTERACTRECT_BOX;

	__pragma(warning(push));
	__pragma(warning(disable : 4127));

	if ((DEFAULT_IMAGE_CHECKED >= 0) && (DEFAULT_IMAGE_UNCHECKED >= 0)
		&& (DEFAULT_IMAGE_CHECKED_DISABLED >= 0) && (DEFAULT_IMAGE_UNCHECKED_DISABLED >= 0))
		this->cbxControl.useCustomImages = true;

	__pragma(warning(pop));
}

CustomCheckbox::~CustomCheckbox()
{
	DeleteObject(this->cbxProperty.ctrlFont);
}

void CustomCheckbox::setEventHandler(customCheckboxEventSink * handler)
{
	this->eventHandler = handler;
}

void CustomCheckbox::setCustomImages(int cImage_Checked, int cImage_Unchecked, int squareSize)
{
	this->cbxControl.useCustomImages = true;
	this->cbxProperty.iconID_checked = cImage_Checked;
	this->cbxProperty.iconID_unchecked = cImage_Unchecked;
	this->cbxProperty.iconSquareSize = squareSize;

	// redraw ??
}

void CustomCheckbox::setCustomDisabledImages(int for_checked, int for_unchecked)
{
	this->cbxProperty.iconID_checked_disabled = for_checked;
	this->cbxProperty.iconID_unchecked_disabled = for_unchecked;
}

void CustomCheckbox::setColors(COLORREF backgroundcolor, COLORREF textcolor)
{
	this->cbxProperty.background = backgroundcolor;
	this->cbxProperty.textcolor = textcolor;

	// redraw ??
}

void CustomCheckbox::setAlignment(int align)
{
	this->cbxProperty.alignment = align;

	// redraw ??
}

void CustomCheckbox::setText(iString text)
{
	this->cbxProperty.text = text;
}

void CustomCheckbox::setFont(HFONT font)
{
	if (font)
	{
		DeleteObject(this->cbxProperty.ctrlFont);
		this->cbxProperty.ctrlFont = font;

		// redraw ??
	}
}

void CustomCheckbox::setConstraints(int padding, int spacing)
{
	this->cbxProperty.padding = padding;
	this->cbxProperty.spacing = spacing;

	// redraw ??
}

void CustomCheckbox::setEnabledState(bool state)
{
	this->cbxControl.isEnabled = state;

	RedrawWindow(this->Checkbox, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
}

void CustomCheckbox::selectInteractionRect(DWORD flag)
{
	this->cbxControl.interactionMode = flag;
}

HRESULT CustomCheckbox::Create()
{
	this->registerCCboxWndClass();

	HRESULT hr;

	this->Checkbox = CreateWindow(
		CCUSTOMCHECKBOXCLASS,
		nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		this->cbxProperty.x,
		this->cbxProperty.y,
		this->cbxProperty.width,
		this->cbxProperty.height,
		this->Parent,
		reinterpret_cast<HMENU>(this->cbxProperty.ctrlID),
		this->hInstance,
		reinterpret_cast<LPVOID>(this)
	);

	hr = (this->Checkbox != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		this->cbxControl.isCreated = true;
	}
	return hr;
}

void CustomCheckbox::registerCCboxWndClass()
{
	WNDCLASSEX wcx;
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = sizeof(LONG_PTR);
	wcx.lpfnWndProc = CustomCheckbox::checkboxProc;
	wcx.hbrBackground = nullptr;
	wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcx.hIcon = nullptr;
	wcx.hInstance = this->hInstance;
	wcx.lpszMenuName = nullptr;
	wcx.lpszClassName = CCUSTOMCHECKBOXCLASS;
	wcx.hIconSm = nullptr;
	
	RegisterClassEx(&wcx);
}

LRESULT CustomCheckbox::checkboxProc(HWND checkBox, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CustomCheckbox* cbx = nullptr;

	if (msg == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		cbx = reinterpret_cast<CustomCheckbox*>(pcr->lpCreateParams);
		SetWindowLongPtr(checkBox, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cbx));
		return 1;
	}
	else
	{
		cbx = reinterpret_cast<CustomCheckbox*>(GetWindowLongPtr(checkBox, GWLP_USERDATA));

		if (cbx != nullptr)
		{
			switch (msg)
			{
			case WM_DESTROY:
				SafeRelease(&cbx);
				return 0;
			case WM_PAINT:
				return cbx->onPaint(checkBox);
			case WM_LBUTTONDOWN:
				return cbx->onLButton(
					GET_X_LPARAM(lParam),
					GET_Y_LPARAM(lParam)
				);
			default:
				return DefWindowProc(checkBox, msg, wParam, lParam);
			}
		}
		return DefWindowProc(checkBox, msg, wParam, lParam);
	}
}

LRESULT CustomCheckbox::onPaint(HWND hWnd)
{
	HDC hdc;
	RECT rc;
	SIZE textSize;
	HGDIOBJ original;
	PAINTSTRUCT ps;
	GetClientRect(hWnd, &rc);

	int y_icon =
		((rc.bottom - this->cbxProperty.iconSquareSize) == 0)
		? 0 : ((rc.bottom - this->cbxProperty.iconSquareSize) / 2);

	hdc = BeginPaint(hWnd, &ps);

	if (hdc)
	{
		HBRUSH bkgndBrush = CreateSolidBrush(this->cbxProperty.background);

		if (bkgndBrush)
		{
			FillRect(hdc, &rc, bkgndBrush);

			original = SelectObject(hdc, this->cbxProperty.ctrlFont);

			SetBkMode(hdc, TRANSPARENT);

			if(this->cbxControl.isEnabled)
				SetTextColor(hdc, this->cbxProperty.textcolor);
			else
				SetTextColor(hdc, COLOR_DISABLED_CONTROL);

			GetTextExtentPoint32(
				hdc,
				this->cbxProperty.text.GetData(),
				this->cbxProperty.text.GetLength(),
				&textSize);

			int x;
			int cx =
				this->cbxProperty.iconSquareSize +
				this->cbxProperty.spacing +
				textSize.cx;

			int y_text =
				((rc.bottom - textSize.cy) == 0)
				? 0 : (rc.bottom - textSize.cy) / 2;

			if (this->cbxProperty.alignment == CBX_ALIGN_CENTER)
			{
				x = (rc.right - cx) / 2;
			}
			else if (this->cbxProperty.alignment == CBX_ALIGN_LEFT)
			{
				x = this->cbxProperty.padding;
			}
			else
			{
				// right aligned
				x = rc.right - cx;
			}

			TextOut(
				hdc,
				x + this->cbxProperty.iconSquareSize + this->cbxProperty.spacing,
				y_text,
				this->cbxProperty.text.GetData(),
				this->cbxProperty.text.GetLength());

			SelectObject(hdc, original);

			// set check-Box rect
			this->cbxProperty.boxRect.left = x;
			this->cbxProperty.boxRect.right = x + this->cbxProperty.iconSquareSize;
			this->cbxProperty.boxRect.top = y_icon;
			this->cbxProperty.boxRect.bottom = y_icon + this->cbxProperty.iconSquareSize;

			if (this->cbxControl.useCustomImages)
			{
				int Id;

				if (this->cbxControl.isEnabled)
				{
					Id =
						(this->cbxControl.isChecked)
						? this->cbxProperty.iconID_checked : this->cbxProperty.iconID_unchecked;
				}
				else
				{
					Id =
						(this->cbxControl.isChecked)
						? this->cbxProperty.iconID_checked_disabled : this->cbxProperty.iconID_unchecked_disabled;
				}
				auto icon =(HICON)LoadImage(
					this->hInstance,
					MAKEINTRESOURCE(Id),
					IMAGE_ICON,
					this->cbxProperty.iconSquareSize,
					this->cbxProperty.iconSquareSize, LR_DEFAULTCOLOR);

				if (icon)
				{
					DrawIconEx(hdc, x, y_icon, icon, this->cbxProperty.iconSquareSize, this->cbxProperty.iconSquareSize, 0, nullptr, DI_NORMAL);

					DestroyIcon(icon);
				}
			}
			else
			{
				RECT placeholderRect;
				SetRect(&placeholderRect, x, y_icon, x + this->cbxProperty.iconSquareSize, y_icon + this->cbxProperty.iconSquareSize);

				HBRUSH placeholderBrush;
				
				if(this->cbxControl.isEnabled)
					placeholderBrush = CreateSolidBrush(RGB(255, 0, 0));
				else
					placeholderBrush = CreateSolidBrush(RGB(150, 150, 150));

				if (placeholderBrush)
				{
					FillRect(hdc, &placeholderRect, placeholderBrush);

					DeleteObject(placeholderBrush);
				}
			}
			DeleteObject(bkgndBrush);
		}
		EndPaint(hWnd, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CustomCheckbox::onLButton(int xPos, int yPos)
{
	if (this->cbxControl.isEnabled)
	{
		bool isTouchDownValid = false;

		if (this->cbxControl.interactionMode == CBX_INTERACTRECT_ALL)
		{
			isTouchDownValid = true;
		}
		else
		{
			if ((xPos > this->cbxProperty.boxRect.left) &&
				(xPos < this->cbxProperty.boxRect.right) &&
				(yPos > this->cbxProperty.boxRect.top) &&
				(yPos < this->cbxProperty.boxRect.bottom))
			{
				isTouchDownValid = true;
			}
		}
		if (isTouchDownValid)
		{
			if (this->cbxControl.isChecked)
				this->cbxControl.isChecked = false;
			else
				this->cbxControl.isChecked = true;

			RedrawWindow(this->Checkbox, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);

			if (this->eventHandler != nullptr)
			{
				__try
				{
					this->eventHandler->onCustomCheckboxChecked(
						reinterpret_cast<cObject>(this), this->cbxControl.isChecked);
				}
				__except (
					GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
					? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
				{
					return static_cast<LRESULT>(0);
				}
			}
		}
	}
	return static_cast<LRESULT>(0);
}
