#include"CnCS_UI.h"
#include"ApplicationData.h"
#include"DPI_Helper.h"
#include"CnCSuite_Property.h"
#include"DataExchange\DataExchange.h"

CnCS_UI::CnCS_UI(HINSTANCE hInst, LPAPPSTYLEINFO pStyleInfo)
	:MainWindow(nullptr),
	hInstance(hInst),
	startup_result(TRUE),
	pObj(nullptr),
	iParam(nullptr)
{
	copyAppStyleInfo(pStyleInfo, &this->styleInfo);

	this->pObj = new STATICOBJECTS;
	if (this->pObj)
	{
		SecureZeroMemory(this->pObj, sizeof(STATICOBJECTS));

		this->_createDpiDependedResources(false);

		// CREATE BRUSHES
		this->pObj->backgroundBrush = CreateSolidBrush(pStyleInfo->Background);
		this->pObj->titlebarBrush = CreateSolidBrush(pStyleInfo->titlebarColor);
		this->pObj->sizeWndBrush = CreateSolidBrush(pStyleInfo->SizeWndColor);
		this->pObj->frameWndBrush = CreateSolidBrush(pStyleInfo->Stylecolor);
		this->pObj->statusbarBrush = CreateSolidBrush(pStyleInfo->OutlineColor);
		this->pObj->styleBrush = CreateSolidBrush(pStyleInfo->Stylecolor);
		this->pObj->mainToolbarBrush = CreateSolidBrush(pStyleInfo->mainToolbarColor);
		// LOAD ICONS
		this->pObj->size_arrow = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_SIZE_ARROW), IMAGE_ICON, 22, 22, LR_DEFAULTCOLOR);
		this->pObj->size_arrow_disabled = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_SIZE_ARROW_DISABLED), IMAGE_ICON, 22, 22, LR_DEFAULTCOLOR);
		this->pObj->size_cross = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_SIZECROSS), IMAGE_ICON, 128, 128, LR_DEFAULTCOLOR);
		// Create Pens
		this->pObj->outlinePen = CreatePen(PS_SOLID, 1, InvertColor(this->styleInfo.titlebarColor));
		this->pObj->separatorPen = CreatePen(PS_SOLID, 1, InvertColor(this->styleInfo.Background));
		// LOAD CURSORS
		this->pObj->size_nwso = LoadCursor(NULL, IDC_SIZENWSE);
		this->pObj->arrow = LoadCursor(NULL, IDC_ARROW);
		this->pObj->size_ns = LoadCursor(NULL, IDC_SIZENS);
		this->pObj->size_we = LoadCursor(NULL, IDC_SIZEWE);
	}
	else
		this->startup_result = FALSE;

	this->iParam = new IPARAM;
	if (this->iParam)
	{
		SecureZeroMemory(this->iParam, sizeof(IPARAM));

		this->iParam->DRAW_MODE = DRAW_ALL_AREAS;
		this->iParam->sizingStartHeight_Editframe = -1;
		this->iParam->sizingStartHeight_Main = -1;
	}
	else
		this->startup_result = FALSE;

	if (FAILED(this->RegisterChildClasses()))
	{
		this->startup_result = FALSE;
	}

	for (int i = 0; i < 3; i++)
	{
		this->statusbarTextHolder[i].Replace(L"");
	}
}

CnCS_UI::~CnCS_UI()
{
	UnhookWindowsHookEx(this->iParam->MouseHook);

	if (this->pObj != NULL)
	{
		// DELETE BITMAPS
		//DeleteObject(this->pObj->upperleft);
		//DeleteObject(this->pObj->upperright);
		//DeleteObject(this->pObj->titlebar);
		//DeleteObject(this->pObj->size_cross);
		// DELETE FONTS
		DeleteObject(this->pObj->titlefont);
		DeleteObject(this->pObj->statusbarfont);
		// DELETE BRUSHES
		DeleteObject(this->pObj->backgroundBrush);
		DeleteObject(this->pObj->titlebarBrush);
		DeleteObject(this->pObj->sizeWndBrush);
		DeleteObject(this->pObj->frameWndBrush);
		DeleteObject(this->pObj->statusbarBrush);
		DeleteObject(this->pObj->styleBrush);
		DeleteObject(this->pObj->mainToolbarBrush);
		// DESTROY ICONS
		DestroyIcon(this->pObj->maximize_norm);
		DestroyIcon(this->pObj->maximize_marked);
		DestroyIcon(this->pObj->maximize_pressed);
		DestroyIcon(this->pObj->maximize_norm_min);
		DestroyIcon(this->pObj->maximize_marked_min);
		DestroyIcon(this->pObj->maximize_pressed_min);
		DestroyIcon(this->pObj->minimize_norm);
		DestroyIcon(this->pObj->minimize_marked);
		DestroyIcon(this->pObj->minimize_pressed);
		DestroyIcon(this->pObj->close_norm);
		DestroyIcon(this->pObj->close_mark);
		DestroyIcon(this->pObj->close_pressed);
		DestroyIcon(this->pObj->size_arrow);
		DestroyIcon(this->pObj->size_arrow_disabled);
		DestroyIcon(this->pObj->new_norm);
		DestroyIcon(this->pObj->new_mark);
		DestroyIcon(this->pObj->new_pressed);
		DestroyIcon(this->pObj->open_norm);
		DestroyIcon(this->pObj->open_mark);
		DestroyIcon(this->pObj->open_pressed);
		DestroyIcon(this->pObj->save_norm);
		DestroyIcon(this->pObj->save_mark);
		DestroyIcon(this->pObj->save_pressed);
		DestroyIcon(this->pObj->saveall_norm);
		DestroyIcon(this->pObj->saveall_mark);
		DestroyIcon(this->pObj->saveall_pressed);
		DestroyIcon(this->pObj->saveas_norm);
		DestroyIcon(this->pObj->saveas_mark);
		DestroyIcon(this->pObj->saveas_pressed);
		DestroyIcon(this->pObj->copy_norm);
		DestroyIcon(this->pObj->copy_mark);
		DestroyIcon(this->pObj->copy_pressed);
		DestroyIcon(this->pObj->dropdown_norm);
		DestroyIcon(this->pObj->dropdown_mark);
		DestroyIcon(this->pObj->dropdown_pressed);
		DestroyIcon(this->pObj->paste_norm);
		DestroyIcon(this->pObj->paste_mark);
		DestroyIcon(this->pObj->paste_pressed);
		DestroyIcon(this->pObj->cut_norm);
		DestroyIcon(this->pObj->cut_mark);
		DestroyIcon(this->pObj->cut_pressed);
		DestroyIcon(this->pObj->selectall_norm);
		DestroyIcon(this->pObj->selectall_mark);
		DestroyIcon(this->pObj->selectall_pressed);
		DestroyIcon(this->pObj->editdelete_norm);
		DestroyIcon(this->pObj->editdelete_mark);
		DestroyIcon(this->pObj->editdelete_pressed);
		DestroyIcon(this->pObj->export_norm);
		DestroyIcon(this->pObj->export_mark);
		DestroyIcon(this->pObj->export_pressed);
		DestroyIcon(this->pObj->import_norm);
		DestroyIcon(this->pObj->import_mark);
		DestroyIcon(this->pObj->import_pressed);
		DestroyIcon(this->pObj->send_norm);
		DestroyIcon(this->pObj->send_mark);
		DestroyIcon(this->pObj->send_pressed);
		DestroyIcon(this->pObj->receive_norm);
		DestroyIcon(this->pObj->receive_mark);
		DestroyIcon(this->pObj->receive_pressed);
		DestroyIcon(this->pObj->help_norm);
		DestroyIcon(this->pObj->help_mark);
		DestroyIcon(this->pObj->help_pressed);
		DestroyIcon(this->pObj->appSettingsIcon_norm);
		DestroyIcon(this->pObj->appSettingsIcon_marked);
		DestroyIcon(this->pObj->appSettingsIcon_pressed);
		DestroyIcon(this->pObj->appSysIcon);
		DestroyIcon(this->pObj->web_norm);
		DestroyIcon(this->pObj->web_mark);
		DestroyIcon(this->pObj->web_pressed);
		DestroyIcon(this->pObj->history_norm);
		DestroyIcon(this->pObj->history_mark);
		DestroyIcon(this->pObj->history_pressed);
		DestroyIcon(this->pObj->size_cross);
		// DESTROY PENS
		DeleteObject(this->pObj->outlinePen);
		DeleteObject(this->pObj->separatorPen);
		// DESTROY CURSORS
		DestroyCursor(this->pObj->arrow);
		DestroyCursor(this->pObj->size_ns);
		DestroyCursor(this->pObj->size_nwso);
		DestroyCursor(this->pObj->size_we);
		// DELETE STRUCTURE
		delete this->pObj;
		this->pObj = NULL;
	}
	if (this->iParam != NULL)
	{
		delete this->iParam;
		this->iParam = NULL;
	}
	// unregister classes
	UnregisterClass(L"SIZE_CLASS", this->hInstance);
	UnregisterClass(UNI_FRAME_CLASS, this->hInstance);
}

HRESULT CnCS_UI::Init_UI(HWND hWnd, LPCSTMFRAME cFrameSize)
{
	if (!hWnd || !this->startup_result)
	{
		return E_FAIL;
	}
	else
	{
		DWORD ThreadId = GetCurrentThreadId();
		this->MainWindow = hWnd;

		if (cFrameSize->valid)
		{
			this->iParam->WindowSizeUserdefined = TRUE;
			this->iParam->TVFrameWidth = cFrameSize->TVFrameWidth;
			this->iParam->EditFrameHeight = cFrameSize->EditFrameHeight;

			if ((cFrameSize->nCmdShow == SW_SHOWMAXIMIZED) || (cFrameSize->valid == MAXIMIZENORMAL))
			{
				this->iParam->WindowIsMaximized = TRUE;
			}
		}
		if (!SetWindowsHookEx(WH_MOUSE, CnCS_UI::MouseProc, NULL, ThreadId))
		{
			return E_FAIL;
		}
	}
	return this->Init_Childs() ? S_OK : E_NOINTERFACE;
}

HRESULT CnCS_UI::RegisterChildClasses()
{
	HRESULT hr;

	WNDCLASSEX wcx;
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = sizeof(LONG_PTR);
	wcx.lpfnWndProc = CnCS_UI::SizeWndProc;
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.hbrBackground = NULL;
	wcx.hIcon = NULL;
	wcx.hInstance = this->hInstance;
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = L"SIZE_CLASS";
	wcx.hIconSm = NULL;

	hr = (RegisterClassEx(&wcx) == 0) ? E_FAIL : S_OK;
	if (SUCCEEDED(hr))
	{
		WNDCLASSEX wcxx;
		wcxx.cbSize = sizeof(WNDCLASSEX);
		wcxx.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wcxx.cbClsExtra = 0;
		wcxx.cbWndExtra = sizeof(LONG_PTR);
		wcxx.lpfnWndProc = CnCS_UI::FrameWndProc;
		wcxx.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcxx.hbrBackground = NULL;
		wcxx.hIcon = NULL;
		wcxx.hInstance = this->hInstance;
		wcxx.lpszMenuName = NULL;
		wcxx.lpszClassName = UNI_FRAME_CLASS;
		wcxx.hIconSm = NULL;

		hr = (RegisterClassEx(&wcxx) == 0) ? E_FAIL : S_OK;
		if (SUCCEEDED(hr))
		{
			// .....
		}
	}
	return hr;
}

BOOL CnCS_UI::Init_Childs()
{
	BOOL res = TRUE;
	RECT rc;
	GetClientRect(this->MainWindow, &rc);

	// Create static buttons
	if (!this->MakeButton(nullptr, DPIScale(130), 0, DPIScale(24), DPIScale(24), IDM_APPSETTINGS))
		return FALSE;
	if (!this->MakeButton(nullptr, rc.right - DPIScale(24), 0, DPIScale(24), DPIScale(24), IDM_CLOSE))
		return FALSE;
	if (!this->MakeButton(nullptr, rc.right - DPIScale(52), 0, DPIScale(24), DPIScale(24), IDM_MAXIMIZE))
		return FALSE;
	if (!this->MakeButton(nullptr, rc.right - DPIScale(80), 0, DPIScale(24), DPIScale(24), IDM_MINIMIZE))
		return FALSE;
	if (!this->MakeButton(nullptr, DPIScale(160), 0, DPIScale(24), DPIScale(24), IDM_HELP))
		return FALSE;
	if (!this->MakeButton(nullptr, DPIScale(190), 0, DPIScale(24), DPIScale(24), IDM_WEB))
		return FALSE;
	if (!this->MakeButton(nullptr, DPIScale(220), 0, DPIScale(24), DPIScale(24), IDM_HISTORY))
		return FALSE;

	///////////////////////////////////////////////////////////////////////////////////
	// Create Frames and auxilaries >>
	res = this->CreateStatusBar();
	if (res)
	{
		res = this->CreateTVFrame();
		if (res)
		{
			res = this->CreateEditFrame();
			if (res)
			{
				res = this->CreateCBoxFrame();
				if (res)
				{
					res = this->CreateTBButtons();
				}
			}
		}
	}
	return res;
}

