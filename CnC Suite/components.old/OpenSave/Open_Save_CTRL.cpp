#include"Open_Save_CTRL.h"
#include<comdef.h>

HRESULT CDialogEventHandler::OnTypeChange(IFileDialog *pfd)// USELESS ?????
{
    IFileSaveDialog *pfsd;
    HRESULT hr = pfd->QueryInterface(&pfsd);
    if (SUCCEEDED(hr))
    {
        UINT uIndex;
        hr = pfsd->GetFileTypeIndex(&uIndex);   // index of current file-type
        if (SUCCEEDED(hr))
        {
            IPropertyDescriptionList *pdl = NULL;

            switch (uIndex)
            {
            case INDEX_CNC_DOC:
                // When .cnc3 is selected, let's ask for some arbitrary property, say Title.
                hr = PSGetPropertyDescriptionListFromString(L"prop:System.Title", IID_PPV_ARGS(&pdl));
                if (SUCCEEDED(hr))
                {
                    // FALSE as second param == do not show default properties.
                    hr = pfsd->SetCollectedProperties(pdl, FALSE);
                    pdl->Release();
                }
                break;

            case INDEX_TEXTDOC:
                // When .txt is selected, let's ask for some other arbitrary property, say Author.
                hr = PSGetPropertyDescriptionListFromString(L"prop:System.Author", IID_PPV_ARGS(&pdl));
                if (SUCCEEDED(hr))
                {
                    // TRUE as second param == show default properties as well, but show Author property first in list.
                    hr = pfsd->SetCollectedProperties(pdl, TRUE);
                    pdl->Release();
                }
                break;
            }
        }
        pfsd->Release();
    }
    return hr;
}

//HRESULT CDialogEventHandler::OnItemSelected(IFileDialogCustomize *pfdc, DWORD dwIDCtl, DWORD dwIDItem)
//{
//	UNREFERENCED_PARAMETER(dwIDCtl);
//	UNREFERENCED_PARAMETER(dwIDItem);
//
//    IFileDialog *pfd = NULL;
//    HRESULT hr = pfdc->QueryInterface(&pfd);
//    if (SUCCEEDED(hr))
//    {
//
//		//MessageBox(NULL, L"item selected", L"IFileDialogEvents", MB_OK);
//
//        //if (dwIDCtl == CONTROL_RADIOBUTTONLIST)
//        //{
//        //    switch (dwIDItem)
//        //    {
//        //    case CONTROL_RADIOBUTTON1:
//        //        hr = pfd->SetTitle(L"Longhorn Dialog");
//        //        break;
//
//        //    case CONTROL_RADIOBUTTON2:
//        //        hr = pfd->SetTitle(L"Vista Dialog");
//        //        break;
//        //    }
//        //}
//        pfd->Release();
//    }
//    return hr;
//}

/************************************ OPEN/SAVE CLASS ***********************************************/

Open_Save_CTRL::Open_Save_CTRL(void)
	: p_to_Text( NULL ),
	SUCCESS( TRUE ),
	F_F_Path(NULL),
	error_indicator(0),
	CustomizationRequested(FALSE),
	lastError(S_OK),
	fileTypeMode(FILETYPEMODE_NORMAL),
	hWndOwner(nullptr)
{
	StringCbPrintf( oc_error, sizeof( oc_error ), L"NO ERROR-DESCRIPTION FOUND" );

	this->CustomText[ 0 ] = NULL;
	this->CustomText[ 1 ] = NULL;
	this->CustomText[ 2 ] = NULL;
	this->CustomText[ 3 ] = NULL;

	this->F_F_Path = new Path_Ctrl;
	this->F_F_Path->FILE_PATH_AVAILABLE = FALSE;
	this->F_F_Path->FOLDER_PATH_AVAILABLE = FALSE;
}

Open_Save_CTRL::~Open_Save_CTRL(void)
{
	if( this->p_to_Text != NULL )
	{
		delete this->p_to_Text;
		this->p_to_Text = NULL;
	}
	if( this->F_F_Path != NULL )
	{
		delete this->F_F_Path;
		this->F_F_Path = NULL;
	}
	for( int i = 0; i < 3; i++ )
	{
		if( this->CustomText[ i ] != NULL )
		{
			delete this->CustomText[ i ];
			this->CustomText[ i ] = NULL;
		}
	}
}

HRESULT Open_Save_CTRL::CDialogEventHandler_CreateInstance(REFIID riid, void **ppv)
{	// Instance creation helper
	*ppv = NULL;
	CDialogEventHandler *pDialogEventHandler = new (std::nothrow) CDialogEventHandler();
	HRESULT hr = pDialogEventHandler ? S_OK : E_OUTOFMEMORY;
	if (SUCCEEDED(hr))
	{
		hr = pDialogEventHandler->QueryInterface(riid, ppv);
		pDialogEventHandler->Release();
	}
	return hr;
}

