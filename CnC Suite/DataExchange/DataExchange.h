#pragma once
#include"..\\external.h"
#include <SetupAPI.h>
#include"..\\cObject.h"
#include <ItemCollection.h>
#include <StringClass.h>
#include <new>
#include <CSConsole.h>
#include <cListView.h>
#include <CustomButton.h>

#define		CNCS_DATAX_CLASS		L"DATAEXCHANGEWINDOW_CLASS4021"
#define		CNCS_DEVSEL_CLASS		L"DEVICESELECTIONWND_CLASS7398"

#define		CTRLID_OUTPUT_CONSOLE				100
#define		CTRLID_DEVICESELECTIONWINDOW		101
#define		CTRLID_DEVICESELECTIONLISTVIEW		102
#define		CTRLID_SELECTDEVICEBUTTON			103
#define		CTRLID_BACKBUTTON					104


#define		DATAEXCHANGECLASS		1049 // for the WM_CLEANUP message - it's not save - replace or search for another method !!!

//#define		SR_TRAF_EDIT			1168

#define		D_TRAFFIC_CANCEL		1204
#define		D_TRAFFIC_FINISH		1205

#define		SEND_					1240
#define		RECEIVE_				1241
#define		SEND_BUFFER				1242
#define		RECIEVE_BUFFER			1243
#define		TERMINATE				1252
#define		INTERRUPT_TRANSMISSION	1253
#define		TRANSMISSION_COMPLETE	1254

#define		TRAFFICMODE				1256

#define		MAX_STRINGLENGTH	56
#define		UNDEF				-1
#define		COMMONARRAYSIZE		512

#define		DEFAULTCOLOR		RGB( 0,0,0 )
#define		BK_DEFAULT			RGB( 255,255,255 )

#define		BAUDRATE_STRUCTSIZE		15

const GUID GUID_CLASS_COMPORT = { 0x86e0d1e0, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 };

static const DWORD _baud[] = {	CBR_110, CBR_300, CBR_600, CBR_1200, CBR_2400, CBR_4800, CBR_9600, CBR_14400,
								CBR_19200, CBR_38400, CBR_56000, CBR_57600, CBR_115200, CBR_128000, CBR_256000				};

static const BYTE parity[] = {	NOPARITY, ODDPARITY, EVENPARITY, MARKPARITY, SPACEPARITY };

static const BYTE stoppbts[] = {	ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS };

typedef struct String_List{

	int NUM_STRINGS;
	WCHAR Stringlist[ 65 ][ MAX_STRINGLENGTH ];

}STRINGSTOCOMBO, *PSTRINGSTOCOMBO;

typedef struct _TRANSMISSON_THREAD_DATA{

	int MODE;
	int additional;
	LONG_PTR dataTraf;

}TRANSMISSION_THREAD_DATA, *LPTRANSMISSION_THREAD_DATA;

typedef struct _SERIAL_CONFIG{

	int Active_port;
	int baud_index;
	int parity_index;
	int databit_index;
	int stopbit_index;
	int DTR_control_index;
	int RTS_control_index;
	TCHAR XonChar;
	TCHAR XoffChar;
	TCHAR ErrorChar;
	TCHAR EofChar;
	TCHAR EventChar;
	int RI_Timeout;
	int RT_Timeout_cst;
	int RT_Timeout_mpl;
	int WT_Timeout_cst;
	int WT_Timeout_mpl;
	BOOL DSR_Sense;
	BOOL Paritycheck;
	BOOL Abort_on_error;
	BOOL ErrorChar_replace;
	BOOL CTS_Flow;
	BOOL DSR_Flow;
	BOOL XON_XOFF;

	_SERIAL_CONFIG& _SERIAL_CONFIG::operator= (const _SERIAL_CONFIG& sc)
	{
		this->Active_port = sc.Active_port;
		this->baud_index = sc.baud_index;
		this->parity_index = sc.parity_index;
		this->databit_index = sc.databit_index;
		this->stopbit_index = sc.stopbit_index;
		this->DTR_control_index = sc.DTR_control_index;
		this->RTS_control_index = sc.RTS_control_index;
		this->XoffChar = sc.XoffChar;
		this->XonChar = sc.XonChar;
		this->ErrorChar = sc.ErrorChar;
		this->EofChar = sc.EofChar;
		this->EventChar = sc.EventChar;
		this->RI_Timeout = sc.RI_Timeout;
		this->RT_Timeout_cst = sc.RT_Timeout_cst;
		this->RT_Timeout_mpl = sc.RT_Timeout_mpl;
		this->WT_Timeout_cst = sc.WT_Timeout_cst;
		this->WT_Timeout_mpl = sc.WT_Timeout_mpl;
		this->DSR_Sense = sc.DSR_Sense;
		this->Paritycheck = sc.Paritycheck;
		this->Abort_on_error = sc.Abort_on_error;
		this->ErrorChar_replace = sc.ErrorChar_replace;
		this->CTS_Flow = sc.CTS_Flow;
		this->DSR_Flow = sc.DSR_Flow;
		this->XON_XOFF = sc.XON_XOFF;

		return *this;
	}

}SERIAL_CONFIG, *PSERIAL_CONFIG;

