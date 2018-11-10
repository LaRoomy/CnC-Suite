#include"SearchControl.h"
#include<StringClass.h>
#include"..//DPI_Helper.h"
#include"..//HelperF.h"
#include"..//BasicFPO.h"
#include"..//CnC3FileManager.h"

Searchcontrol::Searchcontrol(	HINSTANCE hInst,
								HWND Main,
								HWND Parent,
								LPCTSTR WorkingDirectory,
								LPCTSTR SearchDirectory,
								int language)
	:	SC_info( NULL ),
		container( NULL )
{
	this->SC_info = new SEARCHCTRL_INFO;

	if( this->SC_info == NULL )
	{
		// TODO: replace
		MessageBox( NULL, L"Memory allocation failed ! ( SEARCHCTRL_INFO )", L"Init Searchclass", MB_OK | MB_ICONERROR );
	}
	else
	{
		APPSTYLEINFO sInfo;
		BOOL sSuccess = (BOOL)SendMessage(Main, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&sInfo));

		SecureZeroMemory(this->SC_info, sizeof(SEARCHCTRL_INFO));

		this->SC_info->MainWindow = Main;
		this->SC_info->hInstance = hInst;
		this->SC_info->language = language;
		
		this->SC_info->Listfont = CreateScaledFont(18, FW_NORMAL, APPLICATION_PRIMARY_FONT);//this->GetClientFont(18, FW_MEDIUM, L"Trebuchet MS\0");
		this->SC_info->DESC_Userdefined = FALSE;
		this->SC_info->descriptions = NULL;
		this->SC_info->Contsize = 0;
		this->SC_info->TerminationRequested = FALSE;
		this->SC_info->ThreadActive = FALSE;
		this->SC_info->NewSearchRequested = FALSE;
		this->SC_info->ParentWindow = Parent;

		if (sSuccess)
		{
			this->SC_info->crBackground = sInfo.mainToolbarColor;//sInfo.TabColor;
			this->SC_info->crTextcolor = sInfo.TextColor;
			this->SC_info->background = CreateSolidBrush(this->SC_info->crBackground);
			this->SC_info->crStylecolor = sInfo.TabColor; //sInfo.OutlineColor;
		}
		else
		{
			this->SC_info->crBackground = RGB(255, 255, 255);
			this->SC_info->crTextcolor = RGB(0, 0, 0);
			this->SC_info->background = CreateSolidBrush(RGB(255, 255, 255));
			this->SC_info->crStylecolor = RGB(220, 220, 220);
		}
	}

	size_t len;
	auto hr = StringCbLength( WorkingDirectory, sizeof( TCHAR ) * 4096, &len );
	if (SUCCEEDED(hr))
	{
		if (len > 0)
		{
			len += sizeof(TCHAR);

			this->SC_info->WorkingDir = new TCHAR[len];

			if (this->SC_info->WorkingDir != NULL)
			{
				StringCbCopy(this->SC_info->WorkingDir, len, WorkingDirectory);
			}
		}
	}
	len = 0;

	hr = StringCbLength( SearchDirectory, sizeof( TCHAR ) * 4096, &len );
	if (SUCCEEDED(hr))
	{
		if (len > 0)
		{
			len += sizeof(TCHAR);

			this->SC_info->RootSearchDir = new TCHAR[len];

			if (this->SC_info->RootSearchDir != NULL)
			{
				StringCbCopy(this->SC_info->RootSearchDir, len, SearchDirectory);
			}
		}
	}
	if( !this->LoadSettings( ) )
	{
		this->SetDefault( );
	}
	this->lps = NULL;
}

Searchcontrol::~Searchcontrol( void )
{
	UnregisterClass( L"SEARCHCLASS", this->SC_info->hInstance );

	if( this->SC_info != nullptr )
	{
		DeleteObject( this->SC_info->Listfont );
		DeleteObject( this->SC_info->background );

		if( this->SC_info->RootSearchDir != nullptr )
		{
			delete [] this->SC_info->RootSearchDir;
			this->SC_info->RootSearchDir = nullptr;
		}
		if( this->SC_info->WorkingDir != nullptr )
		{
			delete [] this->SC_info->WorkingDir;
			this->SC_info->WorkingDir = nullptr;
		}
		if (this->SC_info->descriptions != nullptr)
		{
			delete this->SC_info->descriptions;
			this->SC_info->descriptions = nullptr;
		}
		delete this->SC_info;
		this->SC_info = nullptr;
	}
	if( this->container != nullptr )
	{
		delete this->container;
		this->container = nullptr;
	}
	if (this->lps != NULL)
	{
		delete this->lps;
		this->lps = nullptr;
	}
}

HRESULT Searchcontrol::InitSearchWindow( void )
{
	if( SUCCEEDED( this->InitSearchMainFrame( ) ))
	{
		if( FAILED( this->InitSearchChilds( ) ))
			return E_FAIL;
		else
		{
			if( !this->ResizeWindow( ) )
				return E_FAIL;
		}
	}
	else
		return E_FAIL;

	return S_OK;
}

TCHAR* Searchcontrol::searchWindowClass = L"SEARCHCLASS";

HRESULT Searchcontrol::InitSearchMainFrame( void )
{
	auto DPIAssist = 
		reinterpret_cast<DPI_Assist*>(
			getDPIAssist()
		);

	int menu_ID = 0;
	PCWSTR Wname;

	if( this->SC_info->language == GERMAN )
	{
		menu_ID = IDM_SEARCHMENU_G;

		Wname = L"Suchen";
	}
	else
	{
		menu_ID = IDM_SEARCHMENU_E;

		Wname = L"Search";
	}

	LPCTSTR menu_ = nullptr;
	if (DPIAssist != nullptr)
	{
		auto dpiIndex = DPIAssist->GetScaleFactor();
		if (dpiIndex == DPI_Assist::SCALE_100P)
		{
			menu_ = MAKEINTRESOURCE(menu_ID);
		}
	}

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof( wcex );
	wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = Searchcontrol::SearchProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof( LONG_PTR );
	wcex.hInstance = this->SC_info->hInstance;
	wcex.hIcon = reinterpret_cast<HICON>( LoadImage( wcex.hInstance, MAKEINTRESOURCE( IDI_SEARCH ), IMAGE_ICON, 48,48, LR_DEFAULTCOLOR ) );
	wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = this->SC_info->background;
	wcex.lpszClassName = Searchcontrol::searchWindowClass;
	wcex.lpszMenuName = menu_;
	wcex.hIconSm = reinterpret_cast<HICON>( LoadImage( wcex.hInstance, MAKEINTRESOURCE( IDI_SEARCH ), IMAGE_ICON, 48,48, LR_DEFAULTCOLOR ) );
	
	if( RegisterClassEx( &wcex ) == 0 )
		return E_FAIL;

	int cx = GetSystemMetrics( SM_CXSCREEN );
	int cy = GetSystemMetrics( SM_CYSCREEN );

	this->SC_info->SearchWnd =

		CreateWindowEx(
			0,
			Searchcontrol::searchWindowClass,
			Wname,
			WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_CLIPCHILDREN,
			(int)( cx / 5 ),
			(int)( cy / 3 ),
			(int)(( cx / 5 ) *3 ),
			(int)( cy / 3 ),
			this->SC_info->ParentWindow,
			NULL,
			this->SC_info->hInstance,
			reinterpret_cast<LPVOID>(this)
		);

	if( !this->SC_info->SearchWnd )
		return E_FAIL;

	ShowWindow( this->SC_info->SearchWnd, SW_SHOW );
	UpdateWindow( this->SC_info->SearchWnd );

	return S_OK;
}

HRESULT Searchcontrol::InitSearchChilds( void )
{
	RECT rc;
	GetClientRect( this->SC_info->SearchWnd, &rc );

	HWND edit = CreateWindowEx(
		0,
		MSFTEDIT_CLASS,
		NULL,
		WS_CHILD | WS_VISIBLE | ES_SUNKEN,
		0,
		0,
		rc.right - DPIScale(40),
		DPIScale(30),
		this->SC_info->SearchWnd,
		reinterpret_cast<HMENU>( ID_SEARCHEDIT ),
		this->SC_info->hInstance,
		NULL
	);

	if( !edit )
		return E_FAIL;
	else
	{
		CHARFORMAT cf;

		initCharformat(&cf, DPIScale(18), this->SC_info->crTextcolor, L"Segoe UI\0");

		SendMessage( edit, EM_SETCHARFORMAT, static_cast<WPARAM>( SCF_ALL ), reinterpret_cast<LPARAM>( &cf ) );
		SendMessage(edit, EM_SETBKGNDCOLOR, 0, static_cast<LPARAM>(this->SC_info->crBackground));
		SendMessage( edit, EM_SETEVENTMASK, static_cast<WPARAM>( 0 ), static_cast<LPARAM>( ENM_CHANGE ) );

		SetWindowSubclass( edit, Searchcontrol::EditSubProc, ID_EDITSUBCLASS, NULL );
		SetFocus( edit );
	}

	HWND Searchbutton = CreateWindow(
		L"BUTTON",
		NULL,
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_ICON,
		rc.right - DPIScale(40),
		0,
		DPIScale(40),
		DPIScale(30),
		this->SC_info->SearchWnd,
		reinterpret_cast<HMENU>( ID_STARTSEARCHING ),
		this->SC_info->hInstance,
		NULL
	);

	if( !Searchbutton )
		return E_FAIL;
	else
	{
		HICON icon =
			reinterpret_cast<HICON>(
				LoadImage(
					this->SC_info->hInstance,
					MAKEINTRESOURCE( IDI_FIND ),
					IMAGE_ICON,
					24,
					24,
					LR_DEFAULTCOLOR
				)
			);

		if (icon)
		{
			SendMessage(
				Searchbutton,
				BM_SETIMAGE,
				static_cast<WPARAM>(IMAGE_ICON),
				reinterpret_cast<LPARAM>(icon)
			);
			DeleteObject(icon);
		}
	}
	INITCOMMONCONTROLSEX icex;
	icex.dwICC = ICC_LISTVIEW_CLASSES;

	InitCommonControlsEx( &icex );

	HWND Listview =
		CreateWindowEx(
			WS_EX_CLIENTEDGE,
			WC_LISTVIEW,
			L"",
			WS_CHILD | LVS_REPORT | LVS_ICON,
			0,
			DPIScale(30),
			rc.right,
			rc.bottom - DPIScale(58),
			this->SC_info->SearchWnd,
			reinterpret_cast<HMENU>( ID_SEARCHLIST ),
			this->SC_info->hInstance,
			nullptr
		);

	if( !Listview )
		return E_FAIL;
	else
	{
		ListView_SetExtendedListViewStyleEx(
			Listview,
			LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER,
			LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER
		);

		SendMessage(
			Listview,
			WM_SETFONT,
			reinterpret_cast<WPARAM>( this->SC_info->Listfont ),
			static_cast<LPARAM>( TRUE )
		);

		ListView_SetBkColor(Listview, this->SC_info->crStylecolor);
		ListView_SetTextBkColor(Listview, this->SC_info->crStylecolor);
		ListView_SetTextColor(Listview, this->SC_info->crTextcolor);

		this->InitListviewColums( Listview );
		this->InitListViewImageLists( Listview );

		ShowWindow( Listview, SW_SHOW );
	}
	TCHAR germantext[] = L" Öffnen";
	TCHAR englishtext[] = L" Open";

	LPTSTR text;

	if( this->SC_info->language == GERMAN )
	{
		text = germantext;
	}
	else
	{
		text = englishtext;
	}
	HWND OpenButton = CreateWindow(
		L"BUTTON",
		text,
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		rc.right - DPIScale(120),
		rc.bottom - DPIScale(28),
		DPIScale(120),
		DPIScale(28),
		this->SC_info->SearchWnd,
		reinterpret_cast<HMENU>(ID_SEARCHOPEN),
		this->SC_info->hInstance,
		NULL
	);

	if( !OpenButton )
		return E_FAIL;
	else
	{
		SendMessage(
			OpenButton,
			WM_SETFONT,
			reinterpret_cast<WPARAM>( this->SC_info->Listfont ),
			static_cast<LPARAM>( TRUE )
		);

		HICON icon =
			(HICON)LoadImage(
				this->SC_info->hInstance,
				MAKEINTRESOURCE( IDI_OPENRESULT ),
				IMAGE_ICON,
				16,
				16,
				LR_DEFAULTCOLOR
			);

		if (icon)
		{
			SendMessage(
				OpenButton,
				BM_SETIMAGE,
				static_cast<WPARAM>(IMAGE_ICON),
				reinterpret_cast<LPARAM>(icon)
			);

			DestroyIcon(icon);
		}
	}
	return S_OK;
}

