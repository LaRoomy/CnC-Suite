#include"Application.h"
#include"LanguageDispatcher.h"
#include"ApplicationData.h"
#include"CommonControls\ItemCollection.h"
#include"AppPath.h"

Application* pApplication = nullptr;
ApplicationData* ApplicationDataContainer = nullptr;
itemCollection<ApplicationData> extendedApplicationDataContainer;
DPI_Assist* dpiAssist = nullptr;
CRITICAL_SECTION CriticalSection;

// Application entry:
extern int APIENTRY _tWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInst,
	_In_ LPTSTR nCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInst);
	UNREFERENCED_PARAMETER(nCmdShow);

	SetUnhandledExceptionFilter(lpTopLevelExceptionFilter);
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	auto notused = InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x00000400);
	_NOT_USED(notused);

	int result = I_ERROR_DPI_FAILED;
	DWORD restart = NO_RESTART;

	dpiAssist = new DPI_Assist();
	if (dpiAssist != nullptr)
	{
		pApplication = new Application(hInstance);
		if (pApplication != nullptr)
		{
			result = pApplication->CHK_Mutex(nCmdLine);
			if (result)
			{
				pApplication->ShowSplashScreen();

				auto hr = pApplication->Init_Data();
				if (SUCCEEDED(hr))
				{
					hr = pApplication->Init_Application();
					if (SUCCEEDED(hr))
					{
						result
							= pApplication->Run();
					}
					else
						result = I_ERROR_INIT_FAILED;
				}
				else
					result = I_ERROR_DATA_FAILED;

				// TODO: wait for all threads of the process to finish, because if the threads are terminated, the data will be lost!
			}
			restart = pApplication->GetRestartOption();

			SafeRelease(&pApplication);//delete pApplication;
		}
		else
			result = I_ERROR_PAPP_INVALID;

		SafeRelease(&dpiAssist);
	}
	SafeRelease(&ApplicationDataContainer);
	DeleteCriticalSection(&CriticalSection);

	if (restart != NO_RESTART)
	{
		ScheduleRestart(restart);
	}
	return result;
}

TCHAR * getStringFromResource(int ID)
{
	// TODO: This method would be very faster if the LanguageDispatcher would be static and not created every invokation of this method

	if (pApplication != nullptr)
	{
		int lang = pApplication->getLanguage();

		LanguageDispatcher dispatcher(lang);

		return dispatcher.getLangString(ID);
	}
	else
	{
		int lang = getSystemLanguage();

		LanguageDispatcher dispatcher(lang);

		return dispatcher.getLangString(ID);
	}
}

int getCurrentAppLanguage()
{
	return pApplication->getLanguage();
}

HRESULT initDefaultApplicationDataContainer()
{
	if (ApplicationDataContainer == nullptr)
	{
		ApplicationDataContainer = new ApplicationData();
		ApplicationDataContainer->selectFilekey(FILEKEY_DEFAULT);
		return (ApplicationDataContainer != nullptr) ? S_OK : E_FAIL;
	}
	else
	{
		return E_POINTER;
	}
}

LONG_PTR getDefaultApplicationDataContainer()
{
	return reinterpret_cast<LONG_PTR>(ApplicationDataContainer);
}

HRESULT initExtendedApplicationDataContainer()
{
	HRESULT hr = S_OK;

	if (extendedApplicationDataContainer.GetCount() > 0)
	{
		extendedApplicationDataContainer.Clear();
	}
	auto bfpo = CreateBasicFPO();

	hr = (bfpo != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		LPDIRLIST list = nullptr;

		AppPath appPath;
		auto settingsPath =
			appPath.Get(PATHID_FOLDER_CNCSUITE_APPDATA_SETTINGS
			);

		if (settingsPath.succeeded())
		{
			auto size = _dirContentListing(settingsPath.GetData(), &list, INCLUDE_FILES);
			if (size > 0)
			{
				for (int i = 0; i < size; i++)
				{
					TCHAR *ext = nullptr;

					if (bfpo->RemoveFileExtension(list[i].filename, &ext))
					{
						if (CompareStringsB(ext, L".xml"))
						{
							// stack the filekeys
							// and exclude the default-file (cncsuitesettings.xml)

							if (!CompareStringsB(FILEKEY_DEFAULT, list[i].filename))
							{
								ApplicationData appData;

								if (appData.selectFilekey((FILEKEY)list[i].filename))
								{
									extendedApplicationDataContainer.AddItem(appData);
								}
							}
						}
						SafeDeleteArray(&ext);
					}
				}
				SafeDeleteArray(&list);
			}
		}
		SafeRelease(&bfpo);
	}
	return hr;
}

