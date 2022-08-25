#include"Application.h"
#include"DataExchange\DataExchange.h"
#include"Searchcontrol\SearchControl.h"
#include"SampleManager\SampleManager.h"
#include"CommonControls\StringClass.h"
#include"ApplicationData.h"
#include"AppPath.h"
#include"ThreadWatcher.h"
#include"CommandLineTool.h"
#include"EditorContentManager.h"
#include"Url.h"

Application::Application(HINSTANCE hInst)
	:hInstance( hInst ),
	MainWindow(nullptr),
	hMutex(nullptr),
	restartOptions(NO_RESTART),
	FileNavigator(nullptr),
	Tabcontrol(nullptr),
	CBox(nullptr),
	FileHistory(nullptr)
{
	// load the style-data before creating the user-interface
	this->LoadStyleInfo();

	// create the user interface instance
	this->UserInterface = CreateUI_Instance(hInst, &this->StyleInfo);
	if (!this->UserInterface)
	{
		MessageBox(NULL, L"User Interface creation error!", L"Critical error", MB_OK | MB_ICONERROR);
		ExitProcess(11);
	}

	// Zeroinit the app-data-structure and load the user data
	SecureZeroMemory(&this->AppData, sizeof(APPLICATION_DATA));

	auto result = this->LoadNCSPath();
	if (result)
	{
		result = this->LoadUserData();
	}
	if (!result)
	{
		this->setDefaultWindowSizeData();
	}
	this->ValidateWindowSizeData();

	// pre-initialize the language id with the system-language (this setting may changes later if the user has overwritten the language)
	this->langID = (LanguageID)getSystemLanguage();

	// set the keyboard-hook
	DWORD threadId = GetCurrentThreadId();
	this->keyboardHook = SetWindowsHookEx(WH_KEYBOARD, Application::KeyboardProc, nullptr, threadId);
}

Application::~Application()
{
	//this->SaveStyleInfo();

	if (this->hMutex != NULL)
		ReleaseMutex(this->hMutex);

	UnhookWindowsHookEx(this->keyboardHook);

	SafeRelease(&this->FileNavigator);
	SafeRelease(&this->Tabcontrol);
	SafeRelease(&this->UserInterface);
	SafeRelease(&this->appProperty);
	SafeRelease(&this->CBox);
	SafeRelease(&this->FileHistory);

	//FreeLibrary()
}

HRESULT Application::Init_Data()
{
	// show startup window
	// show working indication
	// create filesystem checkup thread
	// create data initialization

	this->appProperty = CreateCnCSuiteProperty();
	if (this->appProperty != nullptr)
	{
		auto res = this->appProperty->VerifyApplicationFileSystem();
		if (res)
		{
			if (res == APPLICATION_ISFIRSTUSE)
				this->AppData.IsFirstUse = TRUE;

			if (!this->appProperty->AcquireData())
			{
				// no settings found -> apply default settings
				this->SetDefaultAppSettings();
			}
			else
			{
				// settings exist -> get the neccessary parameter
				this->langID = this->appProperty->GetUserLanguage();
			}
		}
		else
		{
			// error: filesystem creation, repair or verification failed
			// messagebox ??

			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT Application::Init_Application()
{
	//HRESULT hr = this->appProperty->StringToSplashScreen(
	//	getStringFromResource(UI_SSCREEN_INITMAINFRAME)
	//);
	HRESULT hr = S_OK;

	if (SUCCEEDED(hr))
	{
		hr = this->Init_Mainframe();
		if (SUCCEEDED(hr))
		{
			CSTMFRAME cFrame;

			hr = this->CollectFrameData(&cFrame);
			if (SUCCEEDED(hr))
			{
				hr = this->UserInterface->Init(this->MainWindow, &cFrame);
				if (SUCCEEDED(hr))
				{
					HMODULE hMod = LoadLibrary(L"Msftedit.dll");

					hr = (hMod != NULL) ? S_OK : E_FAIL;
					if (SUCCEEDED(hr))
					{
						hr = this->Init_Components();
						if (SUCCEEDED(hr))
						{
#pragma warning(push)
#pragma warning(disable : 4127)

							// set the update timer for the free app version only!
							if (AppType == APPTYPE_FREEAPP)
							{
								SetTimer(this->MainWindow, ID_TIMER_UPDATESEARCH, 30000, nullptr);
							}
#pragma warning(pop)
							if (this->splScreen != nullptr)
							{
								hr = this->splScreen->Stop();
							}
							if (SUCCEEDED(hr))
							{
								SetTimer(this->MainWindow, ID_TIMER_AUTOSAVE, 15000, nullptr);
								SetTimer(this->MainWindow, ID_TIMER_SESSIONSAVE, 20000, nullptr);


								// set default restore frame????

								ShowWindow(this->MainWindow, this->AppData.WndSizeData.nCmdShow);
								UpdateWindow(this->MainWindow);
							}
						}
					}
				}
			}
		}
	}
	return hr;
}

int Application::Run()
{
	MSG msg;
	BOOL bReturn;

	while ((bReturn = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bReturn == -1)
		{
			MessageBox(NULL, L"Messageloop failed.\nApp will be terminated - pos::main::loop", L"Critical error", MB_OK | MB_ICONERROR);
			ExitProcess((UINT)bReturn);
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return static_cast<int>(msg.wParam);
}

BOOL Application::CHK_Mutex(LPTSTR nCmdLine)
{
	HRESULT hr = S_OK;

	this->hMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, L"CnC Suite 1.0\0");
	if (!this->hMutex)
	{
		this->hMutex = CreateMutex(NULL, 0, L"CnC Suite 1.0\0");

		if (nCmdLine != NULL)
		{
			BasicFPO* pfpo = CreateBasicFPO();
			if (pfpo != NULL)
			{
				/*LPTSTR Cmd = GetCommandLine();*/

				TCHAR* nCmdPath = NULL;

				hr = pfpo->VerifyCommandline(nCmdLine, &nCmdPath) ? S_OK : E_FAIL;
				if (SUCCEEDED(hr))
				{
					hr = (CopyStringToPtrA(nCmdPath, &this->AppData.CmdLine) == TRUE) ? S_OK : E_FAIL;
					if (SUCCEEDED(hr))
					{
						this->AppData.AppStartedWithCmdLine = TRUE;
					}
					SafeDeleteArray(&nCmdPath);
				}
				SafeRelease(&pfpo);
			}
		}		
		return TRUE;
	}
	else
	{
		HWND hWnd = FindWindow(IDSEX_APPLICATIONCLASS, NULL);
		if (hWnd)
		{
			size_t len;

			SendMessage(hWnd, WM_PROCESSNEWINSTANCE, 0, 0);

			SetForegroundWindow(hWnd);

			hr = StringCbLength(nCmdLine, STRSAFE_MAX_LENGTH, &len);
			if (SUCCEEDED(hr))
			{
				COPYDATASTRUCT cds;
				cds.cbData = (DWORD)(len + sizeof(TCHAR));
				cds.dwData = 0;
				cds.lpData = nCmdLine;

				SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM)&cds);
			}
		}
		return FALSE;
	}
}

LanguageID Application::getLanguage()
{
	return this->langID;
}

LONG_PTR Application::getPropertyComponent()
{
	return reinterpret_cast<LONG_PTR>(this->appProperty);
}

LONG_PTR Application::getFileExplorerComponent()
{
	return reinterpret_cast<LONG_PTR>(this->FileNavigator);
}

LONG_PTR Application::getTabControlComponent()
{
	return reinterpret_cast<LONG_PTR>(this->Tabcontrol);
}

LONG_PTR Application::getCBoxComponent()
{
	return reinterpret_cast<LONG_PTR>(this->CBox);
}

LONG_PTR Application::getHistoryComponent()
{
	return reinterpret_cast<LONG_PTR>(this->FileHistory);
}

void Application::OnEntryClicked(cObject sender, HistoryItem* item)
{
	UNREFERENCED_PARAMETER(sender);

	iString message(
		getStringFromResource(UI_FILEINFO)
	);
	message += L": ";
	message += item->GetDisplayName();
	message += L"\n\n";
	message += getStringFromResource(UI_PATH);
	message += L": ";
	message += item->GetItemPath();
	message += L"\n\n";
	message += getStringFromResource(UI_FILETIME_LASTACCESS);
	message += L"  ";
	message += item->GetLastOpenedTime().SimpleDateAsString();
	message += L" ";
	message += item->GetLastOpenedTime().SimpleTimeAsString();
	message += L"\n\n";
	message += getStringFromResource(UI_GNRL_OPENFILE);
	message += L"?";

	auto res =
		MessageBox(
			this->MainWindow,
			message.GetData(),
			item->GetDisplayName().GetData(),
			MB_YESNO | MB_DEFBUTTON1 | MB_ICONINFORMATION
		);

	if (res == IDYES)
	{
		// open file
		this->OnNavigatorOpenRequest(
			MAKEWPARAM(0, FORCE_OPEN_IN_NEW_TAB),
			reinterpret_cast<LPARAM>(
				item->GetItemPath()
				.GetData()
				)
		);
	}
}

void Application::FormatForExport(const CnC3File & file, iString & buffer_out)
{
	auto dataContainer =
		reinterpret_cast<ApplicationData*>(
			getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
			);
	if (dataContainer != nullptr)
	{
		auto formatIndex = dataContainer->getIntegerData(DATAKEY_EXSETTINGS_EXPORT_LINEENDFORMAT, 0);

		EditorContentManager em;
		em.SetExecuteAsync(false);
		em.SetContent(
			file.GetNCContent()
		);

		switch (formatIndex)
		{
		case 0:
			em.SetEndOfLineFormat(EditorContentManager::ENDOFLINE_FORMAT_CRLF);
			break;
		case 1:
			em.SetEndOfLineFormat(EditorContentManager::ENDOFLINE_FORMAT_CR);
			break;
		case 2:
			em.SetEndOfLineFormat(EditorContentManager::ENDOFLINE_FORMAT_LF);
			break;
		default:
			em.SetEndOfLineFormat(EditorContentManager::ENDOFLINE_FORMAT_CRLF);
			break;
		}

		if (em.PrepareContentForSubsequentUsage())
		{
			buffer_out.Replace(
				em.GetContent()
			);
		}
	}
}

HRESULT Application::Init_Mainframe()
{
	HRESULT hr;
	WCHAR Appclass[100] = { 0 };

	hr = (LoadString(this->hInstance, IDS_APPLICATIONCLASS, Appclass, MAX_LOADSTRING) == 0) ? E_FAIL : S_OK;
	if( SUCCEEDED(hr))
	{
		WCHAR Appname[100] = { 0 };

		hr = (LoadString(this->hInstance, IDS_APPLICATIONNAME, Appname, MAX_LOADSTRING) == 0) ? E_FAIL : S_OK;
		if(SUCCEEDED(hr))
		{
			WNDCLASSEX wcx;
			wcx.cbSize = sizeof(WNDCLASSEX);
			wcx.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
			wcx.cbClsExtra = 0;
			wcx.cbWndExtra = sizeof(LONG_PTR);
			wcx.lpfnWndProc = Application::WndProc;
			wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcx.hbrBackground = this->UserInterface->GetBkgndBrush();
			wcx.hIcon = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_CNCSUITE_NORM), IMAGE_ICON, 128, 128, LR_DEFAULTCOLOR);
			wcx.hInstance = this->hInstance;
			wcx.lpszMenuName = NULL;
			wcx.lpszClassName = Appclass;
			wcx.hIconSm = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_CNCSUITE_SMLL), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
			
			hr = (RegisterClassEx(&wcx) == 0) ? E_FAIL : S_OK;
			if(SUCCEEDED(hr))
			{
				int x, y, cx, cy;

				// the windowsize-data existed and were loaded correctly
				if (this->AppData.WndSizeData.valid)
				{
					// set normal rect
					x = this->AppData.WndSizeData.rc_Window.left;
					y = this->AppData.WndSizeData.rc_Window.top;
					cx = this->AppData.WndSizeData.rc_Window.right - this->AppData.WndSizeData.rc_Window.left;
					cy = this->AppData.WndSizeData.rc_Window.bottom - this->AppData.WndSizeData.rc_Window.top;


					// check if the window was maximized on last close
					this->AppData.WndSizeData.nCmdShow =
						(this->AppData.WndSizeData.valid == MAXIMIZENORMAL)
						? SW_SHOWMAXIMIZED : SW_SHOW;
				}
				else
				{
					// the windowsize-data was invalid
					// there are two scenarios where this could happen: (1.The App was launched the first time) OR (2.The User deleted the local-settings file)

					// set default values:
					int sys_x = GetSystemMetrics(SM_CXSCREEN);
					int sys_y = GetSystemMetrics(SM_CYSCREEN);

					x = 100;
					y = 100;
					
					cx = sys_x - 200;
					if (cx < 600)
						cx = 600;
					cy = sys_y - 200;
					if (cy < 400)
						cy = 400;

					// by default, show the window maximized
					this->AppData.WndSizeData.nCmdShow = SW_SHOWMAXIMIZED;
				}

				this->MainWindow
					= CreateWindow(
						Appclass,
						Appname,
						WS_POPUP | WS_BORDER | WS_CLIPCHILDREN,
						x, y, cx, cy,
						NULL, NULL,
						this->hInstance,
						reinterpret_cast<LPVOID>(this)
					);

				hr = (this->MainWindow != nullptr) ? S_OK : E_FAIL;
				if (SUCCEEDED(hr))
				{
					// apply rounded corners (only if the window is not maximized)
					if (this->AppData.WndSizeData.nCmdShow != SW_SHOWMAXIMIZED)
					{
						DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_ROUND;
						DwmSetWindowAttribute(this->MainWindow, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
					}
				}
			}
		}
	}
	return hr;
}

