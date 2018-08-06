#include"SingleLineEdit.h"
#include"..//HelperF.h"
#include"..//BasicFPO.h"
#include<TextServ.h>

singleLineEdit::singleLineEdit(HINSTANCE hInst)
	:hInstance(hInst),
	eventHandler(nullptr),
	edit(nullptr),
	parent(nullptr),
	changePreventer(TRUE)
{
	this->setDefaultValues();
}

singleLineEdit::~singleLineEdit()
{
	if (this->Properties.ctrlFont != nullptr)
	{
		DeleteObject(this->Properties.ctrlFont);
	}
}

void singleLineEdit::setEventHandler(singleLineEditEventSink * handler)
{
	this->eventHandler = handler;
}

void singleLineEdit::setDimensions(int x, int y, int cx)
{
	this->Properties.xPos = x;
	this->Properties.yPos = y;
	this->Properties.cx = cx;
}

void singleLineEdit::setType(DWORD type, int descriptionWidth)
{
	this->Properties.type = type;
	this->Properties.descriptionWidth = descriptionWidth;
}

void singleLineEdit::setEditFontProperties(iString fontFamily, int textheight, int correctedControlHeight)
{
	this->editFontFamily = fontFamily;
	this->Properties.cy = correctedControlHeight;
	this->Properties.textHeight = textheight;
}

void singleLineEdit::setCtrlFont(HFONT font)
{
	this->Properties.ctrlFont = font;
}

void singleLineEdit::setBorder(bool draw, COLORREF borderColor)
{
	this->Properties.drawBorder = draw;
	this->Properties.borderColor = borderColor;
}

void singleLineEdit::setColors(COLORREF framebackground, COLORREF editBackground, COLORREF textcolor, COLORREF editTextcolor)
{
	this->Properties.framebackground = framebackground;
	this->Properties.editboxbackground = editBackground;
	this->Properties.textcolor = textcolor;
	this->Properties.editTextcolor = editTextcolor;
}

void singleLineEdit::setErrorColors(COLORREF errorFillColor, COLORREF errorBorderColor)
{
	this->Properties.errorFillColor = errorFillColor;
	this->Properties.errorBorderColor = errorBorderColor;
}

void singleLineEdit::setAlignment(DWORD alignment)
{
	this->Properties.alignment = alignment;
}

void singleLineEdit::setAdditionalEditStyles(DWORD editStyles)
{
	this->Properties.editStyles = editStyles;
}

void singleLineEdit::setSpacing(int spacing)
{
	this->Properties.spacing = spacing;
}

void singleLineEdit::setTabulatorAcceptance(bool acceptTabs)
{
	this->Properties.acceptTabs = acceptTabs;
}

void singleLineEdit::setDescriptionText(iString text)
{
	this->descriptionText = text;
}

void singleLineEdit::setContent(iString text)
{
	this->editContent = text;

	if (this->edit != nullptr)
	{
		SetRichEditContent(this->edit, text.getContentReference(), ST_DEFAULT);
		this->setCursorToIndex(SLE_CURSORTOTEXTEND);
	}
}

void singleLineEdit::setContent(LPTSTR text)
{
	this->editContent.Replace(text);
	this->setContent(this->editContent);
}

void singleLineEdit::setContentWithEventBlocker(iString text)
{
	this->enableChangeBlocker();
	this->setContent(text);
	this->disableChangeBlocker();
}

void singleLineEdit::setContentWithEventBlocker(LPTSTR text)
{
	this->enableChangeBlocker();
	this->setContent(text);
	this->disableChangeBlocker();
}

iString singleLineEdit::getContent()
{
	updateContentHolder();

	return this->editContent;
}

void singleLineEdit::selectAll()
{
	if (this->edit != nullptr)
	{
		CHARRANGE cr;
		cr.cpMin = 0;
		cr.cpMax = -1;
		SendMessage(this->edit, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&cr));
	}
}

void singleLineEdit::setFocus()
{
	SetFocus(this->edit);
}

void singleLineEdit::setCursorToIndex(int index)
{
	CHARRANGE cr;

	if (index == SLE_CURSORTOTEXTEND)
	{
		auto len = this->editContent.GetLength();
		cr.cpMin = len;
		cr.cpMax = len;
	}
	else
	{
		cr.cpMax = index;
		cr.cpMin = index;
	}
	SendMessage(this->edit, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&cr));
}