BOOL CnCS_UI::CreateStatusBar()
{
	int parts[3] = { 0 };

	RECT rc;
	GetClientRect(this->MainWindow, &rc);

	parts[0] = rc.right / 2;
	parts[1] = (rc.right / 2) + DPIScale(80);
	parts[2] = rc.right;

	InitCommonControls();

	this->iParam->statusBar_WND = CreateWindowEx(
		WS_EX_COMPOSITED,
		STATUSCLASSNAME,
		nullptr,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		this->MainWindow,
		(HMENU)ID_STATUSBAR,
		this->hInstance,
		nullptr
	);

	if (!this->iParam->statusBar_WND)
		return FALSE;
	else
	{
		SendMessage(this->iParam->statusBar_WND, SB_SETPARTS, (WPARAM)3, reinterpret_cast<LPARAM>(&parts));
		SendMessage(this->iParam->statusBar_WND, SB_SETMINHEIGHT, (WPARAM)DPIScale(22), 0);

		this->ChangeStatusbar(0, getStringFromResource(UI_STATUSBAR_READY_STATE));

		auto dataCont =
			reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer()
			);
		if (dataCont != nullptr)
		{
			iString com(L"COM ");

			auto comPort = dataCont->getIntegerData(DATAKEY_SETTINGS_CURRENTCOMPORT, 1);
			com += comPort;
			this->ChangeStatusbar(1, com.GetData());
		}
		this->ChangeStatusbar(2, L"-- -- --");

		return SetWindowSubclass(this->iParam->statusBar_WND, CnCS_UI::StatusbarSub, NULL, reinterpret_cast<DWORD_PTR>(this));
	}
}

BOOL CnCS_UI::CreateTVFrame()
{
	RECT rc, rc_statusbar;
	GetClientRect(this->MainWindow, &rc);
	GetWindowRect(this->iParam->statusBar_WND, &rc_statusbar);

	int cx;

	if (this->iParam->WindowSizeUserdefined)
		cx = this->iParam->TVFrameWidth;
	else
		cx = rc.right / 4;

	this->iParam->TVFrame_WND =
		CreateWindowEx(
			0,
			UNI_FRAME_CLASS,
			nullptr,
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			0, DPIScale(24),
			cx,
			rc.bottom - (DPIScale(25) + (rc_statusbar.bottom - rc_statusbar.top)),
			this->MainWindow,
			(HMENU)ID_TVFRAME,
			this->hInstance,
			nullptr
		);

	if (this->iParam->TVFrame_WND)
		return TRUE;
	else
		return FALSE;
}

BOOL CnCS_UI::CreateEditFrame()
{
	RECT rc, rc_statusbar;
	GetClientRect(this->MainWindow, &rc);
	GetWindowRect(this->iParam->statusBar_WND, &rc_statusbar);

	int x, cx, cy;

	if (this->iParam->WindowSizeUserdefined)
	{
		x = this->iParam->TVFrameWidth + 5;
		cx = rc.right - (this->iParam->TVFrameWidth + 5);
		cy = this->iParam->EditFrameHeight;
	}
	else
	{
		x = (rc.right / 4) + 5;
		cx = rc.right - x;
		cy = rc.bottom - (DPIScale(68) + (rc_statusbar.bottom - rc_statusbar.top) + DPIScale(105));// 105 ?? CBox-height + gap (5)??
	}
	this->iParam->EditFrame_WND =
		CreateWindowEx(
			0,
			UNI_FRAME_CLASS,
			nullptr,
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			x, DPIScale(68),
			cx, cy,
			this->MainWindow,
			(HMENU)ID_TABCTRLFRAME,
			this->hInstance,
			nullptr
		);

	if (this->iParam->EditFrame_WND)
		return TRUE;
	else
		return FALSE;
}

BOOL CnCS_UI::CreateCBoxFrame()
{
	RECT rc, rc_statusbar;
	GetClientRect(this->MainWindow, &rc);
	GetWindowRect(this->iParam->statusBar_WND, &rc_statusbar);

	int x, y, cx, cy;

	if (this->iParam->WindowSizeUserdefined)
	{
		x = this->iParam->TVFrameWidth + DPIScale(5);
		y = DPIScale(68) + this->iParam->EditFrameHeight + DPIScale(5);
		cx = rc.right - (this->iParam->TVFrameWidth + DPIScale(5));
		cy = rc.bottom - (this->iParam->EditFrameHeight + (rc_statusbar.bottom - rc_statusbar.top) + DPIScale(68) + DPIScale(5));
	}
	else
	{
		x = (rc.right / 4) + DPIScale(5);
		y = rc.bottom - ((rc_statusbar.bottom - rc_statusbar.top) + DPIScale(100));
		cx = rc.right - x;
		cy = DPIScale(100);
	}
	this->iParam->CBoxFrame_WND =
		CreateWindowEx(
			0,
			UNI_FRAME_CLASS,
			nullptr,
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			x, y,
			cx, cy,
			this->MainWindow,
			(HMENU)ID_CBOXFRAME,
			this->hInstance,
			nullptr
		);

	if (this->iParam->CBoxFrame_WND)
		return TRUE;
	else
		return FALSE;
}

BOOL CnCS_UI::CreateTBButtons()
{
	RECT rc, rc_tvFrame;
	GetClientRect(this->MainWindow, &rc);
	GetWindowRect(this->iParam->TVFrame_WND, &rc_tvFrame);

	int X_pos = (rc_tvFrame.right - rc_tvFrame.left) + DPIScale(5);

	int i = 0;

	// Create toolbar-buttons :: amount depending on the window width
	while ((X_pos < (rc.right - (2*DPIScale(52)))) && ((IDM_NEW + i) < TBBUTTON_END))
	{
		if (!this->MakeButton(
			nullptr,
			X_pos,
			DPIScale(25),
			DPIScale(42),
			DPIScale(42), 
			IDM_NEW + i
		))
			return FALSE;

		X_pos += DPIScale(52);
		i++;
	}
	// Save the toolbar condition >>
	if ((IDM_NEW + i) < TBBUTTON_END)
	{
		if (!this->MakeButton(
				nullptr,
				rc.right - DPIScale(42),
				DPIScale(44),
				DPIScale(22),
				DPIScale(22),
				IDM_TOOLBAR_DROPDOWN
			))
				return FALSE;

		this->iParam->LAST_TBBUTTON_ID = IDM_NEW + (i - 1);
		this->iParam->DropdownbuttonIsVisible = TRUE;
	}
	else
		this->iParam->LAST_TBBUTTON_ID = TBBUTTON_END - 1;

	return TRUE;
}

BOOL CnCS_UI::CreateExtendedToolbar()
{
	RECT rc;
	GetClientRect(this->MainWindow, &rc);

	int num_buttons = (TBBUTTON_END - 1) - this->iParam->LAST_TBBUTTON_ID;
	if (num_buttons > 0)
	{
		POINT pt;
		pt.x = rc.right - DPIScale(57);
		pt.y = DPIScale(68);
		ClientToScreen(this->MainWindow, &pt);

		this->iParam->extendedToolbar_WND =
			CreateWindowEx(
				WS_EX_TOOLWINDOW,
				L"STATIC", NULL,
				WS_POPUP | WS_VISIBLE | WS_BORDER,
				pt.x, pt.y,
				DPIScale(52),
				DPIScale(52) + (DPIScale(47) * (num_buttons - 1)),
				this->MainWindow,
				NULL,
				this->hInstance, NULL
			);

		if (this->iParam->extendedToolbar_WND)
		{
			this->iParam->ext_toolbar_exsisting = TRUE;

			if (!SetWindowSubclass(this->iParam->extendedToolbar_WND, CnCS_UI::ExtendedToolbarSub, NULL, reinterpret_cast<DWORD_PTR>(this)))
				return FALSE;

			for (int i = 0; i < num_buttons; i++)
			{
				if (!MakeButton(
					this->iParam->extendedToolbar_WND,
					DPIScale(5),
					DPIScale(5) + DPIScale(47 * i),
					DPIScale(42),
					DPIScale(42),
					(this->iParam->LAST_TBBUTTON_ID + 1) + i)
					)
					return FALSE;
			}
		}
		else
			return FALSE;
	}
	return TRUE;
}

BOOL CnCS_UI::CreateSizeBeam(LPRECT rc_beam, int alignment)
{
	RECT rc;
	GetWindowRect(this->MainWindow, &rc);

	this->iParam->SizeBeam_WND = CreateWindowEx(
		WS_EX_LAYERED,
		L"SIZE_CLASS", NULL,
		WS_POPUP | WS_VISIBLE,
		rc_beam->left, rc_beam->top,
		rc_beam->right - rc_beam->left,
		rc_beam->bottom - rc_beam->top,
		this->MainWindow,
		NULL,
		this->hInstance,
		reinterpret_cast<LPVOID>(this));

	if (this->iParam->SizeBeam_WND)
	{
		SetCapture(this->MainWindow);
		SetLayeredWindowAttributes(this->iParam->SizeBeam_WND, 0, 150, LWA_ALPHA);
		this->iParam->FrameSizeAreaActivated = alignment;

		if (alignment == WE_SIZEAREA_SIZING)
		{
			rc.left += 80;
			rc.top += 80;
			rc.right -= 80;
			rc.bottom -= 24;

			ClipCursor(&rc);
		}
		else if (alignment == NS_SIZEAREA_SIZING)
		{
			rc.left = this->iParam->rc_multiU.left;
			rc.top += 180;
			rc.bottom -= 74;

			ClipCursor(&rc);
		}
		return TRUE;
	}
	else
		return FALSE;
}

LRESULT CnCS_UI::_DefaultHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_PAINT:
			return this->OnPaint(hWnd);
		case WM_SYSCOMMAND:
			return this->OnSysCommand(hWnd, wParam, lParam);
		case WM_DRAWITEM:
			return this->OnDrawItem(hWnd, lParam);
		case WM_NORMALIZE:
			return this->OnNormalize(hWnd, wParam);
		case WM_GETUIINSTANCE:
			return reinterpret_cast<LRESULT>(this);
		case WM_SIZE:
			return this->OnSize(hWnd, wParam, lParam);
		case WM_MOVE:
			return this->OnMove(hWnd);
		case WM_TIMER:
			return this->OnTimer(hWnd, wParam);
		case WM_GETMINMAXINFO:
			return this->OnMinMaxInfo(lParam);
		case WM_MOUSEMOVE:
			return this->OnMouseMove(hWnd, wParam, lParam);
		case WM_LBUTTONDOWN:
			return this->OnLButtondown(hWnd, wParam, lParam);
		case WM_LBUTTONUP:
			return this->OnLButtonUp(hWnd, lParam);
		case WM_ERASEBKGND:
			return this->OnEraseBkgnd(wParam);
		case WM_SETAPPSTYLE:
			return this->OnSetAppStyle(lParam);
		case WM_CTLCOLORSTATIC:
			return reinterpret_cast<LRESULT>(this->pObj->mainToolbarBrush);
		case WM_PROCESSNEWINSTANCE:
			return this->OnProcessNewInstance();
		case WM_BLOCKINPUT:
			return this->OnBlockInput(wParam);
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

LRESULT CnCS_UI::ButtonSub(HWND button, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR refData)
{
	UNREFERENCED_PARAMETER(uID);
	
	CnCS_UI* pUI = reinterpret_cast<CnCS_UI*>(refData);
	if (pUI != NULL)
	{
		if (!pUI->iParam->blockButtonAnimation)
		{
			int ID_ = static_cast<int>(GetWindowLongPtr(button, GWLP_ID));

			switch (message)
			{
			case WM_MOUSEMOVE:
				pUI->DrawButtonFromID(button, NULL, ID_, BDRAW_MARKED);
				return 0;
			case WM_LBUTTONDOWN:
				pUI->DrawButtonFromID(button, NULL, ID_, BDRAW_PRESSED);
				return 0;
			case WM_LBUTTONUP:
				pUI->HandleSpecialActions(ID_);
				pUI->DrawButtonFromID(button, NULL, ID_, BDRAW_MARKED);
				SendMessage(pUI->MainWindow, WM_COMMAND, MAKEWPARAM(ID_, 0), 0);
				return 0;
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(1);
			default:
				break;
			}
		}
	}
	return DefSubclassProc(button, message, wParam, lParam);
}