HRESULT Application::Init_Components()
{
	HRESULT hr;

	auto dataContainer = reinterpret_cast<ApplicationData*>(
		getDefaultApplicationDataContainer()
		);
	hr = (dataContainer != nullptr) ? S_OK : E_UNEXPECTED;
	if (SUCCEEDED(hr))
	{
		this->CBox = CreateCBoxInstance(this->hInstance, this->MainWindow);

		hr = (this->CBox == nullptr) ? E_NOINTERFACE : S_OK;
		if (SUCCEEDED(hr))
		{
			hr = this->CBox->Init(
				this->UserInterface->GetFrameHandles(GFWH_CBOXFRAME)
			);

			if (SUCCEEDED(hr))
			{
				this->Tabcontrol = CreateTabcontrol(this->hInstance, this->MainWindow);

				hr = (this->Tabcontrol == nullptr) ? E_NOINTERFACE : S_OK;
				if (SUCCEEDED(hr))
				{
					TCSTARTUPINFO tInfo;
					SecureZeroMemory(&tInfo, sizeof(TCSTARTUPINFO));

					if (this->AppData.IsFirstUse)
						tInfo.restoreSession = FALSE;
					else
					{
						tInfo.restoreSession =
							dataContainer->getBooleanData(DATAKEY_SETTINGS_SAVE_TABWND_COND, true)
							? TRUE : FALSE;
					}

					tInfo.WorkingDirectory = this->AppData.UserPath;
					if (this->AppData.AppStartedWithCmdLine)
					{
						tInfo.mode = TCI_OPENREQUEST;
						tInfo.PathToFile = this->AppData.CmdLine;
					}
					else
					{
						tInfo.mode = this->AppData.IsFirstUse ? TCI_ISFIRSTUSE : TCI_NOINITACTION;
						tInfo.PathToFile = NULL;
					}
					auto box = reinterpret_cast<iBox<bool>*>(
						dataContainer->lookUp(DATAKEY_SETTINGS_COLORSCHEME_USAGE));
					if (box != nullptr)
					{
						tInfo.useXMLColorScheme
							= box->get()
							? TRUE : FALSE;

						box->Release();
					}

					hr = this->Tabcontrol->Init(
						this->UserInterface->GetFrameHandles(GFWH_EDITFRAME),
						&tInfo
					);

					if (SUCCEEDED(hr))
					{
						this->FileNavigator = CreateFileNavigator(this->hInstance, this->MainWindow);

						hr = (this->FileNavigator == nullptr) ? E_NOINTERFACE : S_OK;
						if (SUCCEEDED(hr))
						{
							hr = this->FileNavigator->Init(
								this->UserInterface->GetFrameHandles(GFWH_TVFRAME),
								this->AppData.UserPath
							);

							if (SUCCEEDED(hr))
							{
								auto restoreEXPCondition =
									this->AppData.IsFirstUse
									? false
									: dataContainer->getBooleanData(DATAKEY_SETTINGS_SAVE_EXPLORER_COND, true);

								if (restoreEXPCondition)
								{
									this->FileNavigator->LoadCondition();
								}

								this->FileNavigator->SetEventHandler(dynamic_cast<IFileSystemModificationProtocol*>(this));

								this->Tabcontrol->SetFocusOnCurrentVisibleEditbox();

								auto async = new Async();
								if (async != nullptr)
								{
									async->callFunction(&updateFocusRect);
								}

								this->FileHistory = new UIHistory();

								hr = (this->FileHistory != nullptr) ? S_OK : E_NOINTERFACE;
								if (SUCCEEDED(hr))
								{
									this->FileHistory->SetEventHandler(
										dynamic_cast<IHistroyEventProtocol*>(this)
									);

									// ...
								}
							}
						}
					}					
				}
			}
		}
		auto singleRecover = dataContainer->getUnsignedIntegerData(DATAKEY_SETTINGS_SINGLESESSIONRECOVER, 0);
		if (singleRecover & 0x01)
		{
			// the last time the app was closed it was due to a forced end-session
			// restore the 3 save data fields
			if (singleRecover & 0x02)
				dataContainer->saveValue(DATAKEY_SETTINGS_SAVE_EXPLORER_COND, false);
			if (singleRecover & 0x04)
				dataContainer->saveValue(DATAKEY_SETTINGS_SAVE_TABWND_COND, false);
			if (singleRecover & 0x08)
				dataContainer->saveValue(DATAKEY_SETTINGS_SAVE_UNSAVED_CONTENT, false);

			// delete the trigger-data
			dataContainer->deleteValue(DATAKEY_SETTINGS_SINGLESESSIONRECOVER);
		}
	}
	return hr;
}

LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Application* pApp = NULL;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT lpc = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pApp = reinterpret_cast<Application*>(lpc->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApp));

		return 1;
	}
	else
	{
		pApp = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (pApp != NULL)
		{
			switch( message )
			{
			case WM_COMMAND:
				return pApp->OnCommand(hWnd, wParam, lParam);
			case WM_FRAMECHANGED:
				return pApp->OnFrameChanged(hWnd, lParam);
			case WM_RESTARTAPPLICATION:
				return pApp->OnRestartApp(wParam);
			case WM_GETAPPLICATIONDATA:
				return pApp->OnGetAppData(lParam);
			case WM_QUERYENDSESSION:
				return pApp->OnQueryEndSession(lParam);
			case WM_ENDSESSION:
				return pApp->OnSessionEnd(lParam);
			case WM_DPICHANGED:
				return pApp->OnDPIChanged(wParam, lParam);
			case WM_COPYDATA:
				return pApp->OnCopyData(lParam);
			case WM_GETTABCTRLPROPERTY:
				return pApp->OnGetTabCtrlProperty(lParam);
			case WM_GETCURRENTSELECTION:
				return pApp->OnGetCurrentSelection(lParam);
			case WM_DISPLAYERROR:
				return pApp->OnDisplayError(lParam);
			case WM_GETSTYLEINFO:
				return pApp->OnStyleInfo(lParam);
			case WM_TIMER:
				return pApp->OnTimerEvent(hWnd, wParam);
			case WM_VALIDATEERROR:
				return pApp->OnValidateError(lParam);
			case WM_UPDATESTATUSBAR:
				return pApp->OnStatusbarUpdate(lParam);
			case WM_GETABSEDITPOS:
				return SendMessage(pApp->UserInterface->GetFrameHandles(GFWH_EDITFRAME), message, wParam, lParam);
			case WM_TRANSMISSIONCOMPLETE:
				return pApp->OnTransmissionComplete(lParam);
			case WM_HANDLESEARCHRESULT:
				return pApp->OnSearchResult(lParam);
			case WM_DISPATCHTEXT:
				return pApp->OnDispatchText(lParam);
			case WM_GETDESCRIPTIONS:
				return pApp->OnGetDescriptions(lParam);
			case WM_SETDESCRIPTIONS:
				return pApp->OnSetDescriptions(lParam);
			case WM_GETEDITCONTROLPROPERIES:
				return pApp->OnGetEditControlProperties(wParam, lParam);
			case WM_UPDATEEDITCOLORS:
				return pApp->OnUpdateEditColors(wParam);
			case WM_INTERNALCOMMAND:
				return pApp->OnInternalCommand(wParam, lParam);
			case WM_CLEANUP:
				return pApp->OnCleanUp(wParam, lParam);
			case WM_CLOSE:
				return pApp->OnClose();
			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;
			default:
				if (pApp->UserInterface != nullptr)
				{
					if (pApp->UserInterface->IsReady())
					{
						return pApp->UserInterface->DefaultHandler(hWnd, message, wParam, lParam);
					}
				}
				break;
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

LRESULT Application::KeyboardProc(int Code, WPARAM wParam, LPARAM lParam)
{
	if(Code >= 0)
	{
		HWND mainWindow = FindWindow(IDSEX_APPLICATIONCLASS, nullptr);
		if (mainWindow != nullptr)
		{
			auto pApp = reinterpret_cast<Application*>(GetWindowLongPtr(mainWindow, GWLP_USERDATA));
			if (pApp != nullptr)
			{
				if (!(lParam & 0x40000000))// only process this message if the key is pressed once
				{
					if (!(lParam & 0x80000000))// filter keydown messages
					{
						auto result = pApp->OnKeydown(wParam);
						if (result == 0)
						{
							return 0;
						}
					}
				}
			}
		}		
	}
	return CallNextHookEx(nullptr, Code, wParam, lParam);
}

LRESULT Application::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case IDM_CLOSE:
		SendMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	case IDM_APPSETTINGS:
		this->OpenAppProperties();
		break;
	case IDM_NEW:
		this->Tabcontrol->UserRequest_AddNewTab();
		break;
	case IDM_OPEN:
		this->Open();
		break;
	case IDM_SAVE:
		this->Save();
		break;
	case IDM_SAVEAS:
		this->SaveAs();
		break;
	case IDM_SAVEALL:
		this->SaveAll();
		break;
	case IDM_IMPORT:
		this->Import(nullptr);
		break;
	case IDM_COPY:
		this->OnClipboardAction(IDM_COPY);
		break;
	case IDM_PASTE:
		this->OnClipboardAction(IDM_PASTE);
		break;
	case IDM_SELECTALL:
		this->OnClipboardAction(IDM_SELECTALL);
		break;
	case IDM_EDITDELETE:
		this->OnClipboardAction(IDM_EDITDELETE);
		break;
	case IDM_CUT:
		this->OnClipboardAction(IDM_CUT);
		break;
	case IDM_SEND:
		this->Send();
		break;
	case IDM_RECEIVE:
		this->Receive();
		break;
	case IDM_EXPORT:
		this->Export();
		break;
	case IDM_HELP:
		this->ShowHelpExtension();
		break;
	case IDM_WEB:
		this->LaunchWebsite();
		break;
	case IDM_HISTORY:
		this->ShowHistoryWnd();
		break;
	case NAVIGATOR_SETCURRENTFOLDER:
		this->NavigationFolder.Replace(
			reinterpret_cast<const TCHAR*>(lParam)
		);
		break;
	case NAVIGATOR_RQ_OPENPATH:
		this->OnNavigatorOpenRequest(wParam, lParam);
		break;
	case NAVIGATOR_FILEWASCONVERTED:
		this->OnFileConverted(
			reinterpret_cast<LPTSTR>(lParam)
		);
		break;
	//case NAVIGATOR_FILEWASRENAMED:
	//	this->Tabcontrol->UserAction_FileRenamed(
	//		reinterpret_cast<LPCNCFILEUPDATE>(lParam)
	//	);
	//	break;
	case NAVIGATOR_RQ_IMPORTFILE:
		this->Import(
			reinterpret_cast<LPTSTR>(lParam)
		);
		break;
	//case NAVIGATOR_FILEWASDELETED:
	//	this->Tabcontrol->UserAction_FileDeleted(
	//		reinterpret_cast<LPTSTR>(lParam)
	//	);
	//	break;
	case ICOMMAND_PROPERTYWINDOW_CLOSED:
		this->UserInterface->setParameter(
			PARAID_UI_SETTINGSBUTTON_TOUCHDOWNLOCK,
			(LONG_PTR)FALSE
		);
		SendMessage(this->MainWindow, WM_NORMALIZE, (WPARAM)IDM_APPSETTINGS, 0);
		break;
	//case ICOMMAND_TVITEMMOVED:
	//	this->Tabcontrol->UserAction_TVItemMoved(
	//		reinterpret_cast<LPFILEOPERATIONINFO>(lParam)
	//	);
	//	break;
	case ICOMMAND_TVREFRESH:
		this->FileNavigator->ReloadAsync();
		break;
	case ICOMMAND_EXPANDPATHTOFILE:
		this->FileNavigator->ExpandPathToFile(reinterpret_cast<LPCTSTR>(lParam));
		break;
	default:
		break;
	}
	return static_cast<LRESULT>(0);
}

LRESULT Application::OnFrameChanged(HWND hWnd, LPARAM lParam)
{
	BOOL result = TRUE;

	LPCSTMFRAME pCframe = reinterpret_cast<LPCSTMFRAME>(lParam);
	if (pCframe != NULL)
	{
		this->AppData.WndSizeData.valid = pCframe->valid;
		this->AppData.WndSizeData.EditFrameHeight = pCframe->EditFrameHeight;
		this->AppData.WndSizeData.TVFrame_width = pCframe->TVFrameWidth;
		this->AppData.WndSizeData.nCmdShow = pCframe->nCmdShow;

		if ((pCframe->nCmdShow != SW_SHOWMAXIMIZED) && (pCframe->valid != MAXIMIZENORMAL))
		{
			GetWindowRect(hWnd, &this->AppData.WndSizeData.rc_Window);
		}
		if (!this->SaveUserData())
		{
			result = FALSE;
		}
	}
	else
		result = FALSE;

	return static_cast<LRESULT>(result);
}

LRESULT Application::OnSessionEnd(LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	// App will be terminated by the system due to OS-Shutdown...
	// -> SAVE ALL
	// save the session for recovery ... regardless if the user has set the appropriate settings

	ThreadWatcher watcher(
		GetCurrentProcessId()
	);

	auto appDataContainer
		= reinterpret_cast<ApplicationData*>(
			getDefaultApplicationDataContainer()
		);

	if (appDataContainer != nullptr)
	{
		unsigned int singleRecover = 1;

		auto rc_exp = appDataContainer->getBooleanData(DATAKEY_SETTINGS_SAVE_EXPLORER_COND, true);
		auto rc_tab = appDataContainer->getBooleanData(DATAKEY_SETTINGS_SAVE_TABWND_COND, true);
		auto rc_sus = appDataContainer->getBooleanData(DATAKEY_SETTINGS_SAVE_UNSAVED_CONTENT, false);
		if (!rc_exp)
			singleRecover |= 0x02;
		if (!rc_tab)
			singleRecover |= 0x04;
		if (!rc_sus)
			singleRecover |= 0x08;

		appDataContainer->saveValue(DATAKEY_SETTINGS_SAVE_EXPLORER_COND, true);
		appDataContainer->saveValue(DATAKEY_SETTINGS_SAVE_TABWND_COND, true);
		appDataContainer->saveValue(DATAKEY_SETTINGS_SAVE_UNSAVED_CONTENT, true);
		appDataContainer->saveValue(DATAKEY_SETTINGS_SINGLESESSIONRECOVER, singleRecover);
	}
	this->Tabcontrol->SaveTabCondition();
	this->FileNavigator->SaveCondition();

	// let the running threads finish and then return!
	watcher.wait(2000);

	//Sleep(500); // substitute with a better waiting method!

	return static_cast<LRESULT>(0);
}

LRESULT Application::OnQueryEndSession(LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	return static_cast<LRESULT>(TRUE);
}

LRESULT Application::OnGetAppData(LPARAM lParam)
{
	BOOL result;

	LPAPPLICATION_DATA pAdata = reinterpret_cast<LPAPPLICATION_DATA>(lParam);

	result = (pAdata == nullptr) ? FALSE : TRUE;
	if (result)
	{
		HRESULT hr = S_OK;
		size_t len;

		SecureZeroMemory(pAdata, sizeof(APPLICATION_DATA));

		pAdata->AppStartedWithCmdLine = this->AppData.AppStartedWithCmdLine;

		if (this->AppData.CmdLine != nullptr)
		{
			hr = StringCbLength(this->AppData.CmdLine, STRSAFE_MAX_CCH* sizeof(TCHAR), &len);
			if (SUCCEEDED(hr))
			{
				pAdata->CmdLine = new WCHAR[len + sizeof(WCHAR)];

				hr = (pAdata->CmdLine == nullptr) ? E_OUTOFMEMORY : S_OK;
				if (SUCCEEDED(hr))
				{
					hr = StringCbCopy(pAdata->CmdLine, len + sizeof(WCHAR), this->AppData.CmdLine);
				}
			}
		}
		result = SUCCEEDED(hr) ? TRUE : FALSE;
		if (result)
		{
			if (this->AppData.UserPath != nullptr)
			{
				hr = StringCbLength(this->AppData.UserPath, STRSAFE_MAX_CCH* sizeof(TCHAR), &len);
				if (SUCCEEDED(hr))
				{
					pAdata->UserPath = new WCHAR[len + sizeof(WCHAR)];

					hr = (pAdata->UserPath == nullptr) ? E_OUTOFMEMORY : S_OK;
					if (SUCCEEDED(hr))
					{
						hr = StringCbCopy(pAdata->UserPath, len + sizeof(WCHAR), this->AppData.UserPath);
					}
				}
			}
		}
		result = SUCCEEDED(hr) ? TRUE : FALSE;
		if (result)
		{
			pAdata->WndSizeData.EditFrameHeight = this->AppData.WndSizeData.EditFrameHeight;
			pAdata->WndSizeData.nCmdShow = this->AppData.WndSizeData.nCmdShow;
			pAdata->WndSizeData.TVFrame_width = this->AppData.WndSizeData.TVFrame_width;
			pAdata->WndSizeData.valid = this->AppData.WndSizeData.valid;

			SetRect(&pAdata->WndSizeData.rc_Window,
				this->AppData.WndSizeData.rc_Window.left,
				this->AppData.WndSizeData.rc_Window.top,
				this->AppData.WndSizeData.rc_Window.right,
				this->AppData.WndSizeData.rc_Window.bottom);
		}
	}
	return static_cast<LRESULT>(result);
}

LRESULT Application::OnCopyData(LPARAM lParam)
{
	PCOPYDATASTRUCT pcds = reinterpret_cast<PCOPYDATASTRUCT>(lParam);
	if (pcds != NULL)
	{
		auto lpCmdLine = reinterpret_cast<LPTSTR>(pcds->lpData);
		if (lpCmdLine != nullptr)
		{
			auto fpop = CreateBasicFPO();
			if (fpop != nullptr)
			{
				TCHAR* path = NULL;

				if (fpop->VerifyCommandline(lpCmdLine, &path))
				{
					this->Tabcontrol->UserRequest_Open(path, TRUE, FALSE);

					SafeDeleteArray(&path);
				}
				SafeRelease(&fpop);
			}
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT Application::OnDisplayError(LPARAM lParam)
{
	BOOL result = TRUE;

	LPEDSPSTRUCT pedsp = reinterpret_cast<LPEDSPSTRUCT>(lParam);
	if (pedsp != NULL)
	{
		//int type = pedsp->type;
		this->CBox->DisplayNotification(pedsp->type, pedsp->Code, pedsp->Description, pedsp->Location);
	}
	else
		result = FALSE;

	return static_cast<LRESULT>(result);
}

LRESULT Application::OnStyleInfo(LPARAM lParam)
{
	BOOL result = TRUE;

	LPAPPSTYLEINFO sInfo = reinterpret_cast<LPAPPSTYLEINFO>(lParam);
	if (sInfo != NULL)
	{
		copyAppStyleInfo(&this->StyleInfo, sInfo);
	}
	else
		result = FALSE;

	return static_cast<LRESULT>(result);
}

LRESULT Application::OnTimerEvent(HWND hWnd, WPARAM wParam)
{
	switch (wParam)
	{
	case ID_TIMER_UPDATESEARCH:
		this->CheckForUpdates();
		KillTimer(this->MainWindow, ID_TIMER_UPDATESEARCH);
		break;
	case ID_TIMER_VALIDATEERROR:
		this->CBox->RequestErrorValidity();
		break;
	case ID_TIMER_SAVEFRAMEDATA:
		this->UserInterface->DefaultHandler(hWnd, WM_TIMER, wParam, (LPARAM)0);
		break;
	case ID_TIMER_AUTOSAVE:
		this->Autosave();
		break;
	case ID_TIMER_SESSIONSAVE:
		this->SaveSession();
		break;
	default:
		break;
	}
	return static_cast<LRESULT>(0);
}

LRESULT Application::OnValidateError(LPARAM lParam)
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

LRESULT Application::OnStatusbarUpdate(LPARAM lParam)
{
	LPSTBUPDATE pst = reinterpret_cast<LPSTBUPDATE>(lParam);
	
	if (pst != NULL)
	{
		this->UserInterface->SetStatusbarInfo(pst->part, pst->text);
	}
	return static_cast<LRESULT>(0);
}

LRESULT Application::OnClose()
{
	if (this->restartOptions != RESTART_DONOTSAVE)
	{
		auto appDataContainer
			= reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer()
				);
		if (appDataContainer != nullptr)
		{
			auto saveEXPCond = appDataContainer->getBooleanData(DATAKEY_SETTINGS_SAVE_EXPLORER_COND, true);
			if (saveEXPCond)
			{
				this->FileNavigator->SaveCondition();
			}
		}
		BOOL closeApp = this->Tabcontrol->UserRequest_CloseApp();

		if (closeApp)
		{
			DestroyWindow(this->MainWindow);
		}
	}
	else
	{
		DestroyWindow(this->MainWindow);
	}
	return static_cast<LRESULT>(0);
}

LRESULT Application::OnTransmissionComplete(LPARAM lParam)
{
	this->Tabcontrol->UserRequest_Import(
		reinterpret_cast<LPTSTR>(lParam)
	);
	return static_cast<LRESULT>(0);
}

LRESULT Application::OnCleanUp(WPARAM wParam, LPARAM lParam)
{
	if (wParam == DATAEXCHANGECLASS)
	{
		auto dataTraffic = reinterpret_cast<SerialComm*>(lParam);
		if (dataTraffic != nullptr)
		{
			delete dataTraffic;
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT Application::OnSearchResult(LPARAM lParam)
{
	auto searchResult = reinterpret_cast<LPOPENRESULT>(lParam);
	if (searchResult != nullptr)
	{
		for (int i = 0; i < searchResult->resultCount; i++)
		{
			if (searchResult->OpenType[i] == RSLT_TYPE_FILE)
			{
				this->Tabcontrol->UserRequest_Open(searchResult->openPath[i], TRUE, FALSE);
			}
			if (searchResult->expandPath)
			{
				// expand path
				auto dataContainer =
					reinterpret_cast<ApplicationData*>(
						getDefaultApplicationDataContainer()
					);

				if (dataContainer != nullptr)
				{
					auto expPath = dataContainer->getBooleanData(DATAKEY_SETTINGS_ALWAYS_EXPAND_PATH, true);
					if (expPath)
					{
						this->FileNavigator->ExpandPathToFile(
							searchResult->openPath[i]
						);
					}
				}
			}
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT Application::OnGetTabCtrlProperty(LPARAM lParam)
{
	auto ptc = reinterpret_cast<LPTCPROPERTY>(lParam);
	if (ptc != nullptr)
	{
		this->Tabcontrol->GetProperty(ptc);
	}
	return static_cast<LRESULT>(0);
}

LRESULT Application::OnGetCurrentSelection(LPARAM lParam)
{
	auto selInfo = reinterpret_cast<LPSELECTIONINFO>(lParam);
	if (selInfo != nullptr)
	{
		selInfo->success = this->Tabcontrol->GetCurrentSelectedText(selInfo);
	}
	return static_cast<LRESULT>(0);
}

LRESULT Application::OnDispatchText(LPARAM lParam)
{
	auto pdt = reinterpret_cast<LPDISPATCHTEXT>(lParam);
	if (pdt != nullptr)
	{
		switch (pdt->destination)
		{
		case DT_DESTINATION_TABCONTROL:
			switch (pdt->mode)
			{
			case DT_INSERT:
				this->Tabcontrol->UserRequest_InsertText(pdt->text);
				break;
			case DT_SETTEXT:
				this->Tabcontrol->UserRequest_Import(pdt->text);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT Application::OnGetDescriptions(LPARAM lParam)
{
	return static_cast<LRESULT>(
		this->Tabcontrol->GetDescriptionNames(
			reinterpret_cast<LPDESCRIPTIONINFO>(lParam)
		)
	);
}

LRESULT Application::OnSetDescriptions(LPARAM lParam)
{
	this->Tabcontrol->SetDescriptionNames(reinterpret_cast<LPDESCRIPTIONINFO>(lParam));

	return static_cast<LRESULT>(0);
}

LRESULT Application::OnGetEditControlProperties(WPARAM wParam, LPARAM lParam)
{
	this->Tabcontrol->GetEditControlProperties(wParam, lParam);

	return static_cast<LRESULT>(0);
}

LRESULT Application::OnUpdateEditColors(WPARAM wParam)
{
	this->Tabcontrol->UserRequest_SetNewColorScheme((BOOL)wParam);

	return static_cast<LRESULT>(0);
}

LRESULT Application::OnInternalCommand(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (LOWORD(wParam))
	{
	case ICOMMAND_AUTOCOMPLETE_DATA_CHANGED:
		this->Tabcontrol->UpdateAutocompleteData();
		break;
	case ICOMMAND_AUTOSYNTAX_DATA_CHANGED:
		this->Tabcontrol->UpdateAutosyntaxSettings();
		break;
	default:
		break;
	}
	return static_cast<LRESULT>(0);
}

LRESULT Application::OnDPIChanged(WPARAM wParam, LPARAM lParam)
{
	(reinterpret_cast<DPI_Assist*>(getDPIAssist()))->onDPIchanged(wParam);

	auto prc_scale = reinterpret_cast<LPRECT>(lParam);

	// update all fonts and graphic resources
	this->UserInterface->onDpiChanged();
	this->CBox->onDpiChanged();
	this->FileNavigator->onDpiChanged();
	this->Tabcontrol->onDpiChanged();
	this->appProperty->onDpiChanged();

	// set new size and redraw
	if (prc_scale != nullptr)
	{
		SetWindowPos(
			this->MainWindow,
			HWND_TOP,
			prc_scale->left,
			prc_scale->top,
			prc_scale->right - prc_scale->left,
			prc_scale->bottom - prc_scale->top,
			SWP_NOZORDER | SWP_NOACTIVATE
		);
	}
	RedrawWindow(
		this->MainWindow,
		nullptr,
		nullptr,
		RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_NOERASE
	);	

	return static_cast<LRESULT>(0);
}

LRESULT Application::OnRestartApp(WPARAM wParam)
{
	this->restartOptions = (DWORD)wParam;
	PostMessage(this->MainWindow, WM_CLOSE, 0, 0);
	return static_cast<LRESULT>(0);
}

void Application::Open()
{
	CnC3File currentFile;
	CnC3FileManager cnc3FileManager(this->MainWindow);

	//cnc3FileManager.SetDialogText(???)

	this->Tabcontrol->GetCurrentCnC3File(currentFile);
	if (currentFile.HasPath())
	{
		auto bfpo = CreateBasicFPO();
		if (bfpo != nullptr)
		{
			TCHAR* path = nullptr;

			if (CopyStringToPtr(currentFile.GetPath(), &path) == TRUE)
			{
				if (bfpo->RemoveFilenameFromPath(path))
				{
					cnc3FileManager.SetTargetFolder(path);
				}
				SafeDeleteArray(&path);
			}
			bfpo->Release();
		}
	}
	else
	{
		cnc3FileManager.SetTargetFolder(
			this->FileNavigator->GetRootFolder()
		);
	}

	auto files = cnc3FileManager.Open();
	if (files.Succeeded())
	{
		auto fileCount = files.GetCount();

		for (int i = 0; i < fileCount; i++)
		{
			auto file = files.GetAt(i);

			if (file.Succeeded())
			{
				auto dataCont
					= reinterpret_cast<ApplicationData*>(
						getDefaultApplicationDataContainer()
						);
				if (dataCont != nullptr)
				{
					auto openInNewTab =
						(i > 0)
						? true
						: dataCont->getBooleanData(DATAKEY_SETTINGS_OPEN_IN_NEW_TAB, false);

					this->Tabcontrol->UserRequest_Open(
						file,
						openInNewTab,
						(i == (fileCount - 1)) ? true : false
					);

					// expand path
					auto expPath = dataCont->getBooleanData(DATAKEY_SETTINGS_ALWAYS_EXPAND_PATH, true);
					if (expPath)
					{
						this->FileNavigator->ExpandPathToFile(
							file.GetPath()
						);
					}

					// add to history
					auto saveHist =
						reinterpret_cast<ApplicationData*>(
							getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
							)->getBooleanData(DATAKEY_EXSETTINGS_HISTORY_SAVEHISTORY, true);

					if (saveHist)
					{
						HistoryItem hiItem;
						hiItem.SetActionType(HistoryItem::FILE_OPENED);
						hiItem.SetLastOpenedTime();
						hiItem.SetItemPath(
							file.GetPath()
						);

						this->FileHistory->AddToHistory(hiItem);
						this->FileHistory->Save();
					}
				}
			}
			else
			{
				auto hr = file.GetStatus();

				if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED))
				{
					iString errorMsg(
						getStringFromResource(ERROR_MSG_OPENFILE_FAILED)
					);
					errorMsg += iString::fromHex((uintX)hr);

					DispatchEWINotification(
						EDSP_ERROR,
						L"AC0003",
						errorMsg.GetData(),
						L"IFileOpenDialog"
					);
				}
			}
		}
	}
}

void Application::Save()
{
	if (!this->Tabcontrol->UserRequest_Save(TOS_SAVE))
	{
		this->SaveAs();
	}
}

void Application::SaveAll()
{
	if (!this->Tabcontrol->UserRequest_Save(TOS_SAVEALL))
	{
		DispatchEWINotification(
			EDSP_WARNING, 
			L"TC0003\0",
			getStringFromResource(WARNING_MSG_SAVEALL_NOPATHFOUND),
			getStringFromResource(UI_TABCONTROL));
	}
}

void Application::SaveAs()
{
	TCHAR* path = nullptr;

	auto result = this->Tabcontrol->UserRequest_SaveAs(&path);

	_NOT_USED(result);

	if (path != nullptr)
	{
		auto dataContainer =
			reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer()
			);
		if (dataContainer != nullptr)
		{
			auto expPath = dataContainer->getBooleanData(DATAKEY_SETTINGS_ALWAYS_EXPAND_PATH, true);
			if (expPath)
			{
				this->FileNavigator->ExpandPathToFile(path);
			}
		}

		auto extendedDataContainer =
			reinterpret_cast<ApplicationData*>(
				getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
				);
		if (extendedDataContainer != nullptr)
		{
			auto saveHist =
				extendedDataContainer->getBooleanData(DATAKEY_EXSETTINGS_HISTORY_SAVEHISTORY, true);

			if (saveHist)
			{
				HistoryItem hiItem;
				hiItem.SetActionType(HistoryItem::FILE_OPENED);
				hiItem.SetLastOpenedTime();
				hiItem.SetItemPath(path);

				this->FileHistory->AddToHistory(hiItem);
				this->FileHistory->Save();
			}
		}

		// deprecated (since the filesystem-watcher will do that)
		//if (result == TRUE)// only insert in treeview if the file was newly created (if the file was overwritten the return-value of 'SaveAs' is 'FILE_OVERWRITTEN')
		//{
		//	this->FileNavigator->AddNewFileToView(path, this->AppSetup.expandPathToItem);
		//}

		SafeDeleteArray(&path);
	}
}

void Application::Import(LPCTSTR path)
{
	if (path == nullptr)
	{
		CnC3FileManager cnc3FileManager(this->MainWindow);

		cnc3FileManager.SetDialogText(
			getStringFromResource(UI_IMPORT_FILE_CONTENT),
			getStringFromResource(UI_IMPORT_ACTION),
			nullptr
		);

		auto importElement =
			cnc3FileManager.Import();

		if (importElement.Succeeded())
		{
			this->Tabcontrol->UserRequest_Import(
				importElement.GetNCContent()
			);
		}
		else
		{
			auto hr =
				importElement.GetStatus();

			if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED))
			{
				iString errorMsg(
					getStringFromResource(ERROR_MSG_IMPORT_FAILED)
				);
				errorMsg += iString::fromHex((uintX)hr);

				DispatchEWINotification(
					EDSP_ERROR,
					L"AC0004",
					errorMsg.GetData(),
					L"IFileOpenDialog"
				);
			}
		}
	}
	else
	{
		BasicFPO* bfpo = CreateBasicFPO();
		if (bfpo != NULL)
		{
			TCHAR* fileContent = NULL;

			if (bfpo->LoadBufferFmFile(&fileContent, path))			//CODEPAGE		// use different codepages, selected by the user??
			{
				this->Tabcontrol->UserRequest_Import(fileContent);

				TCHAR* filename = NULL;
				if (bfpo->GetFilenameOutOfPath(path, &filename, FALSE))
				{
					iString string(getStringFromResource(INFO_MSG_FILEWASIMPORTED));
					string.Append(filename);

					DispatchEWINotification(
						EDSP_INFO,
						L"FN0002\0",
						string.getContentReference(),
						getStringFromResource(UI_FILENAVIGATOR)
					);

					SafeDeleteArray(&filename);
				}
				SafeDeleteArray(&fileContent);
			}
			SafeRelease(&bfpo);
		}
	}
}

void Application::OnFileConverted(LPTSTR filename)
{	
	iString string(getStringFromResource(INFO_MSG_FILEWASCONVERTED));
	string.Append(filename);

	DispatchEWINotification(
		EDSP_INFO,
		L"FN0001\0",
		string.getContentReference(), 
		getStringFromResource(UI_FILENAVIGATOR));
}

void Application::Send()
{
	AppPath appPath;
	TCHAR* buffer = nullptr;

	auto isAlreadyOpen =
		(FindWindow(CNCS_DATAX_CLASS, nullptr) != nullptr)
		? true : false;

	if (!isAlreadyOpen)
	{
		if (this->Tabcontrol->GetCurrentTabContent(&buffer))
		{
			auto dataContainer =
				reinterpret_cast<ApplicationData*>(
					getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
					);
			if (dataContainer != nullptr)
			{
				auto res = SerialComm::PrepareBufferForTransmission(
					&buffer,
					dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXPORT_REMOVEBRACKETCOMMENT, false),
					dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXPORT_REMOVEAPOSTROPHECOMMENT, true),
					dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXPORT_REMOVESPACES, false)
				);

				if (res > 0)
				{
					auto commsetuppath = appPath.Get(PATHID_FILE_COMSETUP);

					if (commsetuppath.succeeded())
					{
						auto dataTraffic =
							new SerialComm(
								this->MainWindow,
								this->hInstance,
								commsetuppath.GetData()
							);
						if (dataTraffic != nullptr)
						{
							HWND cBox = this->UserInterface->GetFrameHandles(GFWH_CBOXFRAME);
							if (cBox)
							{
								RECT rc;
								GetWindowRect(cBox, &rc);


								if (dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXCHANGEWND_MONITORTRANSMISSION, true))
								{
									if ((rc.bottom - rc.top) < DPIScale(250))
										rc.top = rc.top - (DPIScale(250) - (rc.bottom - rc.top));
								}
								else
								{
									rc.top = rc.bottom - DPIScale(30);
								}

								dataTraffic->enableVerboseMessaging(
									dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXCHANGEWND_VERBOSETRANSMISSION, false)
								);
								dataTraffic->InitDataTransmission(SEND_BUFFER, buffer, &rc);
							}
						}
					}
				}
			}
			SafeDeleteArray(&buffer);
		}
	}
}

void Application::Receive()
{
	auto isAlreadyOpen =
		(FindWindow(CNCS_DATAX_CLASS, nullptr) != nullptr)
		? true : false;

	if (!isAlreadyOpen)
	{
		AppPath appPath;
		auto commsetuppath = appPath.Get(PATHID_FILE_COMSETUP);

		if (commsetuppath.succeeded())
		{
			SerialComm* dataTraffic = new SerialComm(this->MainWindow, this->hInstance, commsetuppath.GetData());
			if (dataTraffic != nullptr)
			{
				HWND cBox = this->UserInterface->GetFrameHandles(GFWH_CBOXFRAME);
				if (cBox)
				{
					RECT rc;
					GetWindowRect(cBox, &rc);

					auto dataContainer =
						reinterpret_cast<ApplicationData*>(
							getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
							);

					if (dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXCHANGEWND_MONITORTRANSMISSION, true))
					{
						if ((rc.bottom - rc.top) < DPIScale(250))
							rc.top = rc.top - (DPIScale(250) - (rc.bottom - rc.top));
					}
					else
					{
						rc.top = rc.bottom - DPIScale(30);
					}

					dataTraffic->enableVerboseMessaging(
						dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXCHANGEWND_VERBOSETRANSMISSION, false)
					);
					dataTraffic->InitDataTransmission(RECIEVE_BUFFER, nullptr, &rc);
				}
			}
		}
	}
}

void Application::Export()
{
	CnC3File file;
	CnC3FileManager cnc3FileManager(this->MainWindow);

	cnc3FileManager.SetExportFormatHandler(
		dynamic_cast<IExportFormatProtocol*>(this)
	);

	this->Tabcontrol->GetCurrentCnC3File(file);

	if (file.Succeeded())
	{
		cnc3FileManager.SetDialogText(
			L"Export", L"Export",
			file.GetFilename(true)
		);

		auto hr = cnc3FileManager.ExportAs(file);
		if (FAILED(hr))
		{
			if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED))
			{
				iString errorMsg(
					getStringFromResource(ERROR_MSG_EXPORT_FAILED)
				);
				errorMsg +=
					iString::fromHex((uintX)hr);

				DispatchEWINotification(
					EDSP_ERROR,
					L"AC0002",
					errorMsg.GetData(),
					L"IFileSaveDialog"
				);
			}
		}
	}
}

