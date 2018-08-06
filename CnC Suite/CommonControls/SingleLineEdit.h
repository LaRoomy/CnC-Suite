#pragma once
#include"..//external.h"
#include<ctrlHelper.h>
#include"StringClass.h"

#define		SLE_CTRLCLASS				L"SLECTRLCLASS\0"

#define		SLE_TYPE_NODESCRIPTION		1
#define		SLE_TYPE_WITHDESCRIPTION	2

#define		DESC_ALIGN_LEFT				3
#define		DESC_ALIGN_RIGHT			4
#define		DESC_ALIGN_CENTER			5

#define		SLE_RESTRICTTYPE_ONLYNUMBERS	10
#define		SLE_RESTRICTTYPE_FORFILENAMES	11

#define		SLE_CURSORTOTEXTEND			-1

#define		SLE_EX_CHECK_VALIDRANGE				0x01	// TODO: implement
#define		SLE_EX_NOEVENT_ON_INVALIDCONTENT	0x02	// TODO: implement
#define		SLE_EX_SETNEGATIVEVALUEPERMISSION	0x04	// TODO: implement

typedef struct _SINGLELINEEDITPROPERTIES {

	int xPos;
	int yPos;
	int cx;
	int cy;
	int textHeight;
	int descriptionWidth;
	int spacing;
	DWORD type;
	DWORD restrictType;
	DWORD alignment;
	DWORD editStyles;

	COLORREF textcolor;
	COLORREF editTextcolor;
	COLORREF framebackground;
	COLORREF editboxbackground;
	COLORREF borderColor;
	COLORREF errorFillColor;
	COLORREF errorBorderColor;

	bool drawBorder;
	bool contentInvalid;
	bool acceptTabs;

	CTRLID ctrlID;
	POINT editboxPosition;
	SIZE editboxSize;

	int max_range;
	int min_range;

	DWORD exFlags;

	HFONT ctrlFont;

}SINGLELINEEDITPROPERTIES, *LPSINGLELINEEDITPROPERTIES;

class singleLineEditEventSink
{
public:
	virtual void onEditboxContentChanged(cObject sender, CTRLID ctrlID) = 0;
	virtual void onTabKeyWasPressed(cObject sender, CTRLID ctrlID) = 0;
};

class singleLineEdit
{
public:
	singleLineEdit(HINSTANCE);
	~singleLineEdit();

	void setEventHandler(singleLineEditEventSink* handler);
	void setDimensions(int x, int y, int cx);
	void setType(DWORD type, int descriptionWidth);
	void setEditFontProperties(iString fontFamily, int textheight, int correctedControlHeight);
	void setCtrlFont(HFONT font);
	void setBorder(bool draw, COLORREF borderColor);
	void setColors(COLORREF framebackground, COLORREF editBackground, COLORREF textcolor, COLORREF editTextcolor);
	void setErrorColors(COLORREF errorFillColor, COLORREF errorBorderColor);
	void setAdditionalEditStyles(DWORD styles);
	void setAlignment(DWORD alignment);
	void setSpacing(int spacing);
	void setTabulatorAcceptance(bool acceptTabs);
	void setDescriptionText(iString text);
	void setContent(iString text);
	void setContent(LPTSTR text);
	void setContentWithEventBlocker(iString text);
	void setContentWithEventBlocker(LPTSTR text);
	iString getContent();
	void selectAll();
	void setFocus();
	// -1 means the end of the text (use SLE_CURSORTOTEXTEND)
	void setCursorToIndex(int index);

	HRESULT Create(HWND Parent, DWORD ctrlID);

	void Release() { delete this; }

	void enableChangeBlocker() { this->changePreventer = TRUE; }
	void disableChangeBlocker() { this->changePreventer = FALSE; }

	void restrictedContent(int restrictMode);
	// this method has no effect if the SLE_RESTRICTTYPE_ONLYNUMBERS type isn't set
	void setValidContentRange(int min, int max);

	bool isContentInvalid() { return this->Properties.contentInvalid; }

	void setExtendedFlag(DWORD flag);

	static singleLineEdit* GetSingleLineEditObject(HWND host, int ID);

private:
	HINSTANCE hInstance;
	HWND parent;
	HWND frame;
	HWND edit;

	BOOL changePreventer;

	iString descriptionText;
	iString editContent;
	iString editFontFamily;

	SINGLELINEEDITPROPERTIES Properties;

	singleLineEditEventSink* eventHandler;

	static LRESULT CALLBACK sleFrameProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK editSub(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);

	LRESULT onPaintInFrame(HWND);
	LRESULT onCommand(HWND, WPARAM, LPARAM);

	void setDefaultValues();
	void registerSLEClass();
	//void calculateEditBoxPositionWithDescription();
	void calculateEditBoxPosition();
	void updateContentHolder();
	void checkContentType();
};