#pragma once
#include"external.h"
#include"CommonControls\ctrlHelper.h"
#include<StringClass.h>
#include<CSConsole.h>

#define		CTRLID_COMMANDLINECONSOLE			200


class CnCS_CommandLine
	: public ClsObject<CnCS_CommandLine>,
	public IConsoleInputEvent
{
public:
	CnCS_CommandLine();

	HRESULT CreatePopUp(LPCTRLCREATIONSTRUCT pccs);

	static const WCHAR* COMMANDLINETOOLCLASS;

	// console events:
	bool UserPressedReturn(cObject sender) {

		reinterpret_cast<CSConsole*>(sender)->AddLine(L"invalid command", RGB(255, 0, 0));
		return true;
	}
	bool UserEnteredCommand(cObject sender, LPCWSTR command) {

		return this->analyzeCommand(
			reinterpret_cast<CSConsole*>(sender),
			command
		);
	}

	const wchar_t* ToString() {
		return L"commandline tool";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

private:
	HINSTANCE hInstance;
	HWND Parent;
	HWND clToolWnd;

	CSConsole* console;

	HRESULT registerCLToolWindow();

	static LRESULT CALLBACK cltoolwndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT onSize();

	bool analyzeCommand(CSConsole* csconsole, LPCWSTR command);

	// first level commands
	void onHelpCommand(LPCWSTR command);
	void onInfoCommand(LPCWSTR command);
	void onExitCommand();
	void onDataCommand(iString& command);
	void onSetValueCommand(iString& command);
	void onGetValueCommand(iString& command);

	// sublevel commands
	void onDriveLoadingBlockerCommand(iString& command);
	void onFocusrectCommand(iString& command);
	void onTvFontHeightCommand(iString& command);
};