LRESULT CALLBACK Searchcontrol::SearchProc( HWND SearchWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	Searchcontrol* pSearch = NULL;

	if( message == WM_CREATE )
	{
		LPCREATESTRUCT createstruct;
		createstruct = reinterpret_cast<LPCREATESTRUCT>( lParam );
		pSearch = reinterpret_cast<Searchcontrol*>( createstruct->lpCreateParams );
		SetWindowLongPtr( SearchWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( pSearch ) );
		return 1;
	}
	else
	{
		pSearch = reinterpret_cast<Searchcontrol*>( GetWindowLongPtr( SearchWnd, GWLP_USERDATA ) );

		if( pSearch != NULL )
		{
			switch( message )
			{
				case WM_SIZE:
					pSearch->OnSize( SearchWnd );
					return 0;
				case WM_COMMAND:
					switch( LOWORD( wParam ) )
					{
						case IDM_SEARCHSETTINGS:
							pSearch->CreateSetupWindow( );
							break;
						case ID_STARTSEARCHING:
							pSearch->StartSearch( SearchWnd );
							break;
						case IDM_SEARCHCLOSE:
							SendMessage( SearchWnd, WM_CLOSE, 0, 0 );
							break;
						case ID_SEARCHOPEN:
							pSearch->HandleOpening( SearchWnd );
							break;
						case CU_DELETEPROGRESSBAR:
							pSearch->OnTerminateProgressbar();
						default:
							break;
					}
					return 0;
				case WM_CTLCOLORBTN:
					return reinterpret_cast<LONG_PTR>(pSearch->SC_info->background);
				case WM_NOTIFY:
					pSearch->OnNotify( SearchWnd, lParam );
					return 0;
				case WM_GETMINMAXINFO:
					pSearch->SetMinMaxInfo( lParam );
					return 0;
				case WM_CLOSE:
					pSearch->OnClose( SearchWnd );
					return 0;
				//case WM_NCACTIVATE:
				//	return pSearch->OnNCActivate( SearchWnd, wParam, lParam );
				case WM_DESTROY:
					pSearch->OnDestroy( SearchWnd );
					return 0;
				default:
					return DefWindowProc( SearchWnd, message, wParam, lParam );
			}
		}
		return DefWindowProc( SearchWnd, message, wParam, lParam );
	}
}

LRESULT CALLBACK Searchcontrol::EditSubProc( HWND edit, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR IDsubclass, DWORD_PTR RefData)
{
	UNREFERENCED_PARAMETER(IDsubclass);
	UNREFERENCED_PARAMETER(RefData);

	if( msg == WM_CHAR )
	{
		if( wParam == 0x0D )
		{
			SendMessage( GetParent( edit ), WM_COMMAND, static_cast<WPARAM>( LOWORD( ID_STARTSEARCHING) ), 0);
			return 0;
		}
	}
	return DefSubclassProc( edit, msg, wParam, lParam );
}

void Searchcontrol::OnDestroy( HWND SearchWnd )
{
	UNREFERENCED_PARAMETER(SearchWnd);

	PostMessage( this->SC_info->ParentWindow, WM_CLEANUP, static_cast<WPARAM>( CU_DELETESEARCHCLASS ), static_cast<LPARAM>( 0 ) );
	//this->Release();
}

BOOL Searchcontrol::ResizeWindow( void )
{
	OnSize( this->SC_info->SearchWnd );

	return TRUE;
}

HRESULT Searchcontrol::CreateSetupWindow( void )
{
	if( this->SC_info->language == GERMAN )
	{
		DialogBoxParam( this->SC_info->hInstance,
						MAKEINTRESOURCE( IDD_SETUPDIALOG_G ),
						this->SC_info->SearchWnd, Searchcontrol::SetupProc,
						reinterpret_cast<LPARAM>( this )					);
	}
	else
	{
		DialogBoxParam( this->SC_info->hInstance,
						MAKEINTRESOURCE( IDD_SETUPDIALOG_E ),
						this->SC_info->SearchWnd, Searchcontrol::SetupProc,
						reinterpret_cast<LPARAM>( this )					);
	}
	return S_OK;
}

HRESULT Searchcontrol::InitProgressBar()
{
	HRESULT hr;

	BasicFPO* pfo = CreateBasicFPO();
	hr = (pfo != NULL) ? S_OK : E_FAIL;
	if(SUCCEEDED(hr))
	{
		DWORD files = 0;
		DWORD folders = 0;

		int items = pfo->CountDirectoryContent(this->SC_info->RootSearchDir, &files, &folders);
		hr = (items) ? S_OK : E_FAIL;
		if(SUCCEEDED(hr))
		{
			RECT rc_button, rc;
			GetClientRect(this->SC_info->SearchWnd, &rc);
			GetWindowRect(
				GetDlgItem(this->SC_info->SearchWnd, ID_SEARCHOPEN), &rc_button);

			InitCommonControls();

			HWND Progress = CreateWindowEx(
				0,
				PROGRESS_CLASS,
				NULL,
				WS_CHILD | WS_VISIBLE,
				DPIScale(2),
				rc.bottom - DPIScale(26),
				rc.right - ((rc_button.right - rc_button.left) + DPIScale(4)),
				DPIScale(24),
				this->SC_info->SearchWnd,
				(HMENU)ID_SEARCHPROGRESS,
				this->SC_info->hInstance,
				NULL);

			hr = (Progress != NULL) ? S_OK : E_FAIL;
			if(SUCCEEDED(hr))
			{
				SendMessage(Progress, PBM_SETRANGE, 0, MAKELPARAM(0, (files + folders)));
				SendMessage(Progress, PBM_SETSTEP, (WPARAM)1, 0);

				this->SC_info->Progressbar_visible = TRUE;
			}
		}
		SafeRelease(&pfo);
	}
	return hr;
}

HFONT Searchcontrol::GetClientFont( int height, int cWidth_style, PCWSTR Facename )
{
	HFONT font = CreateFont(	height,
								0, 0, 0,
								cWidth_style,
								FALSE, FALSE, FALSE,
								ANSI_CHARSET,
								OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,
								DEFAULT_QUALITY,
								FF_DONTCARE,
								Facename			);
	return font;
}

BOOL Searchcontrol::InitListviewColums( HWND ListView )
{
	RECT rc;
	GetClientRect( ListView, &rc );

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for( int iCol = 0; iCol < 5; iCol++ )
	{
		lvc.iSubItem = iCol;

		if( this->SC_info->language == GERMAN )
		{
			switch( iCol )
			{
				case 0:
					lvc.pszText = L"Projekt\0";
					lvc.cx = ( rc.right / 6 );
					break;
				case 1:
					lvc.pszText = L"Programm\0";
					lvc.cx = ( rc.right / 6 );
					break;
				case 2:
					if( this->SC_info->DESC_Userdefined )
						lvc.pszText = this->SC_info->descriptions->DESC_1;
					else
						lvc.pszText = L"Zeichnungsnummer\0";
					lvc.cx = ( rc.right / 6 );
					break;
				case 3:
					if( this->SC_info->DESC_Userdefined )
						lvc.pszText = this->SC_info->descriptions->DESC_2;
					else
						lvc.pszText = L"Kunde\0";
					lvc.cx = ( rc.right / 6 );
					break;
				case 4:
					if( this->SC_info->DESC_Userdefined )
						lvc.pszText = this->SC_info->descriptions->DESC_3;
					else
						lvc.pszText = L"Bezeichnung\0";
					lvc.cx = (( rc.right / 6 ) * 2) -14;
					break;
				default:
					return FALSE;
			}
		}
		else
		{
			switch( iCol )
			{
				case 0:
					lvc.pszText = L"Project\0";
					lvc.cx = ( rc.right / 6 );
					break;
				case 1:
					lvc.pszText = L"Program\0";
					lvc.cx = ( rc.right / 6 );
					break;
				case 2:
					if( this->SC_info->DESC_Userdefined )
						lvc.pszText = this->SC_info->descriptions->DESC_1;
					else
						lvc.pszText = L"Drawing number\0";
					lvc.cx = ( rc.right / 6 );
					break;
				case 3:
					if( this->SC_info->DESC_Userdefined )
						lvc.pszText = this->SC_info->descriptions->DESC_2;
					else
						lvc.pszText = L"Customer\0";
					lvc.cx = ( rc.right / 6 );
					break;
				case 4:
					if( this->SC_info->DESC_Userdefined )
						lvc.pszText = this->SC_info->descriptions->DESC_3;
					else
						lvc.pszText = L"Description\0";
					lvc.cx = (( rc.right / 6 ) * 2 ) -14;
					break;
				default:
					return FALSE;
			}
		}
		lvc.fmt = LVCFMT_LEFT;

		if( ListView_InsertColumn( ListView, iCol, &lvc ) == -1 )
			return FALSE;
	}
	return TRUE;
}

BOOL Searchcontrol::InsertListviewItems( HWND Listview, int NumItems )
{
	if( !ListView_DeleteAllItems( Listview ))
		return FALSE;

	LVITEM lvi;

	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE;
	lvi.stateMask = 0;
	lvi.iSubItem = 0;
	lvi.state = 0;

	for( int index = 0; index < NumItems; index++ )
	{
		lvi.iItem = index;
		lvi.iImage = this->container[ index ].ImageIndex;

		if( ListView_InsertItem( Listview, &lvi ) == -1 )
			return FALSE;
	}
	return TRUE;
}

BOOL Searchcontrol::InsertSingleItem( HWND Listview, int index )
{
	if (this->SC_info->SRCItems > 300)
		return FALSE;

	LVITEM lvi;

	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE;
	lvi.stateMask = 0;
	lvi.iSubItem = 0;
	lvi.state = 0;
	lvi.iItem = index;
	lvi.iImage = this->container[ index ].ImageIndex;

	if( ListView_InsertItem( Listview, &lvi ) == -1 )
			return FALSE;

	this->SC_info->SRCItems++;

	return TRUE;
}

BOOL Searchcontrol::InitListViewImageLists( HWND Listview )
{
	BOOL result = TRUE;

	HICON hfolder;
	HICON hfile;
	HICON cnc3;
	HIMAGELIST hsmall;
	HIMAGELIST hlarge;

	hlarge = ImageList_Create( GetSystemMetrics( SM_CXICON ), GetSystemMetrics( SM_CYICON ), ILC_COLOR32, 1, 1 );
	if( !hlarge )
		return FALSE;

	hsmall = ImageList_Create( GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ), ILC_COLOR32, 1, 1 );
	if( !hsmall )
		return FALSE;

	hfolder = (HICON)LoadImage(this->SC_info->hInstance, MAKEINTRESOURCE(IDI_TV_FOLDERCLOSED), IMAGE_ICON, 24, 24, LR_DEFAULTCOLOR);//LoadIcon( this->SC_info->hInstance, MAKEINTRESOURCE(IDI_TV_FOLDERCLOSED));
	if( !hfolder )
		return FALSE;

	hfile	= (HICON)LoadImage(this->SC_info->hInstance, MAKEINTRESOURCE(IDI_TV_FILE), IMAGE_ICON, 24, 24, LR_DEFAULTCOLOR);//LoadIcon( this->SC_info->hInstance, MAKEINTRESOURCE( IDI_TV_FILE ));
	if( !hfile )
		return FALSE;

	cnc3	= (HICON)LoadImage(this->SC_info->hInstance, MAKEINTRESOURCE(IDI_SC_CNC3), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);// LoadIcon( this->SC_info->hInstance, MAKEINTRESOURCE( IDI_TV_CNC3 ));
	if( !cnc3 )
		return FALSE;

	if( ImageList_AddIcon( hlarge, hfolder ) == -1 )
	{
		result = FALSE;
		goto CleanUp;
	}
	if( ImageList_AddIcon( hlarge, hfile ) == -1 )
	{
		result = FALSE;
		goto CleanUp;
	}
	if( ImageList_AddIcon( hlarge, cnc3 ) == -1 )
	{
		result = FALSE;
		goto CleanUp;
	}
	if( ImageList_AddIcon( hsmall, hfolder ) == -1 )
	{
		result = FALSE;
		goto CleanUp;
	}
	if( ImageList_AddIcon( hsmall, hfile ) == -1 )
	{
		result = FALSE;
		goto CleanUp;
	}
	if( ImageList_AddIcon( hsmall, cnc3 ) == -1 )
	{
		result = FALSE;
		goto CleanUp;
	}
	ListView_SetImageList( Listview, hlarge, LVSIL_NORMAL );
	ListView_SetImageList( Listview, hsmall, LVSIL_SMALL );

