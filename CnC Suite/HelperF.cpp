#include<new.h>
#include"HelperF.h"
#include<TlHelp32.h>
#include<ctime>


void show_integer(int v1, int v2)
{
	TCHAR buffer[1024] = { 0 };
	StringCbPrintf(buffer, sizeof(buffer), L"V1 = %i\nV2 = %i\0", v1, v2);
	MessageBox(NULL, buffer, L"show integer values...", MB_OK | MB_ICONINFORMATION);
}

void show_float(double v1, double v2)
{
	TCHAR buffer[1024] = { 0 };
	StringCbPrintf(buffer, sizeof(buffer), L"V1 = %.3lf\nV2 = %.3lf\0", v1, v2);
	MessageBox(NULL, buffer, L"show floating values...", MB_OK | MB_ICONINFORMATION);
}

BOOL IsNumber(TCHAR L)
{
	if ((L == L'0') ||
		(L == L'1') ||
		(L == L'2') ||
		(L == L'3') ||
		(L == L'4') ||
		(L == L'5') ||
		(L == L'6') ||
		(L == L'7') ||
		(L == L'8') ||
		(L == L'9'))
		return TRUE;
	else
		return FALSE;
}

BOOL IsChar(TCHAR LT)
{
	/*
	much easier:

	auto c_val = static_cast<int>(LT);
	if((c_val >= 65 && c_val <= 90) ||
		(c_val >= 97 && c_val <= 122))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

	*/

	if (// capitals
		(LT == 'A') ||
		(LT == 'B') ||
		(LT == 'C') ||
		(LT == 'D') ||
		(LT == 'E') ||
		(LT == 'F') ||
		(LT == 'G') ||
		(LT == 'H') ||
		(LT == 'I') ||
		(LT == 'J') ||
		(LT == 'K') ||
		(LT == 'L') ||
		(LT == 'M') ||
		(LT == 'N') ||
		(LT == 'O') ||
		(LT == 'P') ||
		(LT == 'Q') ||
		(LT == 'R') ||
		(LT == 'S') ||
		(LT == 'T') ||
		(LT == 'U') ||
		(LT == 'V') ||
		(LT == 'W') ||
		(LT == 'X') ||
		(LT == 'Y') ||
		(LT == 'Z') ||
		// normal
		(LT == 'a') ||
		(LT == 'b') ||
		(LT == 'c') ||
		(LT == 'd') ||
		(LT == 'e') ||
		(LT == 'f') ||
		(LT == 'g') ||
		(LT == 'h') ||
		(LT == 'i') ||
		(LT == 'j') ||
		(LT == 'k') ||
		(LT == 'l') ||
		(LT == 'm') ||
		(LT == 'n') ||
		(LT == 'o') ||
		(LT == 'p') ||
		(LT == 'q') ||
		(LT == 'r') ||
		(LT == 's') ||
		(LT == 't') ||
		(LT == 'u') ||
		(LT == 'v') ||
		(LT == 'w') ||
		(LT == 'x') ||
		(LT == 'y') ||
		(LT == 'z'))
	{
		return TRUE;
	}
	return FALSE;
}