LRESULT CnCS_UI::StatusbarSub(HWND Statusbar, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR refData)
{
	UNREFERENCED_PARAMETER(uID);

	CnCS_UI* pUI = reinterpret_cast<CnCS_UI*>(refData);
	if (pUI != NULL)
	{
		if (pUI->iParam->WindowIsMaximized)
			return DefSubclassProc(Statusbar, message, wParam, lParam);

		switch (message)
		{
			case WM_MOUSEMOVE:
			{
				RECT rc;
				GetClientRect(Statusbar, &rc);

				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				if ((x > (rc.right - 23)) && (x < (rc.right)) && (y > 0) && (y < (rc.bottom)))
				{
					SetCursor(pUI->pObj->size_nwso);
				}
				return static_cast<LRESULT>(0);
			}
			case WM_LBUTTONDOWN:
			{
				RECT rc;
				GetClientRect(Statusbar, &rc);

				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);

				if ((x > (rc.right - 23)) && (x < (rc.right)) && (y > 0) && (y < (rc.bottom)))
				{
					RECT mainWnd;
					GetClientRect(pUI->MainWindow, &mainWnd);

					pUI->SavePreSizingParameter(&mainWnd, nullptr);

					SetCapture(Statusbar);
					//SetCursor(pUI->pObj->size_nwso); // redundant

					pUI->iParam->SizeAreaActivated = TRUE;
					pUI->iParam->InitialPos_X = rc.right - x;
					pUI->iParam->InitialPos_Y = rc.bottom - y;

					ShowWindow(pUI->iParam->TVFrame_WND, SW_HIDE);
					ShowWindow(pUI->iParam->CBoxFrame_WND, SW_HIDE);
					ShowWindow(pUI->iParam->EditFrame_WND, SW_HIDE);

					pUI->ControlFloatingWndShowState(SW_HIDE);
				}
				return static_cast<LRESULT>(0);
			}
			case WM_LBUTTONUP:
			{
				if (pUI->iParam->SizeAreaActivated)
				{
					pUI->iParam->SizeAreaActivated = FALSE;
					SetCursor(pUI->pObj->arrow);
					ReleaseCapture();

					pUI->AdaptEditframeHeight();
					pUI->OnRestoreFrames();
					pUI->OnRestoreFloatingWnd();

					RECT rc;
					GetClientRect(pUI->MainWindow, &rc);

					RECT rc_fill = { (rc.right / 2) - 64, (rc.bottom / 2) - 64,(rc.right / 2) + 64, (rc.bottom / 2) + 64 };
					RedrawWindow(pUI->MainWindow, &rc_fill, NULL, RDW_NOCHILDREN | RDW_NOERASE | RDW_INVALIDATE);

					ShowWindow(pUI->iParam->TVFrame_WND, SW_SHOW);
					ShowWindow(pUI->iParam->CBoxFrame_WND, SW_SHOW);
					ShowWindow(pUI->iParam->EditFrame_WND, SW_SHOW);

					pUI->ControlFloatingWndShowState(SW_SHOW);
				}
				return static_cast<LRESULT>(0);
			}
		}
	}
	return DefSubclassProc(Statusbar, message, wParam, lParam);
}

LRESULT CnCS_UI::ExtendedToolbarSub(HWND E_toolbar, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR refData)
{
	UNREFERENCED_PARAMETER(uID);

	CnCS_UI* pUI = reinterpret_cast<CnCS_UI*>(refData);
	if (pUI != NULL)
	{
		if (message == WM_DRAWITEM)
		{
			SendMessage(pUI->MainWindow, message, wParam, lParam);
		}
	}
	return DefSubclassProc(E_toolbar, message, wParam, lParam);
}

LRESULT CnCS_UI::SizeWndProc(HWND size, UINT message, WPARAM wParam, LPARAM lParam)
{
	CnCS_UI* pUI = NULL;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pUI = reinterpret_cast<CnCS_UI*>(pcr->lpCreateParams);
		SetWindowLongPtr(size, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pUI));

		return 1;
	}
	else
	{
		pUI = reinterpret_cast<CnCS_UI*>(GetWindowLongPtr(size, GWLP_USERDATA));
		if (pUI != NULL)
		{
			// ....... ?

			if (message == WM_ERASEBKGND)
			{
				RECT rc;
				GetClientRect(size, &rc);

				FillRect(reinterpret_cast<HDC>(wParam), &rc, pUI->pObj->sizeWndBrush);
			}
		}
		return DefWindowProc(size, message, wParam, lParam);
	}
}

LRESULT CnCS_UI::FrameWndProc(HWND frame, UINT message, WPARAM wParam, LPARAM lParam)
{
	CnCS_UI* pUI = NULL;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pUI = reinterpret_cast<CnCS_UI*>(pcr->lpCreateParams);
		SetWindowLongPtr(frame, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pUI));

		return 1;
	}
	else
	{
		pUI = reinterpret_cast<CnCS_UI*>(GetWindowLongPtr(frame, GWLP_USERDATA));
		if (pUI != NULL)
		{
			// ....... ?
		}
		return DefWindowProc(frame, message, wParam, lParam);
	}
}