CleanUp:

	DestroyIcon( hfolder );
	DestroyIcon( hfile );
	DestroyIcon( cnc3 );

	return result;
}

void Searchcontrol::OnSize( HWND SearchWnd )
{
	RECT rc;
	GetClientRect( SearchWnd, &rc );

	MoveWindow(
		GetDlgItem( SearchWnd, ID_SEARCHEDIT ),
		0,
		0,
		rc.right - DPIScale(40),
		DPIScale(30),
		TRUE
	);
	MoveWindow(
		GetDlgItem( SearchWnd, ID_STARTSEARCHING ),
		rc.right - DPIScale(40),
		0,
		DPIScale(40),
		DPIScale(30),
		TRUE
	);
	MoveWindow(
		GetDlgItem( SearchWnd, ID_SEARCHLIST ),
		0,
		DPIScale(30),
		rc.right,
		rc.bottom - DPIScale(58),
		FALSE
	);
	MoveWindow(
		GetDlgItem( SearchWnd, ID_SEARCHOPEN ),
		rc.right - DPIScale(120), 
		rc.bottom - DPIScale(28), 
		DPIScale(120), 
		DPIScale(28), 
		TRUE
	);

	if (this->SC_info->Progressbar_visible)
	{
		MoveWindow(
			GetDlgItem(this->SC_info->SearchWnd, ID_SEARCHPROGRESS),
			DPIScale(2),
			rc.bottom - DPIScale(26),
			rc.right - DPIScale(122),
			DPIScale(24),
			TRUE
		);
	}
}

void Searchcontrol::SetMinMaxInfo( LPARAM lParam )
{
	LPMINMAXINFO pMMinfo = reinterpret_cast<LPMINMAXINFO>( lParam );
	if( pMMinfo != NULL )
	{
		pMMinfo->ptMinTrackSize.x = 500;
		pMMinfo->ptMinTrackSize.y = 300;
	}
}

void Searchcontrol::FinalizeProgress()
{
	INT range = (INT)SendMessage(this->lps->Progress, PBM_GETRANGE, (WPARAM)FALSE, 0);

	INT pos = (INT)SendMessage(this->lps->Progress, PBM_GETPOS, 0, 0);

	if (pos < range)
	{
		SendMessage(this->lps->Progress, PBM_SETPOS, (WPARAM)range, 0);
	}
	Sleep(800);

	SendMessage(
		FindWindow(L"SEARCHCLASS", NULL),
		WM_COMMAND,
		MAKEWPARAM(CU_DELETEPROGRESSBAR, 0), 0);
}

HRESULT Searchcontrol::SetDescriptions( PCWSTR desc1, PCWSTR desc2, PCWSTR desc3 )
{
	HRESULT hr = E_FAIL;

	this->SC_info->descriptions = new UDESC;

	if( this->SC_info->descriptions != NULL )
	{
		if( FAILED( hr = StringCbCopy( this->SC_info->descriptions->DESC_1, sizeof( this->SC_info->descriptions->DESC_1 ), desc1 ) ))
			goto End1;
		if( FAILED( hr = StringCbCopy( this->SC_info->descriptions->DESC_2, sizeof( this->SC_info->descriptions->DESC_2 ), desc2 ) ))
			goto End1;
		if( FAILED( hr = StringCbCopy( this->SC_info->descriptions->DESC_3, sizeof( this->SC_info->descriptions->DESC_3 ), desc3 ) ))
			goto End1;
	}
	else
		goto End2;

	this->SC_info->DESC_Userdefined = TRUE;

	return S_OK;
End1:
	if( this->SC_info->descriptions != NULL )
	{
		delete this->SC_info->descriptions;
		this->SC_info->descriptions = NULL;
	}
End2:
	return hr;
}

BOOL Searchcontrol::getSettings(LPSRCHSET settings)
{
	*settings = this->SC_info->settings;

	return TRUE;
}

BOOL Searchcontrol::setSettings(LPSRCHSET settings)
{
	this->SC_info->settings = *settings;

	return this->SaveSettings(nullptr);
}

void Searchcontrol::OnNotify( HWND SearchWnd, LPARAM lParam )
{
	NMLVDISPINFO* dInfo = reinterpret_cast<NMLVDISPINFO*>( lParam );

	if( dInfo->hdr.code == LVN_GETDISPINFO )
	{
		if( !this->TryInsert( dInfo ))
			return;
	}
	LPNMITEMACTIVATE iact = reinterpret_cast<LPNMITEMACTIVATE>( lParam );

	if( iact->hdr.code == NM_DBLCLK )
	{
		this->HandleOpening( SearchWnd );
	}
	else if( iact->hdr.code == NM_RCLICK )
	{
		if( iact->iItem != -1 )
		{
			//AppendMenu( ?? );
		}
	}
	else if( iact->hdr.code == NM_CLICK )
	{
		if( iact->iItem != -1 )
		{
			this->ShowSingleResult( SearchWnd );
		}
	}
	LPNMLVKEYDOWN vkey = reinterpret_cast<LPNMLVKEYDOWN>( lParam );

	if( vkey->hdr.code == LVN_KEYDOWN )
	{
		if( vkey->wVKey == VK_RETURN )
		{
			this->HandleOpening( SearchWnd );
		}
	}
	LPNMLISTVIEW llv = reinterpret_cast<LPNMLISTVIEW>( lParam );

	if( llv->hdr.code == LVN_ITEMCHANGED )
	{
		if( iact->iItem != -1 )
		{
			this->ShowSingleResult( SearchWnd );
		}
	}
}

void Searchcontrol::OnTerminateProgressbar()
{
	HWND Progress = GetDlgItem(this->SC_info->SearchWnd, ID_SEARCHPROGRESS);

	DestroyWindow(Progress);

	this->SC_info->Progressbar_visible = FALSE;
}

LONG Searchcontrol::ConvertPixToTwips( int Charheight )
{
	HWND desktop = GetDesktopWindow( );

	HDC hdc = GetDC( desktop );

	int logPixY = GetDeviceCaps( hdc, LOGPIXELSY );

	ReleaseDC( desktop, hdc );

	int stockValue = 1440/logPixY;

	return static_cast<LONG>( (Charheight * stockValue ) );
}

void Searchcontrol::StartSearch( HWND SearchWnd )
{
	if( this->SC_info->ThreadActive )
	{
		this->SC_info->NewSearchRequested = TRUE;
		return;
	}
	if( !this->CheckCriteria( ) )
		return;

	ListView_DeleteAllItems(GetDlgItem(SearchWnd, ID_SEARCHLIST));

	TCHAR* buffer = NULL;

	HWND edit = GetDlgItem( SearchWnd, ID_SEARCHEDIT );
	if( !edit )
		return;

	GETTEXTLENGTHEX gtlx;
	gtlx.codepage = 1200;
	gtlx.flags = GTL_DEFAULT;

	int len = (int)SendMessage( edit, EM_GETTEXTLENGTHEX, reinterpret_cast<WPARAM>( &gtlx ), 0 );

	if( len < 1 )
		return;

	DWORD size = sizeof(TCHAR) * (len + 1);

	buffer = new TCHAR[ size ];

	if( buffer != NULL )
	{
		GETTEXTEX gtx;
		gtx.cb = size;
		gtx.codepage = 1200;
		gtx.flags = GT_DEFAULT;
		gtx.lpDefaultChar = NULL;
		gtx.lpUsedDefChar = NULL;

		SendMessage( edit, EM_GETTEXTEX, reinterpret_cast<WPARAM>( &gtx ), reinterpret_cast<LPARAM>( buffer ) );

		if (SUCCEEDED(this->InitProgressBar()))
		{
			this->SC_info->SRCItems = 0;

			if (!this->EnableSearchProcess(buffer))
			{
				MessageBox(NULL,
					L"Search Process failed !",
					L"Error",
					MB_OK | MB_ICONERROR);

			}
		}
		delete [] buffer;
	}
}

BOOL Searchcontrol::EnableSearchProcess( LPTSTR SearchString )
{
	DWORD dwThreadID;
	HANDLE hThread;

	LPSCTHREADDATA pThreadData = new SCTHREADDATA;
	pThreadData->Searchstring = nullptr;
	pThreadData->thisptr = NULL;

	if(pThreadData == nullptr)
		return FALSE;
	else
	{
		CopyStringToPtr(SearchString, &pThreadData->Searchstring);
		pThreadData->thisptr = reinterpret_cast<LONG_PTR>( this );
	}

	hThread =
		CreateThread(
			nullptr, 0,
			Searchcontrol::SearchProcess,
			reinterpret_cast<LPVOID>(pThreadData),
			0,
			&dwThreadID
		);
	if(hThread == nullptr)
		return FALSE;
	else
	{
		WaitForSingleObject(hThread, 20);
		CloseHandle(hThread);
		return TRUE;
	}
}

INT_PTR CALLBACK Searchcontrol::SetupProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	Searchcontrol* pSearch = NULL;

	if( message == WM_INITDIALOG )
	{
		pSearch = reinterpret_cast<Searchcontrol*>( lParam );
		SetWindowLongPtr( hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( pSearch ) );

		if( pSearch->SC_info->DESC_Userdefined )
		{
			SetWindowText( GetDlgItem( hDlg, ID_SRCHCKBX6 ), pSearch->SC_info->descriptions->DESC_1 );
			SetWindowText( GetDlgItem( hDlg, ID_SRCHCKBX7 ), pSearch->SC_info->descriptions->DESC_2 );
			SetWindowText( GetDlgItem( hDlg, ID_SRCHCKBX8 ), pSearch->SC_info->descriptions->DESC_3 );
		}
		pSearch->SetValues( hDlg );
	}
	else
	{
		pSearch = reinterpret_cast<Searchcontrol*>( GetWindowLongPtr( hDlg, GWLP_USERDATA ) );

		if( pSearch != NULL )
		{
			switch( message )
			{
				case WM_COMMAND:
					switch( LOWORD( wParam ))
					{
						case IDOK:
							pSearch->SaveSettings( hDlg );
							EndDialog( hDlg, TRUE );
							break;
						case IDCANCEL:
							EndDialog( hDlg, FALSE );
							break;
						default:
							break;
					}
					return FALSE;
				default:
					break;
			}
		}
	}
	return FALSE;
}

DWORD WINAPI Searchcontrol::SearchProcess( LPVOID lParam )
{
	auto threadData = reinterpret_cast<LPSCTHREADDATA>(lParam);
	if (threadData != nullptr)
	{
		auto pSearch = reinterpret_cast<Searchcontrol*>(threadData->thisptr);
		if (pSearch != NULL)
		{
			InterlockedExchange((LONG*)&pSearch->SC_info->ThreadActive, (LONG)TRUE);

			pSearch->PrintResultDC(CLEARPRINTAREA, nullptr, nullptr);

			if (pSearch->SearchInRoot(threadData->Searchstring))
			{
				pSearch->PrintResultDC(PRINTRESULT, nullptr, nullptr);
			}
			else
			{
				if (pSearch->SC_info->SRCItems >= 300)
				{
					pSearch->PrintResultDC(PRINTMAXRESULT, nullptr, nullptr);
				}
			}
			InterlockedExchange((LONG*)&pSearch->SC_info->ThreadActive, (LONG)FALSE);
		}
		SafeDeleteArray(&threadData->Searchstring);
		SafeDelete(&threadData);
	}
	return 0;
}