HRESULT singleLineEdit::Create(HWND Parent, DWORD ctrlID)
{
	HRESULT hr = (Parent != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		this->parent = Parent;
		this->Properties.ctrlID = ctrlID;

		this->registerSLEClass();

		this->frame
			= CreateWindow(
				SLE_CTRLCLASS,
				nullptr,
				WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
				this->Properties.xPos,
				this->Properties.yPos,
				this->Properties.cx,
				this->Properties.cy,
				Parent,
				IDTOHMENU(ctrlID),
				this->hInstance,
				reinterpret_cast<LPVOID>(this)
			);

		hr = (this->frame != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			//LoadLibrary(L"Msftedit.dll");

			this->calculateEditBoxPosition();

			this->edit
				= CreateWindow(
					MSFTEDIT_CLASS,
					nullptr,
					WS_CHILD | WS_VISIBLE | this->Properties.editStyles,
					this->Properties.editboxPosition.x,
					this->Properties.editboxPosition.y,
					this->Properties.editboxSize.cx,
					this->Properties.editboxSize.cy,
					this->frame, nullptr,
					this->hInstance, nullptr
				);

			hr = (this->edit != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				hr =
					SetWindowSubclass(this->edit, singleLineEdit::editSub, 0, reinterpret_cast<DWORD_PTR>(this))
					? S_OK : E_FAIL;

				if (SUCCEEDED(hr))
				{
					CHARFORMAT cf;

					initCharformat(&cf, this->Properties.textHeight, this->Properties.editTextcolor, this->editFontFamily.getContentReference());

					SendMessage(this->edit, EM_SETBKGNDCOLOR, 0, static_cast<LPARAM>(this->Properties.editboxbackground));
					SendMessage(this->edit, EM_SETCHARFORMAT, static_cast<WPARAM>(SCF_ALL), reinterpret_cast<LPARAM>(&cf));
					
					SendMessage(this->edit, EM_SETEVENTMASK, 0, static_cast<LPARAM>(ENM_CHANGE));
					
					// finally disable change blocker
					this->changePreventer = FALSE;
				}
			}
		}
	}
	return hr;
}

void singleLineEdit::restrictedContent(int restrictMode)
{
	this->Properties.restrictType = restrictMode;
}

void singleLineEdit::setValidContentRange(int min, int max)
{
	this->Properties.exFlags |= SLE_EX_CHECK_VALIDRANGE;

	this->Properties.max_range = max;
	this->Properties.min_range = min;
}

void singleLineEdit::setExtendedFlag(DWORD flag)
{
	this->Properties.exFlags |= flag;
}

singleLineEdit * singleLineEdit::GetSingleLineEditObject(HWND host, int ID)
{
	auto wnd = GetDlgItem(host, ID);
	if (wnd != nullptr)
	{
		return
			reinterpret_cast<singleLineEdit*>(
				GetWindowLongPtr(wnd, GWLP_USERDATA)
				);
	}
	return nullptr;
}

LRESULT singleLineEdit::sleFrameProc(HWND frame, UINT message, WPARAM wParam, LPARAM lParam)
{
	singleLineEdit* sle = nullptr;

	if (message == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		sle = reinterpret_cast<singleLineEdit*>(pcr->lpCreateParams);
		SetWindowLongPtr(frame, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(sle));

		return 1;
	}
	else
	{
		sle = reinterpret_cast<singleLineEdit*>(GetWindowLongPtr(frame, GWLP_USERDATA));

		if (sle != nullptr)
		{
			switch (message)
			{
			case WM_COMMAND:
				return sle->onCommand(frame, wParam, lParam);
			case WM_PAINT:
				return sle->onPaintInFrame(frame);
			case WM_DESTROY:
				SafeRelease(&sle);
				return 0;
			default:
				break;
			}
		}
		return DefWindowProc(frame, message, wParam, lParam);
	}
}

LRESULT singleLineEdit::editSub(HWND edit, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR refData)
{
	UNREFERENCED_PARAMETER(uID);

	auto sle = reinterpret_cast<singleLineEdit*>(refData);

	if (sle != nullptr)
	{
		if (msg == WM_CHAR)
		{
			if (wParam == 0x09)// TAB was pressed
			{
				if (sle->eventHandler != nullptr)
				{
					sle->eventHandler->onTabKeyWasPressed(
						reinterpret_cast<cObject>(sle),
						sle->Properties.ctrlID
					);
				}
				if (!sle->Properties.acceptTabs)// tab key not accepted -> discard
				{
					return 0;
				}
			}
		}
		else
		{
			if (msg == WM_MOUSEWHEEL)
			{
				// bypass mousewheel messages
				SendMessage(sle->parent, msg, wParam, lParam);
			}
		}
	}
	return DefSubclassProc(edit, msg, wParam, lParam);
}