LRESULT CnCS_UI::OnMinMaxInfo(LPARAM lParam)
{
	LPMINMAXINFO pmmi = reinterpret_cast<LPMINMAXINFO>(lParam);
	if(pmmi)
	{ 
		pmmi->ptMinTrackSize.x = 600;
		pmmi->ptMinTrackSize.y = 400;
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_UI::OnPaint(HWND hWnd)
{
	if (this->iParam->DRAW_MODE == DRAW_ALL_AREAS)
	{
		this->DrawAll(hWnd);
		this->iParam->DRAW_MODE = DRAW_NCS_AREAS;
	}
	else
	{
		//this->DrawNeccessaryAreas(hWnd);
		this->DrawAll(hWnd);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_UI::OnDrawItem(HWND hWnd, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hWnd);

	DRAWITEMSTRUCT* pdi = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
	if (pdi != NULL)
	{
		ItemType type = this->WhatIsItem(pdi->CtlID);
		switch (type)
		{
		case IS_BUTTON_ITEM:
			this->DrawButtonFromID(pdi->hwndItem, pdi, pdi->CtlID, BDRAW_NORMAL);
			break;
		case IS_STATUSBAR_ITEM:
			this->DrawStatusbar(pdi);
			break;
		default:
			break;
		}
		
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_UI::OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	RECT rc, rc_tv, rc_ed, rc_cb;
	GetWindowRect(hWnd, &rc);
	GetWindowRect(this->iParam->TVFrame_WND, &rc_tv);
	GetWindowRect(this->iParam->EditFrame_WND, &rc_ed);
	GetWindowRect(this->iParam->CBoxFrame_WND, &rc_cb);

	POINT pt;
	GetCursorPos(&pt);

	if (wParam & MK_LBUTTON)
	{
		if (this->iParam->MoveAreaActivated)
		{
			MoveWindow(hWnd, pt.x - this->iParam->InitialPos_X, pt.y - this->iParam->InitialPos_Y, rc.right-rc.left, rc.bottom-rc.top, TRUE);
		}
		if (this->iParam->FrameSizeAreaActivated)
		{
			if (this->iParam->FrameSizeAreaActivated == NS_SIZEAREA_SIZING)
			{
				this->iParam->rc_multiU.top = pt.y - this->iParam->add_val_y;

				MoveWindow(this->iParam->SizeBeam_WND, this->iParam->rc_multiU.left, this->iParam->rc_multiU.top, this->iParam->rc_multiU.right - this->iParam->rc_multiU.left, DPIScale(5), TRUE);

			}
			else if (this->iParam->FrameSizeAreaActivated == WE_SIZEAREA_SIZING)
			{
				this->iParam->rc_multiU.left = pt.x - this->iParam->add_val_x;

				MoveWindow(this->iParam->SizeBeam_WND, this->iParam->rc_multiU.left, this->iParam->rc_multiU.top, DPIScale(5), this->iParam->rc_multiU.bottom - this->iParam->rc_multiU.top, TRUE);
			}
		}
	}
	if ((pt.x < rc_ed.left) && (pt.x > rc_tv.right) && (pt.y > (rc.top + 80)) && (pt.y < (rc.bottom - 24)))
	{
		SetCursor(this->pObj->size_we);
	}
	if ((pt.x < rc_ed.right) && (pt.x > rc_ed.left) && (pt.y > rc_ed.bottom) && (pt.y < rc_cb.top))
	{
		if (this->iParam->FrameSizeAreaActivated != WE_SIZEAREA_SIZING)
		{
			SetCursor(this->pObj->size_ns);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_UI::OnNormalize(HWND hWnd, WPARAM wParam)
{
	UNREFERENCED_PARAMETER(hWnd);
	BOOL result = TRUE;

	// if the setup-window is open the setup-button should not be normalized
	//HWND propWnd = FindWindow(CS_PROPERTYWINDOW_CLASS, nullptr);
	//if (propWnd != nullptr)
	//	return 0;

	if (this->iParam->blockSettingsButtonNormalization)
		return result;

	HWND wndToNorm = this->GetButtonHandle((int)wParam);
	if (wndToNorm)
	{
		this->DrawButtonFromID(wndToNorm, NULL, static_cast<int>(wParam), BDRAW_NORMAL);
	}
	else
		result = FALSE;

	return static_cast<LRESULT>(result);
}

LRESULT CnCS_UI::OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case SIZE_MAXIMIZED:
		this->OnMaximized();
		break;
	case SIZE_MINIMIZED:
		this->iParam->DRAW_MODE = DRAW_ALL_AREAS;
		break;
	case SIZE_RESTORED:
		this->OnRestoreWindow(hWnd, lParam);
		break;
	default:
		break;
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_UI::OnMove(HWND hWnd)
{
	if (!this->iParam->WindowIsMinimized)
	{
		//if (this->iParam->WindowIsMaximized)
		//{
		//	SendMessage(this->MainWindow, WM_SIZE, 0, 0);
		//}
		//else
		//{
			this->OnRestoreFloatingWnd();
			this->WindowChanged(hWnd);
		//}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_UI::OnTimer(HWND hWnd, WPARAM wParam)
{
	if (wParam == ID_TIMER_SAVEFRAMEDATA)
	{
		if (!this->iParam->WindowIsMinimized)
		{
			if (this->iParam->SizeChanged)
			{
				CSTMFRAME ctm;

				if (this->iParam->WindowIsMaximized)
				{
					ctm.nCmdShow = SW_SHOWMAXIMIZED;
					ctm.valid = MAXIMIZENORMAL;					
				}
				else
				{
					ctm.nCmdShow = SW_SHOW;
					ctm.valid = TRUE;
				}
				ctm.EditFrameHeight = this->iParam->EditFrameHeight;
				ctm.TVFrameWidth = this->iParam->TVFrameWidth;

				SendMessage(hWnd, WM_FRAMECHANGED, 0, reinterpret_cast<LPARAM>(&ctm));

				KillTimer(hWnd, ID_TIMER_SAVEFRAMEDATA);
				InterlockedExchange((LONG*)&this->iParam->TimerActive, (LONG)FALSE);
				this->iParam->SizeChanged = FALSE;
			}
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_UI::OnLButtondown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

	RECT rc_client;
	GetClientRect(hWnd, &rc_client);

	RECT rc, rc_tv, rc_ed, rc_cb;
	GetWindowRect(hWnd, &rc);
	GetWindowRect(this->iParam->TVFrame_WND, &rc_tv);
	GetWindowRect(this->iParam->EditFrame_WND, &rc_ed);
	GetWindowRect(this->iParam->CBoxFrame_WND, &rc_cb);

	POINT pt;
	GetCursorPos(&pt);

	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);

	if ((x > 80) && (x < (rc_client.right - 80)) && (y > 0) && (y < 25))
	{
		if (this->iParam->WindowIsMaximized)
			return static_cast<LRESULT>(0);

		SetCapture(hWnd);

		this->iParam->MoveAreaActivated = TRUE;
		this->iParam->InitialPos_X = x;
		this->iParam->InitialPos_Y = y;
	}
	if ((pt.x < rc_ed.left) && (pt.x > rc_tv.right) && (pt.y > (rc.top + 80)) && (pt.y < (rc.bottom - DPIScale(24))))
	{
		SetRect(&this->iParam->rc_multiU, rc_tv.right, rc_tv.top, rc_ed.left, rc.bottom - DPIScale(24));

		this->iParam->add_val_x = pt.x - rc_tv.right;

		this->CreateSizeBeam(&this->iParam->rc_multiU, WE_SIZEAREA_SIZING);
	}
	if ((pt.x < rc_ed.right) && (pt.x > rc_ed.left) && (pt.y > rc_ed.bottom) && (pt.y < rc_cb.top))
	{
		SetRect(&this->iParam->rc_multiU, rc_ed.left, rc_ed.bottom, rc_ed.right, rc_cb.top);

		this->iParam->add_val_y = pt.y - rc_ed.bottom;

		this->CreateSizeBeam(&this->iParam->rc_multiU, NS_SIZEAREA_SIZING);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_UI::OnLButtonUp(HWND hWnd, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(lParam);

	if (this->iParam->MoveAreaActivated)
	{
		this->iParam->MoveAreaActivated = FALSE;
		ReleaseCapture();

		this->iParam->InitialPos_X = 0;
		this->iParam->InitialPos_Y = 0;
	}
	if (this->iParam->FrameSizeAreaActivated)
	{
		this->SetNewFrameAlignment();

		if (this->iParam->SizeBeam_WND)
		{
			DestroyWindow(this->iParam->SizeBeam_WND);
			this->iParam->SizeBeam_WND = NULL;
		}
		ClipCursor(NULL);
		ReleaseCapture();
		this->iParam->FrameSizeAreaActivated = FALSE;
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_UI::OnEraseBkgnd(WPARAM wParam)
{
	UNREFERENCED_PARAMETER(wParam);

	return static_cast<LRESULT>(TRUE);
}

LRESULT CnCS_UI::OnSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case SC_RESTORE:
		this->iParam->WindowIsMinimized = FALSE;	
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, WM_SYSCOMMAND, wParam, lParam);
}

LRESULT CnCS_UI::OnSetAppStyle(LPARAM lParam)
{
	BOOL result = TRUE;

	LPAPPSTYLEINFO pStyleInfo = reinterpret_cast<LPAPPSTYLEINFO>(lParam);
	if (pStyleInfo != NULL)
	{
		copyAppStyleInfo(pStyleInfo, &this->styleInfo);

		//BMPID ids;
		//this->SetBitmapIDs(&ids, pStyleInfo->StyleID);

		DeleteObject(this->pObj->backgroundBrush);
		DeleteObject(this->pObj->sizeWndBrush);
		DeleteObject(this->pObj->frameWndBrush);
		DeleteObject(this->pObj->titlebarBrush);
		DeleteObject(this->pObj->styleBrush);
		DeleteObject(this->pObj->mainToolbarBrush);
		DeleteObject(this->pObj->outlinePen);
		DeleteObject(this->pObj->separatorPen);

		this->pObj->backgroundBrush = CreateSolidBrush(pStyleInfo->Background);
		this->pObj->sizeWndBrush = CreateSolidBrush(pStyleInfo->SizeWndColor);
		this->pObj->frameWndBrush = CreateSolidBrush(pStyleInfo->Stylecolor);
		this->pObj->titlebarBrush = CreateSolidBrush(pStyleInfo->titlebarColor);
		this->pObj->styleBrush = CreateSolidBrush(pStyleInfo->Stylecolor);
		this->pObj->mainToolbarBrush = CreateSolidBrush(pStyleInfo->mainToolbarColor);
		this->pObj->outlinePen = CreatePen(PS_SOLID, 1, pStyleInfo->OutlineColor);
		this->pObj->separatorPen = CreatePen(PS_SOLID, 1, InvertColor(pStyleInfo->Background));

		SendMessage(this->iParam->CBoxFrame_WND, WM_SETAPPSTYLE, 0, lParam);
		SendMessage(this->iParam->TVFrame_WND, WM_SETAPPSTYLE, 0, lParam);
		SendMessage(this->iParam->EditFrame_WND, WM_SETAPPSTYLE, 0, lParam);

		this->iParam->DRAW_MODE = DRAW_ALL_AREAS;
		RedrawWindow(this->MainWindow, NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE);
	}
	else
		result = FALSE;

	return static_cast<LRESULT>(result);
}

LRESULT CnCS_UI::OnProcessNewInstance()
{
	if (this->iParam->WindowIsMinimized)
	{
		this->iParam->WindowIsMinimized = FALSE;

		if (this->iParam->WindowIsMaximized)
		{
			ShowWindow(this->MainWindow, SW_MAXIMIZE);
		}
		else
		{
			ShowWindow(this->MainWindow, SW_RESTORE);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_UI::OnBlockInput(WPARAM wParam)
{
	this->iParam->blockButtonAnimation = (BOOL)wParam;

	return static_cast<LRESULT>(0);
}

LRESULT CALLBACK CnCS_UI::MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	else
	{
		HWND CnCSuite = FindWindow(L"CNCSUITECLASS", NULL);
		if (CnCSuite)
		{
			LPMOUSEHOOKSTRUCT pmh = reinterpret_cast<LPMOUSEHOOKSTRUCT>(lParam);
			if (pmh)
			{
				CnCS_UI* pUI =
					reinterpret_cast<CnCS_UI*>(
						SendMessage(
							CnCSuite,
							WM_GETUIINSTANCE,
							0,0
						)
					);
				if (pUI)
				{
					pUI->Global_Tracking(pmh, pUI);
				}
			}
		}
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
}


void CnCS_UI::OnRestoreWindow(HWND hWnd, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	if (this->iParam->WindowIsMinimized)
		return;
	else
	{
		RECT rc_client;
		auto result = GetClientRect(hWnd, &rc_client);
		if (result)
		{
			if (this->MainWindow != nullptr)
			{
				int cx = rc_client.right;

				MoveWindow(
					GetDlgItem(hWnd, IDM_APPSETTINGS), DPIScale(130), 0, DPIScale(24), DPIScale(24), TRUE);
				MoveWindow(
					GetDlgItem(hWnd, IDM_CLOSE), cx - DPIScale(24), 0, DPIScale(24), DPIScale(24), TRUE);
				MoveWindow(
					GetDlgItem(hWnd, IDM_MAXIMIZE), cx - DPIScale(52), 0, DPIScale(24), DPIScale(24), TRUE);
				MoveWindow(
					GetDlgItem(hWnd, IDM_MINIMIZE), cx - DPIScale(80), 0, DPIScale(24), DPIScale(24), TRUE);
				MoveWindow(
					GetDlgItem(hWnd, IDM_HELP), DPIScale(160), 0, DPIScale(24), DPIScale(24), TRUE);
				MoveWindow(
					GetDlgItem(hWnd, IDM_WEB), DPIScale(190), 0, DPIScale(24), DPIScale(24), TRUE);
				MoveWindow(
					GetDlgItem(hWnd, IDM_HISTORY), DPIScale(220), 0, DPIScale(24), DPIScale(24), TRUE);

				if (this->iParam->DropdownbuttonIsVisible)
				{
					MoveWindow(
						GetDlgItem(hWnd, IDM_TOOLBAR_DROPDOWN), cx - DPIScale(84), DPIScale(55), DPIScale(22), DPIScale(22), TRUE);
				}
				this->OnRestoreStatusbar();
				/*this->OnRestoreToolbar();*/

				if (!this->iParam->SizeAreaActivated)
				{
					this->OnRestoreFrames();
					this->OnRestoreFloatingWnd();
				}

				this->OnRestoreToolbar();

				this->WindowChanged(hWnd);
			}
		}
	}
}

void CnCS_UI::OnRestoreStatusbar()
{
	int parts[3] = { 0 };

	RECT rc;
	GetClientRect(this->MainWindow, &rc);

	parts[0] = rc.right / 2;
	parts[1] = (rc.right / 2) + DPIScale(80);
	parts[2] = rc.right;

	SendMessage(this->iParam->statusBar_WND, SB_SETPARTS, (WPARAM)3, reinterpret_cast<LPARAM>(&parts));
	SendMessage(this->iParam->statusBar_WND, SB_SETMINHEIGHT, (WPARAM)DPIScale(22), 0);

	MoveWindow(
		GetDlgItem(this->MainWindow, ID_STATUSBAR), 0, 0, 0, 0, TRUE);
}

void CnCS_UI::OnRestoreToolbar()
{
	RECT rc, rc_tvFrame;
	GetClientRect(this->MainWindow, &rc);
	GetWindowRect(this->iParam->TVFrame_WND, &rc_tvFrame);

	// get button count
	int cButtons = TBBUTTON_END - IDM_NEW;

	// calculate the button start position
	int startPos = (rc_tvFrame.right - rc_tvFrame.left) + DPIScale(5);

	// calculate the available area for the toolbar (minus one-button-length for the dropdown-button)
	int toolbarArea = rc.right - (startPos + DPIScale(52));

	/*** resize all visible buttons *********************************************/
	int curID = IDM_NEW;

	while (curID <= this->iParam->LAST_TBBUTTON_ID)
	{
		MoveWindow(
			GetDlgItem(this->MainWindow, curID),
			startPos + (DPIScale(52) * (curID - IDM_NEW)),
			DPIScale(25),
			DPIScale(42),
			DPIScale(42),
			TRUE
		);
		curID++;

		if (curID == TBBUTTON_END)
			break;
	}
	/****************************************************************************/


	/*** control the toolbar under- / overflow **********************************/

	// get the max possible button count
	int buttonCount = (int)(toolbarArea / DPIScale(52));

	// control the show state of all buttons
	for (int i = 0; i < cButtons; i++)
	{
		auto hWndButton = GetDlgItem(this->MainWindow, IDM_NEW + i);
		if (hWndButton)
		{
			if (i < buttonCount)
				ShowWindow(hWndButton, SW_SHOW);
			else
				ShowWindow(hWndButton, SW_HIDE);
		}
		else
		{
			if (i < buttonCount)
			{
				// the button wasn't initally created -> do it now
				this->MakeButton(
					nullptr,
					(startPos + (DPIScale(52) * i)),
					DPIScale(25),
					DPIScale(42),
					DPIScale(42),
					IDM_NEW + i
				);

			}
		}
	}

	// control the dropdownbutton-state
	if (buttonCount < (TBBUTTON_END - IDM_NEW))
	{
		this->iParam->LAST_TBBUTTON_ID = IDM_NEW + (buttonCount - 1);

		if (this->iParam->LAST_TBBUTTON_ID < (IDM_NEW - 1))
			this->iParam->LAST_TBBUTTON_ID = (IDM_NEW - 1);

		// not all buttons are visible -> show or create the dropdownbutton
		auto hWndDropdown = GetDlgItem(this->MainWindow, IDM_TOOLBAR_DROPDOWN);
		if (hWndDropdown)
		{
			MoveWindow(
				hWndDropdown,
				rc.right - DPIScale(42),
				DPIScale(44),
				DPIScale(22),
				DPIScale(22),
				TRUE
			);
			ShowWindow(hWndDropdown, SW_SHOW);
		}
		else
		{
			this->MakeButton(
				nullptr,
				rc.right - DPIScale(42),
				DPIScale(44),
				DPIScale(22),
				DPIScale(22),
				IDM_TOOLBAR_DROPDOWN
			);
		}
		this->iParam->DropdownbuttonIsVisible = TRUE;
	}
	else
	{
		this->iParam->LAST_TBBUTTON_ID = TBBUTTON_END - 1;

		// all buttons are visible -> hide the dropdownbutton (if necessary)
		auto hWndDropdown = GetDlgItem(this->MainWindow, IDM_TOOLBAR_DROPDOWN);
		if (hWndDropdown)
		{
			ShowWindow(hWndDropdown, SW_HIDE);
			this->iParam->DropdownbuttonIsVisible = FALSE;
		}
	}
}

void CnCS_UI::OnRestoreFrames()
{
	RECT rc, rc_statusbar;
	GetClientRect(this->MainWindow, &rc);
	GetWindowRect(this->iParam->statusBar_WND, &rc_statusbar);

	// TreeToolbox Frame >>
	int TVFrameWidth;

	if (this->iParam->WindowSizeUserdefined)
	{
		////////
		if (this->iParam->TVFrameWidth > (rc.right - DPIScale(80)))
		{
			this->iParam->TVFrameWidth = rc.right - DPIScale(80);
		}
		////////
		TVFrameWidth = this->iParam->TVFrameWidth;
	}
	else
		TVFrameWidth = rc.right / 4;

	MoveWindow(
		this->iParam->TVFrame_WND,
		0, DPIScale(25),
		TVFrameWidth,
		rc.bottom - (DPIScale(25) + (rc_statusbar.bottom - rc_statusbar.top)),
		TRUE
	);

	// Edit Frame >>
	int x, cx, cy;

	if (this->iParam->WindowSizeUserdefined)
	{
		x = this->iParam->TVFrameWidth + DPIScale(5);
		cx = rc.right - (this->iParam->TVFrameWidth + DPIScale(5));
		/////////
		if ((this->iParam->EditFrameHeight + DPIScale(80)/* DPIScale(68) ?!? */+ (rc_statusbar.bottom - rc_statusbar.top)) > (rc.bottom - DPIScale(50)))
		{
			this->iParam->EditFrameHeight = rc.bottom - (DPIScale(80)/* DPIScale(68) ?!? */ + (rc_statusbar.bottom - rc_statusbar.top) + DPIScale(50));
		}
		///////////
		cy = this->iParam->EditFrameHeight;
	}
	else
	{
		x = (rc.right / 4) + DPIScale(5);
		cx = rc.right - x;
		cy = rc.bottom - (DPIScale(68) + (rc_statusbar.bottom - rc_statusbar.top) + DPIScale(105));
	}
	MoveWindow(
		this->iParam->EditFrame_WND,
		x,
		DPIScale(68),
		cx,
		cy,
		TRUE
	);


	// Codebox Frame >>
	int y;

	if (this->iParam->WindowSizeUserdefined)
	{
		x = this->iParam->TVFrameWidth + DPIScale(5);
		y = DPIScale(68) + this->iParam->EditFrameHeight + DPIScale(5);
		cx = rc.right - (this->iParam->TVFrameWidth + DPIScale(5));
		cy = rc.bottom - (this->iParam->EditFrameHeight + (rc_statusbar.bottom - rc_statusbar.top) + DPIScale(68 + 5));
	}
	else
	{
		x = (rc.right / 4) + DPIScale(5);
		y = rc.bottom - ((rc_statusbar.bottom - rc_statusbar.top) + DPIScale(100));
		cx = rc.right - x;
		cy = DPIScale(100);
	}
	MoveWindow(this->iParam->CBoxFrame_WND, x, y, cx, cy, TRUE);
}

void CnCS_UI::OnRestoreFloatingWnd()
{
	HWND dataTrafficWnd =
		FindWindow(CNCS_DATAX_CLASS, nullptr);

	if (dataTrafficWnd != nullptr)
	{
		RECT rc;
		GetWindowRect(this->iParam->CBoxFrame_WND, &rc);
		if ((rc.bottom - rc.top) < 250)
			rc.top = rc.top - (250 - (rc.bottom - rc.top));

		MoveWindow(dataTrafficWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	}

	HWND sampleManager = FindWindow(L"SMPMNGR_CLASS\0", nullptr);
	if (sampleManager != nullptr)
	{
		RECT rc;
		GetWindowRect(this->iParam->TVFrame_WND, &rc);

		rc.bottom -= 20;

		MoveWindow(sampleManager, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	}
}

void CnCS_UI::OnMaximized()
{
	this->iParam->DRAW_MODE = DRAW_ALL_AREAS;

	if (this->iParam->WindowIsMaximized)
	{
		RECT rc;

		if (this->Get_MaximizeRect(&rc))
		{
			RECT cur;
			GetWindowRect(this->MainWindow, &cur);

			cur.right -= cur.left;
			cur.bottom -= cur.top;

			if (CompareRect(&cur, &rc))
			{
				this->AdaptEditframeHeight();
				this->OnRestoreWindow(this->MainWindow, (LPARAM)0);
			}
			else
			{
				MoveWindow(this->MainWindow, rc.left, rc.top, rc.right, rc.bottom, TRUE);
			}
		}
	}
}

void CnCS_UI::Global_Tracking(LPMOUSEHOOKSTRUCT pmh, CnCS_UI* pUI)
{
	if (this->iParam->ACTIVE_BUTTON_ID != 0)
	{
		// normalize the highlighted button
		RECT rc_button;

		GetWindowRect(
			pUI->GetButtonHandle(this->iParam->ACTIVE_BUTTON_ID),
			&rc_button
		);

		if ((pmh->pt.x < rc_button.left) || (pmh->pt.x > rc_button.right) || (pmh->pt.y < rc_button.top) || (pmh->pt.y > rc_button.bottom))
		{
			SendMessage(
				pUI->MainWindow,
				WM_NORMALIZE,
				(WPARAM)this->iParam->ACTIVE_BUTTON_ID,
				0
			);
		}
	}

	/////// DROPDOWN >>
	if (pUI->iParam->dropdownbutton_active)
	{
		if (!pUI->iParam->ext_toolbar_exsisting)
		{
			RECT rc_button;
			GetWindowRect(
				pUI->GetButtonHandle(IDM_TOOLBAR_DROPDOWN), &rc_button);

			if ((pmh->pt.x < rc_button.left) || (pmh->pt.x > rc_button.right) || (pmh->pt.y < rc_button.top) || (pmh->pt.y > rc_button.bottom))
			{
				SendMessage(pUI->MainWindow, WM_NORMALIZE, (WPARAM)IDM_TOOLBAR_DROPDOWN, 0);
			}
		}
	}
	if (pUI->iParam->ext_toolbar_exsisting)
	{
		RECT rc_Etoolbar, rc_button;

		GetWindowRect(
			pUI->iParam->extendedToolbar_WND,
			&rc_Etoolbar
		);
		GetWindowRect(
			pUI->GetButtonHandle(IDM_TOOLBAR_DROPDOWN),
			&rc_button
		);

		if (((pmh->pt.x < rc_Etoolbar.left) || (pmh->pt.x > rc_Etoolbar.right) || (pmh->pt.y < rc_Etoolbar.top) || (pmh->pt.y > rc_Etoolbar.bottom)) &&
			((pmh->pt.x < rc_button.left) || (pmh->pt.x > rc_button.right) || (pmh->pt.y < rc_button.top) || (pmh->pt.y > rc_button.bottom)))
		{
			SHORT keystate1, keystate2;
			keystate1 = GetAsyncKeyState(VK_LBUTTON);
			keystate2 = GetAsyncKeyState(VK_RBUTTON);

			if ((keystate1 & 0x8000) || (keystate2 & 0x8000))
			{
				DestroyWindow(pUI->iParam->extendedToolbar_WND);
				pUI->iParam->extendedToolbar_WND = NULL;
				pUI->iParam->ext_toolbar_exsisting = FALSE;
				pUI->iParam->dropdownbutton_active = FALSE;
				SendMessage(pUI->MainWindow, WM_NORMALIZE, (WPARAM)IDM_TOOLBAR_DROPDOWN, 0);
			}
		}
	}
	if (pUI->iParam->SizeAreaActivated)
	{
		SHORT keystate = GetAsyncKeyState(VK_LBUTTON);
		if (keystate & 0x8000)
		{
			RECT rc;
			GetWindowRect(pUI->MainWindow, &rc);

			RECT rc_client;
			POINT pt;
			GetClientRect(pUI->iParam->statusBar_WND, &rc_client);
			GetCursorPos(&pt);

			int width, height;

			if (((pt.x - rc.left) + pUI->iParam->InitialPos_X) < 600)
				width = 600;
			else
				width = ((pt.x - rc.left) + pUI->iParam->InitialPos_X);

			if (((pt.y - rc.top) + pUI->iParam->InitialPos_Y) < 400)
				height = 400;
			else
				height = ((pt.y - rc.top) + pUI->iParam->InitialPos_Y);

			MoveWindow(pUI->MainWindow, rc.left, rc.top, width, height, TRUE);
		}
		else
			SendMessage(pUI->iParam->statusBar_WND, WM_LBUTTONUP, 0, 0);
	}
}

void CnCS_UI::HandleSpecialActions(int ID)
{
	switch (ID)
	{
		case IDM_MAXIMIZE:
		{
			if (this->iParam->WindowIsMaximized)
			{
				this->iParam->WindowIsMaximized = FALSE;
				this->iParam->DRAW_MODE = DRAW_ALL_AREAS;

				ShowWindow(this->MainWindow, SW_RESTORE);
			}
			else
			{
				this->SavePreSizingParameter(nullptr, nullptr);

				this->iParam->WindowIsMaximized = TRUE;
				ShowWindow(this->MainWindow, SW_MAXIMIZE);
			}
		}
		break;
		case IDM_MINIMIZE:
		{
			this->iParam->WindowIsMinimized = TRUE;
			ShowWindow(this->MainWindow, SW_MINIMIZE);
		}
		break;
		case IDM_TOOLBAR_DROPDOWN:
		{
			if (this->iParam->ext_toolbar_exsisting)
			{
				DestroyWindow(this->iParam->extendedToolbar_WND);
				this->iParam->extendedToolbar_WND = NULL;
				this->iParam->ext_toolbar_exsisting = FALSE;
				this->iParam->dropdownbutton_active = FALSE;
				SendMessage(this->MainWindow, WM_NORMALIZE, (WPARAM)IDM_TOOLBAR_DROPDOWN, 0);
			}
			else
				this->CreateExtendedToolbar();
		}
		break;
		default:
			if (this->iParam->ext_toolbar_exsisting)
			{
				if ((ID > this->iParam->LAST_TBBUTTON_ID) && (ID < TBBUTTON_END))
				{
					DestroyWindow(this->iParam->extendedToolbar_WND);
					this->iParam->extendedToolbar_WND = NULL;
					this->iParam->ext_toolbar_exsisting = FALSE;
					this->iParam->ACTIVE_BUTTON_ID = 0;
					SendMessage(this->MainWindow, WM_NORMALIZE, (WPARAM)IDM_TOOLBAR_DROPDOWN, 0);
				}
			}
			break;
	}
}

void CnCS_UI::ChangeStatusbar(int part, LPCTSTR text)
{
	if (part < 3 && part >= 0)
	{
		if(text != nullptr)
			this->statusbarTextHolder[part].Replace(text);
		else
			this->statusbarTextHolder[part].Replace(L"");

		SendMessage(
			this->iParam->statusBar_WND,
			SB_SETTEXT,
			MAKEWPARAM(
				part | SBT_OWNERDRAW,
				0
			),
			reinterpret_cast<LPARAM>(text)
		);
	}
}

void CnCS_UI::DrawAll(HWND hWnd)
{
	HDC hdc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(hWnd, &ps);
	if (hdc)
	{
		HDC hdcMem = CreateCompatibleDC(hdc);
		if (hdcMem)
		{
			RECT rc;
			GetClientRect(this->MainWindow, &rc);

			RECT rcTV;
			GetClientRect(this->iParam->TVFrame_WND, &rcTV);

			//auto tvWidth = this->getTvFrameWidth(&rc);

			HBITMAP hbmOffscreen =
				CreateCompatibleBitmap(
					hdc,
					rc.right,
					rc.bottom
				);
			if (hbmOffscreen)
			{
				HGDIOBJ hbmOld = SelectObject(hdcMem, hbmOffscreen);

				RECT rc_fill;

				SetRect(
					&rc_fill,
					0, 0,
					DPIScale(126),
					DPIScale(24)
				);
				// titlesegment
				FillRect(hdcMem, &rc_fill, this->pObj->styleBrush);

				SetRect(
					&rc_fill,
					DPIScale(126), 0,
					rc.right,
					DPIScale(24)
				);
				// titlebar
				FillRect(hdcMem, &rc_fill, this->pObj->titlebarBrush);

				SetRect(
					&rc_fill,
					(rcTV.right - rcTV.left) + DPIScale(5),
					DPIScale(24),
					rc.right,
					DPIScale(68)
				);
				// toolbar
				FillRect(hdcMem, &rc_fill, this->pObj->mainToolbarBrush);

				SetRect(
					&rc_fill,
					0,
					DPIScale(67),
					rc.right,
					rc.bottom
				);
				// background
				FillRect(hdcMem, &rc_fill, this->pObj->backgroundBrush);

				SetRect(
					&rc_fill,
					0,
					DPIScale(24),
					(rcTV.right - rcTV.left) + DPIScale(5),
					DPIScale(69)
				);
				// background
				FillRect(hdcMem, &rc_fill, this->pObj->backgroundBrush);

				HGDIOBJ originPen = SelectObject(hdcMem, this->pObj->outlinePen);

				MoveToEx(hdcMem, 0, DPIScale(24), nullptr);
				LineTo(hdcMem, rc.right, DPIScale(24));

				MoveToEx(hdcMem, DPIScale(127), 0, nullptr);
				LineTo(hdcMem, DPIScale(127), DPIScale(24));


				SelectObject(hdcMem, this->pObj->separatorPen);

				//MoveToEx(hdcMem, DPIScale(127), DPIScale(2), nullptr);
				//LineTo(hdcMem, DPIScale(127), DPIScale(22));
				MoveToEx(hdcMem, DPIScale(157), DPIScale(2), nullptr);
				LineTo(hdcMem, DPIScale(157), DPIScale(22));
				MoveToEx(hdcMem, DPIScale(187), DPIScale(2), nullptr);
				LineTo(hdcMem, DPIScale(187), DPIScale(22));
				MoveToEx(hdcMem, DPIScale(217), DPIScale(2), nullptr);
				LineTo(hdcMem, DPIScale(217), DPIScale(22));

				// app sys icon
				DrawIconEx(hdcMem, DPIScale(2), 0, this->pObj->appSysIcon, DPIScale(24), DPIScale(24), 0, nullptr, DI_NORMAL);

				// draw size-cross while sizing
				if (this->iParam->SizeAreaActivated)
				{
					DrawIconEx(hdcMem, (rc.right / 2) - 64, (rc.bottom / 2) - 64, this->pObj->size_cross, 128, 128, 0, nullptr, DI_NORMAL);
				}

				// app name
				SIZE sz;
				HGDIOBJ originFont = SelectObject(hdcMem, this->pObj->titlefont);
				SetBkMode(hdcMem, TRANSPARENT);
				SetTextColor(hdcMem, this->styleInfo.TextColor);

				int tLen = _lengthOfString(APPLICATION_DISPLAY_NAME);
				if (tLen > 0)
				{
					GetTextExtentPoint32(
						hdcMem,
						APPLICATION_DISPLAY_NAME,
						tLen,
						&sz
					);
					TextOut(
						hdcMem,
						DPIScale(30),
						DPIScale(13) - (sz.cy / 2),
						APPLICATION_DISPLAY_NAME,
						tLen
					);
				}
				SelectObject(hdcMem, originFont);

				BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

				SelectObject(hdcMem, originPen);
				SelectObject(hdcMem, hbmOld);
				DeleteObject(hbmOffscreen);
			}
			DeleteDC(hdcMem);
		}
		EndPaint(hWnd, &ps);
	}
}

void CnCS_UI::DrawNeccessaryAreas(HWND hWnd)
{
	UNREFERENCED_PARAMETER(hWnd);
	//PAINTSTRUCT ps;
	//SIZE sz;
	//HDC hdc, hdc_offscreen;
	//HGDIOBJ original_offscreen, original;
	//RECT rc, rc_fill, rc_status;

	//GetClientRect(hWnd, &rc);
	//GetWindowRect(this->iParam->statusBar_WND, &rc_status);

	//SetRect(&rc_fill, 0, DPIScale(80), rc.right, rc.bottom);//SetRect(&rc_fill, 0, 80, rc.right, rc.bottom);

	//hdc = BeginPaint(hWnd, &ps);
	//hdc_offscreen = CreateCompatibleDC(hdc);

	//FillRect(hdc, &rc_fill, this->pObj->backgroundBrush);

	//original_offscreen = SelectObject(hdc_offscreen, this->pObj->upperright);
	//BitBlt(hdc, rc.right - DPIScale(134), 0, DPIScale(134), DPIScale(80), hdc_offscreen, 0, 0, SRCCOPY);//BitBlt(hdc, rc.right - 126, 0, 126, 80, hdc_offscreen, 0, 0, SRCCOPY);

	//SelectObject(hdc_offscreen, this->pObj->titlebar);
	//StretchBlt(hdc, DPIScale(191), 0, rc.right - DPIScale(191 + 134), DPIScale(80), hdc_offscreen, 0, 0, DPIScale(314), DPIScale(80), SRCCOPY);

	//if (this->iParam->SizeAreaActivated)
	//{
	//	SelectObject(hdc_offscreen, this->pObj->size_cross);
	//	BitBlt(hdc, (rc.right / 2) - 64, (rc.bottom / 2) - 64, 128, 128, hdc_offscreen, 0, 0, SRCCOPY);
	//}
	//original = SelectObject(hdc, this->pObj->titlefont);
	//SetBkMode(hdc, TRANSPARENT);
	//SetTextColor(hdc, RGB(255, 255, 255));

	//original = SelectObject(hdc, this->pObj->titlefont);
	//SetBkMode(hdc, TRANSPARENT);
	//SetTextColor(hdc, RGB(255, 255, 255));

	//int tLen = _lengthOfString(APPLICATION_DISPLAY_NAME);
	//if (tLen > 0)
	//{
	//	GetTextExtentPoint32(
	//		hdc,
	//		APPLICATION_DISPLAY_NAME,
	//		tLen,
	//		&sz
	//	);
	//	TextOut(
	//		hdc,
	//		(rc.right / 2) - (sz.cx / 2)
	//		,
	//		2,
	//		APPLICATION_DISPLAY_NAME,
	//		tLen
	//	);
	//}
	//SelectObject(hdc, original);
	//SelectObject(hdc, original_offscreen);
	//DeleteDC(hdc_offscreen);

	//EndPaint(hWnd, &ps);
}

void CnCS_UI::WindowChanged(HWND hWnd)
{
	if (!this->iParam->WindowIsMinimized)
	{
		if (!this->iParam->SizeChanged)
		{
			if (!this->iParam->TimerActive)
			{
				this->iParam->TimerActive = TRUE;			
				SetTimer(hWnd, ID_TIMER_SAVEFRAMEDATA, 2000, (TIMERPROC)NULL);
			}
			this->iParam->SizeChanged = TRUE;
		}
	}
}

void CnCS_UI::SetNewFrameAlignment()
{
	RECT rc_tv, rc_ed;
	GetWindowRect(this->iParam->TVFrame_WND, &rc_tv);
	GetWindowRect(this->iParam->EditFrame_WND, &rc_ed);

	if (this->iParam->FrameSizeAreaActivated == WE_SIZEAREA_SIZING)
	{
		this->iParam->EditFrameHeight = rc_ed.bottom - rc_ed.top;
		this->iParam->TVFrameWidth = this->iParam->rc_multiU.left - rc_tv.left;
	}
	else if (this->iParam->FrameSizeAreaActivated == NS_SIZEAREA_SIZING)
	{
		this->iParam->EditFrameHeight = this->iParam->rc_multiU.top - rc_ed.top;
		this->iParam->TVFrameWidth = rc_tv.right - rc_tv.left;
	}
	this->iParam->WindowSizeUserdefined = TRUE;

	RECT rc;
	GetClientRect(this->MainWindow, &rc);

	SendMessage(this->MainWindow, WM_SIZE,(WPARAM)SIZE_RESTORED ,MAKELPARAM(rc.right,rc.bottom));
}

void CnCS_UI::dpiChanged()
{
	this->_createDpiDependedResources(true);
	this->iParam->DRAW_MODE = DRAW_ALL_AREAS;
}

BOOL CnCS_UI::MakeButton(HWND Parent_Special, int x, int y, int cx, int cy, DWORD ID)
{
	if (Parent_Special == nullptr)
		Parent_Special = this->MainWindow;

	HWND button =
		CreateWindow(
			L"BUTTON",
			nullptr,
			WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_CLIPSIBLINGS,
			x, y, cx, cy,
			Parent_Special,
			IDTOHMENU(ID),//reinterpret_cast<HMENU>(ID),
			this->hInstance,
			nullptr
		);

	if (button)
	{
		if (!SetWindowSubclass(button, CnCS_UI::ButtonSub, NULL, reinterpret_cast<DWORD_PTR>(this)))
			return FALSE;
		else
		{
			return this->MakeTooltip(button, ID);
		}
	}
	else
		return FALSE;
}

BOOL CnCS_UI::MakeTooltip(HWND button, int ID)
{
	HRESULT hr;
	TCHAR tooltiptext[256] = { 0 };

	switch (ID)
	{
	case IDM_NEW:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_ADDTAB));
		break;
	case IDM_OPEN:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_OPEN));
		break;
	case IDM_SAVE:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_SAVE));
		break;
	case IDM_SAVEAS:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_SAVEAS));
		break;
	case IDM_SAVEALL:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_SAVEALL));
		break;
	case IDM_IMPORT:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_IMPORT));
		break;
	case IDM_CUT:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_CUT));
		break;
	case IDM_PASTE:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_PASTE));
		break;
	case IDM_EXPORT:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_TEMPLATEMANAGER));
		break;
	case IDM_SEND:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_SENDPROGRAM));
		break;
	case IDM_RECEIVE:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_RECEIVEPROGRAM));
		break;
	case IDM_COPY:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_COPY));
		break;
	case IDM_EDITDELETE:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_DELETE_SELECTEDTEXT));
		break;
	case IDM_APPSETTINGS:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_APPLICATIONPROPERTIES));
		break;
	case IDM_SELECTALL:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_SELECTALL));
		break;
	case IDM_HELP:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_OPENHELP));
		break;
	case IDM_WEB:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_VISITWEBSITE));
		break;
	case IDM_HISTORY:
		hr = StringCbCopy(tooltiptext, sizeof(tooltiptext), getStringFromResource(UI_GNRL_HISTORY));
		break;

		

	default:
		return TRUE;
	}
	if (SUCCEEDED(hr))
		return AddToolipToControl(button, this->MainWindow, tooltiptext, this->hInstance, NULL);
	else
		return FALSE;
}