void Searchcontrol::SetDefault( void )
{
	this->SC_info->settings.EnableMultipleFileOpening = 0;
	this->SC_info->settings.ExpandPathToFile = 1;
	this->SC_info->settings.CloseAfterOpening = 1;
	this->SC_info->settings.SF_Projectname = 1;
	this->SC_info->settings.SF_Filename = 1;
	this->SC_info->settings.SF_Description1 = 1;
	this->SC_info->settings.SF_Description2 = 1;
	this->SC_info->settings.SF_Description3 = 1;

	this->SaveSettings(nullptr);
}

BOOL Searchcontrol::SaveSettings( HWND hDlg )
{
	BOOL result = TRUE;
	DWORD dwBytesWritten;

	if (hDlg != nullptr)
	{
		this->SC_info->settings.EnableMultipleFileOpening = 
			Button_GetCheck(
				GetDlgItem(hDlg, ID_SRCHCKBX1)
			);
		this->SC_info->settings.ExpandPathToFile =
			Button_GetCheck(
				GetDlgItem(hDlg, ID_SRCHCKBX2)
			);
		this->SC_info->settings.CloseAfterOpening =
			Button_GetCheck(
				GetDlgItem(hDlg, ID_SRCHCKBX3)
			);
		this->SC_info->settings.SF_Projectname =
			Button_GetCheck(
				GetDlgItem(hDlg, ID_SRCHCKBX4)
			);
		this->SC_info->settings.SF_Filename =
			Button_GetCheck(
				GetDlgItem(hDlg, ID_SRCHCKBX5)
			);
		this->SC_info->settings.SF_Description1 =
			Button_GetCheck(
				GetDlgItem(hDlg, ID_SRCHCKBX6)
			);
		this->SC_info->settings.SF_Description2 =
			Button_GetCheck(
				GetDlgItem(hDlg, ID_SRCHCKBX7)
			);
		this->SC_info->settings.SF_Description3 =
			Button_GetCheck(
				GetDlgItem(hDlg, ID_SRCHCKBX8)
			);
	}
	TCHAR savebuffer[ 1024 ] = { 0 };

	StringCbPrintf(
		savebuffer,
		sizeof( savebuffer ),
		L"%d|%d|%d|%d|%d|%d|%d|%d",
		this->SC_info->settings.EnableMultipleFileOpening,
		this->SC_info->settings.ExpandPathToFile,
		this->SC_info->settings.CloseAfterOpening,
		this->SC_info->settings.SF_Projectname,
		this->SC_info->settings.SF_Filename,
		this->SC_info->settings.SF_Description1,
		this->SC_info->settings.SF_Description2,
		this->SC_info->settings.SF_Description3
	);

	size_t len;
	TCHAR path[ 4096 ] = { 0 };

	auto hr = StringCbLength( savebuffer, sizeof( savebuffer ), &len );
	_NOT_USED(hr);
	hr = StringCbPrintf( path, sizeof( path ), L"%s\\srchset.sys", this->SC_info->WorkingDir );
	_NOT_USED(hr);

	HANDLE hFile = CreateFile(	path,
								GENERIC_WRITE,
								FILE_SHARE_WRITE,
								NULL,
								CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL,
								NULL					);

	if( hFile == INVALID_HANDLE_VALUE )
		result = FALSE;
	else
	{
		result = WriteFile( hFile, savebuffer, (DWORD)len, &dwBytesWritten, NULL );

		CloseHandle( hFile );
	}
	return result;
}

BOOL Searchcontrol::LoadSettings( void )
{
	LARGE_INTEGER li;
	DWORD dwBytesRead;
	BOOL result = TRUE;

	TCHAR loadbuffer[ 1024 ] = { 0 };
	TCHAR path[ 4096 ] = { 0 };

	StringCbPrintf(
		path,
		sizeof( path ),
		L"%s\\srchset.sys",
		this->SC_info->WorkingDir
	);

	HANDLE hFile = CreateFile(	path,
								GENERIC_READ,
								FILE_SHARE_READ,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL					);

	if( hFile == INVALID_HANDLE_VALUE )
		result = FALSE;
	else
	{
		result = GetFileSizeEx( hFile, &li );

		if( result )
		{
			result = ReadFile( hFile, loadbuffer, li.LowPart, &dwBytesRead, NULL );

			if( result )
			{
#if defined(_WIN64)
				swscanf_s(
					loadbuffer,
					L"%lld|%lld|%lld|%lld|%lld|%lld|%lld|%lld",
					&this->SC_info->settings.EnableMultipleFileOpening,
					&this->SC_info->settings.ExpandPathToFile,
					&this->SC_info->settings.CloseAfterOpening,
					&this->SC_info->settings.SF_Projectname,
					&this->SC_info->settings.SF_Filename,
					&this->SC_info->settings.SF_Description1,
					&this->SC_info->settings.SF_Description2,
					&this->SC_info->settings.SF_Description3
				);
#else
				swscanf_s(
					loadbuffer,
					L"%d|%d|%d|%d|%d|%d|%d|%d",
					&this->SC_info->settings.EnableMultipleFileOpening,
					&this->SC_info->settings.ExpandPathToFile,
					&this->SC_info->settings.CloseAfterOpening,
					&this->SC_info->settings.SF_Projectname,
					&this->SC_info->settings.SF_Filename,
					&this->SC_info->settings.SF_Description1,
					&this->SC_info->settings.SF_Description2,
					&this->SC_info->settings.SF_Description3
				);
#endif
			}
		}
		CloseHandle( hFile );
	}
	return result;
}

void Searchcontrol::SetValues( HWND hDlg )
{
	Button_SetCheck(
		GetDlgItem( hDlg, ID_SRCHCKBX1 ),
		this->SC_info->settings.EnableMultipleFileOpening
	);
	Button_SetCheck(
		GetDlgItem( hDlg, ID_SRCHCKBX2 ),
		this->SC_info->settings.ExpandPathToFile
	);
	Button_SetCheck(
		GetDlgItem( hDlg, ID_SRCHCKBX3 ),
		this->SC_info->settings.CloseAfterOpening
	);
	Button_SetCheck(
		GetDlgItem( hDlg, ID_SRCHCKBX4 ),
		this->SC_info->settings.SF_Projectname
	);
	Button_SetCheck(
		GetDlgItem( hDlg, ID_SRCHCKBX5 ),
		this->SC_info->settings.SF_Filename
	);
	Button_SetCheck(
		GetDlgItem( hDlg, ID_SRCHCKBX6 ),
		this->SC_info->settings.SF_Description1
	);
	Button_SetCheck(
		GetDlgItem( hDlg, ID_SRCHCKBX7 ),
		this->SC_info->settings.SF_Description2
	);
	Button_SetCheck(
		GetDlgItem( hDlg, ID_SRCHCKBX8 ),
		this->SC_info->settings.SF_Description3
	);
}

void Searchcontrol::HandleOpening( HWND SearchWnd )
{
	UINT cnt = ListView_GetSelectedCount( GetDlgItem( SearchWnd, ID_SEARCHLIST ) );

	if( cnt > 0 )
	{
		if( ( cnt > 1 ) && ( this->SC_info->settings.EnableMultipleFileOpening == 0 ) )
		{
			if( this->SC_info->language == GERMAN )
				MessageBox( NULL, L"Es sind mehrere Programme ausgewählt.\nMultiple Programmöffnung ist nicht eingestellt.", L"Öffnen...", MB_OK | MB_ICONINFORMATION );
			else
				MessageBox( NULL, L"Several programs are selected.\nMultiple program opening is not set.", L"Open...", MB_OK | MB_ICONINFORMATION );
			return;
		}
		else if( ( cnt > 1 ) && ( this->SC_info->settings.EnableMultipleFileOpening == 1 ) )
		{
			if( cnt > 4 )
			{
				if( this->SC_info->language == GERMAN )
					MessageBox( NULL, L"Es sind zu viele Programme ausgewählt.\nDie maximal erlaubte Anzahl ist 4.", L"Öffnen...", MB_OK | MB_ICONINFORMATION );
				else
					MessageBox( NULL, L"Too much programs are selected.\nMaximum is 4.", L"Open...", MB_OK | MB_ICONINFORMATION );
				return;
			}
			else
			{
				int start = ListView_GetSelectionMark( GetDlgItem( SearchWnd, ID_SEARCHLIST ) );

				OPENRESULT or;
				or.resultCount = cnt;
				or.expandPath  = (int)this->SC_info->settings.ExpandPathToFile;

				for( int j = 0; j < (int)cnt; j++ )
				{
					if( this->container[ start + j ].dwMatchFlags & RSLT_TYPE_FILE )
					{
						or.OpenType[ j ] = RSLT_TYPE_FILE;
						or.openPath[ j ] = this->container[ start + j ].FilePath;
					}
					else if( this->container[ start + j ].dwMatchFlags & RSLT_TYPE_DIRECTORY )
					{
						or.OpenType[ j ] = RSLT_TYPE_DIRECTORY;
						or.openPath[ j ] = this->container[ start + j ].ProjectPath;
					}
					else if( this->container[ start + j ].dwMatchFlags & RSLT_TYPE_EMPTYDIR )
					{
						or.OpenType[ j ] = RSLT_TYPE_EMPTYDIR;
						or.openPath[ j ] = this->container[ start + j ].ProjectPath;
					}
					else
					{
						or.OpenType[ j ] = RSLT_INVALID;
						or.openPath[ j ] = NULL;
					}
				}
				SendMessage( this->SC_info->MainWindow, WM_HANDLESEARCHRESULT, 0, reinterpret_cast<LPARAM>( &or ) );
			}
		}
		else
		{
			int start = ListView_GetSelectionMark( GetDlgItem( SearchWnd, ID_SEARCHLIST ) );

			OPENRESULT or;
			or.resultCount = cnt;
			or.expandPath  = (int)this->SC_info->settings.ExpandPathToFile;

			if( this->container[ start ].dwMatchFlags & RSLT_TYPE_FILE )
			{
				or.OpenType[ 0 ] = RSLT_TYPE_FILE;
				or.openPath[ 0 ] = this->container[ start ].FilePath;
			}
			else if( this->container[ start ].dwMatchFlags & RSLT_TYPE_DIRECTORY )
			{
				or.OpenType[ 0 ] = RSLT_TYPE_DIRECTORY;
				or.openPath[ 0 ] = this->container[ start ].ProjectPath;
			}
			else if( this->container[ start ].dwMatchFlags & RSLT_TYPE_EMPTYDIR )
			{
				or.OpenType[ 0 ] = RSLT_TYPE_EMPTYDIR;
				or.openPath[ 0 ] = this->container[ start ].ProjectPath;
			}
			else
			{
				if( this->SC_info->language == GERMAN )
					MessageBox( NULL, L"Eintrag kann nicht geöffnet werden.", L"Öffnen...", MB_OK | MB_ICONINFORMATION );
				else
					MessageBox( NULL, L"Entry cannot be opened", L"Open...", MB_OK | MB_ICONINFORMATION );
				return;
			}
			SendMessage( this->SC_info->MainWindow, WM_HANDLESEARCHRESULT, 0, reinterpret_cast<LPARAM>( &or ) );
		}
		if( this->SC_info->settings.CloseAfterOpening == 1 )
		{
			SendMessage( SearchWnd, WM_CLOSE, 0,0 );
		}
	}
}

BOOL Searchcontrol::CheckCriteria( void )
{
	if( ( this->SC_info->settings.SF_Projectname == 0 ) &&
		( this->SC_info->settings.SF_Filename == 0 ) &&
		( this->SC_info->settings.SF_Description1 == 0 ) &&
		( this->SC_info->settings.SF_Description2 == 0 ) &&
		( this->SC_info->settings.SF_Description3 == 0 )		)
	{
		if( this->SC_info->language == GERMAN )
			MessageBox( NULL, L"Keine Suchkriterien ausgewählt.", L"Suchen", MB_OK | MB_ICONINFORMATION );
		else
			MessageBox( NULL, L"No searchcriteria selected.", L"Search", MB_OK | MB_ICONINFORMATION );

		return FALSE;
	}
	return TRUE;
}

