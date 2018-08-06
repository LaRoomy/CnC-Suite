#pragma once
#include"external.h"
#include"HelperF.h"
#include"Error dispatcher.h"

#define		UPDATEAVAILABLE		2

#define		UPDATESEARCHLOCATION		L"https://drive.google.com/uc?export=download&id=0B0BVwkRdt3xkZHJaUzljTXFfTEk\0"

#define		UAERR_NOINTERNETCONNECTION		5

typedef struct {
	BOOL _delete_;
	LONG_PTR toClass;
}THREADDATA1, *LPTHREADDATA1;

class updateAgentEventSink
{
public:
	virtual void onUpdateSearchComplete(bool isUpdateAvailable) = 0;
	virtual void onUpdateSearchFailed(int errorcode) = 0;
};

class UpdateAgent
{
public:
	UpdateAgent(HINSTANCE);
	~UpdateAgent();

	void Release() { delete this; }

	BOOL StartUpdateSearch(BOOL deleteAfterExecution);
	BOOL StartUpdateProgramm();

	void setEventHandler(updateAgentEventSink* handler);

	int getLatestVersion();

private:
	HINSTANCE hInstance;

	int latestVersion;

	updateAgentEventSink* eventHandler;

	static DWORD WINAPI _updateSearchProc(LPVOID);
	
	BOOL CheckForUpdates();
};