int Application::OnKeydown(WPARAM wParam)
{
	auto ctrlKeyState = GetKeyState(VK_CONTROL);
	auto ctrlKeyIsPressed = (ctrlKeyState & 0x8000) ? true : false;

	auto shiftKeyState = GetKeyState(VK_SHIFT);
	auto shiftKeyIsPressed = (shiftKeyState & 0x800) ? true : false;

	switch (wParam)
	{
	//case VK_MENU:	// ALT
	//	//if(ctrlKeyIsPressed)
	//	//	this->OnOpenAppProperties();
	//	break;
	//case 0x2E:
	//	if (ctrlKeyIsPressed)
	//		return 0;				//TODO: handle close event / update: this is an exception generated by the undostack
	//	break;
	case 0x4E:		// N
		if(ctrlKeyIsPressed)
			this->Tabcontrol->UserRequest_AddNewTab();	// ctrl + N	(new tab)
		break;
	case 0x4F:		// O
		if (ctrlKeyIsPressed)
			this->Open();						// ctrl + O	(open)
		break;
	case 0x53:		// S
		if (ctrlKeyIsPressed)
		{
			if (shiftKeyIsPressed)				// ctrl + shift + S	(save all)
				this->SaveAll();
			else
				this->Save();					// ctrl + S	(save)
		}
		break;
	case 0x54:		// T
		if (ctrlKeyIsPressed)
		{
			if (shiftKeyIsPressed)
				this->LaunchCommandlineTool();		// ctrl + shift + T (open commandline tool)
			else
				this->Send();	// ctrl + T	(send)
		}
		break;
	case 0x52:		// R
		if(ctrlKeyIsPressed)
			this->Receive();	// ctrl + R (receive)
		break;
	case 0x49:		// I
		if (ctrlKeyIsPressed)
			this->Import(nullptr);	// ctrl + I (import)
		break;
	case 0x4D:		// M
		if (ctrlKeyIsPressed)
			this->Export();	// ctrl + M (open sample manager)
		break;
	default:
		// F-Keys for editproperties
		if (wParam >= 0x70 && wParam <= 0x76)
		{
			this->Tabcontrol->UserAction_Keyboard(wParam);
		}
		break;
	}
	return -1;
}