BOOL Searchcontrol::SearchForString( LPCTSTR Searchstring, LPCTSTR buffer )
{
	if( ( Searchstring == NULL ) || ( buffer == NULL ) )
		return FALSE;

	int i = 0;
	int j = 0;

	while( buffer[ i ] != Searchstring[ 0 ] )
	{
		if( buffer[ i ] == L'\0' )
			return FALSE;

		i++;
	}
	while( buffer[ i ] == Searchstring[ j ] )
	{
		if (Searchstring[j] == L'\0')
		{
			if (j == 1)
				return FALSE;
			else
				return TRUE;
		}
		if( buffer[ i ] == L'\0' )
			return FALSE;
		i++;
		j++;
	}
	if( Searchstring[ j ] == L'\0' )
	{
		if (j == 1)
			return FALSE;
		else
			return TRUE;
	}
	j = 0;
	
	return FALSE;
}

BOOL Searchcontrol::SearchInRoot( LPTSTR Searchstring )
{
	if( this->container != nullptr )
	{
		if (this->SC_info->Contsize < 2)
			delete this->container;
		else
			delete[] this->container;
		
		this->container = nullptr;
		this->SC_info->Contsize = 0;
	}
	this->lps = new THREADSTORAGE;

	if( this->lps != nullptr )
	{
		this->lps->Progress
			= GetDlgItem(
				FindWindow(L"SEARCHCLASS", NULL),
				ID_SEARCHPROGRESS
			);
	}
	else
		return FALSE;

	BOOL immediateExit = TRUE;
	BOOL result = TRUE;
	HRESULT hr;
	TCHAR* rootPath = NULL;
	TCHAR* nextPath = NULL;
	WIN32_FIND_DATA rootData;
	WIN32_FIND_DATA nextData;
	HANDLE hFindRoot = INVALID_HANDLE_VALUE;
	HANDLE hFindNext = INVALID_HANDLE_VALUE;

	int index = 0;

	size_t buffer_len, name_len;

	hr = StringCbLength( this->SC_info->RootSearchDir, UNIC_PATH, &buffer_len );
	if( FAILED( hr ))
	{
		immediateExit = FALSE;
	}
	else
	{
		rootPath = new TCHAR[ buffer_len + 15 ];

		hr = StringCbPrintf( rootPath, sizeof( TCHAR ) * ( buffer_len + 15 ), L"\\\\?\\%s\\*", this->SC_info->RootSearchDir );
		if( FAILED( hr ))
		{
			immediateExit = FALSE;
		}
		else
		{
			hFindRoot = FindFirstFile( rootPath, &rootData );

			this->ReconvertPath( rootPath );
		}
	}
	if( (hFindRoot == INVALID_HANDLE_VALUE ) || ( !immediateExit ) )
	{
		result = FALSE;
	}
	else
	{
		while( rootData.cFileName[ 0 ] == '.' )
		{
			if( FindNextFile( hFindRoot, &rootData ) == 0 )
			{
				immediateExit = this->ResultDisplayAndStorage( RSLT_TYPE_EMPTYDIR | SC_ERROR_NOSEARCHABLEDIRECTORY, index, NULL,NULL,NULL,NULL,NULL,NULL,NULL,0 );
				break;
			}
		}
		do
		{
			if( !immediateExit )
				break;

			this->Interrupt( );

			if( rootData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				hr = StringCbLength( rootPath, UNIC_PATH, &buffer_len );
				if( FAILED( hr ))
				{
					immediateExit = FALSE;
				}
				else
				{
					hr = StringCbLength( rootData.cFileName, MAX_PATH * sizeof( TCHAR ), &name_len );
					if( FAILED( hr ))
					{
						immediateExit = FALSE;
					}
					else
					{
						nextPath = new TCHAR[ (( sizeof( TCHAR )* 10) + buffer_len + name_len ) ];

						if( nextPath != NULL )
						{
							hr = StringCbPrintf(
								nextPath,
								(( sizeof( TCHAR )* 10) + buffer_len + name_len ),
								L"\\\\?\\%s\\%s\\*",
								rootPath,
								rootData.cFileName );
							if( FAILED( hr ))
							{
								immediateExit = FALSE;
							}
							else
							{
								hFindNext = FindFirstFile( nextPath, &nextData );

								this->ReconvertPath( nextPath );

								if( hFindNext == INVALID_HANDLE_VALUE )
								{
									immediateExit = FALSE;
								}
								else
								{
									BOOL Proceed = TRUE;

									while( nextData.cFileName[ 0 ] == '.' )
									{
										if( FindNextFile( hFindNext, &nextData ) == 0 )
										{
											immediateExit = this->Analyse( Searchstring, index, NULL, rootData.cFileName, nextPath, RSLT_TYPE_EMPTYDIR );
											SendMessage(this->lps->Progress, PBM_STEPIT, 0, 0);
											Proceed = FALSE;
											break;
										}
									}
									if( Proceed )
									{
										immediateExit = this->Analyse( Searchstring, index, NULL, rootData.cFileName, nextPath, RSLT_TYPE_DIRECTORY ); 
										SendMessage(this->lps->Progress, PBM_STEPIT, 0, 0);

										this->Interrupt( );

										if( immediateExit )
										{
											immediateExit = this->ProcessNextLevel(1, index, Searchstring, nextPath, &nextData, hFindNext);
										}
									}
									FindClose(hFindNext);
								}
							}
							SafeDeleteArray(&nextPath);
						}
						else
						{
							immediateExit = FALSE;
						}
					}
				}
			}
			else
			{
				immediateExit = this->Analyse(Searchstring, index, rootData.cFileName, NULL, rootPath, 0);
				SendMessage(this->lps->Progress, PBM_STEPIT, 0, 0);
			}
		}while(FindNextFile(hFindRoot, &rootData) != 0);

		SafeDeleteArray(&rootPath);
		FindClose(hFindRoot);
	}
	this->CleanUpThreadMemory( );

	return immediateExit;
}

BOOL Searchcontrol::ResultDisplayAndStorage( DWORD dwFlags, int& Index, LPCTSTR filename, LPCTSTR filepath, LPCTSTR projectname, LPCTSTR projectpath, LPCTSTR desc1, LPCTSTR desc2, LPCTSTR desc3, int image )
{
	HRESULT hr;

	HWND search = FindWindow( L"SEARCHCLASS\0", NULL );

	if( dwFlags & SC_ERROR_NOSEARCHABLEDIRECTORY )
	{
		if( dwFlags & RSLT_TYPE_EMPTYDIR )
			this->PrintResultDC( PRINTINPUTSTRING, L"Fehler: Stammverzeichnis ist leer ...", L"Error: Root directory is empty ..." );
		else
			this->PrintResultDC( PRINTINPUTSTRING, L"Fehler: Stammverzeichnis enthält keine lesbaren daten ...", L"Error: Root directory contains no readable data ..." );

		return FALSE;
	}
	else if(( dwFlags & RSLT_TYPE_EMPTYDIR ) || ( dwFlags & RSLT_TYPE_DIRECTORY ))
	{
		if( !this->ExeedContainer( ))
		{
			this->PrintResultDC( PRINTINPUTSTRING, L"Internal error >> exeeding memory failed", L"Internal error >> exeeding memory failed" );

			return FALSE;
		}

		this->container[ Index ].dwMatchFlags = dwFlags;
		this->container[ Index ].InitialIndex = Index;
		this->container[ Index ].ImageIndex   = image;

		if( projectname != NULL )
		{
			hr = StringCbCopy( this->container[ Index ].ProjectName, sizeof( this->container[ Index ].ProjectName ), projectname );
			if( FAILED( hr ))
				return FALSE;
		}
		if( projectpath != NULL )
		{
			hr = StringCbCopy( this->container[ Index ].ProjectPath, sizeof( this->container[ Index ].ProjectPath ), projectpath );
			if( FAILED( hr ))
				return FALSE;
		}
	}
	else
	{
		if( !this->ExeedContainer( ))
		{
			this->PrintResultDC( PRINTINPUTSTRING, L"Internal error >> exeeding memory failed", L"Internal error >> exeeding memory failed" );

			return FALSE;
		}

		this->container[ Index ].dwMatchFlags = dwFlags;
		this->container[ Index ].InitialIndex = Index;
		this->container[ Index ].ImageIndex   = image;

		if( projectname != NULL )
		{
			hr = StringCbCopy( this->container[ Index ].ProjectName, sizeof( this->container[ Index ].ProjectName ), projectname );
			if( FAILED( hr ))
				return FALSE;
		}
		if( projectpath != NULL )
		{
			hr = StringCbCopy( this->container[ Index ].ProjectPath, sizeof( this->container[ Index ].ProjectPath ), projectpath );
			if( FAILED( hr ))
				return FALSE;
		}
		if( filename != NULL )
		{
			hr = StringCbCopy( this->container[ Index ].FileName, sizeof( this->container[ Index ].FileName ), filename );
			if( FAILED( hr ))
				return FALSE;
		}
		if( filepath != NULL )
		{
			hr = StringCbCopy( this->container[ Index ].FilePath, sizeof( this->container[ Index ].FilePath ), filepath );
			if( FAILED( hr ))
				return FALSE;
		}
		if( desc1 != NULL )
		{
			hr = StringCbCopy( this->container[ Index ].Description_ONE, sizeof( this->container[ Index ].Description_ONE ), desc1 );
			if( FAILED( hr ))
				return FALSE;
		}
		if( desc2 != NULL )
		{
			hr = StringCbCopy( this->container[ Index ].Description_TWO, sizeof( this->container[ Index ].Description_TWO ), desc2 );
			if( FAILED( hr ))
				return FALSE;
		}
		if( desc3 != NULL )
		{
			hr = StringCbCopy( this->container[ Index ].Description_THREE, sizeof( this->container[ Index ].Description_THREE ), desc3 );
			if( FAILED( hr ))
				return FALSE;
		}
	}
//	BOOL result = this->InsertListviewItems( GetDlgItem( search, ID_SEARCHLIST ), Index + 1 );
	BOOL result = this->InsertSingleItem( GetDlgItem( search, ID_SEARCHLIST ), Index );

	if( result )
	{
		Sleep( 10 );
	}
	Index++;

	return result;
}

