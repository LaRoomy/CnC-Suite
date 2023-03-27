#include"DataExchange.h"
#include"..\\BasicFPO.h"
#include"..\\HelperF.h"
#include"..\\Statusbar_dispatcher.h"
#include"..\\ApplicationData.h"
#include"..\\Error dispatcher.h"
#include"..\\Colors.h"
#include"..\\EditorContentManager.h"

SerialComm::SerialComm( HWND MainWnd, HINSTANCE hInstance, LPCTSTR Root_Directory)
	: ParentWindow( MainWnd ),
	hInst( hInstance ),
	Configuration(nullptr),
	threadInterruptCtrl(0),
	SRtext(nullptr),
	threadActive(FALSE),
	DataTrafficWnd(nullptr),
	ActivationStatus(TRUE),
	SENDBUFFERTOMAIN(FALSE),
	console(nullptr),
	deviceSelectionEvents(nullptr),
	DeviceSelectionWnd(nullptr),
	deviceSelectionListview(nullptr)
{
	this->Root_Dir = nullptr;
	CopyStringToPtr(Root_Directory, &this->Root_Dir);

	SecureZeroMemory(&this->IO_Info, sizeof(INOUTINFO));

	_MSG_TO_MAIN(WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&this->sInfo));

	this->Configuration = new SERIAL_CONFIG;
	this->_LoadControlsFromFile();
	this->DataTrafficWnd = NULL;
	this->TrafBackground = CreateSolidBrush(this->sInfo.Background);
	this->controlFont = CreateScaledFont(20, FW_MEDIUM, APPLICATION_PRIMARY_FONT);
	this->messageColors.info = RGB(230, 230, 230);

	if (this->sInfo.StyleID == STYLEID_LIGHTGRAY)
	{
		this->messageColors.error = COLOR_DARKRED;
		this->messageColors.warning = COLOR_DARKYELLOW;
		this->messageColors.ok = COLOR_DARKGREEN;
	}
	else
	{
		this->messageColors.error = COLOR_RED;
		this->messageColors.warning = COLOR_YELLOW;
		this->messageColors.ok = COLOR_GREEN;
	}
}

SerialComm::SerialComm()
{
	this->Configuration = new SERIAL_CONFIG;
	SecureZeroMemory(this->Configuration, sizeof(SERIAL_CONFIG));
	this->Configuration->Active_port = 1; // default for first use!!!

	// use ONLY the getter and setter method for the configuration when creating an instance with the default constructor
	// DO NOTHING ELSE!!!!
}

SerialComm::~SerialComm(void)
{
	DeleteObject( this->TrafBackground );
	DeleteObject(this->controlFont);

	UnregisterClass( CNCS_DATAX_CLASS, this->hInst );

	SafeDelete(&this->Configuration);
	SafeDeleteArray(&this->Root_Dir);
	SafeDeleteArray(&this->SRtext);
}

HRESULT SerialComm::_LoadControlsFromFile()
{
	HRESULT hr = S_OK;
	TCHAR *buffer = nullptr;

	auto bfpo = CreateBasicFPO();

	hr = (bfpo != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		hr = bfpo->LoadBufferFmFileAsUtf8(&buffer, this->Root_Dir)
			? S_OK : E_FAIL;

		int xonchar, xoffchar, errorchar, eofchar, eventchar;

		if (SUCCEEDED(hr))
		{
			swscanf_s(buffer,
				L"%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n%i\n",
				&this->Configuration->Abort_on_error,
				&this->Configuration->Active_port,
				&this->Configuration->baud_index,
				&this->Configuration->CTS_Flow,
				&this->Configuration->databit_index,
				&this->Configuration->DSR_Flow,
				&this->Configuration->DSR_Sense,
				&this->Configuration->DTR_control_index,
				&this->Configuration->ErrorChar_replace,
				&this->Configuration->Paritycheck,
				&this->Configuration->parity_index,
				&this->Configuration->RI_Timeout,
				&this->Configuration->RTS_control_index,
				&this->Configuration->RT_Timeout_cst,
				&this->Configuration->RT_Timeout_mpl,
				&this->Configuration->stopbit_index,
				&this->Configuration->WT_Timeout_cst,
				&this->Configuration->WT_Timeout_mpl,
				&this->Configuration->XON_XOFF,
				&xonchar,
				&xoffchar,
				&errorchar,
				&eofchar,
				&eventchar
			);
			this->Configuration->XonChar = static_cast<WCHAR>(xonchar);
			this->Configuration->XoffChar = static_cast<WCHAR>(xoffchar);
			this->Configuration->ErrorChar = static_cast<WCHAR>(errorchar);
			this->Configuration->EofChar = static_cast<WCHAR>(eofchar);
			this->Configuration->EventChar = static_cast<WCHAR>(eventchar);
		}
		else
		{
			this->SetDefault();
		}
		SafeDeleteArray(&buffer);
		SafeRelease(&bfpo);
	}
	return hr;
}

HRESULT SerialComm::_SaveControlsToFile(void)
{
	HRESULT hr = S_OK;
	TCHAR *buffer = new TCHAR[2048];

	hr = StringCbPrintf(	buffer, sizeof( WCHAR ) * 2048,
							L"%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\r\n%i\n",
							this->Configuration->Abort_on_error,
							this->Configuration->Active_port,
							this->Configuration->baud_index,
							this->Configuration->CTS_Flow,
							this->Configuration->databit_index,
							this->Configuration->DSR_Flow,
							this->Configuration->DSR_Sense,
							this->Configuration->DTR_control_index,
							this->Configuration->ErrorChar_replace,
							this->Configuration->Paritycheck,
							this->Configuration->parity_index,
							this->Configuration->RI_Timeout,
							this->Configuration->RTS_control_index,
							this->Configuration->RT_Timeout_cst,
							this->Configuration->RT_Timeout_mpl,
							this->Configuration->stopbit_index,
							this->Configuration->WT_Timeout_cst,
							this->Configuration->WT_Timeout_mpl,
							this->Configuration->XON_XOFF,
							static_cast<int>(this->Configuration->XonChar),
							static_cast<int>(this->Configuration->XoffChar),
							static_cast<int>(this->Configuration->ErrorChar),
							static_cast<int>(this->Configuration->EofChar),
							static_cast<int>(this->Configuration->EventChar)
	);

	if (SUCCEEDED(hr))
	{
		auto bfpo = CreateBasicFPO();

		hr = (bfpo != nullptr)
			? S_OK : E_FAIL;

		if (SUCCEEDED(hr))
		{
			bfpo->SaveBufferToFileAsUtf8(buffer, this->Root_Dir);

			SafeRelease(&bfpo);
		}
	}
	return hr;
}

void SerialComm::SetDefault(void)
{
	this->Configuration->Active_port = 1;
	this->Configuration->baud_index = 4;
	this->Configuration->parity_index = 2;
	this->Configuration->databit_index = 2;
	this->Configuration->stopbit_index = 0;
	this->Configuration->XON_XOFF = FALSE;
	this->Configuration->DTR_control_index = 0;
	this->Configuration->RTS_control_index = 0;
	this->Configuration->CTS_Flow = FALSE;
	this->Configuration->DSR_Flow = FALSE;
	this->Configuration->Abort_on_error = FALSE;
	this->Configuration->ErrorChar_replace = TRUE;
	this->Configuration->Paritycheck = TRUE;
	this->Configuration->DSR_Sense = FALSE;
	this->Configuration->XonChar = '0';
	this->Configuration->XoffChar = '0';
	this->Configuration->EofChar = '0';
	this->Configuration->ErrorChar = '0';
	this->Configuration->EventChar = '0';
	this->Configuration->RI_Timeout = UNDEF;
	this->Configuration->RT_Timeout_cst = UNDEF;
	this->Configuration->RT_Timeout_mpl = UNDEF;
	this->Configuration->WT_Timeout_cst = UNDEF;
	this->Configuration->WT_Timeout_mpl = UNDEF;
}

BOOL SerialComm::IsLF_Format()
{
	auto dataContainer =
		reinterpret_cast<ApplicationData*>(
			getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
			);
	if (dataContainer != nullptr)
	{
		auto formatIndex =
			dataContainer->getIntegerData(DATAKEY_EXSETTINGS_EXCHANGEWND_LINEENDFORMAT, 0);

		return (formatIndex == 2) ? TRUE : FALSE;
	}
	return false;
}

