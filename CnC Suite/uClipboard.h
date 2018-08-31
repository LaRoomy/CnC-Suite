#pragma once
#include"external.h"
#include"cObject.h"
#include"CommonControls\StringClass.h"

class clipBoardOperations
	: public ClsObject<clipBoardOperations>
{
public:
	clipBoardOperations();
	clipBoardOperations(HWND newOwner);
	~clipBoardOperations();

	bool copyTextToClipboard(iString text);

	const wchar_t* ToString() {
		return L"clipboardoperationclass";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

private:
	BOOL success;

};