void Searchcontrol::PrintResultDC( int Mode, LPTSTR germantext, LPTSTR englishtext )
{
	int c = 0;
	TCHAR *buffer = NULL;

	if( Mode == PRINTRESULT )
	{
		c = this->CreateSearchResultString( NULL );

		if( c == -1 )
			return;

		buffer = new TCHAR[ c + 1 ];

		if( buffer == NULL )
			return;
		else
			this->CreateSearchResultString( buffer );
	}
	else if (Mode == PRINTMAXRESULT)
	{
		iString strBuf(
			getStringFromResource(UI_SEARCHCTRL_MAXSRCHRESULT)
		);
		c = strBuf.GetLength();

		size_t len = (c + 1) * sizeof(TCHAR);

		buffer = new TCHAR[len];

		if (buffer == NULL)
			return;
		else
			StringCbCopy(
				buffer,
				len, 
				strBuf.GetData()
			);
	}
	else if( Mode == PRINTINPUTSTRING )
	{
		if( this->SC_info->language == GERMAN )
		{
			size_t len;
			auto hr = StringCbLength( germantext, sizeof( TCHAR ) * 1024, &len );
			_NOT_USED(hr);

			if( len < 1 )
				return;

			c = (int)(len / sizeof( TCHAR ));

			buffer = new TCHAR[ len + sizeof( TCHAR ) ];

			if( buffer == NULL )
				return;
			else
				StringCbCopy( buffer, len + sizeof( TCHAR ), germantext );
		}
		else
		{
			size_t len;
			auto hr = StringCbLength( englishtext, sizeof( TCHAR ) * 1024, &len );
			_NOT_USED(hr);

			if( len < 1 )
				return;

			c = (int)(len / sizeof( TCHAR ));

			buffer = new TCHAR[ len + sizeof( TCHAR ) ];

			if( buffer == NULL )
				return;
			else
				StringCbCopy( buffer, len + sizeof( TCHAR ), englishtext );
		}
	}
	else if( Mode == CLEARPRINTAREA )
	{
		HDC hdc = GetDC( this->SC_info->SearchWnd );

		RECT rc;
		GetClientRect( this->SC_info->SearchWnd, &rc );

		RECT fill = { 0, ( rc.bottom - DPIScale(28) ), ( rc.right - DPIScale(130) ), rc.bottom };
		FillRect( hdc, &fill, this->SC_info->background );

		ReleaseDC( this->SC_info->SearchWnd, hdc );
		return;
	}
	if( ( buffer != NULL ) && ( c > 0 ) )
	{
		HDC hdc = GetDC( this->SC_info->SearchWnd );

		if( hdc )
		{
			HFONT font = CreateScaledFont(20, FW_MEDIUM, APPLICATION_PRIMARY_FONT);

			if( font )
			{
				HGDIOBJ original;

				RECT rc;
				GetClientRect( this->SC_info->SearchWnd, &rc );

				RECT fill = { 0, ( rc.bottom - DPIScale(28) ), ( rc.right - DPIScale(130) ), rc.bottom };
				FillRect( hdc, &fill, this->SC_info->background );

				original = SelectObject(hdc, font);

				SIZE sz;
				GetTextExtentPoint32( hdc, buffer, c, &sz );

				if( sz.cx > ( rc.right - DPIScale(140) ))
				{
					while( sz.cx > ( rc.right - DPIScale(140) ))
					{
						c--;

						GetTextExtentPoint32( hdc, buffer, c, &sz );
					}
					buffer[ c ]		= L'.';
					buffer[ c - 1 ]	= L'.';
					buffer[ c - 2 ]	= L'.';
				}
				
				SetBkMode( hdc, TRANSPARENT );
				SetTextColor(hdc, this->SC_info->crTextcolor);

				TextOut(
					hdc,
					DPIScale(10),
					rc.bottom - ((DPIScale(28)/2) + (sz.cy/2)),
					buffer,
					c
				);
				
				SelectObject( hdc, original );
				DeleteObject( font );
			}
			ReleaseDC( this->SC_info->SearchWnd, hdc );
		}
		delete[] buffer;
	}
}

int Searchcontrol::CreateSearchResultString( LPTSTR string_out )
{
	int c = 0;

	if( this->container == NULL )
	{
		if( this->SC_info->language == GERMAN )
		{
			if( string_out != NULL )
			{	
				if( FAILED( StringCbCopy( string_out, sizeof( TCHAR ) * 17, L"Keine Treffer..." )))
				{
					return -1;
				}
			}
			return 16;
		}
		else
		{
			if( string_out != NULL )
			{	
				if( FAILED( StringCbCopy( string_out, sizeof( TCHAR ) * 14, L"No results..." )))
				{
					return -1;
				}
			}
			return 13;
		}
	}
	else
	{
		int file = 0;
		int proj = 0;
		int des1 = 0;
		int des2 = 0;
		int des3 = 0;

		for( DWORD i = 0; i < this->SC_info->Contsize; i++ )
		{
			if( this->container[ i ].dwMatchFlags & FILENAME_MATCH )
			{
				file++;
			}
			if( this->container[ i ].dwMatchFlags & PROJECT_MATCH )
			{
				proj++;
			}
			if( this->container[ i ].dwMatchFlags & DESC_1_MATCH )
			{
				des1++;
			}
			if( this->container[ i ].dwMatchFlags & DESC_2_MATCH )
			{
				des2++;
			}
			if( this->container[ i ].dwMatchFlags & DESC_3_MATCH )
			{
				des3++;
			}
		}
		TCHAR buffer[ 1024 ] = { 0 };

		if( this->SC_info->language == GERMAN )
		{
			if( FAILED( StringCbPrintf( buffer, sizeof( buffer ), L"Treffer: %i   Evaluation:  ", this->SC_info->Contsize )))
				return -1;
			else
			{
				TCHAR result[ 64 ] = { 0 };

				if( proj > 0 )
				{
					if( FAILED( StringCbPrintf( result, sizeof( result ), L"> %i in Projektname  ", proj )))
						return -1;
					else
					{
						if( FAILED( StringCbCat( buffer, sizeof( buffer ), result )))
							return -1;
					}
				}
				if( file > 0 )
				{
					SecureZeroMemory( result, sizeof( result ));

					if( FAILED( StringCbPrintf( result, sizeof( result ), L"> %i in Programmname  ", file )))
						return -1;
					else
					{
						if( FAILED( StringCbCat( buffer, sizeof( buffer ), result )))
							return -1;
					}
				}
				if(( des1 > 0 ) || ( des2 > 0 ) || ( des3 > 0 ))
				{
					SecureZeroMemory( result, sizeof( result ));

					if( FAILED( StringCbPrintf( result, sizeof( result ), L"> %i in Programmbeschreibung", ( des1 + des2 + des3 ))))
						return -1;
					else
					{
						if( FAILED( StringCbCat( buffer, sizeof( buffer ), result )))
							return -1;
					}
				}
			}
		}
		else
		{
			if( FAILED( StringCbPrintf( buffer, sizeof( buffer ), L"Results: %i   Evaluation:  ", this->SC_info->Contsize )))
				return -1;
			else
			{
				TCHAR result[ 64 ] = { 0 };

				if( proj > 0 )
				{
					if( FAILED( StringCbPrintf( result, sizeof( result ), L"> %i in Projectname  ", proj )))
						return -1;
					else
					{
						if( FAILED( StringCbCat( buffer, sizeof( buffer ), result )))
							return -1;
					}
				}
				if( file > 0 )
				{
					SecureZeroMemory( result, sizeof( result ));

					if( FAILED( StringCbPrintf( result, sizeof( result ), L"> %i in Programname  ", file )))
						return -1;
					else
					{
						if( FAILED( StringCbCat( buffer, sizeof( buffer ), result )))
							return -1;
					}
				}
				if(( des1 > 0 ) || ( des2 > 0 ) || ( des3 > 0 ))
				{
					SecureZeroMemory( result, sizeof( result ));

					if( FAILED( StringCbPrintf( result, sizeof( result ), L"> %i in Programdescription", ( des1 + des2 + des3 ))))
						return -1;
					else
					{
						if( FAILED( StringCbCat( buffer, sizeof( buffer ), result )))
							return -1;
					}
				}
			}
		}
		size_t len;
		
		if( FAILED( StringCbLength( buffer, sizeof( buffer ), &len )))
			return -1;
		else
		{
			c = (int)(len / sizeof( TCHAR ));
		}
		if( string_out != NULL )
		{
			if( FAILED( StringCbCopy( string_out, sizeof( TCHAR ) + len, buffer )))
				return -1;
		}
	}
	return c;
}

BOOL Searchcontrol::ExeedContainer( void )
{
	BOOL result = TRUE;

	if( this->SC_info->Contsize == 0 )
	{
		if( this->container == NULL )
		{
			this->container = new SEARCHRESULT;

			SecureZeroMemory( this->container, sizeof( SEARCHRESULT ));

			this->SC_info->Contsize++;
		}
		else
			return FALSE;
	}
	else if( this->SC_info->Contsize == 1 )
	{
		if( this->container == NULL )
			return FALSE;

		LPSEARCHRESULT pSR = new SEARCHRESULT;

		SecureZeroMemory( pSR, sizeof( SEARCHRESULT ));

		if( pSR != NULL )
		{
			HRESULT hr;
			
			pSR->dwMatchFlags = this->container->dwMatchFlags;
			pSR->InitialIndex = this->container->InitialIndex;
			pSR->ImageIndex	  = this->container->ImageIndex;

			hr = StringCbCopy( pSR->ProjectName, sizeof( pSR->ProjectName ), this->container->ProjectName );
			if( FAILED( hr ))
				result = FALSE;
			hr = StringCbCopy( pSR->ProjectPath, sizeof( pSR->ProjectPath ), this->container->ProjectPath );
			if( FAILED( hr ))
				result = FALSE;
			hr = StringCbCopy( pSR->FileName, sizeof( pSR->FileName ), this->container->FileName );
			if( FAILED( hr ))
				result = FALSE;
			hr = StringCbCopy( pSR->FilePath, sizeof( pSR->FilePath ), this->container->FilePath );
			if( FAILED( hr ))
				result = FALSE;
			hr = StringCbCopy( pSR->Description_ONE, sizeof( pSR->Description_ONE ), this->container->Description_ONE );
			if( FAILED( hr ))
				result = FALSE;
			hr = StringCbCopy( pSR->Description_TWO, sizeof( pSR->Description_TWO ), this->container->Description_TWO );
			if( FAILED( hr ))
				result = FALSE;
			hr = StringCbCopy( pSR->Description_THREE, sizeof( pSR->Description_THREE ), this->container->Description_THREE );
			if( FAILED( hr ))
				result = FALSE;

			delete this->container;
			this->container = NULL;
			this->container = new SEARCHRESULT[ 2 ];

			if( this->container != NULL )
			{
				SecureZeroMemory( this->container, sizeof( SEARCHRESULT )*2);

				this->container[ 0 ].dwMatchFlags = pSR->dwMatchFlags;
				this->container[ 0 ].InitialIndex = pSR->InitialIndex;
				this->container[ 0 ].ImageIndex   = pSR->ImageIndex;
				

				hr = StringCbCopy( this->container[ 0 ].ProjectName, sizeof(  this->container[ 0 ].ProjectName ), pSR->ProjectName );
				if( FAILED( hr ))
					result = FALSE;
				hr = StringCbCopy( this->container[ 0 ].ProjectPath, sizeof( this->container[ 0 ].ProjectPath ), pSR->ProjectPath );
				if( FAILED( hr ))
					result = FALSE;
				hr = StringCbCopy( this->container[ 0 ].FileName, sizeof( this->container[ 0 ].FileName ), pSR->FileName );
				if( FAILED( hr ))
					result = FALSE;
					hr = StringCbCopy( this->container[ 0 ].FilePath, sizeof( this->container[ 0 ].FilePath ), pSR->FilePath );
				if( FAILED( hr ))
					result = FALSE;
				hr = StringCbCopy( this->container[ 0 ].Description_ONE, sizeof( this->container[ 0 ].Description_ONE ), pSR->Description_ONE );
				if( FAILED( hr ))
					result = FALSE;
				hr = StringCbCopy( this->container[ 0 ].Description_TWO, sizeof( this->container[ 0 ].Description_TWO ), pSR->Description_TWO );
				if( FAILED( hr ))
					result = FALSE;
				hr = StringCbCopy( this->container[ 0 ].Description_THREE, sizeof( this->container[ 0 ].Description_THREE ), pSR->Description_THREE );
				if( FAILED( hr ))
					result = FALSE;
	
				this->SC_info->Contsize++;
			}
			else
				result = FALSE;

			delete pSR;
		}
		else
			return FALSE;
	}
	else
	{
		if( this->container == NULL )
			return FALSE;

		LPSEARCHRESULT pSR = new SEARCHRESULT[ this->SC_info->Contsize ];

		if( pSR != NULL )
		{
			HRESULT hr;

			for( DWORD i = 0; i < this->SC_info->Contsize; i++ )
			{
				pSR[ i ].dwMatchFlags = this->container[ i ].dwMatchFlags;
				pSR[ i ].InitialIndex = this->container[ i ].InitialIndex;
				pSR[ i ].ImageIndex	  = this->container[ i ].ImageIndex;

				hr = StringCbCopy( pSR[ i ].ProjectName, sizeof( pSR[ i ].ProjectName ), this->container[ i ].ProjectName );
				if( FAILED( hr ))
					result = FALSE;
				hr = StringCbCopy( pSR[ i ].ProjectPath, sizeof( pSR[ i ].ProjectPath ), this->container[ i ].ProjectPath );
				if( FAILED( hr ))
					result = FALSE;
				hr = StringCbCopy( pSR[ i ].FileName, sizeof( pSR[ i ].FileName ), this->container[ i ].FileName );
				if( FAILED( hr ))
					result = FALSE;
				hr = StringCbCopy( pSR[ i ].FilePath, sizeof( pSR[ i ].FilePath ), this->container[ i ].FilePath );
				if( FAILED( hr ))
					result = FALSE;
				hr = StringCbCopy( pSR[ i ].Description_ONE, sizeof( pSR[ i ].Description_ONE ), this->container[ i ].Description_ONE );
				if( FAILED( hr ))
					result = FALSE;
				hr = StringCbCopy( pSR[ i ].Description_TWO, sizeof( pSR[ i ].Description_TWO ), this->container[ i ].Description_TWO );
				if( FAILED( hr ))
					result = FALSE;
				hr = StringCbCopy( pSR[ i ].Description_THREE, sizeof( pSR[ i ].Description_THREE ), this->container[ i ].Description_THREE );
				if( FAILED( hr ))
					result = FALSE;
			}
			delete[] this->container;
			this->container = NULL;
			this->container = new SEARCHRESULT[ this->SC_info->Contsize + 1 ];

			if( this->container != NULL )
			{
				SecureZeroMemory( this->container, sizeof( SEARCHRESULT ) * ( this->SC_info->Contsize + 1) );

				for( DWORD j = 0; j < this->SC_info->Contsize; j++ )
				{
					this->container[ j ].dwMatchFlags = pSR[ j ].dwMatchFlags;
					this->container[ j ].InitialIndex = pSR[ j ].InitialIndex;
					this->container[ j ].ImageIndex   = pSR[ j ].ImageIndex;

					hr = StringCbCopy( this->container[ j ].ProjectName, sizeof(  this->container[ j ].ProjectName ), pSR[ j ].ProjectName );
					if( FAILED( hr ))
						result = FALSE;
					hr = StringCbCopy( this->container[ j ].ProjectPath, sizeof( this->container[ j ].ProjectPath ), pSR[ j ].ProjectPath );
					if( FAILED( hr ))
						result = FALSE;
					hr = StringCbCopy( this->container[ j ].FileName, sizeof( this->container[ j ].FileName ), pSR[ j ].FileName );
					if( FAILED( hr ))
						result = FALSE;
						hr = StringCbCopy( this->container[ j ].FilePath, sizeof( this->container[ j ].FilePath ), pSR[ j ].FilePath );
					if( FAILED( hr ))
						result = FALSE;
					hr = StringCbCopy( this->container[ j ].Description_ONE, sizeof( this->container[ j ].Description_ONE ), pSR[ j ].Description_ONE );
					if( FAILED( hr ))
						result = FALSE;
					hr = StringCbCopy( this->container[ j ].Description_TWO, sizeof( this->container[ j ].Description_TWO ), pSR[ j ].Description_TWO );
					if( FAILED( hr ))
						result = FALSE;
					hr = StringCbCopy( this->container[ j ].Description_THREE, sizeof( this->container[ j ].Description_THREE ), pSR[ j ].Description_THREE );
					if( FAILED( hr ))
						result = FALSE;
				}
				this->SC_info->Contsize++;
			}
			else
				result = FALSE;

			delete[] pSR;
		}
		else
			return FALSE;
	}

	return result;
}

