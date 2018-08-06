#pragma once
#include"..\external.h"
#include"..\OpenSave\Open_Save_CTRL.h"

#ifndef _SEARCHCTRL_INCLUDED_
#define _SEARCHCTRL_INCLUDED_


#define		NO_MATCH			0x0
#define		FILENAME_MATCH		0x01
#define		PROJECT_MATCH		0x02
#define		DESC_1_MATCH		0x04
#define		DESC_2_MATCH		0x08
#define		DESC_3_MATCH		0x10

#define		SEARCHRESULTPRIORITY_HIGH	0x20
#define		SEARCHRESULTPRIORITY_MID	0x40
#define		SEARCHRESULTPRIORITY_LOW	0x80

#define		SC_ERROR_MAXIMUMRESULTSEXEEDED		0x100
#define		SC_ERROR_NOSEARCHABLEDIRECTORY		0x200
#define		SC_ERROR_RUNNINGOUTOFMEMORY			0x400
#define		SC_ERROR_INVALIDHANDLEDATATYPE		0x800

#define		RSLT_TYPE_EMPTYDIR					0x1000
#define		RSLT_TYPE_DIRECTORY					0x2000
#define		RSLT_TYPE_FILE						0x4000
#define		RSLT_INVALID						0x8000

#define		ID_SEARCHEDIT		2341
#define		ID_SEARCHLIST		2342
#define		ID_SEARCHOPEN		2343
#define		ID_STARTSEARCHING	2344
#define		ID_SEARCHPROGRESS	2345

#define		ID_EDITSUBCLASS		2349

#define		UNIC_PATH			( 32767 * sizeof( TCHAR ))

#define		PRINTMAXRESULT		96
#define		PRINTRESULT			97
#define		PRINTINPUTSTRING	98
#define		CLEARPRINTAREA		99

#define		CU_DELETESEARCHCLASS	120
#define		CU_DELETEPROGRESSBAR	121

typedef struct{
	TCHAR DESC_1[ 512 ];
	TCHAR DESC_2[ 512 ];
	TCHAR DESC_3[ 2048 ];
}UDESC, *LPUDESC;

typedef struct{
	LONG_PTR thisptr;
	TCHAR* Searchstring;
}SCTHREADDATA, *LPSCTHREADDATA;

typedef struct SRCHSettings{

	LRESULT EnableMultipleFileOpening;
	LRESULT ExpandPathToFile;
	LRESULT CloseAfterOpening;
	LRESULT SF_Projectname;
	LRESULT SF_Filename;
	LRESULT SF_Description1;
	LRESULT SF_Description2;
	LRESULT SF_Description3;

	SRCHSettings& SRCHSettings::operator= (const SRCHSettings& s)
	{
		this->CloseAfterOpening = s.CloseAfterOpening;
		this->EnableMultipleFileOpening = s.EnableMultipleFileOpening;
		this->ExpandPathToFile = s.ExpandPathToFile;
		this->SF_Description1 = s.SF_Description1;
		this->SF_Description2 = s.SF_Description2;
		this->SF_Description3 = s.SF_Description3;
		this->SF_Filename = s.SF_Filename;
		this->SF_Projectname = s.SF_Projectname;

		return *this;
	}

}SRCHSET, *LPSRCHSET;

typedef struct SearchControlInformation{

	HINSTANCE hInstance;
	HWND MainWindow;
	HWND ParentWindow;
	HWND SearchWnd;
	BOOL TerminationRequested;
	BOOL NewSearchRequested;
	BOOL ThreadActive;
	BOOL Progressbar_visible;
	TCHAR *WorkingDir;
	TCHAR *RootSearchDir;
	HBRUSH background;
	HFONT Listfont;
	DWORD Contsize;
	int language;
	BOOL DESC_Userdefined;
	LPUDESC descriptions;
	SRCHSET settings;
	int SRCItems;
	COLORREF crBackground;
	COLORREF crTextcolor;
	COLORREF crStylecolor;

}SEARCHCTRL_INFO, *LPSEARCHCTRL_INFO;

typedef struct SearchResultStruct{

	DWORD dwMatchFlags;
	int InitialIndex;
	int ImageIndex;
	TCHAR FileName[ 256 ];
	TCHAR FilePath[ 4096 ];
	TCHAR ProjectName[ 256 ];
	TCHAR ProjectPath[ 4096 ];
	TCHAR Description_ONE[ 512 ];
	TCHAR Description_TWO[ 512 ];
	TCHAR Description_THREE[ 2048 ];

}SEARCHRESULT, *LPSEARCHRESULT;

typedef struct OpenResults{

	int resultCount;
	int expandPath;
	DWORD OpenType[ 4 ];
	LPTSTR openPath[ 4 ];

}OPENRESULT, *LPOPENRESULT;

typedef struct ThreadStorage{

	Open_Save_CTRL* osc;
	HWND Progress;

}THREADSTORAGE, *LPTHREADSTORAGE;

class Searchcontrol
{
public:
	Searchcontrol(HINSTANCE hInst,HWND Main, HWND Parent,LPCTSTR WorkingDirectory,LPCTSTR SearchDirectory, int language);
	~Searchcontrol(void);

