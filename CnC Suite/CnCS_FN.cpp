#include "CnCS_FN.h"
#include "Error dispatcher.h"
#include "ApplicationData.h"
#include "ProgressDialog.h"

CnCS_FN::CnCS_FN(HINSTANCE hInst, HWND MainWindow)
	: hInstance(hInst),
	pTV(nullptr),
	smpManager(nullptr),
	FileSystemEvents(nullptr)
{
	SendMessage(MainWindow, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&this->styleInfo));

	SecureZeroMemory(&this->fnParam, sizeof(FILENAVPARAMS));

	this->_createDpiDependendResources();

	this->fnParam.background = CreateSolidBrush(this->styleInfo.Stylecolor);
	this->fnParam.markBrush = CreateSolidBrush(RGB(140, 140, 140));//CreateSolidBrush(RGB(236, 118, 0));
	this->fnParam.buttonbrush = CreateSolidBrush(RGB(80, 80, 80));
	this->fnParam.toolbarBrush = CreateSolidBrush(this->styleInfo.TabColor);
	this->fnParam.Main = MainWindow;

	this->fnParam.searchico = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_FN_SEARCH), IMAGE_ICON, 18, 18, LR_DEFAULTCOLOR);
	this->fnParam.newfolderico = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_FN_OPENFOLDER), IMAGE_ICON, 18, 18, LR_DEFAULTCOLOR);
}

CnCS_FN::~CnCS_FN()
{
	DeleteObject(this->fnParam.font);
	DeleteObject(this->fnParam.buttonfont);
	DeleteObject(this->fnParam.background);
	DeleteObject(this->fnParam.buttonbrush);
	DeleteObject(this->fnParam.markBrush);
	DeleteObject(this->fnParam.toolbarBrush);

	DestroyIcon(this->fnParam.newfolderico);
	DestroyIcon(this->fnParam.searchico);

	UnhookWindowsHookEx(this->fnParam.hHook);

	SafeRelease(&this->pTV);
}

