#include"textField.h"

Textfield::Textfield()
	:hInstance(nullptr),
	parent(nullptr),
	textfield(nullptr),
	textfieldId(Textfield::TF_DEFAULT_ID),
	dwFlags(Textfield::CF_APPLYSIZETOCONTEXT),
	padding_bottom(5),
	padding_left(5),
	padding_right(5),
	padding_top(5),
	additionalWindowStyles(0)
{
	this->textcolor = RGB(0, 0, 0);
	this->backgroundcolor = RGB(240, 240, 240);
	this->ctrlFont = _CREATE_FONT(18, FW_NORMAL, L"SegoeUI");
}

Textfield::~Textfield()
{
	if (this->ctrlFont != nullptr)
	{
		DeleteObject(this->ctrlFont);
	}
}

const TCHAR* Textfield::textFieldWindowClass = L"TEXTFIELDWINDOWCLASS";

HRESULT Textfield::Create(HINSTANCE hInst, HWND Parent, LPPOINT pos, LPSIZE size)
{
	HRESULT hr;

	hr =
		((hInst != nullptr) && (Parent != nullptr))
		? S_OK : E_FAIL;

	if (SUCCEEDED(hr))
	{
		this->hInstance = hInst;
		this->parent = Parent;

		int x = 0, y = 0, cx = 0, cy = 0;

		if (size != nullptr)
		{
			cx = size->cx;
			cy = size->cy;
		}
		else
		{
			if (this->dwFlags & Textfield::CF_USEFIXEDSIZE)
				hr = E_FAIL;
			else
			{
				// set placeholder sizes
				cx = 10;
				cy = 10;
			}
		}

		if (pos != nullptr)
		{
			x = pos->x;
			y = pos->y;
		}

		hr =
			(this->text.GetLength() > 0)
			? S_OK : E_FAIL;

		if (SUCCEEDED(hr))
		{
			hr = this->_registerTextfieldClass();
			if (SUCCEEDED(hr))
			{
				this->textfield =
					CreateWindow(
						Textfield::textFieldWindowClass,
						nullptr,
						WS_CHILD | WS_VISIBLE | additionalWindowStyles,
						x,
						y,
						cx,
						cy,
						Parent,
						IDTOHMENU(this->textfieldId),
						this->hInstance,
						reinterpret_cast<LPVOID>(this)
					);

				hr =
					(this->textfield != nullptr)
					? S_OK : E_FAIL;

				if (SUCCEEDED(hr))
				{
					// ...
				}
			}
		}
	}
	return hr;
}

void Textfield::Update()
{
	RedrawWindow(this->textfield, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
}

void Textfield::setPadding(int padding)
{
	this->padding_bottom = padding;
	this->padding_left = padding;
	this->padding_right = padding;
	this->padding_top = padding;
}

void Textfield::setPadding(int paddingX, int paddingY)
{
	this->padding_bottom = paddingY;
	this->padding_top = paddingY;
	this->padding_left = paddingX;
	this->padding_right = paddingX;
}

void Textfield::setPadding(int left, int top, int right, int bottom)
{
	this->padding_bottom = bottom;
	this->padding_left = left;
	this->padding_right = right;
	this->padding_top = top;
}

void Textfield::setColors(COLORREF ctext, COLORREF cbackground)
{
	this->textcolor = ctext;
	this->backgroundcolor = cbackground;
}

void Textfield::setFont(HFONT font)
{
	if (font != nullptr)
	{
		if (this->ctrlFont != nullptr)
		{
			DeleteObject(this->ctrlFont);
		}
		this->ctrlFont = font;
	}
}

bool Textfield::getDimensions(LPRECT window, LPRECT client)
{
	if (this->textfield != nullptr)
	{
		if (window != nullptr)
		{
			GetWindowRect(this->textfield, window);
		}
		if (client != nullptr)
		{
			GetClientRect(this->textfield, client);
		}
		return true;
	}
	return false;
}

HRESULT Textfield::_registerTextfieldClass()
{
	HRESULT hr = S_OK;
	WNDCLASSEX wcx;

	if (!GetClassInfoEx(this->hInstance, Textfield::textFieldWindowClass, &wcx))
	{
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = sizeof(LONG_PTR);
		wcx.lpfnWndProc = Textfield::textFieldProc;
		wcx.hInstance = this->hInstance;
		wcx.hIcon = nullptr;
		wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcx.hbrBackground = nullptr;
		wcx.lpszClassName = Textfield::textFieldWindowClass;
		wcx.lpszMenuName = nullptr;
		wcx.hIconSm = nullptr;

		hr = (RegisterClassEx(&wcx) != 0) ? S_OK : E_FAIL;
	}
	return hr;
}

LRESULT Textfield::textFieldProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Textfield* tf = nullptr;

	if (msg == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		if (pcr != nullptr)
		{
			tf = reinterpret_cast<Textfield*>(pcr->lpCreateParams);
			if (tf != nullptr)
			{
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(tf));
			}
		}
		return 1;
	}
	else
	{
		tf = reinterpret_cast<Textfield*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (tf != nullptr)
		{
			switch (msg)
			{
			case WM_PAINT:
				return tf->onPaint(hWnd);
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_DESTROY:
				SafeRelease(&tf);
				return 0;
			}
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

LRESULT Textfield::onPaint(HWND tWnd)
{
	bool applySize = false;

	int reqWndSizeX = 0;
	int reqWndSizeY = 0;

	HDC hdc;
	RECT rc;
	PAINTSTRUCT ps;

	GetClientRect(tWnd, &rc);

	hdc = BeginPaint(tWnd, &ps);
	if (hdc)
	{
		HDC hdcMem = CreateCompatibleDC(hdc);
		if (hdcMem)
		{
			HBITMAP offScreenBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			if (offScreenBmp)
			{
				HBRUSH bkgnd = CreateSolidBrush(this->backgroundcolor);
				if (bkgnd)
				{
					// bitmap to offscreen DC
					HGDIOBJ originBmp = SelectObject(hdcMem, offScreenBmp);

					// erase bitmap
					FillRect(hdcMem, &rc, bkgnd);

					int tLen = this->text.GetLength();
					if (tLen > 0)
					{
						SIZE sz;
						HGDIOBJ originFont = SelectObject(hdcMem, this->ctrlFont);

						SetBkMode(hdcMem, TRANSPARENT);
						SetTextColor(hdcMem, this->textcolor);

						GetTextExtentPoint32(hdcMem, this->text.GetData(), tLen, &sz);

						TextOut(hdcMem, this->padding_left, this->padding_top, this->text.GetData(), tLen);

						if (this->dwFlags & Textfield::CF_APPLYSIZETOCONTEXT)
						{
							reqWndSizeX =
								this->padding_left
								+ this->padding_right
								+ sz.cx;

							reqWndSizeY =
								this->padding_bottom
								+ this->padding_top
								+ sz.cy;

							if ((rc.right != reqWndSizeX) || (rc.bottom != reqWndSizeY))
								applySize = true;
						}

						BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

						SelectObject(hdcMem, originFont);
					}
					SelectObject(hdcMem, originBmp);
					DeleteObject(bkgnd);
				}
				DeleteObject(offScreenBmp);
			}
			DeleteDC(hdcMem);
		}
		EndPaint(tWnd, &ps);
	}
	if (applySize)
	{
		SetWindowPos(tWnd, nullptr, 0, 0, reqWndSizeX, reqWndSizeY, SWP_NOMOVE | SWP_NOZORDER);
	}
	return static_cast<LRESULT>(0);
}