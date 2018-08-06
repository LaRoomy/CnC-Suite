#include"ProgressDialog.h"
#include"..//HelperF.h"

ProgressDialog::ProgressDialog()
	: type(ProgressDialog::PROGRESSTYPE_PROGRESS | ProgressDialog::PROGRESSTYPE_POPUP | ProgressDialog::PROGRESSTYPE_WORKER),
	absoluteRange(376),
	currentRange(0),
	animationPosition(0),
	disableOwner(false),
	pWI(nullptr),
	hDlg(nullptr),
	hBar(nullptr),
	ownerWindow(nullptr),
	backgroundBrush(nullptr)
{
	auto scx = GetSystemMetrics(SM_CXSCREEN);
	auto scy = GetSystemMetrics(SM_CYSCREEN);

	this->wndSize.cx = 400;
	this->wndSize.cy = 80;

	this->position.x = (scx / 2) - (this->wndSize.cx / 2);
	this->position.y = (scy / 2) - (this->wndSize.cy / 2);

	this->cBackground = RGB(220, 220, 220);
	this->dlgFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Segoe UI\0");
}

ProgressDialog::~ProgressDialog()
{
	if (this->pWI != nullptr)
		this->pWI->killEx(0);

	if (this->backgroundBrush != nullptr)
		DeleteObject(this->backgroundBrush);
}

const TCHAR* ProgressDialog::PROGRESSCLASS = L"CNCSUITEPROGRESSCLASSID";

void ProgressDialog::setDialogSizeParameter(int x, int y, int cx, int cy)
{
	this->position.x = x;
	this->position.y = y;
	this->wndSize.cx = cx;
	this->wndSize.cy = cy;
}

ProgressDialog * ProgressDialog::getOpenInstance()
{
	auto pWnd = FindWindow(ProgressDialog::PROGRESSCLASS, nullptr);
	if (pWnd != nullptr)
	{
		return reinterpret_cast<ProgressDialog*>(GetWindowLongPtr(pWnd, GWLP_USERDATA));
	}
	else
		return nullptr;
}

HRESULT ProgressDialog::Create(HWND owner, HINSTANCE hInst)
{
	this->hInstance = hInst;

	auto hr = this->_registerProgressClass();
	if (SUCCEEDED(hr))
	{
		this->ownerWindow = owner;

		DWORD styles =
			(this->type & ProgressDialog::PROGRESSTYPE_CHILD)
			? WS_CHILD : WS_POPUP;

		if (this->type & ProgressDialog::PROGRESSTYPE_BORDER)
			styles |= WS_BORDER;

		this->hDlg =
			CreateWindow(
				ProgressDialog::PROGRESSCLASS,
				nullptr,
				styles,
				this->position.x,
				this->position.y,
				this->wndSize.cx,
				this->wndSize.cy,
				owner,
				nullptr,
				hInst,
				reinterpret_cast<LPVOID>(this)
			);

		hr = (this->hDlg != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			hr = this->_createContent();
			if (SUCCEEDED(hr))
			{
				// ...

				ShowWindow(this->hDlg, SW_SHOW);

				if (this->disableOwner)
				{
					EnableWindow(this->ownerWindow, FALSE);
				}
			}
		}
	}
	return hr;
}

void ProgressDialog::Close()
{
	if (this->disableOwner)
	{
		EnableWindow(this->ownerWindow, TRUE);
	}
	SendMessage(this->hDlg, WM_CLOSE, 0, 0);
}

void ProgressDialog::Update()
{
	this->_updateDialog();
}

void ProgressDialog::FinishAndClose()
{
	if (type & ProgressDialog::PROGRESSTYPE_PROGRESS)
	{
		this->type |= ProgressDialog::PROGRESSTYPE_CLOSEONFULLPROGRESS;
		this->setCurrentRange(this->absoluteRange);
	}
	else
	{
		if (this->pWI != nullptr)
		{
			this->pWI->setFinishNotificationTarget(this->hDlg);
			this->pWI->killEx(FINISH_SEQUENCE);
			this->pWI = nullptr;
		}
	}
}

bool ProgressDialog::stepRange()
{
	if (this->currentRange < this->absoluteRange)
	{
		auto tWnd = FindWindow(ProgressDialog::PROGRESSCLASS, nullptr);
		if (tWnd != nullptr)
		{
			PostMessage(tWnd, WM_STEPPROGRESS, 0, 0);
		}
		return true;
	}
	else
		return false;
}

bool ProgressDialog::setCurrentRange(DWORD _range)
{
	if ((_range > this->absoluteRange)
		|| (_range <= this->currentRange))
		return false;

	DWORD threadId;
	int para[2] = { (int)_range, (int)this->currentRange };

	HANDLE hThread = CreateThread(nullptr, 0, ProgressDialog::increaseProc, reinterpret_cast<LPVOID>(para), 0, &threadId);
	if (hThread)
	{
		WaitForSingleObject(hThread, 5);
		CloseHandle(hThread);

		return true;
	}
	return false;
}

