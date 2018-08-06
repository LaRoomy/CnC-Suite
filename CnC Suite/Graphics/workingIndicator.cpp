#include"workingIndicator.h"
#include<math.h>
#include"..//HelperF.h"
#include"..//CommonControls/StringClass.h"

workingIndicator::workingIndicator(HINSTANCE hInst, int indicationIcon)
	:hInstance(hInst),
	parent(nullptr),
	workingWnd(nullptr),
	stopThread(0),
	bkgndBrush(nullptr),
	animationStarted(false),
	animationMode(ANIMATIONMODE_LINEAR),
	spacing(20),
	numMover(5),
	moverBrush(nullptr),
	delay(100),
	notifyTarget(nullptr)
{
	if (indicationIcon != -1)
		this->iIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(indicationIcon), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	else
		this->iIcon = nullptr;

	this->background = RGB(240, 240, 240);
	this->moverColor = RGB(200, 100, 0);

	this->gInfo.currentPosition[0] = -10;
	this->gInfo.currentPosition[1] = -50;
	this->gInfo.currentPosition[2] = -90;
	this->gInfo.currentPosition[3] = -130;
	this->gInfo.currentPosition[4] = -170;

	for (int u = 0; u < 5; u++)
		this->gInfo.calculatedValue[u] = 3;
}

workingIndicator::workingIndicator(HINSTANCE hInst)
	:hInstance(hInst),
	parent(nullptr),
	workingWnd(nullptr),
	stopThread(0),
	bkgndBrush(nullptr),
	animationStarted(false),
	animationMode(ANIMATIONMODE_LINEAR),
	spacing(20),
	numMover(5),
	moverBrush(nullptr),
	delay(100)
{
	// extended methods!
	this->iIcon = nullptr;

	this->background = RGB(240, 240, 240);
	this->moverColor = RGB(200, 100, 0);

	this->gInfo.currentPosition[0] = -10;
	this->gInfo.currentPosition[1] = -50;
	this->gInfo.currentPosition[2] = -90;
	this->gInfo.currentPosition[3] = -130;
	this->gInfo.currentPosition[4] = -170;

	for (int u = 0; u < 5; u++)
		this->gInfo.calculatedValue[u] = 3;
}

workingIndicator::~workingIndicator()
{
	UnregisterClass(WKI_CLASS, nullptr);// this approach does not work!!

	if(this->bkgndBrush != nullptr)
		DeleteObject(this->bkgndBrush);
	if (this->iIcon != nullptr)
		DestroyIcon(this->iIcon);
	if (this->moverBrush != nullptr)
		DeleteObject(this->moverBrush);
}

HRESULT workingIndicator::showAsPopUp(HWND parent_, LPRECT rc, COLORREF bkgnd)
{
	HRESULT hr = E_FAIL;

	if (parent_ != nullptr)
	{
		this->parent = parent_;
		this->background = bkgnd;
		this->bkgndBrush = CreateSolidBrush(bkgnd);
		this->windowMode = WINDOWMODE_POPUP;

		hr = S_OK;

		WNDCLASSEX wcx;
		if (!GetClassInfoEx(this->hInstance, WKI_CLASS, &wcx))
		{
			wcx.cbSize = sizeof(WNDCLASSEX);
			wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
			wcx.lpfnWndProc = workingIndicator::WNDProc;
			wcx.cbClsExtra = 0;
			wcx.cbWndExtra = sizeof(LONG_PTR);
			wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wcx.hInstance = this->hInstance;
			wcx.hIcon = nullptr;
			wcx.hbrBackground = nullptr;
			wcx.lpszClassName = WKI_CLASS;
			wcx.hIconSm = nullptr;
			wcx.lpszMenuName = nullptr;

			hr = (RegisterClassEx(&wcx) == 0) ? E_FAIL : S_OK;
		}
		if (SUCCEEDED(hr))
		{
			this->workingWnd = CreateWindow(
				WKI_CLASS, nullptr,
				WS_POPUP,
				rc->left, rc->top, rc->right, rc->bottom,
				parent_, NULL, this->hInstance, reinterpret_cast<LPVOID>(this));

			hr = (this->workingWnd != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				ShowWindow(this->workingWnd, SW_SHOW);
				UpdateWindow(this->workingWnd);

				this->startIndicationThread();
			}
		}		
	}
	return hr;
}