void Application::OnClipboardAction(int actionID)
{
	switch (actionID)
	{
	case IDM_CUT:
		this->Tabcontrol->UserAction_Cut();
		break;
	case IDM_EDITDELETE:
		this->Tabcontrol->UserAction_Clear();
		break;
	case IDM_COPY:
		this->Tabcontrol->UserAction_Copy();
		break;
	case IDM_PASTE:
		this->Tabcontrol->UserAction_Paste();
		break;
	case IDM_SELECTALL:
		this->Tabcontrol->UserAction_SelectAll();
		break;
	default:
		break;
	}
}

void Application::OpenAppProperties()
{
	this->UserInterface->setParameter(
		PARAID_UI_SETTINGSBUTTON_TOUCHDOWNLOCK,
		(LONG_PTR)TRUE
	);

	auto hr =
		this->appProperty->CreatePropertyWindow(
			this->MainWindow,
			this->hInstance
		);
	if (FAILED(hr))
	{
		// TODO: error to c-box!
	}
}

void Application::OnNavigatorOpenRequest(WPARAM wParam, LPARAM lParam)
{
	auto path = reinterpret_cast<LPTSTR>(lParam);
	if (path != nullptr)
	{
 		auto bfpo = CreateBasicFPO();
		if (bfpo != nullptr)
		{
			TCHAR *fileExt = nullptr;
			bool isValid = true;

			if (bfpo->GetFileExtension(path, &fileExt))
			{
				if (CompareStringsB(fileExt, L".cnc3\0"))
				{
					auto openFlags = HIWORD(wParam);

					auto openInNewTab
						= (reinterpret_cast<ApplicationData*>(
							getDefaultApplicationDataContainer())
							)->getBooleanData(DATAKEY_SETTINGS_OPEN_IN_NEW_TAB, true);

					if ((openFlags & FORCE_OPEN_IN_NEW_TAB) && !openInNewTab)
						openInNewTab = true;

					CnC3File file;
					file.Load(path);

					this->Tabcontrol->UserRequest_Open(
						file,
						openInNewTab,
						(openFlags & DO_NOT_SET_FOCUS) ? false : true
					);

					// add to history
					auto saveHist =
						reinterpret_cast<ApplicationData*>(
							getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
							)->getBooleanData(DATAKEY_EXSETTINGS_HISTORY_SAVEHISTORY, true);

					if (saveHist)
					{
						HistoryItem hiItem;
						hiItem.SetActionType(HistoryItem::FILE_OPENED);
						hiItem.SetLastOpenedTime();
						hiItem.SetItemPath(path);

						this->FileHistory->AddToHistory(hiItem);
						this->FileHistory->Save();
					}
				}
				else
				{
					isValid = false;
				}
			}
			else
			{
				isValid = false;
			}

			if (!isValid)
			{
				DispatchEWINotification(
					EDSP_ERROR,
					L"AC0001",
					getStringFromResource(ERROR_MSG_FILETYPE_NOTSUPPORTED),
					L"Application"
				);
			}
			SafeRelease(&bfpo);
		}
	}
}

