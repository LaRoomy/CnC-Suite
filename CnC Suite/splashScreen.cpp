#include"splashScreen.h"
#include"HelperF.h"

splashScreen::splashScreen(HINSTANCE hInst)
	:hInstance(hInst), canceledByUser(FALSE), mouseHook(nullptr), exitButtonState(0)
{
	this->gInfo.currentPosition[0] = -40;
	this->gInfo.currentPosition[1] = -80;
	this->gInfo.currentPosition[2] = -120;

	this->exitIcon = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_CLOSEBUTTON), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->exitIconMarked = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_CLOSEBUTTON_MARKED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->exitIconPressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_CLOSEBUTTON_PRESSED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);

	this->runnerBrush = CreateSolidBrush(RGB(190, 190, 0));
	this->font = CreateScaledFont(18, FW_BOLD, APPLICATION_PRIMARY_FONT);
		
	this->gInfo.outputString.Replace(
		getStringFromResource(UI_SSCREEN_LOADING)
	);
}

splashScreen::~splashScreen()
{
	UnhookWindowsHookEx(this->mouseHook);

	DeleteObject(this->runnerBrush);
	DeleteObject(this->font);

	DestroyIcon(this->exitIcon);
	DestroyIcon(this->exitIconMarked);
	DestroyIcon(this->exitIconPressed);
}

HRESULT splashScreen::Start()
{
	this->startAsync();
	return S_OK;
}

HRESULT splashScreen::Stop()
{
	BOOL cbUser = FALSE;

	auto spScreen = FindWindow(SPLASHSCREENCLASS, nullptr);
	if (spScreen != nullptr)
	{
		cbUser = this->canceledByUser;
		PostMessage(spScreen, WM_DESTROY, 0, 0);
	}
	return cbUser ? E_ABORT : S_OK;
}

HRESULT splashScreen::create()
{
	auto hr = this->registerSCClass();
	if (SUCCEEDED(hr))
	{
		auto cx = GetSystemMetrics(SM_CXSCREEN);
		auto cy = GetSystemMetrics(SM_CYSCREEN);

		this->sScreen = CreateWindowEx(
			WS_EX_TOOLWINDOW,
			SPLASHSCREENCLASS,
			nullptr,
			WS_POPUP | WS_BORDER,
			(cx / 2) - ((BITMAP_SIZE_CX + 2) / 2),
			(cy / 2) - ((BITMAP_SIZE_CY + 2) / 2),
			BITMAP_SIZE_CX + 2,
			BITMAP_SIZE_CY + 2,
			nullptr, nullptr,
			this->hInstance,
			reinterpret_cast<LPVOID>(this)
		);

		hr = (this->sScreen != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			hr = this->createControls();
			if (SUCCEEDED(hr))
			{
				SetTimer(this->sScreen, TIMERID_15MSEC, 15, nullptr);

				this->mouseHook = 
					SetWindowsHookEx(
						WH_MOUSE,
						splashScreen::mouseProc,
						this->hInstance,
						GetCurrentThreadId()
					);

				ShowWindow(this->sScreen, SW_SHOW);
				UpdateWindow(this->sScreen);
				SetForegroundWindow(this->sScreen);
			}
		}
	}
	return hr;
}

HRESULT splashScreen::createControls()
{
	//RECT rc;
	//GetClientRect(this->sScreen, &rc);

	//POINT pos;
	//SIZE size;

	//pos.x = rc.right - 30;
	//pos.y = 2;

	//size.cx = 28;
	//size.cy = 28;

	//this->exitButton = new CustomButton(this->sScreen, BUTTONMODE_ICON, &pos, &size, CTRLID_EXIT, this->hInstance);

	//auto hr = (exitButton != nullptr) ? S_OK : E_FAIL;
	//if (SUCCEEDED(hr))
	//{
	//	this->exitButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
	//	this->exitButton->setAppearance_onlyIcon(IDI_GNRL_CANCEL_YELLOW, 24);
	//	this->exitButton->setColors(RGB(0, 0, 0), COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);

	//	hr = this->exitButton->Create();
	//	if (SUCCEEDED(hr))
	//	{
	//		// ...
	//	}
	//}
	//return hr;
	return S_OK;
}

