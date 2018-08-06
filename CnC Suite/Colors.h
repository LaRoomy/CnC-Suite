#pragma once
#ifndef _CNCS_COLORS_H_
#define _CNCS_COLORS_H_

#include<Windows.h>

#define			COLOR_RED		RGB(255,0,0)
#define			COLOR_GREEN		RGB(0,255,0)
#define			COLOR_BLUE		RGB(0,0,255)
#define			COLOR_YELLOW	RGB(255,255,0)
#define			COLOR_CYAN		RGB(0,255,255)
#define			COLOR_PURPLE	RGB(255,0,255)
#define			COLOR_ORANGE	RGB(255,128,0)
#define			COLOR_YELLOWGREEN	RGB(207,250,61)
#define			COLOR_MINTGREEN		RGB(61,250,188)
#define			COLOR_LIGHTBLUE		RGB(58,218,252)
#define			COLOR_DARKGREY		RGB(100,100,100)
#define			COLOR_LIGHTGREY		RGB(220,220,220)

#define			COLOR_DARKGREEN		RGB(0,150,0)
#define			COLOR_DARKRED		RGB(150,0,0)
#define			COLOR_DARKBLUE		RGB(0,0,150)
#define			COLOR_DARKYELLOW	RGB(170,170,0)

#define			COLOR_BLACK			((COLORREF)0)
#define			COLOR_WHITE			((COLORREF)0xFFFFFF)

#define			COLOR_BUTTON_MOUSE_OVER		RGB(0, 120, 180)
#define			COLOR_BUTTON_MOUSE_OVER_LIGHT	RGB(0, 170, 230)
#define			COLOR_BUTTON_PRESSED_LIGHT		RGB(0, 140, 200)
#define			COLOR_BUTTON_PRESSED		RGB(0, 100, 150)
#define			COLOR_DISABLED_CONTROL		RGB(150, 150, 150)

#define			COLOR_SUCCESS	RGB(0,255,0)

#define			ERROR_COLOR		RGB(255,0,0)
#define			WARNING_COLOR	RGB(255,255,0)
#define			INFO_COLOR		RGB(255,255,255)

typedef struct _MSGCOLORS {

	COLORREF error;
	COLORREF warning;
	COLORREF info;
	COLORREF ok;

	_MSGCOLORS& _MSGCOLORS::operator= (const _MSGCOLORS& mc)
	{
		this->error = mc.error;
		this->warning = mc.warning;
		this->info = mc.info;
		this->ok = mc.ok;

		return *this;
	}
}MSGCOLORS, *LPMSGCOLORS;

#endif