HRESULT workingIndicator::showAsChild(HWND parent_, LPRECT rc, COLORREF bkgnd)
{
	HRESULT hr = E_FAIL;

	if (parent_ != nullptr)
	{
		this->parent = parent_;
		this->background = bkgnd;
		this->bkgndBrush = CreateSolidBrush(bkgnd);
		this->windowMode = WINDOWMODE_CHILD;

		hr = S_OK;

		WNDCLASSEX wcx;
		if (!GetClassInfoEx(this->hInstance, WKI_CLASS, &wcx))
		{
			wcx.cbSize = sizeof(WNDCLASSEX);
			wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
			wcx.lpfnWndProc = workingIndicator::WNDProc;
			wcx.cbClsExtra = 0;
			wcx.cbWndExtra = sizeof(LONG_PTR);
			wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wcx.hInstance = this->hInstance;
			wcx.hIcon = nullptr;
			wcx.hbrBackground = nullptr;
			wcx.lpszClassName = WKI_CLASS;
			wcx.hIconSm = nullptr;
			wcx.lpszMenuName = nullptr;

			hr = (RegisterClassEx(&wcx) == 0) ? E_FAIL : S_OK;
		}
		if (SUCCEEDED(hr))
		{
			this->workingWnd = CreateWindow(
				WKI_CLASS, nullptr,
				WS_CHILD | WS_VISIBLE,
				rc->left, rc->top, rc->right, rc->bottom,
				parent_, (HMENU)ID_WORKINGINDICATOR, this->hInstance, reinterpret_cast<LPVOID>(this));

			hr = (this->workingWnd != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				hr =
					SetWindowSubclass(this->workingWnd, workingIndicator::WndSubProc, 0, reinterpret_cast<DWORD_PTR>(this))
					? S_OK : E_FAIL;

				if (SUCCEEDED(hr))
				{
					this->startIndicationThread();
				}
			}
		}
	}
	return hr;
}

void workingIndicator::kill()
{
	__try
	{
		this->stopThread = STOP_THREAD_IMMEDIATELY;
	}
	__except (GetExceptionCode()
		== EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return;
	}
}

void workingIndicator::setAnimationProperty(DWORD Mode, int numOfMovingItems, DWORD _animationTime, DWORD _spacing)
{
	this->animationMode = Mode;
	this->numMover = numOfMovingItems;
	this->animationTime = _animationTime;
	this->spacing = _spacing;
}

HRESULT workingIndicator::showExtendedAnimation(HWND _parent, LPPOINT pos, int width)
{
	HRESULT hr = E_FAIL;

	if (_parent != nullptr)
	{
		this->parent = _parent;		
		this->windowMode = WINDOWMODE_CHILD;

		hr = S_OK;

		WNDCLASSEX wcx;
		if (!GetClassInfoEx(this->hInstance, WKI_CLASS, &wcx))
		{
			wcx.cbSize = sizeof(WNDCLASSEX);
			wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
			wcx.lpfnWndProc = workingIndicator::WNDProc;
			wcx.cbClsExtra = 0;
			wcx.cbWndExtra = sizeof(LONG_PTR);
			wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wcx.hInstance = this->hInstance;
			wcx.hIcon = nullptr;
			wcx.hbrBackground = nullptr;
			wcx.lpszClassName = WKI_CLASS;
			wcx.hIconSm = nullptr;
			wcx.lpszMenuName = nullptr;

			hr = (RegisterClassEx(&wcx) == 0) ? E_FAIL : S_OK;
		}
		if (SUCCEEDED(hr))
		{
			this->workingWnd = CreateWindow(
				WKI_CLASS,
				nullptr,
				WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
				pos->x,
				pos->y,
				width,
				5,
				_parent,
				(HMENU)ID_WORKINGINDICATOR,
				this->hInstance,
				reinterpret_cast<LPVOID>(this)
			);

			hr = (this->workingWnd != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				SetTimer(this->workingWnd, ID_EXWORKING_TIMER, 15, nullptr);
			}
		}
	}
	return hr;
}

void workingIndicator::setExtendedAnimationProperty(DWORD Mode, COLORREF _background, int iconID)
{
	this->animationMode = Mode;
	this->numMover = 5;
	this->animationTime = 15;
	this->spacing = 2;
	this->moverColor = RGB(240, 170, 30);
	this->background = _background;

	this->iIcon = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(iconID), IMAGE_ICON, 5, 5, LR_DEFAULTCOLOR);

	if (this->bkgndBrush != nullptr)
		DeleteObject(this->bkgndBrush);

	this->bkgndBrush = CreateSolidBrush(this->background);

	if (this->moverBrush != nullptr)
		DeleteObject(this->moverBrush);

	this->moverBrush = CreateSolidBrush(this->moverColor);
}