LRESULT singleLineEdit::onPaintInFrame(HWND Frame)
{
	HDC hdc;
	PAINTSTRUCT ps;

	RECT rc;
	GetClientRect(Frame, &rc);

	hdc = BeginPaint(Frame, &ps);
	if (hdc)
	{
		HBRUSH bkgnd = CreateSolidBrush(this->Properties.framebackground);
		if (bkgnd)
		{
			FillRect(hdc, &rc, bkgnd);

			DeleteObject(bkgnd);
		}
		if (this->Properties.drawBorder)
		{
			HPEN pen;

			if (this->Properties.contentInvalid)
				pen = CreatePen(PS_SOLID, 1, this->Properties.errorBorderColor);
			else
				pen = CreatePen(PS_SOLID, 1, this->Properties.borderColor);

			if (pen)
			{
				HGDIOBJ origin = SelectObject(hdc, pen);

				MoveToEx(hdc, this->Properties.descriptionWidth - 1, 1, nullptr);
				LineTo(hdc, this->Properties.cx - 1, 1);
				LineTo(hdc, this->Properties.cx - 1, this->Properties.cy - 2);
				LineTo(hdc, this->Properties.descriptionWidth - 1, this->Properties.cy - 2);
				LineTo(hdc, this->Properties.descriptionWidth - 1, 1);

				SelectObject(hdc, origin);
				DeleteObject(pen);
			}
		}

		if (this->Properties.type == SLE_TYPE_WITHDESCRIPTION)
		{
			int c = this->descriptionText.GetLength();

			if (c > 0)
			{
				SIZE textsize;
				POINT pos = { 0,0 };
				HGDIOBJ original = SelectObject(hdc, this->Properties.ctrlFont);

				if (GetTextExtentPoint32(hdc, this->descriptionText.GetData(), c, &textsize))
				{
					pos.y = (this->Properties.cy / 2) - (textsize.cy / 2);

					switch (this->Properties.alignment)
					{
					case DESC_ALIGN_LEFT:
						pos.x = this->Properties.spacing;
						break;
					case DESC_ALIGN_CENTER:
						pos.x = (this->Properties.descriptionWidth / 2) - (textsize.cx / 2);
						break;
					case DESC_ALIGN_RIGHT:
						pos.x = this->Properties.descriptionWidth - (this->Properties.spacing + textsize.cx);
						break;
					default:
						break;
					}
					SetBkMode(hdc, TRANSPARENT);
					SetTextColor(hdc, this->Properties.textcolor);

					TextOut(hdc, pos.x, pos.y, this->descriptionText.GetData(), this->descriptionText.GetLength());

					SelectObject(hdc, original);
				}
			}
		}
		EndPaint(Frame, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT singleLineEdit::onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(lParam);

	if (HIWORD(wParam) == EN_CHANGE)
	{
		if (!this->changePreventer)
		{
			this->updateContentHolder();
			this->checkContentType();

			if (!(this->Properties.contentInvalid && (this->Properties.exFlags & SLE_EX_NOEVENT_ON_INVALIDCONTENT)))
			{
				if (this->eventHandler != nullptr)
				{
					this->eventHandler->onEditboxContentChanged(
						reinterpret_cast<cObject>(this),
						this->Properties.ctrlID
					);
				}
			}
		}
	}
	return static_cast<LRESULT>(0);
}

void singleLineEdit::setDefaultValues()
{
	SecureZeroMemory(&this->Properties, sizeof(SINGLELINEEDITPROPERTIES));

	this->Properties.cx = 200;
	this->Properties.cy = 24;
	this->Properties.spacing = 0;
	this->Properties.descriptionWidth = 1;
	this->Properties.textHeight = 15;
	this->Properties.type = SLE_TYPE_NODESCRIPTION;
	this->Properties.restrictType = 0;
	this->Properties.contentInvalid = false;
	this->Properties.acceptTabs = false;
	this->Properties.alignment = DESC_ALIGN_LEFT;
	this->Properties.ctrlFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Segoe UI\0");
	this->Properties.errorFillColor = RGB(255, 100, 100);
	this->Properties.errorBorderColor = RGB(255, 0, 0);
	this->Properties.editStyles = ES_AUTOHSCROLL;
	this->Properties.exFlags = 0;

	this->editFontFamily.Replace(L"Segoe UI\0");
}

void singleLineEdit::registerSLEClass()
{
	WNDCLASSEX wcx;

	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = sizeof(LONG_PTR);
	wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcx.lpfnWndProc = singleLineEdit::sleFrameProc;
	wcx.hbrBackground = nullptr;
	wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcx.hIcon = nullptr;
	wcx.hIconSm = nullptr;
	wcx.hInstance = this->hInstance;
	wcx.lpszClassName = SLE_CTRLCLASS;
	wcx.lpszMenuName = nullptr;

	RegisterClassEx(&wcx);
}

//void singleLineEdit::calculateEditBoxPositionWithDescription()
//{
//	RECT rc;
//	GetClientRect(this->frame, &rc);
//
//	this->Properties.editboxPosition.x = this->Properties.descriptionWidth;
//	this->Properties.editboxPosition.y = 2;
//	this->Properties.editboxSize.cx = (rc.right - this->Properties.descriptionWidth) -1;	// minus 1 because of the border which may will be drawn
//	this->Properties.editboxSize.cy = rc.bottom - 4;
//}

void singleLineEdit::calculateEditBoxPosition()
{
	RECT rc;
	GetClientRect(this->frame, &rc);

	this->Properties.editboxPosition.x = this->Properties.descriptionWidth;
	this->Properties.editboxPosition.y = 2;
	this->Properties.editboxSize.cx = (rc.right - this->Properties.descriptionWidth) - 1;	// minus 1 because of the border which may will be drawn
	this->Properties.editboxSize.cy = rc.bottom - 4;
}

void singleLineEdit::updateContentHolder()
{
	TCHAR* text = nullptr;

	if (GetRichEditContent(this->edit, &text) == TRUE)
	{
		this->editContent.Replace(text);

		SafeDeleteArray(&text);
	}
}

void singleLineEdit::checkContentType()
{
	if (this->Properties.restrictType == SLE_RESTRICTTYPE_ONLYNUMBERS)
	{
		bool isInvalid = false;
		auto data = this->editContent.GetData();
		auto max = this->editContent.GetLength();

		for (int i = 0; i < max; i++)
		{
			if (!IsNumber(data[i]))
			{
				isInvalid = true;
				break;
			}
		}
		if (!isInvalid)
		{
			// if the number is valid -> check range!
			if (this->Properties.exFlags & SLE_EX_CHECK_VALIDRANGE)
			{
				isInvalid =
					!IS_VALUE_IN_RANGE(
						_wtoi(data),
						this->Properties.min_range,
						this->Properties.max_range
					);
			}
		}

		if (isInvalid)
		{
			if (!this->Properties.contentInvalid)// draw invalid state
			{
				this->Properties.contentInvalid = true;
				RedrawWindow(this->frame, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
				SendMessage(this->edit, EM_SETBKGNDCOLOR, 0, static_cast<LPARAM>(this->Properties.errorFillColor));
			}
		}
		else
		{
			if (this->Properties.contentInvalid)// restore to valid state
			{
				this->Properties.contentInvalid = false;
				RedrawWindow(this->frame, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
				SendMessage(this->edit, EM_SETBKGNDCOLOR, 0, static_cast<LPARAM>(this->Properties.editboxbackground));
			}
		}
	}
	else if (this->Properties.restrictType == SLE_RESTRICTTYPE_FORFILENAMES)
	{
		bool isInvalid = false;
		//auto data = this->editContent.GetData();
		auto max = this->editContent.GetLength();

		for (int i = 0; i < max; i++)
		{
			auto c = this->editContent.getCharAt(i);
			
			int j = 0;
			while (forbiddenLiterals[j] != L'\0')
			{
				if (c == forbiddenLiterals[j])
				{
					isInvalid = true;
					break;
				}
				j++;
			}
		}
		if (isInvalid)
		{
			if (!this->Properties.contentInvalid)
			{
				this->Properties.contentInvalid = true;
				RedrawWindow(this->frame, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
				SendMessage(this->edit, EM_SETBKGNDCOLOR, 0, static_cast<LPARAM>(this->Properties.errorFillColor));
			}
		}
		else
		{
			if (this->Properties.contentInvalid)
			{
				this->Properties.contentInvalid = false;
				RedrawWindow(this->frame, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
				SendMessage(this->edit, EM_SETBKGNDCOLOR, 0, static_cast<LPARAM>(this->Properties.editboxbackground));
			}
		}
	}
}
