#pragma once
#include"..//external.h"
#include<ctrlHelper.h>
#include<StringClass.h>

class Textfield
{
public:
	Textfield();
	~Textfield();

	HRESULT Create(HINSTANCE hInst, HWND Parent, LPPOINT pos, LPSIZE size);

	void Release() { delete this; }
	
	void Update();

	void setCreationFlags(DWORD flags) {
		this->dwFlags = flags;
	}
	void setText(const iString& Text) {
		this->text = Text;

		if (this->textfield != nullptr)
		{
			this->Update();
		}
	}
	void setText(LPCTSTR _text_) {
		this->text.Replace(_text_);

		if (this->textfield != nullptr)
		{
			this->Update();
		}
	}

	void setPadding(int padding);
	void setPadding(int paddingX, int paddingY);
	void setPadding(int left, int top, int right, int bottom);

	void setTextfieldId(int ID) {
		this->textfieldId = ID;
	}
	void setColors(COLORREF ctext, COLORREF cbackground);
	void setFont(HFONT font);
	void setAdditionalWindowStyles(DWORD styles) {
		this->additionalWindowStyles = styles;
	}

	bool getDimensions(LPRECT window, LPRECT client);

	static const int CF_USEFIXEDSIZE = 0x01;
	static const int CF_APPLYSIZETOCONTEXT = 0x02;

	static const int TF_DEFAULT_ID = 2839;

	static const TCHAR* textFieldWindowClass;

private:
	HINSTANCE hInstance;
	HWND parent;
	HWND textfield;
	DWORD additionalWindowStyles;

	HFONT ctrlFont;

	COLORREF textcolor;
	COLORREF backgroundcolor;

	DWORD textfieldId;

	int padding_left;
	int padding_right;
	int padding_top;
	int padding_bottom;

	DWORD dwFlags;
	iString text;

	HRESULT _registerTextfieldClass();

	static LRESULT CALLBACK textFieldProc(HWND, UINT, WPARAM, LPARAM);

	LRESULT onPaint(HWND);
};