HRESULT Open_Save_CTRL::BasicFileOpenSave(const IID Mode,int Index,TCHAR* default_ext, bool performFileOperation)
{
	// CoCreate the File Open Dialog object.
	IFileDialog *pfd = NULL;
	HRESULT hr = CoCreateInstance( Mode, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
        // Create an event handling object, and hook it up to the dialog.
        IFileDialogEvents *pfde = NULL;
        hr = CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
        if (SUCCEEDED(hr))
        {
            // Hook up the event handler.
            DWORD dwCookie;
            hr = pfd->Advise(pfde, &dwCookie);
            if (SUCCEEDED(hr))
            {
                // Set the options on the dialog.
                DWORD dwFlags;

                // Before setting, always get the options first in order not to override existing options.
                hr = pfd->GetOptions(&dwFlags);
                if (SUCCEEDED(hr))
                {
                    // In this case, get shell items only for file system items.
                    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
                    if (SUCCEEDED(hr))
                    {
						if( this->CustomizationRequested )
						{
							hr = this->SetCustomText( pfd );
						}
						if( SUCCEEDED( hr ))
						{
							// Set the file types to display only. Notice that, this is a 1-based array.

							hr = (this->fileTypeMode == FILETYPEMODE_NORMAL)
								? pfd->SetFileTypes(ARRAYSIZE(Data_Types), Data_Types)
								: pfd->SetFileTypes(ARRAYSIZE(Data_Type), Data_Type);

							//hr = pfd->SetFileTypes(ARRAYSIZE(Data_Types), Data_Types);
							if (SUCCEEDED(hr))
							{
								// Set the selected file type index
								hr = pfd->SetFileTypeIndex( Index );
								if (SUCCEEDED(hr))
								{
									// Set the default extension 
									hr = pfd->SetDefaultExtension( default_ext);
									if (SUCCEEDED(hr))
									{
										// Show the dialog
										hr = pfd->Show(this->hWndOwner);
										if (SUCCEEDED(hr))
										{
											// Obtain the result, the result is an IShellItem object.
											IShellItem *psiResult;
											hr = pfd->GetResult(&psiResult);
											if (SUCCEEDED(hr))
											{
												PWSTR pszFilePath = NULL;
												hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
												if (SUCCEEDED(hr))
												{
													if (performFileOperation)
													{
														this->additionalOperationResult = FALSE;

														if (Mode == CLSID_FileSaveDialog)
														{
															if (CheckForFileExist(pszFilePath))
															{
																this->additionalOperationResult = EXISTING_FILE_OVERWRITTEN;
															}

															this->_Write_formated_Data_toFile(
																pszFilePath,
																this->p_to_Text->MainText,
																this->p_to_Text->Description_one,
																this->p_to_Text->Description_two,
																this->p_to_Text->Description_three
															);

															this->F_F_Path->FILE_PATH_AVAILABLE = TRUE;
															StringCbCopy(this->F_F_Path->file_path, sizeof(this->F_F_Path->file_path), pszFilePath);
														}
														else if (Mode == CLSID_FileOpenDialog)
														{
															if (this->CheckFileType(pszFilePath) > 0)
															{
																hr = this->_Extract_formated_Data(pszFilePath);
																if (SUCCEEDED(hr))
																{
																	this->F_F_Path->FILE_PATH_AVAILABLE = TRUE;

																	hr = StringCbCopy(this->F_F_Path->file_path, sizeof(this->F_F_Path->file_path), pszFilePath);
																}
															}
															else
															{
																if (Index == INDEX_ALLFILES)
																{
																	this->F_F_Path->FILE_PATH_AVAILABLE = TRUE;

																	hr = StringCbCopy(this->F_F_Path->file_path, sizeof(this->F_F_Path->file_path), pszFilePath);
																}
																else
																{
																	this->error_indicator = INVALID_FILE_FORMAT;
																	hr = E_FAIL;
																}
															}
														}
													}
													else
													{
														this->F_F_Path->FILE_PATH_AVAILABLE = TRUE;
														StringCbCopy(this->F_F_Path->file_path, sizeof(this->F_F_Path->file_path), pszFilePath);
													}
													CoTaskMemFree( pszFilePath );
												}
												psiResult->Release();
											}
										}
									}
								}
							}
                        }
                    }
                }
                // Unhook the event handler.
                pfd->Unadvise(dwCookie);
            }
            pfde->Release();
        }
        pfd->Release();
    }
    return hr;
}

HRESULT Open_Save_CTRL::_WriteDataToFile(HANDLE hFile, PCWSTR pszDataIn)
{
    // First figure out our required buffer size.
    DWORD cbData = WideCharToMultiByte(CP_UTF8, 0, pszDataIn, -1, NULL, 0, NULL, NULL);		//CODEPAGE
    HRESULT hr = (cbData == 0) ? HRESULT_FROM_WIN32(GetLastError()) : S_OK;
    if (SUCCEEDED(hr))
    {
        // Now allocate a buffer of the required size, and call WideCharToMultiByte again to do the actual conversion.
        char *pszData = new (std::nothrow) CHAR[cbData];
        hr = pszData ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            hr = WideCharToMultiByte(CP_UTF8, 0, pszDataIn, -1, pszData, cbData, NULL, NULL)			//CODEPAGE
                 ? S_OK
                 : HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr))
            {
                DWORD dwBytesWritten = 0;
                hr = WriteFile(hFile, pszData, cbData - 1, &dwBytesWritten, NULL)
                     ? S_OK
                     : HRESULT_FROM_WIN32(GetLastError());
            }
            delete [] pszData;
        }
    }
    return hr;
}

HRESULT Open_Save_CTRL::_Write_formated_Data_toFile(	PCWSTR pszFileName,
														PCWSTR Main_Prog,
/*MAKE SURE THAT ALL STRINGS ARE ZERO TERMINATED !!*/	PCWSTR D_tag1,
														PCWSTR D_tag2,
														PCWSTR D_tag3	)
{
	HANDLE hFile =	CreateFileW(	pszFileName,
									GENERIC_WRITE,
									FILE_SHARE_WRITE,
									NULL,
									CREATE_ALWAYS,
									FILE_ATTRIBUTE_NORMAL,
									NULL						);

	HRESULT hr = (hFile == INVALID_HANDLE_VALUE) ? HRESULT_FROM_WIN32(GetLastError()) : S_OK;
	if ( SUCCEEDED(hr) )
	{
		const WCHAR           wszPropertyStartTag[]   = L"[PROPERTY]";			// >> 10
		const WCHAR           wszPropertyEndTag[]     = L"[ENDPROPERTY]\r\n";	// >> 15

		int string_length =	lstrlen( Main_Prog ) +
							lstrlen( D_tag1 ) +
							lstrlen( D_tag2 ) +
							lstrlen( D_tag3 ) +	81;	// 81 for 3 x (START + END) + ZERO-TERMINATOR + 4 x linefeed

		WCHAR* buffer = NULL;
		buffer = new WCHAR[ string_length ];

		if ( buffer != NULL )
		{
			hr = StringCbPrintfW(	buffer,
									string_length * sizeof( WCHAR ),
									L"%s\r\n%s%s%s\n%s%s%s\n%s%s%s",
									Main_Prog,
									wszPropertyStartTag,D_tag1,wszPropertyEndTag,
									wszPropertyStartTag,D_tag2,wszPropertyEndTag,
									wszPropertyStartTag,D_tag3,wszPropertyEndTag	);

			if (SUCCEEDED(hr))
			{
					hr = this->_WriteDataToFile( hFile, buffer );
			}
		delete[] buffer;
		}
	CloseHandle(hFile);
	}

	return hr;
}