void workingIndicator::killEx(DWORD delay_ms)
{
	if (delay_ms == 0)
	{
		HWND nTarget = nullptr;

		if (this->notifyTarget != nullptr)
		{
			nTarget = this->notifyTarget;
		}

		KillTimer(this->workingWnd, ID_EXWORKING_TIMER);
		SendMessage(this->workingWnd, WM_CLEANUP, 0, 0);

		if (nTarget != nullptr)
		{
			PostMessage(nTarget, WM_FINISHNOTIFY, static_cast<WPARAM>(ID_EXWORKING_TIMER), 0);
		}
	}
	else
	{
		this->delay = delay_ms;
		if (delay_ms != FINISH_SEQUENCE)
		{
			this->startDelayThread();
		}
	}
}

void workingIndicator::setFinishNotificationTarget(HWND msgReceiver)
{
	this->notifyTarget = msgReceiver;
}


LRESULT workingIndicator::WNDProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	workingIndicator* wki = nullptr;

	if (msg == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		wki = reinterpret_cast<workingIndicator*>(pcr->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wki));
		return 1;
	}
	else
	{
		wki = reinterpret_cast<workingIndicator*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (wki != nullptr)
		{
			switch (msg)
			{
			case WM_TIMER:
				return wki->onExTimer(hWnd, wParam);
			case WM_PAINT:
				return wki->onPaint(hWnd);
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_ANIMATE:
				return wki->onAnimate(hWnd, lParam);
			case WM_DESTROY:
				SafeRelease(&wki);
				return 0;
			case WM_CLEANUP:
				DestroyWindow(hWnd);				
				return 0;
			default:
				return DefWindowProc(hWnd, msg, wParam, lParam);
			}
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

LRESULT workingIndicator::WndSubProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR refData)
{
	UNREFERENCED_PARAMETER(uID);

	auto wki = reinterpret_cast<workingIndicator*>(refData);

	if (wki != nullptr)
	{
		switch (msg)
		{
		case WM_PAINT:
			return wki->onPaint(hWnd);
		case WM_ERASEBKGND:
			return static_cast<LRESULT>(TRUE);
		case WM_ANIMATE:
			return wki->onAnimate(hWnd, lParam);
		case WM_DESTROY:
			SafeRelease(&wki);
			return 0;
		case WM_CLEANUP:
			DestroyWindow(hWnd);
			return 0;
		default:
			return DefSubclassProc(hWnd, msg, wParam, lParam);
		}
	}
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}

DWORD workingIndicator::indicationProc(LPVOID lParam)
{
	auto wki = reinterpret_cast<workingIndicator*>(lParam);

	// save animation-mode so that the setter cannot corrupt it at runtime
	int animation_Mode_ = wki->animationMode;
	int wndSearchTryout = 0;
	DWORD _windowmode = wki->windowMode;
	DWORD _animationTime = wki->animationTime;
	DWORD _spacing = wki->spacing;
	HWND _parent = wki->parent;

	if (wki->workingWnd == nullptr)
		return 11;

	RECT rc;
	GetClientRect(wki->workingWnd, &rc);

	ANIMATEINFO aInfo;
	aInfo.aPt.x = 0;
	aInfo.aPt.y = 0;
	aInfo.numMover = wki->numMover;
	aInfo.precedingMover = nullptr;

	if (animation_Mode_ == ANIMATIONMODE_CIRCLE)
		aInfo.animateInfo = ANIMATE_CIRCLE_Q1;
	else
		aInfo.animateInfo = ANIMATE_DIRECTION_UP;

	if (aInfo.numMover > 1)
	{
		aInfo.precedingMover = new ASUBINFO[(aInfo.numMover - 1)];
		if (aInfo.precedingMover != nullptr)
		{
			for (int i = 0; i < (aInfo.numMover - 1); i++)
			{
				aInfo.precedingMover[i].started = FALSE;
				aInfo.precedingMover[i].aPt.x = 0;
				aInfo.precedingMover[i].aPt.y = 0;

				if (animation_Mode_ == ANIMATIONMODE_CIRCLE)
					aInfo.precedingMover[i].animateInfo = ANIMATE_CIRCLE_Q1;
				else
					aInfo.precedingMover[i].animateInfo = ANIMATE_DIRECTION_UP;
			}
		}
	}

	while (1)
	{
		HWND hWnd;
		// only hold thread if the window is existing
		// -> this prevents access violation
		if (_windowmode == WINDOWMODE_POPUP)
			hWnd = FindWindow(WKI_CLASS, nullptr);
		else
			hWnd = GetDlgItem(_parent, ID_WORKINGINDICATOR);

		if (hWnd != nullptr)
		{
			auto _this = reinterpret_cast<workingIndicator*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
			if (_this != nullptr)
			{
				__try
				{
					if (_this->stopThread == STOP_THREAD_IMMEDIATELY)
					{
						SendMessage(hWnd, WM_CLEANUP, 0, 0);
						delete aInfo.precedingMover;
						return 3;
					}
					else
					{
						if (animation_Mode_ == ANIMATIONMODE_LINEAR)
						{
							aInfo.aPt.y = (rc.bottom / 2) - 8;

							if (aInfo.animateInfo == ANIMATE_DIRECTION_UP)
							{
								aInfo.aPt.x += 1;

								if (aInfo.aPt.x == rc.right - 16)
									aInfo.animateInfo = ANIMATE_DIRECTION_DOWN;
							}
							else if (aInfo.animateInfo == ANIMATE_DIRECTION_DOWN)
							{
								aInfo.aPt.x -= 1;

								if (aInfo.aPt.x == 0)
									aInfo.animateInfo = ANIMATE_DIRECTION_UP;
							}
							// set the preceding bullets
							if (aInfo.numMover > 1)
							{
								if (aInfo.precedingMover != nullptr)
								{
									int y = (rc.bottom / 2) - 8;

									for (int i = 0; i < (aInfo.numMover - 1); i++)
									{
										if (aInfo.precedingMover[i].started == 2)
											aInfo.precedingMover[i].started = TRUE;

										if (aInfo.precedingMover[i].started == TRUE)
										{
											aInfo.precedingMover[i].aPt.y = y;

											if (aInfo.precedingMover[i].animateInfo == ANIMATE_DIRECTION_UP)
											{
												aInfo.precedingMover[i].aPt.x += 1;

												if (aInfo.precedingMover[i].aPt.x == rc.right - 16)
													aInfo.precedingMover[i].animateInfo = ANIMATE_DIRECTION_DOWN;
											}
											else if (aInfo.precedingMover[i].animateInfo == ANIMATE_DIRECTION_DOWN)
											{
												aInfo.precedingMover[i].aPt.x -= 1;

												if (aInfo.precedingMover[i].aPt.x == 0)
													aInfo.precedingMover[i].animateInfo = ANIMATE_DIRECTION_UP;
											}
										}
										else
										{
											if (aInfo.aPt.x == ((i + 1) * 32))
											{
												aInfo.precedingMover[i].started = 2;
											}
										}
									}
								}
							}

						}
						else if (animation_Mode_ == ANIMATIONMODE_CIRCLE)
						{
							int sqare, radius;

							if (rc.right >= rc.bottom)sqare = (rc.right - 16);
							else sqare = (rc.bottom - 16);
							radius = sqare / 2;

							if (aInfo.animateInfo == ANIMATE_CIRCLE_Q1)
							{
								aInfo.aPt.x++;
								aInfo.aPt.y = ((rc.bottom - 16) / 2) - wki->calculateHeigtFromLength(radius - aInfo.aPt.x, radius);
								if (aInfo.aPt.x == radius)
									aInfo.animateInfo = ANIMATE_CIRCLE_Q2;
							}
							else if (aInfo.animateInfo == ANIMATE_CIRCLE_Q2)
							{
								aInfo.aPt.x++;
								aInfo.aPt.y = ((rc.bottom - 16) / 2) - wki->calculateHeigtFromLength(aInfo.aPt.x - radius, radius);
								if (aInfo.aPt.x == sqare)
									aInfo.animateInfo = ANIMATE_CIRCLE_Q3;
							}
							else if (aInfo.animateInfo == ANIMATE_CIRCLE_Q3)
							{
								aInfo.aPt.x--;
								aInfo.aPt.y = ((rc.bottom - 16) / 2) + wki->calculateHeigtFromLength(aInfo.aPt.x - radius, radius);
								if (aInfo.aPt.x == radius)
									aInfo.animateInfo = ANIMATE_CIRCLE_Q4;
							}
							else if (aInfo.animateInfo == ANIMATE_CIRCLE_Q4)
							{
								aInfo.aPt.x--;
								aInfo.aPt.y = ((rc.bottom - 16) / 2) + wki->calculateHeigtFromLength(radius - aInfo.aPt.x, radius);
								if (aInfo.aPt.x == 0)
									aInfo.animateInfo = ANIMATE_CIRCLE_Q1;
							}

							if (aInfo.numMover > 1)
							{
								// calculate the preceding
								if (aInfo.precedingMover != nullptr)
								{
									for (int i = 0; i < (aInfo.numMover - 1); i++)
									{
										if (aInfo.precedingMover[i].started == 2)
											aInfo.precedingMover[i].started = TRUE;

										if (aInfo.precedingMover[i].started == TRUE)
										{
											if (aInfo.precedingMover[i].animateInfo == ANIMATE_CIRCLE_Q1)
											{
												aInfo.precedingMover[i].aPt.x++;
												aInfo.precedingMover[i].aPt.y = ((rc.bottom - 16) / 2) - wki->calculateHeigtFromLength(radius - aInfo.precedingMover[i].aPt.x, radius);
												if (aInfo.precedingMover[i].aPt.x == radius)
													aInfo.precedingMover[i].animateInfo = ANIMATE_CIRCLE_Q2;
											}
											else if (aInfo.precedingMover[i].animateInfo == ANIMATE_CIRCLE_Q2)
											{
												aInfo.precedingMover[i].aPt.x++;
												aInfo.precedingMover[i].aPt.y = ((rc.bottom - 16) / 2) - wki->calculateHeigtFromLength(aInfo.precedingMover[i].aPt.x - radius, radius);
												if (aInfo.precedingMover[i].aPt.x == sqare)
													aInfo.precedingMover[i].animateInfo = ANIMATE_CIRCLE_Q3;
											}
											else if (aInfo.precedingMover[i].animateInfo == ANIMATE_CIRCLE_Q3)
											{
												aInfo.precedingMover[i].aPt.x--;
												aInfo.precedingMover[i].aPt.y = ((rc.bottom - 16) / 2) + wki->calculateHeigtFromLength(aInfo.precedingMover[i].aPt.x - radius, radius);
												if (aInfo.precedingMover[i].aPt.x == radius)
													aInfo.precedingMover[i].animateInfo = ANIMATE_CIRCLE_Q4;
											}
											else if (aInfo.precedingMover[i].animateInfo == ANIMATE_CIRCLE_Q4)
											{
												aInfo.precedingMover[i].aPt.x--;
												aInfo.precedingMover[i].aPt.y = ((rc.bottom - 16) / 2) + wki->calculateHeigtFromLength(radius - aInfo.precedingMover[i].aPt.x, radius);
												if (aInfo.precedingMover[i].aPt.x == 0)
													aInfo.precedingMover[i].animateInfo = ANIMATE_CIRCLE_Q1;
											}
										}
										else
										{
											if (aInfo.aPt.x == ((LONG)((i + 1) * _spacing)))
											{
												aInfo.precedingMover[i].started = 2;
											}
										}
									}
								}
							}
						}
						else
						{
							return 6;
						}
						SendMessage(hWnd, WM_ANIMATE, 0, reinterpret_cast<LPARAM>(&aInfo));
					}
				}
				__except (GetExceptionCode()
					== EXCEPTION_ACCESS_VIOLATION
					? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
				{
					SendMessage(hWnd, WM_CLEANUP, 0, 0);
					return 4;
				}
			}			
		}
		else
		{
			if (wndSearchTryout == 10)
			{
				// window may was closed without calling the kill funktion -> close thread
				return 5;
			}
			wndSearchTryout++;
		}	
		Sleep(_animationTime);
	}
	return 0;
}

DWORD workingIndicator::killdelayProc(LPVOID lParam)
{
	auto _this = reinterpret_cast<workingIndicator*>(lParam);
	if (_this != nullptr)
	{
		if (_this->delay > 0 && _this->delay != FINISH_SEQUENCE)
		{
			Sleep(_this->delay);
			_this->killEx(0);
		}
		return 1;
	}
	return 0;
}


void workingIndicator::startIndicationThread()
{
	HANDLE hTread;
	DWORD threadId;

	this->animationStarted = true;

	hTread = CreateThread(nullptr, 0, workingIndicator::indicationProc, reinterpret_cast<LPVOID>(this), 0, &threadId);
	if (hTread != nullptr)
	{
		CloseHandle(hTread);
	}
}

void workingIndicator::startDelayThread()
{
	HANDLE hThread;
	DWORD dwThreadId;

	hThread = CreateThread(nullptr, 0, workingIndicator::killdelayProc, reinterpret_cast<LPVOID>(this), 0, &dwThreadId);
	if (hThread != nullptr)
	{
		//WaitForSingleObject(hThread, 10);
		CloseHandle(hThread);
	}
}

LRESULT workingIndicator::onPaint(HWND hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	PAINTSTRUCT ps;
	HDC hdc;

	hdc = BeginPaint(hWnd, &ps);
	if (hdc)
	{
		if (this->animationMode == ANIMATIONMODE_LINEAR_EXTENDED)
		{
			this->animateEx(hWnd, hdc);
			//this->animateExOpt(hWnd, hdc);
		}
		else
		{
			if (!this->animationStarted)
			{
				FillRect(hdc, &rc, this->bkgndBrush);

				DrawIconEx(hdc, rc.left, (rc.bottom / 2) - 8, this->iIcon, 16, 16, 0, nullptr, DI_NORMAL);
			}
		}
	}
	EndPaint(hWnd, &ps);

	return static_cast<LRESULT>(0);
}

LRESULT workingIndicator::onAnimate(HWND hWnd, LPARAM lParam)
{
	auto anI = reinterpret_cast<LPANIMATEINFO>(lParam);
	if (anI != nullptr)
	{
		RECT rc;
		GetClientRect(hWnd, &rc);

		HDC hdc = GetDC(hWnd);
		if (hdc)
		{
			HDC hdc_mem = CreateCompatibleDC(hdc);
			if (hdc_mem)
			{
				HBITMAP dcBit = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
				if (dcBit != nullptr)
				{
					HGDIOBJ org_mem = SelectObject(hdc_mem, dcBit);

					FillRect(hdc_mem, &rc, this->bkgndBrush);
					DrawIconEx(hdc_mem, anI->aPt.x, anI->aPt.y, this->iIcon, 16, 16, 0, nullptr, DI_NORMAL);

					if (anI->numMover > 1)
					{
						if (anI->precedingMover != nullptr)
						{
							for (int i = 0; i < (anI->numMover - 1); i++)
							{
								if (anI->precedingMover[i].started == TRUE)
								{
									DrawIconEx(hdc_mem, anI->precedingMover[i].aPt.x, anI->precedingMover[i].aPt.y, this->iIcon, 16, 16, 0, nullptr, DI_NORMAL);
								}
							}
						}
					}
					BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdc_mem, 0, 0, SRCCOPY);
					SelectObject(hdc_mem, org_mem);
					DeleteObject(dcBit);
				}
				DeleteDC(hdc_mem);
			}
			ReleaseDC(hWnd, hdc);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT workingIndicator::onExTimer(HWND hWnd, WPARAM wParam)
{
	switch (wParam)
	{
	case ID_EXWORKING_TIMER:
		RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE);
		break;
	default:
		break;
	}
	return static_cast<LRESULT>(0);
}

int workingIndicator::calculateHeigtFromLength(int length, int radius)
{
	 return (int)(sqrt(
		 (pow(
			 radius, 2)
			 - pow(
				 length, 2)
			 )
		)
	);
}

void workingIndicator::animateEx(HWND hWnd, HDC hdc)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	HDC hdcMem = CreateCompatibleDC(hdc);
	if (hdcMem)
	{
		auto compBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
		if (compBmp != nullptr)
		{
			// calculate the speed-transition areas
			int protrection_area_width = static_cast<int>((rc.right / 100) * 6);// % of the width of the host-window
			int slow_area = static_cast<int>((rc.right / 100) * 30);
			int start = static_cast<int>((rc.right - ((2 * protrection_area_width) + slow_area)) / 2);
			int end = start + slow_area + (2 * protrection_area_width);
			int transition_point_one = start + protrection_area_width;
			int transition_point_two = start + slow_area + protrection_area_width;
			
			// select the compatible bitmap
			auto hbmOld = SelectObject(hdcMem, compBmp);

			// erase the bitmap
			FillRect(hdcMem, &rc, this->bkgndBrush);

			// draw the mover
			for (int i = 0; i < 5; i++)
			{
				// only draw the visible elements
				if((this->gInfo.currentPosition[i] > -6) && (this->gInfo.currentPosition[i] < (rc.right + 5)))
					DrawIconEx(hdcMem, this->gInfo.currentPosition[i], 0, this->iIcon, 5, 5, 0, nullptr, DI_NORMAL);

				// set the new loadrunner position
				if (this->gInfo.currentPosition[i] <= start || this->gInfo.currentPosition[i] >= end)
					this->gInfo.currentPosition[i] += 3;
				else if (this->gInfo.currentPosition[i] <= transition_point_one || this->gInfo.currentPosition[i] >= transition_point_two)
					this->gInfo.currentPosition[i] += 2;
				else
					this->gInfo.currentPosition[i]++;
			}
			// restart run
			if (this->gInfo.currentPosition[4] >= (rc.right + 10))
			{
				if (this->delay == FINISH_SEQUENCE)
				{
					this->killEx(0);
				}
				this->gInfo.currentPosition[0] = -10;
				this->gInfo.currentPosition[1] = -50;
				this->gInfo.currentPosition[2] = -90;
				this->gInfo.currentPosition[3] = -130;
				this->gInfo.currentPosition[4] = -170;
			}
			// bitmap output
			BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
			// cleanup
			SelectObject(hdcMem, hbmOld);
			DeleteObject(compBmp);
		}
		DeleteDC(hdcMem);
	}
}

void workingIndicator::animateExOpt(HWND hWnd, HDC hdc)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	// calculate the speed-transition areas
	int protrection_area_width = static_cast<int>((rc.right / 100) * 15);// % of the width of the host-window
	int slow_area = static_cast<int>((rc.right / 100) * 30);
	int start = static_cast<int>((rc.right - ((2 * protrection_area_width) + slow_area)) / 2);
	int end = start + slow_area + (2 * protrection_area_width);
	int transition_point_one = start + protrection_area_width;
	int transition_point_two = start + slow_area + protrection_area_width;

	// draw the mover
	for (int i = 0; i < 5; i++)
	{
		// clear the gap-pixels
		RECT front, tail, mover;
		SetRect(&front, this->gInfo.currentPosition[i] - 4, 2, this->gInfo.currentPosition[i] - 1, 4);
		FillRect(hdc, &front, this->bkgndBrush);
		SetRect(&tail, this->gInfo.currentPosition[i] + 2, 2, this->gInfo.currentPosition[i] + 6, 4);
		FillRect(hdc, &tail, this->bkgndBrush);
		SetRect(&mover, this->gInfo.currentPosition[i], 2, this->gInfo.currentPosition[i] + 1, 4);
		FillRect(hdc, &mover, this->moverBrush);


		//// only draw the visible elements
		//if ((this->gInfo.currentPosition[i] > -6) && (this->gInfo.currentPosition[i] < (rc.right + 5)))
		//	DrawIconEx(hdc, this->gInfo.currentPosition[i], 0, this->iIcon, 5, 5, 0, nullptr, DI_NORMAL);

		//// clear the gap-pixels
		//RECT front, tail;
		//SetRect(&front, this->gInfo.currentPosition[i] - 5, 0, this->gInfo.currentPosition[i] - 1, 5);
		//FillRect(hdc, &front, this->bkgndBrush);
		//SetRect(&tail, this->gInfo.currentPosition[i] + 5, 0, this->gInfo.currentPosition[i] + 10, 5);
		//FillRect(hdc, &tail, this->bkgndBrush);

		// set the new loadrunner position
		if (this->gInfo.currentPosition[i] <= start || this->gInfo.currentPosition[i] >= end)
			this->gInfo.currentPosition[i] += 3;
		else if (this->gInfo.currentPosition[i] <= transition_point_one || this->gInfo.currentPosition[i] >= transition_point_two)
			this->gInfo.currentPosition[i] += 2;
		else
			this->gInfo.currentPosition[i]++;

		
	}
	// restart run
	if (this->gInfo.currentPosition[4] >= (rc.right + 10))
	{
		if (this->delay == FINISH_SEQUENCE)
		{
			this->killEx(0);
		}
		this->gInfo.currentPosition[0] = -10;
		this->gInfo.currentPosition[1] = -50;
		this->gInfo.currentPosition[2] = -90;
		this->gInfo.currentPosition[3] = -130;
		this->gInfo.currentPosition[4] = -170;
	}
}