BOOL CnCS_UI::DrawButtonWithIcon(HWND button, LPDRAWITEMSTRUCT pdi, HBRUSH bkgnd, HICON icon, int cx, int cy)
{
	BOOL cleanUpRequired = FALSE;
	BOOL result = TRUE;

	if (!pdi)
	{
		if (!button)
			return FALSE;
		else
		{
			pdi = new DRAWITEMSTRUCT;
			if (!pdi)
				return FALSE;
			else
			{
				SecureZeroMemory(pdi, sizeof(DRAWITEMSTRUCT));

				if (!GetClientRect(button, &pdi->rcItem))
					return FALSE;
				else
				{
					pdi->hDC = GetDC(button);
					if (!pdi->hDC)
						return FALSE;
					else
					{
						cleanUpRequired = TRUE;
					}
				}
			}
		}
	}
	if (bkgnd)
	{
		FillRect(pdi->hDC, &pdi->rcItem, bkgnd);
	}
	if (!DrawIconEx(
		pdi->hDC,
		(pdi->rcItem.right - cx) / 2,
		(pdi->rcItem.bottom - cy) / 2,
		icon,
		cx,
		cy,
		0,
		NULL,
		DI_NORMAL))
		result = FALSE;

	if (cleanUpRequired)
	{
		ReleaseDC(button, pdi->hDC);
		delete pdi;
	}
	return result;
}