HRESULT Open_Save_CTRL::Open_Save_Control(		Mode Mode,
												int File_Index,
												HWND Main_edit,
												HWND Descriptedit_one,
												HWND Descriptedit_two,
												HWND Descriptedit_three	)
{
	IID CLSID;

	TCHAR DEFAULT_FILE_EXTENSION [ 3 ][ 6 ] = { L"cnc3\0" , L"txt\0" , L"all\0" };

	if( ( Main_edit == NULL ) ||
		( Descriptedit_one == NULL ) ||
		( Descriptedit_two == NULL ) ||
		( Descriptedit_three == NULL ) )
	{
		this->SUCCESS = FALSE;
		StringCbCopy(	this->oc_error,
						sizeof( this->oc_error ),
						L"INVALID WINDOW HANDLE (INPUT) >> [ OPEN_SAVE_CONTROL ]" );
		return E_FAIL;
	}

	if( FAILED( this->Buffer_Alloc() ) )
	{
		this->SUCCESS = FALSE;
		StringCbCopy(	this->oc_error,
						sizeof( this->oc_error ),
						L"MEMORY ALLICATION FOR BUFFERING FAILED >> [ OPEN_SAVE_CONTROL ]" );
		return E_FAIL;
	}
	HRESULT result = S_OK;

	if( Mode == MODE_OPEN )
	{
		CLSID =  CLSID_FileOpenDialog;
	}
	else if( Mode == MODE_SAVE )
	{
		CLSID =  CLSID_FileSaveDialog;
	}
	else
	{
		this->SUCCESS = FALSE;
		StringCbCopy(	this->oc_error,
						sizeof( this->oc_error ),
						L"INVALID MODE SELECTED >> [ OPEN_SAVE_CONTROL ]" );
		result = E_FAIL;
		goto Clean_Up;
	}

	if( ( File_Index < 1 ) || ( File_Index > 3 ) )
	{
		this->SUCCESS = FALSE;
		StringCbCopy(	this->oc_error,
						sizeof( this->oc_error ),
						L"INVALID FILE EXTENSION INDEX\n >> [ OPEN_SAVE_CONTROL ]" );
		result = E_FAIL;
		goto Clean_Up;
	}

	if( Mode == MODE_SAVE )
	{
		if( FAILED( this->Retrieve_WindowText( Main_edit, Descriptedit_one, Descriptedit_two, Descriptedit_three ) ) )
		{
			this->SUCCESS = FALSE;
			StringCbCopy(	this->oc_error,
						sizeof( this->oc_error ),
						L"RETRIEVING ERROR [ GETWINDOWTEXT ]\n >> [ OPEN_SAVE_CONTROL ]" );
			result = E_FAIL;
			goto Clean_Up;
		}
	}

	HRESULT hr = this->BasicFileOpenSave( CLSID, File_Index, DEFAULT_FILE_EXTENSION[ File_Index -1 ], true );
	if( FAILED( hr ) )
	{
		this->SUCCESS = FALSE;
		StringCbCopy(	this->oc_error,
						sizeof( this->oc_error ),
						L"COMMON DLG INIT ERROR OR DLG CANCELED\n >> [ OPEN_SAVE_CONTROL ]" );		// ACTUALLY THIS IS NO ERROR BUT ...... mhh should be E_ABORT

		result = hr;
		this->error_indicator = ( ERROR_CANCELLED == HRESULT_CODE( hr ) ) ? DLG_ABORTED : -1 ;
		goto Clean_Up;
	}

	if( Mode == MODE_OPEN )
	{
		if( FAILED( this->SetOpened_WindowText( Main_edit, Descriptedit_one, Descriptedit_two, Descriptedit_three) ) )
		{
			this->SUCCESS = FALSE;
			StringCbCopy(	this->oc_error,
							sizeof( this->oc_error ),
							L"TEXT LOADING ERROR [ SETWINDOWTEXT ]\n >> [ OPEN_SAVE_CONTROL ]" );
			result = E_FAIL;
			goto Clean_Up;
		}
	}
Clean_Up:
	this->Buffer_Release();

	return result;
}

HRESULT Open_Save_CTRL::Retrieve_WindowText(HWND Main_edit,HWND Descriptedit_one,HWND Descriptedit_two,HWND Descriptedit_three)
{
	HWND Wnd_dest[ 4 ];

	TCHAR NO_TEXT[] = L"void\0";

	GETTEXTEX gtx;
	gtx.codepage		= 1200;
	gtx.flags			= GT_DEFAULT;
	gtx.lpDefaultChar	= NULL;
	gtx.lpUsedDefChar	= NULL;

	GETTEXTLENGTHEX gtlx;
	gtlx.codepage	= 1200;
	gtlx.flags		= GTL_DEFAULT;

	Wnd_dest[ 0 ] = Main_edit;
	Wnd_dest[ 1 ] = Descriptedit_one;
	Wnd_dest[ 2 ] = Descriptedit_two;
	Wnd_dest[ 3 ] = Descriptedit_three;

	for( int i = 0; i < 4; i++ )
	{
		int len =
			(int)SendMessage(
				Wnd_dest[ i ],
				EM_GETTEXTLENGTHEX,
				reinterpret_cast<WPARAM>( &gtlx ),
				static_cast<LPARAM>( 0 )
			);

		if( len == E_INVALIDARG )
		{
			return E_FAIL;
		}
		else
		{
			int buffersize = ( len == 0 ) ? 5 : len;

			gtx.cb = sizeof( TCHAR ) * ( buffersize + 1 );

			switch( i )
			{
				case 0:
				{
					this->p_to_Text->MainText = new TCHAR[ buffersize + 1 ];

					if( len == 0 )
					{
						StringCbCopy( this->p_to_Text->MainText, sizeof( TCHAR ) * ( buffersize + 1 ), NO_TEXT );
					}
					else
					{
						SendMessage(	Wnd_dest[ i ],
										EM_GETTEXTEX,
										reinterpret_cast<WPARAM>( &gtx ),
										reinterpret_cast<LPARAM>( this->p_to_Text->MainText ) );
					}
				}
				case 1:
				{
					this->p_to_Text->Description_one = new TCHAR[ buffersize + 1 ];

					if( len == 0 )
					{
						StringCbCopy( this->p_to_Text->Description_one, sizeof( TCHAR ) * ( buffersize + 1 ), NO_TEXT );
					}
					else
					{
						SendMessage(	Wnd_dest[ i ],
										EM_GETTEXTEX,
										reinterpret_cast<WPARAM>( &gtx ),
										reinterpret_cast<LPARAM>( this->p_to_Text->Description_one ) );
					}
				}
				case 2:
				{
					this->p_to_Text->Description_two = new TCHAR[ buffersize + 1 ];

					if( len == 0 )
					{
						StringCbCopy( this->p_to_Text->Description_two, sizeof( TCHAR ) * ( buffersize + 1 ), NO_TEXT );
					}
					else
					{
						SendMessage(	Wnd_dest[ i ],
										EM_GETTEXTEX,
										reinterpret_cast<WPARAM>( &gtx ),
										reinterpret_cast<LPARAM>( this->p_to_Text->Description_two ) );
					}
				}
				case 3:
				{
					this->p_to_Text->Description_three = new TCHAR[ buffersize + 1 ];

					if( len == 0 )
					{
						StringCbCopy( this->p_to_Text->Description_three, sizeof( TCHAR ) * ( buffersize + 1 ), NO_TEXT );
					}
					else
					{
						SendMessage(	Wnd_dest[ i ],
										EM_GETTEXTEX,
										reinterpret_cast<WPARAM>( &gtx ),
										reinterpret_cast<LPARAM>( this->p_to_Text->Description_three ) );
					}
				}
			}
		}
	}
	return S_OK;
}

