#pragma once
#include"..//external.h"
#include<ctrlHelper.h>
#include"StringClass.h"

#define			CCUSTOMCHECKBOXCLASS	L"CNCSCUSTOMCHECKBOXCLASS\0"

#define			DEFAULT_IMAGE_SIZE			16

// set the default-images to a negative value in order to use no image
// otherwise set valid icon ID's (don't forget to adjust the square-size to the appropriate value)
/*
#define			DEFAULT_IMAGE_CHECKED		-1
#define			DEFAULT_IMAGE_UNCHECKED		-2

#define			DEFAULT_IMAGE_CHECKED_DISABLED		-3
#define			DEFAULT_IMAGE_UNCHECKED_DISABLED	-4
*/

#define			DEFAULT_IMAGE_CHECKED		IDI_PROPCHECKBOX_CHECKED
#define			DEFAULT_IMAGE_UNCHECKED		IDI_PROPCHECKBOX_UNCHECKED

#define			DEFAULT_IMAGE_CHECKED_DISABLED		IDI_PROPCHECKBOX_CHECKED_DISABLED
#define			DEFAULT_IMAGE_UNCHECKED_DISABLED	IDI_PROPCHECKBOX_UNCHECKED_DISABLED

#define			CBX_ALIGN_LEFT			10
#define			CBX_ALIGN_CENTER		11
#define			CBX_ALIGN_RIGHT			12

#define			CBX_INTERACTRECT_BOX	13
#define			CBX_INTERACTRECT_ALL	14

typedef struct _CHECKBOXPROPERTY {

	int x;
	int y;
	int width;
	int height;

	int iconID_checked;
	int iconID_unchecked;
	int iconSquareSize;

	int iconID_checked_disabled;
	int iconID_unchecked_disabled;

	iString text;

	int padding;
	int spacing;
	int alignment;

	CTRLID ctrlID;

	COLORREF background;
	COLORREF textcolor;

	HFONT ctrlFont;
	RECT boxRect;

}CHECKBOXPROPERTY, *LPCHECKBOXPROPERTY;

typedef struct _CHECKBOXCONTROL {

	bool isChecked;
	bool isCreated;
	bool useCustomImages;
	bool isEnabled;
	DWORD interactionMode;

}CHECKBOXCONTROL, *LPCHECKBOXCONTROL;

class customCheckboxEventSink
{
public:
	virtual void onCustomCheckboxChecked(cObject sender, bool newState) = 0;
};

class CustomCheckbox : public ClsObject<CustomCheckbox>
{
public:
	CustomCheckbox(HINSTANCE hInst, HWND parent, LPPOINT position, LPSIZE size, int ctrlID);
	~CustomCheckbox();

	void setEventHandler(customCheckboxEventSink* handler);
	void setCustomImages(int cImage_Checked, int cImage_Unchecked, int squareSize);
	void setCustomDisabledImages(int for_checked, int for_unchecked);
	void setColors(COLORREF backgroundcolor, COLORREF textcolor);
	void setAlignment(int align);
	void setText(iString text);
	void setFont(HFONT font);
	void setConstraints(int padding, int spacing);
	void setEnabledState(bool state);
	void selectInteractionRect(DWORD flag);

	HRESULT Create();

	HWND getCtrlHandle() { return this->Checkbox; }
	CTRLID getCtrlID() { return this->cbxProperty.ctrlID; }
	bool isChecked() { return this->cbxControl.isChecked; }
	void setChecked(bool state) { this->cbxControl.isChecked = state; }

	const wchar_t* ToString() {
		return L"checkbox";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

private:
	HINSTANCE hInstance;
	HWND Parent;
	HWND Checkbox;

	CHECKBOXPROPERTY cbxProperty;
	CHECKBOXCONTROL cbxControl;

	customCheckboxEventSink* eventHandler;

	void registerCCboxWndClass();

	static LRESULT CALLBACK checkboxProc(HWND, UINT, WPARAM, LPARAM);

	LRESULT onPaint(HWND);
	LRESULT onLButton(int, int);
};
