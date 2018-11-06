#include"CommandLineTool.h"
#include"CnCSuite_Tabcontrol.h"
#include"ApplicationData.h"
#include"HelperF.h"

CnCS_CommandLine::CnCS_CommandLine()
	: hInstance(nullptr),
	Parent(nullptr),
	clToolWnd(nullptr),
	console(nullptr)
{
}

const WCHAR* CnCS_CommandLine::COMMANDLINETOOLCLASS = L"COMMANDLINETOOLCLASS3905";

HRESULT CnCS_CommandLine::CreatePopUp(LPCTRLCREATIONSTRUCT pccs)
{
	auto hr = (pccs != nullptr) ? S_OK : E_INVALIDARG;
	if (SUCCEEDED(hr))
	{
		this->Parent = pccs->parent;
		this->hInstance = pccs->hInst;

		hr = this->registerCLToolWindow();
		if (SUCCEEDED(hr))
		{
			this->clToolWnd =
				CreateWindow(
					CnCS_CommandLine::COMMANDLINETOOLCLASS,
					L"ConsoleX",
					WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_THICKFRAME,// !!!!!!!!!!!!!!!!!!
					pccs->pos.x,
					pccs->pos.y,
					pccs->size.cx,
					pccs->size.cy,
					pccs->parent,
					nullptr,
					pccs->hInst,
					reinterpret_cast<LPVOID>(this)
				);

			hr = (this->clToolWnd != nullptr)
				? S_OK : E_HANDLE;

			if (SUCCEEDED(hr))
			{
				this->console = new CSConsole(this->hInstance);
				hr = (this->console != nullptr)
					? S_OK : E_FAIL;
				if(SUCCEEDED(hr))
				{
					RECT rc;
					GetClientRect(this->clToolWnd, &rc);

					POINT pos = { 0,0 };
					SIZE sz = { rc.right, rc.bottom };

					this->console->SetConsoleType(CSConsole::CONSOLETYPE_MULTICOLOR | CSConsole::CONSOLETYPE_ACCEPTINPUT | CSConsole::CONSOLETYPE_SCROLLABLE);

					this->console->SetCaretWidth(
						DPIScale(12)
					);
					this->console->SetCommandEntry(L"_>");
					this->console->SetLineHeight(
						DPIScale(20)
					);
					this->console->SetFontProperty(
						DPIScale(16),
						FW_BOLD,
						L"Consolas"
					);

					this->console->RegisterForUserEvents(
						dynamic_cast<IConsoleInputEvent*>(this)
					);

					hr = this->console->Create(
						this->clToolWnd,
						&pos, &sz,
						CTRLID_COMMANDLINECONSOLE
					);

					if (SUCCEEDED(hr))
					{
						ShowWindow(this->clToolWnd, SW_SHOW);
						UpdateWindow(this->clToolWnd);
					}
				}
			}
		}
	}
	return hr;
}

HRESULT CnCS_CommandLine::registerCLToolWindow()
{
	HRESULT hr = S_OK;
	WNDCLASSEX wcex;

	if (GetClassInfoEx(this->hInstance, CnCS_CommandLine::COMMANDLINETOOLCLASS, &wcex) == 0)
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = CnCS_CommandLine::cltoolwndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = this->hInstance;
		wcex.hIcon = nullptr;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = CnCS_CommandLine::COMMANDLINETOOLCLASS;
		wcex.hIconSm = (HICON)LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(IDI_CNC3_FILEICON_SQ32_DSBL),
			IMAGE_ICON, 32,32,LR_DEFAULTCOLOR
		);
		hr = (RegisterClassEx(&wcex) != 0) ? S_OK : E_FAIL;
	}
	return hr;
}