	void Release() { delete this; }
	HRESULT InitSearchWindow(void);
	HRESULT SetDescriptions(PCWSTR,PCWSTR,PCWSTR);

	// the following methods are only for operation in settings get/set mode
	BOOL getSettings(LPSRCHSET settings);
	BOOL setSettings(LPSRCHSET settings);

	static TCHAR* searchWindowClass;

private:
	LPSEARCHCTRL_INFO SC_info;
	volatile LPSEARCHRESULT container;
	LPTHREADSTORAGE lps;

	HRESULT InitSearchMainFrame(void);
	HRESULT InitSearchChilds(void);
	HRESULT CreateSetupWindow(void);
	HRESULT InitProgressBar();

	static LRESULT CALLBACK SearchProc(HWND,UINT,WPARAM,LPARAM);
	static LRESULT CALLBACK EditSubProc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
	static INT_PTR CALLBACK SetupProc(HWND,UINT,WPARAM,LPARAM);
	static DWORD WINAPI SearchProcess(LPVOID);

	BOOL EnableSearchProcess(LPTSTR);
	BOOL InitListviewColums(HWND);
	BOOL InsertListviewItems(HWND,int);
	BOOL InitListViewImageLists(HWND);

	BOOL ResizeWindow( void );

	BOOL TryInsert( NMLVDISPINFO* );

	BOOL InsertSingleItem(HWND,int);

	void OnDestroy( HWND );
	void OnClose( HWND );
	void OnSize( HWND );
	void OnNotify( HWND, LPARAM );
	void OnTerminateProgressbar();
	void StartSearch( HWND );
	void SetMinMaxInfo( LPARAM );
	void FinalizeProgress();
	LRESULT OnNCActivate(HWND,WPARAM,LPARAM);

	HFONT GetClientFont(int,int,PCWSTR);
	BOOL ConvertMultilineDescription(LPTSTR);
	BOOL GetFolderName(LPTSTR,LPTSTR);
	//BOOL CheckErrorFlags(DWORD);
	BOOL SaveSettings(HWND);
	BOOL LoadSettings(void);
	void SetValues(HWND);
	void SetDefault(void);
	void HandleOpening(HWND);
	LONG ConvertPixToTwips(int);
	BOOL CheckCriteria(void);
	int CheckFileType(LPTSTR);
	BOOL SearchForString(LPTSTR,LPTSTR);
	int CreateSearchResultString(LPTSTR);
	BOOL ExeedContainer(void);
	void CleanUpThreadMemory(void);
	void PrintResultDC(int,LPTSTR,LPTSTR);
	void ShowSingleResult(HWND);
	BOOL SearchInRoot(LPTSTR);


	inline BOOL ResultDisplayAndStorage(DWORD,int&,LPTSTR,LPTSTR,LPTSTR,LPTSTR,LPTSTR,LPTSTR,LPTSTR,int);
	inline void Interrupt(void);
	inline BOOL ProcessNextLevel(int,int&,LPTSTR,LPTSTR,LPWIN32_FIND_DATA,HANDLE);
	inline BOOL Analyse(LPTSTR,int&,LPTSTR,LPTSTR,LPTSTR,DWORD);
	inline void ReconvertPath(TCHAR*);
};
#endif