void Application::Autosave()
{
	auto autosave
		= reinterpret_cast<ApplicationData*>(
			getDefaultApplicationDataContainer()
		)->getBooleanData(DATAKEY_SETTINGS_AUTOSAVE, false);

	if (autosave)
	{
		this->Tabcontrol->UserRequest_Save(TOS_SAVEALL);
	}
}

void Application::SaveSession()
{
	auto dataContainer
		= reinterpret_cast<ApplicationData*>(
			getDefaultApplicationDataContainer()
		);

	if (dataContainer != nullptr)
	{
		auto expCond = dataContainer->getBooleanData(DATAKEY_SETTINGS_SAVE_EXPLORER_COND, true);
		if (expCond)
		{
			this->FileNavigator->SaveCondition();
		}
		auto tabCond = dataContainer->getBooleanData(DATAKEY_SETTINGS_SAVE_TABWND_COND, true);
		if (tabCond)
		{
			this->Tabcontrol->SaveTabCondition();
		}
	}
}

void Application::LaunchWebsite()
{
	ShellExecute(nullptr, nullptr, L"http://cnc-suite.blogspot.com/", nullptr, nullptr, SW_SHOW);
}

void Application::ShowHelpExtension()
{
	auto lang = getCurrentAppLanguage();
	AppPath appPath;
	iString pathToManual;
	Url manualUrl;

	switch (lang)
	{
	case LANG_GERMAN:
		pathToManual = appPath.Get(PATHID_FILE_HELPHTML_GERMAN);
		manualUrl.SetUrlFromLocalPath(
			pathToManual
		);
		ShellExecute(
			nullptr, nullptr,
			manualUrl.GetUrl(),
			nullptr, nullptr,
			SW_SHOW
		);
		break;
	case LANG_ENGLISH:
		pathToManual = appPath.Get(PATHID_FILE_HELPHTML_ENGLISH);
		manualUrl.SetUrlFromLocalPath(
			pathToManual
		);
		ShellExecute(
			nullptr, nullptr,
			manualUrl.GetUrl(),
			nullptr, nullptr,
			SW_SHOW
		);
		break;
	default:
		break;
	}
}