BOOL AppendStringToStringA(LPCTSTR first, LPCTSTR second, TCHAR** string_out)
{
	__try
	{
		if ((first == NULL) || (second == NULL))
			return PTR_IS_NULL;

		if (*string_out != NULL)
			return PTR_ALREADY_REFERENCED;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return PTR_ACCESS_VIOLATION;
	}
	HRESULT hr;
	BOOL result = TRUE;
	size_t str1, str2;

	hr = StringCbLength(first, STRSAFE_MAX_CCH * sizeof(TCHAR), &str1);
	if (SUCCEEDED(hr))
	{
		hr = StringCbLength(second, STRSAFE_MAX_CCH * sizeof(TCHAR), &str2);
		if (SUCCEEDED(hr))
		{
			(*string_out) = new TCHAR[(str1 + str2 + sizeof(TCHAR))];
			if (*string_out != NULL)
			{
				hr = StringCbPrintf(*string_out, (str1 + str2 + sizeof(TCHAR)), L"%s%s", first, second);
				if (FAILED(hr))
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

	return result;
}

BOOL AppendStringToStringW(LPCWSTR first, LPCWSTR second, WCHAR** string_out)
{
	__try
	{
		if ((first == NULL) || (second == NULL))
			return PTR_IS_NULL;

		if (*string_out != NULL)
			return PTR_ALREADY_REFERENCED;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return PTR_ACCESS_VIOLATION;
	}
	HRESULT hr;
	BOOL result = TRUE;
	size_t str1, str2;

	hr = StringCbLength(first, STRSAFE_MAX_CCH * sizeof(TCHAR), &str1);
	if (SUCCEEDED(hr))
	{
		hr = StringCbLength(second, STRSAFE_MAX_CCH * sizeof(TCHAR), &str2);
		if (SUCCEEDED(hr))
		{
			(*string_out) = new TCHAR[(str1 + str2 + sizeof(TCHAR))];
			if (*string_out != NULL)
			{
				hr = StringCbPrintf(*string_out, (str1 + str2 + sizeof(TCHAR)), L"%s%s", first, second);
				if (FAILED(hr))
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

	return result;
}

BOOL CopyStringToPtrW(LPCWSTR source_in, WCHAR ** destination_out)
{
	__try
	{
		if (source_in == NULL)return PTR_IS_NULL;

		if (*destination_out != NULL)
			return PTR_ALREADY_REFERENCED;

		if (destination_out == NULL)
			return PTR_DEST_INVALID;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return PTR_ACCESS_VIOLATION;
	}
	HRESULT hr;
	size_t len;

	hr = StringCbLength(source_in, STRSAFE_MAX_CCH*sizeof(TCHAR), &len);
	if (SUCCEEDED(hr))
	{
		len += sizeof(WCHAR);

		*destination_out = new WCHAR[len];

		hr = (*destination_out != NULL) ? S_OK : E_OUTOFMEMORY;
		if (SUCCEEDED(hr))
		{
			hr = StringCbCopy(*destination_out, len, source_in);
		}
	}
	return SUCCEEDED(hr) ? TRUE : FALSE;
}

BOOL CopyStringToPtrA(LPCTSTR source_in, TCHAR ** destination_out)
{
	__try
	{
		if (source_in == NULL)return PTR_IS_NULL;

		if (*destination_out != NULL)
			return PTR_ALREADY_REFERENCED;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return PTR_ACCESS_VIOLATION;
	}
	HRESULT hr;
	size_t len;

	hr = StringCbLength(source_in, STRSAFE_MAX_CCH*sizeof(TCHAR), &len);
	if (SUCCEEDED(hr))
	{
		len += sizeof(TCHAR);

		*destination_out = new TCHAR[len];

		hr = (*destination_out != NULL) ? S_OK : E_OUTOFMEMORY;
		if (SUCCEEDED(hr))
		{
			hr = StringCbCopy(*destination_out, len, source_in);
		}
	}
	return SUCCEEDED(hr) ? TRUE : FALSE;
}

int GetIndexOfCharForSort(TCHAR CHR_in)
{
	switch (CHR_in)
	{

	case L'0':
		return 0;
	case L'1':
		return 1;
	case L'2':
		return 2;
	case L'3':
		return 3;
	case L'4':
		return 4;
	case L'5':
		return 5;
	case L'6':
		return 6;
	case L'7':
		return 7;
	case L'8':
		return 8;
	case L'9':
		return 9;
	case L'A':
		return 10;
	case L'a':
		return 11;
	case L'B':
		return 12;
	case L'b':
		return 13;
	case L'C':
		return 14;
	case L'c':
		return 15;
	case L'D':
		return 16;
	case L'd':
		return 17;
	case L'E':
		return 18;
	case L'e':
		return 19;
	case L'F':
		return 20;
	case L'f':
		return 21;
	case L'G':
		return 22;
	case L'g':
		return 23;
	case L'H':
		return 24;
	case L'h':
		return 25;
	case L'I':
		return 26;
	case L'i':
		return 27;
	case L'J':
		return 28;
	case L'j':
		return 29;
	case L'K':
		return 30;
	case L'k':
		return 31;
	case L'L':
		return 32;
	case L'l':
		return 33;
	case L'M':
		return 34;
	case L'm':
		return 35;
	case L'N':
		return 36;
	case L'n':
		return 37;
	case L'O':
		return 38;
	case L'o':
		return 39;
	case L'P':
		return 40;
	case L'p':
		return 41;
	case L'Q':
		return 42;
	case L'q':
		return 43;
	case L'R':
		return 44;
	case L'r':
		return 45;
	case L'S':
		return 46;
	case L's':
		return 47;
	case L'T':
		return 48;
	case L't':
		return 49;
	case L'U':
		return 50;
	case L'u':
		return 51;
	case L'V':
		return 52;
	case L'v':
		return 53;
	case L'W':
		return 54;
	case L'w':
		return 55;
	case L'X':
		return 56;
	case L'x':
		return 57;
	case L'Y':
		return 58;
	case L'y':
		return 59;
	case L'Z':
		return 60;
	case L'z':
		return 61;
	case L'_':
		return 62;
	case L'-':
		return 63;
	case L'!':
		return 64;
	case L'§':
		return 65;
	case L'$':
		return 66;
	case L'%':
		return 67;
	case L'&':
		return 68;
	case L'(':
		return 69;
	case L')':
		return 70;
	case L'=':
		return 71;
	case L'+':
		return 72;
	case L'~':
		return 73;
	case L'#':
		return 74;
	case L';':
		return 75;
	case L'.':
		return 76;
	case L',':
		return 77;
	default:
		return 78;
	}
}

int CompareStringsForSort(LPCTSTR key, LPCTSTR toCompare)
{
	if ((key == NULL) || (toCompare == NULL))
	{
		return EXPT_INVALID_ARGUMENT;
	}
	else
	{
		__try
		{
			int i = 0;
			int key_index = -1;
			int com_index = -1;

			while ((key[i] != L'\0') && (toCompare[i] != L'\0'))
			{
				key_index = GetIndexOfCharForSort(key[i]);
				com_index = GetIndexOfCharForSort(toCompare[i]);

				if (key_index < com_index)
				{
					return 1;
				}
				else if (key_index > com_index)
				{
					return -1;
				}
				else
				{
					i++;
				}
			}
			if ((key[i] == L'\0') || (toCompare[i] == L'\0'))
			{
				if ((key[i] == L'\0') && (toCompare[i] == L'\0'))// equal
				{
					return 0;
				}
				else if ((key[i] == L'\0'))// 'toCompare' contains 'key' -> 'toCompare' should follow 'key' 
				{
					return 1;
				}
				else
				{
					return -1;// 'key' contains 'toCompare' -> 'toCompare' should precede 'key'
				}
			}
		}
		__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			return EXPT_ARRAY_BOUNDS_EXCEEDED;
		}
	}
	return 0;
}

BOOL CompareStringsB(LPCTSTR string1, LPCTSTR string2)
{
	if ((string1 == NULL) || (string2 == NULL))
	{
		return FALSE;
	}
	__try
	{
		int i = 0;

		while (i < MAXXCOMPARE)
		{
			if (string1[i] != string2[i])
			{
				return FALSE;
			}
			else if ((string1[i] == L'\0') && (string2[i] == L'\0'))
			{
				break;
			}
			i++;
		}
		if (i == MAXXCOMPARE)
			return FALSE;

		return TRUE;
	}
	__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return FALSE;
	}
}

BOOL CompareStringsNoCase(LPCTSTR string1, LPCTSTR string2)
{
	if ((string1 == NULL) || (string2 == NULL))
	{
		return FALSE;
	}
	__try
	{
		int i = 0;

		while (i < MAXXCOMPARE)
		{
			if (string1[i] != string2[i])
			{
				if(!CompareCharNoCase(string1[i], string2[i]))
					return FALSE;
			}
			else if ((string1[i] == L'\0') && (string2[i] == L'\0'))
			{
				break;
			}
			i++;
		}
		if (i == MAXXCOMPARE)
			return FALSE;

		return TRUE;
	}
	__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return FALSE;
	}
}

BOOL CompareCharNoCase(TCHAR L1, TCHAR L2)
{
	if (L1 == L2)
		return TRUE;
	else
	{
		if (IsChar(L1) && IsChar(L2))
		{
			auto index_L1 = static_cast<int>(L1);
			auto index_L2 = static_cast<int>(L2);

			if (index_L1 < 91)
			{
				if ((index_L1 + 32) == index_L2)
					return TRUE;			
			}
			else if (index_L2 < 91)
			{
				if ((index_L2 + 32) == index_L1)
					return TRUE;
			}
		}
		return FALSE;
	}
}

void showText(LPCWSTR text)
{
	MessageBox(NULL, text, L"Showbox", MB_OK);
}

BOOL GetFilenameOutOfPath(LPCTSTR path, TCHAR ** filename_out, BOOL hideFileExt)
{
	BOOL result = TRUE;

	__try
	{
		__try
		{
			if (path == NULL)
				return PTR_IS_NULL;
			if (*filename_out != NULL)
				return PTR_ALREADY_REFERENCED;
		}
		__except (
			GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
			? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH
			)
		{
			return PTR_ACCESS_VIOLATION;
		}
		int i = 0;
		int j = 0;
		BOOL isExt = FALSE;

		while (path[i] != L'\0')i++;
		j = i;
		while (path[i] != L'\\')
		{
			if (path[i] == L'.')
				isExt = TRUE;
			i--;
		}

		size_t len = ((size_t)(((j - i) + 2)*sizeof(TCHAR)));

		TCHAR* buffer = new TCHAR[ len ];
		result = (buffer != NULL) ? TRUE : FALSE;
		if (result)
		{
			SecureZeroMemory(buffer, len);

			j = 0;
			i += 1;
			while (path[i] != L'\0')
			{
				buffer[j] = path[i];
				i++;
				j++;
			}
			buffer[j] = L'\0';

			if (hideFileExt && isExt)
			{
				j -= 1;

				if (j > 0)
				{

					while (buffer[j] != L'.')
					{
						if (j == 0)
							break;
						buffer[j] = L'\0';
						j--;
					}
					buffer[j] = L'\0';
				}
			}

			HRESULT hr = StringCbLength(buffer, sizeof(TCHAR)*STRSAFE_MAX_CCH, &len);

			result = SUCCEEDED(hr) ? TRUE : FALSE;
			if (result)
			{
				len += sizeof(TCHAR);

				*filename_out = new TCHAR[len];
				result = (*filename_out != NULL) ? TRUE : FALSE;
				if (result)
				{
					hr = StringCbCopy(*filename_out, len, buffer);

					result = SUCCEEDED(hr) ? TRUE : FALSE;
				}
			}
		}
	}
	__except (
		GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH
		)
	{
		return EXPT_ARRAY_BOUNDS_EXCEEDED;
	}
	return result;
}

BOOL AppendStringsWithVaList(TCHAR ** assembledString_out, ...)
{
	__try
	{
		va_list Stringlist, copyStringlist;
		HRESULT hr = S_OK;
		LPTSTR curString;
		size_t aStringSize = 0;
		size_t curSize;

		va_start(Stringlist, assembledString_out);
		va_copy(copyStringlist, Stringlist);

		do
		{
			curString = va_arg(Stringlist, LPTSTR);
			if (curString != NULL)
			{
				hr = StringCbLength(curString, sizeof(TCHAR)*STRSAFE_MAX_CCH, &curSize);
				if (FAILED(hr))
					break;

				aStringSize += curSize;
			}
		} while (curString != NULL);
		va_end(Stringlist);

		if (FAILED(hr))
			return FALSE;
		else
		{
			aStringSize += sizeof(TCHAR);

			*assembledString_out = new TCHAR[aStringSize];
			if (*assembledString_out != NULL)
			{
				BOOL start = TRUE;

				do
				{
					curString = va_arg(copyStringlist, LPTSTR);
					if (curString != NULL)
					{
						if (start)
						{
							hr = StringCbCopy(*assembledString_out, aStringSize, curString);
							if (FAILED(hr))
								break;

							start = FALSE;
						}
						else
						{
							hr = StringCbCat(*assembledString_out, aStringSize, curString);
							if (FAILED(hr))
								break;
						}
					}
				} while (curString != NULL);
				if (FAILED(hr))
				{
					return FALSE;
				}
				va_end(copyStringlist);
			}
			else
				return FALSE;
		}
	}
	__except (
		GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH
		)
	{
		return FALSE;
	}
	return TRUE;
}

COLORREF InvertColor(COLORREF toInvert)
{
	BYTE red = GetRValue(toInvert);
	BYTE green = GetGValue(toInvert);
	BYTE blue = GetBValue(toInvert);

	return RGB(~red, ~green, ~blue);
}

BOOL IsPtrValid(LPVOID ptr)
{
	BOOL result;

	__try
	{
		result = (ptr != NULL) ? TRUE : FALSE;
		return result;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return FALSE;
	}
}

BOOL AddToolipToControl(HWND ControlWnd, HWND Parent, PTSTR text, HINSTANCE hInst, HWND* tipWnd_out)
{
	BOOL result = TRUE;

	if ((ControlWnd == NULL) || (text == NULL))
	{
		result = FALSE;
	}
	if (result)
	{
		HWND tip = CreateWindowEx(
			0,
			TOOLTIPS_CLASS,
			NULL,
			WS_POPUP | TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			Parent, NULL, hInst, NULL);

		result = (tip != NULL) ? TRUE : FALSE;
		if (result)
		{
			if (tipWnd_out != NULL)
				(*tipWnd_out) = tip;

			TOOLINFO ti = { 0 };
			ti.cbSize = sizeof(ti);
			ti.hwnd = Parent;
			ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
			ti.uId = (UINT_PTR)ControlWnd;
			ti.lpszText = text;

			SendMessage(tip, TTM_ADDTOOL, 0, (LPARAM)&ti);
		}
	}
	return result;
}

BOOL ChangeTooltipText(HWND tooltip, HWND ControlWnd, HWND Parent, PTSTR text, HINSTANCE hInst)
{
	if ((ControlWnd == NULL) || (Parent == NULL) || (text == NULL) || (hInst == NULL))
		return FALSE;
	else
	{
		TOOLINFO ti = { 0 };
		ti.cbSize = sizeof(ti);
		ti.hwnd = Parent;
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		ti.uId = (UINT_PTR)ControlWnd;
		ti.lpszText = text;
		ti.hinst = hInst;

		SendMessage(tooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);

		return TRUE;
	}
}

BOOL ConvertWCHARtoCHAR(const WCHAR * toConvert, char ** converted_out)
{
	__try
	{
		__try
		{
			DWORD cbData = WideCharToMultiByte(CP_UTF8, 0, toConvert, -1, NULL, 0, NULL, NULL);

			BOOL result = (cbData == 0) ? FALSE : TRUE;
			if (result)
			{
				*converted_out = new (std::nothrow) CHAR[cbData];

				result = (*converted_out != NULL) ? TRUE : FALSE;
				if (result)
				{
					result = WideCharToMultiByte(CP_UTF8, 0, toConvert, -1, *converted_out, cbData, NULL, NULL)
						? TRUE
						: FALSE;
					if (result)
					{
						// ...
					}
				}
			}
			return result;
		}
		__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED
			? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			return FALSE;
		}
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return FALSE;
	}
}

BOOL ConvertCHARtoWCHAR(const char * toConvert, WCHAR ** converted_out)
{
	__try
	{
		__try
		{
			if (toConvert == NULL)
				return PTR_IS_NULL;
			if (*converted_out != NULL)
				return PTR_ALREADY_REFERENCED;
		}
		__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
			? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			return PTR_ACCESS_VIOLATION;
		}

		BOOL result;
		size_t len_multib = strlen(toConvert);

		DWORD cbData = MultiByteToWideChar(CP_UTF8, 0, toConvert, -1, NULL, 0);

		result = (cbData > 0) ? TRUE : FALSE;
		if (result)
		{
			*converted_out = new WCHAR[cbData];

			result = (*converted_out == NULL) ? FALSE : TRUE;
			if (result)
			{
				SecureZeroMemory(*converted_out, sizeof(WCHAR)* cbData);

				result = (MultiByteToWideChar(CP_UTF8, 0, toConvert, (int)len_multib, *converted_out, cbData) > 0)
					? TRUE : FALSE;
			}
		}
		return result;
	}
	__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return EXPT_ARRAY_BOUNDS_EXCEEDED;
	}
}

