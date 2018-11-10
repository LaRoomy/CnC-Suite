#pragma once
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Propsys.lib")

#include"..\\external.h"
#include"..\\HelperF.h"
#include"..\\BasicFPO.h"

#ifndef _OSD_INCLUDED_
#define _OSD_INCLUDED_
#include <shlobj.h>
#include <objbase.h>      // For COM headers
#include <shobjidl.h>     // for IFileDialogEvents and IFileDialogControlEvents
#include <shlwapi.h>
#include <knownfolders.h> // for KnownFolder APIs/datatypes/function headers
#include <propvarutil.h>  // for PROPVAR-related functions
#include <propkey.h>      // for the Property key APIs/datatypes
#include <propidl.h>      // for the Property System APIs
#include <propsys.h>
#include <strsafe.h>      // for StringCchPrintfW
#include <shtypes.h>      // for COMDLG_FILTERSPEC
#include <Unknwn.h>
#include <new>

#define STRICT_TYPED_ITEMIDS

typedef struct{		TCHAR* MainText;
					TCHAR* Description_one;
					TCHAR* Description_two;
					TCHAR* Description_three;	}Pointer_to_Window_Text, *p_window_text;

typedef struct{		BOOL FILE_PATH_AVAILABLE;
					BOOL FOLDER_PATH_AVAILABLE;
					WCHAR file_path[ 1024 ];
					WCHAR folder_path[ 1024 ];	}Path_Ctrl, *p__path_Ctrl;

typedef struct Return_Path{		WCHAR file_path[ 1024 ];
								WCHAR file_name[ 128 ];
								WCHAR folder_path[ 1024 ];
								WCHAR folder_name[ 128 ];	}RETURNPATH, *PRETURNPATH;

typedef int Mode;

const COMDLG_FILTERSPEC Data_Types[] =
{
	{L"NC Program (*.cnc3)",		L"*.cnc3"},
	{L"Text Document (*.txt)",		L"*.txt"},
	{L"All Documents (*.*)",		L"*.*"}
};

const COMDLG_FILTERSPEC Data_Type[] =
{
	{ L"NC Program (*.cnc3)",		L"*.cnc3" }
};

// Indices of file types
#define INDEX_CNC_DOC	1
#define INDEX_TEXTDOC	2
#define INDEX_ALLFILES	3

// O/S Modes
#define MODE_OPEN		36
#define MODE_SAVE		41

// ERROR ID's
#define	DIALOG_CANCELLED	4// the user closed the dialog without selecting a path

#define FILE_SYNTAX_ERROR	-4
#define NO_PROPERTY			-5
#define BUFFER_END			-6

#define	DLG_ABORTED			50
#define	INVALID_FILE_FORMAT	51

#define EXISTING_FILE_OVERWRITTEN	55

#define	FILETYPEMODE_ONLY_CNC3		5
#define	FILETYPEMODE_NORMAL			6

class CDialogEventHandler : public IFileDialogEvents,
                            public IFileDialogControlEvents
{
public:
	// IUnknown methods
#pragma warning(push)
#pragma warning(disable : 4838)

	IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
	{
		static const QITAB qit[] = {
			QITABENT(CDialogEventHandler, IFileDialogEvents),
			QITABENT(CDialogEventHandler, IFileDialogControlEvents),
			{0}
		};
		return QISearch(this, qit, riid, ppv);
	}

#pragma warning(pop)

	IFACEMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&_cRef);
	}

	IFACEMETHODIMP_(ULONG) Release()
	{
		long cRef = InterlockedDecrement(&_cRef);
		if (!cRef)
			delete this;
		return cRef;
	}

	// IFileDialogEvents methods
	IFACEMETHODIMP OnFileOk(IFileDialog *) {

		//MessageBox(NULL, L"OnFileOk", L"IFileDialog", MB_OK);

		// IN OnTypeChange: (not here!!!)

		// 1. inform the user that only cnc3 files are supported
		// 2. ask the user to open the export dialog
		// 3. -> if IDYES >> return S_OK to close the IFileDialog and open the export-dialog
		//    -> if IDNO >> return E_FAIL to keep the IFileDialog open


		return S_OK;
	};
	IFACEMETHODIMP OnFolderChange(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnFolderChanging(IFileDialog *, IShellItem *) { return S_OK; };
	IFACEMETHODIMP OnHelp(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnSelectionChange(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnShareViolation(IFileDialog *, IShellItem *, FDE_SHAREVIOLATION_RESPONSE *) { return S_OK; };
	IFACEMETHODIMP OnTypeChange(IFileDialog *pfd);
	IFACEMETHODIMP OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *) { return S_OK; };

	// IFileDialogControlEvents methods
	IFACEMETHODIMP OnItemSelected(IFileDialogCustomize *pfdc, DWORD dwIDCtl, DWORD dwIDItem) {
		UNREFERENCED_PARAMETER(pfdc);
		UNREFERENCED_PARAMETER(dwIDCtl);
		UNREFERENCED_PARAMETER(dwIDItem);
		return S_OK;
	}
	IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *, DWORD){ return S_OK; };
	IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize *, DWORD, BOOL) { return S_OK; };
	IFACEMETHODIMP OnControlActivating(IFileDialogCustomize *, DWORD) { return S_OK; };

	CDialogEventHandler() : _cRef(1) { };