void SerialComm::drawWindow(HWND hWnd, HDC hdc)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	HDC offscreenDC = CreateCompatibleDC(hdc);
	if (offscreenDC)
	{
		HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right - (rc.right / 4), DPIScale(30));
		if (bmp)
		{
			auto origin = SelectObject(offscreenDC, bmp);

			RECT updateRect = { 0, 0, rc.right, DPIScale(30) };

			FillRect(offscreenDC, &updateRect, this->TrafBackground);

			iString outString(
				(this->IO_Info.mode == SEND_)
				? getStringFromResource(UI_GNRL_OUTPUT)
				: getStringFromResource(UI_GNRL_INPUT)
			);
			outString += L":  ";
			outString += this->IO_Info.processedDataSize;
			outString += L"bytes";

			SIZE tsize;
			int strLen = outString.GetLength();

			SelectObject(offscreenDC, this->controlFont);

			SetBkMode(offscreenDC, TRANSPARENT);
			SetTextColor(offscreenDC, this->sInfo.TextColor);

			GetTextExtentPoint32(
				offscreenDC,
				outString.GetData(),
				strLen,
				&tsize
			);
			TextOut(
				offscreenDC,
				DPIScale(20),
				(DPIScale(30) / 2) - (tsize.cy / 2),
				outString.GetData(),
				strLen
			);
			BitBlt(
				hdc,
				0, 0,
				rc.right - (rc.right / 4),
				DPIScale(30),
				offscreenDC,
				0, 0,
				SRCCOPY
			);
			SelectObject(offscreenDC, origin);
			DeleteObject(bmp);
		}
		DeleteDC(offscreenDC);
	}	
}

LRESULT SerialComm::OnDestroy()
{
	//this->threadActive = false;

	InterlockedExchange((LONG*)&this->threadActive, (LONG)FALSE);

	NormalizeStatusbarInfoAreaAsync(200);

	// not secure, especially the index of DATAEXCHANGECLASS
	PostMessage(
		this->ParentWindow,
		WM_CLEANUP,
		static_cast<WPARAM>( DATAEXCHANGECLASS ),
		reinterpret_cast<LPARAM>( this )
	);

	return static_cast<LRESULT>(0);
}

BOOL SerialComm::CheckForNumber( WCHAR L )
{
	BOOL result = FALSE;

	if( ( L == '0' ) ||
		( L == '1' ) ||
		( L == '2' ) ||
		( L == '3' ) ||
		( L == '4' ) ||
		( L == '5' ) ||
		( L == '6' ) ||
		( L == '7' ) ||
		( L == '8' ) ||
		( L == '9' )	)
	{
		result = TRUE;
	}
	return result;
}

HRESULT SerialComm::InitDataTransmission( int Mode, LPTSTR directBuffer, LPRECT pos)
{
	if( Mode == SEND_BUFFER )
	{
		if( directBuffer == NULL )
		{
			return E_FAIL;
		}
		else
		{
			size_t len;
			HRESULT hr;

			_dispatchTextToStatusbar(
				0,
				getStringFromResource(UI_STATUSBAR_SENDINPROGRESS)
			);

			hr = StringCbLength( directBuffer, sizeof( TCHAR )* STRSAFE_MAX_CCH, &len );
			if( SUCCEEDED( hr ))
			{
				this->IO_Info.totalDataSize = (int)(len / sizeof(TCHAR));
				this->IO_Info.processedDataSize = 0;

				this->SRtext = new WCHAR[ (len + ( 10* sizeof( WCHAR ))) ];

				if( this->SRtext != NULL )
				{
					hr = StringCbCopy( this->SRtext, (len + ( 10* sizeof( WCHAR ))), directBuffer );
					if( SUCCEEDED( hr ))
					{
						hr = StringCbCat( this->SRtext, (len + ( 10 * sizeof( WCHAR ))), L"\n \n\0" );	// what was the purpose of that ?!
						if( FAILED( hr ))
						{
							return hr;
						}
					}
					else
						return E_FAIL;
				}
				else
					return E_FAIL;
			}
			else
				return E_FAIL;

		}
		Mode = SEND_;
	}
	else if( Mode == RECIEVE_BUFFER )
	{
		_dispatchTextToStatusbar(
			0,
			getStringFromResource(UI_STATUSBAR_RECEIVEINPROGRESS)
		);

		this->SENDBUFFERTOMAIN = TRUE;

		this->IO_Info.processedDataSize = 0;
		this->IO_Info.totalDataSize = -1;// unknown!

		Mode = RECEIVE_;
	}
	else
		return E_FAIL;

	this->IO_Info.mode = Mode;

	POINT position;
	SIZE size;
	position.x = pos->left;
	position.y = pos->top;
	size.cx = pos->right - pos->left;
	size.cy = pos->bottom - pos->top;

	HRESULT hr = this->InitDataExchangeWindowEx(&position, &size);
	if (SUCCEEDED(hr))
	{
		hr = this->startTransmissionThread(Mode)
			? S_OK : E_FAIL;

		if (SUCCEEDED(hr))
		{
			// ...
		}
	}
	return hr;
}

int SerialComm::PrepareBufferForTransmission(LPPTSTR buffer, bool removeBracketAnnotations, bool removeApostrophAnnotations, bool removeSpaces)
{
	int retVal = -1;

	if ((*buffer != nullptr) && (buffer != nullptr))
	{
		auto oldBufferSize = _lengthOfString(*buffer);

		if (oldBufferSize > 0)
		{
			DWORD newbufferSize = 0;
			int idx = 0;
			bool blocker = false;
			TCHAR bracketType = L'\0';

			// at first elaborate the size of the new buffer

			while (idx < oldBufferSize)
			{
				if ((*buffer)[idx] == L'\0')
					break;

				//overrun the annotations introduced with ';'  (if requested)
				if ((((*buffer)[idx] == L';')) && removeApostrophAnnotations)
				{
					while (((*buffer)[idx] != 0x0D) && ((*buffer)[idx] != L'\n') && ((*buffer)[idx] != L'\0'))
					{
						idx++;
					}

					if ((*buffer)[idx] == L'\0')
						break;
				}

				if (!blocker)
				{
					// if this is a bracket, activate the blocker  (if requested)
					if ((((*buffer)[idx] == L'{') || ((*buffer)[idx] == L'(')) && removeBracketAnnotations)
					{
						bracketType = (*buffer)[idx];
						blocker = true;
					}
					else
					{
						// only count if spaces are allowed and this is no space
						if (!(removeSpaces && ((*buffer)[idx] == L' ')))
						{
							newbufferSize++;
						}
					}
				}
				else
				{
					// deactivate the blocker if the appropriate bracket is found
					if ((((*buffer)[idx] == L'}') && (bracketType == L'{'))
						|| (((*buffer)[idx] == L')') && (bracketType == L'(')))
					{
						blocker = false;
						bracketType = L'\0';
					}
				}
				idx++;
			}
			newbufferSize++;

			// save the old buffer and reallocate the buffer with the new size
			TCHAR* oldBuff = nullptr;
			
			if (CopyStringToPtr(*buffer, &oldBuff) == TRUE)
			{
				SafeDeleteArray(buffer);

				*buffer = new TCHAR[newbufferSize];
				if (*buffer != nullptr)
				{
					idx = 0;
					blocker = false;
					bracketType = L'\0';
					DWORD transferIndex = 0;

					// store the converted text in the new buffer
					while (idx < oldBufferSize)
					{
						if (oldBuff[idx] == L'\0')
							break;

						//overrun the annotations introduced with ';' (if requested)
						if (((oldBuff[idx] == L';')) && removeApostrophAnnotations)
						{
							while ((oldBuff[idx] != 0x0D) && (oldBuff[idx] != L'\n') && (oldBuff[idx] != L'\0'))
							{
								idx++;
							}

							if (oldBuff[idx] == L'\0')
								break;
						}

						if (!blocker)
						{
							// if this is a bracket, activate the blocker (if requested)
							if (((oldBuff[idx] == L'{') || (oldBuff[idx] == L'(')) && removeBracketAnnotations)
							{
								bracketType = oldBuff[idx];
								blocker = true;
							}
							else
							{
								// only transfer if spaces are allowed and this is no space
								if (!(removeSpaces && (oldBuff[idx] == L' ')))
								{
									(*buffer)[transferIndex] = oldBuff[idx];
									transferIndex++;
								}
							}
						}
						else
						{
							// deactivate the blocker if the appropriate bracket is found
							if (((oldBuff[idx] == L'}') && (bracketType == L'{'))
								|| ((oldBuff[idx] == L')') && (bracketType == L'(')))
							{
								blocker = false;
								bracketType = L'\0';
							}
						}
						idx++;
					}
					(*buffer)[transferIndex] = L'\0';

					retVal = newbufferSize;
				}
				SafeDeleteArray(&oldBuff);
			}
		}
	}
	return retVal;
}

