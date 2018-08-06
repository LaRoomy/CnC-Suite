#include"CSConsole.h"
#include<strsafe.h>
#include<ctrlHelper.h>
#include"..//SafeRelease.h"
#include"..//HelperF.h"

// temp:
#include<StringClass.h>

CSConsole::CSConsole()
	: hInstance(nullptr),
	consoleWindow(nullptr),
	parentWindow(nullptr),
	cType(CSConsole::CONSOLETYPE_READONLY | CSConsole::CONSOLETYPE_SCROLLABLE),
	inputEvents(nullptr),
	VerticalScrollbar(nullptr),
	lineHeight(24),
	contentHeight(0),
	commandEntry(nullptr),
	margin_left(20),
	caretWidth(15),
	autoScroll(true),
	freezer(false),
	additionalStyles(0)
{
	this->setDefaultParameter();
	this->ContentManager.SetExecuteAsync(false);
}

CSConsole::CSConsole(HINSTANCE hInst)
	: hInstance(hInst),
	consoleWindow(nullptr),
	parentWindow(nullptr),
	cType(CSConsole::CONSOLETYPE_READONLY | CSConsole::CONSOLETYPE_SCROLLABLE),
	inputEvents(nullptr),
	VerticalScrollbar(nullptr),
	lineHeight(24),
	contentHeight(0),
	commandEntry(nullptr),
	margin_left(20),
	caretWidth(15),
	autoScroll(true),
	freezer(false),
	additionalStyles(0)
{
	this->setDefaultParameter();
	this->ContentManager.SetExecuteAsync(false);
}

CSConsole::~CSConsole()
{
	DeleteObject(this->ctrlFont);
	DeleteObject(this->backgroundBrush);

	SafeDeleteArray(&this->commandEntry);
}

const TCHAR* CSConsole::CONSOLE_CLASS_ID = L"ICONSOLE_CLASS_IDENTIFIER";

HRESULT CSConsole::Create(HWND Parent, LPPOINT pos, LPSIZE size, DWORD CtrlID)
{
	auto hr =
		((Parent != nullptr)&& (pos != nullptr)&& (size != nullptr))
		? S_OK : E_INVALIDARG;

	if (SUCCEEDED(hr))
	{
		this->consolePosition = *pos;
		this->consoleSize = *size;

		this->ctrlID = CtrlID;
		this->parentWindow = Parent;

		hr = this->registerConsoleClass();
		if (SUCCEEDED(hr))
		{
			if (this->commandEntry != nullptr)
			{
				this->ContentManager.AddLine(this->commandEntry);
			}
			else
			{
				this->ContentManager.AddLine(L"");
			}
			this->lineColor.AddItem(this->defaultTextColor);

			this->consoleWindow =
				CreateWindow(
					CSConsole::CONSOLE_CLASS_ID,
					nullptr,
					WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | this->additionalStyles,
					pos->x,
					pos->y,
					size->cx,
					size->cy,
					Parent,
					IDTOHMENU(CtrlID),
					this->hInstance,
					reinterpret_cast<LPVOID>(this)
				);

			hr = (this->consoleWindow != nullptr)
				? S_OK : E_FAIL;

			if (SUCCEEDED(hr))
			{
				this->updateContentHeight();

				this->VerticalScrollbar = new CSScrollbar(this->hInstance);

				hr = (this->VerticalScrollbar != nullptr) ? S_OK : E_FAIL;
				if (SUCCEEDED(hr))
				{
					this->VerticalScrollbar->SetEventHandler(
						dynamic_cast<IScrollEventProtocol*>(this)
					);
					this->VerticalScrollbar->SetScrollbarType(
						CSScrollbar::SCROLLBARTYPE_VERTICAL
					);
					this->VerticalScrollbar->SetImages(IDI_SYS_SCROLLARROW_DOWN, IDI_SYS_SCROLLARROW_UP, 16);
					this->VerticalScrollbar->SetStyleImages(IDI_SYS_SCROLLARROW_DOWN_HGL, IDI_SYS_SCROLLARROW_DOWN_PRS, IDI_SYS_SCROLLARROW_UP_HGL, IDI_SYS_SCROLLARROW_UP_PRS);
					this->VerticalScrollbar->SetDisabledImages(IDI_SYS_SCROLLARROW_DOWN_DSBL, IDI_SYS_SCROLLARROW_UP_DSBL);

					hr = this->VerticalScrollbar->Create(this->consoleWindow, nullptr, nullptr);
					if (SUCCEEDED(hr))
					{
						if(!(this->cType & CSConsole::CONSOLETYPE_SCROLLABLE))
							this->VerticalScrollbar->SetShowState(false);

						this->updateCaretRect();
						SetFocus(this->consoleWindow); // this starts also the timer for the caret!
					}
				}
			}
		}
	}
	return hr;
}