void Searchcontrol::Interrupt( void )
{
	if( this->SC_info->TerminationRequested || this->SC_info->NewSearchRequested )
	{
		this->SC_info->ThreadActive = FALSE;

		this->CleanUpThreadMemory();

		HWND search = FindWindow( L"SEARCHCLASS", NULL );
		
		if( !search )
			MessageBox( NULL, L"Searchwindow not found !", L"Terminate thread", MB_OK | MB_ICONERROR );
		else
		{
			if( this->SC_info->TerminationRequested )
			{
				PostMessage( search, WM_CLOSE, 0, 0 );
			}
			else if( this->SC_info->NewSearchRequested )
			{
				this->SC_info->NewSearchRequested = FALSE;

				PostMessage( search, WM_COMMAND, static_cast<WPARAM>( LOWORD( ID_STARTSEARCHING )), 0 );
			}
		}


		ExitThread( 2 );
	}
}

void Searchcontrol::OnClose( HWND SearchWnd )
{
	if( this->SC_info->ThreadActive )
	{
		this->SC_info->TerminationRequested = TRUE;
	}
	else
	{
		EnableWindow(this->SC_info->MainWindow, TRUE);

		DestroyWindow( SearchWnd );
	}
}

void Searchcontrol::CleanUpThreadMemory( void )
{
	this->FinalizeProgress();

	SafeDelete(&this->lps);
}

BOOL Searchcontrol::ProcessNextLevel( int level, int& index, LPTSTR Searchstring, LPTSTR subPath, LPWIN32_FIND_DATA subData, HANDLE hFindSub )
{
	if( level == 99 )
		return TRUE;
/////////////////////////////////////////////////////////////
	HRESULT hr;
	BOOL immediateExit = TRUE;
	size_t buffer_len, name_len;
	TCHAR* levelPath = NULL;
	WIN32_FIND_DATA levelData;
	HANDLE hFindLevel = INVALID_HANDLE_VALUE;

	do
	{
		if( !immediateExit )
			break;

		this->Interrupt( );

		if( subData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			hr = StringCbLength( subPath, UNIC_PATH, &buffer_len );
			if( FAILED( hr ))
			{
				immediateExit = FALSE;
			}
			else
			{
				hr = StringCbLength( subData->cFileName, MAX_PATH * sizeof( TCHAR ), &name_len );
				if( FAILED( hr ))
				{
					immediateExit = FALSE;
				}
				else
				{
					levelPath = new TCHAR[ (( sizeof( TCHAR )* 10) + buffer_len + name_len ) ];

					if( levelPath != NULL )
					{
						hr = StringCbPrintf(
							levelPath,
							(( sizeof( TCHAR )* 10) + buffer_len + name_len ),
							L"\\\\?\\%s\\%s\\*",
							subPath,
							subData->cFileName );

						if( FAILED( hr ))
						{
							immediateExit = FALSE;
						}
						else
						{
							hFindLevel = FindFirstFile( levelPath, &levelData );

							this->ReconvertPath( levelPath );

							if( hFindLevel == INVALID_HANDLE_VALUE )
							{
								immediateExit = FALSE;
							}
							else
							{
								BOOL Proceed = TRUE;

								while( levelData.cFileName[ 0 ] == '.' )
								{
									if( FindNextFile( hFindLevel, &levelData ) == 0 )
									{
										immediateExit = this->Analyse( Searchstring, index, NULL, subData->cFileName, levelPath, RSLT_TYPE_EMPTYDIR );
										SendMessage(this->lps->Progress, PBM_STEPIT, 0, 0);
										Proceed = FALSE;
										break;
									}
								}
								if( Proceed )
								{
									immediateExit = this->Analyse( Searchstring, index, NULL, subData->cFileName, levelPath, RSLT_TYPE_DIRECTORY );
									SendMessage(this->lps->Progress, PBM_STEPIT, 0, 0);

									this->Interrupt( );

									if( immediateExit )
									{
										immediateExit = this->ProcessNextLevel(level + 1, index, Searchstring, levelPath, &levelData, hFindLevel);
									}
								}
								FindClose(hFindLevel);
							}
						}
						SafeDeleteArray(&levelPath);
					}
					else
					{
						immediateExit = FALSE;
					}
				}
			}
		}
		else
		{
			immediateExit = this->Analyse( Searchstring, index, subData->cFileName, NULL, subPath, 0 );
			SendMessage(this->lps->Progress, PBM_STEPIT, 0, 0);
		}
	}while( FindNextFile( hFindSub, subData ) != 0 );

	//SafeDeleteArray(&levelPath);

	return immediateExit;
}

BOOL Searchcontrol::Analyse( LPTSTR Searchstring, int& index, LPTSTR filename, LPTSTR foldername, LPTSTR folderpath, DWORD Flag )
{
	if( Searchstring == NULL )
		return FALSE;

	int imageindex = this->CheckFileType( filename );
	if( imageindex == -1 )
		return TRUE;

	BOOL result = TRUE;

	if( filename == NULL )
	{
		if( foldername != NULL )
		{
			if( this->SC_info->settings.SF_Projectname != 0 )
			{
				if( this->SearchForString( Searchstring, foldername ))
				{
					if( !this->ResultDisplayAndStorage(		Flag | PROJECT_MATCH,
															index,
															NULL,
															NULL,
															foldername,
															folderpath,
															NULL,NULL,NULL,0	))
					{
						return FALSE;
					}
				}
			}
		}
		else
			return FALSE;
	}
	else
	{
		if( filename != NULL )
		{
			if( folderpath != NULL )
			{
				HRESULT hr;
				size_t file_len, fpath_len;

				TCHAR _foldername_[ 256 ] = { 0 };
				this->GetFolderName( folderpath, _foldername_ );

				hr = StringCbLength( folderpath, UNIC_PATH, &fpath_len );
				if( SUCCEEDED( hr ))
				{
					hr = StringCbLength( filename, UNIC_PATH, &file_len );
					if( SUCCEEDED( hr ))
					{
						TCHAR* filepath = new TCHAR[ ( file_len + fpath_len + ( sizeof( TCHAR ) * 5 )) ];

						if( filepath != NULL )
						{
							hr = StringCbPrintf( filepath, ( file_len + fpath_len + ( sizeof( TCHAR ) * 5 )), L"%s\\%s", folderpath, filename );
							if( SUCCEEDED( hr ))
							{
								DWORD resultFlags = RSLT_TYPE_FILE;
								BOOL success = FALSE;

								CnC3File file;
								hr =
									file.Load(filepath);

								auto description_one = file.GetProperty(CnC3File::PID_DESCRIPTION_ONE);
								auto description_two = file.GetProperty(CnC3File::PID_DESCRIPTION_TWO);
								auto description_three = file.GetProperty(CnC3File::PID_DESCRIPTION_THREE);

								if (SUCCEEDED(hr))
								{
									if (this->SC_info->settings.SF_Description1 != 0)
									{
										if (this->SearchForString(Searchstring, description_one.GetData()))
										{
											resultFlags = (resultFlags | DESC_1_MATCH);
											success = TRUE;
										}
									}
									if (this->SC_info->settings.SF_Description2 != 0)
									{
										if (this->SearchForString(Searchstring, description_two.GetData()))
										{
											resultFlags = (resultFlags | DESC_2_MATCH);
											success = TRUE;
										}
									}
									if (this->SC_info->settings.SF_Description3 != 0)
									{
										if (this->SearchForString(Searchstring, description_three.GetData()))
										{
											resultFlags = (resultFlags | DESC_3_MATCH);
											success = TRUE;
										}
									}
									if (this->SC_info->settings.SF_Filename != 0)
									{
										if (this->SearchForString(Searchstring, filename))
										{
											resultFlags = (resultFlags | FILENAME_MATCH);
											success = TRUE;
										}
									}
									if (this->SC_info->settings.SF_Projectname != 0)
									{
										if (this->SearchForString(Searchstring, _foldername_))
										{
											resultFlags = (resultFlags | PROJECT_MATCH);
											success = TRUE;
										}
									}
								}
								if( success )
								{
									//this->ConvertMultilineDescription( description_three.GetData() );

									if( !this->ResultDisplayAndStorage(		resultFlags,
																			index,
																			filename,
																			filepath,
																			_foldername_,
																			folderpath,
																			description_one.GetData(),
																			description_two.GetData(),
																			description_three.GetData(),
																			imageindex	))
									{
										result = FALSE;
									}									
								}
								//else
								//	result = FALSE;
							}
							else
								result = FALSE;

							SafeDeleteArray(&filepath);
						}
						else
							result = FALSE;
					}
					else
						result = FALSE;
				}
				else
					result = FALSE;
			}
			else
				result = FALSE;
		}
		else
			result = FALSE;
	}
	return result;
}

