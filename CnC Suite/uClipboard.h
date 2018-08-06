#pragma once
#include"external.h"
#include"cObject.h"
#include"CommonControls\StringClass.h"

class clipBoardOperations
	: public ObjectRelease<clipBoardOperations>
{
public:
	clipBoardOperations();
	clipBoardOperations(HWND newOwner);
	~clipBoardOperations();

	bool copyTextToClipboard(iString text);

private:
	BOOL success;

};


