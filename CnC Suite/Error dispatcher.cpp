#include "Error dispatcher.h"
#include "HelperF.h"
#include"CommonControls\StringClass.h"

BOOL DispatchEWINotification(int type, LPCTSTR Code, LPCTSTR Description, LPCTSTR Location)
{
	BOOL result;

	HWND Main = FindWindow(L"CNCSUITECLASS\0", NULL);

	result = (Main != NULL) ? TRUE : FALSE;
	if (result)
	{
		EDSPSTRUCT edsp;
		edsp.type = type;
		edsp.Code = Code;
		edsp.Description = Description;
		edsp.Location = Location;

		SendMessage(Main, WM_DISPLAYERROR, 0, reinterpret_cast<LPARAM>(&edsp));
	}
	return result;
}

void DispatchSystemError()
{
	TCHAR* msg = nullptr;

	DWORD err
		= (DWORD)TranslateLastError(&msg);
	auto cd
		= iString::FromInt((int)err);
	iString code(L"SYS \0");
	code += cd;
	SafeRelease(&cd);

	DispatchEWINotification(EDSP_ERROR, code.getContentReference(), msg, L"System");
}

