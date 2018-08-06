#pragma once
#include"external.h"
#include"Async.h"


inline void _dispatchTextToStatusbar(int part, LPCTSTR text)
{
	if (text != nullptr)
	{
		STBUPDATE sbu;
		sbu.action = 0;
		sbu.part = part;

		auto hr = StringCbCopy(sbu.text, sizeof(sbu.text), text);
		if (SUCCEEDED(hr))
		{
			_MSG_TO_MAIN(WM_UPDATESTATUSBAR, 0, reinterpret_cast<LPARAM>(&sbu));
		}
	}
}

inline void NormalizeStatusbarPartOne()
{
	_dispatchTextToStatusbar(0, getStringFromResource(UI_STATUSBAR_READY_STATE));
}

inline void NormalizeStatusbarInfoAreaAsync(DWORD delay_ms)
{
	auto async = new Async();
	if (async != nullptr)
	{
		async->setDelay(delay_ms);

		//auto fmt = (async_smpl)&NormalizeStatusbarPartOne;
		async->callFunction(&NormalizeStatusbarPartOne);
	}
}