HRESULT Open_Save_CTRL::SetOpened_WindowText(HWND Main_edit,HWND Descriptedit_one,HWND Descriptedit_two,HWND Descriptedit_three)
{
	SETTEXTEX stx;
	stx.codepage	= 1200;
	stx.flags		= ST_DEFAULT | ST_NEWCHARS;

	if( this->p_to_Text->MainText != NULL )
	{
		if( Main_edit == NULL )
		{
			this->SUCCESS = FALSE;
			StringCbCopy(	this->oc_error,
							sizeof( this->oc_error ),
							L"INVALID WINDOW HANDLE (Main) >> [ SET OPENED WINDOW TEXT ]" );
			return E_FAIL;
		}
		else
		{
			SendMessage(	Main_edit,
							EM_SETTEXTEX,
							reinterpret_cast<WPARAM>( &stx ),
							reinterpret_cast<LPARAM>( this->p_to_Text->MainText ) );
		}
	}
	if( this->p_to_Text->Description_one != NULL )
	{
		if( Descriptedit_one == NULL )
		{
			this->SUCCESS = FALSE;
			StringCbCopy(	this->oc_error,
							sizeof( this->oc_error ),
							L"INVALID WINDOW HANDLE (Descrpt 1) >> [ SET OPENED WINDOW TEXT ]" );
			return E_FAIL;
		}
		else
		{
			SendMessage(	Descriptedit_one,
							EM_SETTEXTEX,
							reinterpret_cast<WPARAM>( &stx ),
							reinterpret_cast<LPARAM>( this->p_to_Text->Description_one ) );
		}
	}
	else
	{
		SendMessage(	Descriptedit_one,
						EM_SETTEXTEX,
						reinterpret_cast<WPARAM>( &stx ),
						reinterpret_cast<LPARAM>( L"" ) );
	}
	if( this->p_to_Text->Description_two != NULL )
	{
		if( Descriptedit_two == NULL )
		{
			this->SUCCESS = FALSE;
			StringCbCopy(	this->oc_error,
							sizeof( this->oc_error ),
							L"INVALID WINDOW HANDLE (Descrpt 2) >> [ SET OPENED WINDOW TEXT ]" );
			return E_FAIL;
		}
		else
		{
			SendMessage(	Descriptedit_two,
							EM_SETTEXTEX,
							reinterpret_cast<WPARAM>( &stx ),
							reinterpret_cast<LPARAM>( this->p_to_Text->Description_two ) );
		}
	}
	else
	{
		SendMessage(	Descriptedit_two,
						EM_SETTEXTEX,
						reinterpret_cast<WPARAM>( &stx ),
						reinterpret_cast<LPARAM>( L"" ) );
	}
	if( this->p_to_Text->Description_three != NULL )
	{
		if( Descriptedit_three == NULL )
		{
			this->SUCCESS = FALSE;
			StringCbCopy(	this->oc_error,
							sizeof( this->oc_error ),
							L"INVALID WINDOW HANDLE (Descrpt 3) >> [ SET OPENED WINDOW TEXT ]" );
			return E_FAIL;
		}
		else
		{
			SendMessage(	Descriptedit_three,
							EM_SETTEXTEX,
							reinterpret_cast<WPARAM>( &stx ),
							reinterpret_cast<LPARAM>( this->p_to_Text->Description_three ) );
		}
	}
	else
	{
		SendMessage(	Descriptedit_three,
						EM_SETTEXTEX,
						reinterpret_cast<WPARAM>( &stx ),
						reinterpret_cast<LPARAM>( L"" ) );
	}
	return S_OK;
}

HRESULT Open_Save_CTRL::Buffer_Alloc(void)
{
	this->p_to_Text = new Pointer_to_Window_Text;

	if( this->p_to_Text == NULL )
	{
		return E_FAIL;
	}
	else
	{
		this->p_to_Text->MainText			= NULL;
		this->p_to_Text->Description_one	= NULL;
		this->p_to_Text->Description_two	= NULL;
		this->p_to_Text->Description_three	= NULL;
	}
	return S_OK;
}

void Open_Save_CTRL::Buffer_Release(void)
{
	if( this->p_to_Text != NULL )
	{
		SafeDeleteArray(&this->p_to_Text->MainText);
		SafeDeleteArray(&this->p_to_Text->Description_one);
		SafeDeleteArray(&this->p_to_Text->Description_two);
		SafeDeleteArray(&this->p_to_Text->Description_three);
		SafeDelete(&this->p_to_Text);
	}
}

void Open_Save_CTRL::Get_O_S_Error(void)
{
	if( !this->SUCCESS )
	{
		MessageBox( NULL, oc_error, L"Display error", MB_OK | MB_ICONERROR );
	}
}

HRESULT Open_Save_CTRL::GetTranslated_hResultErrorCode(TCHAR ** error_out)
{
	_com_error err(this->lastError);

	if ((error_out != nullptr) && (*error_out == nullptr))
	{
		CopyStringToPtr(err.ErrorMessage(), error_out);
	}
	return this->lastError;
}

void Open_Save_CTRL::SetFileTypeMode(DWORD filetype_mode)
{
	this->fileTypeMode = filetype_mode;
}

