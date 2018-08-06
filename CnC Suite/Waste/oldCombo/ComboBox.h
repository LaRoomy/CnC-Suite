#pragma once
#include"external.h"


class comboBox
{
public:
	comboBox(HINSTANCE, HWND, DWORD, int, int, int, int);



private:
	HINSTANCE hInst;
	HWND Combo;
	HWND Parent;
	DWORD Id;

};