private:
	~CDialogEventHandler() { };
	long _cRef;
};

class Open_Save_CTRL
{
public:			Open_Save_CTRL(void);
			~Open_Save_CTRL(void);

			BOOL SUCCESS;
			int error_indicator;

			void Release() { delete this; }

	HRESULT Open_Save_Control(	Mode,
								int File_Index,
								HWND Main_edit,
								HWND Descriptedit_one,
								HWND Descriptedit_two,
								HWND Descriptedit_three	);
	BOOL Open_direct(WCHAR* Path,HWND Main,HWND Descript_one,HWND Descript_two,HWND Descript_three);
	BOOL Save_direct(WCHAR* Path,HWND Main,HWND Descript_one,HWND Descript_two,HWND Descript_three);
	BOOL Open_For_Searching( WCHAR* Path, LPTSTR desc1, LPTSTR desc2, LPTSTR desc3 );
	BOOL OpenWithoutLoading( LPCTSTR Path, TCHAR** MainText, TCHAR** desc1, TCHAR** desc2, TCHAR** desc3 );
	BOOL SaveBuffersDirect(WCHAR* Path,LPTSTR Main,LPTSTR Descript_one,LPTSTR Descript_two,LPTSTR Descript_three);
	BOOL GetFilePathFromUser(TCHAR **Path, int Mode, int FileIndex);
	HRESULT OpenFolder(HWND owner, WCHAR** folderPath);
	BOOL ConvertFileToCnc3(HWND MB_Owner, LPTSTR path);// if succeeded, path contains the new filename// rAction determines the handling with the older file (IDYES -> keep the file) (IDNO -> delete the file)
	BOOL OS_DialogCustomText(LPCTSTR Caption, LPCTSTR ButtonText, LPCTSTR FileText, LPCTSTR Folderpath );
	BOOL Get_requested_ID(PRETURNPATH r_path);// << CREATE A >RETURNPATH< OBJECT AND YOU CAN GET PATH + ITEM STRINGS FROM THIS METHOD
	void Get_O_S_Error(void);// << CALL THIS MEMBER TO GET EXTENDED ERROR INFORMATION !!
	HRESULT GetTranslated_hResultErrorCode(TCHAR** error_out);
	void SetFileTypeMode(DWORD filetype_mode);
	void setHwndOwner(HWND owner) { this->hWndOwner = owner; }
	BOOL getAdditionalOperationResult() { return this->additionalOperationResult; }// indicates some additional info after an operation was executed (e.g. when a file was overwritten)
private:
	BOOL CustomizationRequested;
	TCHAR oc_error[ 512 ];
	p_window_text p_to_Text;
	p__path_Ctrl F_F_Path;
	TCHAR *CustomText[ 4 ];
	HRESULT lastError;
	BOOL additionalOperationResult;
	DWORD fileTypeMode;
	HWND hWndOwner;

	HRESULT _WriteDataToFile(HANDLE,PCWSTR);
	HRESULT _ReadDataFromFile(HANDLE,WCHAR*&);
	HRESULT _Write_formated_Data_toFile(PCWSTR,PCWSTR,PCWSTR,PCWSTR,PCWSTR);
	HRESULT _Extract_formated_Data(PCWSTR);
	HRESULT CDialogEventHandler_CreateInstance(REFIID riid, void **ppv);
	HRESULT BasicFileOpenSave(const IID,int,TCHAR*, bool);
	HRESULT Retrieve_WindowText(HWND,HWND,HWND,HWND);
	HRESULT SetOpened_WindowText(HWND,HWND,HWND,HWND);
	HRESULT Buffer_Alloc(void);
	BOOL Extract(WCHAR*);
	void Buffer_Release(void);
	int SearchFor_Property_begin(int,WCHAR*,int,WCHAR*);
	int Get_Property(int,int,WCHAR*);
	BOOL Get_path_end(WCHAR*,WCHAR*);
	int CheckFileType(LPTSTR);
	HRESULT SetCustomText(IFileDialog*);
};

#endif
// TODO edit >> OnItemSelected >> to show property of CNC File