BOOL CnCS_UI::DrawButtonWithBitmap(HWND button, LPDRAWITEMSTRUCT pdi, HBITMAP bitmap, int cx, int cy)
{
	BOOL cleanUpRequired = FALSE;
	BOOL result = TRUE;

	if (!pdi)
	{
		if (!button)
			return FALSE;
		else
		{
			pdi = new DRAWITEMSTRUCT;
			if (!pdi)
				return FALSE;
			else
			{
				SecureZeroMemory(pdi, sizeof(DRAWITEMSTRUCT));

				if (!GetClientRect(button, &pdi->rcItem))
					return FALSE;
				else
				{
					pdi->hDC = GetDC(button);
					if (!pdi->hDC)
						return FALSE;
					else
					{
						cleanUpRequired = TRUE;
					}
				}
			}
		}
	}
	HDC hdc_mem = CreateCompatibleDC(pdi->hDC);
	if (hdc_mem)
	{
		HGDIOBJ original = SelectObject(hdc_mem, bitmap);

		result=BitBlt(pdi->hDC, 0, 0, cx, cy, hdc_mem, 0, 0, SRCCOPY);

		SelectObject(hdc_mem, original);
		DeleteDC(hdc_mem);
	}
	if (cleanUpRequired)
	{
		ReleaseDC(button, pdi->hDC);
		delete pdi;
	}
	return result;

}

BOOL CnCS_UI::DrawButtonFromID(HWND button, LPDRAWITEMSTRUCT pdi, int ID, int Status)
{
	if (!this->SwitchButtonActivationStatus(ID, Status))
		return FALSE;

	BUTTONDRAWINFO bdi;

	if (this->GetButtonDrawInfo(ID, Status, &bdi))
	{
		if (bdi.bitmap == NULL)
		{
			return this->DrawButtonWithIcon(button, pdi, bdi.bkgnd, bdi.icon, bdi.cx, bdi.cy);
		}
		else
		{
			return this->DrawButtonWithBitmap(button, pdi, bdi.bitmap, bdi.cx, bdi.cy);
		}
	}
	else
		return FALSE;
}