void Application::LaunchCommandlineTool()
{
	auto cmdlntool = new CnCS_CommandLine();
	if (cmdlntool != nullptr)
	{
		CTRLCREATIONSTRUCT ccs;
		ccs.ctrlID = 0;
		ccs.hInst = this->hInstance;
		ccs.parent = this->MainWindow;
		ccs.pos.x = 200;
		ccs.pos.y = 100;
		ccs.size.cx = 600;
		ccs.size.cy = 600;

		cmdlntool->CreatePopUp(&ccs);
	}
}

void Application::ShowHistoryWnd()
{
	if (this->FileHistory->GetEntryCount() > 0)
	{
		RECT rc;
		SPECIALCOLORSTRUCT scs;

		this->SpecialColorForID(this->StyleInfo.StyleID, &scs);

		auto fNav = this->UserInterface->GetFrameHandles(GFWH_TVFRAME);
		GetClientRect(fNav, &rc);

		CTRLCREATIONSTRUCT ccs;
		ccs.ctrlID = 0;
		ccs.hInst = this->hInstance;
		ccs.parent = this->MainWindow;
		ccs.pos = { 0,
			DPIScale(25) };
		ccs.size = {
			rc.right,
			rc.bottom };

		this->FileHistory->SetColors(
			this->StyleInfo.Background,
			scs.normal,
			scs.highlighted,
			scs.text,
			scs.accent_text,
			scs.outline
		);

		this->FileNavigator->Hide();
		this->FileHistory->ShowHistoryWindow(&ccs);
	}
	else
	{
		DispatchEWINotification(
			EDSP_INFO,
			L"HST0001",
			getStringFromResource(INFO_MSG_NOHISTORYCONTENT),
			L"Mainframe"
		);
	}
}

BOOL Application::LoadUserData()
{
	BOOL result = TRUE;

	BasicFPO* pFPO = CreateBasicFPO();
	if (pFPO != nullptr)
	{
		AppPath wnduserPath;
		auto path = wnduserPath.Get(PATHID_FILE_WINDOW_DATA);

		if (path.succeeded())
		{
			WCHAR* buffer = nullptr;

			result = pFPO->LoadBufferFmFileAsUtf8(
				&buffer,
				path.GetData()
			);
			if (result)
			{
				if (buffer != nullptr)
				{
					//__try
					//{
						int i = 0, j = 0, int_num = 0;
						DWORD counter = 0;
						WCHAR number[32] = { 0 };

						while (buffer[counter] != L'\0')
						{
							while (!IsNumber(buffer[counter]))
							{
								if (buffer[counter] == L'\0')
									break;

								counter++;
							}
							if (buffer[counter] == L'\0')
								break;

							while (IsNumber(buffer[counter]))
							{
								if (buffer[counter] == L'\0')
									break;
								
								number[i] = buffer[counter];

								i++;
								counter++;
							}
							number[i] = L'\0';
							i = 0;

							int_num = _wtoi(number);

							switch (j)
							{
							case 0:
								this->AppData.WndSizeData.valid = int_num;
								break;
							case 1:
								this->AppData.WndSizeData.EditFrameHeight = int_num;
								break;
							case 2:
								this->AppData.WndSizeData.TVFrame_width = int_num;
								break;
							case 3:
								this->AppData.WndSizeData.nCmdShow = int_num;
								break;
							case 4:
								this->AppData.WndSizeData.rc_Window.bottom = int_num;
								break;
							case 5:
								this->AppData.WndSizeData.rc_Window.left = int_num;
								break;
							case 6:
								this->AppData.WndSizeData.rc_Window.right = int_num;
								break;
							case 7:
								this->AppData.WndSizeData.rc_Window.top = int_num;
								break;
							default:
								break;
							}
							j++;
							SecureZeroMemory(number, sizeof(number));
						}
					//}
					//__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
					//{
					//	result = FALSE;
					//}
					delete[] buffer;
				}
				else
					result = FALSE;
			}
		}
		else
			result = FALSE;

		SafeRelease(&pFPO);
	}
	else
		result = FALSE;

	return result;
}

BOOL Application::LoadNCSPath()
{
	BOOL result = TRUE;

	BasicFPO* pFPO = CreateBasicFPO();

	result = (pFPO != NULL) ? TRUE : FALSE;
	if (result)
	{
		result = pFPO->GetKnownFolderPath(&this->AppData.UserPath, FOLDERID_Documents);

		SafeRelease(&pFPO);
	}
	return result;
}

BOOL Application::SaveUserData()
{
	BOOL result = TRUE;

	BasicFPO* pFPO = CreateBasicFPO();
	if (pFPO != nullptr)
	{
		AppPath wnduserPath;
		auto path = wnduserPath.Get(PATHID_FILE_WINDOW_DATA);

		if (path.succeeded())
		{
			HRESULT hr;
			WCHAR buffer[256] = { 0 };

			hr = StringCbPrintf(buffer, sizeof(buffer), L"%i|%i|%i|%i|%i|%i|%i|%i|\0",
				this->AppData.WndSizeData.valid,
				this->AppData.WndSizeData.EditFrameHeight,
				this->AppData.WndSizeData.TVFrame_width,
				this->AppData.WndSizeData.nCmdShow,
				this->AppData.WndSizeData.rc_Window.bottom,
				this->AppData.WndSizeData.rc_Window.left,
				this->AppData.WndSizeData.rc_Window.right,
				this->AppData.WndSizeData.rc_Window.top);

			if (SUCCEEDED(hr))
			{
				if (!pFPO->SaveBufferToFileAsUtf8(
					buffer,
					path.GetData()
				))
				{
					result = FALSE;
				}
			}
			else
				result = FALSE;
		}
		else
			result = FALSE;

		SafeRelease(&pFPO);
	}
	else
		result = FALSE;

	return result;
}

BOOL Application::LoadStyleInfo()
{
	BOOL result = TRUE;
	int styleID = 0;

	BasicFPO* pFPO = CreateBasicFPO();
	if (pFPO != nullptr)
	{
		AppPath styleIdPath;
		auto path = styleIdPath.Get(PATHID_FILE_APPSTYLE);

		if (!PathFileExists(path.GetData()))
		{
			pFPO->SaveBufferToFileAsUtf8(
				L"6",
				path.GetData()
			);
			styleID = 6;
		}
		else
		{
			if (path.succeeded())
			{
				WCHAR* buffer = nullptr;

				result = pFPO->LoadBufferFmFileAsUtf8(
					&buffer,
					path.GetData()
				);
				if (result)
				{
					if (buffer != nullptr)
					{
						styleID = _wtoi(buffer);

						delete[] buffer;
					}
				}
			}
		}
		SafeRelease(&pFPO);
	}
	SecureZeroMemory(&this->StyleInfo, sizeof(APPSTYLEINFO));
	this->StyleInfo.StyleID = styleID;
	this->SetAppStyleProperty(styleID);

	return TRUE;
}

BOOL Application::SaveStyleInfo()
{
	BOOL result = TRUE;

	BasicFPO* pFPO = CreateBasicFPO();
	if (pFPO != nullptr)
	{
		AppPath styleIdPath;
		auto path = styleIdPath.Get(PATHID_FILE_APPSTYLE);

		if (path.succeeded())
		{
			HRESULT hr;
			WCHAR buffer[256] = { 0 };

			hr = StringCbPrintf(buffer, sizeof(buffer), L"%i\0", this->StyleInfo.StyleID);
			if (SUCCEEDED(hr))
			{
				if (!pFPO->SaveBufferToFileAsUtf8(
					buffer,
					path.GetData()
				))
				{
					result = FALSE;
				}
			}
			else
				result = FALSE;
		}
		else
			result = FALSE;

		SafeRelease(&pFPO);
	}
	else
		result = FALSE;

	return result;
}

HRESULT Application::CollectFrameData(LPCSTMFRAME FrameData)
{
	if (FrameData == NULL)
		return E_FAIL;
	else
	{
		SecureZeroMemory(FrameData, sizeof(CSTMFRAME));

		if (this->AppData.WndSizeData.valid)
		{
			FrameData->valid = this->AppData.WndSizeData.valid;
			FrameData->nCmdShow = this->AppData.WndSizeData.nCmdShow;
			FrameData->EditFrameHeight = this->AppData.WndSizeData.EditFrameHeight;
			FrameData->TVFrameWidth = this->AppData.WndSizeData.TVFrame_width;
		}
		else
			FrameData->valid = FALSE;
	}
	return S_OK;
}