typedef struct _INOUTINFO {
	int mode;
	int totalDataSize;//(in bytes)
	int processedDataSize;//(in bytes)

}INOUTINFO, *LPINOUTINFO;

__interface IDeviceSelectionProtocol {
public:
	void OnDeviceSelected(cObject sender, iString& friendlyName);
	void OnQuit(cObject sender);
};

class SerialComm
	: public ObjectRelease<SerialComm>,
	//public IConsoleInputEvent,
	public customButtonEventSink
{
public:			
	// the default constructor is for the config setter/getter methods only !!
	SerialComm();
	// the special constructor must be used for the data transmission and the device-listing
	SerialComm(HWND Parent,HINSTANCE hInstance,WCHAR* Root_Directory);
	
	~SerialComm();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Exchange-window:

	// start transmission
	HRESULT InitDataTransmission(int Mode, LPTSTR directBuffer, LPRECT pos);

	void enableVerboseMessaging(bool enable) {
		this->verbose = enable;
	}

	// Converts the buffer corresponding to the rules
	// Return value: -1 == error (otherwise the return value is the size of the new buffer in tchars)
	static int PrepareBufferForTransmission(LPPTSTR buffer, bool removeBracketAnnotations, bool removeApostrophAnnotations, bool removeSpaces);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// setter + getter for the saved configuration:

	HRESULT setConfiguration(const TCHAR* path, PSERIAL_CONFIG config_in);
	HRESULT getConfiguration(const TCHAR* path, PSERIAL_CONFIG config_out);
	HRESULT setDefaultConfiguration(const TCHAR* path);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Devicelisting:

	// the listing method can be used static without instantiation
	static HRESULT CreateSerialDeviceListing(itemCollection<iString>* deviceListing);

	// Creates the listing window as a child-window
	// NOTE: Do not call Release on the serialComm object if the listing window is created!
	//       Call 'DestroyDeviceListingWindow' instead, and the object will be released automatically
	HRESULT CreateDeviceListingWindow(LPPOINT pos, LPSIZE size, IDeviceSelectionProtocol* eventHandler);
	void DestroyDeviceListingWindow();

	// This method returns the com-port in a device-string out of the device-listing
	static int ExtractComPortFromDeviceName(iString& deviceName);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// button event:

	void onCustomButtonClick(cObject sender, CTRLID ID_button);
			
private:
	HINSTANCE hInst;
	HWND DataTrafficWnd;
	HWND DeviceSelectionWnd;
	HWND ParentWindow;

	POINT anchorPosition;
	SIZE anchorSize;

	HBRUSH TrafBackground;
	HFONT controlFont;

	BOOL ActivationStatus;
	BOOL SENDBUFFERTOMAIN;

	int threadInterruptCtrl;// old - not valid anymore : (DOUBLE USAGE ( S/R threadInterruptCtrl & THREAD INTERRUPT ))

	BOOL threadActive;
	bool verbose;

	WCHAR *Root_Dir;
	WCHAR *SRtext;

	PSERIAL_CONFIG Configuration;
	APPSTYLEINFO sInfo;
	INOUTINFO IO_Info;
	MSGCOLORS messageColors;

	CSConsole* console;
	cListView* deviceSelectionListview;
	IDeviceSelectionProtocol* deviceSelectionEvents;

	static DWORD WINAPI TransmissionProc(LPVOID);
	static LRESULT CALLBACK DataExchangeWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK DeviceSelectionWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HRESULT registerDataExchangeWindowClass();
	HRESULT registerDeviceSelectionWindowClass();

	HRESULT InitDataExchangeWindowEx(LPPOINT position, LPSIZE size);// the position + size must be the anchor-placement, the free placement will be automatically calculated
	HRESULT InitDataExchangeWindowContentEx();

	HRESULT createDeviceSelectionWindowContent();

	LRESULT onSizeInTrafficWindow();
	LRESULT onPaintInTrafficWindow(HWND hWnd);
	LRESULT onPaintInDeviceSelectionWindow(HWND hWnd);

	void PrintMessage(LPCTSTR message, COLORREF color);
	void PrintMessage(LPCTSTR message);
	void PrintErrorMessage(LPCTSTR location, LPCTSTR message);
	void PrintCharacter(char c);
	void NextLine();

	BOOL startTransmissionThread(int);
	HRESULT serialInit(LPDCB,LPCOMMTIMEOUTS,LPOVERLAPPED);
	DWORD bufferConvert(char**);
	DWORD bufferReconvert(char*);
	void Set_Recieved_Text(void);

	LRESULT OnClose(HWND);
	LRESULT OnDestroy();
	LRESULT OnInterrupt(HWND hWnd, WPARAM wParam);

	void onCancelTransmission(HWND);
	void onFinishTransmission(HWND);
	
	HRESULT _LoadControlsFromFile(void);
	HRESULT _SaveControlsToFile(void);

	BOOL Process_Sending(HANDLE,LPOVERLAPPED);
	BOOL Process_Recieving(HANDLE,LPOVERLAPPED);

	BOOL CheckForNumber(WCHAR);
	void SetDefault(void);
	BOOL IsLF_Format();

	void drawWindow(HWND hWnd, HDC hdc);
};