LONG_PTR getApplicationDataContainerFromFilekey(LPVOID key)
{
	if (extendedApplicationDataContainer.GetCount() > 0)
	{
		for (int i = 0; i < extendedApplicationDataContainer.GetCount(); i++)
		{
			auto appData = extendedApplicationDataContainer.getObjectCoreReferenceAt(i);
			if (appData != nullptr)
			{
				if (_compareFileKey((FILEKEY)key, appData->getFilekey()))
				{
					return reinterpret_cast<LONG_PTR>(appData);
				}
			}
		}
	}
	ApplicationData _adata;
	_adata.selectFilekey((FILEKEY)key);

	extendedApplicationDataContainer.AddItem(_adata);

	auto pAdata = extendedApplicationDataContainer.getObjectCoreReferenceAt(
		extendedApplicationDataContainer.GetCount() - 1
	);

	if (pAdata != nullptr)
	{
		if (_compareFileKey((FILEKEY)key, pAdata->getFilekey()))
		{
			return reinterpret_cast<LONG_PTR>(pAdata);
		}
	}
	return (LONG_PTR)nullptr;
}

void changeAppStyle(int styleID)
{
	pApplication->ChangeAppStyle(styleID);
}

LONG_PTR getApplicationPropertyComponent()
{
	return pApplication->getPropertyComponent();
}

LONG_PTR getComponentHandle(DWORD component_ID)
{
	switch (component_ID)
	{
	case COMP_ID_FILE_EXPLORER:
		return pApplication->getFileExplorerComponent();
	case COMP_ID_ERROR_BOX:
		return pApplication->getCBoxComponent();
	case COMP_ID_PROPERTIES:
		return pApplication->getPropertyComponent();
	case COMP_ID_TAB_CONTROL:
		return pApplication->getTabControlComponent();
	case COMP_ID_HISTORY:
		return pApplication->getHistoryComponent();
	default:
		return LONG_PTR(NULL);		
	}
}

LONG_PTR getDPIAssist()
{
	return reinterpret_cast<LONG_PTR>(dpiAssist);
}

int DPIScale(int val)
{
	auto _dpiAssist =
		reinterpret_cast<DPI_Assist*>(
			getDPIAssist()
		);
	if (_dpiAssist != nullptr)
	{
		return _dpiAssist->Scale(val);
	}
	else
	{
		return 0;	// return val or -1 if fail ?!
	}
}

HFONT CreateScaledFont(int fontHeight, int fontWeight, LPCTSTR fontName)
{
	auto _dpiAssist =
		reinterpret_cast<DPI_Assist*>(
			getDPIAssist()
			);
	if (_dpiAssist != nullptr)
	{
		LOGFONT lf;
		SecureZeroMemory(&lf, sizeof(LOGFONT));

		lf.lfQuality = CLEARTYPE_QUALITY;
		lf.lfHeight = (_dpiAssist->Scale(fontHeight));
		lf.lfWeight = fontWeight;

		auto hr = StringCbCopy(lf.lfFaceName, sizeof(TCHAR)* LF_FACESIZE, fontName);
		if (SUCCEEDED(hr))
		{
			return CreateFontIndirect(&lf);
		}
		else
			return nullptr;
	}
	else
	{
		return CreateFont(
			fontHeight,
			0, 0, 0,
			fontWeight,
			FALSE, FALSE, FALSE,
			ANSI_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			FF_DONTCARE,
			fontName
		);
	}
}