int GetCharCount(TCHAR *string)
{
	size_t len;

	HRESULT hr = StringCbLength(string, sizeof(TCHAR) *STRSAFE_MAX_CCH, &len);
	if (SUCCEEDED(hr))
	{
		return (int)(len / sizeof(TCHAR));
	}
	else
		return -1;
}

LONG ConvertYPixToTwips(HWND hWnd, int Ypix)
{	
	HDC hdc = GetDC(hWnd);
	if (hdc)
	{
		double res = 250;
		double yPixelPerInch = (double)GetDeviceCaps(hdc, LOGPIXELSY);

		if (yPixelPerInch != 0)// prevent division by zero
		{
			double _ypix = Ypix;
			res = (1440 / yPixelPerInch) * _ypix;
		}
		ReleaseDC(hWnd, hdc);
		return (LONG)res;
	}
	else
		return -1;
}

int ConvertTwipsToPix(int twips)
{
	HWND dWnd = GetDesktopWindow();
	HDC hdc = GetDC(dWnd);
	if (hdc)
	{
		__try
		{
			double yPixelPerInch = (double)GetDeviceCaps(hdc, LOGPIXELSY);
			double _twips = twips;

			auto res = _twips / (1440 / yPixelPerInch);

			ReleaseDC(dWnd, hdc);

			return (int)res;
		}
		__except (
			GetExceptionCode() == EXCEPTION_FLT_DIVIDE_BY_ZERO
			? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH
			)
		{
			return (int)(twips / 15);
		}
	}
	return 0;
}