HRESULT SerialComm::setConfiguration(const TCHAR* path, PSERIAL_CONFIG config_in)
{
	HRESULT hr;

	if (path == nullptr)
	{
		hr = (this->Root_Dir != nullptr) ? S_OK : E_FAIL;
	}
	else
	{
		SafeDeleteArray(&this->Root_Dir);

		hr = (CopyStringToPtr(path, &this->Root_Dir) == TRUE)
			? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		this->Configuration->Abort_on_error = config_in->Abort_on_error;
		this->Configuration->Active_port = config_in->Active_port;
		this->Configuration->baud_index = config_in->baud_index;
		this->Configuration->CTS_Flow = config_in->CTS_Flow;
		this->Configuration->databit_index = config_in->databit_index;
		this->Configuration->DSR_Flow = config_in->DSR_Flow;
		this->Configuration->DSR_Sense = config_in->DSR_Sense;
		this->Configuration->DTR_control_index = config_in->DTR_control_index;
		this->Configuration->EofChar = config_in->EofChar;
		this->Configuration->ErrorChar = config_in->ErrorChar;
		this->Configuration->ErrorChar_replace = config_in->ErrorChar_replace;
		this->Configuration->EventChar = config_in->EventChar;
		this->Configuration->Paritycheck = config_in->Paritycheck;
		this->Configuration->parity_index = config_in->parity_index;
		this->Configuration->RI_Timeout = config_in->RI_Timeout;
		this->Configuration->RTS_control_index = config_in->RTS_control_index;
		this->Configuration->RT_Timeout_cst = config_in->RT_Timeout_cst;
		this->Configuration->RT_Timeout_mpl = config_in->RT_Timeout_mpl;
		this->Configuration->stopbit_index = config_in->stopbit_index;
		this->Configuration->WT_Timeout_cst = config_in->WT_Timeout_cst;
		this->Configuration->WT_Timeout_mpl = config_in->WT_Timeout_mpl;
		this->Configuration->XoffChar = config_in->XoffChar;
		this->Configuration->XonChar = config_in->XonChar;
		this->Configuration->XON_XOFF = config_in->XON_XOFF;

		return this->_SaveControlsToFile();
	}
	return hr;
}

HRESULT SerialComm::getConfiguration(const TCHAR* path, PSERIAL_CONFIG config_out)
{
	HRESULT hr;

	hr = (path != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		hr = (CopyStringToPtr(path, &this->Root_Dir) == TRUE)
			? S_OK : E_FAIL;

		if (SUCCEEDED(hr))
		{
			hr = this->_LoadControlsFromFile();
			if (SUCCEEDED(hr))
			{
				config_out->Abort_on_error = this->Configuration->Abort_on_error;
				config_out->Active_port = this->Configuration->Active_port;
				config_out->baud_index = this->Configuration->baud_index;
				config_out->CTS_Flow = this->Configuration->CTS_Flow;
				config_out->databit_index = this->Configuration->databit_index;
				config_out->DSR_Flow = this->Configuration->DSR_Flow;
				config_out->DSR_Sense = this->Configuration->DSR_Sense;
				config_out->DTR_control_index = this->Configuration->DTR_control_index;
				config_out->EofChar = this->Configuration->EofChar;
				config_out->ErrorChar = this->Configuration->ErrorChar;
				config_out->ErrorChar_replace = this->Configuration->ErrorChar_replace;
				config_out->EventChar = this->Configuration->EventChar;
				config_out->Paritycheck = this->Configuration->Paritycheck;
				config_out->parity_index = this->Configuration->parity_index;
				config_out->RI_Timeout = this->Configuration->RI_Timeout;
				config_out->RTS_control_index = this->Configuration->RTS_control_index;
				config_out->RT_Timeout_cst = this->Configuration->RT_Timeout_cst;
				config_out->RT_Timeout_mpl = this->Configuration->RT_Timeout_mpl;
				config_out->stopbit_index = this->Configuration->stopbit_index;
				config_out->WT_Timeout_cst = this->Configuration->WT_Timeout_cst;
				config_out->WT_Timeout_mpl = this->Configuration->WT_Timeout_mpl;
				config_out->XoffChar = this->Configuration->XoffChar;
				config_out->XonChar = this->Configuration->XonChar;
				config_out->XON_XOFF = this->Configuration->XON_XOFF;
			}
		}
	}
	return hr;
}

HRESULT SerialComm::setDefaultConfiguration(const TCHAR * path)
{
	if (path != nullptr)
	{
		SERIAL_CONFIG config;
		SecureZeroMemory(&config, sizeof(SERIAL_CONFIG));

		config.baud_index = 4;
		config.databit_index = 2;
		config.parity_index = 2;
		config.stopbit_index = 0;
		config.Active_port = 1;

		config.EofChar = L'0';
		config.ErrorChar = L'0';
		config.EventChar = L'0';
		config.XoffChar = L'0';
		config.XonChar = L'0';

		config.Paritycheck = TRUE;
		config.ErrorChar_replace = TRUE;

		config.RI_Timeout = 50;
		config.RT_Timeout_mpl = 2000;
		config.RT_Timeout_cst = 0;
		config.WT_Timeout_mpl = 12000;
		config.WT_Timeout_cst = -1;

		return this->setConfiguration(path, &config);
	}
	return E_POINTER;
}

HRESULT SerialComm::CreateSerialDeviceListing(itemCollection<iString>* deviceListing)
{
	auto hr = (deviceListing != nullptr) ? S_OK : E_POINTER;
	if (SUCCEEDED(hr))
	{
		deviceListing->Clear();

		// Initialize reference to device information set with the serial-device-GUID
		HDEVINFO devInfo =
			SetupDiGetClassDevs(&GUID_CLASS_COMPORT, nullptr, nullptr, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);// do not use present!?

		hr = (devInfo != INVALID_HANDLE_VALUE) ? S_OK : E_HANDLE;
		if(SUCCEEDED(hr))
		{
			DWORD devIndex = 0;
			DWORD reqSize = 0;

			SP_DEVINFO_DATA devInfoData;
			SecureZeroMemory(&devInfoData, sizeof(SP_DEVINFO_DATA));
			devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

			// start enumeration:
			hr =
				SetupDiEnumDeviceInfo(devInfo, devIndex, &devInfoData)
				? S_OK : E_FAIL;

			if (SUCCEEDED(hr))
			{
				do
				{
					// at first retrieve the required buffersize
					// NOTE: ignore the return value, because the function will fail and GetLastError() returns ERROR_INSUFFICIENT_BUFFER
					SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_FRIENDLYNAME, nullptr, nullptr, 0, &reqSize);

					hr = (reqSize == 0) ? E_FAIL : S_OK;
					if (SUCCEEDED(hr))
					{
						auto pBuffer = new BYTE[reqSize + 1];

						hr = (pBuffer != nullptr) ? S_OK : E_OUTOFMEMORY;
						if (SUCCEEDED(hr))
						{
							// retrieve the friendlyname with the second call
							hr =
								SetupDiGetDeviceRegistryProperty(devInfo, &devInfoData, SPDRP_FRIENDLYNAME, nullptr, pBuffer, reqSize + 1, nullptr)
								? S_OK : E_FAIL;

							if (SUCCEEDED(hr))
							{
								iString friendlyName((TCHAR*)pBuffer);

								deviceListing->AddItem(friendlyName);
							}
							SafeDeleteArray(&pBuffer);
						}
						devIndex++;

						// enumerate next device
						hr =
							SetupDiEnumDeviceInfo(devInfo, devIndex, &devInfoData)
							? S_OK : E_FAIL;
					}

				} while (SUCCEEDED(hr));

				if (FAILED(hr))
				{
					// SetupDiEnumDeviceInfo() will return FALSE if there are no more device-items in the device-info element
					// >> actually, this is not an error so set S_OK if GetLastError() returns ERROR_NO_MORE_ITEMS
					if (GetLastError() == ERROR_NO_MORE_ITEMS)
						hr = S_OK;
				}
			}
			else
			{
				// there are no serial devices in the system registered
				hr = E_NOINTERFACE;
			}
			SetupDiDestroyDeviceInfoList(devInfo);
		}
	}
	return hr;
}

HRESULT SerialComm::CreateDeviceListingWindow(LPPOINT pos, LPSIZE size, IDeviceSelectionProtocol * eventHandler)
{
	auto hr =
		((pos == nullptr) || (size == nullptr) || (this->ParentWindow == nullptr))
		? E_INVALIDARG : S_OK;

	if (SUCCEEDED(hr))
	{
		this->deviceSelectionEvents = eventHandler;

		hr = this->registerDeviceSelectionWindowClass();
		if (SUCCEEDED(hr))
		{
			this->DeviceSelectionWnd =
				CreateWindow(
					CNCS_DEVSEL_CLASS,
					nullptr,
					WS_CHILD | WS_CLIPSIBLINGS,
					pos->x,
					pos->y,
					size->cx,
					size->cy,
					this->ParentWindow,
					(HMENU)CTRLID_DEVICESELECTIONWINDOW,
					this->hInst,
					reinterpret_cast<LPVOID>(this)
				);

			hr =
				(this->DeviceSelectionWnd != nullptr)
				? S_OK : E_HANDLE;

			if (SUCCEEDED(hr))
			{
				hr = this->createDeviceSelectionWindowContent();
				if (SUCCEEDED(hr))
				{
					ShowWindow(this->DeviceSelectionWnd, SW_SHOW);
					UpdateWindow(this->DeviceSelectionWnd);
				}
			}
		}
	}
	return hr;
}

