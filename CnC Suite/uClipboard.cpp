#include"uClipboard.h"

clipBoardOperations::clipBoardOperations()
	:success(FALSE)
{
	this->success = OpenClipboard(nullptr);
	if (this->success)
	{
		this->success = EmptyClipboard();
	}
}

clipBoardOperations::clipBoardOperations(HWND newOwner)
{
	this->success = OpenClipboard(newOwner);
	if (this->success)
	{
		this->success = EmptyClipboard();
	}
}

clipBoardOperations::~clipBoardOperations()
{
	CloseClipboard();
}

bool clipBoardOperations::copyTextToClipboard(iString text)
{
	if (!this->success)return false;
	else
	{
		bool result = false;
		HGLOBAL hglb;

		hglb = GlobalAlloc(GMEM_MOVEABLE, text.GetSize() + sizeof(TCHAR));
		if (hglb != nullptr)
		{
			LPTSTR lptstrCopy = reinterpret_cast<LPTSTR>(GlobalLock(hglb));
			if (lptstrCopy != nullptr)
			{
				memcpy(lptstrCopy, text.GetData(), text.GetSize() + sizeof(TCHAR));
				lptstrCopy[text.GetLength()] = L'\0';
			}
			GlobalUnlock(hglb);

			return SetClipboardData(CF_UNICODETEXT, hglb) ? true : false;
		}
		return result;
	}
}
