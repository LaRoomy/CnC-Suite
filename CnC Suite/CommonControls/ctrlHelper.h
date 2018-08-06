#pragma once
#include<Windows.h>

#ifndef _WIN64
typedef DWORD32 CTRLID;
#else
typedef DWORD64 CTRLID;
#endif

#define		MAKECTRLID(ID)		((CTRLID)ID)
#define		IDTOHMENU(ID)		(reinterpret_cast<HMENU>(MAKECTRLID(ID)))

typedef struct _CTRLCREATIONSTRUCT {

	HWND parent;
	POINT pos;
	SIZE size;
	CTRLID ctrlID;
	HINSTANCE hInst;

}CTRLCREATIONSTRUCT, *LPCTRLCREATIONSTRUCT;

typedef struct _PARTIALBORDER {
	bool left;
	bool right;
	bool top;
	bool bottom;
}PARTIALBORDER, *LPPARTIALBORDER;