BOOL CnCS_UI::GetButtonDrawInfo(int ID, int Status, LPBUTTONDRAWINFO pbdi)
{
	if (!pbdi)
		return FALSE;

	auto ico = !((this->styleInfo.StyleID == STYLEID_BLACK) || (this->styleInfo.StyleID == STYLEID_LIGHTGRAY) || (this->styleInfo.StyleID == STYLEID_GREEN));
	ico = true;

	switch (ID)
	{
	case IDM_APPSETTINGS:
		pbdi->cx = DPIScale(24);
		pbdi->cy = DPIScale(24);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->titlebarBrush;

		if (Status == BDRAW_NORMAL)		
			pbdi->icon = this->pObj->appSettingsIcon_norm;
		else if (Status == BDRAW_MARKED)		
			pbdi->icon = this->pObj->appSettingsIcon_marked;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->appSettingsIcon_pressed;
		break;
	case IDM_CLOSE:
		pbdi->cx = DPIScale(24);
		pbdi->cy = DPIScale(24);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->titlebarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = this->pObj->close_norm;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = this->pObj->close_mark;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->close_pressed;
		break;

	case IDM_MAXIMIZE:
		pbdi->cx = DPIScale(24);
		pbdi->cy = DPIScale(24);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->titlebarBrush;

		if(this->iParam->WindowIsMaximized)
		{ 
			if (Status == BDRAW_NORMAL)
				pbdi->icon = this->pObj->maximize_norm_min;
			else if (Status == BDRAW_MARKED)
				pbdi->icon = this->pObj->maximize_marked_min;
			else if (Status == BDRAW_PRESSED)
				pbdi->icon = this->pObj->maximize_pressed_min;
		}
		else
		{
			if (Status == BDRAW_NORMAL)
				pbdi->icon = this->pObj->maximize_norm;
			else if (Status == BDRAW_MARKED)
				pbdi->icon = this->pObj->maximize_marked;
			else if (Status == BDRAW_PRESSED)
				pbdi->icon = this->pObj->maximize_pressed;
		}
		break;
	case IDM_MINIMIZE:
		pbdi->cx = DPIScale(24);
		pbdi->cy = DPIScale(24);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->titlebarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = this->pObj->minimize_norm;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = this->pObj->minimize_marked;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->minimize_pressed;
			break;
	case IDM_NEW:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ? this->pObj->new_norm : this->pObj->new_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->new_mark : this->pObj->new_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->new_pressed;
		break;
	case IDM_OPEN:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ? this->pObj->open_norm : this->pObj->open_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->open_mark : this->pObj->open_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->open_pressed;
		break;
	case IDM_SAVE:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ?this->pObj->save_norm : this->pObj->save_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->save_mark : this->pObj->save_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->save_pressed;
		break;
	case IDM_SAVEALL:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ? this->pObj->saveall_norm : this->pObj->saveall_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->saveall_mark : this->pObj->saveall_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->saveall_pressed;
		break;
	case IDM_SAVEAS:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ? this->pObj->saveas_norm : this->pObj->saveas_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->saveas_mark : this->pObj->saveas_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->saveas_pressed;
		break;
	case IDM_COPY:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ? this->pObj->copy_norm : this->pObj->copy_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->copy_mark : this->pObj->copy_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->copy_pressed;
		break;
	case IDM_TOOLBAR_DROPDOWN:
		pbdi->cx = DPIScale(22);
		pbdi->cy = DPIScale(22);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = this->pObj->dropdown_norm;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = this->pObj->dropdown_mark;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->dropdown_pressed;
		break;
	case IDM_PASTE:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ? this->pObj->paste_norm : this->pObj->paste_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->paste_mark : this->pObj->paste_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->paste_pressed;
		break;
	case IDM_CUT:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ? this->pObj->cut_norm : this->pObj->cut_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->cut_mark : this->pObj->cut_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->cut_pressed;
		break;
	case IDM_SELECTALL:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ? this->pObj->selectall_norm : this->pObj->selectall_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->selectall_mark : this->pObj->selectall_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->selectall_pressed;
		break;
	case IDM_EDITDELETE:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ? this->pObj->editdelete_norm : this->pObj->editdelete_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->editdelete_mark : this->pObj->editdelete_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->editdelete_pressed;
		break;
	case IDM_EXPORT:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ? this->pObj->export_norm : this->pObj->export_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->export_mark : this->pObj->export_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->export_pressed;
		break;
	case IDM_IMPORT:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ? this->pObj->import_norm : this->pObj->import_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->import_mark : this->pObj->import_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->import_pressed;
		break;
	case IDM_SEND:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ? this->pObj->send_norm : this->pObj->send_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->send_mark : this->pObj->send_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->send_pressed;
		break;
	case IDM_RECEIVE:
		pbdi->cx = DPIScale(42);
		pbdi->cy = DPIScale(42);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->mainToolbarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = ico ? this->pObj->receive_norm : this->pObj->receive_mark;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = ico ? this->pObj->receive_mark : this->pObj->receive_norm;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->receive_pressed;
		break;
	case IDM_HELP:
		pbdi->cx = DPIScale(24);
		pbdi->cy = DPIScale(24);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->titlebarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = this->pObj->help_norm;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = this->pObj->help_mark;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->help_pressed;
		break;
	case IDM_WEB:
		pbdi->cx = DPIScale(24);
		pbdi->cy = DPIScale(24);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->titlebarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = this->pObj->web_norm;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = this->pObj->web_mark;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->web_pressed;
		break;
	case IDM_HISTORY:
		pbdi->cx = DPIScale(24);
		pbdi->cy = DPIScale(24);
		pbdi->bitmap = NULL;
		pbdi->bkgnd = this->pObj->titlebarBrush;

		if (Status == BDRAW_NORMAL)
			pbdi->icon = this->pObj->history_norm;
		else if (Status == BDRAW_MARKED)
			pbdi->icon = this->pObj->history_mark;
		else if (Status == BDRAW_PRESSED)
			pbdi->icon = this->pObj->history_pressed;
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

BOOL CnCS_UI::DrawStatusbar(LPDRAWITEMSTRUCT pdi)
{
	if (pdi != nullptr)
	{
		FillRect(pdi->hDC, &pdi->rcItem, this->pObj->titlebarBrush);

		SIZE sz;
		HGDIOBJ original;
		int part = pdi->itemID;
		
		if (part >= 0 && part < 3)
		{
			int c = this->statusbarTextHolder[part].GetLength();
			if (c > 0)
			{
				SetTextColor(pdi->hDC, this->styleInfo.TextColor);
				SetBkMode(pdi->hDC, TRANSPARENT);

				original = SelectObject(pdi->hDC, this->pObj->statusbarfont);

				GetTextExtentPoint32(pdi->hDC, this->statusbarTextHolder[part].GetData(), c, &sz);

				TextOut(pdi->hDC, pdi->rcItem.left + 10, ((pdi->rcItem.bottom / 2) + 1) - (sz.cy / 2), this->statusbarTextHolder[part].GetData(), c);

				SelectObject(pdi->hDC, original);

				if (pdi->itemID == 2)
				{
					if (this->iParam->WindowIsMaximized)
					{
						DrawIconEx(pdi->hDC, pdi->rcItem.right - 22, pdi->rcItem.bottom - 22, this->pObj->size_arrow_disabled, 22, 22, 0, NULL, DI_NORMAL);
					}
					else
					{
						DrawIconEx(pdi->hDC, pdi->rcItem.right - 22, pdi->rcItem.bottom - 22, this->pObj->size_arrow, 22, 22, 0, NULL, DI_NORMAL);
					}
				}
			}
		}
	}
	return TRUE;
}

BOOL CnCS_UI::SwitchButtonActivationStatus(int ID, int Status)
{
	if (Status == BDRAW_MARKED)
	{
		switch (ID)
		{
		case IDM_TOOLBAR_DROPDOWN:
			if (this->iParam->dropdownbutton_active||this->iParam->ext_toolbar_exsisting)
				return FALSE;
			else
				this->iParam->dropdownbutton_active = TRUE;
			break;
		default:
			if (ID == this->iParam->ACTIVE_BUTTON_ID)
			{
				return FALSE;
			}
			else
			{
				this->iParam->ACTIVE_BUTTON_ID = ID;
				return TRUE;
			}
		}
	}
	else if ((Status == BDRAW_NORMAL)|| (Status == BDRAW_PRESSED))
	{
		switch (ID)
		{
		case IDM_TOOLBAR_DROPDOWN:
			if (this->iParam->ext_toolbar_exsisting)
				return FALSE;
			this->iParam->dropdownbutton_active = FALSE;
			break;
		default:
			this->iParam->ACTIVE_BUTTON_ID = 0;
			return TRUE;
		}
	}
	return TRUE;
}

int CnCS_UI::WhatIsItem(int ID)
{
	if ((ID >= IDM_APPSETTINGS) && (ID < ID_LASTBUTTON))
		return IS_BUTTON_ITEM;
	else if (ID == ID_STATUSBAR)
		return IS_STATUSBAR_ITEM;
	else
		return 0;
}

void CnCS_UI::SavePreSizingParameter(LPRECT main, LPRECT editframe)
{
	if (main != nullptr)
	{
		this->iParam->sizingStartHeight_Main = main->bottom;
	}
	else
	{
		RECT mainWndRC;
		GetClientRect(this->MainWindow, &mainWndRC);
		this->iParam->sizingStartHeight_Main = mainWndRC.bottom;
	}

	if (editframe != nullptr)
	{
		this->iParam->sizingStartHeight_Editframe = editframe->bottom;
	}
	else
	{
		RECT editframeWnd;
		GetClientRect(this->iParam->EditFrame_WND, &editframeWnd);
		this->iParam->sizingStartHeight_Editframe = editframeWnd.bottom;
	}
}

void CnCS_UI::AdaptEditframeHeight()
{
	if ((this->iParam->sizingStartHeight_Editframe > -1)
		&& (this->iParam->sizingStartHeight_Main > -1))
	{
		RECT mainRC, statusBarRC;
		GetClientRect(this->MainWindow, &mainRC);
		GetWindowRect(this->iParam->statusBar_WND, &statusBarRC);

		if (mainRC.bottom > this->iParam->sizingStartHeight_Main)
		{
			auto heightPercentageCoefficient =
				((double)(((double)this->iParam->sizingStartHeight_Editframe) / ((double)this->iParam->sizingStartHeight_Main)));

			if (heightPercentageCoefficient > 0)
			{
				this->iParam->EditFrameHeight = (int)(((double)mainRC.bottom) * heightPercentageCoefficient);

				if ((this->iParam->EditFrameHeight + DPIScale(80)/* DPIScale(68) ?!? */ + (statusBarRC.bottom - statusBarRC.top)) > (mainRC.bottom - DPIScale(50)))
				{
					this->iParam->EditFrameHeight = mainRC.bottom - (DPIScale(80)/* DPIScale(68) ?!? */ + (statusBarRC.bottom - statusBarRC.top) + DPIScale(50));
				}

			}
		}
		this->iParam->sizingStartHeight_Editframe = -1;
		this->iParam->sizingStartHeight_Main = -1;
	}
}

HWND CnCS_UI::GetButtonHandle(int ID)
{
	HWND button = NULL;

	if (((ID > this->iParam->LAST_TBBUTTON_ID) && (ID < TBBUTTON_END)) && this->iParam->ext_toolbar_exsisting)
	{
		button = GetDlgItem(this->iParam->extendedToolbar_WND, ID);
	}
	else
	{
		button = GetDlgItem(this->MainWindow, ID);
		if (!button)
		{
			if (this->iParam->ext_toolbar_exsisting)
			{
				button = GetDlgItem(this->iParam->extendedToolbar_WND, ID);
			}
			else
			{
				button = NULL;
				//GET OTHER HANDLES HERE !
			}
		}
	}
	return button;
}

HWND CnCS_UI::Get_Frame_Handles(int RequestedFrame)
{
	switch (RequestedFrame)
	{
	case GFWH_TVFRAME:
		return this->iParam->TVFrame_WND;
	case GFWH_EDITFRAME:
		return this->iParam->EditFrame_WND;
	case GFWH_CBOXFRAME:
		return this->iParam->CBoxFrame_WND;
	default:
		return NULL;
	}
}

BOOL CnCS_UI::Get_MaximizeRect(LPRECT rc_out)
{
	HWND hwnd_tray = FindWindow(L"Shell_TrayWnd", NULL);
	if (hwnd_tray)
	{
		RECT rc_tray;
		GetWindowRect(hwnd_tray, &rc_tray);

		int cx = GetSystemMetrics(SM_CXSCREEN);
		int cy = GetSystemMetrics(SM_CYSCREEN);

		this->iParam->DRAW_MODE = DRAW_ALL_AREAS;

		if ((rc_tray.left == 0) && (rc_tray.top == 0) && (rc_tray.bottom == cy))// taskbar left aligned
		{
			rc_out->left = rc_tray.right;
			rc_out->top = 0;
			rc_out->right = cx - rc_tray.right;
			rc_out->bottom = cy;
		}
		else if ((rc_tray.bottom == cy) && (rc_tray.left == 0) && (rc_tray.right == cx))// bottom aligned
		{
			rc_out->left = 0;
			rc_out->top = 0;
			rc_out->right = cx;
			rc_out->bottom = cy - (rc_tray.bottom - rc_tray.top);
		}
		else if ((rc_tray.right == cx) && (rc_tray.top == 0) && (rc_tray.bottom == cy))// right aligned
		{
			rc_out->left = 0;
			rc_out->top = 0;
			rc_out->right = cx - (rc_tray.right - rc_tray.left);
			rc_out->bottom = cy;
		}
		else if ((rc_tray.left == 0) && (rc_tray.top == 0) && (rc_tray.right == cx))// top aligned
		{
			rc_out->left = 0;
			rc_out->top = rc_tray.bottom;
			rc_out->right = cx;
			rc_out->bottom = cy - rc_tray.bottom;
		}
		else// not visible... (hidden mode)
		{
			rc_out->left = 0;
			rc_out->top = 0;
			rc_out->bottom = cy;
			rc_out->right = cx;
		}
	}
	else
		return FALSE;

	return TRUE;
}

//void CnCS_UI::SetBitmapIDs(LPBMPID IDs, int StyleID)
//{
//	int dpiIndex = 0;
//
//	auto dpiAssist = new DPI_Assist();
//	if (dpiAssist != nullptr)
//	{
//		dpiIndex = dpiAssist->GetDpiIndex();
//		SafeRelease(&dpiAssist);
//	}
//
//	if (StyleID == STYLEID_BLACK)
//	{
//		//IDs->ul = IDB_BKGND_UPPERLEFT_BLACK_100 + dpiIndex;
//		//IDs->ur = IDB_BKGND_UPPERRIGHT_BLACK_100 + dpiIndex;
//		//IDs->tb = IDB_BKGND_MID_BLACK_100 + dpiIndex;
//	}
//	else if (StyleID == STYLEID_DARKGRAY)
//	{
//		//IDs->ul = IDB_BKGND_UPPERLEFT_DARKGREY_100 + dpiIndex;
//		//IDs->ur = IDB_BKGND_UPPERRIGHT_DARKGREY_100 +dpiIndex;
//		//IDs->tb = IDB_BKGND_MID_DARKGREY_100 + dpiIndex;
//	}
//	else if (StyleID == STYLEID_GREEN)
//	{
//		//IDs->ul = IDB_BKGND_UPPERLEFT_GREEN_100 + dpiIndex;
//		//IDs->ur = IDB_BKGND_UPPERRIGHT_GREEN_100 + dpiIndex;
//		//IDs->tb = IDB_BKGND_MID_GREEN_100 + dpiIndex;
//	}
//	else if (StyleID == STYLEID_LIGHTGRAY)
//	{
//		//IDs->ul = IDB_BKGND_UPPERLEFT_LIGHTGREY_100 + dpiIndex;
//		//IDs->ur = IDB_BKGND_UPPERRIGHT_LIGHTGREY_100 + dpiIndex;
//		//IDs->tb = IDB_BKGND_MID_LIGHTGREY_100 + dpiIndex;
//	}
//	else
//	{
//		//IDs->ul = IDB_BKGND_UPPERLEFT_LIGHTGREY_100 + dpiIndex;
//		//IDs->ur = IDB_BKGND_UPPERRIGHT_LIGHTGREY_100 + dpiIndex;
//		//IDs->tb = IDB_BKGND_MID_LIGHTGREY_100 + dpiIndex;
//	}
//}

void CnCS_UI::ControlFloatingWndShowState(int state)
{
	HWND dataTrafficWnd = FindWindow(CNCS_DATAX_CLASS, nullptr);
	if (dataTrafficWnd != nullptr)
	{
		ShowWindow(dataTrafficWnd, state);
	}
	HWND sampleManager = FindWindow(L"SMPMNGR_CLASS\0", nullptr);
	if (sampleManager != nullptr)
	{
		ShowWindow(sampleManager, state);
	}
}

void CnCS_UI::setParameterFromExtern(int pID, LONG_PTR val)
{
	switch (pID)
	{
	case PARAID_UI_SETTINGSBUTTON_TOUCHDOWNLOCK:
		this->iParam->blockSettingsButtonNormalization = (BOOL)val;
		break;
	default:
		break;
	}
}

void CnCS_UI::_createDpiDependedResources(bool cleanOld)
{
	int dpiIndex = 0;
	auto dpiAssist = reinterpret_cast<DPI_Assist*>(getDPIAssist());
	if (dpiAssist != nullptr)
	{
		dpiIndex = dpiAssist->GetDpiIndex();
	}

	// fonts
	if (this->pObj->titlefont != nullptr)
		DeleteObject(this->pObj->titlefont);
	
	this->pObj->titlefont = CreateScaledFont(21, FW_SEMIBOLD, APPLICATION_PRIMARY_FONT);

	if (this->pObj->statusbarfont != nullptr)
		DeleteObject(this->pObj->statusbarfont);

	this->pObj->statusbarfont = CreateScaledFont(20, FW_MEDIUM, APPLICATION_PRIMARY_FONT);

	if (cleanOld)
	{
		DestroyIcon(this->pObj->new_norm);
		DestroyIcon(this->pObj->new_mark);
		DestroyIcon(this->pObj->new_pressed);
		DestroyIcon(this->pObj->open_norm);
		DestroyIcon(this->pObj->open_mark);
		DestroyIcon(this->pObj->open_pressed);
		DestroyIcon(this->pObj->save_norm);
		DestroyIcon(this->pObj->save_mark);
		DestroyIcon(this->pObj->save_pressed);
		DestroyIcon(this->pObj->saveall_norm);
		DestroyIcon(this->pObj->saveall_mark);
		DestroyIcon(this->pObj->saveall_pressed);
		DestroyIcon(this->pObj->saveas_norm);
		DestroyIcon(this->pObj->saveas_mark);
		DestroyIcon(this->pObj->saveas_pressed);
		DestroyIcon(this->pObj->copy_norm);
		DestroyIcon(this->pObj->copy_mark);
		DestroyIcon(this->pObj->copy_pressed);
		DestroyIcon(this->pObj->paste_norm);
		DestroyIcon(this->pObj->paste_mark);
		DestroyIcon(this->pObj->paste_pressed);
		DestroyIcon(this->pObj->cut_norm);
		DestroyIcon(this->pObj->cut_mark);
		DestroyIcon(this->pObj->cut_pressed);
		DestroyIcon(this->pObj->selectall_norm);
		DestroyIcon(this->pObj->selectall_mark);
		DestroyIcon(this->pObj->selectall_pressed);
		DestroyIcon(this->pObj->editdelete_norm);
		DestroyIcon(this->pObj->editdelete_mark);
		DestroyIcon(this->pObj->editdelete_pressed);
		DestroyIcon(this->pObj->export_norm);
		DestroyIcon(this->pObj->export_mark);
		DestroyIcon(this->pObj->export_pressed);
		DestroyIcon(this->pObj->import_norm);
		DestroyIcon(this->pObj->import_mark);
		DestroyIcon(this->pObj->import_pressed);
		DestroyIcon(this->pObj->send_norm);
		DestroyIcon(this->pObj->send_mark);
		DestroyIcon(this->pObj->send_pressed);
		DestroyIcon(this->pObj->receive_norm);
		DestroyIcon(this->pObj->receive_mark);
		DestroyIcon(this->pObj->receive_pressed);
		DestroyIcon(this->pObj->maximize_norm);
		DestroyIcon(this->pObj->maximize_marked);
		DestroyIcon(this->pObj->maximize_pressed);
		DestroyIcon(this->pObj->maximize_norm_min);
		DestroyIcon(this->pObj->maximize_marked_min);
		DestroyIcon(this->pObj->maximize_pressed_min);
		DestroyIcon(this->pObj->minimize_norm);
		DestroyIcon(this->pObj->minimize_marked);
		DestroyIcon(this->pObj->minimize_pressed);
		DestroyIcon(this->pObj->close_norm);
		DestroyIcon(this->pObj->close_mark);
		DestroyIcon(this->pObj->close_pressed);
		DestroyIcon(this->pObj->help_norm);
		DestroyIcon(this->pObj->help_mark);
		DestroyIcon(this->pObj->help_pressed);
		DestroyIcon(this->pObj->appSysIcon);
		DestroyIcon(this->pObj->appSettingsIcon_norm);
		DestroyIcon(this->pObj->appSettingsIcon_marked);
		DestroyIcon(this->pObj->appSettingsIcon_pressed);
		DestroyIcon(this->pObj->web_norm);
		DestroyIcon(this->pObj->web_mark);
		DestroyIcon(this->pObj->web_pressed);
		DestroyIcon(this->pObj->history_norm);
		DestroyIcon(this->pObj->history_mark);
		DestroyIcon(this->pObj->history_pressed);
		DestroyIcon(this->pObj->dropdown_norm);
		DestroyIcon(this->pObj->dropdown_mark);
		DestroyIcon(this->pObj->dropdown_pressed);
	}


	this->pObj->new_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_NEW), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->new_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_NEW_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->new_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_NEW_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->open_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_OPEN), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->open_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_OPEN_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->open_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_OPEN_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->save_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SAVE), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->save_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SAVE_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->save_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SAVE_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->saveall_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SAVEALL), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->saveall_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SAVEALL_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->saveall_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SAVEALL_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->saveas_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SAVEAS), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->saveas_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SAVEAS_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->saveas_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SAVEAS_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->copy_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_COPY), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->copy_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_COPY_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->copy_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_COPY_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->paste_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_PASTE), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->paste_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_PASTE_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->paste_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_PASTE_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->cut_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_CUT), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->cut_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_CUT_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->cut_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_CUT_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->selectall_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SELECTALL), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->selectall_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SELECTALL_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->selectall_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SELECTALL_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->editdelete_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_DELETE), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->editdelete_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_DELETE_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->editdelete_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_DELETE_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->export_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_EXPORT), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->export_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_EXPORT_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->export_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_EXPORT_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->import_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_IMPORT), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->import_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_IMPORT_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->import_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_IMPORT_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->send_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SEND), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->send_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SEND_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->send_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_SEND_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->receive_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_RECEIVE), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->receive_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_RECEIVE_MARKED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);
	this->pObj->receive_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_RECEIVE_PRESSED), IMAGE_ICON, DPIScale(42), DPIScale(42), LR_DEFAULTCOLOR);

	this->pObj->dropdown_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_DROPDOWN), IMAGE_ICON, DPIScale(22), DPIScale(22), LR_DEFAULTCOLOR);
	this->pObj->dropdown_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_DROPDOWN_MARKED), IMAGE_ICON, DPIScale(22), DPIScale(22), LR_DEFAULTCOLOR);
	this->pObj->dropdown_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_TOOLBAR_DROPDOWN_PRESSED), IMAGE_ICON, DPIScale(22), DPIScale(22), LR_DEFAULTCOLOR);


	this->pObj->help_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_HELPMARK), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->help_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_HELPMARK_MARKED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->help_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_HELPMARK_PRESSED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);


	this->pObj->maximize_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_MAXIMIZE), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->maximize_marked = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_MAXIMIZE_MARKED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->maximize_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_MAXIMIZE_PRESSED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->maximize_norm_min = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_MAXIMIZE_MIN), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->maximize_marked_min = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_MAXIMIZE_MARKED_MIN), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->maximize_pressed_min = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_MAXIMIZE_PRESSED_MIN), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->minimize_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_MINIMIZE), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->minimize_marked = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_MINIMIZE_MARKED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->minimize_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_MINIMIZE_PRESSED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->close_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_CLOSEBUTTON), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->close_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_CLOSEBUTTON_MARKED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->close_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_CLOSEBUTTON_PRESSED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->appSysIcon = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_APPSYSICON), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->appSettingsIcon_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_APPSETTINGS), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->appSettingsIcon_marked = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_APPSETTINGS_MARKED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->appSettingsIcon_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_APPSETTINGS_PRESSED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);

	this->pObj->history_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_HISTORY), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->history_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_HISTORY_MARKED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->history_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_HISTORY_PRESSED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);

	this->pObj->web_norm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_WEB), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->web_mark = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_WEB_MARKED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
	this->pObj->web_pressed = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_WEB_PRESSED), IMAGE_ICON, DPIScale(24), DPIScale(24), LR_DEFAULTCOLOR);
}