LRESULT ProgressDialog::progressProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	ProgressDialog* pDlg = nullptr;

	if (message == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pDlg = reinterpret_cast<ProgressDialog*>(pcr->lpCreateParams);
		SetWindowLongPtr(hDialog, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDlg));
		return 1;
	}
	else
	{
		pDlg = reinterpret_cast<ProgressDialog*>(GetWindowLongPtr(hDialog, GWLP_USERDATA));
		if (pDlg != nullptr)
		{
			switch (message)
			{
			case WM_STEPPROGRESS:
				return pDlg->onStepProgress();
			case WM_PAINT:
				return pDlg->onPaint(hDialog);
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_CTLCOLORSTATIC:
				return static_cast<LRESULT>(pDlg->cBackground);
			case WM_CLOSE:
				DestroyWindow(hDialog);
				return 0;
			case WM_FINISHNOTIFY:
				if (wParam == ID_EXWORKING_TIMER)
				{
					pDlg->Close();
				}
				return 0;
			case WM_DESTROY:
				pDlg->Release();
				return 0;
			default:
				break;
			}
		}
		return DefWindowProc(hDialog, message, wParam, lParam);
	}
}

LRESULT ProgressDialog::barProc(HWND bar, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR refData)
{
	UNREFERENCED_PARAMETER(uID);

	auto _this = reinterpret_cast<ProgressDialog*>(refData);
	if (_this != nullptr)
	{
		switch (msg)
		{
		case WM_ERASEBKGND:
			return static_cast<LRESULT>(TRUE);
		case WM_PAINT:
			return _this->onBarPaint(bar);
		case WM_STEPANIMATION:
			return _this->onStepAnimation();
		default:
			break;
		}
	}
	return DefSubclassProc(bar, msg, wParam, lParam);
}

DWORD ProgressDialog::animateProc(LPVOID lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	while (1)
	{
		auto progressDialogWnd = FindWindow(ProgressDialog::PROGRESSCLASS, nullptr);
		if (progressDialogWnd != nullptr)
		{
			auto pBar = GetDlgItem(progressDialogWnd, ProgressDialog::PROGRESSBAR_ID);
			if (pBar != nullptr)
			{
				SendMessage(pBar, WM_STEPANIMATION, 0, 0);
			}
			Sleep(10);
		}
		else
			return 289;
	}
	return 0;
}

DWORD ProgressDialog::increaseProc(LPVOID lParam)
{
	int targetVal = ((int*)lParam)[0];
	int startVal = ((int*)lParam)[1];

	while (startVal < targetVal)
	{
		auto progressDialogWnd = FindWindow(ProgressDialog::PROGRESSCLASS, nullptr);
		if (progressDialogWnd != nullptr)
		{
			PostMessage(progressDialogWnd, WM_STEPPROGRESS, 0, 0);
			
			Sleep(5);
		}
		else
			return 234;

		startVal++;
	}
	return 37;
}

void ProgressDialog::_updateDialog()
{
	RedrawWindow(this->hDlg, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE | RDW_ALLCHILDREN);
}