void CSConsole::SetColors(COLORREF background, COLORREF defaultText)
{
	this->backgroundColor = background;
	this->defaultTextColor = defaultText;

	if (this->backgroundBrush != nullptr)
		DeleteObject(this->backgroundBrush);

	this->backgroundBrush = CreateSolidBrush(background);

	if (this->caretBrush != nullptr)
		DeleteObject(this->caretBrush);

	this->caretBrush = CreateSolidBrush(defaultText);
}

bool CSConsole::AddLine(LPCWSTR data)
{
	if (data != nullptr)
	{
		this->ContentManager.AddLine(data);
		this->updateContentHeight();

		if(this->cType & CSConsole::CONSOLETYPE_MULTICOLOR)
			this->lineColor.AddItem(this->defaultTextColor);

		if (this->consoleWindow != nullptr)
		{
			RECT rc;
			GetClientRect(this->consoleWindow, &rc);

			if (this->contentHeight > rc.bottom)
			{
				this->VerticalScrollbar->SetScrollRange(this->contentHeight - rc.bottom);

				if (this->autoScroll)
				{
					this->VerticalScrollbar->UpdateThumb(
						this->VerticalScrollbar->GetScrollRange()
					);
				}
			}
		}
		return true;
	}
	return false;
}

bool CSConsole::AddLine(LPCWSTR data, COLORREF linecolor)
{
	if (data != nullptr)
	{
		this->ContentManager.AddLine(data);

		this->lineColor.AddItem(linecolor);

		this->updateContentHeight();

		if (this->consoleWindow != nullptr)
		{
			RECT rc;
			GetClientRect(this->consoleWindow, &rc);

			if (this->contentHeight > rc.bottom)
			{
				this->VerticalScrollbar->SetScrollRange(this->contentHeight - rc.bottom);

				if (this->autoScroll)
				{
					this->VerticalScrollbar->UpdateThumb(
						this->VerticalScrollbar->GetScrollRange()
					);
				}
			}
		}
		return true;
	}
	return false;
}

bool CSConsole::AddEmptyLine()
{
	return 
		(this->cType & CSConsole::CONSOLETYPE_MULTICOLOR)
		? this->AddLine(EditorContentManager::EMPTY_LINE, this->defaultTextColor)
		:this->AddLine(EditorContentManager::EMPTY_LINE);
}

bool CSConsole::AddCharacterToLine(WCHAR data)
{
	if(this->consoleWindow == nullptr)
		return false;
	else
	{
		this->ContentManager.AddCharToLine(
			data,
			this->ContentManager.GetLineCount() - 1
		);

		this->eraseCaret();
		this->Update();
		
		return true;
	}
}

void CSConsole::Update()
{
	this->updateCaretRect();
	RedrawWindow(this->consoleWindow, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW);
}

void CSConsole::Clear()
{
	this->ContentManager.Clear();
	this->lineColor.Clear();
	this->updateContentHeight();
	this->VerticalScrollbar->SetScrollPosition(0);
	this->VerticalScrollbar->SetScrollRange(0);
	this->Update();
}

void CSConsole::Move(int x, int y, int cx, int cy)
{
	if (MoveWindow(this->consoleWindow, x, y, cx, cy, TRUE))
	{
		this->consolePosition.x = x;
		this->consolePosition.y = y;
		this->consoleSize.cx = cx;
		this->consoleSize.cy = cy;
	}
}