BOOL GetRichEditContent(HWND edit, TCHAR ** text_out)
{
	__try
	{
		BOOL result = TRUE;

		if (edit == NULL)
			return PTR_IS_NULL;
		else
		{
			if (*text_out != NULL)
				return PTR_ALREADY_REFERENCED;
			else
			{
				GETTEXTLENGTHEX gtlx;
				gtlx.codepage = 1200;
				gtlx.flags = GTL_DEFAULT;

				auto len = SendMessage(
					edit,
					EM_GETTEXTLENGTHEX,
					reinterpret_cast<WPARAM>(&gtlx),
					0
				);

				*text_out = new TCHAR[len + 1];
				if (*text_out != NULL)
				{
					GETTEXTEX gtx;
					gtx.codepage = 1200;
					gtx.flags = GT_DEFAULT;
					gtx.cb = (DWORD)(sizeof(TCHAR) * (len + 1));
					gtx.lpDefaultChar = NULL;
					gtx.lpUsedDefChar = NULL;

					SendMessage(
						edit,
						EM_GETTEXTEX,
						reinterpret_cast<WPARAM>(&gtx),
						reinterpret_cast<LPARAM>(*text_out)
					);					
				}
				else
					result = FALSE;
			}
		}
		return result;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return PTR_ACCESS_VIOLATION;
	}
}