void ProgressDialog::_updateProgressBar()
{
	RedrawWindow(this->hBar, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
}

HRESULT ProgressDialog::_registerProgressClass()
{
	HRESULT hr = S_OK;
	WNDCLASSEX wcx;

	if (!GetClassInfoEx(this->hInstance, ProgressDialog::PROGRESSCLASS, &wcx))
	{
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = sizeof(LONG_PTR);
		wcx.lpfnWndProc = ProgressDialog::progressProc;
		wcx.hInstance = this->hInstance;
		wcx.hIcon = nullptr;
		wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcx.hbrBackground = this->backgroundBrush;
		wcx.lpszClassName = ProgressDialog::PROGRESSCLASS;
		wcx.lpszMenuName = nullptr;
		wcx.hIconSm = nullptr;

		hr = (RegisterClassEx(&wcx) != 0) ? S_OK : E_FAIL;
	}
	return hr;
}

HRESULT ProgressDialog::_createContent()
{
	RECT rc;
	HRESULT hr = E_FAIL;

	GetClientRect(this->hDlg, &rc);

	if (this->type & ProgressDialog::PROGRESSTYPE_WORKER)
	{
		this->pWI = new workingIndicator(this->hInstance);
		if (this->pWI != nullptr)
		{
			POINT pt;
			pt.x = 0;
			pt.y = rc.bottom - 20;

			this->pWI->setExtendedAnimationProperty(
				ANIMATIONMODE_LINEAR_EXTENDED,
				this->cBackground,
				IDI_WKI_MOVER_SQ5_ORANGE
			);

			hr = this->pWI->showExtendedAnimation(this->hDlg, &pt, rc.right);
			if (SUCCEEDED(hr))
			{
				// ...
			}
		}
	}
	else if (this->type & ProgressDialog::PROGRESSTYPE_PROGRESS)
	{
		this->hBar = CreateWindow(
			L"STATIC",
			nullptr,
			WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPSIBLINGS,
			10, rc.bottom - 30, rc.right - 20, 10,
			this->hDlg,
			(HMENU)ProgressDialog::PROGRESSBAR_ID,
			this->hInstance,
			nullptr
		);
		hr = (this->hBar != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			hr =
				SetWindowSubclass(this->hBar, ProgressDialog::barProc, 0, reinterpret_cast<DWORD_PTR>(this))
				? S_OK : E_FAIL;

			if (this->type & ProgressDialog::PROGRESSTYPE_ANIMATE)
			{
				DWORD dwThreadId;

				HANDLE hThread = CreateThread(nullptr, 0, ProgressDialog::animateProc, nullptr, 0, &dwThreadId);
				if (hThread)
				{
					WaitForSingleObject(hThread, 5);
					CloseHandle(hThread);
				}
			}
		}
	}
	return hr;
}

LRESULT ProgressDialog::onPaint(HWND hDialog)
{
	RECT rc;
	GetClientRect(hDialog, &rc);

	PAINTSTRUCT ps;
	HDC hdc;

	hdc = BeginPaint(hDialog, &ps);
	if (hdc)
	{
		HBRUSH background = CreateSolidBrush(this->cBackground);
		if (background)
		{
			FillRect(hdc, &rc, background);

			auto len = this->displayMessage.GetLength();
			if (len > 0)
			{
				SIZE sz;

				if (getTextSizeDC(hdc, this->dlgFont, this->displayMessage.GetData(), &sz))
				{
					TextOutDC(
						hdc,
						this->displayMessage.GetData(),
						(rc.right / 2) - (sz.cx / 2),
						20,
						this->dlgFont,
						InvertColor(this->cBackground));
				}
			}
			DeleteObject(background);
		}
		EndPaint(hDialog, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT ProgressDialog::onBarPaint(HWND bar)
{
	HDC hdc;
	RECT rcWindow;
	PAINTSTRUCT ps;

	GetClientRect(bar, &rcWindow);

	hdc = BeginPaint(bar, &ps);
	if (hdc)
	{
		HBRUSH barBrush = CreateSolidBrush(RGB(20, 220, 20));
		if (barBrush)
		{
			RECT rcProgress;

			double rcright = rcWindow.right;
			double absrange = this->absoluteRange;
			double currange = this->currentRange;
			double rlrange = 0;
			
			if(absrange != 0)// prevent division by zero
				rlrange = rcright / absrange;

			auto x = currange * rlrange;

			if (this->currentRange == this->absoluteRange)
				x = rcright;

			SetRect(&rcProgress, 0, 0, (int)x, rcWindow.bottom);

			FillRect(hdc, &rcProgress, barBrush);

			if (this->type & ProgressDialog::PROGRESSTYPE_ANIMATE)
			{
				if (this->animationPosition < x)
				{
					HBRUSH forwardBrush = CreateSolidBrush(RGB(200, 220, 200));
					if (forwardBrush)
					{
						int end = this->animationPosition + 3;
						if (end > (int)x)
							end -= (end - (int)x);

						RECT fwdRect;
						SetRect(&fwdRect, this->animationPosition, 0, end, rcWindow.bottom);
						FillRect(hdc, &fwdRect, forwardBrush);

						HBRUSH bmask1 = CreateSolidBrush(RGB(180, 220, 180));
						if (bmask1)
						{
							if (this->animationPosition > 0)
							{
								RECT rcPost;
								SetRect(&rcPost, this->animationPosition - 2, 0, this->animationPosition - 1, rcWindow.bottom);
								FillRect(hdc, &rcPost, bmask1);
							}
							if ((((int)(this->animationPosition)) + 4) < ((int)x))
							{
								RECT rcPre;
								SetRect(&rcPre, this->animationPosition + 4, 0, this->animationPosition + 5, rcWindow.bottom);
								FillRect(hdc, &rcPre, bmask1);
							}
							DeleteObject(bmask1);
						}
						DeleteObject(forwardBrush);
					}
				}
			}
			DeleteObject(barBrush);
		}
		EndPaint(bar, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT ProgressDialog::onStepProgress()
{
	if (this->currentRange < this->absoluteRange)
	{
		this->currentRange++;
		this->_updateProgressBar();

		if (this->type & ProgressDialog::PROGRESSTYPE_CLOSEONFULLPROGRESS)
		{
			if (this->currentRange == this->absoluteRange)
			{
				this->Close();
			}
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT ProgressDialog::onStepAnimation()
{
	RECT rc;
	GetClientRect(this->hBar, &rc);

	if (this->animationPosition >= ((DWORD)rc.right))
	{
		this->animationPosition = 0;
	}
	else
	{
		this->animationPosition++;
	}
	RedrawWindow(this->hBar, nullptr, nullptr, RDW_NOERASE | RDW_INVALIDATE | RDW_NOCHILDREN);

	return static_cast<LRESULT>(0);
}