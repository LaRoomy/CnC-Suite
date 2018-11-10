#pragma once
#ifndef _BASIC_HELPER_FUNCTIONS_
#define _BASIC_HELPER_FUNCTIONS_

#define		MAXXCOMPARE		0x7FFF

#include"external.h"

#define		RichEdit_SetBkColor(e,c)		(COLORREF)SNDMSG((HWND)e, EM_SETBKGNDCOLOR, (WPARAM)0, (LPARAM)c)
#define		RichEdit_SetCharFormat(e,f,c)	(BOOL)SNDMSG((HWND)e, EM_SETCHARFORMAT, (WPARAM)f, reinterpret_cast<LPARAM>(c))
#define		RichEdit_GetSelection(ew,cf)	SNDMSG((HWND)ew, EM_EXGETSEL, (WPARAM)0, reinterpret_cast<LPARAM>(cf))
#define		RichEdit_SetSelection(ew,cf)	SNDMSG((HWND)ew, EM_EXSETSEL, (WPARAM)0, reinterpret_cast<LPARAM>(cf))

#define		IS_VALUE_IN_RANGE(v,min,max)		((bool)((((LONG)v)>=((LONG)min))&&(((LONG)v)<=((LONG)max))))

// 1 is reserved(TRUE)
#define		PTR_ALREADY_REFERENCED		2
#define		PTR_ACCESS_VIOLATION		3
#define		PTR_IS_NULL					4
#define		PTR_DEST_INVALID			5

#define		EXPT_INVALID_ARGUMENT		6
#define		EXPT_ARRAY_BOUNDS_EXCEEDED	7

#define		INCLUDE_FOLDERS				0x01
#define		INCLUDE_FILES				0x02

typedef struct _DIRLIST {

	DWORD fileAttributes;
	TCHAR filename[256];

}DIRLIST, *LPDIRLIST;

// monitor integer values
void show_integer(int v1, int v2);

// monitor float values
void show_float(double v1, double v2);

// returns TRUE if L is a number, FALSE if not (dot is not included)
BOOL IsNumber(TCHAR L);

// returns TRUE if 'LT' is a character(letter a-z & A-Z), FALSE if not
BOOL IsChar(TCHAR LT);

// the calling process is responsible for freeing the converted string !
// string_out must be a pointer set to NULL at the time of the function-call if it's not the function returns PTR_ALREADY_REFERENCED
// RETURN VALUE: only TRUE indicates success
BOOL AppendStringToStringA(LPCTSTR first, LPCTSTR second, TCHAR** string_out);

// the calling process is responsible for freeing the converted string !
// string_out must be a pointer set to NULL at the time of the function-call if it's not the function returns PTR_ALREADY_REFERENCED
// RETURN VALUE: only TRUE indicates success
BOOL AppendStringToStringW(LPCWSTR first, LPCWSTR second, WCHAR** string_out);

// the calling process is responsible for freeing the converted string !
// string_out must be a pointer set to NULL at the time of the function-call if it's not the function returns PTR_ALREADY_REFERENCED
// RETURN VALUE: only TRUE indicates success
BOOL CopyStringToPtrW(LPCWSTR source_in, WCHAR** destination_out);

// the calling process is responsible for freeing the converted string !
// string_out must be a pointer set to NULL at the time of the function-call if it's not the function returns PTR_ALREADY_REFERENCED
// RETURN VALUE: only TRUE indicates success
BOOL CopyStringToPtrA(LPCTSTR source_in, TCHAR** destination_out);

// retrieves the zero-based index of CHR for sort routines
int GetIndexOfCharForSort(TCHAR CHR_in);

// compares the key-string with the compare-string -> returns 1 if the comparestring should follow the keystring, -1 if it should precede
// and 0 if the strings are equal - all other return values pose an error / watch out !-> both strings should be zero-terminated
int CompareStringsForSort(LPCTSTR key, LPCTSTR toCompare);

// Compares both strings -> TRUE if equal, FALSE otherwise
BOOL CompareStringsB(LPCTSTR string1, LPCTSTR string2);

// Compares two strings (case insensitive)
BOOL CompareStringsNoCase(LPCTSTR string1, LPCTSTR string2);