LRESULT splashScreen::splashProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	splashScreen* ssc = nullptr;

	if (message == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		ssc = reinterpret_cast<splashScreen*>(pcr->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ssc));
		return 1;
	}
	else
	{
		ssc = reinterpret_cast<splashScreen*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (ssc != nullptr)
		{
			switch (message)
			{
			case WM_COMMAND:
				return ssc->onCommand(hWnd, wParam, lParam);
			case WM_TIMER:
				return ssc->onTimer(hWnd, wParam);
			case WM_PAINT:
				return ssc->onPaint(hWnd);
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_LBUTTONDOWN:
				return ssc->onLButtonDown(lParam);
			case WM_DESTROY:
				// cancel graphics
				ssc->cleanUp();
				// release this
				SafeRelease(&ssc);
				// quit message loop

				SetForegroundWindow(
					FindWindow(IDSEX_APPLICATIONCLASS, nullptr)
				);

				PostQuitMessage(0);
				return 0;
			default:
				break;
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

DWORD splashScreen::asyncProc(LPVOID lParam)
{
	auto _this = reinterpret_cast<splashScreen*>(lParam);
	if (_this != nullptr)
	{
		auto hr = _this->create();
		if (SUCCEEDED(hr))
		{
			return _this->run();
		}
		return 1;
	}
	return 0;
}

LRESULT splashScreen::mouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);
	else
	{
		auto mhs = reinterpret_cast<LPMOUSEHOOKSTRUCT>(lParam);
		if (mhs != nullptr)
		{
			auto _this_ =
				reinterpret_cast<splashScreen*>(
					USERDATA_FROMWINDOW(SPLASHSCREENCLASS)
					);
			if (_this_ != nullptr)
			{
				RECT rc;

				if (GetWindowRect(_this_->sScreen, &rc))
				{
					RECT buttonRect;
					SetRect(
						&buttonRect,
						rc.right - DPIScale(26),
						rc.top + DPIScale(2),
						rc.right - DPIScale(2),
						rc.top + DPIScale(26)
					);

					if (isPointInRect(mhs->pt.x, mhs->pt.y, &buttonRect))
					{
						short keystate = GetAsyncKeyState(VK_LBUTTON);

						if (keystate & 0x8000)
						{
							InterlockedExchange((LONG*)&_this_->exitButtonState, (LONG)2);
						}
						else
							InterlockedExchange((LONG*)&_this_->exitButtonState, (LONG)1);
					}
					else
						InterlockedExchange((LONG*)&_this_->exitButtonState, (LONG)0);

				}
			}
		}
		return CallNextHookEx(nullptr, nCode, wParam, lParam);
	}
}

bool splashScreen::startAsync()
{
	HANDLE hThread;
	DWORD threadID;

	hThread = CreateThread(nullptr, 0, splashScreen::asyncProc, reinterpret_cast<LPVOID>(this), 0, &threadID);
	if (hThread != nullptr)
	{
		CloseHandle(hThread);
		return true;
	}
	else
		return false;
}

DWORD splashScreen::run()
{
	MSG msg;
	BOOL bReturn;

	while ((bReturn = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bReturn == -1)
		{
			MessageBox(NULL, L"Messageloop failed.\nApp will be terminated - pos:splashscreen:loop", L"Critical error", MB_OK | MB_ICONERROR);
			ExitProcess((UINT)bReturn);
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return static_cast<DWORD>(msg.wParam);
}

LRESULT splashScreen::onPaint(HWND hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	PAINTSTRUCT ps;
	SIZE sz;
	HDC hdc;

	hdc = BeginPaint(hWnd, &ps);
	if (hdc)
	{
		HDC hdcMem = CreateCompatibleDC(hdc);
		if (hdcMem)
		{
			HBITMAP bmp =
				(HBITMAP)LoadImage(
					this->hInstance,
					MAKEINTRESOURCE(IDB_SPLASHSCREEN),
					IMAGE_BITMAP,
					BITMAP_SIZE_CX, BITMAP_SIZE_CY,
					LR_DEFAULTCOLOR
				);
			if (bmp)
			{
				// select the background bitmap in the offscreen DC
				auto hbmOld = SelectObject(hdcMem, bmp);

				// draw the runner
				RECT runnerRect;
				SetRect(&runnerRect, this->gInfo.currentPosition[0], rc.bottom - 2, this->gInfo.currentPosition[2], rc.bottom);
				FillRect(hdcMem, &runnerRect, this->runnerBrush);

				// set the new runner position
				// 1
				if (this->gInfo.currentPosition[0] <= 100 || this->gInfo.currentPosition[0] >= 300)
					this->gInfo.currentPosition[0] += 3;
				else if (this->gInfo.currentPosition[0] <= 130 || this->gInfo.currentPosition[0] >= 270)
					this->gInfo.currentPosition[0] += 2;
				else
					this->gInfo.currentPosition[0]++;
				// 2
				if (this->gInfo.currentPosition[1] <= 100 || this->gInfo.currentPosition[1] >= 300)
					this->gInfo.currentPosition[1] += 3;
				else if (this->gInfo.currentPosition[1] <= 130 || this->gInfo.currentPosition[1] >= 270)
					this->gInfo.currentPosition[1] += 2;
				else
					this->gInfo.currentPosition[1]++;
				// 3
				if (this->gInfo.currentPosition[2] <= 100 || this->gInfo.currentPosition[2] >= 300)
					this->gInfo.currentPosition[2] += 3;
				else if (this->gInfo.currentPosition[2] <= 130 || this->gInfo.currentPosition[2] >= 270)
					this->gInfo.currentPosition[2] += 2;
				else
					this->gInfo.currentPosition[2]++;

				// restart run
				if (this->gInfo.currentPosition[2] >= (rc.right + 3))
				{
					this->gInfo.currentPosition[0] = -3;
					this->gInfo.currentPosition[1] = -30;
					this->gInfo.currentPosition[2] = -60;
				}

				// draw text
				if (this->gInfo.outputString.GetLength() > 0)
				{
					auto hfntOld = SelectObject(hdcMem, this->font);

					SetBkMode(hdcMem, TRANSPARENT);
					SetTextColor(hdcMem, RGB(255, 255, 255));

					GetTextExtentPoint32(
						hdcMem,
						this->gInfo.outputString.GetData(),
						this->gInfo.outputString.GetLength(),
						&sz
					);
					TextOut(
						hdcMem,
						10, 10,
						this->gInfo.outputString.GetData(),
						this->gInfo.outputString.GetLength()
					);

					SelectObject(hdcMem, hfntOld);
				}

				// draw exit icon
				DrawIconEx(
					hdcMem,
					rc.right - DPIScale(26),
					DPIScale(2),
					this->getCurrentExitIcon(),
					DPIScale(24),
					DPIScale(24),
					0, nullptr,
					DI_NORMAL
				);

				// load Bitmap to screen DC
				BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

				SelectObject(hdcMem, hbmOld);
				DeleteObject(bmp);
			}
			DeleteDC(hdcMem);
		}
		EndPaint(hWnd, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT splashScreen::onTimer(HWND hWnd, WPARAM wParam)
{	
	UNREFERENCED_PARAMETER(hWnd);

	switch (wParam)
	{
	case TIMERID_15MSEC:
		RedrawWindow(this->sScreen, nullptr, nullptr, RDW_NOERASE | RDW_INVALIDATE);
		break;
	default:
		break;
	}
	return static_cast<LRESULT>(0);
}

LRESULT splashScreen::onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hWnd);

	switch (LOWORD(wParam))
	{
	case ICOMMAND_SETOUTPUT_STRING:
		this->setOutputString(lParam);
		break;
	case ICOMMAND_GETEXITCONDITION:
		return static_cast<LRESULT>(this->canceledByUser);
	default:
		break;
	}
	if (this->canceledByUser)
	{
		DestroyWindow(this->sScreen);
		return static_cast<LRESULT>(FALSE);
	}
	return static_cast<LRESULT>(TRUE);
}

LRESULT splashScreen::onLButtonDown(LPARAM lParam)
{
	auto xPos = GET_X_LPARAM(lParam);
	auto yPos = GET_Y_LPARAM(lParam);

	RECT rc;

	if (GetClientRect(this->sScreen, &rc))
	{
		RECT buttonRect;

		SetRect(
			&buttonRect,
			rc.right - DPIScale(26),
			DPIScale(2),
			rc.right - DPIScale(2),
			DPIScale(26)
		);

		if (isPointInRect(xPos, yPos, &buttonRect))
		{
			this->onExitButtonClick();
		}
	}

	return static_cast<LRESULT>(0);
}

void splashScreen::onExitButtonClick()
{
	this->canceledByUser = TRUE;
}

void splashScreen::cleanUp()
{
	KillTimer(this->sScreen, TIMERID_15MSEC);
}

void splashScreen::setOutputString(LPARAM lParam)
{
	this->gInfo
		.outputString
		.Replace(
			reinterpret_cast<TCHAR*>(lParam)
		);
}

HICON splashScreen::getCurrentExitIcon()
{
	switch (this->exitButtonState)
	{
	case 0:
		return this->exitIcon;
	case 1:
		return this->exitIconMarked;
	case 2:
		return this->exitIconPressed;
	default:
		return this->exitIcon;
	}
}


HRESULT splashScreen::registerSCClass()
{
	WNDCLASSEX wcx;
	HRESULT hr = S_OK;

	if (!GetClassInfoEx(this->hInstance, SPLASHSCREENCLASS, &wcx))
	{
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcx.lpfnWndProc = splashScreen::splashProc;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = sizeof(LONG_PTR);
		wcx.hbrBackground = nullptr;
		wcx.hIcon = nullptr;
		wcx.hInstance = this->hInstance;
		wcx.hIconSm = nullptr;
		wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcx.lpszClassName = SPLASHSCREENCLASS;
		wcx.lpszMenuName = nullptr;

		hr = (RegisterClassEx(&wcx) != 0) ? S_OK : E_FAIL;
	}
	return hr;
}
