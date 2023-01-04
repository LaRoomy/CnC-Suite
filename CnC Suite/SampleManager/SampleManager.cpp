#include"SampleManager.h"
#include"..//HelperF.h"
#include"..//Error dispatcher.h"
#include"..//CommonControls/StringClass.h"
#include"..//CnCSuite_Tabcontrol.h"
#include"..//BasicFPO.h"
#include"..//uClipboard.h"
#include"..//ApplicationData.h"

#define		MAX_TBBUTTON	6


SampleManager::SampleManager(HWND host, HWND Main, const TCHAR* pathToSamples, HINSTANCE hInst)
	: startupSucceeded(TRUE),
	smpEditWnd(nullptr),
	sampleWnd(nullptr),
	edc(nullptr),
	draggingUnderway(FALSE),
	pWI(nullptr),
	toolbarMultiline(FALSE)
{
	this->mainWnd = Main;
	this->hostWnd = host;
	this->hInstance = hInst;

	this->dragInfo.isValid = false;

	if (pathToSamples == nullptr)
	{
		this->startupSucceeded = FALSE;
	}
	else
	{
		this->workingDir.Replace(pathToSamples);

		SendMessage(Main, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&this->StyleInfo));
		this->obj.background = CreateSolidBrush(this->StyleInfo.TabColor);
		this->obj.outlinePen = CreatePen(PS_SOLID, 1, this->StyleInfo.OutlineColor);
		this->obj.samplefile = (HCURSOR)LoadImage(this->hInstance, MAKEINTRESOURCE(IDC_SAMPLE), IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);

		this->_createDpiDependendResources();
	}
}

SampleManager::~SampleManager()
{
	// stop indicating
	if (this->pWI != nullptr)
		this->pWI->killEx(0);
	// kill child windows (only those who use the event sink)
	for (int i = IDMM_ADDSAMPLE; i <= IDMM_CLOSE; i++)
	{
		DestroyWindow(
			GetDlgItem(this->sampleWnd, i));
	}
	// unregister classes
	UnregisterClass(L"SMPMNGR_CLASS\0", this->hInstance);
	UnregisterClass(L"SMPEDIT_CLASS\0", this->hInstance);
	// clean up
	SafeRelease(&this->edc);
	DeleteObject(this->obj.background);
	DeleteObject(this->obj.font);
	DeleteObject(this->obj.outlinePen);
	DestroyCursor(this->obj.samplefile);
}

HRESULT SampleManager::Init(LPRECT rc, int nCmdShow)
{
	HRESULT hr = startupSucceeded ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		TCHAR className[] = L"SMPMNGR_CLASS\0";

		WNDCLASSEX wcx;
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcx.lpfnWndProc = SampleManager::sampleProc;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = sizeof(LONG_PTR);
		wcx.hIcon = NULL;
		wcx.hInstance = this->hInstance;
		wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcx.hbrBackground = nullptr;
		wcx.lpszClassName = className;
		wcx.lpszMenuName = NULL;
		wcx.hIconSm = NULL;

		hr = (RegisterClassEx(&wcx) == 0) ? E_FAIL : S_OK;
		if (SUCCEEDED(hr))
		{
			this->sampleWnd = CreateWindow(
				className,
				NULL,
				WS_CHILD,
				rc->left, rc->top,
				rc->right,
				rc->bottom,
				this->hostWnd,
				(HMENU)ID_SAMPLEMANAGER,
				this->hInstance,
				reinterpret_cast<LPVOID>(this));

			hr = (this->sampleWnd != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				hr = this->createContent();
				if (SUCCEEDED(hr))
				{
					ShowWindow(this->sampleWnd, nCmdShow);
					UpdateWindow(this->sampleWnd);
				}
			}
		}
	}
	return hr;
}

HRESULT SampleManager::InitSampleEditor(int Mode, LPSAMPLEPROPERTY psp)
{
	HRESULT hr = this->CreateSampleEditor(Mode);
	if (SUCCEEDED(hr))
	{
		hr = this->CreateSampleEditorChilds(Mode, psp);
		if (SUCCEEDED(hr))
		{
			// ...
		}
	}
	return hr;
}

void SampleManager::onQuit()
{
	this->createFilterImageSaverThread();
}

void SampleManager::onDpiChanged()
{
	this->_createDpiDependendResources();
	this->setTVFont();
	this->initTreeViewImageList();
}