// Compares two letter (case insensitive)
BOOL CompareCharNoCase(TCHAR L1, TCHAR L2);

// shows a simple MessageBox with the entered text
void showText(LPCWSTR);

// retrieves the filename out of a path, with or without fileextension
// filename_out must be a pointer set to NULL at the time of the function call if it's not the function returns PTR_ALREADY_REFERENCED
// RETURN VALUE: only TRUE indicates success
BOOL GetFilenameOutOfPath(LPCTSTR path, TCHAR** filename_out, BOOL hideFileExt);

// Appends one string to another -> argumentlist must be limited with NULL !!!!!!!!!!
// assembledString_out must be a pointer set to NULL at the time of the function call if it's not the function returns PTR_ALREADY_REFERENCED
// RETURN VALUE: only TRUE indicates success
BOOL AppendStringsWithVaList(TCHAR** assembledString_out, ...);

// Inverts the specified COLORREF value
COLORREF InvertColor(COLORREF);

// Returns TRUE if this pointer is valid, FALSE otherwise ...
BOOL IsPtrValid(LPVOID ptr);

// Adds a simple Tooltip to a control window
BOOL AddToolipToControl(HWND ControlWnd, HWND Parent, PTSTR text, HINSTANCE hInst, HWND* tipWnd_out);

// Used to change the tooltip-text of an existing control
BOOL ChangeTooltipText(HWND tooltip, HWND ControlWnd, HWND Parent, PTSTR text, HINSTANCE hInst);

// Converts WCHAR to char
// converted_out must be a pointer set to NULL at the time of the function call - if it's not the function returns PTR_ALREADY_REFERENCED
// RETURN VALUE: only TRUE indicates success
BOOL ConvertWCHARtoCHAR(const WCHAR* toConvert, char** converted_out);

// Converts char to WCHAR
// converted_out must be a pointer set to NULL at the time of the function call - if it's not the function returns PTR_ALREADY_REFERENCED
// RETURN VALUE: only TRUE indicates success
BOOL ConvertCHARtoWCHAR(const char* toConvert, WCHAR** converted_out);

// Retrieves the absolut amount of chars in a string
// RETURN VALUE: -1 is error, success otherwise
int GetCharCount(TCHAR *string);

// Converts Pixel to Twips in Y direction!
LONG ConvertYPixToTwips(HWND hWnd, int Ypix);

// Converts Twips to Pixel
int ConvertTwipsToPix(int twips);

// Retrieves the text from an Richeditcontrol
BOOL GetRichEditContent(HWND edit, TCHAR** text_out);

// sets the text of an Richeditcontrol
BOOL SetRichEditContent(HWND edit, LPCTSTR text,	DWORD flags);

// Retrieves the current selection from an rich edit control
BOOL GetRichEditSelectionContent(HWND edit, TCHAR** text_out);

// Displays an single character
void DisplaySingleChar(TCHAR c);

// translates the last occured error (given back by GetLastError())
// and retrieves a string containing the message, the return-value is the errorcode
int TranslateLastError(TCHAR** msg_out);

// translates the given hresult to a string
void TranslateError(HRESULT err, TCHAR ** msg_out);

// prints text into the selected device context
void TextOutDC(HDC hdc, LPCTSTR text, int x, int y, HFONT font, COLORREF textcolor);

// prints text into the device context retrieved from the window
void TextOutWnd(HWND hWnd, LPTSTR text, int x, int y, HFONT font, COLORREF textcolor);

// retrieves the textsize of a font printed in a Device-Context
BOOL getTextSizeDC(HDC hdc, HFONT font, LPCTSTR string, LPSIZE pSize);

// initializes a charformat structure
void initCharformat(CHARFORMAT* cf, int height, COLORREF Textcolor, LPTSTR fontFamily);

// retrieves the current system language
int getSystemLanguage();

// retrieves the amount of threads in the specified process
int GetRunningThreadCountFromSpecificProcess(_In_ DWORD processID);

