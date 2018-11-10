#pragma once
#include"external.h"
#include"CnCSuite_Userinterface.h"
#include"CnCSuite_Tabcontrol.h"
#include"CnCSuite_FileNavigator.h"
#include"CnCSuite_Property.h"
#include"CnCSuite_CBox.h"
#include"UpdateAgent.h"
#include"splashScreen.h"
#include"HelperF.h"
#include"BasicFPO.h"
#include"DPI_Helper.h"
#include"history.h"
#include"CnC3FileManager.h"

class Application
	:public IFileSystemModificationProtocoll, public IHistroyEventProtocoll, public IExportFormatProtocol
{

public:			
	Application(HINSTANCE);
	~Application();

	void Release() {
		delete this;
	}

	HRESULT Init_Data();
	HRESULT Init_Application();
	int Run();
	BOOL CHK_Mutex(LPTSTR);

	LanguageID getLanguage();

	LONG_PTR getPropertyComponent();
	LONG_PTR getFileExplorerComponent();
	LONG_PTR getTabControlComponent();
	LONG_PTR getCBoxComponent();

	HWND GetMainWindowHandle() {
		return this->MainWindow;
	}

	void ChangeAppStyle(int styleID);
	void ShowSplashScreen();

	void RestartApplication(DWORD restartOption);

	DWORD GetRestartOption() {
		return this->restartOptions;
	}

	// IFileSystemModificationProtocoll Base
	void Application::onFilesysItemCreated(cObject sender, LPFILESYSTEMOBJECT fso){
		UNREFERENCED_PARAMETER(sender);
		UNREFERENCED_PARAMETER(fso);
	}
	void Application::onFilesysItemDeleted(cObject sender, LPFILESYSTEMOBJECT fso) {
		UNREFERENCED_PARAMETER(sender);
		this->Tabcontrol->FileSystemChanged(fso);
	}
	void Application::onFilesysItemMoved(cObject sender, LPFILESYSTEMOBJECT fso) {
		UNREFERENCED_PARAMETER(sender);
		this->Tabcontrol->FileSystemChanged(fso);
	}
	void Application::onFilesysItemRenamed(cObject sender, LPFILESYSTEMOBJECT fso) {
		UNREFERENCED_PARAMETER(sender);
		this->Tabcontrol->FileSystemChanged(fso);
		this->FileHistory->OnFilesystemModification(fso);
	}

	// IHistroyEventProtocoll Base
	void OnEntryClicked(cObject sender, HistoryItem* item) {
	
		MessageBox(
			this->MainWindow,			
			item->GetItemPath().GetData(),
			item->GetDisplayName().GetData(),
			MB_OK
		);
	}
	void OnWindowClosed(cObject sender) {
		this->FileNavigator->Show();
	}

	// IExportFormatProtocol Base
	void FormatForExport(const CnC3File& file, iString& buffer_out);

private:
	HINSTANCE hInstance;
	HWND MainWindow;
	HANDLE hMutex;
	HHOOK keyboardHook;

	LanguageID langID;
	DWORD restartOptions;

	APPLICATION_DATA AppData;
	APPSTYLEINFO StyleInfo;

	iString NavigationFolder;

	CnCSuite_Userinterface* UserInterface;
	CnCSuite_Tabcontrol* Tabcontrol;
	CnCSuite_FileNavigator* FileNavigator;
	CnCSuite_Property* appProperty;
	CnCSuite_CBox* CBox;
	
	UIHistory* FileHistory;

	splashScreen* splScreen;

	HRESULT Init_Mainframe();
	HRESULT Init_Components();

	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK KeyboardProc(int, WPARAM, LPARAM);

	LRESULT OnCommand(HWND, WPARAM, LPARAM);
	LRESULT OnFrameChanged(HWND,LPARAM);
	LRESULT OnSessionEnd(LPARAM);
	LRESULT OnQueryEndSession(LPARAM);
	LRESULT OnGetAppData(LPARAM);
	LRESULT OnCopyData(LPARAM);
	LRESULT OnDisplayError(LPARAM);
	LRESULT OnStyleInfo(LPARAM);
	LRESULT OnTimerEvent(HWND,WPARAM);
	LRESULT OnValidateError(LPARAM);
	LRESULT OnStatusbarUpdate(LPARAM);
	LRESULT OnClose();
	LRESULT OnTransmissionComplete(LPARAM);
	LRESULT OnCleanUp(WPARAM, LPARAM);
	LRESULT OnSearchResult(LPARAM);
	LRESULT OnGetTabCtrlProperty(LPARAM);
	LRESULT OnGetCurrentSelection(LPARAM);
	LRESULT OnDispatchText(LPARAM);
	LRESULT OnGetDescriptions(LPARAM);
	LRESULT OnSetDescriptions(LPARAM);
	LRESULT OnGetEditControlProperties(WPARAM, LPARAM);
	LRESULT OnUpdateEditColors(WPARAM);
	LRESULT OnInternalCommand(WPARAM, LPARAM);
	LRESULT OnDPIChanged(WPARAM, LPARAM);
	LRESULT OnRestartApp(WPARAM);

	void Open();
	void Save();
	void SaveAll();
	void SaveAs();
	void Import(LPCTSTR);
	void OnFileConverted(LPTSTR);
	//void OnCut();
	void Send();
	void Receive();
	void Export();
	int OnKeydown(WPARAM);
	void OnClipboardAction(int);
	void OpenAppProperties();
	void OnNavigatorOpenRequest(WPARAM, LPARAM);
	void Autosave();
	void SaveSession();
	void LaunchWebsite();
	void ShowHelpExtension();
	void LaunchCommandlineTool();
	void ShowHistoryWnd();

	BOOL LoadUserData();
	BOOL LoadNCSPath();
	BOOL SaveUserData();

	BOOL LoadStyleInfo();
	BOOL SaveStyleInfo();

	HRESULT CollectFrameData(LPCSTMFRAME);

	void SetAppStyleProperty(int);
	void ValidateWindowSizeData();
	void setDefaultWindowSizeData();

	BOOL CheckForUpdates();

	void SetDefaultAppSettings();
	void SetDefaultRestoreFrame();
	void SpecialColorForID(int ID, LPSPECIALCOLORSTRUCT pscs);
};