LRESULT SampleManager::sampleProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SampleManager* sMngr = nullptr;

	if (message == WM_CREATE)
	{
		auto pCr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		sMngr = reinterpret_cast<SampleManager*>(pCr->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(sMngr));
		return 1;
	}
	else
	{
		sMngr = reinterpret_cast<SampleManager*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (sMngr != nullptr)
		{
			switch (message)
			{
			case WM_SIZE:
				return sMngr->OnSize();
			case WM_PAINT:
				return sMngr->OnPaint(hWnd);
			case WM_NOTIFY:
				return sMngr->OnNotify(lParam);
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_SETAPPSTYLE:
				return sMngr->OnSetAppStyle(lParam);
			case WM_MOUSEMOVE:
				return sMngr->OnMousemove(wParam, lParam);
			case WM_LBUTTONUP:
				return sMngr->OnLButtonUp();
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

LRESULT SampleManager::sampleEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SampleManager* sMngr = nullptr;

	if (message == WM_CREATE)
	{
		auto pCr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		sMngr = reinterpret_cast<SampleManager*>(pCr->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(sMngr));
		return 1;
	}
	else
	{
		sMngr = reinterpret_cast<SampleManager*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (sMngr != nullptr)
		{
			switch (message)
			{
			case WM_SIZE:
				return sMngr->OnSizeInAddWnd();
			case WM_PAINT:
				return sMngr->OnPaintInAddWnd(hWnd);
				//case WM_NOTIFY:
				//	return sMngr->OnNotify(lParam);
				//case WM_ERASEBKGND:
				//	return static_cast<LRESULT>(TRUE);
				//case WM_SETAPPSTYLE:
				//	return sMngr->OnSetAppStyle(lParam);
			case WM_DESTROY:
				SafeRelease(&sMngr->edc);
				return 0;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

DWORD SampleManager::filterImageProc(LPVOID lParam)
{
	auto _this = reinterpret_cast<SampleManager*>(lParam);
	if (_this != nullptr)
	{
		_this->saveFilterImage();
	}
	return 0;
}

HRESULT SampleManager::createContent()
{
	HRESULT hr = this->CreateToolbar();
	if (SUCCEEDED(hr))
	{
		hr = this->CreateTreeview();
		if (SUCCEEDED(hr))
		{
			// ...

		}
	}
	return hr;
}

HRESULT SampleManager::CreateToolbar()
{
	HRESULT hr = E_FAIL;

	RECT rcSmp;
	GetClientRect(this->sampleWnd, &rcSmp);

	POINT pt = { DPIScale(2),DPIScale(5) };	
	SIZE sz = { DPIScale(32),DPIScale(32) };

	iString toolTip;

	for (int i = 0; i < MAX_TBBUTTON; i++)
	{
		auto button =
			new CustomButton(
				this->sampleWnd,
				BUTTONMODE_ICON,
				&pt,
				&sz,
				IDMM_ADDSAMPLE + i,
				this->hInstance
			);
		hr = (button != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			toolTip.Replace(
				getStringFromResource(UI_TTP_SMPMNGR_ADDSAMPLE + i)
			);

			button->setEventListener(
				dynamic_cast<customButtonEventSink*>(this)
			);
			button->setAppearance_onlyIcon(
				this->getIconIDFromMenuId(IDMM_ADDSAMPLE + i, false),
				DPIScale(30)
			);
			button->setColors(
				this->StyleInfo.mainToolbarColor,
				makeSelectionColor(this->StyleInfo.mainToolbarColor),
				makePressedColor(this->StyleInfo.mainToolbarColor)
			);
			button->setBorder(
				TRUE,
				this->StyleInfo.OutlineColor
			);
			button->setDisabledIcon(this->getIconIDFromMenuId(IDMM_ADDSAMPLE + i, true));
			button->setTooltip(toolTip);

			if (this->StyleInfo.StyleID == STYLEID_LIGHTGRAY)
			{
				button->setDisabledColor(
					darkenColor(this->StyleInfo.mainToolbarColor, 30)
				);
			}
				
			hr = button->Create();
		}
		if (FAILED(hr))
		{
			break;
		}
		else
		{
			pt.x += DPIScale(34);

			if((pt.x + DPIScale(34)) > rcSmp.right)
			{
				if (pt.y < (rcSmp.bottom - DPIScale(50)))// 100????
				{
					pt.y += DPIScale(34);
					pt.x = DPIScale(2);
					this->toolbarMultiline++;
				}
			}
		}
	}
	// resize if multiline????

	return hr;
}

HRESULT SampleManager::CreateTreeview()
{
	RECT rc;
	HRESULT hr;

	hr = (this->sampleWnd != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		InitCommonControls();
		GetClientRect(this->sampleWnd, &rc);

		rc.bottom -= DPIScale(40);

		this->treeView = CreateWindowEx(
			0,
			WC_TREEVIEW,
			nullptr,
			WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_INFOTIP | TVS_FULLROWSELECT | TVS_SHOWSELALWAYS | TVS_EDITLABELS | TVS_TRACKSELECT | TVS_HASBUTTONS,
			0, DPIScale(40), rc.right, rc.bottom,
			this->sampleWnd,
			(HMENU)IDMM_SAMPLETREE,
			this->hInstance,
			nullptr
		);

		hr = (this->treeView != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			TreeView_SetBkColor(this->treeView, this->StyleInfo.Stylecolor);
			TreeView_SetTextColor(this->treeView, this->StyleInfo.TextColor);

			this->setTVFont();

			hr = (this->initTreeViewImageList()) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				hr = this->loadFilterImage() ? S_OK : E_FAIL;
			}
		}
	}
	return hr;
}

HRESULT SampleManager::CreateSampleEditor(int Mode)
{
	HRESULT hr = startupSucceeded ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		TCHAR className[] = L"SMPEDIT_CLASS\0";

		WNDCLASSEX wcx;
		if (!GetClassInfoEx(this->hInstance, className, &wcx))
		{
			wcx.cbSize = sizeof(WNDCLASSEX);
			wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
			wcx.lpfnWndProc = SampleManager::sampleEditProc;
			wcx.cbClsExtra = 0;
			wcx.cbWndExtra = sizeof(LONG_PTR);
			wcx.hIcon = NULL;
			wcx.hInstance = this->hInstance;
			wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcx.hbrBackground = this->obj.background;
			wcx.lpszClassName = className;
			wcx.lpszMenuName = NULL;
			wcx.hIconSm = NULL;

			hr = (RegisterClassEx(&wcx) == 0) ? E_FAIL : S_OK;
		}
		if (SUCCEEDED(hr))
		{
			int resID = (Mode == IDMM_ADDSAMPLE) ? UI_TTP_SMPMNGR_ADDSAMPLE : UI_TTP_SMPMNGR_CHANGESAMPLE;

			int cx = GetSystemMetrics(SM_CXSCREEN);
			int cy = GetSystemMetrics(SM_CYSCREEN);

			int x, y, width, height;
			x = (cx / 2) - DPIScale(250);
			y = (cy / 2) - DPIScale(300);
			width = DPIScale(500);
			height = DPIScale(600);
			
			if (x < 0)x = 0;
			if (y < 0)y = 0;
			if (width > cx)width = cx;
			if (height > cy)height = cy;

			this->smpEditWnd
				= CreateWindow(
					className,
					getStringFromResource(resID),
					WS_POPUP | WS_BORDER | WS_CAPTION | WS_THICKFRAME,
					x, y, width, height,
					this->mainWnd, nullptr, this->hInstance, reinterpret_cast<LPVOID>(this));

			hr = (this->smpEditWnd != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				ShowWindow(this->smpEditWnd, SW_SHOW);
				UpdateWindow(this->smpEditWnd);
			}
		}
	}
	return hr;
}

HRESULT SampleManager::CreateSampleEditorChilds(int Mode, LPSAMPLEPROPERTY psp)
{
	RECT rc;
	HRESULT hr;

	GetClientRect(this->smpEditWnd, &rc);

	POINT pt = { (rc.right / 2), (rc.bottom - DPIScale(30)) };
	SIZE sz = { rc.right / 2, DPIScale(30) };

	auto closeButton = new CustomButton(this->smpEditWnd, BUTTONMODE_ICONTEXT, &pt, &sz, IDMM_CLOSESMPEDITOR, this->hInstance);

	hr = (closeButton != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		iString buttonText(getStringFromResource(UI_SMP_REJECT));

		closeButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
		closeButton->setAppearance_IconText(IDI_SMP_CLOSEMANAGER, 24, buttonText);
		closeButton->setColors(this->StyleInfo.ToolbarbuttonBkgnd, RGB(0, 120, 180), RGB(0, 100, 150));
		closeButton->setTextColor(this->StyleInfo.TextColor);
		closeButton->setBorder(TRUE, this->StyleInfo.OutlineColor);
		closeButton->setFont(
			CreateScaledFont(18, FW_BOLD, APPLICATION_PRIMARY_FONT)
		);
		
		hr = closeButton->Create();
		if (SUCCEEDED(hr))
		{
			this->obj.rejectButton = closeButton->getHandle();
			pt.x = 0;

			int id;
			if (Mode == IDMM_ADDSAMPLE)id = IDMM_SAVESAMPLE;
			else if (Mode == IDMM_CHANGEELEMENT)id = IDMM_CHANGESAMPLE;
			else id = Mode;

			auto saveButton = new CustomButton(this->smpEditWnd, BUTTONMODE_ICONTEXT, &pt, &sz, id, this->hInstance);

			hr = (saveButton != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				buttonText.Replace(getStringFromResource(UI_SMP_SAVESAMPLE));

				saveButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
				saveButton->setAppearance_IconText(IDI_SMP_SAVESAMPLE, 24, buttonText);
				saveButton->setColors(this->StyleInfo.ToolbarbuttonBkgnd, RGB(0, 120, 180), RGB(0, 100, 150));
				saveButton->setTextColor(this->StyleInfo.TextColor);
				saveButton->setBorder(TRUE, this->StyleInfo.OutlineColor);
				saveButton->setFont(
					CreateScaledFont(18, FW_BOLD, APPLICATION_PRIMARY_FONT)
				);

				hr = saveButton->Create();
				if (SUCCEEDED(hr))
				{
					this->obj.saveButton = saveButton->getHandle();

					CHARFORMAT cf;
					initCharformat(&cf, DPIScale(19), this->StyleInfo.TextColor, L"Segoe UI\0");

					this->obj.nameEdit =
						CreateWindow(
							MSFTEDIT_CLASS,
							nullptr,
							WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
							DPIScale(5),
							DPIScale(25),
							rc.right - DPIScale(10),
							DPIScale(28),
							this->smpEditWnd,
							(HMENU)IDM_NAMEEDIT,
							this->hInstance,
							nullptr
						);
					RichEdit_SetBkColor(this->obj.nameEdit, this->StyleInfo.Stylecolor);
					RichEdit_SetCharFormat(this->obj.nameEdit, SCF_ALL, &cf);

					this->obj.descEdit =
						CreateWindow(
							MSFTEDIT_CLASS,
							nullptr,
							WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
							DPIScale(5),
							DPIScale(78),
							rc.right - DPIScale(10),
							DPIScale(28),
							this->smpEditWnd,
							(HMENU)IDM_DESCEDIT,
							this->hInstance,
							nullptr
						);
					RichEdit_SetBkColor(this->obj.descEdit, this->StyleInfo.Stylecolor);
					RichEdit_SetCharFormat(this->obj.descEdit, SCF_ALL, &cf);

					this->obj.content =
						CreateWindow(
							MSFTEDIT_CLASS,
							nullptr,
							WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL,
							DPIScale(5),
							DPIScale(135),
							rc.right - DPIScale(10),
							rc.bottom - DPIScale(135 + 32),
							this->smpEditWnd,
							(HMENU)IDM_CONTENTEDIT,
							this->hInstance,
							nullptr
						);

					this->edc = CreateEditControlInstance(this->obj.content, this->smpEditWnd);

					TCPROPERTY tcp;
					SendMessage(this->mainWnd, WM_GETTABCTRLPROPERTY, 0, reinterpret_cast<LPARAM>(&tcp));

					tcp.ecp.focusmark = FALSE;// no focusmark in this window!

					this->edc->ConfigureComponent(&tcp.esc, &tcp.ecp, TRUE, TRUE);
					

					if (Mode == IDMM_ADDSAMPLE)
					{
						SELECTIONINFO selInfo;
						selInfo.success = FALSE;
						selInfo.selContent = nullptr;
						SendMessage(this->mainWnd, WM_GETCURRENTSELECTION, 0, reinterpret_cast<LPARAM>(&selInfo));

						if (selInfo.success)
						{
							this->edc->SetTextContent(selInfo.selContent, TRUE, TRUE, FALSE);
						}
					}
					else if (Mode == IDMM_CHANGEELEMENT)
					{
						this->SetSMPEditorContent(psp);

						this->elementChangeInfo.isValid = true;
						this->elementChangeInfo.name = psp->name;
						this->elementChangeInfo.path = psp->path;
						this->elementChangeInfo.item = this->curSelElement.item;
					}
					else
					{
						// fail...
					}
				}
			}
		}
	}
	return hr;
}

LRESULT SampleManager::OnSize()
{
	RECT rc;
	GetClientRect(this->sampleWnd, &rc);

	POINT pos = { DPIScale(2),DPIScale(5) };
	SIZE size = { DPIScale(32), DPIScale(32) };

	this->toolbarMultiline = FALSE;

	for (int i = 0; i < MAX_TBBUTTON; i++)
	{
		auto button =
			reinterpret_cast<CustomButton*>(
				CUSTOMBUTTONCLASS_FROMID(this->sampleWnd, IDMM_ADDSAMPLE + i)
				);
		if (button != nullptr)
		{
			button->setAppearance_onlyIcon(
				this->getIconIDFromMenuId(IDMM_ADDSAMPLE + i, false),
				DPIScale(30)
			);
			button->updateDimensions(&pos, &size);
		}
		pos.x += DPIScale(34);

		if ((pos.x + DPIScale(34)) > rc.right)
		{
			if (pos.y < (rc.bottom - DPIScale(50)))
			{
				pos.x = DPIScale(2);
				pos.y += DPIScale(34);

				if (i < (MAX_TBBUTTON - 1))
					this->toolbarMultiline++;
			}
		}
	}
	int yTVPos = DPIScale(40) + (this->toolbarMultiline * DPIScale(34));

	MoveWindow(
		GetDlgItem(this->sampleWnd, IDMM_SAMPLETREE),
		0,
		yTVPos,
		rc.right,
		rc.bottom - yTVPos,
		TRUE
	);
	return static_cast<LRESULT>(0);
}

LRESULT SampleManager::OnSizeInAddWnd()
{
	RECT rc;
	GetClientRect(this->smpEditWnd, &rc);

	MoveWindow(
		this->obj.rejectButton,
		rc.right / 2,
		rc.bottom - DPIScale(30),
		rc.right / 2,
		DPIScale(30),
		TRUE
	);
	MoveWindow(
		this->obj.saveButton,
		0,
		rc.bottom - DPIScale(30),
		rc.right / 2,
		DPIScale(30),
		TRUE
	);
	MoveWindow(
		this->obj.nameEdit,
		DPIScale(5),
		DPIScale(25),
		rc.right - DPIScale(10),
		DPIScale(28),
		TRUE
	);
	MoveWindow(
		this->obj.descEdit,
		DPIScale(5),
		DPIScale(78),
		rc.right - DPIScale(10),
		DPIScale(28),
		TRUE
	);
	MoveWindow(
		this->obj.content,
		DPIScale(5),
		DPIScale(135),
		rc.right - DPIScale(10),
		rc.bottom - DPIScale(135 + 32),
		TRUE
	);
	return static_cast<LRESULT>(0);
}

void SampleManager::onTBButtonClick(CTRLID ID)
{
	if (!this->isSelectionValid())
	{
		DispatchEWINotification(
			EDSP_INFO,
			L"SMP003",
			getStringFromResource(INFO_MSG_NOTARGETSELECTED),
			L"Templates"
		);
		return;
	}

	DWORD ID32 = (DWORD)ID;//x64 conformity

	switch (ID)
	{
	case IDMM_ADDSAMPLE:
		this->InitSampleEditor(IDMM_ADDSAMPLE, nullptr);
		break;
	case IDMM_ADDFILTER:
		this->AddNewFilter();
		break;
	case IDMM_CHANGEELEMENT:
		this->ChangeElement();
		break;
	case IDMM_REMOVESAMPLE:
		this->RemoveElement();
		break;
	case IDMM_CLOSESMPEDITOR:
		this->CloseSMPEditor(ID32);
		break;
	case IDMM_SAVESAMPLE:
		this->CloseSMPEditor(ID32);
		break;
	case IDMM_CHANGESAMPLE:
		this->CloseSMPEditor(ID32);
		break;
	case IDMM_INSERTSAMPLE:
		this->sendSampleInsertRequest();
		break;
	case IDMM_CCOPYSAMPLE:
		this->copySampleToClipboard();
		break;
	default:
		break;
	}
}

LRESULT SampleManager::OnNotify(LPARAM lParam)
{
	int result = 0;

	switch (((LPNMHDR)lParam)->code)
	{
	case TVN_ITEMEXPANDED:
		this->onItemExpandStatusChanged(lParam);
		break;
	case TVN_GETINFOTIP:
		this->onGetInfoTip(lParam);
		break;
	case TVN_SELCHANGED:
		this->onSelchange(lParam);
		break;
	case TVN_BEGINDRAG:
		this->onBeginDrag(lParam);
		break;
	case TVN_BEGINLABELEDIT:
		return static_cast<LRESULT>(this->onBeginLabelEdit(lParam));
	case TVN_ENDLABELEDIT:
		return static_cast<LRESULT>(this->onEndLabelEdit(lParam));
	case NM_DBLCLK:
		return this->OnDblClick();
	case NM_RCLICK:
		return this->OnRClick();
	default:
		break;
	}
	return static_cast<LRESULT>(result);
}

LRESULT SampleManager::OnSetAppStyle(LPARAM lParam)
{
	auto styleNfo = reinterpret_cast<LPAPPSTYLEINFO>(lParam);
	if (styleNfo != nullptr)
	{
		this->StyleInfo = *styleNfo;

		DeleteObject(this->obj.background);
		this->obj.background = CreateSolidBrush(this->StyleInfo.TabColor);
		
		TreeView_SetBkColor(this->treeView, this->StyleInfo.Stylecolor);
		TreeView_SetTextColor(this->treeView, this->StyleInfo.TextColor);

		for (int i = IDMM_ADDSAMPLE; i <= IDMM_CLOSE; i++)
		{
			HWND button = GetDlgItem(this->sampleWnd, i);
			if (button != nullptr)
			{
				auto cButton = reinterpret_cast<CustomButton*>(SendMessage(button, WM_GETWNDINSTANCE, 0,0));
				if (cButton != nullptr)
				{
					cButton->setColors(
						this->StyleInfo.mainToolbarColor,
						makeSelectionColor(this->StyleInfo.mainToolbarColor),
						makePressedColor(this->StyleInfo.mainToolbarColor)
					);
					cButton->setBorder(TRUE, this->StyleInfo.OutlineColor);

					if (this->StyleInfo.StyleID == STYLEID_LIGHTGRAY)
					{
						cButton->setDisabledColor(
							darkenColor(this->StyleInfo.mainToolbarColor, 30)
						);
					}
					else
					{
						cButton->setDisabledColor(RGB(100, 100, 100));
					}
				}
			}
		}
		RedrawWindow(this->sampleWnd, nullptr, nullptr, RDW_INVALIDATE);
	}
	return static_cast<LRESULT>(0);
}

LRESULT SampleManager::OnPaint(HWND hWnd)
{
	HDC hdc;
	RECT rc;
	PAINTSTRUCT ps;
	GetClientRect(hWnd, &rc);
	rc.bottom = DPIScale(45) + (this->toolbarMultiline * DPIScale(34));

	hdc = BeginPaint(hWnd, &ps);

	FillRect(hdc, &rc, this->obj.background);

	EndPaint(hWnd, &ps);

	return static_cast<LRESULT>(0);
}

LRESULT SampleManager::OnPaintInAddWnd(HWND hWnd)
{
	HDC hdc;
	RECT rc;
	HGDIOBJ original;
	PAINTSTRUCT ps;
	GetClientRect(hWnd, &rc);

	hdc = BeginPaint(hWnd, &ps);

	TextOutDC(
		hdc,
		getStringFromResource(UI_GNRL_NAME),
		DPIScale(5),
		DPIScale(5),
		this->obj.font,
		this->StyleInfo.TextColor
	);

	TextOutDC(
		hdc,
		getStringFromResource(UI_GNRL_DESCRIPTION),
		DPIScale(5),
		DPIScale(55),
		this->obj.font,
		this->StyleInfo.TextColor
	);

	TextOutDC(
		hdc,
		getStringFromResource(UI_GNRL_CONTENT),
		DPIScale(5),
		DPIScale(110),
		this->obj.font,
		this->StyleInfo.TextColor
	);

	original = SelectObject(hdc, this->obj.outlinePen);

	MoveToEx(hdc, 0, DPIScale(24), nullptr);
	LineTo(hdc, rc.right, DPIScale(24));
	MoveToEx(hdc, 0, DPIScale(77), nullptr);
	LineTo(hdc, rc.right, DPIScale(77));
	MoveToEx(hdc, 0, DPIScale(130), nullptr);
	LineTo(hdc, rc.right, DPIScale(130));

	SelectObject(hdc, original);
	EndPaint(hWnd, &ps);

	return static_cast<LRESULT>(0);
}

LRESULT SampleManager::OnClose(HWND hWnd)
{
	UNREFERENCED_PARAMETER(hWnd);
	//ShowWindow(hWnd, SW_HIDE);

	this->createFilterImageSaverThread();
	
	return static_cast<LRESULT>(0);
}

LRESULT SampleManager::OnDblClick()
{
	auto sample = this->loadSample();

	if (sample.GetLength() > 0)
	{
		DISPATCHTEXT dt;
		dt.destination = DT_DESTINATION_TABCONTROL;
		dt.from = this->sampleWnd;
		dt.mode = DT_INSERT;
		dt.text = sample.getContentReference();

		SendMessage(this->mainWnd, WM_DISPATCHTEXT, 0, reinterpret_cast<LPARAM>(&dt));
	}
	return static_cast<LRESULT>(0);
}

LRESULT SampleManager::OnRClick()
{
	POINT pt;
	TVHITTESTINFO thi;

	if (GetCursorPos(&pt))
	{
		if (ScreenToClient(this->treeView, &pt))
		{
			thi.pt.x = pt.x;
			thi.pt.y = pt.y;

			HTREEITEM item = TreeView_HitTest(this->treeView, &thi);
			if (item)
			{
				if (TreeView_SelectItem(this->treeView, thi.hItem))
				{
					TVITEM tvi;
					tvi.mask = TVIF_PARAM;
					tvi.hItem = thi.hItem;
					TreeView_GetItem(this->treeView, &tvi);

					int level = LOWORD(tvi.lParam);
					int type = HIWORD(tvi.lParam);

					auto cMenu =
						new CustomPopUpMenu(
							this->hInstance,
							dynamic_cast<customPopUpMenuEventSink*>(this)
						);
					if (cMenu != nullptr)
					{
						cMenu->setControlFont(
							CreateScaledFont(16, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
						);

						if (level == 1)
						{
							// is root item
							MenuEntry addSample;
							addSample.setID(IDMM_ADDSAMPLE);
							addSample.setText(getStringFromResource(UI_TTP_SMPMNGR_ADDSAMPLE));
							addSample.setIcon(IDI_EXP_ADDFILE, DPIScale(18));
							addSample.setColors(this->StyleInfo.MenuPopUpColor, this->StyleInfo.TextColor, RGB(0, 120, 180), RGB(0, 100, 150));
							cMenu->addMenuEntry(&addSample);

							MenuEntry addFilter;
							addFilter.setID(IDMM_ADDFILTER);
							addFilter.setText(getStringFromResource(UI_TTP_SMPMNGR_ADDFILTER));
							addFilter.setIcon(IDI_EXP_ADDFOLDER, DPIScale(18));
							addFilter.setColors(this->StyleInfo.MenuPopUpColor, this->StyleInfo.TextColor, RGB(0, 120, 180), RGB(0, 100, 150));
							cMenu->addMenuEntry(&addFilter);

							cMenu->Show(this->mainWnd, nullptr, 250, this->StyleInfo.MenuPopUpColor);
						}
						else
						{
							if (type == ITEMTYPE_FILTER)
							{
								MenuEntry addSample;
								addSample.setID(IDMM_ADDSAMPLE);
								addSample.setText(getStringFromResource(UI_TTP_SMPMNGR_ADDSAMPLE));
								addSample.setIcon(IDI_EXP_ADDFILE, DPIScale(18));
								addSample.setColors(this->StyleInfo.MenuPopUpColor, this->StyleInfo.TextColor, RGB(0, 120, 180), RGB(0, 100, 150));
								cMenu->addMenuEntry(&addSample);

								MenuEntry addFilter;
								addFilter.setID(IDMM_ADDFILTER);
								addFilter.setText(getStringFromResource(UI_TTP_SMPMNGR_ADDFILTER));
								addFilter.setIcon(IDI_EXP_ADDFOLDER, DPIScale(18));
								addFilter.setColors(this->StyleInfo.MenuPopUpColor, this->StyleInfo.TextColor, RGB(0, 120, 180), RGB(0, 100, 150));
								cMenu->addMenuEntry(&addFilter);

								MenuEntry changeElement;
								changeElement.setID(IDMM_CHANGEELEMENT);
								changeElement.setText(getStringFromResource(UI_SMP_CHANGEELEMENT));
								changeElement.setIcon(IDI_EXP_RENAMEFILE, DPIScale(18));
								changeElement.setColors(this->StyleInfo.MenuPopUpColor, this->StyleInfo.TextColor, RGB(0, 120, 180), RGB(0, 100, 150));
								cMenu->addMenuEntry(&changeElement);

								MenuEntry removeElement;
								removeElement.setID(IDMM_REMOVESAMPLE);
								removeElement.setText(getStringFromResource(UI_TTP_SMPMNGR_REMOVESAMPLE));
								removeElement.setIcon(IDI_EXP_DELETEELEMENT, DPIScale(18));
								removeElement.setColors(this->StyleInfo.MenuPopUpColor, this->StyleInfo.TextColor, RGB(0, 120, 180), RGB(0, 100, 150));
								cMenu->addMenuEntry(&removeElement);								

								cMenu->Show(this->mainWnd, nullptr, 250, this->StyleInfo.MenuPopUpColor);

							}
							else if (type == ITEMTYPE_SAMPLE)
							{
								MenuEntry insert;
								insert.setID(IDMM_INSERTSAMPLE);
								insert.setText(getStringFromResource(UI_SMP_INSERTSAMPLE));
								insert.setIcon(IDI_SMP_INSERTSAMPLE, DPIScale(18));
								insert.setColors(this->StyleInfo.MenuPopUpColor, this->StyleInfo.TextColor, RGB(0, 120, 180), RGB(0, 100, 150));
								cMenu->addMenuEntry(&insert);

								MenuEntry ccopy;
								ccopy.setID(IDMM_CCOPYSAMPLE);
								ccopy.setText(getStringFromResource(UI_GNRL_COPYTOCLIPBOARD));
								ccopy.setIcon(IDI_EXP_COPYELEMENT, DPIScale(18));
								ccopy.setColors(this->StyleInfo.MenuPopUpColor, this->StyleInfo.TextColor, RGB(0, 120, 180), RGB(0, 100, 150));
								cMenu->addMenuEntry(&ccopy);

								MenuEntry changeElement;
								changeElement.setID(IDMM_CHANGEELEMENT);
								changeElement.setText(getStringFromResource(UI_SMP_CHANGEELEMENT));
								changeElement.setIcon(IDI_EXP_RENAMEFILE, DPIScale(18));
								changeElement.setColors(this->StyleInfo.MenuPopUpColor, this->StyleInfo.TextColor, RGB(0, 120, 180), RGB(0, 100, 150));
								cMenu->addMenuEntry(&changeElement);

								MenuEntry removeElement;
								removeElement.setID(IDMM_REMOVESAMPLE);
								removeElement.setText(getStringFromResource(UI_TTP_SMPMNGR_REMOVESAMPLE));
								removeElement.setIcon(IDI_EXP_DELETEELEMENT, DPIScale(18));
								removeElement.setColors(this->StyleInfo.MenuPopUpColor, this->StyleInfo.TextColor, RGB(0, 120, 180), RGB(0, 100, 150));
								cMenu->addMenuEntry(&removeElement);

								cMenu->Show(this->treeView, nullptr, 250, this->StyleInfo.MenuPopUpColor);

							}
						}
					}
				}
			}
		}
	}

	return static_cast<LRESULT>(TRUE);
}

LRESULT SampleManager::OnMousemove(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

	if (this->draggingUnderway)
	{
		HTREEITEM target;
		TVHITTESTINFO tvht;

		auto xPos = GET_X_LPARAM(lParam);
		auto yPos = GET_Y_LPARAM(lParam);

		POINT pt;
		pt.x = xPos;
		pt.y = yPos;

		ClientToScreen(this->sampleWnd, &pt);
		ScreenToClient(this->treeView, &pt);

		tvht.pt.x = pt.x;
		tvht.pt.y = pt.y;

		if ((target = TreeView_HitTest(this->treeView, &tvht)) != nullptr)
		{
			TreeView_SelectDropTarget(this->treeView, target);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT SampleManager::OnLButtonUp()
{
	if (this->draggingUnderway)
	{
		HTREEITEM hti
			= TreeView_GetDropHilight(this->treeView);

		if (hti != nullptr)
		{
			// move item and save filterimage
			if (this->dragInfo.isValid)
			{
				iString sourceName(this->dragInfo.sourceName);

				TVITEM target;
				target.mask = TVIF_PARAM | TVIF_HANDLE;
				target.hItem = hti;
				TreeView_GetItem(this->treeView, &target);

				HTREEITEM newParent;

				if (HIWORD(target.lParam) == ITEMTYPE_SAMPLE)
				{
					this->dragInfo.source.lParam
						= MAKELPARAM(
							LOWORD(target.lParam), ITEMTYPE_SAMPLE);

					newParent = TreeView_GetParent(this->treeView, hti);
				}
				else
				{
					this->dragInfo.source.lParam
						= MAKELPARAM(
							LOWORD(target.lParam) + 1, ITEMTYPE_SAMPLE);

					newParent = hti;
				}

				TreeView_DeleteItem(this->treeView, this->dragInfo.source.hItem);

				TVINSERTSTRUCT tvins;
				tvins.hParent = newParent;
				tvins.hInsertAfter = this->FindInsertAfter(newParent, ITEMTYPE_SAMPLE, sourceName);
				tvins.item = this->dragInfo.source;

				TreeView_InsertItem(this->treeView, &tvins);

				SecureZeroMemory(&this->dragInfo, sizeof(SMPDRAGINFORMATION));
				this->dragInfo.isValid = false;

				this->createFilterImageSaverThread();
			}
		}
		TreeView_SelectDropTarget(this->treeView, nullptr);
		ReleaseCapture();

		SetCursor(
			LoadCursor(nullptr, IDC_ARROW)
		);
		this->draggingUnderway = FALSE;
	}
	return static_cast<LRESULT>(0);
}

BOOL SampleManager::initTreeViewImageList()
{
	HIMAGELIST hList;
	HICON hIcon;
	BOOL res;

	hList =
		ImageList_Create(
			DPIScale(24),
			DPIScale(24),
			ILC_COLOR32,
			3, 0
		);

	res = (hList != nullptr) ? TRUE : FALSE;
	if (res)
	{
		hIcon = (HICON) LoadImage(
			this->hInstance, 
			MAKEINTRESOURCE(IDI_SMP_TV_FOLDERCLOSED), 
			IMAGE_ICON, 
			DPIScale(24),
			DPIScale(24), 
			LR_DEFAULTCOLOR
		);
		
		res = (hIcon != nullptr) ? TRUE : FALSE;
		if (res)
		{
			res = (ImageList_AddIcon(hList, hIcon) == -1) ? FALSE : TRUE;
			if (res)
			{
				DestroyIcon(hIcon);

				hIcon = (HICON)LoadImage(
					this->hInstance, 
					MAKEINTRESOURCE(IDI_SMP_TV_FOLDEROPEN), 
					IMAGE_ICON, 
					DPIScale(24), 
					DPIScale(24), 
					LR_DEFAULTCOLOR
				);

				res = (hIcon != nullptr) ? TRUE : FALSE;
				if (res)
				{
					res = (ImageList_AddIcon(hList, hIcon) == -1) ? FALSE : TRUE;
					if (res)
					{
						DestroyIcon(hIcon);

						hIcon = (HICON)LoadImage(
							this->hInstance, 
							MAKEINTRESOURCE(IDI_SMP_TV_SAMPLEFILE),
							IMAGE_ICON, 
							DPIScale(24), 
							DPIScale(24), 
							LR_DEFAULTCOLOR
						);

						res = (hIcon != nullptr) ? TRUE : FALSE;
						if (res)
						{
							res = (ImageList_AddIcon(hList, hIcon) == -1) ? FALSE : TRUE;
							if (res)
							{
								DestroyIcon(hIcon);

								hIcon = (HICON)LoadImage(
									this->hInstance, 
									MAKEINTRESOURCE(IDI_SMP_TV_FOLDERCLOSED_SEL), 
									IMAGE_ICON, 
									DPIScale(24), 
									DPIScale(24), 
									LR_DEFAULTCOLOR
								);

								res = (hIcon != nullptr) ? TRUE : FALSE;
								if (res)
								{
									res = (ImageList_AddIcon(hList, hIcon) == -1) ? FALSE : TRUE;
									if (res)
									{
										DestroyIcon(hIcon);

										hIcon = (HICON)LoadImage(
											this->hInstance, 
											MAKEINTRESOURCE(IDI_SMP_TV_FOLDEROPEN_SEL),
											IMAGE_ICON, 
											DPIScale(24), 
											DPIScale(24), 
											LR_DEFAULTCOLOR
										);

										res = (hIcon != nullptr) ? TRUE : FALSE;
										if (res)
										{
											res = (ImageList_AddIcon(hList, hIcon) == -1) ? FALSE : TRUE;
											if (res)
											{
												DestroyIcon(hIcon);

												hIcon = (HICON)LoadImage(
													this->hInstance, 
													MAKEINTRESOURCE(IDI_SMP_TV_SAMPLEFILE_SEL), 
													IMAGE_ICON, 
													DPIScale(24), 
													DPIScale(24), 
													LR_DEFAULTCOLOR
												);

												res = (hIcon != nullptr) ? TRUE : FALSE;
												if (res)
												{
													res = (ImageList_AddIcon(hList, hIcon) == -1) ? FALSE : TRUE;
													if (res)
													{
														DestroyIcon(hIcon);

														res = (ImageList_GetImageCount(hList) < 6) ? FALSE : TRUE;
														if (res)
														{
															TreeView_SetImageList(this->treeView, hList, TVSIL_NORMAL);
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return res;
}

BOOL SampleManager::initTreeViewItems(LPSAMPLEINSERTARRAY pSIA, int itemCount)
{
	HTREEITEM *hList = new HTREEITEM[itemCount];
	if (hList != nullptr)
	{
		for (int i = 0; i < itemCount; i++)
		{
			hList[i] = this->addItemToTree(pSIA[i].heading, pSIA[i].level, pSIA[i].type, pSIA[i].state);
			if (!hList[i])
				return FALSE;
		}
		for (int i = 0; i < itemCount; i++)
		{
			if (pSIA[i].state == TVE_EXPAND)
			{
				TreeView_Expand(this->treeView, hList[i], TVE_EXPAND);
			}
		}
		SafeDeleteArray(&hList);
	}
	return TRUE;
}

HTREEITEM SampleManager::addItemToTree(LPTSTR name, int level, int type, int state)
{
	UNREFERENCED_PARAMETER(state);

	TVITEM tvi;
	TVINSERTSTRUCT tvins;
	static HTREEITEM hPrev = (HTREEITEM)TVI_FIRST;
	static HTREEITEM hPrevRootItem = NULL;
	static HTREEITEM hPrevLev2Item = NULL;
	static HTREEITEM hPrevLev3Item = NULL;
	static HTREEITEM hPrevLev4Item = NULL;
	static HTREEITEM hPrevLev5Item = NULL;

	LPARAM lParam = MAKELPARAM(level, type);

	tvi.mask = TVIF_TEXT | TVIF_IMAGE
		| TVIF_SELECTEDIMAGE | TVIF_PARAM;

	// Set the text of the item. 
	tvi.pszText = name;
	tvi.cchTextMax = SMPMNGR_MAX_HEADING_LEN;

	// Set the image. 
	if (type == ITEMTYPE_FILTER)
	{
		tvi.iImage = TV_IMG_INDEX_FILTERCLOSED;
		tvi.iSelectedImage = TV_IMG_INDEX_FILTERCLOSED_SEL;
	}
	else if (type == ITEMTYPE_SAMPLE)
	{
		tvi.iImage = TV_IMG_INDEX_SAMPLEFILE;
		tvi.iSelectedImage = TV_IMG_INDEX_SAMPLEFILE_SEL;
	}

	// Save the heading level in the item's application-defined 
	// data area. 
	tvi.lParam = lParam;
	tvins.item = tvi;
	tvins.hInsertAfter = hPrev;

	// Set the parent item based on the specified level. 
	if (level == 1)
		tvins.hParent = TVI_ROOT;
	else if (level == 2)
		tvins.hParent = hPrevRootItem;
	else if (level == 3)
		tvins.hParent = hPrevLev2Item;
	else if (level == 4)
		tvins.hParent = hPrevLev3Item;
	else if (level == 5)
		tvins.hParent = hPrevLev4Item;
	else
		tvins.hParent = hPrevLev5Item;

	// Add the item to the tree-view control. 
	hPrev =
		(HTREEITEM)SendMessage(
			this->treeView,
			TVM_INSERTITEM,
			0,
			reinterpret_cast<LPARAM>(&tvins));

	if (hPrev == nullptr)
		return nullptr;

	// Save the handle to the item. 
	if (level == 1)
		hPrevRootItem = hPrev;
	else if (level == 2)
		hPrevLev2Item = hPrev;
	else if (level == 3)
		hPrevLev3Item = hPrev;
	else if (level == 4)
		hPrevLev4Item = hPrev;
	else if (level == 5)
		hPrevLev5Item = hPrev;

	return hPrev;
}

BOOL SampleManager::loadFilterImage()
{
	auto parser = new XML_Parser(dynamic_cast<XMLParsingEventSink*>(this));
	if (parser != nullptr)
	{
		iString path(this->workingDir);
		path.Append(L"\\\0");
		path.Append(L"filterImage.xml\0");

		if (parser->OpenDocument(path))
		{
			this->pWI = new workingIndicator(this->hInstance);
			if (this->pWI != nullptr)
			{
				RECT rc;
				GetClientRect(this->sampleWnd, &rc);

				POINT pt;
				pt.x = 0;
				pt.y = 35;

				this->pWI->setExtendedAnimationProperty(
					ANIMATIONMODE_LINEAR_EXTENDED,
					this->StyleInfo.TabColor,
					IDI_WKI_MOVER_SQ5_ORANGE
				);
				this->pWI->showExtendedAnimation(this->sampleWnd, &pt, rc.right);
			}
			parser->setSublevelScan(true);
			parser->ParseAsync();
		}
		else
		{
			// filterimage does not exist -> insert root item
			this->insertRootFilter();
		}
		return TRUE;
	}
	return FALSE;
}

BOOL SampleManager::resolveFilterImage(itemCollection<iXML_Tag>* xmlStructure, int size)
{
	if (xmlStructure == nullptr)
	{
		return FALSE;
	}
	else
	{
		BOOL result = TRUE;
		int level = 1;
		size += 1;

		LPSAMPLEINSERTARRAY sia = new SAMPLEINSERTARRAY[size];
		if (sia != nullptr)
		{
			int cnt = 0;
			// add template-folder-item:
			sia[cnt].level = level;
			sia[cnt].state = TVE_EXPAND;
			sia[cnt].type = ITEMTYPE_FILTER;

			StringCbCopy(
				sia[cnt].heading,
				sizeof(sia[cnt].heading),
				getStringFromResource(UI_SMP_TEMPLATEFOLDER));


			cnt++;
			level++;

			for (int i = 0; i < xmlStructure->GetCount(); i++)
			{
				if (cnt == size)return FALSE;

				if (xmlStructure->GetAt(i).tagName.Equals(L"container\0"))
				{
					// type must be filter - check property-type???

					sia[cnt].level = level;
					sia[cnt].type = ITEMTYPE_FILTER;

					StringCbCopy(
						sia[cnt].heading,
						sizeof(sia[cnt].heading),
						xmlStructure->GetAt(i).getPropertyContentFromName(L"name\0").GetData());

					sia[cnt].state
						= xmlStructure->GetAt(i).getPropertyContentFromName(L"state").Equals(L"collapsed\0")
						? TVE_COLLAPSE : TVE_EXPAND;

					//auto xws = xmlStructure->GetAt(i).subTAG_Structure.getInstance();
					//_NOT_USED(xws);

					cnt++;
					cnt = this->resolveFilterImageNextLevel(
						xmlStructure->GetAt(i).subTAG_Structure.getInstance(),
						sia, size, level, cnt);

					if (cnt == -1)
						result = FALSE;
				}
				else
				{
					// type must be sample - check property-type???

					if (xmlStructure->GetAt(i).tagName.Equals(L"element\0"))
					{
						sia[cnt].state = 0;
						sia[cnt].level = level;
						sia[cnt].type = ITEMTYPE_SAMPLE;

						StringCbCopy(
							sia[cnt].heading,
							sizeof(sia[cnt].heading),
							xmlStructure->GetAt(i).getPropertyContentFromName(L"name\0").GetData());

						cnt++;
					}
				}
			}
			if (this->pWI != nullptr)
			{
				this->pWI->killEx(0); // FINISH_SEQUENCE (...old)
				this->pWI = nullptr;
			}

			this->initTreeViewItems(sia, size);

			delete[] sia;

			return result;
		}
		return FALSE;
	}
}

int SampleManager::resolveFilterImageNextLevel(itemCollection<iXML_Tag>* xmlStructure, LPSAMPLEINSERTARRAY sia, int size, int level, int cnt)
{
	level++;

	for (int i = 0; i < xmlStructure->GetCount(); i++)
	{
		if (cnt == size)return -1;

		if (xmlStructure->GetAt(i).tagName.Equals(L"container\0"))
		{
			// type must be filter - check property-type???

			sia[cnt].level = level;
			sia[cnt].type = ITEMTYPE_FILTER;

			StringCbCopy(
				sia[cnt].heading,
				sizeof(sia[cnt].heading),
				xmlStructure->GetAt(i).getPropertyContentFromName(L"name\0").GetData());

			sia[cnt].state
				= xmlStructure->GetAt(i).getPropertyContentFromName(L"state").Equals(L"collapsed\0")
				? TVE_COLLAPSE : TVE_EXPAND;

			cnt++;
			cnt = this->resolveFilterImageNextLevel(xmlStructure->GetAt(i).subTAG_Structure.getInstance(), sia, size, level, cnt);
		}
		else
		{
			// type must be sample - check property-type???

			if (xmlStructure->GetAt(i).tagName.Equals(L"element\0"))
			{
				sia[cnt].state = 0;
				sia[cnt].level = level;
				sia[cnt].type = ITEMTYPE_SAMPLE;

				StringCbCopy(
					sia[cnt].heading,
					sizeof(sia[cnt].heading),
					xmlStructure->GetAt(i).getPropertyContentFromName(L"name\0").GetData());

				cnt++;
			}
		}
	}
	return cnt;
}

BOOL SampleManager::saveFilterImage()
{
	HTREEITEM root = TreeView_GetRoot(this->treeView);
	if (root != nullptr)
	{
		iString path(this->workingDir);
		iString file(L"\\filterImage.xml\0");
		path += file;

		HTREEITEM item = TreeView_GetChild(this->treeView, root);
		if (item != nullptr)
		{
			TCHAR buffer[SMPMNGR_MAX_HEADING_LEN] = { 0 };
			int type, level;

			TVITEM tvi;
			tvi.mask = TVIF_PARAM | TVIF_TEXT | TVIF_HANDLE | TVIF_STATE;
			tvi.cchTextMax = SMPMNGR_MAX_HEADING_LEN;
			tvi.pszText = buffer;
			tvi.stateMask = TVIS_STATEIMAGEMASK;

			auto builder = new XML_Builder();
			if (builder == nullptr)return FALSE;

			builder->setUp_DocumentDefinition(L"1.0\0", L"utf-8\0", L"cncf\0", true);

			iXML_Tag xmlTag;
			XML_TAG_Property properties;

			do
			{	
				tvi.hItem = item;

				TreeView_GetItem(this->treeView, &tvi);
				level = LOWORD(tvi.lParam);
				type = HIWORD(tvi.lParam);

				xmlTag.Clear();
				properties.Clear();

				if (type == ITEMTYPE_FILTER)
				{
					// define tag and properties
					xmlTag.tagName.Replace(L"container\0");
					xmlTag.hasProperties = true;

					properties.set(L"name\0", buffer);
					xmlTag.tagProperties.AddItem(properties);
					properties.set(L"type\0", L"filter\0");
					xmlTag.tagProperties.AddItem(properties);

					if (tvi.state & TVIS_EXPANDED)
						properties.set(L"state\0", L"expanded\0");
					else
						properties.set(L"state\0", L"collapsed\0");

					xmlTag.tagProperties.AddItem(properties);

					// scan next level
					this->saveFilterImageNextLevel(&xmlTag.subTAG_Structure, item);

					builder->AddTag(&xmlTag);
				}
				else if (type == ITEMTYPE_SAMPLE)
				{
					// define tag and properties
					xmlTag.tagName.Replace(L"element\0");

					xmlTag.hasProperties = true;
					xmlTag.initallyClosed = true;

					properties.set(L"name\0", buffer);
					xmlTag.tagProperties.AddItem(properties);
					properties.set(L"type\0", L"sample\0");
					xmlTag.tagProperties.AddItem(properties);

					builder->AddTag(&xmlTag);
				}
				else
				{
					return FALSE;
				}

			} while ((item = TreeView_GetNextSibling(this->treeView, item)) != nullptr);

			if (builder->getTAGCount() > 0)
			{
				builder->finalizeAsync(path, true);
			}
		}
		else
		{
			// there is no child item anymore so delete the filter image!
			DeleteFile(path.GetData());
		}
	}
	return TRUE;
}

BOOL SampleManager::saveFilterImageNextLevel(itemCollection<iXML_Tag>* parentTag, HTREEITEM item)
{
	item = TreeView_GetChild(this->treeView, item);
	if (item != nullptr)
	{
		TCHAR buffer[SMPMNGR_MAX_HEADING_LEN] = { 0 };
		int type, level;

		TVITEM tvi;
		tvi.mask = TVIF_PARAM | TVIF_TEXT | TVIF_HANDLE | TVIF_STATE;
		tvi.cchTextMax = SMPMNGR_MAX_HEADING_LEN;
		tvi.pszText = buffer;
		tvi.stateMask = TVIS_STATEIMAGEMASK;

		iXML_Tag xmlTag;
		XML_TAG_Property properties;

		do
		{
			tvi.hItem = item;

			TreeView_GetItem(this->treeView, &tvi);
			level = LOWORD(tvi.lParam);
			type = HIWORD(tvi.lParam);

			xmlTag.Clear();
			properties.Clear();

			if (type == ITEMTYPE_FILTER)
			{
				// define tag and properties
				xmlTag.tagName.Replace(L"container\0");
				xmlTag.hasProperties = true;

				properties.set(L"name\0", buffer);
				xmlTag.tagProperties.AddItem(properties);
				properties.set(L"type\0", L"filter\0");
				xmlTag.tagProperties.AddItem(properties);

				if (tvi.state & TVIS_EXPANDED)
					properties.set(L"state\0", L"expanded\0");
				else
					properties.set(L"state\0", L"collapsed\0");

				xmlTag.tagProperties.AddItem(properties);

				// scan next level
				this->saveFilterImageNextLevel(&xmlTag.subTAG_Structure, item);

				parentTag->AddItem(xmlTag);
			}
			else if (type == ITEMTYPE_SAMPLE)
			{
				// define tag and properties
				xmlTag.tagName.Replace(L"element\0");

				xmlTag.hasProperties = true;
				xmlTag.initallyClosed = true;

				properties.set(L"name\0", buffer);
				xmlTag.tagProperties.AddItem(properties);
				properties.set(L"type\0", L"sample\0");
				xmlTag.tagProperties.AddItem(properties);

				parentTag->AddItem(&xmlTag);
			}
			else
			{
				return FALSE;
			}

		} while ((item = TreeView_GetNextSibling(this->treeView, item)) != nullptr);
	}
	return TRUE;
}

void SampleManager::createFilterImageSaverThread()
{
	HANDLE hThread;
	DWORD threadId;

	hThread = CreateThread(nullptr, 0, SampleManager::filterImageProc, reinterpret_cast<LPVOID>(this), 0, &threadId);
	if (hThread != nullptr)
	{
		WaitForSingleObject(hThread, 10);
		CloseHandle(hThread);
	}
}

void SampleManager::onItemExpandStatusChanged(LPARAM lParam)
{
	auto ptv = reinterpret_cast<LPNMTREEVIEW>(lParam);
	if (ptv != nullptr)
	{
		TVITEM tvi;
		tvi.mask = TVIF_IMAGE | TVIF_HANDLE | TVIF_SELECTEDIMAGE;
		tvi.hItem = ptv->itemNew.hItem;

		if (ptv->action == TVE_EXPAND)
		{
			tvi.iImage = TV_IMG_INDEX_FILTEROPEN;
			tvi.iSelectedImage = TV_IMG_INDEX_FILTEROPEN_SEL;
		}
		else
		{
			tvi.iImage = TV_IMG_INDEX_FILTERCLOSED;
			tvi.iSelectedImage = TV_IMG_INDEX_FILTERCLOSED_SEL;
		}
		TreeView_SetItem(this->treeView, &tvi);
	}
	this->createFilterImageSaverThread();
}

void SampleManager::onGetInfoTip(LPARAM lParam)
{
	auto pTip = reinterpret_cast<LPNMTVGETINFOTIP>(lParam);
	if (pTip != nullptr)
	{
		TCHAR* tipText = nullptr;
		TCHAR itemName[SMPMNGR_MAX_HEADING_LEN] = { 0 };
		
		TVITEM tvi;
		tvi.mask = TVIF_TEXT | TVIF_IMAGE;
		tvi.hItem = pTip->hItem;
		tvi.pszText = itemName;
		tvi.cchTextMax = SMPMNGR_MAX_HEADING_LEN;

		TreeView_GetItem(this->treeView, &tvi);

		if (tvi.iImage == TV_IMG_INDEX_SAMPLEFILE)
		{
			if (this->GetInfoTip(itemName, &tipText))
			{
				StringCbCopy(pTip->pszText, sizeof(TCHAR) * STRSAFE_MAX_CCH, tipText);

				SafeDeleteArray(&tipText);
			}
		}
	}
}

void SampleManager::onSelchange(LPARAM lParam)
{
	// set the current selection info for insert, delete or change

	auto pTv = reinterpret_cast<LPNMTREEVIEW>(lParam);
	if (pTv != nullptr)
	{
		this->getItemInfo(pTv->itemNew.hItem, &this->curSelElement);
		this->setTBButtonEnabledState();
	}
}

BOOL SampleManager::onEndLabelEdit(LPARAM lParam)
{
	auto pDisp = reinterpret_cast<LPNMTVDISPINFO>(lParam);
	if ( pDisp != nullptr)
	{
		if (pDisp->item.pszText != nullptr)
		{
			iString _name_(pDisp->item.pszText);

			HTREEITEM parent = TreeView_GetParent(this->treeView, pDisp->item.hItem);
			int level = LOWORD(pDisp->item.lParam);
			int type = HIWORD(pDisp->item.lParam);

			BOOL exp = TreeView_Expand(this->treeView, pDisp->item.hItem, TVE_COLLAPSE);

			TVITEM tvi_old;
			tvi_old.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_HANDLE;
			tvi_old.hItem = pDisp->item.hItem;
			TreeView_GetItem(this->treeView, &tvi_old);

			TVITEM tvi_new;
			tvi_new.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
			tvi_new.cchTextMax = _name_.GetLength();
			tvi_new.pszText = _name_.getContentReference();
			tvi_new.lParam = MAKELPARAM(level, type);
			tvi_new.iImage = tvi_old.iImage;
			tvi_new.iSelectedImage = tvi_old.iSelectedImage;

			TVINSERTSTRUCT tvins;
			tvins.hParent = parent;
			tvins.hInsertAfter = this->FindInsertAfter(parent, type, _name_);
			tvins.item = tvi_new;

			HTREEITEM newitem = TreeView_InsertItem(this->treeView, &tvins);
			if (newitem != nullptr)
			{
				this->moveSubItems(tvi_old.hItem, newitem);

				TreeView_SelectItem(this->treeView, newitem);

				if (exp)
				{
					TreeView_Expand(this->treeView, newitem, TVE_EXPAND);
				}
				TreeView_DeleteItem(this->treeView, pDisp->item.hItem);

				this->createFilterImageSaverThread();
			}
		}
		else
		{
			// label editing was cancelled
			// save anyway...

			this->createFilterImageSaverThread();
		}
	}
	return TRUE;
}

BOOL SampleManager::onBeginLabelEdit(LPARAM lParam)
{
	auto pDisp = reinterpret_cast<LPNMTVDISPINFO>(lParam);
	if (pDisp != nullptr)
	{
		int type = HIWORD(pDisp->item.lParam);
		if (type == ITEMTYPE_SAMPLE)
		{
			// TODO: notify user sth like: "use the 'change element'- button to change a sample
			return TRUE;
		}
	}
	return FALSE;
}

BOOL SampleManager::GetInfoTip(LPTSTR smpName, TCHAR ** tipOut)
{
	BOOL result = FALSE;

	iString path(this->workingDir);
	path.Append(L"\\\0");
	path.Append(smpName);
	path.Append(L".xml\0");

	//TODO: this function is a performance killer -> better is to hold all infotips in an itemCollection<iString> (Loaded by start + change)

	auto xmlP = new XML_Parser();
	if (xmlP != nullptr)
	{
		if (xmlP->OpenDocument(path))
		{
			if (xmlP->Parse())
			{
				if (xmlP->isParsingSucceeded())
				{
					auto docStruct = xmlP->getDocumentStructure();

					if (docStruct.GetCount() == 3)
					{
						if (docStruct.GetAt(1).tagName.Equals(L"description"))
						{
							if (CopyStringToPtr(docStruct.GetAt(1).tagContent.GetData(), tipOut) == TRUE)
							{
								result = TRUE;
							}
						}
					}
				}
			}
		}
		SafeRelease(&xmlP);
	}	
	return result;
}

BOOL SampleManager::ResolveFilter()
{
	HTREEITEM parent
		= TreeView_GetParent(this->treeView, this->curSelElement.item);

	if (parent != nullptr)
	{
		TCHAR buffer[SMPMNGR_MAX_HEADING_LEN] = { 0 };

		TVITEM tvi;
		tvi.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_CHILDREN;
		tvi.pszText = buffer;
		tvi.cchTextMax = SMPMNGR_MAX_HEADING_LEN;

		HTREEITEM child = TreeView_GetChild(this->treeView, this->curSelElement.item);
		HTREEITEM deleter = nullptr;
		HTREEITEM newItem = nullptr;
		TVINSERTSTRUCT tvins;

		if (child != nullptr)
		{			
			do
			{
				deleter = child;
				tvi.hItem = child;
				TreeView_GetItem(this->treeView, &tvi);

				int level = LOWORD(tvi.lParam);
				int type = HIWORD(tvi.lParam);

				if (level > 1)level--;
				tvi.lParam = MAKELPARAM(level, type);
				if (level == 1)
					tvins.hParent = TVI_ROOT;
				else
					tvins.hParent = parent;

				tvins.item = tvi;

				newItem = TreeView_InsertItem(this->treeView, &tvins);
				child = TreeView_GetNextSibling(this->treeView, child);

				//int type = this->getItemType(deleter);
				if ((type == ITEMTYPE_FILTER) && (tvi.cChildren > 0))
				{
					this->ResolveFilter_NextLevel(deleter, newItem);
				}
				TreeView_DeleteItem(this->treeView, deleter);

			} while (child != nullptr);

			// save filter image???
			//this->createFilterImageSaverThread();

		}
		TreeView_DeleteItem(this->treeView, this->curSelElement.item);
		TreeView_SelectItem(this->treeView, parent);
	}
	else
	{
		// this must be the root-item and cannot be deleted, so inform the user??
	}
	return TRUE;
}

BOOL SampleManager::ResolveFilter_NextLevel(HTREEITEM parent, HTREEITEM destination)
{
	if ((parent == nullptr) || (destination == nullptr))return FALSE;
	else
	{
		TCHAR buffer[SMPMNGR_MAX_HEADING_LEN] = { 0 };

		TVITEM tvi;
		tvi.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_CHILDREN;
		tvi.pszText = buffer;
		tvi.cchTextMax = SMPMNGR_MAX_HEADING_LEN;

		HTREEITEM child = TreeView_GetChild(this->treeView, parent);
		HTREEITEM newItem = nullptr;
		TVINSERTSTRUCT tvins;

		if (child != nullptr)
		{
			do
			{
				tvi.hItem = child;
				TreeView_GetItem(this->treeView, &tvi);

				int level = LOWORD(tvi.lParam);
				int type = HIWORD(tvi.lParam);

				if (level > 0)level--;
				tvi.lParam = MAKELPARAM(level, type);
				tvins.hParent = destination;
				tvins.item = tvi;

				newItem = TreeView_InsertItem(this->treeView, &tvins);
				//int type = this->getItemType(child);

				if ((type == ITEMTYPE_FILTER) && (tvi.cChildren > 0))
				{
					this->ResolveFilter_NextLevel(child, newItem);
				}
				child = TreeView_GetNextSibling(this->treeView, child);

			} while (child != nullptr);
		}
		return TRUE;
	}
}

BOOL SampleManager::GetSamplePropertyFromSelectedItem(LPSAMPLEPROPERTY psp)
{
	iString path(this->workingDir);
	iString filename(L"\\\0");
	filename.Append(this->curSelElement.name);
	filename.Append(L".xml\0");
	path += filename;
	psp->path = path;

	XML_Parser parser;
	if (!parser.OpenDocument(path))
		return FALSE;

	parser.Parse();

	if (parser.isParsingSucceeded())
	{
		auto xml_structure = parser.getDocumentStructure();

		for (int i = 0; i < xml_structure.GetCount(); i++)
		{
			if (xml_structure.GetAt(i).tagName.Equals(L"name\0"))
			{
				auto str = xml_structure.GetAt(i).tagContent;
				psp->name.Replace(str);
			}
			else if (xml_structure.GetAt(i).tagName.Equals(L"description\0"))
			{
				auto str = xml_structure.GetAt(i).tagContent;
				psp->description.Replace(str);
			}
			else if (xml_structure.GetAt(i).tagName.Equals(L"content\0"))
			{
				auto str = xml_structure.GetAt(i).tagContent;
				psp->content.Replace(str);
			}
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void SampleManager::AddNewFilter()
{
	int level, type;

	HTREEITEM item = TreeView_GetSelection(this->treeView);
	if (item != nullptr)
	{
		TVITEM tvi;
		tvi.mask = TVIF_PARAM;
		tvi.hItem = item;
		TreeView_GetItem(this->treeView, &tvi);

		level = LOWORD(tvi.lParam);
		type = HIWORD(tvi.lParam);

		// selected item is no filter
		if (type == ITEMTYPE_SAMPLE)
		{
			item = TreeView_GetParent(this->treeView, item);
			if (item == nullptr)
			{
				level = 0;
				item = TreeView_GetRoot(this->treeView);
			}
			else
				level--;
		}
	}
	else
	{
		item = TreeView_GetRoot(this->treeView);
		level = 0;
	}
	if (level == 4)
	{
		// max level reached
		DispatchEWINotification(
			EDSP_INFO,
			L"SMP001",
			getStringFromResource(UI_SMP_MAXLEVELREACHED),
			getStringFromResource(UI_SAMPLEMANAGER));
	}
	else
	{
		iString name(
			getStringFromResource(UI_SMP_NEWFILTER_PLACEHOLDER));

		TVITEM newFilter;
		newFilter.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
		newFilter.pszText = name.getContentReference();
		newFilter.iImage = TV_IMG_INDEX_FILTERCLOSED;
		newFilter.iSelectedImage = TV_IMG_INDEX_FILTERCLOSED;
		newFilter.lParam = MAKELPARAM((level + 1), ITEMTYPE_FILTER);

		TVINSERTSTRUCT tvins;
		tvins.hInsertAfter = this->FindInsertAfter(item, ITEMTYPE_FILTER, name.GetData());
		tvins.hParent = item;
		tvins.item = newFilter;

		HTREEITEM newItem = TreeView_InsertItem(this->treeView, &tvins);
		if (newItem != nullptr)
		{
			TreeView_Expand(this->treeView, item, TVE_EXPAND);
			TreeView_SelectItem(this->treeView, newItem);

			TreeView_EditLabel(this->treeView, newItem);

			// this->saveFilterImage();	// editlabel is doing this...!
		}
	}
	
}

void SampleManager::ChangeElement()
{
	if (this->curSelElement.type == ITEMTYPE_SAMPLE)
	{
		SAMPLEPROPERTY sp;
		if (this->GetSamplePropertyFromSelectedItem(&sp))
		{
			this->InitSampleEditor(IDMM_CHANGEELEMENT, &sp);
		}
	}
	else
	{
		TreeView_EditLabel(this->treeView, this->curSelElement.item);
	}
}

void SampleManager::RemoveElement()
{
	if (this->curSelElement.type == ITEMTYPE_FILTER)
	{
		iString message(this->curSelElement.name);
		message.Append(L"\n\n\0");
		message.Append(getStringFromResource(UI_SMP_CONFIRM_REMOVEFILTER));

		int res = MessageBox(
			this->mainWnd,
			message.GetData(),
			getStringFromResource(UI_SMP_CAPTIONCONFIRMATION),
			MB_OKCANCEL | MB_ICONASTERISK | MB_DEFBUTTON2	);

		if (res == IDCANCEL)return;
		else
		{
			// resolve filter and remove item
			if (!this->ResolveFilter())
			{
				// error message ??
			}
		}
	}
	else if (this->curSelElement.type == ITEMTYPE_SAMPLE)
	{
		iString message(this->curSelElement.name);
		message.Append(L"\n\n\0");
		message.Append(getStringFromResource(UI_SMP_CONFIRM_DELETESAMPLE));

		int res = MessageBox(
			this->mainWnd,
			message.GetData(),
			getStringFromResource(UI_SMP_CAPTIONCONFIRMATION),
			MB_OKCANCEL | MB_ICONASTERISK | MB_DEFBUTTON2);

		if (res == IDCANCEL)return;
		else
		{
			// delete file and remove item
			if (!DeleteFile(this->curSelElement.path.GetData()))
			{
				DispatchSystemError();
			}
			HTREEITEM parent
				= TreeView_GetParent(this->treeView, this->curSelElement.item);

			TreeView_DeleteItem(this->treeView, this->curSelElement.item);
			TreeView_SelectItem(this->treeView, parent);
		}
	}
	this->createFilterImageSaverThread();
}

void SampleManager::CloseSMPEditor(int mode)
{
	if (mode == IDMM_CLOSESMPEDITOR)
	{
		if (this->smpEditWnd != nullptr)
		{
			// kill childs
			//destroy editor
			// unregister class ...!
			

			DestroyWindow(this->smpEditWnd);
		}
		//else
		//{
		//	this->Release();// ?????
		//}
	}
	else if (mode == IDMM_SAVESAMPLE)
	{
		ShowWindow(this->smpEditWnd, SW_HIDE);

		if (this->saveSample(0))
		{
			DestroyWindow(this->smpEditWnd);
		}
		else
		{
			ShowWindow(this->smpEditWnd, SW_SHOW);
		}
	}
	else if (mode == IDMM_CHANGESAMPLE)
	{
		ShowWindow(this->smpEditWnd, SW_HIDE);

		if (this->saveSample(NO_EXIST_LOOKUP | DO_NOT_INSERT_IN_VIEW | ELEMENT_WAS_CHANGED))
		{
			this->elementChangeInfo.isValid = false;
			DestroyWindow(this->smpEditWnd);
		}
		else
		{
			ShowWindow(this->smpEditWnd, SW_SHOW);
		}
	}
}

void SampleManager::SetSMPEditorContent(LPSAMPLEPROPERTY psp)
{
	SetRichEditContent(
		this->obj.nameEdit,
		psp->name.getContentReference(),
		ST_DEFAULT);

	SetRichEditContent(
		this->obj.descEdit,
		psp->description.getContentReference(),
		ST_DEFAULT);

	this->edc->SetTextContent(
		psp->content.getContentReference(),
		TRUE, TRUE, FALSE
	);
}

int SampleManager::getItemType(HTREEITEM item)
{
	TVITEM tvi;
	tvi.hItem = item;
	tvi.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE;

	if (!TreeView_GetItem(this->treeView, &tvi))
	{
		return -1;
	}

	if (tvi.cChildren == 0)
	{
		if ((tvi.iImage == TV_IMG_INDEX_SAMPLEFILE) || (tvi.iImage == TV_IMG_INDEX_SAMPLEFILE_SEL))
		{
			return ITEMTYPE_SAMPLE;
		}
		else if ((tvi.iImage == TV_IMG_INDEX_FILTERCLOSED) || (tvi.iImage == TV_IMG_INDEX_FILTERCLOSED_SEL))
		{
			return ITEMTYPE_FILTER;
		}
		else
		{
			return ITEMTYPE_FILTER;
		}
	}
	else
	{
		return ITEMTYPE_FILTER;
	}
}

int SampleManager::getItemInfo(HTREEITEM item, LPCURRENTSELECTEDELEMENT pElement)
{
	TCHAR name[SMPMNGR_MAX_HEADING_LEN] = { 0 };

	int type = this->getItemType(item);

	pElement->type = type;
	pElement->item = item;

	TVITEM tvi;
	tvi.mask = TVIF_TEXT;
	tvi.hItem = item;
	tvi.pszText = name;
	tvi.cchTextMax = SMPMNGR_MAX_HEADING_LEN;
	TreeView_GetItem(this->treeView, &tvi);

	pElement->name.Replace(name);

	if (type == ITEMTYPE_FILTER)
	{
		pElement->path.Clear();		
	}
	else if (type == ITEMTYPE_SAMPLE)
	{
		pElement->path.Replace(this->workingDir);
		pElement->path.Append(L"\\\0");
		pElement->path.Append(name);
		pElement->path.Append(L".xml\0");
	}
	else
	{
		return -1;
	}
	return type;
}


int SampleManager::getItemPathEx(HTREEITEM RQitem, iString *Ipath)
{
	int type = -1;
	HRESULT hr;

	hr = (RQitem) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		type = this->getItemType(RQitem);

		hr = (type != -1) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			TCHAR* buffer = new TCHAR[SMPMNGR_MAX_HEADING_LEN];

			hr = (buffer == NULL) ? E_OUTOFMEMORY : S_OK;
			if (SUCCEEDED(hr))
			{
				TCHAR* root = new TCHAR[SMPMNGR_MAX_HEADING_LEN];

				hr = (root == NULL) ? E_OUTOFMEMORY : S_OK;
				if (SUCCEEDED(hr))
				{
					TCHAR* path = new TCHAR[(size_t)MAX_FILEPATH_LEN];

					hr = (path == NULL) ? E_OUTOFMEMORY : S_OK;
					if (SUCCEEDED(hr))
					{
						hr = StringCbCopy(path, MAX_FILEPATH_LEN, this->workingDir.GetData());
						if (SUCCEEDED(hr))
						{
							TVITEM tv;
							tv.hItem = TreeView_GetRoot(this->treeView);
							tv.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_IMAGE;
							tv.cchTextMax = sizeof(TCHAR) * SMPMNGR_MAX_HEADING_LEN;
							tv.pszText = root;

							hr = TreeView_GetItem(this->treeView, &tv) ? S_OK : E_FAIL;
							if (SUCCEEDED(hr))
							{
								HTREEITEM item = RQitem;

								hr = (item) ? S_OK : E_FAIL;
								if (SUCCEEDED(hr))
								{
									int ItemCount = 1;

									while ((item = TreeView_GetParent(this->treeView, item)) != NULL)
									{
										// count the levels ...
										ItemCount++;
									}
									HTREEITEM* cItems = new HTREEITEM[ItemCount];
									int max_array = ItemCount;

									hr = (cItems == NULL) ? E_OUTOFMEMORY : S_OK;
									if (SUCCEEDED(hr))
									{
										cItems[0] = RQitem;

										ItemCount = 1;

										if (ItemCount < max_array)
										{
											while ((cItems[ItemCount] = TreeView_GetParent(this->treeView, cItems[ItemCount - 1])) != NULL)
											{
												// fill the array ...
												ItemCount++;

												if (ItemCount == max_array)		// C6385 suppressor (useless?)
													break;
											}
											ItemCount--;
										}

										for (int j = ItemCount; j >= 0; j--)
										{
											if (j == max_array)		// C6385 suppressor (useless?)
												break;

											// build the path ...
											tv.hItem = cItems[j];
											tv.pszText = buffer;

											hr = TreeView_GetItem(this->treeView, &tv) ? S_OK : E_FAIL;
											if (FAILED(hr))break;

											if (CompareStringsB(root, buffer) && (j == ItemCount))
											{
												continue;
											}
											hr = StringCbCat(path, MAX_FILEPATH_LEN, L"\\");
											if (FAILED(hr))break;

											hr = StringCbCat(path, MAX_FILEPATH_LEN, buffer);
											if (FAILED(hr))break;
										}
										if (SUCCEEDED(hr))
										{
											if (type == ITEMTYPE_SAMPLE)
											{
												hr = StringCbCat(path, MAX_FILEPATH_LEN, L".xml\0");
											}
											if (SUCCEEDED(hr))
											{
												Ipath->Replace(path);											
											}
										}
									}
								}
							}
						}
						delete[] path;
					}
					delete[] root;
				}
				delete[] buffer;
			}
		}
	}
	if (FAILED(hr))
		type = -2;

	return type;
}

BOOL SampleManager::saveSample(DWORD additonalAction)
{
	BOOL result = FALSE;

	TCHAR* name = nullptr;
	TCHAR* description = nullptr;
	TCHAR* content = nullptr;

	if (GetRichEditContent(this->obj.nameEdit, &name) == TRUE)
	{
		if(this->verifySampleName(name))
		{
			if (GetRichEditContent(this->obj.descEdit, &description) == TRUE)
			{
				if (GetRichEditContent(this->obj.content, &content) == TRUE)
				{
					XML_Builder* builder = new XML_Builder();

					builder->setUp_DocumentDefinition(L"1.0\0", L"utf-8\0", L"cncs\0", true);

					iString xml_file_path(this->workingDir);
					xml_file_path.Append(L"\\\0");
					xml_file_path.Append(name);
					xml_file_path.Append(L".xml\0");

					iXML_Tag TAG;
					TAG.tagName.Replace(L"name\0");
					TAG.tagContent.Replace(name);
					TAG.initallyClosed = false;

					builder->AddTag(&TAG);

					TAG.Clear();
					TAG.tagName.Replace(L"description\0");
					TAG.tagContent.Replace(description);

					builder->AddTag(&TAG);

					TAG.Clear();
					TAG.tagName.Replace(L"content\0");
					TAG.tagContent.Replace(content);

					builder->AddTag(&TAG);

					auto bfpo = CreateBasicFPO();
					if (bfpo != nullptr)
					{
						BOOL f_exist = bfpo->CheckForFileExist(xml_file_path.getContentReference());
						if (f_exist)
						{
							if (additonalAction & NO_EXIST_LOOKUP)
							{
								f_exist = FALSE;
							}
							else
							{
								iString msg(name);
								msg.Append(getStringFromResource(UI_SMP_OVERRIDESAMPLE));

								int res = MessageBox(
									this->smpEditWnd,
									msg.GetData(),
									getStringFromResource(UI_SMP_SAVESAMPLE),
									MB_OKCANCEL | MB_ICONINFORMATION | MB_DEFBUTTON2);

								if (res == IDOK)
									f_exist = FALSE;
							}
						}
						if (!f_exist)
						{
							builder->finalizeAsync(xml_file_path, true);
							result = TRUE;
							
							if (additonalAction & ELEMENT_WAS_CHANGED)
							{
								if (this->elementChangeInfo.isValid)
								{
									if (!this->elementChangeInfo.name.Equals(name))
									{
										DeleteFile(
											this->elementChangeInfo.path.GetData()
										);

										TVITEM tvi;										
										tvi.mask = TVIF_TEXT | TVIF_HANDLE;
										tvi.hItem = this->elementChangeInfo.item;
										tvi.pszText = name;
										TreeView_SetItem(this->treeView, &tvi);

										this->updateSelectedItem();
									}
									this->elementChangeInfo.isValid = false;
								}
							}
							if (!(additonalAction & DO_NOT_INSERT_IN_VIEW))
							{
								this->insertSampleInView(name);
							}
							this->createFilterImageSaverThread();
						}
						SafeRelease(&bfpo);
					}
					SafeDeleteArray(&content);
				}
				SafeDeleteArray(&description);
			}
		}
		else
		{
			// filename not valid -> notify user
			DispatchEWINotification(
				EDSP_ERROR,
				L"SMP002\0",
				getStringFromResource(UI_GNRL_NAMEINVALID),
				getStringFromResource(UI_SAMPLEMANAGER));
		}
		SafeDeleteArray(&name);
	}
	return result;
}

iString SampleManager::loadSample()
{
	iString path(this->workingDir);
	path.Append(L"\\\0");
	path.Append(this->curSelElement.name);
	path.Append(L".xml\0");

	auto xmlP = new XML_Parser();
	if (xmlP != nullptr)
	{
		XMLCONTENTFORMATRULES cfRules;
		cfRules.allowCarriageReturnInContent = true;
		cfRules.allowLinefeedInContent = true;
		xmlP->setContentFormatRules(&cfRules);

		if (xmlP->OpenDocument(path))
		{
			if (xmlP->Parse())
			{
				if (xmlP->isParsingSucceeded())
				{
					iXML_Tag reqTag;
					iString name(L"content\0");

					if (xmlP->searchForTAG(name, &reqTag, nullptr))
					{
						return reqTag.tagContent;
					}
				}
			}
		}
		SafeRelease(&xmlP);
	}
	return iString(L"");
}

BOOL SampleManager::verifySampleName(LPTSTR name)
{
	BOOL result = FALSE;

	auto bFPO = CreateBasicFPO();
	if (bFPO != nullptr)
	{
		result = bFPO->VerifyFilename(name);

		SafeRelease(&bFPO);
	}
	return result;
}

BOOL SampleManager::insertSampleInView(LPTSTR name)
{
	iString _name_(name);
	int len = _name_.GetLength();

	HTREEITEM hParent;
	int level, type;

	TVITEM par;
	par.mask = TVIF_PARAM | TVIF_HANDLE;
	par.hItem = this->curSelElement.item;

	if (!TreeView_GetItem(this->treeView, &par))
	{
		hParent = TreeView_GetRoot(this->treeView);
		level = 1;
		type = ITEMTYPE_FILTER;
	}
	else
	{
		hParent = par.hItem;
		level = LOWORD(par.lParam);
		type = HIWORD(par.lParam);

		if (type == ITEMTYPE_SAMPLE)
		{
			hParent = TreeView_GetParent(this->treeView, par.hItem);
		}
	}

	TVITEM tvi;
	tvi.mask = TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	tvi.cchTextMax = len;
	tvi.pszText = name;
	tvi.iImage = TV_IMG_INDEX_SAMPLEFILE;
	tvi.iSelectedImage = TV_IMG_INDEX_SAMPLEFILE_SEL;
	tvi.lParam = MAKELPARAM(level + 1, ITEMTYPE_SAMPLE);

	TVINSERTSTRUCT tvins;
	tvins.hInsertAfter = this->FindInsertAfter(par.hItem, ITEMTYPE_SAMPLE, _name_);
	tvins.hParent = hParent;
	tvins.item = tvi;

	TreeView_InsertItem(this->treeView, &tvins);
	TreeView_Expand(this->treeView, par.hItem, TVE_EXPAND);

	return TRUE;
}

BOOL SampleManager::sendSampleInsertRequest()
{
	auto sample = this->loadSample();

	if (sample.GetLength() > 0)
	{
		DISPATCHTEXT dt;
		dt.destination = DT_DESTINATION_TABCONTROL;
		dt.from = this->sampleWnd;
		dt.mode = DT_INSERT;
		dt.text = sample.getContentReference();

		SendMessage(this->mainWnd, WM_DISPATCHTEXT, 0, reinterpret_cast<LPARAM>(&dt));

		return TRUE;
	}
	return FALSE;
}

bool SampleManager::copySampleToClipboard()
{
	clipBoardOperations cbOperation;

	bool result = cbOperation.copyTextToClipboard(this->loadSample());

	if(!result)
	{
		DispatchSystemError();
	}
	return result;
}

void SampleManager::insertRootFilter()
{
	SAMPLEINSERTARRAY sia;

	// add template-folder-item:
	sia.level = 1;
	sia.state = TVE_EXPAND;
	sia.type = ITEMTYPE_FILTER;

	StringCbCopy(
		sia.heading,
		sizeof(sia.heading),
		getStringFromResource(UI_SMP_TEMPLATEFOLDER));
	
	this->initTreeViewItems(&sia, 1);
}

int SampleManager::getIconIDFromMenuId(int menu_id, bool disabled)
{
	switch (menu_id)
	{
	case IDMM_ADDSAMPLE:
		return disabled ? IDI_EXP_ADDFILE_DISABLED : IDI_EXP_ADDFILE;
	case IDMM_ADDFILTER:
		return disabled ? IDI_EXP_ADDFOLDER_DISABLED : IDI_EXP_ADDFOLDER;
	case IDMM_CCOPYSAMPLE:
		return disabled ? IDI_EXP_COPYELEMENT_DISABLED : IDI_EXP_COPYELEMENT;
	case IDMM_CHANGEELEMENT:
		return disabled ? IDI_EXP_RENAMEFILE_DISABLED : IDI_EXP_RENAMEFILE;
	case IDMM_REMOVESAMPLE:
		return disabled ? IDI_EXP_DELETEELEMENT_DISABLED : IDI_EXP_DELETEELEMENT;
	case IDMM_INSERTSAMPLE:
		return disabled ? IDI_SMP_INSERTSAMPLE_DISABLED : IDI_SMP_INSERTSAMPLE;
	default:
		return -1;
	}
}

void SampleManager::setTBButtonEnabledState()
{
	auto dwEnableInfo = this->getDWButtonEnableInfo(this->curSelElement.type);
	if (dwEnableInfo != 0)
	{
		for (DWORD i = 0; i < MAX_TBBUTTON; i++)
		{
			auto button =
				reinterpret_cast<CustomButton*>(
					CUSTOMBUTTONCLASS_FROMID(this->sampleWnd, IDMM_ADDSAMPLE + i)
					);
			if (button != nullptr)
			{
				//if (dwEnableInfo & (1 << i))
				//{
				//	button->setEnabledState(false);
				//}
				//else
				//{
				//	button->setEnabledState(true);
				//}
				button->setEnabledState((dwEnableInfo & (1 << i)) ? true : false);
			}
		}
	}
}

void SampleManager::translateParsingError(LPPARSINGERROR ppErr)
{
	iString index(ppErr->charindex);
	iString preCode(L"XP \0");
	iString code = preCode + ppErr->errorcode;
	iString error;

	switch (ppErr->errorcode)
	{
	case XERR_SYNTAX_UNEXPECTEDCLOSINGTAG:
		error.Replace(
			getStringFromResource(UI_XML_ERR_UNEXPECTEDCLOSINGTAG));
		error += index;
		break;
	case XERR_DOCUMENTFORMAT_INVALID:
		error.Replace(
			getStringFromResource(UI_XML_ERR_DOCUMENTFORMAT_INVALID));
		break;
	case XERR_NOCLOSINGTAGFOUND:
	{
		iString second(getStringFromResource(UI_XML_ERR_NOCLOSING_ELEMENT_P2));
		error.Replace(getStringFromResource(UI_XML_ERR_NOCLOSING_ELEMENT_P1));
		error += ppErr->additionalInfo;
		error += second;
		error += index;
		break;
	}
	default:
		break;
	}
	DispatchEWINotification(EDSP_ERROR, code.getContentReference(), error.getContentReference(), L"XML Parser\0");
}

void SampleManager::onParsingCompleted(cObject sender, itemCollection<iXML_Tag>* tagStructure)
{
	auto parser = reinterpret_cast<XML_Parser*>(sender);
	if (parser != nullptr)
	{
		if (parser->getDoctype().Equals(L"cncf\0"))
		{
			this->resolveFilterImage(
				tagStructure,
				parser->getTotalTAGNumber()
			);
		}
		SafeRelease(&parser);
	}
}

void SampleManager::onBeginDrag(LPARAM lParam)
{
	auto nmtv = reinterpret_cast<LPNMTREEVIEW>(lParam);
	if (nmtv != nullptr)
	{
		if (HIWORD(nmtv->itemNew.lParam) == ITEMTYPE_SAMPLE)
		{
			SecureZeroMemory(&this->dragInfo, sizeof(SMPDRAGINFORMATION));
			this->dragInfo.source.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_HANDLE | TVIF_TEXT;
			this->dragInfo.source.cchTextMax = SMPMNGR_MAX_HEADING_LEN;
			this->dragInfo.source.pszText = this->dragInfo.sourceName;
			this->dragInfo.source.hItem = nmtv->itemNew.hItem;

			if (
				TreeView_GetItem(this->treeView, &this->dragInfo.source))
				this->dragInfo.isValid = true;

			SetCursor(this->obj.samplefile);
			SetCapture(GetParent(this->treeView));
			this->draggingUnderway = TRUE;
		}
	}
}

HTREEITEM SampleManager::FindInsertAfter(HTREEITEM parent, int rQtype, iString name)
{
	TCHAR buffer[SMPMNGR_MAX_HEADING_LEN] = { 0 };
	HTREEITEM insertAfter = NULL;
	HTREEITEM child = TreeView_GetChild(this->treeView, parent);
	int nCount = 0;

	if (child == nullptr)
		insertAfter = TVI_FIRST;
	else
	{
		do
		{
			TVITEM tvi;
			tvi.mask = TVIF_IMAGE | TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = child;

			if (TreeView_GetItem(this->treeView, &tvi))
			{
				//int level = LOWORD(tvi.lParam);
				int itemtype = HIWORD(tvi.lParam);

				if ((rQtype == ITEMTYPE_FILTER) && (itemtype == ITEMTYPE_SAMPLE))
				{
					break;
				}
				else if ((rQtype == ITEMTYPE_SAMPLE) && (itemtype == ITEMTYPE_FILTER))
				{
					nCount = 0;
				}
			}
			nCount++;

		} while ((child = TreeView_GetNextSibling(this->treeView, child)) != nullptr);

		if (insertAfter != TVI_FIRST)
		{
			insertAfter = TVI_FIRST;

			child = TreeView_GetChild(this->treeView, parent);
			if (child)
			{
				auto N = name.getContentReference();

				do
				{
					TVITEM tvi;
					tvi.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_HANDLE;
					tvi.hItem = child;
					tvi.pszText = buffer;
					tvi.cchTextMax = SMPMNGR_MAX_HEADING_LEN;

					if (TreeView_GetItem(this->treeView, &tvi))
					{
						//int level = LOWORD(tvi.lParam);
						int itemtype = HIWORD(tvi.lParam);

						if (rQtype == ITEMTYPE_SAMPLE)
						{
							if (itemtype == ITEMTYPE_SAMPLE)
							{
								int cmp = CompareStringsForSort(buffer, N);
								if (cmp == -1)
								{
									break;
								}
							}
						}
						else if (rQtype == ITEMTYPE_FILTER)
						{
							if (itemtype == ITEMTYPE_SAMPLE)
							{
								break;
							}
							else
							{
								int cmp = CompareStringsForSort(buffer, N);
								if (cmp == -1)
								{
									break;
								}
							}
						}
					}
					insertAfter = child;
				} while ((child = TreeView_GetNextSibling(this->treeView, child)) != NULL);
			}
		}
		if (insertAfter == NULL)
			insertAfter = TVI_FIRST;
	}
	return insertAfter;
}

void SampleManager::moveSubItems(HTREEITEM source, HTREEITEM destination)
{
	if ((source == nullptr) || (destination == nullptr))return;
	else
	{
		HTREEITEM item = TreeView_GetChild(this->treeView, source);
		if (item != nullptr)
		{
			TCHAR buffer[SMPMNGR_MAX_HEADING_LEN];

			do
			{
				SecureZeroMemory(buffer, sizeof(buffer));

				TVITEM tvi;
				tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT | TVIF_HANDLE;
				tvi.hItem = item;
				tvi.cchTextMax = SMPMNGR_MAX_HEADING_LEN;
				tvi.pszText = buffer;
				TreeView_GetItem(this->treeView, &tvi);

				BOOL exp = FALSE;
				iString name(buffer);
				int type = HIWORD(tvi.lParam);

				if(type == ITEMTYPE_FILTER)
					exp = TreeView_Expand(this->treeView, item, TVE_COLLAPSE);

				TVINSERTSTRUCT tvins;
				tvins.hInsertAfter = FindInsertAfter(destination, type, name);
				tvins.hParent = destination;
				tvins.item = tvi;

				HTREEITEM newItem = TreeView_InsertItem(this->treeView, &tvins);
				if (newItem != nullptr)
				{
					if (type == ITEMTYPE_FILTER)
					{
						// next level
						this->moveSubItems(item, newItem);

						if (exp)
						{
							TreeView_Expand(this->treeView, newItem, TVE_EXPAND);
						}
					}
				}

			} while ((item = TreeView_GetNextSibling(this->treeView, item)) != nullptr);
		}
	}
}

void SampleManager::updateSelectedItem()
{
	auto item = TreeView_GetSelection(this->treeView);
	if (item)
	{
		this->getItemInfo(item, &this->curSelElement);
	}
}

void SampleManager::setTVFont()
{
	SendMessage(this->treeView, WM_SETFONT, reinterpret_cast<WPARAM>(this->obj.tvFont), static_cast<LPARAM>(TRUE));
}

BOOL SampleManager::isSelectionValid()
{
	auto item = TreeView_GetSelection(this->treeView);
	return (item != nullptr) ? TRUE : FALSE;
}

void SampleManager::_createDpiDependendResources()
{
	auto fontHeight =
		reinterpret_cast<ApplicationData*>(
			getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
		)->getIntegerData(DATAKEY_EXSETTINGS_TREEVIEW_FONTHEIGHT, 16);

	this->obj.font = CreateScaledFont(20, FW_BOLD, APPLICATION_PRIMARY_FONT);

	this->obj.tvFont = CreateScaledFont(fontHeight, FW_MEDIUM, APPLICATION_PRIMARY_FONT);
}

DWORD SampleManager::getDWButtonEnableInfo(int type)
{
	switch (type)
	{
	case ITEMTYPE_FILTER:
		return (DWORD)(0b011011);
	case ITEMTYPE_SAMPLE:
		return (DWORD)(0b111100);
	default:
		return (DWORD)0;
	}
}