LRESULT CnCS_CommandLine::cltoolwndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CnCS_CommandLine *clTool = nullptr;

	if (message == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		if (pcr != nullptr)
		{
			clTool = reinterpret_cast<CnCS_CommandLine*>(pcr->lpCreateParams);
			if (clTool != nullptr)
			{
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(clTool));
			}
		}
	}
	else
	{
		clTool = reinterpret_cast<CnCS_CommandLine*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (clTool != nullptr)
		{
			switch (message)
			{
			case WM_SIZE:
				return clTool->onSize();
			case WM_DESTROY:
				SafeRelease(&clTool);
				return 0;
			default:
				break;
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return LRESULT();
}

LRESULT CnCS_CommandLine::onSize()
{
	RECT rc;
	GetClientRect(this->clToolWnd, &rc);

	if (this->console != nullptr)
	{
		this->console->Move(0, 0, rc.right, rc.bottom);
	}
	return static_cast<LRESULT>(0);
}

bool CnCS_CommandLine::analyzeCommand(CSConsole * csconsole, LPCWSTR command)
{
	// return true to insert the default command entry or false to prevent this...

	if (CompareStringsAsmW(command, L"cls"))
	{
		csconsole->Clear();
	}
	else
	{
		iString _cmd_(command);

		if (_cmd_.Equals(L"help"))
			this->onHelpCommand(command);
		else if (_cmd_.Equals(L"info"))
			this->onInfoCommand(command);
		else if (_cmd_.Contains(L"showdata"))
			this->onDataCommand(_cmd_);
		else if (_cmd_.Contains(L"setvalue"))
			this->onSetValueCommand(_cmd_);
		else if (_cmd_.Contains(L"getvalue"))
			this->onGetValueCommand(_cmd_);
		else if (_cmd_.Equals(L"exit"))
		{
			this->onExitCommand();
			return false;
		}
		else
			csconsole->AddLine(L"invalid command", RGB(255,0,0));
	}
	return true;
}

void CnCS_CommandLine::onHelpCommand(LPCWSTR command)
{
	UNREFERENCED_PARAMETER(command);

	this->console->AddEmptyLine();

	this->console->AddLine(L"Available Commands:", RGB(255,255,0));
	this->console->AddLine(L"info - information about this component", RGB(255, 255, 0));
	//this->console->AddLine(L"execute::[modul] - starts the requested modul", RGB(255, 255, 0));
	this->console->AddLine(L"showdata::[segment] - shows the requested data segment", RGB(255, 255, 0));
	this->console->AddLine(L"setvalue::[value ID]::[new value] - sets the value from the given ID", RGB(255, 255, 0));
	this->console->AddLine(L"getvalue::[value ID] - shows the value from the given ID", RGB(255, 255, 0));
	//this->console->AddLine(L"overwritecolor::[color ID]::[RGB Color Value(hexadecimal)] - overwrites the desired color", RGB(255, 255, 0));

	this->console->AddEmptyLine();
}

void CnCS_CommandLine::onInfoCommand(LPCWSTR command)
{
	UNREFERENCED_PARAMETER(command);

	this->console->AddEmptyLine();
	this->console->AddLine(L"CnC Suite Console (C)2018 by Hans Philipp Zimmermann (LaroomySoft)", RGB(0,255,0));
	this->console->AddEmptyLine();
}

void CnCS_CommandLine::onExitCommand()
{
	this->console->Freeze(true);
	DestroyWindow(this->clToolWnd);
}

void CnCS_CommandLine::onDataCommand(iString& command)
{
	command.Remove(L"showdata::");

	if (command.Equals(L"currenttab"))
	{
		auto tabControl =
			reinterpret_cast<CnCSuite_Tabcontrol*>(
				getComponentHandle(COMP_ID_TAB_CONTROL)
				);
		if (tabControl != nullptr)
		{
			itemCollection<iString> tabData;
			tabControl->GetCurrentTabDataAsStringCollection(tabData);

			for (int i = 0; i < tabData.GetCount(); i++)
			{
				this->console->AddLine(
					tabData.GetAt(i)
					.GetData(),
					RGB(255, 127, 0)
				);
			}
		}
	}
	//else if(...){   }// more data commands!!!!
	else
	{
		this->console->AddLine(L"Error - invalid segment", ERROR_COLOR);
	}

}

void CnCS_CommandLine::onSetValueCommand(iString & command)
{
	command.Remove(L"setvalue::");

	if (command.Contains(L"driveloadingblocker"))
	{
		this->onDriveLoadingBlockerCommand(command);
	}
	else if(command.Contains(L"focusrectoffset"))
	{
		this->onFocusrectCommand(command);
	}
	else if (command.Contains(L"treeviewfontheight"))
	{
		this->onTvFontHeightCommand(command);
	}
	else
	{
		this->console->AddLine(L"Error - invalid set command", ERROR_COLOR);
	}
}

void CnCS_CommandLine::onGetValueCommand(iString & command)
{
	command.Remove(L"getvalue::");

	if (command.Contains(L"driveloadingblocker"))
	{
		auto dlb =
			reinterpret_cast<ApplicationData*>(
				getApplicationDataContainerFromFilekey(FILEKEY_INTERNAL_SETTINGS)
				)->getBooleanData(DATAKEY_INTSET_BLOCKDRIVELOADING, true);
		if(dlb)
			this->console->AddLine(L"ID::driveloadingblocker - value: <true>", COLOR_SUCCESS);
		else
			this->console->AddLine(L"ID::driveloadingblocker - value: <false>", COLOR_SUCCESS);
	}
	else if (command.Contains("treeviewfontheight"))
	{
		auto height =
			reinterpret_cast<ApplicationData*>(
				getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
				)->getIntegerData(DATAKEY_EXSETTINGS_TREEVIEW_FONTHEIGHT, 16);

		iString msg(L"ID::treeviewfontheight - value: ");
		msg += height;

		this->console->AddLine(
			msg.GetData(),
			COLOR_SUCCESS
		);
	}
	else if (command.Contains(L"focusrectoffset"))
	{
		auto internalSettings =
			reinterpret_cast<ApplicationData*>(
				getApplicationDataContainerFromFilekey(FILEKEY_INTERNAL_SETTINGS)
			);
		if (internalSettings != nullptr)
		{
			iString msg(L"ID::focusrectoffset - values: <top: ");
			msg +=
				internalSettings->getIntegerData(DATAKEY_INTSET_EDITOR_FOCUSRECTOFFSET_TOP, 1);
			msg += L"> : <bottom: ";
			msg +=
				internalSettings->getIntegerData(DATAKEY_INTSET_EDITOR_FOCUSRECTOFFSET_BOTTOM, 5);
			msg += L">";

			this->console->AddLine(
				msg.GetData(),
				COLOR_SUCCESS
			);
		}
	}
	else
		this->console->AddLine(L"Error - invalid ID", ERROR_COLOR);
}

void CnCS_CommandLine::onDriveLoadingBlockerCommand(iString & command)
{
	command.Remove(L"driveloadingblocker::");
	bool newValue;

	if (command.Equals(L"true"))
		newValue = true;
	else if (command.Equals(L"false"))
		newValue = false;
	else
	{
		this->console->AddLine(L"error - invalid value", ERROR_COLOR);
		return;
	}
	reinterpret_cast<ApplicationData*>(
		getApplicationDataContainerFromFilekey(FILEKEY_INTERNAL_SETTINGS)
		)->saveValue(DATAKEY_INTSET_BLOCKDRIVELOADING, newValue);

	iString successMsg(L"driveloadingblocker successful set, new value: <");
	successMsg += command;
	successMsg += L">";

	this->console->AddLine(successMsg.GetData(), COLOR_SUCCESS);
}

void CnCS_CommandLine::onFocusrectCommand(iString & command)
{
	command.Remove(L"focusrectoffset::");
	int newVal = 0;

	if (command.Contains(L"top"))
	{
		command.Remove(L"top::");

		newVal = command.getAsInt();
		if (IS_VALUE_IN_RANGE(newVal, -10, 10))
		{
			reinterpret_cast<ApplicationData*>(
				getApplicationDataContainerFromFilekey(FILEKEY_INTERNAL_SETTINGS)
				)->saveValue(
					DATAKEY_INTSET_EDITOR_FOCUSRECTOFFSET_TOP,
					newVal
				);

			reinterpret_cast<CnCSuite_Tabcontrol*>(
				getComponentHandle(COMP_ID_TAB_CONTROL)
				)->ICommand(MAKEWPARAM(ICOMMAND_FOCUSRECTOFFSET_CHANGED, 0)
					, 0
				);
		}
		else
		{
			this->console->AddLine(L"Error - focusrectoffset: top-value out of range", ERROR_COLOR);
			return;
		}
	}
	else if (command.Contains(L"bottom"))
	{
		command.Remove(L"bottom::");

		newVal = command.getAsInt();
		if (IS_VALUE_IN_RANGE(newVal, -10, 10))
		{
			reinterpret_cast<ApplicationData*>(
				getApplicationDataContainerFromFilekey(FILEKEY_INTERNAL_SETTINGS)
				)->saveValue(
					DATAKEY_INTSET_EDITOR_FOCUSRECTOFFSET_BOTTOM,
					newVal
				);

			reinterpret_cast<CnCSuite_Tabcontrol*>(
				getComponentHandle(COMP_ID_TAB_CONTROL)
				)->ICommand(MAKEWPARAM(ICOMMAND_FOCUSRECTOFFSET_CHANGED, 0)
					, 0
				);
		}
		else
		{
			this->console->AddLine(L"Error - focusrectoffset: bottom-value out of range", ERROR_COLOR);
			return;
		}
	}
	else if (command.Contains(L"set_default"))
	{
		reinterpret_cast<ApplicationData*>(
			getApplicationDataContainerFromFilekey(FILEKEY_INTERNAL_SETTINGS)
			)->deleteValue(DATAKEY_INTSET_EDITOR_FOCUSRECTOFFSET_TOP);
		reinterpret_cast<ApplicationData*>(
			getApplicationDataContainerFromFilekey(FILEKEY_INTERNAL_SETTINGS)
			)->deleteValue(DATAKEY_INTSET_EDITOR_FOCUSRECTOFFSET_BOTTOM);

		reinterpret_cast<CnCSuite_Tabcontrol*>(
			getComponentHandle(COMP_ID_TAB_CONTROL)
			)->ICommand(MAKEWPARAM(ICOMMAND_FOCUSRECTOFFSET_CHANGED, 0)
				, 0
			);

		this->console->AddLine(L"focusrectoffset - defaultvalues recovered", COLOR_SUCCESS);
		return;
	}
	else
	{
		this->console->AddLine(L"Error - focusrectoffset - invalid value accessor", ERROR_COLOR);
		return;
	}

	iString successMsg(L"focusrectoffset successful set, new value: <");
	successMsg += newVal;
	successMsg += L">";

	this->console->AddLine(successMsg.GetData(), COLOR_SUCCESS);
}

void CnCS_CommandLine::onTvFontHeightCommand(iString & command)
{
	command.Remove(L"treeviewfontheight::");

	auto fontHeight = command.getAsInt();
	if (fontHeight > 0)
	{
		if (fontHeight > 51)
		{
			this->console->AddLine(L"invalid value -> maximum accepted value is 50!", ERROR_COLOR);
		}
		else
		{
			reinterpret_cast<ApplicationData*>(
				getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
			)->saveValue(DATAKEY_EXSETTINGS_TREEVIEW_FONTHEIGHT, fontHeight);

			this->console->AddLine(L"treeviewfontheight successful set", COLOR_SUCCESS);
			this->console->AddLine(L"please restart the application to apply the changes", COLOR_SUCCESS);
		}
	}
}
