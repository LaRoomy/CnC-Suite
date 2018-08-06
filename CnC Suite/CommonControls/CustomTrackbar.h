#pragma once
#include<StringClass.h>
#include<ctrlHelper.h>
#include"..\external.h"

class trackBarEventSink {
public:
	virtual void trackBar_TrackStarted(cObject sender, int trackPosition) = 0;
	virtual void __fastcall trackBar_Tracking(cObject sender, int trackPosition) = 0;
	virtual void trackBar_TrackEnd(cObject sender, int trackPosition) = 0;
};

class customTrackbar
{
public:
	customTrackbar();
	~customTrackbar();

	HRESULT create(HINSTANCE hInst, HWND parent, LPPOINT pos, LPSIZE sz, DWORD controlID);

	void Release() { delete this; }

	void setEventHandler(trackBarEventSink* eventSink);
	void setTrackRange(int lowerMin, int upperMax);
	void setTrackPosition(int pos);
	void setText(const iString& text);
	void setColors(COLORREF background, COLORREF bar, COLORREF text);
	void setTrackbuttonImages(int normal, int tracking, int squareSize);
	void setType(int type);
	void setBarAppearence(int barThickness, bool drawMiddleMarker, bool drawScale, int scaleStep);
	void setBorder(bool drawBorder);
	void setFont(HFONT font);

	int getTrackPosition() { return this->trackPosition; }
	int getCtrlID() { return this->ctrlID; }

	void UpdateTrackbar();

	static const TCHAR* customTrackbarClass;

	static const int TRACKBARTYPE_HORIZONTAL = 0x01;
	static const int TRACKBARTYPE_VERTICAL = 0x02;

private:
	HINSTANCE hInstance;
	HWND _trackbar;
	HWND _parent;

	iString _text;
	trackBarEventSink* _eventSink;

	DWORD ctrlID;

	COLORREF backgroundColor;
	COLORREF barColor;
	COLORREF textColor;

	int normalTrackbuttonImageResId;
	int trackingTrackbuttonImageResId;
	int imageSquareSize;
	int trackbarThickness;
	int fontHeight;

	HICON normalImage;
	HICON trackingImage;

	HFONT controlFont;

	int trackRange;
	int trackMax;
	int trackMin;
	int trackPosition;
	int trackbarType;

	int cursorOffset;

	bool isTracking;
	bool _drawBorder;

	HRESULT _registerTrackbarClass();

	static LRESULT CALLBACK trackbarProc(HWND, UINT, WPARAM, LPARAM);

	LRESULT onPaint(HWND);
	LRESULT onMousemove(LPARAM);
	LRESULT onLButtondown(LPARAM);
	LRESULT onLButtonup(LPARAM);
	LRESULT onMouseLeave();

	int _getAbsoluteTrackPosition();

	void setTrackPositionFromCursor(int xyCursor);
};