BOOL SetRichEditContent(HWND edit, LPCTSTR text, DWORD flags)
{
	if ((edit == nullptr)
		|| (text == nullptr))
		return FALSE;
	else
	{
		SETTEXTEX stx;
		stx.codepage = 1200;
		stx.flags = flags;

		return (BOOL)SendMessage(
			edit,
			EM_SETTEXTEX,
			reinterpret_cast<WPARAM>(&stx),
			reinterpret_cast<LPARAM>(text));
	}
}

BOOL GetRichEditSelectionContent(HWND edit, TCHAR ** text_out)
{
	__try
	{
		BOOL result = TRUE;

		if (edit == NULL)
			return PTR_IS_NULL;
		else
		{
			if (text_out == NULL)
				return PTR_DEST_INVALID;
			else
			{
				if (*text_out != NULL)
					return PTR_ALREADY_REFERENCED;
				else
				{
					CHARRANGE cr;

					SendMessage(
						edit,
						EM_EXGETSEL,
						0,
						reinterpret_cast<WPARAM>(&cr)
					);

					int len = cr.cpMax - cr.cpMin;
					if (len > 0)
					{
						*text_out = new TCHAR[len + 1];
						if (*text_out != NULL)
						{
							GETTEXTEX gtx;
							gtx.codepage = 1200;
							gtx.flags = GT_SELECTION;
							gtx.cb = (sizeof(TCHAR) * (len + 1));
							gtx.lpDefaultChar = NULL;
							gtx.lpUsedDefChar = NULL;

							SendMessage(
								edit,
								EM_GETTEXTEX,
								reinterpret_cast<WPARAM>(&gtx),
								reinterpret_cast<LPARAM>(*text_out)
							);
						}
						else
							result = FALSE;
					}
					else
						result = FALSE;
				}
			}
		}
		return result;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return PTR_ACCESS_VIOLATION;
	}
}

void DisplaySingleChar(TCHAR c)
{
	TCHAR temp[50] = { 0 };
	StringCbPrintf(temp, sizeof(temp), L"Char is > %c", c);
	MessageBox(NULL, temp, L"Display character", MB_OK);
}

int TranslateLastError(TCHAR ** msg_out)
{
	LPTSTR lpBuffer;
	DWORD err = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpBuffer, 0, nullptr);

	CopyStringToPtr(lpBuffer, msg_out);
	HeapFree(
		GetProcessHeap(), 0, lpBuffer);
	return err;
}

void TranslateError(HRESULT err, TCHAR ** msg_out)
{
	LPTSTR lpBuffer;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpBuffer, 0, nullptr);

	CopyStringToPtr(lpBuffer, msg_out);

	HeapFree(
		GetProcessHeap(), 0, lpBuffer);
}

void TextOutDC(HDC hdc, LPCTSTR text, int x, int y, HFONT font, COLORREF textcolor)
{
	int c;
	BOOL deleteFont = FALSE;

	if (text == nullptr)return;
	else
	{
		size_t len;
		if (FAILED(
				StringCbLength(text, sizeof(TCHAR)* STRSAFE_MAX_CCH, &len)))return;
		else
		{
			c = (int)(len / sizeof(TCHAR));
		}
	}

	HFONT t_font;
	if (font != nullptr)t_font = font;
	else
	{
		t_font = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Segoe UI\0");
		if (t_font)
			deleteFont = TRUE;
	}
	if (t_font != nullptr)
	{
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, textcolor);

		HGDIOBJ original = SelectObject(hdc, t_font);

		TextOut(hdc, x, y, text, c);

		SelectObject(hdc, original);

		if (deleteFont)
			DeleteObject(t_font);
	}
}

void TextOutWnd(HWND hWnd, LPTSTR text, int x, int y, HFONT font, COLORREF textcolor)
{
	HDC hdc = GetDC(hWnd);
	if (hdc)
	{
		TextOutDC(hdc, text, x, y, font, textcolor);
		ReleaseDC(hWnd, hdc);
	}
}

BOOL getTextSizeDC(HDC hdc, HFONT font, LPCTSTR string, LPSIZE pSize)
{
	BOOL result = FALSE;

	if (hdc && font && pSize)
	{
		int len = _lengthOfString(string);
		if (len > 0)
		{
			HGDIOBJ original = SelectObject(hdc, font);

			if (GetTextExtentPoint32(hdc, string, len, pSize))
			{
				result = TRUE;
			}
			SelectObject(hdc, original);
		}
	}
	return result;
}

