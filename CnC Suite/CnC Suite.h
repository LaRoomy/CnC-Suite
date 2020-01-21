#pragma once
#ifndef _CNCSUITE_HEADER_
#define _CNCSUITE_HEADER_

#pragma comment( lib, "Kernel32.lib" )
#pragma comment( lib, "User32.lib" )
#pragma comment(lib,"comctl32.lib")
#pragma comment( lib, "Shlwapi.lib")

#include<CommCtrl.h>

// Enable Visual Styles
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include"SafeRelease.h"

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) if(!(b)){ OutputDebugStringA("Assert: " #b "\n");}
#else
#define Assert(b)
#endif //( DEBUG ) || ( _DEBUG )
#endif

#include"Global.h"
#include"GlobalStyles.h"

/// buildinfo section ///////////////////////////////////////////////////////////////////////////////////////////////
// this section contains displayable information regarding the buildinfo of the application (displayed on info page)
#define		CNCSUITE_BUILDVERSION		L"CnC Suite Build 1.3.8"
#define		CNCSUITE_BUILDDATE			L"Build-date: October 25th 2018"
/// buildinfo section END ///////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// app identity section ///////////////////////////////////////////////////////////////////////////////////////////

#define		APPTYPE_STOREAPP		1
#define		APPTYPE_FREEAPP			2

#define			CNCSUITE_USERINSTALLATION		// disable this to create the executable for the classic installation

#define		IDSEX_APPLICATIONCLASS			L"CNCSUITECLASS"
#define		APPLICATION_DISPLAY_NAME		L"CnC Suite"
#define		APPLICATION_FULLVERSION_NAME	L"CnC Suite 1. 3. 8"
#define		APPLICATION_VERSION				L"Version 1.3.84"

#define		EXECUTABLE_NAME					L"CnC Suite.exe"

#define		MUTEX_NAME						L"CnC Suite 1.0"

#define		CNCSUITE_GUID					L"{068a2920-c4ff-315d-b2a7-04003c39ff3c}"

#define		APPLICATION_PRIMARY_FONT		L"Calibri"
#define		APPLICATION_SECONDARY_FONT		L"Segoe UI"

// to find name references regarding the application name, use the mark "FND_APPNAME" !

/// app id section END ///////////////////////////////////////////////////////////////////////////////////////////////

static const int AppVersion = 3001;
static const int AppType = APPTYPE_STOREAPP;

typedef int LanguageID;
typedef TCHAR** LPPTSTR;
typedef WCHAR** LPPWSTR;

typedef __int64 Int64;

#if defined(_WIN64)
typedef DWORD64 uintX;
typedef __int64 intX;
#else
typedef DWORD32 uintX;
typedef int intX;
#endif

// TODO: remove that and clean all existing references!
#define		GERMAN		0x07
#define		ENGLISH		0x09
///////////////////////////////////////////////

#define		_MAIN_WND_						(HWND)(FindWindow(IDSEX_APPLICATIONCLASS,nullptr))

#define		_FWD_MSG_TO_MAIN(wParam,lParam)		(BOOL)SNDMSG((	\
														FindWindow(IDSEX_APPLICATIONCLASS,NULL)),WM_COMMAND,(WPARAM)(wParam),(LPARAM)(lParam))

#define		_MSG_TO_MAIN(msg,wParam,lParam)		(BOOL)SNDMSG((	\
														FindWindow(IDSEX_APPLICATIONCLASS,NULL)),(UINT)msg,(WPARAM)(wParam),(LPARAM)(lParam))

#define		_NOT_USED(p)		if(p == 0)(p); \
								else (p);

#define		_BELOW_ZERO_SETTOZERO(p)	if(p < 0)p = 0;

#define		TOGGLE_BOOL(b)		if(*b == 0)*b = 1; \
								else *b = 0;

#define		TOGGLE_bool(b)		if(*b == false)*b = true; \
								else *b = false;

#define		USERDATA_FROMWINDOW(c)		(LONG_PTR)(GetWindowLongPtr(	\
											FindWindow(((LPCTSTR)(c)), nullptr), GWLP_USERDATA))

#define			I_ERROR_PAPP_INVALID		-2
#define			I_ERROR_INIT_FAILED			-3
#define			I_ERROR_DATA_FAILED			-4
#define			I_ERROR_DPI_FAILED			-5

#define			MAX_LOADSTRING				100

#define			COMP_ID_FILE_EXPLORER		22883
#define			COMP_ID_TAB_CONTROL			22884
#define			COMP_ID_ERROR_BOX			22885
#define			COMP_ID_PROPERTIES			22886
#define			COMP_ID_HISTORY				22887

#define			NO_RESTART				0
#define			RESTART_WITH_CMDLINE	1
#define			PLAIN_RESTART			2
#define			RESTART_DONOTSAVE		3

TCHAR* getStringFromResource(int ID);
int getCurrentAppLanguage();
HRESULT initDefaultApplicationDataContainer();
LONG_PTR getDefaultApplicationDataContainer();
HRESULT initExtendedApplicationDataContainer();
LONG_PTR getApplicationDataContainerFromFilekey(LPVOID key);
void changeAppStyle(int styleID);
LONG_PTR getApplicationPropertyComponent();
LONG_PTR getComponentHandle(DWORD component_ID);
LONG_PTR getDPIAssist();
int DPIScale(int val);
HFONT CreateScaledFont(int fontHeight, int fontWeight, LPCTSTR fontName);
HFONT CreateScaledUnderlinedFont(int fontHeight, int fontWeight, LPCTSTR fontName);
bool isCnC3Path(LPCTSTR path);
LONG WINAPI lpTopLevelExceptionFilter(_EXCEPTION_POINTERS *exceptionInfo);
void ScheduleRestart(DWORD restartOption);
BOOL GetApplicationStyleInformation(LPAPPSTYLEINFO pSInfo);
LPCRITICAL_SECTION GetCriticalSection();


/* TODO GLOBAL !!!

- Information in Autocompletemanager: keine Leerzeichen am Ende
- Satznummer-Steuerung überarbeiten
- Autosyntax implementieren
- Editcontrol-property-setup in tabclass speichern
- Samplemanager -> manche samples wurden nicht gelöscht, andere werden gelistet, sind aber gar nicht vorhanden???
- Autosave integrieren
- Sessiondaten speichern
- Samplemanager anders aufrufen
- Tabcontrol: Standardfarben für editbox anpassen
- Basisklasse ObjectRelease bereiningen (virtueller Destruktor?)

*/


/*	Remarks for the lineend-format

	-	standard in unicode (codepage 1200) is only '\r'(carriage return) -> this format must be applied when inserting the buffer in 'OpenSaveCtrl'
	-	


	NOTE: when searching for codepage-issues, use the mark '//CODEPAGE' for searching
*/

/*

old .gitignore (BUILDRESULTS)

# Build results
[Dd]ebug/
[Dd]ebugPublic/
[Rr]elease/
[Rr]eleases/
x64/
x86/
bld/
[Bb]in/
[Oo]bj/
[Ll]og/

*/

#endif