void SerialComm::DestroyDeviceListingWindow()
{
	DestroyWindow(this->DeviceSelectionWnd);
}

int SerialComm::ExtractComPortFromDeviceName(iString & deviceName)
{
	CHARSCOPE cs;
	
	if (deviceName.Contains(L"COM", &cs))
	{
		auto _char_ = deviceName.getCharAt(cs.endChar + 1);
		WCHAR str[2] = { L'\0' };
		str[0] = _char_;
		auto port = _wtoi(str);
		return port;
	}
	return -1;
}

void SerialComm::onCustomButtonClick(cObject sender, CTRLID ID_button)
{
	UNREFERENCED_PARAMETER(sender);

	switch (ID_button)
	{
	case CTRLID_BACKBUTTON:
		if (this->deviceSelectionEvents != nullptr)
		{
			this->deviceSelectionEvents->OnQuit(reinterpret_cast<cObject>(this));
		}
		break;
	case CTRLID_SELECTDEVICEBUTTON:
		if (this->deviceSelectionEvents != nullptr)
		{
			auto rIndex = this->deviceSelectionListview->getSelectedRowIndex();
			if (rIndex >= 0)
			{
				auto item = this->deviceSelectionListview->getItemAtIndex(rIndex);
				if (item.getRowCount() > 0)
				{
					auto rName = item.getDataInRow(0);

					this->deviceSelectionEvents->OnDeviceSelected(
						reinterpret_cast<cObject>(this),
						rName
					);
				}
				this->deviceSelectionEvents->OnQuit(reinterpret_cast<cObject>(this));
			}
			else
			{
				DispatchEWINotification(
					EDSP_INFO,
					L"STP002",
					getStringFromResource(UI_PROPWND_NODEVICESELECTED),
					getStringFromResource(UI_APPPROPERTIES)
				);
			}
		}
		break;
	case D_TRAFFIC_CANCEL:
		this->onCancelTransmission(this->DataTrafficWnd);
		break;
	case D_TRAFFIC_FINISH:
		this->onFinishTransmission(this->DataTrafficWnd);
		break;
	default:
		break;
	}
}