void initCharformat(CHARFORMAT * cf, int height, COLORREF Textcolor, LPTSTR fontFamily)
{
	cf->cbSize = sizeof(CHARFORMAT);
	cf->dwMask = CFM_ALL | CFM_COLOR;
	cf->yHeight = ConvertYPixToTwips(
		GetDesktopWindow(),
		height
	);
	cf->crTextColor = Textcolor;
	cf->bCharSet = ANSI_CHARSET;
	StringCbCopy(cf->szFaceName, sizeof(TCHAR)*LF_FACESIZE, fontFamily);
	cf->yOffset = 0;
	cf->dwEffects = 0;
}

int getSystemLanguage()
{
	auto langId = GetUserDefaultLangID();
	auto primLangId = PRIMARYLANGID(langId);
	return primLangId;
}

int GetRunningThreadCountFromSpecificProcess(_In_ DWORD processID)
{
	int result = -1;
	HANDLE threadSnapshot = INVALID_HANDLE_VALUE;

	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);

	threadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (threadSnapshot != INVALID_HANDLE_VALUE)
	{
		if (!Thread32First(threadSnapshot, &te32))
			result = 0;
		else
		{
			result = 1;

			do
			{
				if (te32.th32OwnerProcessID == processID)
				{
					result++;
				}

			} while (Thread32Next(threadSnapshot, &te32));
		}
		CloseHandle(threadSnapshot);
	}
	return result;
}

BOOL GetIdentifierListOfActiveThreadsForProcess(_In_ DWORD processID, _Inout_ DWORD** list_out)
{
	if (*list_out != nullptr)
		return FALSE;
	else
	{
		BOOL result = TRUE;
		HANDLE threadSnapshot = INVALID_HANDLE_VALUE;

		THREADENTRY32 te32;
		te32.dwSize = sizeof(THREADENTRY32);

		threadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (threadSnapshot != INVALID_HANDLE_VALUE)
		{
			int cnt = 0;

			for (int i = 0; i < 2; i++)
			{
				if (i == 1)
				{
					if (cnt > 0)
					{
						*list_out = new DWORD[cnt];
						if (*list_out == nullptr)
						{
							result = FALSE;
							break;
						}
						result = cnt;
						cnt = 0;
					}
					else
					{
						result = FALSE;
						break;
					}
				}

				if (!Thread32First(threadSnapshot, &te32))
				{
					result = FALSE;
					break;
				}
				else
				{
					do
					{
						if (te32.th32OwnerProcessID == processID)
						{
							if (i == 0)
							{
								cnt++;
							}
							else
							{
								*list_out[cnt] = te32.th32ThreadID;
								cnt++;
							}
						}

					} while (Thread32Next(threadSnapshot, &te32));
				}
			}
			CloseHandle(threadSnapshot);
		}
		return result;
	}
}

int countLinefeed(LPCTSTR lpszbuffer)
{
	__try
	{
		if (lpszbuffer == nullptr)
			return -1;

		int linefeed = 0;
		int counter = 0;

		while (lpszbuffer[counter] != L'\0')
		{
			if (static_cast<TCHAR>(0x0A) == lpszbuffer[counter])
				linefeed++;

			counter++;
		}
		return linefeed;
	}
	__except (
		GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return -2;
	}
}


int _lengthOfString(LPCTSTR str)
{
	if (str != nullptr)
	{
		size_t s; 
		HRESULT hr = StringCbLength(str, STRSAFE_MAX_CCH, &s);
		if (SUCCEEDED(hr))
		{
			return (int)(s / sizeof(TCHAR));
		}
	}
	return 0;
}