void Application::SetAppStyleProperty(int StyleID)
{
	if (StyleID == STYLEID_BLACK)
	{
		this->StyleInfo.Background = RGB(100, 100, 100);
		this->StyleInfo.Stylecolor = RGB(0, 0, 0);
		this->StyleInfo.SizeWndColor = RGB(220, 220, 220);
		this->StyleInfo.TextColor = RGB(255, 255, 255);
		this->StyleInfo.OutlineColor = RGB(140, 140, 140);
		this->StyleInfo.TabColor = RGB(40, 40, 40);
		this->StyleInfo.ToolbarbuttonBkgnd = RGB(0, 0, 0);
		this->StyleInfo.MenuPopUpColor = RGB(120, 120, 120);
		this->StyleInfo.specialTextcolor = RGB(255, 128, 64);
		this->StyleInfo.titlebarColor = RGB(80, 80, 80);//RGB(111, 138, 145);
		this->StyleInfo.mainToolbarColor = RGB(150, 150, 150);
	}
	else if (StyleID == STYLEID_DARKGRAY)
	{
		this->StyleInfo.Background = RGB(160, 160, 160);
		this->StyleInfo.Stylecolor = RGB(120, 120, 120);
		this->StyleInfo.SizeWndColor = RGB(230, 230, 230);
		this->StyleInfo.TextColor = RGB(255, 255, 255);
		this->StyleInfo.OutlineColor = RGB(170, 170, 170);// 150 old
		this->StyleInfo.TabColor = RGB(100, 100, 100);
		this->StyleInfo.ToolbarbuttonBkgnd = RGB(80, 80, 80);
		this->StyleInfo.MenuPopUpColor = RGB(80, 80, 80);			// pop-up menu is too dark!?
		this->StyleInfo.specialTextcolor = RGB(255, 190, 50);
		this->StyleInfo.titlebarColor = RGB(150, 150, 150);
		this->StyleInfo.mainToolbarColor = RGB(180, 180, 180);
	}
	else if (StyleID == STYLEID_GREEN)
	{
		this->StyleInfo.Background = RGB(150, 210, 210);
		this->StyleInfo.Stylecolor = RGB(80, 160, 160);
		this->StyleInfo.SizeWndColor = RGB(0, 255, 0);
		this->StyleInfo.TextColor = RGB(255, 255, 255);
		this->StyleInfo.OutlineColor = RGB(50, 100, 100);
		this->StyleInfo.TabColor = RGB(121, 176, 186);
		this->StyleInfo.ToolbarbuttonBkgnd = RGB(40, 90, 90);
		this->StyleInfo.MenuPopUpColor = RGB(20, 120, 120);
		this->StyleInfo.specialTextcolor = RGB(255, 255, 0);
		this->StyleInfo.titlebarColor = RGB(111, 138, 145);
		this->StyleInfo.mainToolbarColor = RGB(180, 180, 180);
	}
	else if (StyleID == STYLEID_LIGHTGRAY)
	{
		this->StyleInfo.Background = RGB(150, 150, 150);
		this->StyleInfo.Stylecolor = RGB(220, 220, 220);
		this->StyleInfo.SizeWndColor = RGB(0, 0, 0);
		this->StyleInfo.TextColor = RGB(0, 0, 0);
		this->StyleInfo.OutlineColor = RGB(128, 128, 128);
		this->StyleInfo.TabColor = RGB(200, 200, 200);
		this->StyleInfo.ToolbarbuttonBkgnd = RGB(100, 100, 100);
		this->StyleInfo.MenuPopUpColor = RGB(180, 180, 180);
		this->StyleInfo.specialTextcolor = RGB(50, 50, 50);
		this->StyleInfo.titlebarColor = RGB(150, 150, 150);
		this->StyleInfo.mainToolbarColor = RGB(210, 210, 210);
	}
	else
	{
		this->StyleInfo.StyleID = STYLEID_LIGHTGRAY;
		this->StyleInfo.Background = RGB(100, 100, 100);
		this->StyleInfo.Stylecolor = RGB(220, 220, 220);
		this->StyleInfo.SizeWndColor = RGB(0, 0, 0);
		this->StyleInfo.TextColor = RGB(0, 0, 0);
		this->StyleInfo.OutlineColor = RGB(60, 60, 60);
		this->StyleInfo.TabColor = RGB(200, 200, 200);
		this->StyleInfo.ToolbarbuttonBkgnd = RGB(100, 100, 100);
		this->StyleInfo.MenuPopUpColor = RGB(180, 180, 180);
		this->StyleInfo.specialTextcolor = RGB(255, 128, 64);
		this->StyleInfo.titlebarColor = RGB(150, 150, 150);
		this->StyleInfo.mainToolbarColor = RGB(180, 180, 180);
	}
}

void Application::ValidateWindowSizeData()
{
	bool isValid = false;

	auto cx = GetSystemMetrics(SM_CXSCREEN);
	auto cy = GetSystemMetrics(SM_CYSCREEN);

	if (this->AppData.WndSizeData.valid)
	{
		if ((this->AppData.WndSizeData.rc_Window.left >= 0) && (this->AppData.WndSizeData.rc_Window.top >= 0))
		{
			if ((this->AppData.WndSizeData.rc_Window.right - this->AppData.WndSizeData.rc_Window.left) <= cx)
			{
				if ((this->AppData.WndSizeData.rc_Window.bottom - this->AppData.WndSizeData.rc_Window.top) <= cy)
				{
					if ((this->AppData.WndSizeData.nCmdShow == SW_SHOW) || (this->AppData.WndSizeData.nCmdShow == SW_SHOWMAXIMIZED))
					{
						if (this->AppData.WndSizeData.EditFrameHeight > 50)
						{
							if (this->AppData.WndSizeData.TVFrame_width > 50)
							{
								isValid = true;
							}
						}
					}
				}
			}
		}
	}
	if (!isValid)
	{
		this->setDefaultWindowSizeData();
	}
}

void Application::setDefaultWindowSizeData()
{
	this->AppData.WndSizeData.rc_Window.left = 50;
	this->AppData.WndSizeData.rc_Window.top = 50;
	this->AppData.WndSizeData.rc_Window.right = GetSystemMetrics(SM_CXSCREEN) - 100;
	this->AppData.WndSizeData.rc_Window.bottom = GetSystemMetrics(SM_CYSCREEN) - 100;
	this->AppData.WndSizeData.nCmdShow = SW_SHOWMAXIMIZED;
	this->AppData.WndSizeData.valid = MAXIMIZENORMAL;
	this->AppData.WndSizeData.TVFrame_width = this->AppData.WndSizeData.rc_Window.right / 4;
	this->AppData.WndSizeData.EditFrameHeight = (this->AppData.WndSizeData.rc_Window.bottom / 5) * 4;

	// ....

	this->SaveUserData();
}

void Application::ChangeAppStyle(int StyleID)
{
	this->StyleInfo.StyleID = StyleID;
	this->SetAppStyleProperty(StyleID);
	this->SaveStyleInfo();

	SendMessage(this->MainWindow, WM_SETAPPSTYLE, 0, reinterpret_cast<LPARAM>(&this->StyleInfo));

	// change style in auxilary windows (popups)

	HWND samplemanager = FindWindow(SAMPLEMANAGER_WINDOWCLASS, nullptr);
	if (samplemanager)
	{
		SendMessage(samplemanager, WM_SETAPPSTYLE, 0, reinterpret_cast<LPARAM>(&this->StyleInfo));
	}

}

void Application::ShowSplashScreen()
{
	// at first show the splash screen to indicate that the data will be loaded
	// and inform the user what will be loaded
	this->splScreen = new splashScreen(this->hInstance);
	if (this->splScreen != nullptr)
	{
		this->splScreen->Start();

		Sleep(500); // REMOVE !!!!
	}
}

void Application::RestartApplication(DWORD restartOption)
{
	this->restartOptions = restartOption;
	SendMessage(this->MainWindow, WM_CLOSE, 0, 0);
}

BOOL Application::CheckForUpdates()
{
	if (reinterpret_cast<ApplicationData*>(
		getDefaultApplicationDataContainer()
		)->getBooleanData(DATAKEY_SETTINGS_SEARCHFORUPDATES, true))
	{
		return this->appProperty->CheckForUpdates();
	}
	else
		return TRUE;
}

void Application::SetDefaultAppSettings()
{
	// by now there are no settings ...
}

void Application::SetDefaultRestoreFrame()
{
	if (this->MainWindow != nullptr)
	{
		WINDOWPLACEMENT wpi;
		wpi.length = sizeof(WINDOWPLACEMENT);
		wpi.rcNormalPosition = this->AppData.WndSizeData.rc_Window;
		wpi.showCmd = this->AppData.WndSizeData.nCmdShow;

		SetWindowPlacement(this->MainWindow, &wpi);
	}
}

void Application::SpecialColorForID(int ID, LPSPECIALCOLORSTRUCT pscs)
{
	if (ID == STYLEID_BLACK)
	{
		pscs->normal = RGB(40, 40, 40);
		pscs->selected = RGB(100, 100, 100);
		pscs->pressed = RGB(70, 70, 70);
		pscs->highlighted = RGB(100, 100, 100);
		pscs->accent_norm = RGB(220, 100, 20);
		pscs->text = RGB(240, 240, 240);
		pscs->accent_text = RGB(220, 100, 20);
		pscs->outline = RGB(180, 180, 180);
	}
	else if (ID == STYLEID_DARKGRAY)
	{
		pscs->normal = RGB(100, 100, 100);
		pscs->selected = RGB(150, 150, 150);
		pscs->pressed = RGB(110, 110, 110);
		pscs->highlighted = RGB(150, 150, 150);
		pscs->accent_norm = RGB(240, 120, 20);
		pscs->text = RGB(255, 255, 255);
		pscs->accent_text = RGB(240, 120, 20);
		pscs->outline = RGB(220, 220, 200);
	}
	else// must be STYLEID_LIGHTGRAY !
	{
		pscs->normal = RGB(220, 220, 220);
		pscs->selected = RGB(190, 190, 190);
		pscs->pressed = RGB(160, 160, 160);
		pscs->highlighted = RGB(190, 190, 190);
		pscs->accent_norm = RGB(140, 140, 140);
		pscs->text = RGB(0, 0, 0);
		pscs->accent_text = RGB(100, 100, 100);
		pscs->outline = RGB(100, 100, 100);
	}
}