LRESULT SerialComm::DataExchangeWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SerialComm* serialCom = nullptr;

	if (message == WM_CREATE)
	{
		auto pCr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		if (pCr != nullptr)
		{
			serialCom = reinterpret_cast<SerialComm*>(pCr->lpCreateParams);
			if (serialCom != nullptr)
			{
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(serialCom));
			}
		}
		return 1;
	}
	else
	{
		serialCom = reinterpret_cast<SerialComm*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (serialCom != nullptr)
		{
			switch (message)
			{
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_PAINT:
				return serialCom->onPaintInTrafficWindow(hWnd);
			case WM_INTERRUPT:
				return serialCom->OnInterrupt(hWnd, wParam);
			case WM_SIZE:
				return serialCom->onSizeInTrafficWindow();
			case WM_CLOSE:
				return serialCom->OnClose(hWnd);
			case WM_DESTROY:
				return serialCom->OnDestroy();
			default:
				break;
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

LRESULT SerialComm::DeviceSelectionWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SerialComm* serialCom = nullptr;

	if (message == WM_CREATE)
	{
		auto pCr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		if (pCr != nullptr)
		{
			serialCom = reinterpret_cast<SerialComm*>(pCr->lpCreateParams);
			if (serialCom != nullptr)
			{
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(serialCom));
			}
		}
		return 1;
	}
	else
	{
		serialCom = reinterpret_cast<SerialComm*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (serialCom != nullptr)
		{
			switch (message)
			{
			case WM_PAINT:
				return serialCom->onPaintInDeviceSelectionWindow(hWnd);
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_DESTROY:
				SafeRelease(&serialCom);
				return 0;
			default:
				break;
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

HRESULT SerialComm::registerDataExchangeWindowClass()
{
	HRESULT hr = S_OK;
	WNDCLASSEX wcex;

	if (GetClassInfoEx(this->hInst, CNCS_DATAX_CLASS, &wcex) == 0)
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = SerialComm::DataExchangeWindowProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = this->hInst;
		wcex.hIcon = nullptr;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = this->TrafBackground;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = CNCS_DATAX_CLASS;
		wcex.hIconSm = nullptr;

		hr = (RegisterClassEx(&wcex) != 0) ? S_OK : E_FAIL;
	}
	return hr;
}

HRESULT SerialComm::registerDeviceSelectionWindowClass()
{
	HRESULT hr = S_OK;
	WNDCLASSEX wcex;

	if (GetClassInfoEx(this->hInst, CNCS_DEVSEL_CLASS, &wcex) == 0)
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = SerialComm::DeviceSelectionWindowProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = this->hInst;
		wcex.hIcon = nullptr;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = nullptr;// CreateSolidBrush(RGB(170, 170, 170));// universal color for all styles!?
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = CNCS_DEVSEL_CLASS;
		wcex.hIconSm = nullptr;

		hr = (RegisterClassEx(&wcex) != 0) ? S_OK : E_FAIL;
	}
	return hr;
}

HRESULT SerialComm::InitDataExchangeWindowEx(LPPOINT position, LPSIZE size)
{
	auto hr = this->registerDataExchangeWindowClass();
	if (SUCCEEDED(hr))
	{
		POINT pos;
		SIZE sz;

		if (position != nullptr)
		{
			this->anchorPosition = *position;
			pos = *position;
		}
		else
		{
			this->anchorPosition = { 0,0 };
			pos = { 0,0 };
		}
		if (size != nullptr)
		{
			this->anchorSize = *size;
			sz = *size;
		}
		else
		{
			this->anchorSize = { 400, 300 };
			sz = { 400, 300 };
		}

		auto dataContainer =
			reinterpret_cast<ApplicationData*>(
				getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
				);

		if (dataContainer != nullptr)
		{
			DWORD additionalStyle = WS_BORDER;

			auto monitorTM = dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXCHANGEWND_MONITORTRANSMISSION, true);
			if (monitorTM)
			{
				auto anchorWnd = dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXCHANGEWND_ANCHORWND, true);
				if (!anchorWnd)
				{
					auto cx = GetSystemMetrics(SM_CXSCREEN);
					auto cy = GetSystemMetrics(SM_CYSCREEN);

					if (cx > 600)
						sz.cx = 600;
					else
						sz.cx = cx;

					if (cy > 500)
						sz.cy = 500;
					else
						sz.cy = cy;

					pos.x = (cx - sz.cx) / 2;
					pos.y = (cy - sz.cy) / 2;

					additionalStyle |= (WS_CAPTION | WS_THICKFRAME);
				}
			}

			this->DataTrafficWnd
				= CreateWindowEx(
					0,
					CNCS_DATAX_CLASS,
					nullptr,
					WS_POPUP | WS_CLIPCHILDREN | additionalStyle,
					pos.x,
					pos.y,
					sz.cx,
					sz.cy,
					this->ParentWindow,
					nullptr,
					this->hInst,
					reinterpret_cast<LPVOID>(this)
				);

			hr = (this->DataTrafficWnd != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				hr = this->InitDataExchangeWindowContentEx();
				if (SUCCEEDED(hr))
				{
					ShowWindow(this->DataTrafficWnd, SW_SHOW);
					UpdateWindow(this->DataTrafficWnd);
				}
			}
		}
	}
	return hr;
}

HRESULT SerialComm::InitDataExchangeWindowContentEx()
{
	HRESULT hr;
	RECT rc;
	GetClientRect(this->DataTrafficWnd, &rc);

	POINT pos;
	SIZE sz;
	pos.y = 0;
	pos.x = rc.right - (rc.right / 4);
	sz.cx = (rc.right / 4);
	sz.cy = DPIScale(30);

	auto cancelbutton = new CustomButton(this->DataTrafficWnd, BUTTONMODE_TEXT, &pos, &sz, D_TRAFFIC_CANCEL, this->hInst);
	hr =
		(cancelbutton != nullptr)
		? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		iString buttonText(
			getStringFromResource(UI_GNRL_CANCEL)
		);

		cancelbutton->setAppearance_onlyText(buttonText, FALSE);
		cancelbutton->setBorder(TRUE, this->sInfo.OutlineColor);
		cancelbutton->setColors(this->sInfo.mainToolbarColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
		cancelbutton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
		cancelbutton->setTextColor(this->sInfo.TextColor);
		cancelbutton->setTextHighlight(TRUE, RGB(255, 255, 255));

		hr = cancelbutton->Create();
		if (SUCCEEDED(hr))
		{
			if (this->IO_Info.mode == RECEIVE_)
			{
				pos.x = rc.right / 2;

				buttonText.Replace(
					getStringFromResource(UI_DATAEXCHANGE_FINALIZE)
				);

				auto finalizeButton = new CustomButton(this->DataTrafficWnd, BUTTONMODE_ICONTEXT, &pos, &sz, D_TRAFFIC_FINISH, this->hInst);
				hr =
					(finalizeButton != nullptr)
					? S_OK : E_FAIL;
				if (SUCCEEDED(hr))
				{
					finalizeButton->setAppearance_IconText(IDI_GNRL_ARROW_UP, DPIScale(24), buttonText);
					finalizeButton->setBorder(TRUE, this->sInfo.OutlineColor);
					finalizeButton->setColors(this->sInfo.mainToolbarColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
					finalizeButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
					finalizeButton->setTextColor(this->sInfo.TextColor);
					finalizeButton->setTextHighlight(TRUE, RGB(255, 255, 255));

					hr = finalizeButton->Create();
				}
			}
			if (SUCCEEDED(hr))
			{
				if (reinterpret_cast<ApplicationData*>(
					getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
					)->getBooleanData(DATAKEY_EXSETTINGS_EXCHANGEWND_MONITORTRANSMISSION, true)
					)
				{
					this->console = new CSConsole(this->hInst);
					hr = (this->console != nullptr)
						? S_OK : E_FAIL;

					if (SUCCEEDED(hr))
					{
						POINT cPos = { 0,DPIScale(30) };
						SIZE cSize = {
							rc.right,
							rc.bottom - DPIScale(30)
						};

						this->console->SetConsoleType(CSConsole::CONSOLETYPE_MULTICOLOR | CSConsole::CONSOLETYPE_SCROLLABLE);
						this->console->SetLineHeight(DPIScale(20));
						this->console->SetCaretWidth(DPIScale(12));
						this->console->SetAdditonalWindowStyles(WS_BORDER);
						this->console->SetColors(this->sInfo.TabColor, this->sInfo.TextColor);

						this->console->SetFontProperty(
							DPIScale(16),
							FW_BOLD,
							L"Consolas"
						);

						hr = this->console->Create(
							this->DataTrafficWnd,
							&cPos, &cSize,
							CTRLID_OUTPUT_CONSOLE
						);

						if (SUCCEEDED(hr))
						{
							// ...	
						}
					}
				}
			}
		}		
	}
	return hr;
}

HRESULT SerialComm::createDeviceSelectionWindowContent()
{
	auto hr = (this->DeviceSelectionWnd != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		RECT rc;
		GetClientRect(this->DeviceSelectionWnd, &rc);

		POINT pos;
		SIZE sz;
		pos.x = rc.right - 162;
		pos.y = rc.bottom - DPIScale(40);
		sz.cx = 160;
		sz.cy = 26;

		iString buttontext(
			getStringFromResource(UI_PROPWND_SELECTDEVICE)
		);

		auto selectButton = new CustomButton(this->DeviceSelectionWnd, BUTTONMODE_TEXT, &pos, &sz, CTRLID_SELECTDEVICEBUTTON , this->hInst);
		hr = (selectButton != nullptr)
			? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			selectButton->setAppearance_onlyText(buttontext, FALSE);
			selectButton->setBorder(TRUE, this->sInfo.OutlineColor);
			selectButton->setColors(this->sInfo.mainToolbarColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
			selectButton->setTextColor(this->sInfo.TextColor);
			selectButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
			selectButton->setFont(
				CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
			);
			if (this->sInfo.StyleID == STYLEID_LIGHTGRAY)
			{
				selectButton->setTextHighlight(TRUE, RGB(255, 255, 255));
			}

			hr = selectButton->Create();
			if (SUCCEEDED(hr))
			{
				pos.x = DPIScale(28);
				buttontext.Replace(
					getStringFromResource(UI_GNRL_NAVIGATEBACK)
				);

				auto backButton = new CustomButton(this->DeviceSelectionWnd, BUTTONMODE_TEXT, &pos, &sz, CTRLID_BACKBUTTON, this->hInst);

				hr = (selectButton != nullptr)
					? S_OK : E_FAIL;
				if (SUCCEEDED(hr))
				{
					backButton->setAppearance_onlyText(buttontext, FALSE);
					backButton->setBorder(TRUE, this->sInfo.OutlineColor);
					backButton->setColors(this->sInfo.mainToolbarColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
					backButton->setTextColor(this->sInfo.TextColor);
					backButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
					backButton->setFont(
						CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
					);
					if (this->sInfo.StyleID == STYLEID_LIGHTGRAY)
					{
						backButton->setTextHighlight(TRUE, RGB(255, 255, 255));
					}

					hr = backButton->Create();
					if (SUCCEEDED(hr))
					{
						this->deviceSelectionListview = new cListView(this->hInst);

						hr = (this->deviceSelectionListview != nullptr) ? S_OK : E_FAIL;
						if (SUCCEEDED(hr))
						{
							CTRLCREATIONSTRUCT ccs;
							ccs.ctrlID = CTRLID_DEVICESELECTIONLISTVIEW;
							ccs.parent = this->DeviceSelectionWnd;
							ccs.pos.x = DPIScale(28);
							ccs.pos.y = DPIScale(40);
							ccs.size.cx = rc.right - DPIScale(28);
							ccs.size.cy = rc.bottom - DPIScale(100);

							itemCollection<iString> columns;
							columns.AddItem(L"Device Name");

							itemCollection<int> columnWidths;
							columnWidths.AddItem(rc.right - DPIScale(30));

							this->deviceSelectionListview->setColors(this->sInfo.mainToolbarColor, this->sInfo.TextColor);
							this->deviceSelectionListview->setEventHandler(dynamic_cast<listViewEventSink*>(this));
							this->deviceSelectionListview->setFont(
								CreateScaledFont(20, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
							);
							this->deviceSelectionListview->setColumnDefinitions(1, &columns, &columnWidths, LVCFMT_CENTER);
							this->deviceSelectionListview->setAdditonalListViewStyles(
								LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER,
								LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT
							);

							this->deviceSelectionListview->setAdditonalHostWindowStyles(WS_BORDER);

							hr = this->deviceSelectionListview->Create(&ccs);
							if (SUCCEEDED(hr))
							{
								itemCollection<iString> devices;

								hr = SerialComm::CreateSerialDeviceListing(&devices);
								if (SUCCEEDED(hr))
								{
									listViewItem testitem(1);

									for (int i = 0; i < devices.GetCount(); i++)
									{
										auto dev = devices.GetAt(i);
										testitem.setDataInRow(0, dev);
										this->deviceSelectionListview->addItem(testitem);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return hr;
}

LRESULT SerialComm::onSizeInTrafficWindow()
{
	RECT rc;
	GetClientRect(this->DataTrafficWnd, &rc);

	if (this->console != nullptr)
	{
		this->console->Move(
			0,
			DPIScale(30),
			rc.right,
			rc.bottom - DPIScale(30)
		);

		auto cancelButton = GetDlgItem(this->DataTrafficWnd, D_TRAFFIC_CANCEL);
		if (cancelButton != nullptr)
		{
			MoveWindow(cancelButton, rc.right - (rc.right / 4), 0, rc.right / 4, DPIScale(30), TRUE);
		}

		auto finalizeButton = GetDlgItem(this->DataTrafficWnd, D_TRAFFIC_FINISH);
		if (finalizeButton != nullptr)
		{
			MoveWindow(finalizeButton, rc.right - (rc.right / 2), 0, rc.right / 4, DPIScale(30), TRUE);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT SerialComm::onPaintInTrafficWindow(HWND hWnd)
{
	HDC hdc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(hWnd, &ps);
	if (hdc)
	{
		this->drawWindow(hWnd, hdc);

		EndPaint(hWnd, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT SerialComm::onPaintInDeviceSelectionWindow(HWND hWnd)
{
	HDC hdc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(hWnd, &ps);
	if (hdc)
	{
		RECT rc;
		GetClientRect(hWnd, &rc);

		HBRUSH brush = CreateSolidBrush(this->sInfo.TabColor);
		if (brush)
		{
			FillRect(hdc, &rc, brush);

			auto font = CreateScaledFont(20, FW_SEMIBOLD, APPLICATION_PRIMARY_FONT);
			if (font)
			{
				SIZE sz;
				auto string_ = getStringFromResource(UI_PROPWND_FOUNDDEVICES);
				int strLen = _lengthOfString(string_);

				auto originFont = SelectObject(hdc, font);

				SetBkMode(hdc, TRANSPARENT);
				SetTextColor(hdc, this->sInfo.TextColor);

				GetTextExtentPoint32(hdc, string_, strLen, &sz);

				TextOut(
					hdc,
					((rc.right / 2) - (sz.cx / 2)) + DPIScale(28),
					DPIScale(15),
					string_,
					strLen
				);

				SelectObject(hdc, originFont);
				DeleteObject(font);
			}
			DeleteObject(brush);
		}
		EndPaint(hWnd, &ps);
	}
	return static_cast<LRESULT>(0);
}

HRESULT SerialComm::serialInit( LPDCB dcb, LPCOMMTIMEOUTS timeouts, LPOVERLAPPED ovl )
{
	// general:
	dcb->BaudRate = _baud[ this->Configuration->baud_index ];
	dcb->ByteSize = (BYTE)this->Configuration->databit_index + 5;
	dcb->StopBits = stoppbts[ this->Configuration->stopbit_index ];
	dcb->Parity = parity[ this->Configuration->parity_index ];

	dcb->fBinary = TRUE;

	dcb->fParity = this->Configuration->Paritycheck;
	dcb->fAbortOnError = this->Configuration->Abort_on_error;
	dcb->fErrorChar = this->Configuration->ErrorChar_replace;

	// characters:
	dcb->EofChar = (char)this->Configuration->EofChar;
	dcb->ErrorChar = (char)this->Configuration->ErrorChar;
	dcb->EvtChar = (char)this->Configuration->EventChar;
	dcb->XonChar = (char)this->Configuration->XonChar;
	dcb->XoffChar = (char)this->Configuration->XoffChar;

	// handshake:
	// if Configuration->XON_XOFF == 0 -> no handshake
	// if Configuration->XON_XOFF == 1 -> software handshake
	// if Configuration->XON_XOFF == 2 -> hardware handshake

	if( this->Configuration->XON_XOFF == 1 )
	{
		dcb->fInX = TRUE;
		dcb->fOutX = TRUE;

		dcb->XonLim = 50;
		dcb->XoffLim = 100;

		dcb->fDsrSensitivity = FALSE;
		dcb->fOutxCtsFlow = FALSE;
		dcb->fOutxDsrFlow = FALSE;
		
		dcb->fDtrControl = DTR_CONTROL_DISABLE;
		dcb->fRtsControl = RTS_CONTROL_DISABLE;

		dcb->fTXContinueOnXoff = TRUE;
	}
	else if (this->Configuration->XON_XOFF == 2)
	{
		dcb->fInX = FALSE;
		dcb->fOutX = FALSE;

		dcb->XonLim = 50;
		dcb->XoffLim = 100;

		dcb->fDsrSensitivity = TRUE;
		dcb->fOutxCtsFlow = TRUE;
		dcb->fOutxDsrFlow = TRUE;
		
		dcb->fDtrControl = this->Configuration->DTR_control_index;
		dcb->fRtsControl = this->Configuration->RTS_control_index;

		dcb->fTXContinueOnXoff = TRUE;
	}
	else
	{
		dcb->fInX = FALSE;
		dcb->fOutX = FALSE;

		dcb->fOutxCtsFlow = FALSE;
		dcb->fOutxDsrFlow = FALSE;
		dcb->fDsrSensitivity = FALSE;
		dcb->fAbortOnError = FALSE;
		dcb->fDtrControl = DTR_CONTROL_ENABLE;
		dcb->fRtsControl = RTS_CONTROL_ENABLE;
	}

	// init overlapped structure
	ovl->Internal = 0;
	ovl->InternalHigh = 0;
	ovl->Offset = 0;
	ovl->OffsetHigh = 0;

	// set timeouts
	timeouts->ReadIntervalTimeout = this->Configuration->RI_Timeout;
	timeouts->ReadTotalTimeoutMultiplier = this->Configuration->RT_Timeout_mpl;
	timeouts->ReadTotalTimeoutConstant = this->Configuration->RT_Timeout_cst;

	DWORD wttm = 12000;
	if ((dcb->BaudRate + 1) != 0)// prevent division by zero
		wttm = this->Configuration->WT_Timeout_mpl / (dcb->BaudRate + 1);

	timeouts->WriteTotalTimeoutMultiplier = (wttm);

	if( this->Configuration->WT_Timeout_cst < 2 )
	{
		timeouts->WriteTotalTimeoutConstant = timeouts->WriteTotalTimeoutMultiplier + 1;
	}
	else
	{
		timeouts->WriteTotalTimeoutConstant = this->Configuration->WT_Timeout_cst;
	}
	return S_OK;
}

BOOL SerialComm::startTransmissionThread(int Mode)
{
	if (this->verbose)
	{
		iString message(L"Initializing Transmission - Configuration: ");
		message
			+= (this->IO_Info.mode == SEND_)
			? L"Output"
			: L"Input";

		this->PrintMessage(
			message.GetData(),
			this->messageColors.ok
		);
	}

	DWORD ThreadID;
	HANDLE hThread;
	LPTRANSMISSION_THREAD_DATA tinfo = NULL;

	tinfo = new TRANSMISSION_THREAD_DATA;

	if(tinfo == nullptr)
	{
		return FALSE;
	}

	tinfo->MODE = Mode;
	tinfo->additional = 0;
	tinfo->dataTraf = reinterpret_cast<LONG_PTR>( this );

	hThread =
		CreateThread(
			nullptr, 0,
			SerialComm::TransmissionProc,
			(LPVOID)tinfo,
			0,
			&ThreadID
		);
	if(hThread == nullptr)
	{
		return FALSE;
	}
	else
	{
		WaitForSingleObject(hThread, 50);
		CloseHandle(hThread);

		return TRUE;
	}
}

DWORD WINAPI SerialComm::TransmissionProc( LPVOID lParam )
{
	DWORD exitcode = 0;

	auto gettinfo = reinterpret_cast<LPTRANSMISSION_THREAD_DATA>(lParam);
	if (gettinfo != nullptr)
	{
		auto Mode = gettinfo->MODE;
		auto _this_ = reinterpret_cast<SerialComm*>(gettinfo->dataTraf);

		auto hr = (_this_ != nullptr) ? S_OK : E_POINTER;
		if (SUCCEEDED(hr))
		{
			iString message;
			WCHAR PortID[20] = { 0 };

			InterlockedExchange((LONG*)&_this_->threadActive, (LONG)TRUE);

			hr =
				StringCbPrintf(
					PortID,
					sizeof(PortID),
					L"\\\\.\\COM%i\0",
					_this_->Configuration->Active_port
				);
			if (SUCCEEDED(hr))
			{
				if (_this_->verbose)
				{
					message.Replace(L"Opening Port - COM");
					message += _this_->Configuration->Active_port;
					_this_->PrintMessage(message.GetData(), _this_->messageColors.ok);
					_this_->NextLine();
				}

				DCB dcb;
				COMMTIMEOUTS timeouts;
				OVERLAPPED ovl;
				HANDLE hFile, hEvent;

				hFile =
					CreateFile(
						PortID,
						GENERIC_READ | GENERIC_WRITE,
						0,
						NULL,
						OPEN_EXISTING,
						FILE_FLAG_OVERLAPPED,
						NULL
					);

				hr = (hFile == INVALID_HANDLE_VALUE) ? E_HANDLE : S_OK;
				if(FAILED(hr))
				{
					_this_->PrintErrorMessage(
						L"transmission proc",
						L"error opening port (handle invalid)"
					);
					_this_->NextLine();

					exitcode = 1;
				}
				else
				{
					if (_this_->verbose)
					{
						_this_->PrintMessage(L"Configuring Port...", _this_->messageColors.warning);
					}

					SecureZeroMemory(&dcb, sizeof(DCB));
					dcb.DCBlength = sizeof(DCB);

					hr = GetCommState(hFile, &dcb) ? S_OK : E_FAIL;
					if(FAILED(hr))
					{
						_this_->PrintErrorMessage(
							L"transmission proc",
							L"error retrieving COM-state"
						);
						_this_->NextLine();

						exitcode = 2;
					}
					else
					{
						hr = _this_->serialInit(&dcb, &timeouts, &ovl);
						if (FAILED(hr))
						{
							_this_->PrintErrorMessage(
								L"transmission proc",
								L"serial initalization failed"
							);
							_this_->NextLine();

							exitcode = 3;
						}
						else
						{
							hr = SetCommState(hFile, &dcb) ? S_OK : E_FAIL;
							if(FAILED(hr))
							{
								_this_->PrintErrorMessage(
									L"transmission proc",
									L"error setting COM-state"
								);
								_this_->NextLine();

								exitcode = 4;
							}
							else
							{
								hr = GetCommTimeouts(hFile, &timeouts) ? S_OK : E_FAIL;
								if(FAILED(hr))
								{
									_this_->PrintErrorMessage(
										L"transmission proc",
										L"error getting COM-timeouts"
									);
									_this_->NextLine();

									exitcode = 5;
								}
								else
								{
									hr = SetCommTimeouts(hFile, &timeouts) ? S_OK : E_FAIL;
									if(FAILED(hr))
									{
										_this_->PrintErrorMessage(
											L"transmission proc",
											L"error setting COM-timeouts"
										);
										_this_->NextLine();

										exitcode = 6;
									}
									else
									{
										hEvent =
											CreateEvent(NULL, TRUE, FALSE, NULL);

										hr = (hEvent != nullptr) ? S_OK : E_HANDLE;
										if(FAILED(hr))
										{
											_this_->PrintErrorMessage(
												L"transmission proc",
												L"hooking up hardware event failed"
											);
											_this_->NextLine();

											exitcode = 7;
										}
										else
										{
											ovl.hEvent = hEvent;

											SetCommMask(hFile, EV_RXCHAR);

											if (Mode == SEND_)
											{
												if (_this_->verbose)
												{
													_this_->PrintMessage(L"Config complete - starting Transmission", _this_->messageColors.ok);
													_this_->NextLine();
													_this_->NextLine();
												}

												if (!_this_->Process_Sending(hFile, &ovl))
												{
													_this_->PrintErrorMessage(
														L"transmission proc",
														L"sending-process failed"
													);
													_this_->NextLine();
												}
											}
											else if (Mode == RECEIVE_)
											{
												if (_this_->verbose)
												{
													_this_->PrintMessage(L"Config complete - enable reception", _this_->messageColors.ok);
													_this_->NextLine();
													_this_->NextLine();
												}

												_this_->PrintMessage(
													getStringFromResource(UI_DATAEXCHANGE_COMREADY),
													_this_->messageColors.ok
												);
												_this_->NextLine();
												_this_->NextLine();

												if (!_this_->Process_Recieving(hFile, &ovl))
												{
													_this_->PrintErrorMessage(
														L"transmission proc",
														L"reception-process failed"
													);
													_this_->NextLine();
												}
											}
											InterlockedExchange((LONG*)&_this_->threadActive, (LONG)FALSE);

											if ((_this_->threadInterruptCtrl == INTERRUPT_TRANSMISSION) || (_this_->threadInterruptCtrl == TERMINATE)
												|| (_this_->threadInterruptCtrl == TERMINATE_AND_SET_TOTAB))
											{
												SendMessage(_this_->DataTrafficWnd, WM_INTERRUPT, static_cast<WPARAM>(Mode), static_cast<LPARAM>(0));
											}
											else if (_this_->threadInterruptCtrl == TRANSMISSION_COMPLETE)
											{
												PostMessage(_this_->DataTrafficWnd, WM_CLOSE, static_cast<WPARAM>(0), static_cast<LPARAM>(0));
											}

											CloseHandle(hEvent);
										}
									}
								}
							}
						}
					}
					CloseHandle(hFile);
				}
			}
			InterlockedExchange((LONG*)&_this_->threadActive, (LONG)FALSE);
		}
		delete gettinfo;
	}
	return exitcode;
}

DWORD SerialComm::bufferConvert(char** low_Buffer)
{
	if (this->verbose)
	{
		this->PrintMessage(L"Formatting Buffer for output", this->messageColors.warning);
	}
	DWORD cbData = 0;

	EditorContentManager ecm;
	ecm.SetExecuteAsync(false);
	ecm.SetContent(this->SRtext);

	auto dataContainer =
		reinterpret_cast<ApplicationData*>(
			getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
			);

	if (dataContainer != nullptr)
	{
		auto formatIndex = dataContainer->getIntegerData(DATAKEY_EXSETTINGS_EXCHANGEWND_LINEENDFORMAT, 0);

		switch (formatIndex)
		{
		case 0:
			ecm.SetEndOfLineFormat(EditorContentManager::ENDOFLINE_FORMAT_CRLF);
			break;
		case 1:
			ecm.SetEndOfLineFormat(EditorContentManager::ENDOFLINE_FORMAT_CR);
			break;
		case 2:
			ecm.SetEndOfLineFormat(EditorContentManager::ENDOFLINE_FORMAT_LF);
			break;
		default:
			ecm.SetEndOfLineFormat(EditorContentManager::ENDOFLINE_FORMAT_CRLF);
			break;
		}

		if (ecm.PrepareContentForSubsequentUsage())
		{
			auto res =
				ConvertWCHARtoCHAR(
					ecm.GetContent(),
					low_Buffer
				);

			cbData = _lengthOfString(
				ecm.GetContent()
			);

			if ((res != TRUE) || (cbData == 0))
			{
				this->PrintErrorMessage(L"Formatting routine", L"Formatting failed");
			}
			else
			{
				if (this->verbose)
				{
					this->PrintMessage(L"Format succeeded", this->messageColors.ok);
					this->NextLine();
					this->NextLine();
				}
			}
		}
	}
	return cbData;
}

DWORD SerialComm::bufferReconvert(char* low_Buffer)
{
	DWORD array_size = 0, res = 0;

	array_size = MultiByteToWideChar(CP_UTF8, 0, low_Buffer, -1, NULL, 0);	//CODEPAGE

	if( array_size > 0 )
	{
		this->SRtext = new WCHAR[array_size];

		if(this->SRtext != nullptr)
		{
			res = MultiByteToWideChar(CP_UTF8, 0, low_Buffer, -1, this->SRtext, array_size);	//CODEPAGE

			// the richedit uses Codepage 1200, this only allows /r as linefeed
			// so convert it!

			EditorContentManager ecm;
			ecm.SetExecuteAsync(false);
			ecm.SetContent(this->SRtext);
			ecm.SetEndOfLineFormat(EditorContentManager::ENDOFLINE_FORMAT_CR);

			if (ecm.PrepareContentForSubsequentUsage())
			{
				SafeDeleteArray(&this->SRtext);
				CopyStringToPtr(ecm.GetContent(), &this->SRtext);
			}
		}
	}
	return res;
}

BOOL SerialComm::Process_Sending(HANDLE hFile, LPOVERLAPPED ovl)
{
	DWORD counter = 0;
	char *low_Buffer = nullptr;

	BOOL isLF =
		this->IsLF_Format();

	DWORD bytestoWrite =
		this->bufferConvert(&low_Buffer);

	DWORD bytesWritten, Err, wait_result;

	while( counter < bytestoWrite )
	{
		this->PrintCharacter(low_Buffer[ counter ]);

		if((low_Buffer[ counter ] == '\n') && isLF)
		{
			this->PrintCharacter((char)0x0D);
		}

		WriteFile( hFile, &low_Buffer[ counter ], 1, &bytesWritten, ovl );
		counter++;

		if( this->threadInterruptCtrl == TERMINATE )
		{
			SafeDeleteArray(&low_Buffer);
			return TRUE;
		}

		Err = GetLastError(); // when no byte is written
		if( Err == ERROR_IO_PENDING )
		{
			// wait on writeFile, because the port is used overlapped
			while( 1 )
			{	
				// wait on the 100 milliseconds event
				wait_result = WaitForSingleObject( ovl->hEvent, 100 ); 
				if( wait_result == WAIT_TIMEOUT )
				{
					if( ( this->threadInterruptCtrl == INTERRUPT_TRANSMISSION ) ||
						( this->threadInterruptCtrl == TERMINATE ) )
					{
						SafeDeleteArray(&low_Buffer);
						return TRUE;
					}
				}
				else if( wait_result == WAIT_OBJECT_0 )
				{
					break;
				}
				else
				{
					Err = GetLastError();
					//this->Get_Error( L"Overlapped Transmission (send/1)" );
					break;
				}
			}
			if( !GetOverlappedResult( hFile, ovl, &bytesWritten, FALSE ) )
			{
				Err = GetLastError(); 
				if( (Err == ERROR_IO_PENDING) || (Err == ERROR_IO_INCOMPLETE) )
				{ 
					TCHAR* msg = nullptr;
					TranslateLastError(&msg);
					this->PrintErrorMessage(L"Sending Process _Lc1", msg);
					SafeDeleteArray(&msg);
					SafeDeleteArray(&low_Buffer);
					return FALSE;
				}
			}
			else
			{}
		}// ok -> character written
		else
		{
			TCHAR* msg = nullptr;
			TranslateLastError(&msg);
			this->PrintErrorMessage(L"Sending Process _Lc2", msg);
			SafeDeleteArray(&msg);
			SafeDeleteArray(&low_Buffer);
			return FALSE;
		}
	}
	InterlockedExchange((LONG*)&this->threadInterruptCtrl, (LONG)TRANSMISSION_COMPLETE);

	if(low_Buffer != nullptr)
	{
		delete[] low_Buffer;
	}
	return TRUE;
}

BOOL SerialComm::Process_Recieving(HANDLE hFile, LPOVERLAPPED ovl)
{
	int counter = 0;
	bool array_start = false, holding = true;
	char trans = 0;
	char *low_Buffer = nullptr;
	DWORD BytesRecieved, wait_result, Err;

	low_Buffer = new (std::nothrow) CHAR[500000];
	if (low_Buffer == nullptr)
		return FALSE;

	while(holding)
	{		
		while(ReadFile(hFile, &trans, 1, &BytesRecieved, ovl))
		{
			// new data read
			if((trans != 0)&&(!array_start))			// old line: if( trans == '%' )
			{
				PrintCharacter((char)0x0D);
				PrintCharacter((char)0x0D);

				array_start = true;
			}
			if(array_start)
			{
				low_Buffer[ counter ] = trans;
				this->PrintCharacter(trans);
				counter++;
			}
			if( this->threadInterruptCtrl == TERMINATE )
			{
				SafeDeleteArray(&low_Buffer);
				return TRUE;
			}
			else if (this->threadInterruptCtrl == TERMINATE_AND_SET_TOTAB)
			{
				low_Buffer[counter] = '\0';
				this->bufferReconvert(low_Buffer);
				delete[] low_Buffer;
				return TRUE;
			}
		}
		Err = GetLastError(); // when no byte is read

		if (Err == ERROR_IO_PENDING)
		{
			// wait on a character, because the port is used overlapped
			while( 1 )
			{
				// wait on the 500 milliseconds timeout
				wait_result = WaitForSingleObject(ovl->hEvent, 500);

				if (wait_result == WAIT_TIMEOUT)
				{	
					if(array_start)
					{
						holding = false;
						break;
					}
					if((this->threadInterruptCtrl == INTERRUPT_TRANSMISSION) ||
						(this->threadInterruptCtrl == TERMINATE) || (this->threadInterruptCtrl == TERMINATE_AND_SET_TOTAB))
					{
						if((this->threadInterruptCtrl == INTERRUPT_TRANSMISSION ) || (this->threadInterruptCtrl == TERMINATE_AND_SET_TOTAB))
						{
							low_Buffer[ counter ] = '\0';
							this->bufferReconvert( low_Buffer );
						}
						SafeDeleteArray(&low_Buffer);
						return TRUE;
					}
				}
				else if(wait_result == WAIT_OBJECT_0)
				{
					break;
				}
				else
				{
					Err = GetLastError();
					break;
				}
			}
			if( !holding )
			{
				break;
			}
			if(	!GetOverlappedResult( hFile ,ovl, &BytesRecieved, FALSE ) )
			{
				Err = GetLastError(); 
				if( ( Err == ERROR_IO_PENDING ) || ( Err == ERROR_IO_INCOMPLETE ) )
				{
					// no character ? severe error
					TCHAR* msg = nullptr;
					TranslateLastError(&msg);
					this->PrintErrorMessage(L"Reception Process _Lc1", msg);
					SafeDeleteArray(&msg);
					SafeDeleteArray(&low_Buffer);
					return FALSE;
				}
			}
			else
			{
				// catch the characters before they get lost..
				if ((trans != 0) && (!array_start))
				{
					array_start = true;
				}
				if( array_start )
				{
					low_Buffer[ counter ] = trans;
					this->PrintCharacter(trans);
					counter++;
				}
			}
		}
		else
		{
			TCHAR* msg = nullptr;
			TranslateLastError(&msg);
			this->PrintErrorMessage(L"Reception Process _Lc2", msg);
			SafeDeleteArray(&msg);
			SafeDeleteArray(&low_Buffer);
			return FALSE;
		}
	}
	low_Buffer[ counter ] = '\0';

	this->bufferReconvert( low_Buffer );

	if( low_Buffer != NULL )
	{
		delete[] low_Buffer;
	}
	return TRUE;
}

void SerialComm::PrintMessage(LPCTSTR message, COLORREF color)
{
	if (this->console != nullptr)
	{
		this->console->AddLine(message, color);
		this->console->Update();
	}
}

void SerialComm::PrintMessage(LPCTSTR message)
{
	if (this->console != nullptr)
	{
		this->console->AddLine(message);
		this->console->Update();
	}
}

void SerialComm::PrintErrorMessage(LPCTSTR location, LPCTSTR message)
{
	iString error(L"error <");
	error += location;
	error += L">: ";
	error += message;
	this->PrintMessage(error.GetData(), this->messageColors.error);
}

void SerialComm::PrintCharacter(char c)
{
	this->IO_Info.processedDataSize++;

	HDC hdc = GetDC(this->DataTrafficWnd);
	if (hdc)
	{
		this->drawWindow(this->DataTrafficWnd, hdc);
		ReleaseDC(this->DataTrafficWnd, hdc);
	}

	if (this->console != nullptr)
	{
		if (c != '\n')
		{
			if (c == '\r')
			{
				this->console->AddEmptyLine();
				this->console->Update();
			}
			else
			{
				this->console->AddCharacterToLine((WCHAR)c);
			}
		}
	}
}

void SerialComm::NextLine()
{
	if (this->console != nullptr)
	{
		this->console->AddEmptyLine();
		this->console->Update();
	}
}

LRESULT SerialComm::OnClose( HWND Traffic )
{
	if(this->threadActive)
	{
		InterlockedExchange((LONG*)&this->threadInterruptCtrl, (LONG)TERMINATE);
	}
	else
	{
		if (
			reinterpret_cast<ApplicationData*>(
				getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
				)->getBooleanData(DATAKEY_EXSETTINGS_EXCHANGEWND_AUTOCLOSE, true)
			)
		{
			DestroyWindow(Traffic);
		}
		else
		{
			if (this->IO_Info.mode == SEND_)
			{
				auto button =
					reinterpret_cast<CustomButton*>(
						CUSTOMBUTTONCLASS_FROMID(this->DataTrafficWnd, D_TRAFFIC_CANCEL)
						);
				if (button != nullptr)
				{
					iString buttontext(
						getStringFromResource(UI_GNRL_CLOSE)
					);
					button->setAppearance_onlyText(buttontext, FALSE);
					button->Update();
				}
			}
		}
		
	}
	return static_cast<LRESULT>(0);
}

void SerialComm::Set_Recieved_Text(void)
{
	if( this->SENDBUFFERTOMAIN )
	{
		if(this->SRtext != nullptr)
		{
			if (this->SRtext[0] != L'\0')
			{
				SendMessage(this->ParentWindow, WM_TRANSMISSIONCOMPLETE, 0, reinterpret_cast<LPARAM>(this->SRtext));
			}
			else
			{
				DispatchEWINotification(
					EDSP_WARNING,
					L"DEC0001",
					getStringFromResource(UI_DATAEXCHANGE_SETTEXT_NOCONT),
					L"DataExchange"
				);
			}
		}
	}
}

void SerialComm::onCancelTransmission(HWND Traf)
{
	if (this->DataTrafficWnd != nullptr)
	{
		if (this->console != nullptr)
		{
			this->NextLine();
			this->PrintMessage(
				getStringFromResource(UI_GNRL_USERCANCELCONFIRM),
				this->messageColors.warning
			);
			this->NextLine();
		}
	}

	// dangerous -> race conditions?

	if(this->threadActive)
	{
		// terminate and discard received content
		InterlockedExchange((LONG*)&this->threadInterruptCtrl, (LONG)TERMINATE);
	}
	else
	{
		DestroyWindow(Traf);
	}
}

void SerialComm::onFinishTransmission( HWND Traf )
{
	if(this->threadActive)
	{
		// terminate but set the received content to tab (if there is one)
		InterlockedExchange((LONG*)&this->threadInterruptCtrl, (LONG)TERMINATE_AND_SET_TOTAB);
	}
	else
	{
		if(this->SRtext != nullptr)
		{
			this->Set_Recieved_Text();
			DestroyWindow( Traf );
		}
	}
}

LRESULT SerialComm::OnInterrupt(HWND hWnd, WPARAM wParam)
{
	int Mode = static_cast<int>(wParam);

	if( this->threadInterruptCtrl == TERMINATE )
	{
		DestroyWindow(hWnd);
	}
	else if((this->threadInterruptCtrl == INTERRUPT_TRANSMISSION) || (this->threadInterruptCtrl == TERMINATE_AND_SET_TOTAB))
	{
		if( Mode == RECEIVE_ )
		{
			if(this->SRtext != nullptr)
			{
				this->Set_Recieved_Text();
			}
		}
		DestroyWindow(hWnd);
	}
	return static_cast<LRESULT>(0);
}