int CSConsole::GetLineSize(ScrollBarType type)
{
	if (type == CSScrollbar::SCROLLBARTYPE_VERTICAL)
		return this->lineHeight;
	else
		return 20; // a horizontal scrollbar is not implemented yet, so return a placeholder value
}

int CSConsole::GetContentHeight()
{
	return this->ContentManager.GetLineCount() * this->lineHeight;
}

void CSConsole::SetLineHeight(int lHgt)
{
	if (lHgt > 9)
	{
		this->lineHeight = lHgt;
	}
}

void CSConsole::SetFontProperty(int height, int weight, LPCTSTR faceName)
{
	DeleteObject(this->ctrlFont);

	LOGFONT lf;
	SecureZeroMemory(&lf, sizeof(LOGFONT));

	lf.lfHeight = (LONG)height;
	lf.lfQuality = CLEARTYPE_QUALITY;
	lf.lfWeight = weight;
	StringCbCopy(lf.lfFaceName, sizeof(TCHAR)* LF_FACESIZE, faceName);

	this->ctrlFont = CreateFontIndirect(&lf);
}

void CSConsole::SetFontDirect(HFONT font)
{
	DeleteObject(this->ctrlFont);
	this->ctrlFont = font;
}

void CSConsole::SetCommandEntry(LPCWSTR entry)
{
	if (entry != nullptr)
	{
		SafeDeleteArray(&this->commandEntry);
		CopyStringToPtr(entry, &this->commandEntry);
	}
}

void CSConsole::SetLeftMargin(int marginLeft)
{
	if (marginLeft >= 0)
		this->margin_left = marginLeft;
}

void CSConsole::SetCaretWidth(int cWidth)
{
	if (cWidth > 0)
	{
		this->caretWidth = cWidth;
	}
}

void CSConsole::SetAutoScroll(bool autoscroll)
{
	this->autoScroll = autoscroll;
}

HRESULT CSConsole::registerConsoleClass()
{
	HRESULT hr = S_OK;
	WNDCLASSEX wcx;
	
	if (GetClassInfoEx(this->hInstance, CSConsole::CONSOLE_CLASS_ID, &wcx) == 0)
	{
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wcx.lpfnWndProc = CSConsole::consoleProc;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = sizeof(LONG_PTR);
		wcx.hInstance = this->hInstance;
		wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcx.hIcon = nullptr;
		wcx.hIconSm = nullptr;
		wcx.lpszClassName = CSConsole::CONSOLE_CLASS_ID;
		wcx.lpszMenuName = nullptr;
		wcx.hbrBackground = nullptr;
		
		hr = (RegisterClassEx(&wcx) == 0)
			? E_FAIL : S_OK;
	}
	return hr;
}

LRESULT CSConsole::consoleProc(HWND hWndConsole, UINT message, WPARAM wParam, LPARAM lParam)
{
	CSConsole* csConsole = nullptr;

	if (message == WM_CREATE)
	{
		auto pCr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		if (pCr != nullptr)
		{
			csConsole = reinterpret_cast<CSConsole*>(pCr->lpCreateParams);
			if (csConsole != nullptr)
			{
				SetWindowLongPtr(hWndConsole, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(csConsole));
			}
		}
		return static_cast<LRESULT>(TRUE);
	}
	else
	{
		csConsole = reinterpret_cast<CSConsole*>(GetWindowLongPtr(hWndConsole, GWLP_USERDATA));
		if (csConsole != nullptr)
		{
			switch (message)
			{
			case WM_MOUSEWHEEL:
				return csConsole->onMouseWheel(wParam);
			case WM_LBUTTONDOWN:
				return csConsole->onLButtonDown();
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_PAINT:
				return csConsole->onPaint(hWndConsole);
			case WM_SIZE:
				return csConsole->onSize();
			case WM_CHAR:
				return csConsole->onChar(wParam);
			case WM_SETFOCUS:
				return csConsole->onSetFocus();
			case WM_KILLFOCUS:
				return csConsole->onKillFocus();
			case WM_DESTROY:
				csConsole->Release();
				return 0;
			case WM_TIMER:
				return csConsole->onTimer(wParam);
			default:
				break;
			}
		}
		return DefWindowProc(hWndConsole, message, wParam, lParam);
	}
}