// retrieves a list of the thread IDs of the threads running in the specified process
// return value is the amount if IDs in the array or 0 if the function fails
// the caller is responsible for freeing the memory of the list
BOOL GetIdentifierListOfActiveThreadsForProcess(_In_ DWORD processID,_Inout_ DWORD** list_out);

// This function counts the linefeed(0x0A) inside the buffer
// Return value: 0 -> no linefeed / -1 -> buffer was null / -2 -> fail(exception) / otherwise the amount of linefeeds
int countLinefeed(LPCTSTR lpszbuffer);

// retrieves the length of a string with string-safe functions (excluding the terminating null character)
int _lengthOfString(LPCTSTR str);

// this function cuts a buffer and puts the new segment in a new buffer
// newbuffer_out must be nullptr by calling this function - and the caller is responsible for freeing the buffer
BOOL _cutString(LPCTSTR _string, int indexLow, int indexHigh, TCHAR** newbuffer_out);

// retrieves the current line from the selected index
// if the index is on a carriage return, the previous line will be recorded
// calling conventions std - outPtr
BOOL _getlineFromCharIndexOutOfText(LPCTSTR text, int charIndex, TCHAR** line_out);

// retrieves a list of files and/or folders in the requested directory
// return-value: length of the list (negative value on error)
// please note: the parameter list_out must be set to nullptr when calling the method, and the caller is responsible for the cleanUp
int _dirContentListing(LPCTSTR dir_path, LPDIRLIST *list_out, DWORD dwFlags);

// true if the point is inside of the rect
bool isPointInRect(int x, int y, LPRECT prc);

// true if the rects are equal
bool CompareRect(LPRECT rc1, LPRECT rc2);

// load an icon with default parameters
HICON LoadIconEx(HINSTANCE hInst, int iconID, int squareSize);

// morphes the inputcolor slightly to indicate a selection
COLORREF makeSelectionColor(COLORREF nCol);

// morphes the inputcolor to indicate a pressed area
// (intended to be used in conjunction with the 'makeSelectionColor'-function)
COLORREF makePressedColor(COLORREF nCol);

// ensures that the color will be grayed and will be visible on the given background
COLORREF getDisabledColor(COLORREF bkgnd);

// morphes the given color slightly darker or brighter based on the color
// this function is intended to use for text which is white or black
COLORREF getSlightlyDisabledTextColor(COLORREF txtCol);

// makes sure the window will be fully visible (ppt will be changed if not...)
// returns TRUE if the window fits (on the screen) and FALSE if the window did not fit and the point was changed
BOOL AsureWindowVisibilityFromPoint(LPPOINT ppt, int cx, int cy);

// checks if the file on the ?? path exists
BOOL CheckForFileExist(LPCTSTR path);

// scales a color down
COLORREF darkenColor(COLORREF color, int percentValue);

// scales a color up
COLORREF brightenColor(COLORREF color, int percentValue);

// ansi-version: checks the path for its type - TRUE means file, FALSE means directory (result < 0 == error)
BOOL isFileA(LPCTSTR path);

// unicode-version: checks the path for its type - TRUE means file, FALSE means directory (result < 0 == error)
BOOL isFileW(LPCTSTR path);

// creates a random integer in the between min + max (including min + max)
int CreateRandomInteger(int min, int max);

// checks if the path is a drive - TRUE if yes, FALSE if not
BOOL isDrive(LPCTSTR path);

// sets the last-accessed time in the file at the path location
BOOL setFileAccessTime(LPCTSTR path);

// retrieves the size of the file at the path loacation or 0 in case of error (or the filesize is 0)
LARGE_INTEGER getFileSizeX(LPCTSTR path);

// get a folder-path from the user with the IFileDialog
HRESULT OpenFolder(HWND owner, WCHAR** folderPath);

// checks if the length of the string printed in a device-context is possible and morphes
// the string to the desired length
// TODO: !



#if defined UNICODE
#define		CopyStringToPtr			CopyStringToPtrW
#define		AppendStringToString	AppendStringToStringW
#define		isFile					isFileW
#else
#define		CopyStringToPtr			CopyStringToPtrA
#define		AppendStringToString	AppendStringToStringA
#define		isFile					isFileA
#endif

#endif// _BASIC_HELPER_FUNCTIONS_