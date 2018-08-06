#pragma once
#include"external.h"

typedef UINT DPISCALEFACTOR;

class DPI_Assist
{
public:
	DPI_Assist()
		// intialize scale values
		: scaleFactor(0),
		dpiX(0),
		dpiY(0)
	{
		this->_getScaleFactor();
	}

	int Scale(int val)
	{
		return MulDiv(val, this->scaleFactor, 100);
	}

	UINT GetDPI_X() { return this->dpiX; }
	UINT GetDPI_Y() { return this->dpiY; }

	DPISCALEFACTOR GetScaleFactor() {
		return (DPISCALEFACTOR)this->scaleFactor;
	}

	int GetDpiIndex()
	{
		switch (this->scaleFactor)
		{
		case DPI_Assist::SCALE_100P:
			return 0;
		case DPI_Assist::SCALE_125P:
			return 1;
		case DPI_Assist::SCALE_150P:
			return 2;
		case DPI_Assist::SCALE_175P:
			return 3;
		case DPI_Assist::SCALE_200P:
			return 4;
		default:
			return 0;
		}
	}

	static const DPISCALEFACTOR SCALE_100P = 100;
	static const DPISCALEFACTOR SCALE_125P = 125;
	static const DPISCALEFACTOR SCALE_150P = 150;
	static const DPISCALEFACTOR SCALE_175P = 175;
	static const DPISCALEFACTOR SCALE_200P = 200;

	void Update(){
		this->_getScaleFactor();
	}

	void onDPIchanged(WPARAM wParam)
	{
		this->dpiX = LOWORD(wParam);
		this->dpiY = HIWORD(wParam);
		this->scaleFactor = MulDiv(this->dpiX, 100, 96);
	}

	void Release() { delete this; }

private:
	UINT scaleFactor;
	UINT dpiX;
	UINT dpiY;

	void _getScaleFactor()
	{
		// get scale factor
		HMONITOR hMonitor;
		POINT pt = { 1, 1 };

		hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		if (hMonitor != nullptr)
		{
			HRESULT hr =
				GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &this->dpiX, &this->dpiY);

			if (SUCCEEDED(hr))
			{
				this->scaleFactor = MulDiv(this->dpiX, 100, 96);
			}
		}
	}
};