HRESULT Open_Save_CTRL::_ReadDataFromFile(HANDLE hFile, WCHAR* &high_buffer)
{
	LARGE_INTEGER lg_int;

	if( !GetFileSizeEx( hFile, &lg_int ) )
	{
		this->SUCCESS = FALSE;
		StringCbCopy(	this->oc_error,
						sizeof( this->oc_error ),
						L"GET FILE SIZE FAILED >> [ READ DATA FROM FILE ]" );
		return E_FAIL;
	}
	else
	{
		char* low_buffer = NULL;

		low_buffer = new (std::nothrow) CHAR[ lg_int.LowPart + 1 ];
		
		if( low_buffer != NULL )
		{
			DWORD dwBytesRead = 0;

			if( ReadFile( hFile, low_buffer, lg_int.LowPart, &dwBytesRead, NULL ) )
			{
				if( dwBytesRead == 0 )
				{
					this->SUCCESS = FALSE;
					StringCbCopy(	this->oc_error,
									sizeof( this->oc_error ),
									L"NO DATA READ (file empty ?) >> [ READ DATA FROM FILE ]" );
				}
				else
				{
					if( dwBytesRead < ( lg_int.LowPart + 1 ) )
					{
						low_buffer[ dwBytesRead ] = '\0';
						DWORD array_size = 0;

						if( ( array_size = MultiByteToWideChar( CP_UTF8, 0, low_buffer, -1, NULL, 0 )) != 0 )				//CODEPAGE
						{
							high_buffer = new WCHAR[ array_size ];

							if( high_buffer == NULL )
							{
								this->SUCCESS = FALSE;
								StringCbCopy(	this->oc_error,
												sizeof( this->oc_error ),
												L"HIGH BUFFER ALLOCATION FAILED >> [ READ DATA FROM FILE ]" );
							}
							else
							{
								if( MultiByteToWideChar( CP_UTF8, 0, low_buffer, -1, high_buffer, array_size ) == 0 )			//CODEPAGE
								{
									this->SUCCESS = FALSE;
									StringCbCopy(	this->oc_error,
													sizeof( this->oc_error ),
													L"TEXT CONVERSION IN WIDE CHAR FAILED >> [ READ DATA FROM FILE ]" );
								}
							}
						}
						else
						{
							this->SUCCESS = FALSE;
							StringCbCopy(	this->oc_error,
											sizeof( this->oc_error ),
											L"HIGH BUFFER SIZE-DETERMINING FAILED\n(or file is empty) >> [ READ DATA FROM FILE ]" );
						}
					}
					else
					{
						this->SUCCESS = FALSE;
						StringCbCopy(	this->oc_error,
										sizeof( this->oc_error ),
										L"LOW BUFFER OVERLOAD >> [ READ DATA FROM FILE ]" );
					}
				}
			}
			delete[] low_buffer;
		}
		else
		{
			this->SUCCESS = FALSE;
			StringCbCopy(	this->oc_error,
							sizeof( this->oc_error ),
							L"LOW BUFFER ALLOCATION FAILED >> [ READ DATA FROM FILE ]" );
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT Open_Save_CTRL::_Extract_formated_Data(PCWSTR pszFileName)
{
	HANDLE hFile =	CreateFileW(	pszFileName,
									GENERIC_READ,
									FILE_SHARE_READ,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL,
									NULL						);

	HRESULT hr = (hFile == INVALID_HANDLE_VALUE) ? HRESULT_FROM_WIN32(GetLastError()) : S_OK;
	if ( SUCCEEDED(hr) )
	{
		WCHAR* buffer = NULL;

		if( SUCCEEDED( this->_ReadDataFromFile( hFile, buffer ) ) )
		{
			if( !this->Extract( buffer ) )
			{
				this->SUCCESS = FALSE;
				StringCbCopy(	this->oc_error,
								sizeof( this->oc_error ),
								L"DATA EXTRACTION ERROR >> FILE_SYNTAX\n[ EXTRACT FORMATED DATA ]" );
			}
		}
		if( buffer != NULL )
		{
			delete buffer;
		}
		CloseHandle( hFile );
	}
	return S_OK;
}

BOOL Open_Save_CTRL::Extract( WCHAR* buffer )
{
	if( buffer == NULL )
	{
		return FALSE;
	}
	int i = 0, property_start = NO_PROPERTY;

	do
	{
		if( i > 0 )
		{
			i+=1;
		}
		while( buffer[ i ] != L'[' )
		{
			if( buffer[ i ] == L'\0' )
			{
				break;
			}
			i++;
		}

		if ((i == 0) && (buffer[i] == L'['))
		{
			if (buffer[1] != L'P')
				i++;
		}

		if( buffer[ i ] == L'\0' )
		{
			break;
		}

	}while( ( property_start = this->SearchFor_Property_begin( i, buffer, 9, L"[PROPERTY]" ) ) == NO_PROPERTY );

	if( i == 0 )
	{
		return TRUE;
	}

	this->p_to_Text->MainText = new WCHAR[ i + 1 ];

	if( property_start == NO_PROPERTY )
	{
		StringCbCopy(	this->p_to_Text->MainText,
						sizeof( WCHAR ) * ( i + 1 ),
						buffer							);
		goto Final;
	}
	else
	{
		for( int j = 0; j < i; j++ )
		{
			this->p_to_Text->MainText[ j ] = buffer[ j ];
		}
		this->p_to_Text->MainText[ i ] = '\0';

		for( int j = 0; j < 3; j++ )
		{
			property_start = this->Get_Property( property_start, j, buffer );

			if( property_start == FILE_SYNTAX_ERROR )
			{
				return FALSE;
			}
		}
		goto Final;
	}

Final:

	return TRUE;
}

int Open_Save_CTRL::SearchFor_Property_begin(int start_pos, WCHAR* buffer, int num_char_sample, WCHAR* sample)
{
	int prop_cnt = 0;

	while( sample[ prop_cnt ] == buffer[ start_pos ] )
	{
		if( prop_cnt == num_char_sample )
		{
			return start_pos + 1;
		}
		if( buffer[ start_pos ] == '\0' )
		{
			return start_pos;
		}
		prop_cnt++;
		start_pos++;
	}
	return NO_PROPERTY;
}

int Open_Save_CTRL::Get_Property( int start_pos, int prop_number, WCHAR* buffer)
{
	int array_size = 0, array_start = start_pos;

	while( this->SearchFor_Property_begin( start_pos, buffer, 12, L"[ENDPROPERTY]" ) == NO_PROPERTY )
	{
		if( buffer[ start_pos ] == '\0' )
		{
			return FILE_SYNTAX_ERROR;
		}
		start_pos++;
		array_size++;
	}

	if( prop_number == 0 )
	{
		this->p_to_Text->Description_one = new WCHAR[ array_size + 2 ];

		for( int l = 0; l < array_size; l++ )
		{
			this->p_to_Text->Description_one[ l ] = buffer[ array_start + l ];
		}
		this->p_to_Text->Description_one[ array_size ]  = '\0';
	}
	else if( prop_number == 1 )
	{
		this->p_to_Text->Description_two = new WCHAR[ array_size + 2 ];

		for( int l = 0; l < array_size; l++ )
		{
			this->p_to_Text->Description_two[ l ] = buffer[ array_start + l ];
		}
		this->p_to_Text->Description_two[ array_size ]  = '\0';
	}
	else if( prop_number == 2 )
	{
		this->p_to_Text->Description_three = new WCHAR[ array_size + 2 ];

		for( int l = 0; l < array_size; l++ )
		{
			this->p_to_Text->Description_three[ l ] = buffer[ array_start + l ];
		}
		this->p_to_Text->Description_three[ array_size ]  = '\0';
	}
	int new_pos = 0;

	if( prop_number < 2 )
	{
		while( ( new_pos = this->SearchFor_Property_begin( start_pos, buffer, 9, L"[PROPERTY]" ) ) == NO_PROPERTY )	
		{
			start_pos++;
		}
		if( new_pos == BUFFER_END )
		{
			return FILE_SYNTAX_ERROR;
		}
	}

	return new_pos;
}

BOOL Open_Save_CTRL::Save_direct(WCHAR* Path, HWND Main, HWND Descript_one, HWND Descript_two, HWND Descript_three)
{
	BOOL bRet = TRUE;

	if( FAILED( this->Buffer_Alloc() ) )
	{
		this->SUCCESS = FALSE;
		StringCbCopy(	this->oc_error,
						sizeof( this->oc_error ),
						L"MEMORY ALLICATION FOR BUFFERING FAILED >> [ SAVE_DIRECT ]" );
		return FALSE;
	}
	if( SUCCEEDED( this->Retrieve_WindowText( Main, Descript_one, Descript_two, Descript_three ) ) )
	{
		if( FAILED( this->_Write_formated_Data_toFile(	Path,
														this->p_to_Text->MainText,
														this->p_to_Text->Description_one,
														this->p_to_Text->Description_two,
														this->p_to_Text->Description_three	) ) )
		{
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

BOOL Open_Save_CTRL::Open_direct(WCHAR* Path, HWND Main, HWND Descript_one, HWND Descript_two, HWND Descript_three)
{
	if( this->CheckFileType( Path ) < 0 )
	{
		this->error_indicator = INVALID_FILE_FORMAT;

		return FALSE;
	}
	BOOL bRet = TRUE;

	if( FAILED( this->Buffer_Alloc() ) )
	{
		this->SUCCESS = FALSE;
		StringCbCopy(	this->oc_error,
						sizeof( this->oc_error ),
						L"MEMORY ALLICATION FOR BUFFERING FAILED >> [ OPEN_DIRECT ]" );
		return FALSE;
	}

	if( SUCCEEDED( this->_Extract_formated_Data( Path ) ) )
	{
		if( FAILED( this->SetOpened_WindowText( Main, Descript_one, Descript_two, Descript_three ) ) )
		{
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

HRESULT Open_Save_CTRL::OpenFolder(HWND owner, WCHAR** folderPath)
{
	// CoCreate the File Open Dialog object.
	IFileDialog *pfd = NULL;
	HRESULT hr = CoCreateInstance( CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		// Set the options on the dialog.
		DWORD dwFlags;
		// Before setting, always get the options first in order not to override existing options.
		hr = pfd->GetOptions(&dwFlags);
		if (SUCCEEDED(hr))
		{
			// In this case, get shell items only for folder items.
			hr = pfd->SetOptions(dwFlags | FOS_PICKFOLDERS);
			if (SUCCEEDED(hr))
			{
				// Show the dialog
				hr = pfd->Show(owner);
				if (SUCCEEDED(hr))
				{
					// Obtain the result, once the user clicks the 'Open' button.
					// The result is an IShellItem object.
					IShellItem *psiResult;
					hr = pfd->GetResult(&psiResult);
					if (SUCCEEDED(hr))
					{
						PWSTR pszFolderPath = NULL;
						hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);
						if (SUCCEEDED(hr))
						{
							size_t len;

							hr = StringCbLength(pszFolderPath, sizeof(TCHAR)*STRSAFE_MAX_CCH, &len);
							if (SUCCEEDED(hr))
							{
								len += sizeof(WCHAR);

								(*folderPath) = new WCHAR[len];

								hr = (*folderPath != NULL) ? S_OK : E_FAIL;
								if (SUCCEEDED(hr))
								{
									hr = StringCbCopy(*folderPath, len, pszFolderPath);
									if (SUCCEEDED(hr))
									{
										hr = StringCbCopy(this->F_F_Path->folder_path,
											sizeof(this->F_F_Path->folder_path),
											pszFolderPath);

										if (SUCCEEDED(hr))
										{
											// ...

											this->F_F_Path->FOLDER_PATH_AVAILABLE = TRUE;
										}
									}
								}
							}
							CoTaskMemFree( pszFolderPath );
						}
						psiResult->Release();
					}
				}
			}
        }
        pfd->Release();
    }
	if (FAILED(hr))
	{
		this->lastError = hr;
	}
    return hr;
}

BOOL Open_Save_CTRL::ConvertFileToCnc3(HWND MB_Owner, LPTSTR path)
{
	BOOL rAction = (BOOL)MessageBox(
		MB_Owner,
		getStringFromResource(UI_GNRL_KEEPOLDFILE),
		getStringFromResource(UI_GNRL_CONVERTTOCNC3),
		MB_YESNO | MB_ICONINFORMATION);

	if (rAction != IDCANCEL)
	{
		BOOL result;
		TCHAR* filename = NULL;

		result = (GetFilenameOutOfPath(path, &filename, TRUE) == TRUE) ? TRUE : FALSE;
		if (result)
		{
			BasicFPO* pfpo = CreateBasicFPO();

			result = (pfpo != NULL) ? TRUE : FALSE;
			if (result)
			{
				TCHAR placeholder[2] = L" ";
				TCHAR* buffer = NULL;

				result = pfpo->LoadBufferFmFileAsUtf8(&buffer, path);
				if (result)
				{
					if (buffer == nullptr)
						buffer = placeholder;

					TCHAR* folder = NULL;

					result = (CopyStringToPtrA(path, &folder) == TRUE) ? TRUE : FALSE;
					if (result)
					{
						result = pfpo->RemoveFilenameFromPath(folder);
						if (result)
						{
							TCHAR *newPath = NULL;

							if (rAction == IDNO)
							{
								result = DeleteFile(path);
								if (result)
								{
									result = AppendStringsWithVaList(&newPath, folder, L"\\", filename, L".cnc3", NULL);
								}
							}
							else if (rAction == IDYES)
							{
								result = AppendStringsWithVaList(&newPath, folder, L"\\", filename, L"(conv)", L".cnc3", NULL);
							}
							else
								result = FALSE;

							if (result)
							{
								result = pfpo->IfFileExistsChangePath(&newPath);
								if (result)
								{
									TCHAR* fOld = nullptr;
									result = AppendStringsWithVaList(&fOld, L"filename.old <", filename, L">\0", nullptr);

									if (result == TRUE)
									{
										result = this->SaveBuffersDirect(newPath, buffer, fOld, L"---", L"converted file\0");
										if (result)
										{
											TCHAR* newFilename = NULL;

											result = (pfpo->GetFilenameOutOfPath(newPath, &newFilename, FALSE) == TRUE) ? TRUE : FALSE;
											if (result)
											{
												size_t len;

												result =
													SUCCEEDED(
														StringCbLength(path, sizeof(TCHAR)*STRSAFE_MAX_CCH, &len))
													? TRUE : FALSE;
												if (result)
												{
													SecureZeroMemory(path, len);

													result =
														SUCCEEDED(
															StringCbCopy(path, len, newFilename))
														? TRUE : FALSE;
												}
												SafeDeleteArray(&newFilename);
											}
										}
									}
								}
								SafeDeleteArray(&newPath);
							}
						}
						SafeDeleteArray(&folder);
					}
				}
				SafeRelease(&pfpo);
			}
			SafeDeleteArray(&filename);
		}
		if (!result)
			rAction = (BOOL)IDCANCEL;
	}
	return rAction;// if something goes wrong return IDCANCEL !!!
}

BOOL Open_Save_CTRL::Get_requested_ID(PRETURNPATH r_path)
{
	r_path->file_name[ 0 ] = L'\0';
	r_path->file_path[ 0 ] = L'\0';
	r_path->folder_name[ 0 ] = L'\0';
	r_path->folder_path[ 0 ] = L'\0';
	
	if( this->F_F_Path->FILE_PATH_AVAILABLE )
	{
		StringCbCopy(	r_path->file_path, sizeof( WCHAR ) * 1024, this->F_F_Path->file_path );
		if( !this->Get_path_end( r_path->file_name, this->F_F_Path->file_path ) )
		{
			return FALSE;
		}
	}
	if( this->F_F_Path->FOLDER_PATH_AVAILABLE )
	{
		StringCbCopy(	r_path->folder_path, sizeof( WCHAR ) * 1024, this->F_F_Path->folder_path );
		if( !this->Get_path_end( r_path->folder_name, this->F_F_Path->folder_path ) )
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL Open_Save_CTRL::Get_path_end( WCHAR* Destination, WCHAR* Source )
{
	if( Destination == NULL )
	{
		this->SUCCESS = FALSE;
		StringCbCopy(	this->oc_error,
						sizeof( this->oc_error ),
						L"DESTINATION BUFFER INVALID >> [ GET PATH END ]" );
		return FALSE;
	}
	if( Source == NULL )
	{
		this->SUCCESS = FALSE;
		StringCbCopy(	this->oc_error,
						sizeof( this->oc_error ),
						L"SOURCE BUFFER INVALID >> [ GET PATH END ]" );
		return FALSE;
	}
	int i = 0;

	while( Source[ i ] != '\0' )
	{
		i++;
	}
	while( Source[ i ] != '\\' )
	{
		i--;
	}
	i++;

	int j = 0;

	while( Source[ i ] != '\0' )
	{
		Destination[ j ] = Source[ i ];
		j++;
		i++;
	}
	Destination[ j ] = '\0';

	return TRUE;
}

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

BOOL Open_Save_CTRL::OpenWithoutLoading( LPCTSTR Path, TCHAR** MainText, TCHAR** desc1, TCHAR** desc2, TCHAR** desc3 )//new method
{
	BOOL bRet = TRUE;

	if( FAILED( this->Buffer_Alloc() ) )
	{
		this->SUCCESS = FALSE;

		StringCbCopy(	this->oc_error,
						sizeof( this->oc_error ),
						L"MEMORY ALLOCATION FOR BUFFERING FAILED >> [ OPEN_DIRECT ]" );
		bRet = FALSE;
	}
	else
	{
		if (SUCCEEDED(this->_Extract_formated_Data(Path)))
		{
			if (this->p_to_Text != NULL)
			{
				if (this->p_to_Text->MainText != NULL)
				{
					size_t len;

					if (FAILED(
						StringCbLength(
							this->p_to_Text->MainText, (sizeof(TCHAR) * STRSAFE_MAX_CCH), &len)))
					{
						bRet = FALSE;
					}
					else
					{
						if (MainText != NULL)
						{
							*MainText = new TCHAR[len + sizeof(TCHAR)];

							if (*MainText != NULL)
							{
								if (FAILED(
									StringCbCopy(
										*MainText, (len + sizeof(TCHAR)), this->p_to_Text->MainText)))
								{
									bRet = FALSE;
								}
							}
							else
							{
								bRet = FALSE;
							}
						}
					}
				}
				else
					bRet = FALSE;

				if (this->p_to_Text->Description_one != NULL)
				{
					if (desc1 != NULL)
					{
						if (!(CopyStringToPtr(this->p_to_Text->Description_one, desc1) == TRUE))
						{
							bRet = FALSE;
						}
					}
				}
				else
				{
					bRet = FALSE;
				}

				if (this->p_to_Text->Description_two != NULL)
				{
					if (desc2 != NULL)
					{
						if (!(CopyStringToPtr(this->p_to_Text->Description_two, desc2) == TRUE))
						{
							bRet = FALSE;
						}
					}
				}
				else
				{
					bRet = FALSE;
				}

				if (this->p_to_Text->Description_three != NULL)
				{
					if (desc3 != NULL)
					{
						if (!(CopyStringToPtr(this->p_to_Text->Description_three, desc3) == TRUE))
						{
							bRet = FALSE;
						}
					}
				}
				else
				{
					bRet = FALSE;
				}
			}
		}
		else
		{
			bRet = FALSE;
		}
		this->Buffer_Release();
	}
	return bRet;
}

BOOL Open_Save_CTRL::SaveBuffersDirect(WCHAR* Path, LPTSTR Main, LPTSTR Descript_one, LPTSTR Descript_two, LPTSTR Descript_three)
{
	if( FAILED( this->_Write_formated_Data_toFile(	Path,
													Main,
													Descript_one,
													Descript_two,
													Descript_three	) ) )
	{
		return FALSE;
	}
	return TRUE;
}

int Open_Save_CTRL::CheckFileType( LPTSTR filepath )
{
	if( filepath == NULL )
		return -1;
	else
	{
		__try
		{
			int i = 0;

			while( filepath[ i ] != L'\0' )
			{
				i++;
			}
			if( i > 5 )
			{
				if( ( filepath[ i - 1 ] == L'3' ) &&
					( filepath[ i - 2 ] == L'c' ) &&
					( filepath[ i - 3 ] == L'n' ) &&
					( filepath[ i - 4 ] == L'c' ) &&
					( filepath[ i - 5 ] == L'.' ) )
				{
					return 2;
				}
				if( ( filepath[ i - 1 ] == L't' ) &&
					( filepath[ i - 2 ] == L'x' ) &&
					( filepath[ i - 3 ] == L't' ) &&
					( filepath[ i - 4 ] == L'.' ) )
				{
					return 1;
				}
				if( filepath[ i - 1 ] == L'.' )// no effect....
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

BOOL Open_Save_CTRL::GetFilePathFromUser( TCHAR **Path, int Mode, int FileIndex )
{
	IID CLSID;
	BOOL result = TRUE;

	TCHAR DEFAULT_FILE_EXTENSION [ 3 ][ 6 ] = { L"cnc3\0" , L"txt\0" , L"all\0" };

	if( Mode == MODE_OPEN )
	{
		CLSID =  CLSID_FileOpenDialog;
	}
	else if( Mode == MODE_SAVE )
	{
		CLSID =  CLSID_FileSaveDialog;
	}
	else
	{
		CLSID =  CLSID_FileOpenDialog;
	}
	HRESULT hr = this->Buffer_Alloc( );
	if( SUCCEEDED( hr ))
	{
		hr = this->BasicFileOpenSave( CLSID, FileIndex, DEFAULT_FILE_EXTENSION[ FileIndex - 1 ], false );
		if( SUCCEEDED( hr ))
		{
			if( this->F_F_Path->FILE_PATH_AVAILABLE )
			{
				size_t len;

				hr = StringCbLength( this->F_F_Path->file_path, sizeof( TCHAR ) * 1024, &len );
				if( SUCCEEDED( hr ))
				{
					*Path = new TCHAR[ len + sizeof( TCHAR ) ];

					if( *Path != NULL )
					{
						hr = StringCbCopy( *Path, len + sizeof( TCHAR ), this->F_F_Path->file_path );
						if( FAILED( hr ))
						{
							result = FALSE;
						}
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
		{
			result = ( ERROR_CANCELLED == HRESULT_CODE( hr ) ) ? DIALOG_CANCELLED : FALSE;
		}
		this->Buffer_Release( );
	}
	else
		return FALSE;

	if (FAILED(hr))
	{
		this->lastError = hr;
	}
	return result;
}

BOOL Open_Save_CTRL::OS_DialogCustomText(LPCTSTR Caption, LPCTSTR ButtonText, LPCTSTR FileText, LPCTSTR Folderpath)
{
	if( ( Caption == NULL ) &&
		( ButtonText == NULL ) &&
		( FileText == NULL ) &&
		( Folderpath == NULL ))
	{
		return FALSE;
	}
	else
	{
		size_t len;
		HRESULT hr;

		if( Caption != NULL )
		{
			hr = StringCbLength( Caption, sizeof( TCHAR )* 1024, &len );
			if( SUCCEEDED( hr ))
			{
				this->CustomText[ 0 ] = new TCHAR[ len + sizeof( TCHAR ) ];

				if( this->CustomText[ 0 ] != NULL )
				{
					hr = StringCbCopy( this->CustomText[ 0 ], len + sizeof( TCHAR ), Caption );
					if( FAILED( hr ))
					{
						return FALSE;
					}
				}
				else
					return FALSE;
			}
			else
				return FALSE;
		}
		if( ButtonText != NULL )
		{
			hr = StringCbLength( ButtonText, sizeof( TCHAR )* 100, &len );
			if( SUCCEEDED( hr ))
			{
				this->CustomText[ 1 ] = new TCHAR[ len + sizeof( TCHAR ) ];

				if( this->CustomText[ 1 ] != NULL )
				{
					hr = StringCbCopy( this->CustomText[ 1 ], len + sizeof( TCHAR ), ButtonText );
					if( FAILED( hr ))
					{
						return FALSE;
					}
				}
				else
					return FALSE;
			}
			else
				return FALSE;
		}
		if( FileText != NULL )
		{
			hr = StringCbLength( FileText, sizeof( TCHAR )* 2048, &len );
			if( SUCCEEDED( hr ))
			{
				this->CustomText[ 2 ] = new TCHAR[ len + sizeof( TCHAR ) ];

				if( this->CustomText[ 2 ] != NULL )
				{
					hr = StringCbCopy( this->CustomText[ 2 ], len + sizeof( TCHAR ), FileText );
					if( FAILED( hr ))
					{
						return FALSE;
					}
				}
				else
					return FALSE;
			}
			else
				return FALSE;
		}
		if( Folderpath != NULL )
		{
			hr = StringCbLength( Folderpath, sizeof( TCHAR )* 4096, &len );
			if( SUCCEEDED( hr ))
			{
				this->CustomText[ 3 ] = new TCHAR[ len + sizeof( TCHAR ) ];

				if( this->CustomText[ 3 ] != NULL )
				{
					hr = StringCbCopy( this->CustomText[ 3 ], len + sizeof( TCHAR ), Folderpath );
					if( FAILED( hr ))
					{
						return FALSE;
					}
				}
				else
					return FALSE;
			}
			else
				return FALSE;
		}
		this->CustomizationRequested = TRUE;
	}
	return TRUE;
}

HRESULT Open_Save_CTRL::SetCustomText( IFileDialog* pfd )
{
	HRESULT hr = S_OK;

	if( this->CustomText[ 0 ] != NULL )
	{
		hr = pfd->SetTitle( this->CustomText[ 0 ] );
		if( FAILED( hr ))
		{
			return hr;
		}
	}
	if( this->CustomText[ 1 ] != NULL )
	{
		hr = pfd->SetOkButtonLabel( this->CustomText[ 1 ] );
		if( FAILED( hr ))
		{
			return hr;
		}
	}
	if( this->CustomText[ 2 ] != NULL )
	{
		hr = pfd->SetFileName( this->CustomText[ 2 ] );
		if( FAILED( hr ))
		{
			return hr;
		}
	}
	if( this->CustomText[ 3 ] != NULL )
	{
		IShellItem *pItem = nullptr;

		hr = SHCreateItemFromParsingName( this->CustomText[ 3 ], nullptr, IID_PPV_ARGS( &pItem ) );
		if( SUCCEEDED( hr ))
		{
			hr = pfd->SetFolder( pItem );
			if( FAILED( hr ))
			{
				return hr;
			}
		}
		else
			return hr;
	}
	return hr;
}