BOOL _cutString(LPCTSTR _string, int indexLow, int indexHigh, TCHAR ** newbuffer_out)
{
	if ((_string == nullptr)
		|| (*newbuffer_out != nullptr))
		return FALSE;
	else
	{
		if ((indexLow < 0)
			|| (indexHigh < 1))
			return FALSE;
		else
		{
			int bufSize = indexHigh - indexLow;
			int maxString = _lengthOfString(_string);

			if ((indexHigh < indexLow)
				|| (bufSize < 0)
				|| (indexHigh > maxString))
				return FALSE;
			else
			{
				*newbuffer_out = new TCHAR[bufSize + 1];
				if (*newbuffer_out != nullptr)
				{
					int r = 0;
					SecureZeroMemory(*newbuffer_out, sizeof(TCHAR)*(bufSize + 1));

					__try // now you overact - this could not happen at all??
					{
						for (int i = 0; i < bufSize; i++)
						{
							if ((i >= indexLow) && (i <= indexHigh))
							{
								(*newbuffer_out)[r] = _string[i];
								r++;
							}
							if (i > indexHigh)
								break;
						}
					}
					__except (
						GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
						? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
					{
						return FALSE;
					}
					return TRUE;
				}
				else
					return FALSE;
			}
		}
	}
}

BOOL _getlineFromCharIndexOutOfText(LPCTSTR text, int charIndex, TCHAR ** line_out)
{
	if ((text == nullptr) ||
		(*line_out != nullptr))
		return FALSE;
	else
	{
		auto max_text = _lengthOfString(text);

		if ((charIndex < 0) ||
			(max_text <= 0) ||
			(charIndex > max_text) )
			return FALSE;
		else
		{
			__try
			{
				// if the index is exactly on a carriage return -> decrease
				if (text[charIndex] == 0x0D)
					charIndex--;

				if (charIndex < 0)
					return FALSE;

				// first count the required buffersize
				int counter = charIndex;
				int bufSize = 0;
				int low_end = charIndex;

				// count in negative direction
				while (text[counter] != 0x0D)
				{

					bufSize++;
					counter--;

					if (counter == 0)
						break;
				}
				if (counter > 0)
					low_end = counter + 1;// save startpoint

				counter = charIndex;

				// then count in positive direction
				while (text[counter] != 0x0D)
				{
					if (counter == max_text)
						break;

					counter++;
					bufSize++;
				}

				// counter is now the max-position
				if (text[counter] == 0x0D)
					counter--;

				if (bufSize == 0)
					return FALSE;
				else
				{
					*line_out = new TCHAR[bufSize + 1];
					if (*line_out != nullptr)
					{
						SecureZeroMemory(*line_out, sizeof(TCHAR)*(bufSize + 1));

						int saver = 0;

						while (saver <= bufSize)
						{
							(*line_out)[saver] = text[low_end];

							low_end++;
							saver++;

							if (low_end > counter)
								break;
						}
						return TRUE;
					}
					else
						return FALSE;
				}
			}
			__except (
				GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
				? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
			{
				return FALSE;
			}
		}
	}
}

int _dirContentListing(LPCTSTR dir_path, LPDIRLIST *list_out, DWORD dwFlags)
{
	if ((dir_path == nullptr)
		|| (*list_out != nullptr))
		return -1;
	else
	{
		int result = -1;
		int lBuffer = 0;

		// insert the wildcards
		int l = _lengthOfString(dir_path);
		l += 15;
		TCHAR* wc_Path = new TCHAR[l];

		if (wc_Path != nullptr)
		{
			SecureZeroMemory(wc_Path, sizeof(TCHAR)*l);

			if (
				SUCCEEDED(
					StringCbPrintf(wc_Path, sizeof(TCHAR)*l, L"\\\\?\\%s\\*", dir_path))
				)
			{
				// execute two times: 1->count / 2->record
				for (int s = 0; s < 2; s++)
				{
					if (s)
					{
						*list_out = new DIRLIST[lBuffer];
						if (*list_out == nullptr)
							return -9;
					}

					HANDLE hFind = INVALID_HANDLE_VALUE;
					WIN32_FIND_DATA wfd;
					int counter = 0;

					hFind = FindFirstFile(wc_Path, &wfd);
					if (hFind != INVALID_HANDLE_VALUE)
					{
						while (wfd.cFileName[0] == '.')
						{
							if (FindNextFile(hFind, &wfd) == 0)
							{
								// empty directory...!
								break;
							}
						}

						do
						{
							if ((dwFlags & INCLUDE_FILES) && (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) ||
								((dwFlags & INCLUDE_FOLDERS) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
							{
								if (s)// second run (record)
								{
									if (*list_out != nullptr)
									{
										(*list_out)[counter].fileAttributes = wfd.dwFileAttributes;

										auto hr = StringCbCopy((*list_out)[counter].filename, sizeof(TCHAR) * 256, wfd.cFileName);
										if (FAILED(hr))
										{
											result = -8;
											break;
										}
									}
									else
									{
										result = -7;
										break;
									}
								}
								else// first run (count)
								{
									lBuffer++;
								}
								counter++;
							}

						} while (FindNextFile(hFind, &wfd) != 0);

						result = lBuffer;

						FindClose(hFind);
					}
					else
					{
						result = -2;
						break;
					}
				}
			}
			else
				result = -4;
		}
		else
			result = -5;

		return result;
	}
}

bool isPointInRect(int x, int y, LPRECT prc)
{
	return ((x > prc->left) && (x < prc->right) && (y > prc->top) && (y < prc->bottom));
}

bool CompareRect(LPRECT rc1, LPRECT rc2)
{
	if (
		(rc1->bottom == rc2->bottom)
		&& (rc1->left == rc2->left)
		&& (rc1->right == rc2->right)
		&& (rc1->top == rc2->top)
		)
	{
		return true;
	}
	else
		return false;
}

HICON LoadIconEx(HINSTANCE hInst, int iconID, int squareSize)
{
	return (HICON)LoadImage(
		hInst,
		MAKEINTRESOURCE(iconID),
		IMAGE_ICON,
		DPIScale(squareSize),
		DPIScale(squareSize),
		LR_DEFAULTCOLOR
	);
}

COLORREF makeSelectionColor(COLORREF nCol)
{
	auto red = GetRValue(nCol);
	auto green = GetGValue(nCol);
	auto blue = GetBValue(nCol);

	if (red > 150)
		red -= 20;
	else
		red += 20;

	if (green > 150)
		green -= 20;
	else
		green += 20;

	if (blue > 150)
		blue -= 20;
	else
		blue += 20;

	return RGB(red, green, blue);
}

COLORREF makePressedColor(COLORREF nCol)
{
	auto red = GetRValue(nCol);
	auto green = GetGValue(nCol);
	auto blue = GetBValue(nCol);

	if (red > 150)
		red -= 50;
	else
		red += 50;

	if (green > 150)
		green -= 50;
	else
		green += 50;

	if (blue > 150)
		blue -= 50;
	else
		blue += 50;

	return RGB(red, green, blue);
}

COLORREF getDisabledColor(COLORREF bkgnd)
{
	auto r = GetRValue(bkgnd);
	auto g = GetGValue(bkgnd);
	auto b = GetBValue(bkgnd);

	// TODO!!!

	if (r > 100 && r < 156)
	{
		if (r < 128)
			r = 156;
		else
			r = 100;
	}
	else
		r = 128;

	if (g > 100 && g < 156)
	{
		if (g < 128)
			g = 156;
		else
			g = 100;
	}
	else
		g = 128;

	if (b > 100 && b < 156)
	{
		if (b < 128)
			b = 156;
		else
			b = 100;
	}
	else
		b = 128;

	int midVal = (r + g + b) / 3;

	return RGB((BYTE)midVal, (BYTE)midVal, (BYTE)midVal);
}

COLORREF getSlightlyDisabledTextColor(COLORREF txtCol)
{
	auto r = GetRValue(txtCol);

	if (r > 128)
		r -= 50;
	else
		r += 80;

	return RGB(r,r,r);
}

BOOL AsureWindowVisibilityFromPoint(LPPOINT ppt, int cx, int cy)
{
	BOOL res = TRUE;
	auto sysCX = GetSystemMetrics(SM_CXSCREEN);
	auto sysCY = GetSystemMetrics(SM_CYSCREEN);

	if ((ppt->x + cx) > sysCX)
	{
		ppt->x -= cx;
		res = FALSE;
	}
	if ((ppt->y + cy) > sysCY)
	{
		ppt->y -= cy;
		res = FALSE;
	}
	return res;
}

BOOL CheckForFileExist(LPCTSTR path)
{
	if (path != nullptr)
	{
		HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
		else
		{
			CloseHandle(hFile);
			return TRUE;
		}
	}
	else
		return FALSE;
}

COLORREF darkenColor(COLORREF color, int percentValue)
{
	if (percentValue >= 0)
	{
		double r = (double)GetRValue(color);
		double g = (double)GetGValue(color);
		double b = (double)GetBValue(color);
		double perc = (double)percentValue;

		double rd = r - ((r / 100) * perc);
		double gd = g - ((g / 100) * perc);
		double bd = b - ((b / 100) * perc);

		if (rd < 0)rd = 0;
		if (gd < 0)gd = 0;
		if (bd < 0)bd = 0;

		return RGB((BYTE)rd, (BYTE)gd, (BYTE)bd);
	}
	return 0;
}

COLORREF brightenColor(COLORREF color, int percentValue)
{
	if (percentValue >= 0)
	{
		double r = (double)GetRValue(color);
		double g = (double)GetGValue(color);
		double b = (double)GetBValue(color);
		double perc = (double)percentValue;

		double rd, gd, bd;

		// if the color is to dark the calculation will have no effect
		// -> so adapt the values!
		if (r < 20)
			r = 50.0;
		if (g < 20)
			g = 50.0;
		if (b < 20)
			b = 50.0;

		rd = r + ((r / 100.0) * perc);
		gd = g + ((g / 100.0) * perc);
		bd = b + ((b / 100.0) * perc);

		if (rd > 255)rd = 255;
		if (gd > 255)gd = 255;
		if (bd > 255)bd = 255;

		return RGB((BYTE)rd, (BYTE)gd, (BYTE)bd);
	}
	return 0;
}

BOOL isFileA(LPCTSTR path)
{
	if (path != nullptr)
	{
		auto attributes =
			GetFileAttributes(path);

		if (attributes != INVALID_FILE_ATTRIBUTES)
		{
			if (attributes & FILE_ATTRIBUTE_DIRECTORY)
				return FALSE;
			else
				return TRUE;
		}
		else
		{
			return -2;
		}
	}
	else
		return -1;
}

BOOL isFileW(LPCTSTR path)
{
	int len = _lengthOfString(path);
	if (len > 0)
	{
		BOOL result = -4;
		len += 8;
		TCHAR* unicodePath = nullptr;

		unicodePath = new TCHAR[len];
		if (unicodePath != nullptr)
		{
			auto hr = StringCbPrintf(unicodePath, len*sizeof(TCHAR), L"\\\\?\\%s", path);
			if (SUCCEEDED(hr))
			{
				result = isFileA(unicodePath);
			}
			else
				result = -5;

			SafeDeleteArray((void**)&unicodePath);
		}
		return result;
	}
	else
		return -3;
}

int CreateRandomInteger(int min, int max)
{
	int range = (max - min) + 1;

	srand((unsigned)time(NULL));

	return min + int(range*rand()/(RAND_MAX + 1.0));
}

BOOL isDrive(LPCTSTR path)
{
	TCHAR drive[] = L"?:\\";

	for (TCHAR i = 65; i < 91; i++)
	{
		drive[0] = i;

		if (CompareStringsAsmW(path, drive))
			return TRUE;
	}
	return FALSE;
}

BOOL setFileAccessTime(LPCTSTR path)
{
	BOOL result;
	HANDLE hFile;
	FILETIME fTime;
	SYSTEMTIME sTime;

	result =
		(path != nullptr)
		? TRUE : FALSE;

	if(result)
	{
		GetSystemTime(&sTime);
		SystemTimeToFileTime(&sTime, &fTime);

		hFile = CreateFile(
			path,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);
		result =
			(hFile != INVALID_HANDLE_VALUE)
			? TRUE : FALSE;
		if (result)
		{
			result =
				SetFileTime(
					hFile,
					nullptr,
					&fTime,
					nullptr
				);

			CloseHandle(hFile);
		}
	}
	return result;
}

LARGE_INTEGER getFileSizeX(LPCTSTR path)
{
	LARGE_INTEGER li;
	HANDLE hFile;

	SecureZeroMemory(&li, sizeof(LARGE_INTEGER));

	hFile = CreateFile(
		path,
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		GetFileSizeEx(hFile, &li);

		CloseHandle(hFile);
	}
	return li;
}