HRESULT CnCS_FN::_Init(HWND NavFrame, LPTSTR AppDir)
{
	HRESULT hr = (NavFrame == nullptr) ? E_FAIL : S_OK;
	if (SUCCEEDED(hr))
	{
		RECT rc;
		GetClientRect(NavFrame, &rc);

		rc.top = DPIScale(70);
		rc.bottom -= rc.top;

		this->fnParam.Frame = NavFrame;

		hr = SetWindowSubclass(NavFrame, CnCS_FN::FileNavProc, NULL, reinterpret_cast<DWORD_PTR>(this)) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			this->pTV = new TreeViewCTRL(NavFrame, hInstance, GERMAN, AppDir);

			hr = (this->pTV == nullptr) ? E_FAIL : S_OK;
			if (SUCCEEDED(hr))
			{
				APPSTYLEINFO StyleInfo;
				SendMessage(this->fnParam.Main, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&StyleInfo));

				DWORD additionalStyles = WS_BORDER;

				auto aDataC =
					reinterpret_cast<ApplicationData*>(
						getDefaultApplicationDataContainer()
					);

				if (aDataC != nullptr)
				{
					auto usePropAsTooltip = aDataC->getBooleanData(DATAKEY_SETTINGS_USEFILETAGSASTOOLTIP, true);
					if (usePropAsTooltip)
					{
						additionalStyles |= TVS_INFOTIP;
					}
					auto openNewFileByCreation = aDataC->getBooleanData(DATAKEY_SETTINGS_OPENCREATEDFILE, true);
					if (openNewFileByCreation)
					{
						this->pTV->openNewFileEnable(TRUE);
					}
				}

				int fontheight =
					reinterpret_cast<ApplicationData*>(
						getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
					)->getIntegerData(DATAKEY_EXSETTINGS_TREEVIEW_FONTHEIGHT, 16);

				hr = this->pTV->InitTreeView(
					StyleInfo.Stylecolor,
					CreateScaledFont(fontheight, FW_MEDIUM, APPLICATION_PRIMARY_FONT),
					StyleInfo.TextColor,
					&rc,
					additionalStyles
				);

				if (SUCCEEDED(hr))
				{
					this->pTV->setInfoSink(dynamic_cast<ITreeViewUserEvents*>(this));

					hr = this->CreateCtrlButtons();
					if (SUCCEEDED(hr))
					{
						DWORD ThreadId = GetCurrentThreadId();

						this->fnParam.hHook = SetWindowsHookEx(WH_MOUSE, CnCS_FN::MouseProc, NULL, ThreadId);

						hr = (this->fnParam.hHook != NULL) ? S_OK : E_FAIL;
						if (SUCCEEDED(hr))
						{
							SetLastError(0);

							hr = ((SetWindowLongPtr(NavFrame, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this)) == 0)
								&& (GetLastError() != 0)) ? E_FAIL : S_OK;
							if (SUCCEEDED(hr))
							{
								hr = this->CreateSampleManager();
								if (SUCCEEDED(hr))
								{
									hr = this->CreateTBButtons();
									if (SUCCEEDED(hr))
									{
										// ...

										this->startFileSystemWatcher(this->pTV->Root_Folder);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return hr;
}


LRESULT CnCS_FN::FileNavProc(HWND FNWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR refData)
{
	UNREFERENCED_PARAMETER(uID);

	CnCS_FN* pFileNavigator = reinterpret_cast<CnCS_FN*>(refData);

	if (pFileNavigator != nullptr)
	{
		switch (message)
		{
		case WM_NOTIFY:
			return static_cast<LRESULT>(pFileNavigator->pTV->OnTreeViewNotify(lParam));
		case WM_SIZE:
			return pFileNavigator->OnSize();
		case WM_COMMAND:
			return pFileNavigator->OnCommand(FNWnd, wParam, lParam);
		case WM_MOUSEMOVE:
			return pFileNavigator->pTV->OnMouseMove(lParam);
		case WM_LBUTTONUP:
			return pFileNavigator->pTV->OnLButtonUp();
		case WM_ERASEBKGND:
			return pFileNavigator->OnEraseBkgnd(FNWnd, wParam);
		case WM_SETAPPSTYLE:
			return pFileNavigator->OnSetAppStyle(lParam);
		case WM_CLEANUP:
			return pFileNavigator->OnCleanUp(wParam);
		case WM_VALIDATEERROR:
			return pFileNavigator->OnValidateError(lParam);
		case WM_TIMER:
			return pFileNavigator->OnTimer(wParam);
		default:
			break;
		}
	}
	return DefSubclassProc(FNWnd, message, wParam, lParam);
}

LRESULT CnCS_FN::MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode < 0)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);
	else
	{
		LPMOUSEHOOKSTRUCT pmh = reinterpret_cast<LPMOUSEHOOKSTRUCT>(lParam);
		if (pmh != nullptr)
		{
			HWND Main = FindWindow(IDSEX_APPLICATIONCLASS, nullptr);
			if (Main)
			{
				HWND REFWnd_ = GetDlgItem(Main, ID_TVFRAME);

				if (REFWnd_)
				{
					CnCS_FN* pFN =
						reinterpret_cast<CnCS_FN*>(
							GetWindowLongPtr(REFWnd_, GWLP_USERDATA)
							);

					if (pFN != nullptr)
					{
						pFN->pTV->IsDragging_CursorCTRL(&pmh->pt);
					}
				}
			}
		}
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
}

DWORD CnCS_FN::fileSystemWatcher(LPVOID lParam)
{
	DWORD tResult = 0;

	auto tData = reinterpret_cast<LPFILESYSTEMWATCHERTHREADDATA>(lParam);
	if (tData != nullptr)
	{
		auto fileNavigator = reinterpret_cast<CnCS_FN*>(tData->pToClass);
		if (fileNavigator != nullptr)
		{
			iString unicodePath(tData->root_path);
			unicodePath.setUnicodePrefix();

			HANDLE hDir =
				CreateFile(
					unicodePath.GetData(),
					FILE_LIST_DIRECTORY,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					nullptr,
					OPEN_EXISTING,
					FILE_FLAG_OVERLAPPED | FILE_FLAG_BACKUP_SEMANTICS,
					nullptr
				);

			if (hDir != INVALID_HANDLE_VALUE)
			{
				BYTE buffer[2048] = { 0 };

				DWORD waitResult = 0;
				DWORD numBytesTransferred = 0;
				HANDLE hEvent = CreateEvent(nullptr, FALSE, TRUE, nullptr);
				UINT32 interrupt = 0;
				OVERLAPPED ovl;
				ovl.hEvent = hEvent;				

				while (ReadDirectoryChangesW(
					hDir,
					reinterpret_cast<LPVOID>(&buffer),
					2048,
					TRUE,
					FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME,
					nullptr,
					&ovl,
					nullptr
				))
				{
					while (1)
					{
						waitResult = WaitForSingleObject(ovl.hEvent, 500);

						if (waitResult == WAIT_OBJECT_0)
						{
							auto result = GetOverlappedResult(hDir, &ovl, &numBytesTransferred, FALSE);
							if (result)
							{
								switch (ovl.Internal)
								{
								case 0x10C:	//STATUS_NOTIFY_ENUM_DIR
									fileNavigator->pTV->FileSystem_RefreshAllItems();
									break;
								case 0x108: //STATUS_PENDING
									break;
								case 0x0:	//STATUS_SUCCESS								
									fileNavigator->onFileSystemChanged(buffer, 2048);
									SecureZeroMemory(buffer, 2048);
									break;
								default:
									break;
								}
							}
							break;
						}
						else if (waitResult == WAIT_TIMEOUT)
						{
							// validate root-path!
							auto hCheck =
								CreateFile(
									unicodePath.GetData(),
									GENERIC_READ,
									FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
									nullptr,
									OPEN_EXISTING,
									FILE_FLAG_BACKUP_SEMANTICS,
									nullptr
								);
							if (hCheck == INVALID_HANDLE_VALUE)
							{
								fileNavigator->onRootFolderInvalidated();
								interrupt = TRUE;
								break;
							}
							else
							{
								CloseHandle(hCheck);
							}
						}

						// check for interrupt condition...
						interrupt =
							(UINT32)InterlockedCompareExchange(
							(LONG*)&fileNavigator->fnParam.interruptFileSystemWatcher,
								(LONG)FALSE,
								(LONG)TRUE
							);
						if (interrupt)
							break;
					}
					if (interrupt)
						break;
				}
				CloseHandle(hDir);
			}
			else
			{
				fileNavigator->onRootFolderInvalidated();
				tResult = 2;
			}
			SafeDeleteArray((void**)&tData->root_path);
		}
		delete tData;
	}
	return tResult;
}

LRESULT CnCS_FN::OnSize()
{
	RECT rc;
	HDWP defWindowPos;
	GetClientRect(this->fnParam.Frame, &rc);

	// at first, update toolbar to set up the start position of the treeview
	this->resizeToolbar();

	// position for treeview
	int TV_yPos = DPIScale(70) + (this->fnParam.toolbarDualLine * DPIScale(34));
	int TV_height = rc.bottom - TV_yPos;

	defWindowPos = BeginDeferWindowPos(4);

	if (defWindowPos != NULL)
	{
		defWindowPos = DeferWindowPos(
			defWindowPos,
			this->pTV->GetTreeViewHandle(),
			NULL,
			0, TV_yPos, rc.right, TV_height,
			SWP_NOZORDER
		);

		auto smpMngr = GetDlgItem(this->fnParam.Frame, ID_SAMPLEMANAGER);
		if (smpMngr != nullptr)
		{
			rc.top = DPIScale(30);
			rc.bottom -= DPIScale(30);

			if (defWindowPos != NULL)
			{
				defWindowPos = DeferWindowPos(
					defWindowPos,
					smpMngr,
					NULL,
					0, rc.top, rc.right, rc.bottom,
					SWP_NOZORDER
				);
			}
		}

		if (defWindowPos != NULL)
		{
			defWindowPos = DeferWindowPos(
				defWindowPos,
				GetDlgItem(this->fnParam.Frame, ID_EXPLORER),
				NULL,
				0, 0, rc.right / 2, DPIScale(25),
				SWP_NOZORDER
			);

			if (defWindowPos != NULL)
			{
				defWindowPos = DeferWindowPos(
					defWindowPos,
					GetDlgItem(this->fnParam.Frame, ID_TEMPLATE),
					NULL,
					rc.right / 2, 0, rc.right - (rc.right / 2), DPIScale(25),
					SWP_NOZORDER
				);

				EndDeferWindowPos(defWindowPos);
			}
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_FN::OnConvert(LPARAM lParam)
{
	BOOL rAction = IDCANCEL;

	Open_Save_CTRL* osc = new Open_Save_CTRL();
	if (osc != NULL)
	{
		rAction = osc->ConvertFileToCnc3(this->fnParam.Main, reinterpret_cast<LPTSTR>(lParam));
		if (rAction != IDCANCEL)
		{
			_FWD_MSG_TO_MAIN(MAKEWPARAM(NAVIGATOR_FILEWASCONVERTED, 0), lParam);
		}
		delete osc;
	}
	return static_cast<LRESULT>(rAction);
}

LRESULT CnCS_FN::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case TV_CTRL_SETCURFOLDER:	
		_FWD_MSG_TO_MAIN(
			MAKEWPARAM(NAVIGATOR_SETCURRENTFOLDER,0),
			lParam
		);
		break;
	case TV_CTRL_OPENPATH:
		_FWD_MSG_TO_MAIN(
			MAKEWPARAM(NAVIGATOR_RQ_OPENPATH, HIWORD(wParam)),
			lParam
		);
		break;
	case TV_CTRL_CONVERTTOCNC3:
		return this->OnConvert(lParam);
	case TV_CTRL_IMPORTFILE:
		_FWD_MSG_TO_MAIN(
			MAKEWPARAM(NAVIGATOR_RQ_IMPORTFILE, 0),
			lParam
		);
		break;
	case TV_CTRL_CREATETOOLWND:
		this->OnCreatePopupMenu(HIWORD(wParam));
		break;
	case ID_SLNEWROOTFOLDER:
		this->SelectNewRootFolder();
		break;
	case ID_NEWFOLDER:
		this->InsertNewFolder();
		break;
	case ID_NEWPROGRAMM:
		this->InsertNewProgram();
		break;
	case ID_COPY:
		this->OnCopyItem();
		break;
	case ID_INSERT:
		this->OnInsertItem();
		break;
	case ID_CONVERT:
		this->OnConvertItem();
		break;
	case ID_DELETE:
		this->OnDeleteItem();
		break;
	case ID_IMPORT:
		this->OnImportItem();
		break;
	case ID_RENAME:
		this->OnRenameItem();
		break;
	case ID_SEARCH:
		this->StartSearch();
		break;
	case ID_OPEN:
		this->OnOpenItem(false);
		break;
	case ID_OPEN_IN_NEW_TAB:
		this->OnOpenItem(true);
		break;
	case ID_DATACOLLECTIONSTARTED:
		this->onTVItemDataCollectionStarted();
		break;
	case ID_DATAINITIALIZATIONSTARTED:
		this->onTVItemInitalizingStarted(static_cast<int>(lParam));
		break;
	case ICOMMAND_TVREFRESH:
		this->ReloadAsync();
		break;
	case ICOMMAND_TVRECOVERSCROLL:
		this->pTV->SetScrollPosition(nullptr);
		break;
	default:
		return DefSubclassProc(hWnd, WM_COMMAND, wParam, lParam);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_FN::OnEraseBkgnd(HWND hWnd, WPARAM wParam)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	auto bottom = rc.bottom;

	rc.top = DPIScale(24);
	rc.bottom = DPIScale(30);

	FillRect(
		reinterpret_cast<HDC>(wParam),
		&rc,
		this->fnParam.background
	);

	rc.top = DPIScale(30);

	if (IsWindowVisible(this->pTV->GetTreeViewHandle()))
	{
		// if the treeview is visible -> exclude the treeview-area from the update-area
		rc.bottom = DPIScale(75) + (this->fnParam.toolbarDualLine * DPIScale(34));
	}
	else
	{
		// otherwise -> update all!
		rc.bottom = bottom;
	}

	FillRect(
		reinterpret_cast<HDC>(wParam),
		&rc,
		this->fnParam.toolbarBrush
	);

	return static_cast<LRESULT>(TRUE);
}

LRESULT CnCS_FN::OnCleanUp(WPARAM wParam)
{
	switch(wParam)
	{ 
	case CU_DELETESEARCHCLASS:
		SafeRelease(&this->pSRCH);
		break;
	default:
		break;
	
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_FN::OnSetAppStyle(LPARAM lParam)
{
	BOOL result = TRUE;

	LPAPPSTYLEINFO pStyleInfo = reinterpret_cast<LPAPPSTYLEINFO>(lParam);
	if (pStyleInfo != NULL)
	{
		// copy styleinfo
		copyAppStyleInfo(pStyleInfo, &this->styleInfo);

		// update treeview
		if(this->pTV != nullptr)
			this->pTV->Recolor(pStyleInfo->TextColor, pStyleInfo->Stylecolor);

		// update sample-manager
		auto smpMngr = GetDlgItem(this->fnParam.Frame, ID_SAMPLEMANAGER);
		if (smpMngr != nullptr)
			SendMessage(smpMngr, WM_SETAPPSTYLE, 0, lParam);

		// recreate objects
		DeleteObject(this->fnParam.background);
		this->fnParam.background = CreateSolidBrush(pStyleInfo->Stylecolor);
		DeleteObject(this->fnParam.toolbarBrush);
		this->fnParam.toolbarBrush = CreateSolidBrush(pStyleInfo->TabColor);

		// update tab-buttons
		auto expButton =
			reinterpret_cast<CustomButton*>(
				CUSTOMBUTTONCLASS_FROMID(this->fnParam.Frame, ID_EXPLORER)
				);
		if (expButton != nullptr)
		{
			expButton->setColors(
				this->styleInfo.Stylecolor,
				(this->styleInfo.StyleID == STYLEID_BLACK) ? brightenColor(this->styleInfo.Stylecolor, 30) : makeSelectionColor(this->styleInfo.Stylecolor),
				this->styleInfo.Stylecolor
			);
			expButton->setTextColor(this->styleInfo.TextColor);
			expButton->setBorder(TRUE, this->styleInfo.OutlineColor);
		}

		auto tmpButton =
			reinterpret_cast<CustomButton*>(
				CUSTOMBUTTONCLASS_FROMID(this->fnParam.Frame, ID_TEMPLATE)
				);
		if (tmpButton != nullptr)
		{
			tmpButton->setColors(
				this->styleInfo.Stylecolor,
				(this->styleInfo.StyleID == STYLEID_BLACK) ? brightenColor(this->styleInfo.Stylecolor, 30) : makeSelectionColor(this->styleInfo.Stylecolor),
				this->styleInfo.Stylecolor
			);
			tmpButton->setTextColor(this->styleInfo.TextColor);
			expButton->setBorder(TRUE, this->styleInfo.OutlineColor);
		}

		// update toolbar
		for (int i = 0; i < MAX_TBBUTTON; i++)
		{
			auto hWndButton = GetDlgItem(this->fnParam.Frame, ID_SLNEWROOTFOLDER + i);
			if (hWndButton != nullptr)
			{
				auto button =
					reinterpret_cast<CustomButton*>(
						CUSTOMBUTTONCLASS_FROMID(this->fnParam.Frame, ID_SLNEWROOTFOLDER + i)
					);

				if (button != nullptr)
				{
					button->setBorder(TRUE, this->styleInfo.OutlineColor);
					button->setColors(
						this->styleInfo.mainToolbarColor,
						makeSelectionColor(this->styleInfo.mainToolbarColor),
						makePressedColor(this->styleInfo.mainToolbarColor)
					);
					if (this->styleInfo.StyleID == STYLEID_LIGHTGRAY)
					{
						button->setDisabledColor(
							darkenColor(this->styleInfo.mainToolbarColor, 30)
						);
					}
					else
					{
						button->setDisabledColor(RGB(100, 100, 100));
					}
				}
			}
		}
		// redraw all
		RedrawWindow(this->fnParam.Frame, nullptr, nullptr, RDW_NOCHILDREN | RDW_INVALIDATE | RDW_ERASE);
	}
	else
		result = FALSE;

	return static_cast<LRESULT>(result);
}

LRESULT CnCS_FN::OnValidateError(LPARAM lParam)
{
	BOOL ncsDelete = FALSE;

	LPEDSPSTRUCT pedsp = reinterpret_cast<LPEDSPSTRUCT>(lParam);
	if (pedsp != NULL)
	{
		if (pedsp->type == EDSP_INFO)
			ncsDelete = FALSE;
		else
		{

		}
	}
	return static_cast<LRESULT>(ncsDelete);
}

LRESULT CnCS_FN::OnTimer(WPARAM wParam)
{
	switch (wParam)
	{
	case ID_PROGRESSDIALOG_SHUTDOWMTIMER:
		if (this->fnParam.dataCollectionComplete)
		{
			auto pDlg = ProgressDialog::getOpenInstance();
			if (pDlg != nullptr)
			{
				pDlg->Close();
				KillTimer(this->fnParam.Frame, ID_PROGRESSDIALOG_SHUTDOWMTIMER);
				this->fnParam.dataCollectionComplete = FALSE;
			}
		}
		else
		{
			KillTimer(this->fnParam.Frame, ID_PROGRESSDIALOG_SHUTDOWMTIMER);
		}
		break;
	case ID_TVDRAGSCROLL_TIMER:
		this->pTV->OnTreeViewTimer(wParam);
		break;
	default:
		break;
	}
	return static_cast<LRESULT>(0);
}

void CnCS_FN::OnCreatePopupMenu(int type)
{
	POINT pt;

	GetCursorPos(&pt);

	bool alwaysOpenInNewTAb = false;

	auto dataContainer =
		reinterpret_cast<ApplicationData*>(
			getDefaultApplicationDataContainer()
			);
	if (dataContainer != nullptr)
	{
		alwaysOpenInNewTAb = dataContainer->getBooleanData(DATAKEY_SETTINGS_OPEN_IN_NEW_TAB, true);
	}

	auto menu =
		new CustomPopUpMenu(
			this->hInstance,
			dynamic_cast<customPopUpMenuEventSink*>(this)
		);
	if (menu != nullptr)
	{
		menu->setControlFont(
			CreateScaledFont(16, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
		);

		MenuEntry entry;

		switch (type)
		{
		case PARENT__ITEM:
			this->_defineMenuEntry(
				entry,
				ID_NEWPROGRAMM,
				IDI_EXP_ADDFILE,
				DPIScale(18),
				getStringFromResource(UI_GNRL_NEWCNCFILE)
			);
			menu->addMenuEntry(&entry);

			this->_defineMenuEntry(
				entry,
				ID_NEWFOLDER,
				IDI_EXP_ADDFOLDER,
				DPIScale(18),
				getStringFromResource(UI_GNRL_NEWSUBFOLDER)
			);
			menu->addMenuEntry(&entry);

			this->_defineMenuEntry(
				entry,
				ID_COPY,
				IDI_EXP_COPYELEMENT,
				DPIScale(18),
				getStringFromResource(UI_GNRL_COPYFOLDER)
			);
			menu->addMenuEntry(&entry);

			if (this->pTV->canPasteObject())
			{
				this->_defineMenuEntry(
					entry,
					ID_INSERT,
					IDI_EXP_INSERTELEMENT,
					DPIScale(18),
					getStringFromResource(UI_GNRL_PASTEFILE)
				);
				menu->addMenuEntry(&entry);
			}

			this->_defineMenuEntry(
				entry,
				ID_RENAME,
				IDI_EXP_RENAMEFILE,
				DPIScale(18),
				getStringFromResource(UI_GNRL_RENAME)
			);
			menu->addMenuEntry(&entry);

			this->_defineMenuEntry(
				entry,
				ID_DELETE,
				IDI_EXP_DELETEELEMENT,
				DPIScale(18),
				getStringFromResource(UI_GNRL_DELETE)
			);
			menu->addMenuEntry(&entry);
			break;

		case CHILD__ITEM:

			this->_defineMenuEntry(
				entry,
				ID_CONVERT,
				IDI_EXP_CONVERTELEMENT,
				DPIScale(18),
				getStringFromResource(UI_GNRL_CONVERTTOCNC3)
			);
			menu->addMenuEntry(&entry);

			this->_defineMenuEntry(
				entry,
				ID_IMPORT,
				IDI_EXP_IMPORTCONTENT,
				DPIScale(18),
				getStringFromResource(UI_GNRL_IMPORTCONTENT)
			);
			menu->addMenuEntry(&entry);

			this->_defineMenuEntry(
				entry,
				ID_DELETE,
				IDI_EXP_DELETEELEMENT,
				DPIScale(18),
				getStringFromResource(UI_GNRL_DELETE)
			);
			menu->addMenuEntry(&entry);
			break;

		case CNC3__ITEM:

			this->_defineMenuEntry(
				entry,
				ID_OPEN,
				IDI_TOOLBAR_OPEN,
				DPIScale(18),
				getStringFromResource(UI_GNRL_PLAIN_OPEN)
			);
			menu->addMenuEntry(&entry);

			if (!alwaysOpenInNewTAb)
			{
				this->_defineMenuEntry(
					entry,
					ID_OPEN_IN_NEW_TAB,
					IDI_TOOLBAR_OPEN,
					DPIScale(18),
					getStringFromResource(UI_GNRL_OPENINNEWTAB)
				);
				menu->addMenuEntry(&entry);
			}

			this->_defineMenuEntry(
				entry,
				ID_COPY,
				IDI_EXP_COPYELEMENT,
				DPIScale(18),
				getStringFromResource(UI_GNRL_COPYFILE)
			);
			menu->addMenuEntry(&entry);

			if (this->pTV->canPasteObject())
			{
				this->_defineMenuEntry(
					entry,
					ID_INSERT,
					IDI_EXP_INSERTELEMENT,
					DPIScale(18),
					getStringFromResource(UI_GNRL_PASTEFILE)
				);
				menu->addMenuEntry(&entry);
			}

			this->_defineMenuEntry(
				entry,
				ID_RENAME,
				IDI_EXP_RENAMEFILE,
				DPIScale(18),
				getStringFromResource(UI_GNRL_RENAME)
			);
			menu->addMenuEntry(&entry);

			this->_defineMenuEntry(
				entry,
				ID_DELETE,
				IDI_EXP_DELETEELEMENT,
				DPIScale(18),
				getStringFromResource(UI_GNRL_DELETE)
			);
			menu->addMenuEntry(&entry);
			break;

		case EMPTY__ITEM:

			this->_defineMenuEntry(
				entry,
				ID_NEWPROGRAMM,
				IDI_EXP_ADDFILE,
				DPIScale(18),
				getStringFromResource(UI_GNRL_NEWCNCFILE)
			);
			menu->addMenuEntry(&entry);

			this->_defineMenuEntry(
				entry,
				ID_NEWFOLDER,
				IDI_EXP_ADDFOLDER,
				DPIScale(18),
				getStringFromResource(UI_GNRL_NEWSUBFOLDER)
			);
			menu->addMenuEntry(&entry);
			break;

		case ROOT__ITEM:

			this->_defineMenuEntry(
				entry,
				ID_SLNEWROOTFOLDER,
				IDI_EXP_SELECTROOT,
				DPIScale(18),
				getStringFromResource(UI_GNRL_CHANGEROOTDIR)
			);
			menu->addMenuEntry(&entry);

			this->_defineMenuEntry(
				entry,
				ID_NEWPROGRAMM,
				IDI_EXP_ADDFILE,
				DPIScale(18),
				getStringFromResource(UI_GNRL_NEWCNCFILE)
			);
			menu->addMenuEntry(&entry);

			this->_defineMenuEntry(
				entry,
				ID_NEWFOLDER,
				IDI_EXP_ADDFOLDER,
				DPIScale(18),
				getStringFromResource(UI_GNRL_NEWSUBFOLDER)
			);
			menu->addMenuEntry(&entry);

			this->_defineMenuEntry(
				entry,
				ID_COPY,
				IDI_EXP_COPYELEMENT,
				DPIScale(18),
				getStringFromResource(UI_GNRL_COPYFOLDER)
			);
			menu->addMenuEntry(&entry);

			if (this->pTV->canPasteObject())
			{
				this->_defineMenuEntry(
					entry,
					ID_INSERT,
					IDI_EXP_INSERTELEMENT,
					DPIScale(18),
					getStringFromResource(UI_GNRL_PASTEFILE)
				);
				menu->addMenuEntry(&entry);
			}

			this->_defineMenuEntry(
				entry,
				ID_RENAME,
				IDI_EXP_RENAMEFILE,
				DPIScale(18),
				getStringFromResource(UI_GNRL_RENAME)
			);
			menu->addMenuEntry(&entry);

			this->_defineMenuEntry(
				entry,
				ID_DELETE,
				IDI_EXP_DELETEELEMENT,
				DPIScale(18),
				getStringFromResource(UI_GNRL_DELETE)
			);
			menu->addMenuEntry(&entry);

			break;
		default:
			break;
		}

		menu->Show(
			this->pTV->GetTreeViewHandle(),
			&pt,
			DPIScale(250),
			this->styleInfo.MenuPopUpColor
		);
	}
}

HRESULT CnCS_FN::CreateCtrlButtons()
{
	RECT rc;
	HRESULT hr = S_OK;
	GetClientRect(this->fnParam.Frame, &rc);

	POINT pt;
	pt.x = 0;
	pt.y = 0;
	SIZE sz;
	sz.cx = rc.right / 2;
	sz.cy = DPIScale(25);

	iString buttonText(
		getStringFromResource(UI_NAV_EXPLORERBUTTON)
	);

	auto expButton = new CustomButton(this->fnParam.Frame, BUTTONMODE_TEXT, &pt, &sz, ID_EXPLORER, this->hInstance);
	if (expButton != nullptr)
	{
		expButton->setAppearance_onlyText(buttonText, FALSE);
		expButton->setColors(
			this->styleInfo.Stylecolor,
			(this->styleInfo.StyleID == STYLEID_BLACK) ? brightenColor(this->styleInfo.Stylecolor, 30) : makeSelectionColor(this->styleInfo.Stylecolor),
				this->styleInfo.Stylecolor
				);
		expButton->setTextColor(this->styleInfo.TextColor);
		expButton->setBorder(TRUE, this->styleInfo.OutlineColor);
		expButton->setMouseAnimation(false);
		expButton->setPartialBorderParts(false, false, true, false);
		expButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
		expButton->setFont(
			CreateScaledFont(18, FW_BOLD, APPLICATION_SECONDARY_FONT)
		);

		hr = expButton->Create();
		if (SUCCEEDED(hr))
		{
			pt.x = rc.right / 2;
			pt.y = 0;
			sz.cx = rc.right - pt.x;
			sz.cy = DPIScale(25);

			buttonText = getStringFromResource(UI_NAV_TEMPLATEBUTTON);

			auto tmpButton = new CustomButton(this->fnParam.Frame, BUTTONMODE_TEXT, &pt, &sz, ID_TEMPLATE, this->hInstance);
			if (tmpButton != nullptr)
			{
				tmpButton->setAppearance_onlyText(buttonText, FALSE);
				tmpButton->setColors(
					this->styleInfo.Stylecolor,
					(this->styleInfo.StyleID == STYLEID_BLACK) ? brightenColor(this->styleInfo.Stylecolor, 30) : makeSelectionColor(this->styleInfo.Stylecolor),
					this->styleInfo.Stylecolor
				);
				tmpButton->setTextColor(this->styleInfo.TextColor);
				tmpButton->setBorder(TRUE, this->styleInfo.OutlineColor);
				tmpButton->setPartialBorderParts(true, false, false, true);
				tmpButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
				tmpButton->setFont(
					CreateScaledFont(18, FW_BOLD, APPLICATION_SECONDARY_FONT)
				);

				hr = tmpButton->Create();
				if (SUCCEEDED(hr))
				{

				}
			}
		}
	}

	return hr;
}

HRESULT CnCS_FN::CreateSampleManager()
{
	HRESULT hr;

	AppPath aPath;
	auto strPath = aPath.Get(PATHID_FOLDER_CNCSUITE_SAMPLES);

	this->smpManager =
		new SampleManager(
			this->fnParam.Frame,
			this->fnParam.Main,
			strPath.GetData(),
			this->hInstance
		);

	hr = (this->smpManager != nullptr) ? S_OK : E_FAIL;
	{
		RECT rc;
		GetClientRect(
			this->fnParam.Frame, &rc);

		rc.top = DPIScale(30);
		rc.bottom -= DPIScale(30);

		hr = this->smpManager->Init(&rc, SW_HIDE);
		if (SUCCEEDED(hr))
		{
			// ...
		}
	}	
	return hr;
}

HRESULT CnCS_FN::CreateTBButtons()
{
	HRESULT hr = E_FAIL;

	RECT rcFrame;
	GetClientRect(this->fnParam.Frame, &rcFrame);

	POINT pos;
	SIZE size = { DPIScale(32), DPIScale(32) };

	pos.x = DPIScale(2);
	pos.y = DPIScale(35);

	for (int i = 0; i < MAX_TBBUTTON; i++)
	{
		auto btn =
			new CustomButton(
				this->fnParam.Frame,
				BUTTONMODE_ICON,
				&pos,
				&size,
				ID_SLNEWROOTFOLDER + i,
				this->hInstance
			);

		hr = (btn != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			iString tooltip(
				getStringFromResource(UI_EXPLORER_TOOLTIP_LOADROOTFOLDER + i)
			);

			btn->setAppearance_onlyIcon(IDI_EXP_SELECTROOT + i, DPIScale(30));
			btn->setDisabledIcon(IDI_EXP_SELECTROOT_DISABLED + i);
			btn->setColors(
				this->styleInfo.mainToolbarColor,
				makeSelectionColor(this->styleInfo.mainToolbarColor),//RGB(130,130,130),
				makePressedColor(this->styleInfo.mainToolbarColor)//RGB(90,90,90)
			);
			btn->setBorder(TRUE, this->styleInfo.OutlineColor);
			btn->setEventListener(dynamic_cast<customButtonEventSink*>(this));
			btn->setTooltip(tooltip);

			if (this->styleInfo.StyleID == STYLEID_LIGHTGRAY)
			{
				btn->setDisabledColor(
					darkenColor(this->styleInfo.mainToolbarColor, 30)
				);
			}
			hr = btn->Create();
		}
		
		if(FAILED(hr))
			break;

		pos.x += DPIScale(34);

		if ((pos.x + DPIScale(34)) > rcFrame.right)
		{
			if (pos.y < (rcFrame.bottom - DPIScale(50)))
			{
				pos.y += DPIScale(34);
				pos.x = DPIScale(2);
				this->fnParam.toolbarDualLine++;
			}
		}
	}
	if (this->fnParam.toolbarDualLine)
		this->OnSize();

	return hr;
}

BOOL CnCS_FN::StartSearch()
{
	TCHAR* userPath = NULL;

	BasicFPO* pbf = CreateBasicFPO();
	if (pbf != NULL)
	{
		if (pbf->GetKnownFolderPath(&userPath, FOLDERID_Documents))
		{
			TCHAR* Wdir = NULL;

			if (AppendStringToString(userPath, L"\\CnC Suite\\AppData\0", &Wdir))
			{
				this->pSRCH = new Searchcontrol(
					this->hInstance,
					this->fnParam.Main,
					this->fnParam.Frame, Wdir,
					this->pTV->Root_Folder,
					getCurrentAppLanguage()	);

				if (this->pSRCH != NULL)
				{
					DESCRIPTIONINFO dInfo;
					SecureZeroMemory(&dInfo, sizeof(DESCRIPTIONINFO));

					auto result = (BOOL)SendMessage(this->fnParam.Main, WM_GETDESCRIPTIONS, 0, reinterpret_cast<LPARAM>(&dInfo));
					if (result)
					{
						pSRCH->SetDescriptions(dInfo.desc1, dInfo.desc2, dInfo.desc3);

						SafeDeleteArray(&dInfo.desc1);
						SafeDeleteArray(&dInfo.desc2);
						SafeDeleteArray(&dInfo.desc3);
					}

					HRESULT hr = this->pSRCH->InitSearchWindow();
					if (SUCCEEDED(hr))
					{
						EnableWindow(this->fnParam.Main, FALSE);
					}
				}
				SafeDeleteArray(&Wdir);
			}
			SafeDeleteArray(&userPath);
		}
		SafeRelease(&pbf);
	}
	return TRUE;
}

void CnCS_FN::addNewFileToView(LPTSTR path, BOOL expand)
{
	this->pTV->InsertExistingFileToView(path);

	if (expand)
	{
		this->pTV->ExpandPathToItem(path);
	}
}

void CnCS_FN::startFileSystemWatcher(LPCTSTR root)
{
	LPFILESYSTEMWATCHERTHREADDATA tData = new FILESYSTEMWATCHERTHREADDATA;
	if (tData != nullptr)
	{
		tData->pToClass = reinterpret_cast<LONG_PTR>(this);
		tData->root_path = nullptr;

		if (CopyStringToPtr(root, &tData->root_path) == TRUE)
		{
			HANDLE hThread;
			DWORD dwThreadID;

			hThread =
				CreateThread(
					nullptr,
					0,
					CnCS_FN::fileSystemWatcher,
					reinterpret_cast<LPVOID>(tData),
					0,
					&dwThreadID
				);

			if (hThread)
			{
				// ...
			}
		}
	}
}

void CnCS_FN::SelectNewRootFolder()
{
	Open_Save_CTRL* osc = new Open_Save_CTRL();
	if (osc != NULL)
	{
		WCHAR* path = NULL;

		HRESULT hr = osc->OpenFolder(this->fnParam.Frame, &path);
		if (SUCCEEDED(hr))
		{
			// Validate new root folder!
			auto exec = isDrive(path);

			if (!(reinterpret_cast<ApplicationData*>(
				getApplicationDataContainerFromFilekey(FILEKEY_INTERNAL_SETTINGS)
				)->getBooleanData(DATAKEY_INTSET_BLOCKDRIVELOADING, true)))
			{
				if (exec == TRUE)
					exec = FALSE;
			}
			
			if (!exec)
			{
				this->ClearCondition();
				this->fnParam.interruptFileSystemWatcher = (UINT32)TRUE;

				this->pTV->setEventListener(
					dynamic_cast<ITreeViewItemLoadingProgress*>(this)
				);
				this->pTV->InitTreeViewItemsAsync(path);
			}
			else
			{
				DispatchEWINotification(
					EDSP_ERROR,
					L"FN0006",
					getStringFromResource(ERROR_MSG_DRIVELOADINGNOTALLOWED),
					getStringFromResource(UI_FILENAVIGATOR)
				);
			}
			SafeDeleteArray(&path);
		}
		else
		{
			// get and dispatch the error
			iString errorCode(L"FD");
			TCHAR* errorMessage = nullptr;
			auto _hr = osc->GetTranslated_hResultErrorCode(&errorMessage);
			int code = (int)LOWORD(_hr);
			errorCode += code;

			DispatchEWINotification(EDSP_ERROR, errorCode.GetData(), errorMessage, L"System File Dialog");

			SafeDeleteArray(&errorMessage);
		}
		delete osc;
	}
}

void CnCS_FN::InsertNewProgram()
{
	this->pTV->PerformFileOperation(FOP_NEWFILE);
}

void CnCS_FN::InsertNewFolder()
{
	this->pTV->PerformFileOperation(FOP_NEWFOLDER);
}

void CnCS_FN::OnCopyItem()
{
	this->pTV->PerformFileOperation(FOP_COPY);
}

void CnCS_FN::OnInsertItem()
{
	this->pTV->PerformFileOperation(FOP_INSERT);
}

void CnCS_FN::OnConvertItem()
{
	this->pTV->PerformFileOperation(FOP_CONVERT);
}

void CnCS_FN::OnRenameItem()
{
	this->pTV->PerformFileOperation(FOP_RENAME);
}

void CnCS_FN::OnImportItem()
{
	this->pTV->PerformFileOperation(FOP_IMPORT);
}

void CnCS_FN::OnDeleteItem()
{
	this->pTV->PerformFileOperation(FOP_DELETE);
}

void CnCS_FN::OnOpenItem(bool openInNewTab)
{
	TCHAR* path = nullptr;

	auto result = this->pTV->GetSelectedItemPath(&path);
	if (result)
	{
		_FWD_MSG_TO_MAIN(
			MAKEWPARAM(
				NAVIGATOR_RQ_OPENPATH,
				openInNewTab ? FORCE_OPEN_IN_NEW_TAB : 0
			),
			reinterpret_cast<LPARAM>(path)
		);
	}
}

void CnCS_FN::OnDpiChanged()
{
	_createDpiDependendResources();

	if (this->pTV != nullptr)
	{
		this->pTV->updateFont(
			CreateScaledFont(16, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
		);
		this->pTV->updateImageList();
	}
	if (this->smpManager != nullptr)
	{
		this->smpManager->onDpiChanged();
	}
	auto expButton =
		reinterpret_cast<CustomButton*>(
			CUSTOMBUTTONCLASS_FROMID(this->fnParam.Frame, ID_EXPLORER)
			);
	if (expButton != nullptr)
	{
		expButton->setFont(
			CreateScaledFont(18, FW_BOLD, APPLICATION_SECONDARY_FONT)
		);
	}
	auto tmpButton =
		reinterpret_cast<CustomButton*>(
			CUSTOMBUTTONCLASS_FROMID(this->fnParam.Frame, ID_TEMPLATE)
			);
	if (tmpButton != nullptr)
	{
		tmpButton->setFont(
			CreateScaledFont(18, FW_BOLD, APPLICATION_SECONDARY_FONT)
		);
	}
}

void CnCS_FN::onEnableInfoTip(BOOL _enable)
{
	this->fnParam.UseInfoTip = _enable;
	this->pTV->infoTipEnable(_enable);
}

void CnCS_FN::onEnableNewFileOpening(BOOL _enable)
{
	this->pTV->openNewFileEnable(_enable);
}

void CnCS_FN::_expandPath(LPCTSTR path)
{
	this->pTV->ExpandPathToItem(path);
}

void CnCS_FN::hideSampleManager()
{
	auto smngr = GetDlgItem(this->fnParam.Frame, ID_SAMPLEMANAGER);
	if (smngr)
	{
		ShowWindow(smngr, SW_HIDE);
	}
}

void CnCS_FN::showSampleManager()
{
	auto smngr = GetDlgItem(this->fnParam.Frame, ID_SAMPLEMANAGER);
	if (smngr)
	{
		ShowWindow(smngr, SW_SHOW);
		UpdateWindow(smngr);
	}
}

void CnCS_FN::resizeToolbar()
{
	RECT rcFrame;
	GetClientRect(this->fnParam.Frame, &rcFrame);

	this->fnParam.toolbarDualLine = FALSE;

	POINT pos = { DPIScale(2), DPIScale(35) };
	SIZE sz = { DPIScale(32), DPIScale(32) };

	for (int i = 0; i < MAX_TBBUTTON; i++)
	{
		auto button =
			reinterpret_cast<CustomButton*>(
				CUSTOMBUTTONCLASS_FROMID(this->fnParam.Frame, ID_SLNEWROOTFOLDER + i)
				);
		if (button != nullptr)
		{
			button->setAppearance_onlyIcon(IDI_EXP_SELECTROOT + i, DPIScale(30));
			button->updateDimensions(&pos, &sz);
		}
		pos.x += DPIScale(34);

		if ((pos.x + DPIScale(34)) > rcFrame.right)
		{
			if (pos.y < (rcFrame.bottom - DPIScale(50)))
			{
				pos.x = DPIScale(2);
				pos.y += DPIScale(34);

				if(i < (MAX_TBBUTTON - 1))
					this->fnParam.toolbarDualLine++;
			}
		}
	}
}

void CnCS_FN::onTVItemDataCollectionStarted()
{
	this->fnParam.dataCollectionComplete = FALSE;

	auto pDlg = new ProgressDialog();
	if (pDlg != nullptr)
	{
		APPSTYLEINFO StyleInfo;
		SendMessage(this->fnParam.Main, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&StyleInfo));

		iString dspMsg(
			getStringFromResource(UI_PROGRESS_COLLECTINGDATA)
		);

		pDlg->setDisplayMessage(dspMsg);
		pDlg->disableOwnerUntilFinish(true);
		pDlg->setBackgroundColor(StyleInfo.ToolbarbuttonBkgnd);
		pDlg->setType(
			ProgressDialog::PROGRESSTYPE_POPUP |
			ProgressDialog::PROGRESSTYPE_WORKER |
			ProgressDialog::PROGRESSTYPE_BORDER
		);

		pDlg->Create(this->fnParam.Main, this->hInstance);
	}
}

void CnCS_FN::onTVItemDataCollectionFinished(int nItems)
{
	this->pTV->SaveRootFolderSelection();
	this->startFileSystemWatcher(this->pTV->Root_Folder);

	auto pDlg = ProgressDialog::getOpenInstance();
	if (pDlg != nullptr)
	{
		if (nItems <= 2000)
			pDlg->FinishAndClose();
		else
			pDlg->Close();
	}
	else
	{
		this->fnParam.dataCollectionComplete = TRUE;
		SetTimer(this->fnParam.Frame, ID_PROGRESSDIALOG_SHUTDOWMTIMER, 1000, nullptr);
	}
}

void CnCS_FN::onTVItemInitalizingStarted(int nItems)
{
	if (nItems > 2000)
	{
		auto pDlg = new ProgressDialog();
		if (pDlg != nullptr)
		{
			APPSTYLEINFO StyleInfo;
			SendMessage(this->fnParam.Main, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&StyleInfo));

			iString dspMsg(
				getStringFromResource(UI_PROGRESS_LOADINGDATA)
			);
			pDlg->setDisplayMessage(dspMsg);
			pDlg->disableOwnerUntilFinish(true);
			pDlg->setBackgroundColor(StyleInfo.ToolbarbuttonBkgnd);
			pDlg->setType(
				ProgressDialog::PROGRESSTYPE_POPUP |
				ProgressDialog::PROGRESSTYPE_PROGRESS |
				ProgressDialog::PROGRESSTYPE_BORDER |
				ProgressDialog::PROGRESSTYPE_ANIMATE
			);
			pDlg->setAbsoluteRange(nItems * 2);
			pDlg->Create(this->fnParam.Main, this->hInstance);
		}
	}
	else
		this->pTV->setEventListener(nullptr);
}

void CnCS_FN::onTVItemAdded()
{
	auto pDlg = ProgressDialog::getOpenInstance();
	if (pDlg != nullptr)
	{
		pDlg->stepRange();
	}
}

void CnCS_FN::onTVOperationComplete()
{
	auto pDlg = ProgressDialog::getOpenInstance();
	if (pDlg != nullptr)
	{
		pDlg->Close();
	}
	this->pTV->setEventListener(nullptr);
}

void CnCS_FN::onTVOperationFailed()
{
	Sleep(400);// if the operation will fail before the dialog is created, the dialog will be created and cannot be destroyed -> so wait 400ms to make sure the dialog is created!

	auto pDlg = ProgressDialog::getOpenInstance();
	if (pDlg != nullptr)
	{
		pDlg->Close();
	}
	this->pTV->setEventListener(nullptr);

	DispatchEWINotification(
		EDSP_ERROR,
		L"FN0005",
		getStringFromResource(ERROR_MSG_ROOTFOLDERLOADINGFAILED),
		getStringFromResource(UI_FILENAVIGATOR)
	);
}

void CnCS_FN::onButtonClick(CustomButton * button, CTRLID ctrlID)
{
	switch (ctrlID)
	{
	case ID_EXPLORER:
		button->setPartialBorderParts(false, false, true, false);
		button->setMouseAnimation(false);
		{
			auto templateButton =
				reinterpret_cast<CustomButton*>(
					CUSTOMBUTTONCLASS_FROMID(this->fnParam.Frame, ID_TEMPLATE)
					);
			if (templateButton != nullptr)
			{
				templateButton->setMouseAnimation(true);
				templateButton->setPartialBorderParts(true, false, false, true);
				templateButton->Update();
			}
		}
		ShowWindow(
			this->pTV->GetTreeViewHandle(),
			SW_SHOW
		);
		this->hideSampleManager();
		break;
	case ID_TEMPLATE:
		button->setPartialBorderParts(true, false, false, false);
		button->setMouseAnimation(false);
		{
			auto explorerButton =
				reinterpret_cast<CustomButton*>(
					CUSTOMBUTTONCLASS_FROMID(this->fnParam.Frame, ID_EXPLORER)
					);
			if (explorerButton != nullptr)
			{
				explorerButton->setMouseAnimation(true);
				explorerButton->setPartialBorderParts(false, false, true, true);
				explorerButton->Update();
			}
		}
		this->showSampleManager();
		ShowWindow(
			this->pTV->GetTreeViewHandle(),
			SW_HIDE
		);
		break;
	default:
		if ((ctrlID >= ID_SLNEWROOTFOLDER) && (ctrlID <= ID_SEARCH))
		{
			SendMessage(
				this->fnParam.Frame,
				WM_COMMAND,
				MAKEWPARAM(ctrlID, 0),
				0
			);
		}
		break;
	}
}

void CnCS_FN::_createDpiDependendResources()
{
	if (this->fnParam.font != nullptr)
		DeleteObject(this->fnParam.font);

	this->fnParam.font = CreateScaledFont(16, 550, APPLICATION_PRIMARY_FONT);
		//CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Segoe UI\0");

	if (this->fnParam.buttonfont != nullptr)
		DeleteObject(this->fnParam.buttonfont);

	this->fnParam.buttonfont = CreateScaledFont(14, FW_NORMAL, APPLICATION_PRIMARY_FONT);
		//CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Segoe UI\0");
	this->fnParam.tabFont = CreateScaledFont(18, FW_BOLD, APPLICATION_PRIMARY_FONT);
}

void CnCS_FN::_defineMenuEntry(MenuEntry &entry, int eID, int iconID, int iconSQsize, LPCTSTR text)
{
	entry.Clear();
	entry.setID(eID);
	entry.setIcon(iconID, iconSQsize);
	entry.setText(text);
	entry.setColors(
		this->styleInfo.MenuPopUpColor,
		this->styleInfo.TextColor,
		RGB(0, 120, 180),
		RGB(0, 100, 150)
	);
}

void CnCS_FN::setButtonEnabledStateFromSelectionType(int type)
{
	auto enInfo = this->getDWEnableInfo(type);

	for (DWORD i = 0; i < MAX_TBBUTTON; i++)
	{
		auto button =
			reinterpret_cast<CustomButton*>(
				CUSTOMBUTTONCLASS_FROMID(this->fnParam.Frame, ID_SLNEWROOTFOLDER + i)
				);
		if (button != nullptr)
		{
			if (enInfo & (1 << i))
			{
				button->setEnabledState(true);
			}
			else
			{
				button->setEnabledState(false);
			}
			//button->setEnabledState((enInfo & (1 << i)) ? true : false);
		}
	}
}

DWORD CnCS_FN::getDWEnableInfo(int type)
{
	DWORD canPaste =
		this->pTV->canPasteObject()
		? ((DWORD)0b10000) : 0;

	switch (type)
	{
	case PARENT__ITEM:
		return ((DWORD)(0b100101111)) | canPaste;
	case CHILD__ITEM:
		return ((DWORD)(0b111100001));
	case CNC3__ITEM:
		return ((DWORD)(0b100101001)) | canPaste;
	case EMPTY__ITEM:
		return ((DWORD)(0b100000111)) | canPaste;
	case ROOT__ITEM:
		return ((DWORD)(0b100101111));
	default:
		return 0;
	}
}

void CnCS_FN::onFileSystemChanged(BYTE* buffer, DWORD max_buffer)
{
	DWORD nextEntryOffset = 0;
	LPVOID oldnameStart = nullptr;
	PFILE_NOTIFY_INFORMATION pInfo = nullptr;

	do
	{
		// get entry
		pInfo = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(buffer + nextEntryOffset);
		if (pInfo != nullptr)
		{
			nextEntryOffset = pInfo->NextEntryOffset;

			if (nextEntryOffset >= max_buffer)
				break;

			switch (pInfo->Action)
			{
			case FILE_ACTION_ADDED:

				this->pTV->FileSystem_AddItemToTree(pInfo->FileName);

				if (this->FileSystemEvents != nullptr)
				{
					IPath fullPath(this->pTV->Root_Folder);
					fullPath.AppendPath(pInfo->FileName);

					auto is_file = isFile(
						fullPath.GetPathData()
					);

					if (is_file >= 0)
					{
						FILESYSTEMOBJECT fso;
						fso.Path = fullPath;
						fso.FileSystemObjectType = is_file ? FSO_TYPE_FILE : FSO_TYPE_FOLDER;
						fso.FileSystemOperation = FSO_ITEM_CREATED;

						this->FileSystemEvents->onFilesysItemCreated(
							reinterpret_cast<cObject>(this),
							&fso
						);
					}
				}
				break;
			case FILE_ACTION_REMOVED:				
				// NOTE:	if this was an internal move-operation, the filesystem-watcher will sent an additional delete + add notification.
				//			The treeview-control will block the execution, because of the internal fileoperation-blocker, but the filesystemmodification-protocol-method
				//			will be launched at this point and if there was an open tab with a path in the move-path-scope, the tab will be closed (this should not happen)
				//			-> so it is essential to check for an internal move operation, and skip the execution of this node if neccessary!
				//			-> the FileSystem_RemoveItemFromTree(...) method returns false if this was an internal move-operation!
				if (this->pTV->FileSystem_RemoveItemFromTree(pInfo->FileName))
				{
					if (this->FileSystemEvents != nullptr)
					{
						IPath fullPath(this->pTV->Root_Folder);
						fullPath.AppendPath(pInfo->FileName);

						// the problem here is to determine if this was a file or folder
						// because the file/folder is already deleted and the path is not valid anymore

						FILESYSTEMOBJECT fso;
						fso.Path = fullPath;
						fso.FileSystemObjectType = FSO_TYPE_UNKNOWN;
						fso.FileSystemOperation = FSO_ITEM_DELETED;

						this->FileSystemEvents->onFilesysItemDeleted(
							reinterpret_cast<cObject>(this),
							&fso
						);
					}
				}
				break;
			case FILE_ACTION_RENAMED_NEW_NAME:
				if (oldnameStart != nullptr)
				{
					this->pTV->FileSystem_RenameItemInTree((LPCTSTR)oldnameStart, pInfo->FileName);

					if (this->FileSystemEvents != nullptr)
					{
						IPath fullNewPath(this->pTV->Root_Folder);
						fullNewPath.AppendPath(pInfo->FileName);

						IPath fullOldPath(this->pTV->Root_Folder);
						fullOldPath.AppendPath((LPCTSTR)oldnameStart);

						auto is_file = isFile(
							fullNewPath.GetPathData()
						);

						if (is_file >= 0)
						{
							FILESYSTEMOBJECT fso;
							fso.Path = fullNewPath;
							fso.OldPath = fullOldPath;
							fso.FileSystemObjectType = is_file ? FSO_TYPE_FILE : FSO_TYPE_FOLDER;
							fso.FileSystemOperation = FSO_ITEM_RENAMED;

							this->FileSystemEvents->onFilesysItemRenamed(
								reinterpret_cast<cObject>(this),
								&fso
							);
						}
					}
					oldnameStart = nullptr;
				}
				break;
			case FILE_ACTION_RENAMED_OLD_NAME:
				oldnameStart = (buffer + 12);
				break;
			default:
				break;
			}
		}
	} while (nextEntryOffset != 0);
}

void CnCS_FN::onRootFolderInvalidated()
{
	this->pTV->Clear();

	DispatchEWINotification(
		EDSP_ERROR,
		L"FN0004",
		getStringFromResource(ERROR_MSG_ROOTFOLDERINVALIDATED),
		getStringFromResource(UI_FILENAVIGATOR)
	);
}
