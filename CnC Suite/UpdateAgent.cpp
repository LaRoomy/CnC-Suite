#include"UpdateAgent.h"

UpdateAgent::UpdateAgent(HINSTANCE hInst)
	: hInstance(hInst),
	eventHandler(nullptr),
	latestVersion(0)
{}

UpdateAgent::~UpdateAgent()
{
	// ...
}

BOOL UpdateAgent::StartUpdateSearch(BOOL deleteAfterExecution)
{
	HANDLE hThread;
	DWORD dwThreadID;
	THREADDATA1 tda;
	tda._delete_ = deleteAfterExecution;
	tda.toClass = reinterpret_cast<LONG_PTR>(this);

	hThread = CreateThread(NULL, 0, UpdateAgent::_updateSearchProc, reinterpret_cast<LPVOID>(&tda), NULL, &dwThreadID);
	if (hThread != NULL)
	{
		WaitForSingleObject(hThread, 100);		
	}
	else
		return FALSE;

	return TRUE;
}

BOOL UpdateAgent::StartUpdateProgramm()
{
	return TRUE;
}

void UpdateAgent::setEventHandler(updateAgentEventSink * handler)
{
	this->eventHandler = handler;
}

int UpdateAgent::getLatestVersion()
{
	return this->latestVersion;
}

DWORD UpdateAgent::_updateSearchProc(LPVOID lParam)
{
	DWORD exitCode = 0;

	LPTHREADDATA1 pda = reinterpret_cast<LPTHREADDATA1>(lParam);
	if (pda != NULL)
	{
		BOOL deleteAfterExecution = pda->_delete_;
		UpdateAgent* pthis = reinterpret_cast<UpdateAgent*>(pda->toClass);

		if (pthis != NULL)
		{
			BOOL update;

			if ((update = pthis->CheckForUpdates()) == FALSE)
			{
				exitCode = 1;
			}
			else
			{
				if (update == UPDATEAVAILABLE)
				{
					if (pthis->eventHandler != nullptr)
					{
						pthis->eventHandler->onUpdateSearchComplete(true);
					}
				}
				else
				{
					if (pthis->eventHandler != nullptr)
					{
						pthis->eventHandler->onUpdateSearchComplete(false);
					}
				}
			}
			if (deleteAfterExecution)
			{
				pthis->Release();
			}
		}
	}
	return exitCode;
}

BOOL UpdateAgent::CheckForUpdates()
{
	__try
	{
		BOOL result = FALSE;

		HINTERNET iSession;

		iSession = InternetOpen(L"CnC_Suite_UpdateAgent\0", 0, NULL, NULL, 0);

		result = (iSession != NULL) ? TRUE : FALSE;
		if (result)
		{
			HINTERNET iUrl = InternetOpenUrl(
				iSession,
				UPDATESEARCHLOCATION,
				NULL,
				0,
				0,
				0);

			result = (iUrl != NULL) ? TRUE : FALSE;
			if (result)
			{
				TCHAR *buffer = NULL;
				DWORD numberOfBytesRead = 0;
				char reception_buffer[1024] = { '\0' };

				result = InternetReadFile(iUrl, reception_buffer, 1024, &numberOfBytesRead);
				if (result)
				{
					result = ConvertCHARtoWCHAR(reception_buffer, &buffer);
					if (result == TRUE)
					{
						int i = 0;
						TCHAR version[20] = { 0 };

						//showText(buffer);

						while (buffer[i] != L'|')
						{
							if (buffer[i] == L'\0')
								break;
							else if (i == 10)
								break;


							version[i] = buffer[i];
							i++;
						}
						version[i] = L'\0';

						this->latestVersion = _wtoi(version);

						//show_integer(latestVersion, AppVersion);

						if (this->latestVersion > AppVersion)
						{
							result = UPDATEAVAILABLE;
						}
						SafeDeleteArray(&buffer);
					}
				}
				InternetCloseHandle(iUrl);
			}
			else
			{
				// get the result of the function InternetOpenUrl(....!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

				if (this->eventHandler != nullptr)
				{
					this->eventHandler->onUpdateSearchFailed(UAERR_NOINTERNETCONNECTION);
				}
			}
			InternetCloseHandle(iSession);
		}
		else
		{
			if (this->eventHandler != nullptr)
			{
				this->eventHandler->onUpdateSearchFailed(UAERR_NOINTERNETCONNECTION);	// ??? if there is no connection InternetOpen returns a valid handle anyway, ????
			}
		}
		return result;
	}
	__except ((
		GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ||
		GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ||
		GetExceptionCode() == EXCEPTION_BREAKPOINT)
		? EXCEPTION_EXECUTE_HANDLER
		: EXCEPTION_CONTINUE_SEARCH)
	{
		return FALSE;
	}
}