LRESULT CSConsole::onPaint(HWND console)
{
	HDC hdc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(console, &ps);
	if (hdc)
	{
		this->draw(hdc);

		EndPaint(console, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CSConsole::onSize()
{
	//resize scrollbar
	if (this->VerticalScrollbar != nullptr)
	{
		RECT rc;
		GetClientRect(this->consoleWindow, &rc);

		this->updateContentHeight();
		this->VerticalScrollbar->SetScrollRange(this->contentHeight - rc.bottom);
		this->VerticalScrollbar->Resize();
	}

	//if the window-height changes, the scrollposition must possibly be changed too ??????? (the range does not change - the window changes, not the content!)

	return static_cast<LRESULT>(0);
}

LRESULT CSConsole::onMouseWheel(WPARAM wParam)
{
	if (this->cType & CSConsole::CONSOLETYPE_SCROLLABLE)
	{
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (zDelta > 0)
		{
			// wheel minus
			this->VerticalScrollbar->SetScrollPosition(
				this->VerticalScrollbar->GetScrollPosition()
				- (2 * this->GetLineSize(CSScrollbar::SCROLLBARTYPE_VERTICAL)),
				true
			);
			this->Update();
		}
		else if (zDelta < 0)
		{
			// wheel plus
			this->VerticalScrollbar->SetScrollPosition(
				this->VerticalScrollbar->GetScrollPosition()
				+ (2 * this->GetLineSize(CSScrollbar::SCROLLBARTYPE_VERTICAL)),
				true
			);
			this->Update();
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CSConsole::onLButtonDown()
{
	SetFocus(this->consoleWindow);

	return static_cast<LRESULT>(0);
}

LRESULT CSConsole::onTimer(WPARAM wParam)
{
	if (wParam == CSConsole::CRT_BLINKTIMER_ID)
	{
		HDC hdc = GetDC(this->consoleWindow);
		if (hdc)
		{
			if (this->CaretProperty.isVisible)
			{
				FillRect(hdc, &this->CaretProperty.cArea, this->backgroundBrush);
				this->CaretProperty.isVisible = false;
			}
			else
			{
				FillRect(hdc, &this->CaretProperty.cArea, this->caretBrush);
				this->CaretProperty.isVisible = true;
			}
			ReleaseDC(this->consoleWindow, hdc);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CSConsole::onChar(WPARAM wParam)
{
	if (this->cType & CSConsole::CONSOLETYPE_ACCEPTINPUT)
	{
		switch (wParam)
		{
		case 0x08:
			this->onBackspace();
			break;
		case 0x0D:
			this->onLinefeed();
			break;
		default:
			this->AddCharacterToLine((WCHAR)wParam);
			break;
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CSConsole::onKillFocus()
{
	this->hasFocus = false;
	this->controlTimer(false);
	this->eraseCaret();
	return static_cast<LRESULT>(0);
}

LRESULT CSConsole::onSetFocus()
{
	this->hasFocus = true;
	this->drawCaret();
	this->controlTimer(true);
	return static_cast<LRESULT>(0);
}

void CSConsole::onBackspace()
{
	auto lastLineIndex = this->ContentManager.GetLineCount() - 1;

	this->ContentManager.RemoveCharFromLine(
		lastLineIndex,
		this->ContentManager.GetLineLength(lastLineIndex) - 1
	);

	this->eraseCaret();
	this->Update();
}

void CSConsole::onLinefeed()
{
	if (this->freezer)
		return;

	bool addEntry = true;

	if (this->inputEvents != nullptr)
	{
		WCHAR* command = nullptr;

		auto lastLineIndex = this->ContentManager.GetLineCount() - 1;
		auto lastLine = this->ContentManager.GetLine(lastLineIndex);
		auto lastLineLenght = this->ContentManager.GetLineLength(lastLineIndex) + 1;

		bool validCommand = false;

		if (commandEntry != nullptr)
		{
			if (CompareStringsAsmW(lastLine, this->commandEntry) == 0)
			{
				// remove the commandentry from the beginning of the string
				auto entryLenght = GetStringLengthW(this->commandEntry);
				if (entryLenght > 0)
				{
					int newLen = (lastLineLenght - entryLenght) + 1;
					if (newLen > 0)
					{
						command = new WCHAR[newLen];
						if (command != nullptr)
						{
							if (RemoveBlockFromBufferW(command, lastLine, 0, entryLenght - 2) > 0)
							{
								validCommand = true;
							}
						}
					}
				}
			}
		}
		else
		{
			if (_lengthOfString(lastLine) > 0)
			{
				CopyStringToPtr(lastLine, &command);
				validCommand = true;
			}
		}

		if (validCommand)
		{
			if (!this->inputEvents->UserEnteredCommand(reinterpret_cast<cObject>(this), command))
			{
				addEntry = false;
			}
		}
		else
		{
			if (!this->inputEvents->UserPressedReturn(reinterpret_cast<cObject>(this)))
			{
				addEntry = false;
			}
		}
		SafeDeleteArray(&command);
	}

	if (this->freezer)
		return;

	if(addEntry)
	{
		if (this->cType & CSConsole::CONSOLETYPE_MULTICOLOR)
		{
			if (this->commandEntry != nullptr)
				this->AddLine(this->commandEntry, this->defaultTextColor);
			else
				this->AddLine(L"", this->defaultTextColor);
		}
		else
		{
			if (this->commandEntry != nullptr)
				this->AddLine(this->commandEntry);
			else
				this->AddLine(L"");
		}
	}

	this->eraseCaret();
	this->Update();
}

void CSConsole::draw(HDC hdc)
{
	HDC offscreenDC = CreateCompatibleDC(hdc);
	if (offscreenDC)
	{
		RECT rcClient;
		GetClientRect(this->consoleWindow, &rcClient);

		HBITMAP offscreenBmp = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
		if (offscreenBmp)
		{
			auto originBmp = SelectObject(offscreenDC, offscreenBmp);

			// erase the offscreen
			FillRect(offscreenDC, &rcClient, this->backgroundBrush);

			auto originFont = SelectObject(offscreenDC, this->ctrlFont);

			SetTextColor(offscreenDC, this->defaultTextColor);
			SetBkMode(offscreenDC, TRANSPARENT);

			SIZE sz;
			int lineStartY = 0;
			auto startLine = this->getFirstVisibleLineIndex(&lineStartY);
			auto lineCount = this->ContentManager.GetLineCount();
			auto lineheight = this->GetLineSize(CSScrollbar::SCROLLBARTYPE_VERTICAL);

			if ((lineCount > 0) && (startLine < lineCount))
			{
				if (lineStartY != 0)
					lineStartY = -(lineStartY);

				for (int i = startLine; i < lineCount; i++)
				{
					if (!CompareStringsAsmW(
						this->ContentManager.GetLine(i),
						EditorContentManager::EMPTY_LINE))
					{
						if (this->cType & CSConsole::CONSOLETYPE_MULTICOLOR)
						{
							SetTextColor(
								offscreenDC,
								this->getLineColor(i)
							);
						}
						GetTextExtentPoint32(
							offscreenDC,
							this->ContentManager.GetLine(i),
							this->ContentManager.GetLineLength(i),
							&sz
						);
						TextOut(
							offscreenDC,
							margin_left,
							lineStartY + ((lineheight / 2) - (sz.cy / 2)),
							this->ContentManager.GetLine(i),
							this->ContentManager.GetLineLength(i)
						);
					}
					lineStartY += lineheight;

					if (lineStartY > rcClient.bottom)
						break;
				}
				BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, offscreenDC, 0, 0, SRCCOPY);
			}
			SelectObject(offscreenDC, originFont);
			SelectObject(offscreenDC, originBmp);
			DeleteObject(offscreenBmp);
		}
		DeleteDC(offscreenDC);
	}
}

int CSConsole::getFirstVisibleLineIndex(int* invisibleHeight)
{
	auto scrollPos = this->VerticalScrollbar->GetScrollPosition();
	if (scrollPos >= 0)
	{
		int lineIndex = (scrollPos / this->GetLineSize(CSScrollbar::SCROLLBARTYPE_VERTICAL));

		if (invisibleHeight != nullptr)
		{
			*invisibleHeight = scrollPos - (lineIndex * this->GetLineSize(CSScrollbar::SCROLLBARTYPE_VERTICAL));
		}
		return lineIndex;
	}
	return 0;
}

void CSConsole::setDefaultParameter()
{
	SecureZeroMemory(&this->CaretProperty, sizeof(CARETPROPERTY));

	this->backgroundColor = RGB(30, 30, 30);
	this->backgroundBrush = CreateSolidBrush(this->backgroundColor);

	this->defaultTextColor = RGB(230, 230, 230);
	this->caretBrush = CreateSolidBrush(this->defaultTextColor);
}

void CSConsole::eraseCaret()
{
	HDC hdc = GetDC(this->consoleWindow);
	if (hdc)
	{
		FillRect(hdc, &this->CaretProperty.cArea, this->backgroundBrush);
		this->CaretProperty.isVisible = false;

		ReleaseDC(this->consoleWindow, hdc);
	}
}

void CSConsole::drawCaret()
{
	HDC hdc = GetDC(this->consoleWindow);
	if (hdc)
	{
		FillRect(hdc, &this->CaretProperty.cArea, this->caretBrush);
		this->CaretProperty.isVisible = true;

		ReleaseDC(this->consoleWindow, hdc);
	}
}

void CSConsole::updateCaretRect()
{
	// get y coordinate
	auto yPos =
		this->ContentManager.GetLineCount()
		* this->GetLineSize(CSScrollbar::SCROLLBARTYPE_VERTICAL);

	yPos -= this->VerticalScrollbar->GetScrollPosition();

	// get x coordinate
	SIZE sz = { 0 };

	HDC hdc = GetDC(this->consoleWindow);
	if (hdc)
	{
		HGDIOBJ origin = SelectObject(hdc, this->ctrlFont);

		auto lines = this->ContentManager.GetLineCount();
		if (lines > 0)
		{
			lines--;

			/*HGDIOBJ origin = SelectObject(hdc, this->ctrlFont);*/

			GetTextExtentPoint32(
				hdc,
				this->ContentManager.GetLine(lines),
				this->ContentManager.GetLineLength(lines),
				&sz
			);
		}
		else
		{ 
			GetTextExtentPoint32(
				hdc,
				L"1",
				1,
				&sz
			);
			sz.cx = 0;
		}

		auto xPos = this->margin_left + sz.cx + 2;

		this->CaretProperty.cArea.left = xPos;
		this->CaretProperty.cArea.right = xPos + this->caretWidth;

		this->CaretProperty.cArea.top =
			(yPos - (this->GetLineSize(CSScrollbar::SCROLLBARTYPE_VERTICAL) / 2))
			+ sz.cy / 2;
		this->CaretProperty.cArea.bottom = this->CaretProperty.cArea.top + 2;

		SelectObject(hdc, origin);
		ReleaseDC(this->consoleWindow, hdc);
	}
}

void CSConsole::updateContentHeight()
{
	RECT rc;
	GetClientRect(this->consoleWindow, &rc);

	this->contentHeight =
		this->ContentManager.GetLineCount()
		* this->GetLineSize(CSScrollbar::SCROLLBARTYPE_VERTICAL)
		+ rc.bottom / 2;
}

void CSConsole::controlTimer(bool enable)
{
	if (enable)
		SetTimer(this->consoleWindow, CSConsole::CRT_BLINKTIMER_ID, 500, nullptr);
	else
		KillTimer(this->consoleWindow, CSConsole::CRT_BLINKTIMER_ID);
}

COLORREF CSConsole::getLineColor(int index)
{
	auto lenOfColorCollection = this->lineColor.GetCount();
	if (index < lenOfColorCollection)
	{
		return this->lineColor.GetAt(index);
	}
	else
	{
		return this->defaultTextColor;
	}
}