HFONT CreateScaledUnderlinedFont(int fontHeight, int fontWeight, LPCTSTR fontName)
{
	auto _dpiAssist =
		reinterpret_cast<DPI_Assist*>(
			getDPIAssist()
			);
	if (_dpiAssist != nullptr)
	{
		LOGFONT lf;
		SecureZeroMemory(&lf, sizeof(LOGFONT));

		lf.lfQuality = CLEARTYPE_QUALITY;
		lf.lfHeight = (_dpiAssist->Scale(fontHeight));
		lf.lfWeight = fontWeight;
		lf.lfUnderline = TRUE;

		auto hr = StringCbCopy(lf.lfFaceName, sizeof(TCHAR)* LF_FACESIZE, fontName);
		if (SUCCEEDED(hr))
		{
			return CreateFontIndirect(&lf);
		}
		else
			return nullptr;
	}
	else
	{
		return CreateFont(
			fontHeight,
			0, 0, 0,
			fontWeight,
			FALSE, TRUE, FALSE,
			ANSI_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			FF_DONTCARE,
			fontName
		);
	}
}

bool isCnC3Path(LPCTSTR path)
{
	TCHAR *ext = nullptr;
	bool result = false;

	auto bfpo = CreateBasicFPO();
	if (bfpo != nullptr)
	{
		if (bfpo->GetFileExtension(path, &ext) == TRUE)
		{
			result =
				CompareStringsB(ext, L".cnc3")
				? true : false;

			SafeDeleteArray(&ext);
		}
		SafeRelease(&bfpo);
	}
	return result;
}

LONG WINAPI lpTopLevelExceptionFilter(_EXCEPTION_POINTERS * exceptionInfo)
{
	if (exceptionInfo != nullptr)
	{
		if (exceptionInfo->ExceptionRecord != nullptr)
		{
			iString exInfo(L"CnC Suite ExceptionInfo:\r\n\r\nEventTime: [");

			DateTime time;
			time.FromLocalTime();

			exInfo += time.PreciseTimeAsString();
			exInfo += L"] : [";
			exInfo += time.SimpleDateAsString();
			exInfo += L"]";
			exInfo += L"\r\n\r\n";
			exInfo += L"ExceptionCode: ";
			exInfo += iString::fromHex(exceptionInfo->ExceptionRecord->ExceptionCode);
			exInfo += L"\r\nExceptionAddress: ";
			exInfo += iString::fromHex((uintX)(exceptionInfo->ExceptionRecord->ExceptionAddress));
			exInfo += L"\r\nExceptionFlags: ";
			exInfo += iString::fromHex(exceptionInfo->ExceptionRecord->ExceptionFlags);
			exInfo += L"\r\nWasNested: ";
			if (exceptionInfo->ExceptionRecord->ExceptionRecord != nullptr)
				exInfo += L"true";
			else
				exInfo += L"false";

			AppPath appPath;
			auto exceptPath = appPath.Get(PATHID_FILE_EXCEPTIONINFO);

			auto bfpo = CreateBasicFPO();
			if (bfpo != nullptr)
			{
				bfpo->SaveBufferToFileAsUtf8(
					exInfo.GetData(),
					exceptPath.GetData()
				);
				SafeRelease(&bfpo);
			}
		}
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

void ScheduleRestart(DWORD restartOption)
{
	UNREFERENCED_PARAMETER(restartOption);

#ifdef COMPILE_FOR_WINSTORE_DISTRIBUTION
	// the path for the executable is not redirected in a store installation, so relaunch the current module path

	TCHAR path_buffer[4096] = { 0 };

	auto nSize = GetModuleFileName(nullptr, path_buffer, (DWORD)4096);
	if (nSize > 0) {

		auto bfpo = CreateBasicFPO();
		if (bfpo != nullptr) {

			if (bfpo->RemoveFilenameFromPath(path_buffer))
			{
				ShellExecute(
					nullptr,
					L"open",
					EXECUTABLE_NAME,
					nullptr,
					path_buffer,
					SW_SHOW
				);
				SafeRelease(&bfpo);
			}
		}
	}
#else
	AppPath appPath;

	auto exe_path =
		appPath.Get(PATHID_FOLDER_CNCSUITE_EXECUTABLE_FOLDER);

	ShellExecute(
		nullptr,
		L"open",
		EXECUTABLE_NAME,
		nullptr,
		exe_path.GetData(),
		SW_SHOW
	);
#endif
}

BOOL GetApplicationStyleInformation(LPAPPSTYLEINFO pSInfo)
{
	auto mWnd = pApplication->GetMainWindowHandle();
	if (mWnd != nullptr)
	{
		return (BOOL)SendMessage(mWnd, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(pSInfo));
	}
	return FALSE;
}

LPCRITICAL_SECTION GetCriticalSection()
{
	return &CriticalSection;
}