int Searchcontrol::CheckFileType( LPTSTR filename )
{
	if( filename == NULL )
		return 1;
	else
	{
		__try
		{
			int i = 0;

			while( filename[ i ] != L'\0' )
			{
				i++;
			}
			if( i > 5 )
			{
				if( ( filename[ i - 1 ] == L'3' ) &&
					( filename[ i - 2 ] == L'c' ) &&
					( filename[ i - 3 ] == L'n' ) &&
					( filename[ i - 4 ] == L'c' ) &&
					( filename[ i - 5 ] == L'.' ) )
				{
					return 2;
				}
				if( ( filename[ i - 1 ] == L't' ) &&
					( filename[ i - 2 ] == L'x' ) &&
					( filename[ i - 3 ] == L't' ) &&
					( filename[ i - 4 ] == L'.' ) )
				{
					return 1;
				}
				if( filename[ i - 1 ] == L'.' )// no effect....
				{
					return 1;
				}
			}
		}
		__except( GetExceptionCode( ) == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
		{
			return -1;
		}
	}
	return -1;
}

void Searchcontrol::ReconvertPath( TCHAR* path )
{
	if( path == NULL )
		return;
	else
	{
		size_t len;

		HRESULT hr = StringCbLength( path, UNIC_PATH, &len );

		if( SUCCEEDED( hr ))
		{
			TCHAR* buffer = new TCHAR[ len + sizeof( TCHAR ) ];

			if( buffer != NULL )
			{
				SecureZeroMemory( buffer, ( len + sizeof( TCHAR )));

				int i = 0;

				while( path[ i ] != L'\0' )
				{
					i++;
				}
				i--;

				while( ( path[ i ] == L'\\' ) || ( path[ i ] == L'*' ) )
				{
					path[ i ] = L'\0';

					i--;

					if( i == 0 )
					{
						delete [] buffer;
						return;
					}
				}
				int j = 0;

				i = 0;

				while( ( path[ i ] != L'c' ) && ( path[ i ] != L'C' ) )
				{
					i++;

					if( path[ i ] == L'\0' )
					{
						delete [] buffer;
						return;
					}
				}
				while( path[ i ] != L'\0' )
				{
					buffer[ j ] = path[ i ];

					i++;
					j++;
				}
				buffer[ j ] = L'\0';

				SecureZeroMemory( path, len );

				StringCbCopy( path, len, buffer );

				delete [] buffer;
			}
		}
	}
}

BOOL Searchcontrol::ConvertMultilineDescription( LPTSTR desc )
{
	if( desc == NULL )
		return FALSE;

	int i = 0;
	int max_len = 0;

	size_t len;

	HRESULT hr = StringCbLength( desc, ( sizeof( TCHAR ) * 2048 ), &len );
	if( FAILED( hr ))
		return FALSE;
	else
		max_len = (int)(len / sizeof( TCHAR ));
	
	if( max_len > 0 )
	{
		while( desc[ i ] != L'\0' )
		{
			if( ( desc[ i ] == L'\n' ) || ( desc[ i ] == 0x0D ) )
			{
				desc[ i ] = L' ';
			}
			i++;

			if( i == max_len )
			{
				break;
			}
		}
	}
	else
		return FALSE;

	return TRUE;
}

BOOL Searchcontrol::GetFolderName( LPTSTR path_in, LPTSTR name_out )
{
	__try
	{
		if( ( path_in == NULL ) || ( name_out == NULL ) )
			return FALSE;
		else
		{
			int i = 0;

			while( path_in[ i ] != L'\0' )
			{
				i++;
			}
			if( i > 0 )
			{
				i--;
			}
			while( path_in[ i ] != L'\\' )
			{
				i--;

				if( i == 0 )
					break;
			}
			i++;

			int j = 0;

			while( path_in[ i ] != L'\0' )
			{
				name_out[ j ] = path_in[ i ];

				j++;
				i++;
			}
			name_out[ j ] = L'\0';
		}
	}
	__except( GetExceptionCode( ) == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		return FALSE;
	}
	return TRUE;
}

BOOL Searchcontrol::TryInsert( NMLVDISPINFO* dInfo )
{
	__try
	{
		__try
		{
			switch (dInfo->item.iSubItem)
			{
			case 0:
				StringCbCopy(dInfo->item.pszText, dInfo->item.cchTextMax *sizeof(TCHAR), this->container[dInfo->item.iItem].ProjectName);
				break;
			case 1:
				StringCbCopy(dInfo->item.pszText, dInfo->item.cchTextMax *sizeof(TCHAR), this->container[dInfo->item.iItem].FileName);
				break;
			case 2:
				StringCbCopy(dInfo->item.pszText, dInfo->item.cchTextMax *sizeof(TCHAR), this->container[dInfo->item.iItem].Description_ONE);
				break;
			case 3:
				StringCbCopy(dInfo->item.pszText, dInfo->item.cchTextMax *sizeof(TCHAR), this->container[dInfo->item.iItem].Description_TWO);
				break;
			case 4:
				StringCbCopy(dInfo->item.pszText, dInfo->item.cchTextMax *sizeof(TCHAR), this->container[dInfo->item.iItem].Description_THREE);
				break;
			default:
				break;
			}
		}
		__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			return FALSE;
		}
	}
	__except( GetExceptionCode( ) == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		return FALSE;
	}

	return TRUE;
}

void Searchcontrol::ShowSingleResult( HWND SearchWnd )
{
	UINT cnt = ListView_GetSelectedCount( GetDlgItem( SearchWnd, ID_SEARCHLIST ) );

	if( cnt == 1 )
	{
		int index = ListView_GetSelectionMark( GetDlgItem( SearchWnd, ID_SEARCHLIST ) );

		if( index > -1 )
		{
			if( !this->SC_info->ThreadActive )
			{
				if( this->container != NULL )
				{
					HRESULT hr;
					DWORD flags = this->container[ index ].dwMatchFlags;

					TCHAR GER_resultbuffer[ 1024 ] = { 0 };
					TCHAR ENG_resultbuffer[ 1024 ] = { 0 };

					if( flags & RSLT_TYPE_FILE )
					{
						hr = StringCbPrintf( GER_resultbuffer, sizeof( GER_resultbuffer ), L"%s >> Übereinstimmung in:  ", this->container[ index ].FileName );
						if( FAILED( hr ))
							return;
						hr = StringCbPrintf( ENG_resultbuffer, sizeof( GER_resultbuffer ), L"%s >> Match in:  ", this->container[ index ].FileName );
						if( FAILED( hr ))
							return;

						bool b_switch = false;

						if( flags & FILENAME_MATCH )
						{
							hr = StringCbCat( GER_resultbuffer, sizeof( GER_resultbuffer ), L"Dateiname" );
							if( FAILED( hr ))
								return;
							hr = StringCbCat( ENG_resultbuffer, sizeof( ENG_resultbuffer ), L"Filename" );
							if( FAILED( hr ))
								return;

							b_switch = true;
						}
						if( flags & PROJECT_MATCH )
						{
							if( b_switch )
							{
								hr = StringCbCat( GER_resultbuffer, sizeof( GER_resultbuffer ), L" / " );
								if( FAILED( hr ))
									return;
								hr = StringCbCat( ENG_resultbuffer, sizeof( ENG_resultbuffer ), L" / " );
								if( FAILED( hr ))
									return;
							}
							hr = StringCbCat( GER_resultbuffer, sizeof( GER_resultbuffer ), L"Projektname" );
							if( FAILED( hr ))
								return;
							hr = StringCbCat( ENG_resultbuffer, sizeof( ENG_resultbuffer ), L"Projectname" );
							if( FAILED( hr ))
								return;

							b_switch = true;
						}
						if( flags & DESC_1_MATCH )
						{
							if( b_switch )
							{
								hr = StringCbCat( GER_resultbuffer, sizeof( GER_resultbuffer ), L" / " );
								if( FAILED( hr ))
									return;
								hr = StringCbCat( ENG_resultbuffer, sizeof( ENG_resultbuffer ), L" / " );
								if( FAILED( hr ))
									return;
							}
							if( this->SC_info->DESC_Userdefined )
							{
								hr = StringCbCat( GER_resultbuffer, sizeof( GER_resultbuffer ), this->SC_info->descriptions->DESC_1 );
								if( FAILED( hr ))
									return;
								hr = StringCbCat( ENG_resultbuffer, sizeof( ENG_resultbuffer ), this->SC_info->descriptions->DESC_1 );
								if( FAILED( hr ))
									return;
							}
							else
							{
								hr = StringCbCat( GER_resultbuffer, sizeof( GER_resultbuffer ), L"Zeichnungsnummer" );
								if( FAILED( hr ))
									return;
								hr = StringCbCat( ENG_resultbuffer, sizeof( ENG_resultbuffer ), L"Drawing number" );
								if( FAILED( hr ))
									return;
							}
							b_switch = true;
						}
						if( flags & DESC_2_MATCH )
						{
							if( b_switch )
							{
								hr = StringCbCat( GER_resultbuffer, sizeof( GER_resultbuffer ), L" / " );
								if( FAILED( hr ))
									return;
								hr = StringCbCat( ENG_resultbuffer, sizeof( ENG_resultbuffer ), L" / " );
								if( FAILED( hr ))
									return;
							}
							if( this->SC_info->DESC_Userdefined )
							{
								hr = StringCbCat( GER_resultbuffer, sizeof( GER_resultbuffer ), this->SC_info->descriptions->DESC_2 );
								if( FAILED( hr ))
									return;
								hr = StringCbCat( ENG_resultbuffer, sizeof( ENG_resultbuffer ), this->SC_info->descriptions->DESC_2 );
								if( FAILED( hr ))
									return;
							}
							else
							{
								hr = StringCbCat( GER_resultbuffer, sizeof( GER_resultbuffer ), L"Kunde" );
								if( FAILED( hr ))
									return;
								hr = StringCbCat( ENG_resultbuffer, sizeof( ENG_resultbuffer ), L"Customer" );
								if( FAILED( hr ))
									return;
							}
							b_switch = true;
						}
						if( flags & DESC_3_MATCH )
						{
							if( b_switch )
							{
								hr = StringCbCat( GER_resultbuffer, sizeof( GER_resultbuffer ), L" / " );
								if( FAILED( hr ))
									return;
								hr = StringCbCat( ENG_resultbuffer, sizeof( ENG_resultbuffer ), L" / " );
								if( FAILED( hr ))
									return;
							}
							if( this->SC_info->DESC_Userdefined )
							{
								hr = StringCbCat( GER_resultbuffer, sizeof( GER_resultbuffer ), this->SC_info->descriptions->DESC_3 );
								if( FAILED( hr ))
									return;
								hr = StringCbCat( ENG_resultbuffer, sizeof( ENG_resultbuffer ), this->SC_info->descriptions->DESC_3 );
								if( FAILED( hr ))
									return;
							}
							else
							{
								hr = StringCbCat( GER_resultbuffer, sizeof( GER_resultbuffer ), L"Beschreibung" );
								if( FAILED( hr ))
									return;
								hr = StringCbCat( ENG_resultbuffer, sizeof( ENG_resultbuffer ), L"Description" );
								if( FAILED( hr ))
									return;
							}
						}
					}
					else
					{
						if( flags & RSLT_TYPE_EMPTYDIR )
						{
							hr = StringCbPrintf( GER_resultbuffer, sizeof( GER_resultbuffer ), L"%s >> ( leeres Projekt )", this->container[ index ].ProjectName );
							if( FAILED( hr ))
								return;
							hr = StringCbPrintf( ENG_resultbuffer, sizeof( GER_resultbuffer ), L"%s >> ( empty Project )", this->container[ index ].ProjectName );
							if( FAILED( hr ))
								return;

							goto Print;
						}
						else if( flags & RSLT_TYPE_DIRECTORY )
						{
							hr = StringCbPrintf( GER_resultbuffer, sizeof( GER_resultbuffer ), L"%s >> ( Projektordner )", this->container[ index ].ProjectName );
							if( FAILED( hr ))
								return;
							hr = StringCbPrintf( ENG_resultbuffer, sizeof( GER_resultbuffer ), L"%s >> ( Projectfolder )", this->container[ index ].ProjectName );
							if( FAILED( hr ))
								return;

							goto Print;
						}
						else
							return;
					}
					Print:
						this->PrintResultDC( PRINTINPUTSTRING, GER_resultbuffer,ENG_resultbuffer );
				}
			}
		}
	}
}

LRESULT Searchcontrol::OnNCActivate( HWND SearchWnd, WPARAM wParam, LPARAM lParam )
{
	UNREFERENCED_PARAMETER(SearchWnd);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	// deactivated -> don't use in cnc suite

	// BOOL status = FALSE;

	return 0;// OnNcActivate(SearchWnd, wParam, lParam, status);
}