/* ADD TO RC FILE :

IDI_SEARCH				ICON		"res\\SearchWindow\\search.ico"
IDI_FIND				ICON		"res\\SearchWindow\\find.ico"
IDI_OPENRESULT			ICON		"res\\SearchWindow\\open_result5.ico"
/////////////////////////////////////////////////////////////////////////////////////
IDM_SEARCHMENU_G		MENU
BEGIN
	POPUP	"&Optionen"
		BEGIN
			MENUITEM	"&Einstellungen"		IDM_SEARCHSETTINGS
			MENUITEM	SEPARATOR
			MENUITEM	"&Beenden"				IDM_SEARCHCLOSE
		END
END
/////////////////////////////////////////////////////////////////////////////////////
IDM_SEARCHMENU_E		MENU
BEGIN
	POPUP	"&Optionen"
		BEGIN
			MENUITEM	"&Settings"				IDM_SEARCHSETTINGS
			MENUITEM	SEPARATOR
			MENUITEM	"&Close"				IDM_SEARCHCLOSE
		END
END

/////////////////////////////////////////////////////////////////////////////////////
// DIALOGBOXES
IDD_SETUPDIALOG_G DIALOGEX 0, 0, 170, 190
STYLE DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Einstellungen"
FONT 8, "MS Shell Dlg", 0, 0, 0x1
BEGIN
	CHECKBOX		"Öffnen mehrerer Dateien erlauben",ID_SRCHCKBX1,20,10,200,10,BS_AUTOCHECKBOX
	CHECKBOX		"Pfad zur Datei expandieren",ID_SRCHCKBX2,20,25,200,10,BS_AUTOCHECKBOX
	CHECKBOX		"Fenster nach dem Öffnen schliessen",ID_SRCHCKBX3,20,40,200,10,BS_AUTOCHECKBOX
	GROUPBOX		"Suchen nach...",0,10,60,150,95,BS_GROUPBOX
	CHECKBOX		"Projektname",ID_SRCHCKBX4,30,75,120,10,BS_AUTOCHECKBOX
	CHECKBOX		"Dateiname",ID_SRCHCKBX5,30,90,120,10,BS_AUTOCHECKBOX
	CHECKBOX		"Zeichnungsnummer",ID_SRCHCKBX6,30,105,120,10,BS_AUTOCHECKBOX
	CHECKBOX		"Kunde",ID_SRCHCKBX7,30,120,120,10,BS_AUTOCHECKBOX
	CHECKBOX		"Bezeichnung",ID_SRCHCKBX8,30,135,120,10,BS_AUTOCHECKBOX
    DEFPUSHBUTTON   "OK",IDOK,75,170,30,14
    PUSHBUTTON      "Abbrechen",IDCANCEL,110,170,50,14
END

IDD_SETUPDIALOG_E DIALOGEX 0, 0, 170, 190
STYLE DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Settings"
FONT 8, "MS Shell Dlg", 0, 0, 0x1
BEGIN
	CHECKBOX		"Enable multiple file opening",ID_SRCHCKBX1,20,10,200,10,BS_AUTOCHECKBOX
	CHECKBOX		"Expand path to file",ID_SRCHCKBX2,20,25,200,10,BS_AUTOCHECKBOX
	CHECKBOX		"Close Window after opening",ID_SRCHCKBX3,20,40,200,10,BS_AUTOCHECKBOX
	GROUPBOX		"Search for...",0,10,60,150,95,BS_GROUPBOX
	CHECKBOX		"Projectname",ID_SRCHCKBX4,30,75,120,10,BS_AUTOCHECKBOX
	CHECKBOX		"Filename",ID_SRCHCKBX5,30,90,120,10,BS_AUTOCHECKBOX
	CHECKBOX		"Drawing number",ID_SRCHCKBX6,30,105,120,10,BS_AUTOCHECKBOX
	CHECKBOX		"Customer",ID_SRCHCKBX7,30,120,120,10,BS_AUTOCHECKBOX
	CHECKBOX		"Description",ID_SRCHCKBX8,30,135,120,10,BS_AUTOCHECKBOX
    DEFPUSHBUTTON   "OK",IDOK,75,170,30,14
    PUSHBUTTON      "Cancel",IDCANCEL,110,170,50,14
END
*/

/* ADD TO RESOURCE FILE :

#define		IDI_SEARCH				106
#define		IDI_FIND				107
#define		IDI_OPENRESULT			108

#define		IDM_SEARCHMENU_G		201
#define		IDM_SEARCHMENU_E		202

#define		IDM_SEARCHSETTINGS		228
#define		IDM_SEARCHCLOSE			230

//DIALOGRESOURCES
#define		IDD_SETUPDIALOG_G		700
#define		IDD_SETUPDIALOG_E		702
#define		ID_SRCHCKBX1			720
#define		ID_SRCHCKBX2			721
#define		ID_SRCHCKBX3			722
#define		ID_SRCHCKBX4			723
#define		ID_SRCHCKBX5			724
#define		ID_SRCHCKBX6			725
#define		ID_SRCHCKBX7			726
#define		ID_SRCHCKBX8			727
*/

/*
NEW METHOD IN OPENSAVECONTROL:
PUBLIC:
	BOOL Open_For_Searching( WCHAR* Path, LPTSTR desc1, LPTSTR desc2, LPTSTR desc3 );//new method

BOOL Open_Save_CTRL::Open_For_Searching( WCHAR* Path, LPTSTR desc1, LPTSTR desc2, LPTSTR desc3 )//new method
{
	BOOL bRet = TRUE;

	if( FAILED( this->Buffer_Alloc() ) )
	{
		this->SUCCESS = FALSE;
		StringCbCopy(	this->oc_error,
						sizeof( this->oc_error ),
						L"MEMORY ALLOCATION FOR BUFFERING FAILED >> [ OPEN_DIRECT ]" );
		return FALSE;
	}

	if( SUCCEEDED( this->_Extract_formated_Data( Path ) ) )
	{
		if( this->p_to_Text != NULL )
		{
			if( this->p_to_Text->Description_one != NULL )
			{
				if( FAILED( StringCbCopy( desc1, sizeof( TCHAR )* 512, this->p_to_Text->Description_one ) ))
					bRet = FALSE;
			}
			else
				bRet = FALSE;

			if( this->p_to_Text->Description_two != NULL )
			{
				if( FAILED( StringCbCopy( desc2, sizeof( TCHAR )* 512, this->p_to_Text->Description_two ) ))
					bRet = FALSE;
			}
			else
				bRet = FALSE;

			if( this->p_to_Text->Description_three != NULL )
			{
				if( FAILED( StringCbCopy( desc3, sizeof( TCHAR )* 2048, this->p_to_Text->Description_three ) ))
					bRet = FALSE;
			}
			else
				bRet = FALSE;
		}
	}
	else
	{
		bRet = FALSE;
	}
	this->Buffer_Release();

	return bRet;
}

*/