#include "CnCS_TC.h"
#include "CnCSuite_Property.h"
#include "Error dispatcher.h"
#include"ApplicationData.h"
#include"ColorSchemeManager.h"
#include"AppPath.h"
#include"FontFileReader.h"
#include"ProgressDialog.h"
#include"AutosyntaxManager.h"
#include"CnCSuite_FileNavigator.h"
#include"IPath.h"

//temp:
#include"DataExchange\DataExchange.h"
#include"DateTime.h"
#include"history.h"
#include"CnC3FileManager.h"

CnCS_TC::CnCS_TC(HINSTANCE hInst, HWND MainWindow) : hInstance(hInst), Main(MainWindow), TABCount(0), _thisThreadId(0)
{
	// zero structs
	SecureZeroMemory(&this->iParam, sizeof(TCINTERNALPARAMETER));
	SecureZeroMemory(&this->tcObj, sizeof(TCGDIOBJECTS));
	
	// load the available fonts from file
	this->LoadFontResource();

	SendMessage(MainWindow, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&this->styleInfo));

	this->tcObj.tabbkgnd = CreateSolidBrush(this->styleInfo.TabColor);
	this->setSelectedTabBackground();
	this->tcObj.framebkgnd = CreateSolidBrush(this->styleInfo.Stylecolor);
	this->tcObj.toolbarbkgnd = CreateSolidBrush(this->styleInfo.mainToolbarColor);

	this->tcObj.cPen = CreatePen(PS_SOLID, 1, this->styleInfo.OutlineColor);
	this->tcObj.Textcolor = this->styleInfo.TextColor;


	this->_createDpiDependendResources();
}

CnCS_TC::~CnCS_TC()
{
	// unhook mousehook ??
	// unregister classes ??

	this->SaveDESCPropertyNames();// temp

	SafeDeleteArray(&this->iParam.WorkingDirectory);
	SafeDeleteArray(&this->iParam.DESC_property1);
	SafeDeleteArray(&this->iParam.DESC_property2);
	SafeDeleteArray(&this->iParam.DESC_property3);
	SafeDeleteArray(&this->iParam.descClipboard.d1);
	SafeDeleteArray(&this->iParam.descClipboard.d2);
	SafeDeleteArray(&this->iParam.descClipboard.d3);

	DeleteObject(this->tcObj.Tabfont);
	DeleteObject(this->tcObj.selTabbkgnd);
	DeleteObject(this->tcObj.desc_propertyfont);

	DeleteObject(this->tcObj.tabbkgnd);
	DeleteObject(this->tcObj.framebkgnd);
	DeleteObject(this->tcObj.toolbarbkgnd);

	DeleteObject(this->tcObj.cPen);

	DestroyIcon(this->tcObj.tabclose_green);
	DestroyIcon(this->tcObj.tabclose_red);
	DestroyIcon(this->tcObj.tabclose_marked);

	// destroy toolbar icons ...
}

void CnCS_TC::GetCurrentCnC3File(CnC3File & file)
{
	// set the file definition from the tabinfostruct related to the current tab

	file.Clear();

	auto ptp = this->GetActiveTabProperty();
	if (ptp != nullptr)
	{
		if (ptp->DESC1 != nullptr)
			file.AddProperty(CnC3File::PID_DESCRIPTION_ONE, ptp->DESC1);
		if (ptp->DESC2 != nullptr)
			file.AddProperty(CnC3File::PID_DESCRIPTION_TWO, ptp->DESC2);
		if (ptp->DESC3 != nullptr)
			file.AddProperty(CnC3File::PID_DESCRIPTION_THREE, ptp->DESC3);

		if (ptp->Path != nullptr)
			file.SetPath(ptp->Path);

		if (ptp->Editcontrol != nullptr)
		{
			TCHAR* content = nullptr;

			if (ptp->Editcontrol->GetTextContent(&content) > 0)
			{
				file.SetNCContent(content);

				SafeDeleteArray(&content);
			}
		}
		file.SetStatus(S_OK);
	}
}

void CnCS_TC::UserRequest_Open(const CnC3File & file, bool forceOpenInNewTab, bool setFocus)
{
	if (this->isAlreadyOpened(file))
	{
		DispatchEWINotification(
			EDSP_INFO,
			L"TC0004",
			getStringFromResource(INFO_MSG_FILEISALREADYOPEN),
			getStringFromResource(UI_TABCONTROL)
		);
	}
	else
	{
		if (this->iParam.OpenNewPathInNewTab || forceOpenInNewTab)
		{
			if (this->AddTab(file))
			{
				if (setFocus)
				{
					auto _async = new Async();
					if (_async != nullptr)
					{
						_async->callFunction(&focusToEdit);
					}
				}
			}
		}
		else
		{
			if (this->SaveCheck_singleTab(this->GetActiveTabProperty()))
			{
				if (!this->OpenDisplayAndFormat(file, DO_CLEANUP | SET_DISPLAYNAME))
				{
					// handle error...
				}
				else
				{
					this->updateFileInfoArea();

					if (setFocus)
					{
						auto _async = new Async();
						if (_async != nullptr)
						{
							_async->callFunction(&focusToEdit);
						}
					}
				}
			}
		}
	}
}

void CnCS_TC::GetCurrentTabDataAsStringCollection(itemCollection<iString> & data)
{
	auto ptp = this->GetActiveTabProperty();
	if (ptp != nullptr)
	{
		iString line;

		line = L"TabIndex: ";
		line += ptp->TabIndex;
		data.AddItem(line);
		line.Clear();

		line = L"Path: ";
		line += ptp->Path;
		data.AddItem(line);
		line.Clear();

		line = L"DisplayName: ";
		line += ptp->displayname;
		data.AddItem(line);
		line.Clear();

		line = L"ContentChanged: ";
		line += (ptp->Content_Changed) ? L"True" : L"False";
		data.AddItem(line);
		line.Clear();

		line = L"hWnd(Tab)-Offset: 0x";
		line += iString::fromPointer(&ptp->Tab);
		data.AddItem(line);
		line.Clear();

		line = L"hWnd(RichEdit)-Offset: 0x";
		line += iString::fromPointer(&ptp->AssocEditWnd);
		data.AddItem(line);
		line.Clear();

		line = L"Editcontrol-Offset: 0x";
		line += iString::fromPointer(&ptp->Editcontrol);
		data.AddItem(line);
		line.Clear();

		//...

	}
	else
		data.AddItem(L"error - no tab info found");
}

void CnCS_TC::SaveAsPathWasSelected(cObject sender, LPCTSTR path)
{
	// check if the file will be overwritten
	BOOL fOverwritten = CheckForFileExist(path);
	if (fOverwritten)
	{
		// check if the file is opened in another tab and if so
		// -> close the other tab to prevent that one file is opened in multiple tabs
		this->closeTabIfPathIsEqual(path);
	}
}

HRESULT CnCS_TC::_Init(HWND Frame, LPTCSTARTUPINFO tc_info)
{
	HRESULT hr = (Frame == nullptr) ? E_FAIL : S_OK;
	if (SUCCEEDED(hr))
	{
		this->TCFrame = Frame;

		hr = this->InitializeValues(tc_info) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			hr = this->RegisterTCClasses();
			if (SUCCEEDED(hr))
			{
				SetLastError(0);

				hr = ((SetWindowLongPtr(Frame, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this)) == 0)
					&& GetLastError() != 0) ? E_FAIL : S_OK;
				if (SUCCEEDED(hr))
				{
					hr = SetWindowSubclass(Frame, CnCS_TC::TabFrameProc, NULL, reinterpret_cast<DWORD_PTR>(this)) ? S_OK : E_FAIL;
					if (SUCCEEDED(hr))
					{
						hr = this->InitFrameProperty();
						if (SUCCEEDED(hr))
						{
							hr = this->InitComponents(tc_info);
							if (SUCCEEDED(hr))
							{
								DWORD threadId = GetCurrentThreadId();

								this->_thisThreadId = threadId;

								this->iParam.mouseHook = SetWindowsHookEx(WH_MOUSE, CnCS_TC::MouseProc, NULL, threadId);
								
								hr = (this->iParam.mouseHook != NULL) ? S_OK : E_FAIL;
								if (SUCCEEDED(hr))
								{
									// ...

								}
							}
						}
					}
				}
			}
		}
	}
	return hr;
}

HRESULT CnCS_TC::InitComponents(LPTCSTARTUPINFO tcInfo)
{
	HRESULT hr;
	RECT rc;
	GetClientRect(this->TCFrame, &rc);

	hr = this->CreateDescFrame(DESCFRAME_CREATESTATIC);
	if (SUCCEEDED(hr))
	{
		hr = this->ControlPropertyButton(PROPBUTTON_CREATE);
		if (SUCCEEDED(hr))
		{
			// NOTE: the method CreateTBButton calls DPIScale(...) for the x + y parameter!

			hr = this->CreateTBButton(IDM_UPPERCASE, 2, 133, getStringFromResource(UI_EDITTOOLBAR_UPPERCASE));
			if (SUCCEEDED(hr))
			{
				hr = this->CreateTBButton(IDM_AUTONUM, 2, 160, getStringFromResource(UI_EDITTOOLBAR_LINENUMBERS));
				if (SUCCEEDED(hr))
				{
					hr = this->CreateTBButton(IDM_AUTOCOMPLETE, 2, 187, getStringFromResource(UI_EDITTOOLBAR_AUTOCOMPLETE));
					if (SUCCEEDED(hr))
					{
						hr = this->CreateTBButton(IDM_TEXTCLR, 2, 214, getStringFromResource(UI_EDITTOOLBAR_TEXTCOLOR));
						if (SUCCEEDED(hr))
						{
							hr = this->CreateTBButton(IDM_FOCUSRECT, 2, 241, getStringFromResource(UI_EDITTOOLBAR_MARKCOLOR));
							if (SUCCEEDED(hr))
							{
								hr = this->CreateTBButton(IDM_ECONVERTALL, 2, 278, getStringFromResource(UI_EDITTOOLBAR_CONVERTALL));
								if (SUCCEEDED(hr))
								{
									hr = this->CreateTBButton(IDM_NUMSEQUENCE, 2, 305, getStringFromResource(UI_EDITTOOLBAR_SEQUENCELINENBR));
									if (SUCCEEDED(hr))
									{
#if defined(_DEBUG)
										hr = this->CreateTBButton(IDM_ERRORCHECK, 2, 386, getStringFromResource(UI_EDITTOOLBAR_CHECKFORERROR));// old pos 2, 332
										if (SUCCEEDED(hr))
										{
#endif
											//hr = this->CreateTBButton(IDM_AUTOSYNTAX, 2, 359, getStringFromResource(UI_EDITTOOLBAR_AUTOSYNTAX));
											//if (SUCCEEDED(hr))
											//{
												hr = this->CreateTBButton(IDM_FONTPROPERTIES, 2, 332, getStringFromResource(UI_EDITTOOLBAR_TEXTPROPERTIES));// old pos 2, 386 (do not delete this hint!)
												if (SUCCEEDED(hr))
												{
													hr = this->CreateTBButton(IDM_VT_UNDO, 2, 69, getStringFromResource(UI_GNRL_UNDO));
													if (SUCCEEDED(hr))
													{
														hr = this->CreateTBButton(IDM_VT_REDO, 2, 96, getStringFromResource(UI_GNRL_REDO));
														if (SUCCEEDED(hr))
														{
															if (tcInfo->restoreSession)
															{
																this->restoreCondition();
															}
															else
															{
																if ((tcInfo->mode != TCI_OPENREQUEST)
																	&& (tcInfo->mode != TCI_ISFIRSTUSE))
																{
																	this->ADD_Tab(nullptr);
																}
																else
																{
																	if (tcInfo->mode == TCI_ISFIRSTUSE)
																	{
																		AppPath appPath;

																		// open the template file copied by the installer
																		auto pathToFile =
																			appPath.Get(PATHID_FILE_FIRSTUSEEXAMPLE);

																		if (pathToFile.succeeded())
																		{
																			if (PathFileExists(
																				pathToFile.GetData()
																			))
																			{
																				this->ADD_Tab(
																					pathToFile.GetData()
																				);
																			}
																			else
																			{
																				this->ADD_Tab(nullptr);
																			}
																		}
																		else
																		{
																			this->ADD_Tab(nullptr);
																		}
																	}
																}
															}
															if (tcInfo->mode == TCI_OPENREQUEST)
															{
																// check if this file is already open!
																auto isOpen = false;
																
																for (DWORD i = 0; i < this->TABCount; i++)
																{
																	auto ptp = this->GetTabProperty(i);
																	if (ptp != nullptr)
																	{
																		auto isEqual = CompareStringsB(ptp->Path, tcInfo->PathToFile);
																		if (isEqual)
																		{
																			this->selectTab(i);
																			isOpen = true;
																		}
																	}
																}
																if (!isOpen)
																{
																	this->ADD_Tab(tcInfo->PathToFile);
																}
															}
														}
													}
												}
											//}
#if defined(_DEBUG)
										}
#endif
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return hr;
}

LRESULT CnCS_TC::TabFrameProc(HWND TabFrame, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR RefData)
{
	UNREFERENCED_PARAMETER(uID);

	CnCS_TC* pTC = reinterpret_cast<CnCS_TC*>(RefData);
	if (pTC != NULL)
	{
		switch (message)
		{
		case WM_ADDNEWTAB:
			return static_cast<LRESULT>(pTC->ADD_Tab(reinterpret_cast<LPTSTR>(lParam)));
		case WM_SIZE:
			return pTC->OnSize_inFrame();
		case WM_PAINT:
			return pTC->OnPaint_inFrame(TabFrame);
		case WM_NOTIFY:
			return pTC->OnNotify_inFrame(TabFrame, wParam, lParam);
		case WM_SETAPPSTYLE:
			return pTC->OnSetAppStyle(lParam);
		case WM_DRAWITEM:
			return pTC->OnDrawItem_inFrame(lParam);
		case WM_COMMAND:
			return pTC->OnCommand_inFrame(wParam, lParam);
		case WM_CTLCOLORSTATIC:
			return reinterpret_cast<LRESULT>(pTC->tcObj.framebkgnd);
		case WM_GETABSEDITPOS:
			return pTC->OnGetEditWndPos(lParam);
		default:
			break;
		}
	}
	return DefSubclassProc(TabFrame, message, wParam, lParam);
}

LRESULT CnCS_TC::TabProc(HWND Tab, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPTABPROPERTY ptp =
		reinterpret_cast<LPTABPROPERTY>(
			GetWindowLongPtr(Tab, GWLP_USERDATA)
		);

	if (ptp != NULL)
	{
		CnCS_TC* pTC = NULL;

		pTC = reinterpret_cast<CnCS_TC*>(ptp->toClass);
		if (pTC != NULL)
		{
			switch (message)
			{
			case WM_SIZE:
				return pTC->OnSize_inTab(Tab);
			case WM_MOUSEMOVE:
				return pTC->OnMousemove_inTab(Tab, lParam);
			case WM_LBUTTONDOWN:
				return pTC->OnLButtonDown_inTab(Tab, lParam);
			case WM_MOUSELEAVE:
				return pTC->OnMouseLeave_inTab(Tab);
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_PAINT:
				return pTC->OnPaint_inTab(Tab, ptp);
			case WM_DRAWITEM:
				return pTC->OnDrawItem_inTab(Tab, lParam, ptp);
			case WM_COMMAND:
				return pTC->OnCommand_inTab(Tab, wParam);
			case WM_VALIDATEERROR:
				return pTC->OnValidateError(lParam);
			default:
				break;
			}
		}
	}
	return DefWindowProc(Tab, message, wParam, lParam);
}

LRESULT CnCS_TC::ToolPopupProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CnCS_TC* pTC = NULL;

	if (message == WM_CREATE)
	{
		auto pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pTC = reinterpret_cast<CnCS_TC*>(pcs->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pTC));

		return 1;
	}
	else
	{
		pTC = reinterpret_cast<CnCS_TC*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (pTC != nullptr)
		{
			switch (message)
			{
			case WM_CLOSE:
				return 0;
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_PAINT:
				return pTC->OnPaintInToolPopup(hWnd);
			case WM_COMMAND:
				switch (HIWORD(wParam))
				{
				case CBN_DROPDOWN:
					pTC->iParam.PopupCloseBlocker = TRUE;
					break;
				case CBN_CLOSEUP:
					pTC->iParam.PopupCloseBlocker = FALSE;
					break;
				default:
					break;
				}
				return 0;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

LRESULT CnCS_TC::CloseBtnSub(HWND Closebutton, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR RefData)
{
	UNREFERENCED_PARAMETER(uID);

	LPTABPROPERTY ptp = reinterpret_cast<LPTABPROPERTY>(RefData);
	if (ptp != NULL)
	{
		switch (message)
		{
		case WM_MOUSEMOVE:
			if (!ptp->Closebutton_highl)
			{
				ptp->Closebutton_highl = TRUE;
				RedrawWindow(Closebutton, NULL, NULL, RDW_NOERASE | RDW_INVALIDATE);

				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = Closebutton;
				tme.dwHoverTime = HOVER_DEFAULT;

				TrackMouseEvent(&tme);

				// make sure the tab is in highlight state!
				if (ptp->Tab != ((CnCS_TC*)ptp->toClass)->iParam.hwndActiveTab)
				{
					if (ptp->mouseHover != 2)
						ptp->mouseHover = 2;
				}
			}
			return 0;
		case WM_MOUSELEAVE:
			if (ptp->Closebutton_highl)
			{
				if (ptp->mouseHover == 2)
				{
					// reactivate the tracker for the tab-window!

					TRACKMOUSEEVENT tme;
					tme.cbSize = sizeof(TRACKMOUSEEVENT);
					tme.dwFlags = TME_LEAVE;
					tme.hwndTrack = GetParent(Closebutton);
					tme.dwHoverTime = HOVER_DEFAULT;

					TrackMouseEvent(&tme);
				}

				ptp->Closebutton_highl = FALSE;
				RedrawWindow(Closebutton, NULL, NULL, RDW_NOERASE | RDW_INVALIDATE);
			}
			return 0;
		default:
			break;
		}
	}
	return DefSubclassProc(Closebutton, message, wParam, lParam);
}

LRESULT CnCS_TC::TBBtnSub(HWND button, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR RefData)
{
	UNREFERENCED_PARAMETER(uID);

	CnCS_TC* pTC = reinterpret_cast<CnCS_TC*>(RefData);
	if (pTC != NULL)
	{
		switch (message)
		{
		case WM_ERASEBKGND:
			return static_cast<LRESULT>(1);
		case WM_MOUSEMOVE:
			if (pTC->iParam.TBButtonIDdrawFor == 0)
			{
				pTC->iParam.TBButtonIDdrawFor = (int)GetWindowLongPtr(button, GWLP_ID);
				RedrawWindow(button, NULL, NULL, RDW_NOERASE | RDW_NOCHILDREN | RDW_INVALIDATE);

				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = button;
				tme.dwHoverTime = HOVER_DEFAULT;

				TrackMouseEvent(&tme);
			}
			return 0;
		case WM_MOUSELEAVE:
			if (pTC->iParam.TBButtonIDdrawFor != 0)
			{
				pTC->iParam.TBButtonIDdrawFor = 0;
				RedrawWindow(button, NULL, NULL, RDW_NOERASE | RDW_NOCHILDREN | RDW_INVALIDATE);
			}
			return 0;
		default:
			break;
		}

	}
	return DefSubclassProc(button, message, wParam, lParam);
}

LRESULT CnCS_TC::DescEditSub(HWND edit, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR refData)
{
	UNREFERENCED_PARAMETER(uID);

	auto Tabcontrol = reinterpret_cast<CnCS_TC*>(refData);
	if (Tabcontrol != NULL)
	{
		auto ID = GetWindowLongPtr(edit, GWLP_ID);

		if (message == WM_CHAR)
		{
			if (wParam == 0x09)
			{
				// tab was pressed -> forward cursor to new desc-edit

				HWND descFrame = GetDlgItem(Tabcontrol->TCFrame, ID_DESCFRAME);
				if (descFrame != NULL)
				{
					switch (ID)
					{
					case ID_PROPERTYEDIT_ONE:
						SetFocus(
							GetDlgItem(descFrame, ID_PROPERTYEDIT_TWO));
						break;
					case ID_PROPERTYEDIT_TWO:
						SetFocus(
							GetDlgItem(descFrame, ID_PROPERTYEDIT_THREE));
						break;
					case ID_PROPERTYEDIT_THREE:
						SetFocus(
							Tabcontrol->GetCurrentVisibleEditbox(NULL));
						break;
					default:
						break;
					}
				}
				return 0;
			}
			else
			{
				// if the user selects something (ctrl + A) or copies something (ctrl + C) -> do not set content-changed to true!
				auto ctrlKeyIsPressed =
					(GetKeyState(VK_CONTROL) & 0x8000)
					? true : false;

				auto aKeyIsPressed =
					(GetKeyState(0x41) & 0x8000)
					? true : false;

				auto cKeyIsPressed =
					(GetKeyState(0x43) & 0x8000)
					? true : false;

				auto exec = !((ctrlKeyIsPressed && aKeyIsPressed) || (ctrlKeyIsPressed && cKeyIsPressed));

				if (exec)
				{
					auto ptp = Tabcontrol->GetActiveTabProperty();
					if (ptp != NULL)
					{
						ptp->Content_Changed = TRUE;
						Tabcontrol->RedrawTab_s(REDRAW_CURRENT);
					}
				}
			}
		}
		else
		{
			if (message == WM_KILLFOCUS)
			{
				Tabcontrol->SaveDescritpionsToTabProperty(
					Tabcontrol->GetActiveTabProperty()
				);
			}
		}
	}
	return DefSubclassProc(edit,message, wParam, lParam); // return always the DefSubclass to keep the default processing
}

LRESULT CnCS_TC::DescWndProc(HWND descWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CnCS_TC* pTC = NULL;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pTC = reinterpret_cast<CnCS_TC*>(pcs->lpCreateParams);
		SetWindowLongPtr(descWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pTC));

		return 1;
	}
	else
	{
		pTC = reinterpret_cast<CnCS_TC*>(GetWindowLongPtr(descWnd, GWLP_USERDATA));
		if (pTC != NULL)
		{
			switch (message)
			{
			case WM_PAINT:
				return pTC->OnPaint_inDescWnd(descWnd);
			case WM_SIZE:
				return pTC->OnSize_inPropWnd(descWnd);
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			default:
				break;
			}
		}
		return DefWindowProc(descWnd, message, wParam, lParam);
	}
}

DWORD CnCS_TC::PropWndSoftControlProc(LPVOID lParam)
{
	__try
	{
		DWORD result = 0;

		LPTCTHREADDATA ptdata = reinterpret_cast<LPTCTHREADDATA>(lParam);
		if (ptdata != NULL)
		{
			DWORD Mode = ptdata->Mode;

			// TODO: send the parameter with the threaddata and do not search the window!

			HWND Main = FindWindow(L"CNCSUITECLASS", NULL);
			if (Main != NULL)
			{
				HWND tcFrame = GetDlgItem(Main, ID_TABCTRLFRAME);
				if (tcFrame != NULL)
				{
					HWND propWnd = GetDlgItem(tcFrame, ID_DESCFRAME);
					if (propWnd != NULL)
					{
						CnCS_TC* pTC = reinterpret_cast<CnCS_TC*>(GetWindowLongPtr(tcFrame, GWLP_USERDATA));
						if (pTC != NULL)
						{
							RECT rc;
							GetClientRect(tcFrame, &rc);
							rc.right -= 40;

							int border = 2 * (rc.right / 3);

							if (Mode == PROPWND_SOFTIN)
							{
								RECT rc_inval;
								HWND edit = NULL;
								int moveP = rc.right;

								MoveWindow(propWnd, moveP + 40, 27, rc.right / 3, rc.bottom - 27, TRUE);
								ShowWindow(propWnd, SW_SHOW);

								while (moveP > border)
								{
									MoveWindow(propWnd, moveP + 40, 27, rc.right / 3, rc.bottom - 27, FALSE);
									///////////

									SetRect(&rc_inval, 0, 0, (rc.right - (moveP)), rc.bottom - 27);
									ValidateRect(propWnd, NULL);
									InvalidateRect(propWnd, &rc_inval, FALSE);

									UpdateWindow(propWnd);

									///////////
									edit = pTC->GetCurrentVisibleEditbox(tcFrame);
									if (edit != NULL)
									{
										MoveWindow(edit, 40, 40, moveP, rc.bottom - 40, FALSE);

										SetRect(&rc_inval, moveP - 6, 0, moveP, rc.bottom - 40);
										ValidateRect(edit, NULL);
										InvalidateRect(edit, &rc_inval, FALSE);

										UpdateWindow(edit);
									}
									moveP -= 5;

									//Sleep(1);
								}
								MoveWindow(propWnd, border + 40, 27, rc.right / 3, rc.bottom - 27, TRUE);
								edit = pTC->GetCurrentVisibleEditbox(tcFrame);
								if (edit != NULL)
								{
									MoveWindow(edit, 40, 40, border, rc.bottom - 40, TRUE);
								}
							}
							else if (Mode == PROPWND_SOFTOUT)
							{
								RECT rc_inval;
								HWND edit = NULL;

								while (border < rc.right)
								{
									MoveWindow(propWnd, border + 40, 27, rc.right / 3, rc.bottom - 27, FALSE);

									edit = pTC->GetCurrentVisibleEditbox(tcFrame);
									if (edit != NULL)
									{
										MoveWindow(edit, 40, 40, border, rc.bottom - 40, FALSE);

										SetRect(&rc_inval, border - 25, 0, border, rc.bottom - 40);
										ValidateRect(edit, NULL);
										InvalidateRect(edit, &rc_inval, FALSE);

										UpdateWindow(edit);
									}
									border += 5;

									//Sleep(1);
								}
								edit = pTC->GetCurrentVisibleEditbox(tcFrame);
								if (edit != NULL)
								{
									MoveWindow(edit, 40, 40, rc.right, rc.bottom - 40, TRUE);
								}
								ShowWindow(propWnd, SW_HIDE);
							}
							//pTC->iParam.descWnd_isMoving = FALSE;

							InterlockedExchange((LONG*)&pTC->iParam.descWnd_isMoving, (LONG)FALSE);
						}
						else
							result = 4;
					}
					else
						result = 3;
				}
				else
					result = 2;
			}
			else
				result = 1;
		}
		return result;
	}
	__except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return 11;
	}
}

DWORD CnCS_TC::changeColorProc(LPVOID lParam)
{
	auto _this = reinterpret_cast<CnCS_TC*>(lParam);
	if (_this != nullptr)
	{
		_this->onSetNewColorScheme();
	}
	return 0;
}


LRESULT CnCS_TC::MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	else
	{
		LPMOUSEHOOKSTRUCT pmh = reinterpret_cast<LPMOUSEHOOKSTRUCT>(lParam);
		if (pmh != nullptr)
		{
			HWND REFWnd_ = FindWindow(TOOLWINDOWPOPUPCLASS, NULL);
			if (REFWnd_ == NULL)
			{
				HWND Main = FindWindow(IDSEX_APPLICATIONCLASS, NULL);
				if (Main)
				{
					REFWnd_ = GetDlgItem(Main, ID_TABCTRLFRAME);
				}
			}
			if (REFWnd_)
			{
				auto pTC = reinterpret_cast<CnCS_TC*>(GetWindowLongPtr(REFWnd_, GWLP_USERDATA));
				if (pTC != nullptr)
				{
					if (pTC->iParam.PopupIsOpen)
					{
						if (!pTC->iParam.PopupCloseBlocker)
						{
							if (pTC->iParam.currentOpenedPopUp != nullptr)
							{
								RECT rc_popup;
								GetWindowRect(pTC->iParam.currentOpenedPopUp, &rc_popup);

								if ((pmh->pt.x < rc_popup.left) || (pmh->pt.x > rc_popup.right) || (pmh->pt.y < rc_popup.top) || (pmh->pt.y > rc_popup.bottom))
								{
									SetCapture(REFWnd_);

									SHORT keystate1, keystate2;
									keystate1 = GetAsyncKeyState(VK_LBUTTON);
									keystate2 = GetAsyncKeyState(VK_RBUTTON);

									if ((keystate1 & 0x8000) || (keystate2 & 0x8000))
									{
										if (!(pTC->iParam.excludeRectIsValid &&
											((pmh->pt.x > pTC->iParam.excludeRect.left) && (pmh->pt.x < pTC->iParam.excludeRect.right)
												&& (pmh->pt.y > pTC->iParam.excludeRect.top) && (pmh->pt.y < pTC->iParam.excludeRect.bottom))
											))
										{
											pTC->clearToolWindowPopupControlParameterAndDestroy();
										}
									}
								}
								else
								{
									ReleaseCapture();
								}
							}
						}
					}
				}
			}
		}
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
}


LRESULT CnCS_TC::OnSize_inFrame()
{
	RECT rc;
	GetClientRect(this->TCFrame, &rc);
	
	LPTABPROPERTY ptp =
		reinterpret_cast<LPTABPROPERTY>(
			GetWindowLongPtr(this->iParam.hwndActiveTab, GWLP_USERDATA)
		);
	if (ptp != NULL)
	{
		int Ewidth = rc.right - DPIScale(40);

		if (this->iParam.descWnd_visible)
			Ewidth = 2 * (Ewidth / 3);

		MoveWindow(
			ptp->AssocEditWnd,
			DPIScale(40),
			DPIScale(40),
			Ewidth,
			rc.bottom - DPIScale(40),
			TRUE
		);

		if (this->iParam.descWnd_visible)
		{
			MoveWindow(
				GetDlgItem(this->TCFrame, ID_DESCFRAME),
				DPIScale(40)
				+ (2 * ((rc.right - DPIScale(40)) / 3)),
				DPIScale(27),
				(rc.right - DPIScale(40)) / 3,
				rc.bottom - DPIScale(27),
				TRUE
			);
		}
	}
	this->RefreshTabAlignment();

	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnPaint_inFrame(HWND Frame)
{
	RECT rc, rc_fill;
	HDC hdc, hdc_mem;
	HBITMAP compBMP;
	HGDIOBJ original;
	PAINTSTRUCT ps;

	GetClientRect(Frame, &rc);

	//GetClientRect(
	//	this->GetCurrentVisibleEditbox(nullptr),
	//	&rc_edit
	//);

	hdc = BeginPaint(Frame, &ps);
	if (hdc)
	{
		hdc_mem = CreateCompatibleDC(hdc);
		if (hdc_mem)
		{
			compBMP = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			if (compBMP)
			{
				original = SelectObject(hdc_mem, compBMP);

				int p27 = DPIScale(27);

				SetRect(&rc_fill, 0, 0, rc.right, p27);
				FillRect(hdc_mem, &rc_fill, this->tcObj.framebkgnd);

				SetRect(&rc_fill, 0, p27, rc.right, DPIScale(52));
				FillRect(hdc_mem, &rc_fill, this->tcObj.tabbkgnd);

				SetRect(&rc_fill, 0, DPIScale(52), rc.right, rc.bottom);
				FillRect(hdc_mem, &rc_fill, this->tcObj.tabbkgnd);

				int p26 = DPIScale(26);
				int p28 = DPIScale(28);

				SetRect(&rc_fill, 0, p27, p28, rc.bottom);
				FillRect(hdc_mem, &rc_fill, this->tcObj.toolbarbkgnd);

				// extension of the editscrollbar to fullfill a complete border from top to bottom
				// (disabled because it must be conditional, the edit-wnd could also be without scrollbar)
				//SetRect(
				//	&rc_fill,
				//	DPIScale(40) + rc_edit.right,
				//	p26,
				//	DPIScale(57) + rc_edit.right,
				//	DPIScale(40)
				//);
				//FillRect(hdc_mem, &rc_fill, this->tcObj.framebkgnd);

				SelectObject(hdc_mem, this->tcObj.cPen);
				MoveToEx(hdc_mem, 0, p26, NULL);	
				LineTo(hdc_mem, this->iParam.ActiveTabRange.x, p26);
				MoveToEx(hdc_mem, this->iParam.ActiveTabRange.y, p26, NULL);
				LineTo(hdc_mem, rc.right, p26);

				MoveToEx(hdc_mem, p28, p26, NULL);
				LineTo(hdc_mem, p28, rc.bottom);

				MoveToEx(hdc_mem, DPIScale(29), p26, NULL);
				LineTo(hdc_mem, DPIScale(29), rc.bottom);

				MoveToEx(hdc_mem, DPIScale(39), rc.bottom, NULL);
				LineTo(hdc_mem, DPIScale(39), p27);

				MoveToEx(hdc_mem, p28, p27, NULL);
				LineTo(hdc_mem, 0, p27);

				BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdc_mem, 0, 0, SRCCOPY);

				SelectObject(hdc_mem, original);
				DeleteObject(compBMP);
			}
			DeleteDC(hdc_mem);
		}
		EndPaint(Frame, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnCommand_inFrame(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case IDM_CTLPROPWND:
		this->ControlPropertyWindow();
		break;
	case IDM_AUTOCOMPLETE:
		this->OnVerticalToolBar_Autocomplete();
		break;
	case IDM_AUTONUM:
		this->OnVerticalToolBar_Autonum();
		break;
	case IDM_UPPERCASE:
		this->OnVerticalToolBar_Uppercase();
		break;
	case IDM_TEXTCLR:
		this->OnVerticalToolBar_Textcolor();
		break;
	case IDM_FOCUSRECT:
		this->OnVerticalToolBar_FocusRect();
		break;
	case IDM_ECONVERTALL:
		this->OnVerticalToolBar_eConvertAll();
		break;
	case IDM_FONTPROPERTIES:
		this->CreateFontPropertyBox();
		break;
	case IDM_AUTOSYNTAX:
		this->OnVerticalToolBar_AutoSyntax();
		break;
	case IDM_NUMSEQUENCE:
		this->OnVerticalToolBar_NumSequence();
		break;
	case IDM_ERRORCHECK:
		this->OnVerticalToolBar_ErrorCheck();
		break;
	case IDM_VT_UNDO:
		this->OnVerticalToolBar_Undo();
		break;
	case IDM_VT_REDO:
		this->OnVerticalToolBar_Redo();
		break;
	case IDMX_FILESYSTEMCHANGED:
		this->onFileSystemChanged(reinterpret_cast<LPFILESYSTEMOBJECT>(lParam));
		break;
	case ICOMMAND_FOCUSRECTOFFSET_CHANGED:
		this->onFocusRectOffsetChanged();
		break;
	default:
		if (HIWORD(wParam) == EN_CHANGE)
		{
			auto ptp = this->GetActiveTabProperty();
			if (ptp != nullptr)
			{
				ptp->Editcontrol->OnEnChange(this->TCFrame, wParam, lParam);
			}
		}
		break;
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnSize_inTab(HWND Tab)
{
	RECT rc;
	GetClientRect(Tab, &rc);

	HWND CloseButton = GetDlgItem(Tab, ID_TABCLOSE);
	if (CloseButton)
	{
		MoveWindow(
			CloseButton,
			rc.right - DPIScale(28),
			rc.bottom - DPIScale(21),
			DPIScale(16),
			DPIScale(16),
			TRUE
		);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnLButtonDown_inTab(HWND Tab, LPARAM lParam)
{
	if (Tab != this->iParam.hwndActiveTab)
	{
		LPTABPROPERTY ptp_old = reinterpret_cast<LPTABPROPERTY>(GetWindowLongPtr(this->iParam.hwndActiveTab, GWLP_USERDATA));
		if(ptp_old != NULL)
		{
			ShowWindow(ptp_old->AssocEditWnd, SW_HIDE);
			this->SaveDescritpionsToTabProperty(ptp_old);
		}
		this->iParam.previousTab = this->iParam.dwActiveTab;

		this->iParam.hwndActiveTab = Tab;

		LPTABPROPERTY ptp_new = reinterpret_cast<LPTABPROPERTY>(GetWindowLongPtr(Tab, GWLP_USERDATA));
		if (ptp_new != NULL)
		{
			this->iParam.dwActiveTab = ptp_new->TabIndex;
			/////////////////////////////////////////////
			if (!this->iParam.descWnd_isMoving)
			{
				RECT rc;
				GetClientRect(this->TCFrame, &rc);

				int dpiVal = DPIScale(40);

				int commonwidth;
				
				if (this->iParam.descWnd_visible)
					commonwidth = 2 * ((rc.right - dpiVal) / 3);
				else
					commonwidth = rc.right - dpiVal;

				MoveWindow(ptp_new->AssocEditWnd, dpiVal, dpiVal, commonwidth, rc.bottom - dpiVal, TRUE);
			}
			///////////////////////////////////////////////////////////////////////
			ptp_new->mouseHover = FALSE;
			RedrawWindow(Tab, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
			///////////////////////////////////////////////////////////////////////
			EDITCONTROLPROPERTIES pCP;
			this->SetEditControlProperties(&pCP);
			ptp_new->Editcontrol->UpdateProperties(&pCP, FALSE);
			///////////////////////////////////////////////////////////////////////
			ShowWindow(ptp_new->AssocEditWnd, SW_SHOW);
			this->SetDescriptions(ptp_new->DESC1, ptp_new->DESC2, ptp_new->DESC3);
			SetFocus(ptp_new->AssocEditWnd);
			ptp_new->Editcontrol->UpdateFocusRect();
		}
		this->RefreshTabAlignment();

		this->updateFileInfoArea();
	}
	else
	{
		auto ptp = this->GetActiveTabProperty();
		if (ptp != nullptr)
		{
			auto xTrack = GET_X_LPARAM(lParam);
			auto yTrack = GET_Y_LPARAM(lParam);

			if (((xTrack > DPIScale(10)) && (xTrack < DPIScale(26)) && (yTrack < DPIScale(20)) && (yTrack > DPIScale(4))))// mouse inside icon
			{
				this->OpenTabMenu();
			}
			SetFocus(ptp->AssocEditWnd);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnPaint_inTab(HWND Tab, LPTABPROPERTY ptp)
{
	RECT rc;
	HDC hdc, hdc_mem;
	HBITMAP bkndBmp;
	HGDIOBJ original, original_bitmap;
	PAINTSTRUCT ps;
	GetClientRect(Tab, &rc);

	hdc = BeginPaint(Tab, &ps);
	if (hdc)
	{
		hdc_mem = CreateCompatibleDC(hdc);
		if (hdc_mem)
		{
			bkndBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			if (bkndBmp)
			{
				bool isActiveTab = (ptp->TabIndex == this->iParam.dwActiveTab) ? true : false;

				// select the bitmap in the offscreen DC
				original_bitmap = SelectObject(hdc_mem, bkndBmp);

				// erase the bitmap
				FillRect(hdc_mem, &rc, this->tcObj.framebkgnd);

				// color the tabarea (only if this is the active tab or a selection must be indicated)
				if (isActiveTab || ptp->mouseHover)
				{
					RECT tabrect;
					SetRect(&tabrect, DPIScale(3), 0, rc.right - DPIScale(3), rc.bottom);

					if (ptp->mouseHover)
					{
						auto color = brightenColor(this->styleInfo.Stylecolor, 5);

						auto brush = CreateSolidBrush(color);
						if (brush)
						{
							FillRect(hdc_mem, &tabrect, brush);
							DeleteObject(brush);
						}
					}
					else
						FillRect(hdc_mem, &tabrect, this->tcObj.tabbkgnd);
				}
				// draw the text
				original = SelectObject(hdc_mem, this->tcObj.Tabfont);
				SetBkMode(hdc_mem, TRANSPARENT);

				if (isActiveTab)
				{
					SetTextColor(
						hdc_mem,
						this->styleInfo.TextColor
					);
				}
				else
				{
					SetTextColor(
						hdc_mem,
						getSlightlyDisabledTextColor(this->styleInfo.TextColor)
					);
				}
				this->DrawTabText(hdc_mem, Tab, ptp->displayname);

				// select the outline pen
				SelectObject(hdc_mem, this->tcObj.cPen);

				// if this is not the active tab, draw the separation line
				if (!isActiveTab)
				{
					MoveToEx(hdc_mem, 0, rc.bottom - 1, NULL);
					LineTo(hdc_mem, rc.right, rc.bottom - 1);
				}

				// draw normal border only if this is the active tab
				if (isActiveTab)
				{
					MoveToEx(hdc_mem, 0, rc.bottom - 1, nullptr);
					LineTo(hdc_mem, DPIScale(3), rc.bottom - 1);
					LineTo(hdc_mem, DPIScale(3), 0);
					LineTo(hdc_mem, rc.right - DPIScale(3), 0);
					LineTo(hdc_mem, rc.right - DPIScale(3), rc.bottom - 1);
					LineTo(hdc_mem, rc.right, rc.bottom - 1);
				}
				else
				{
					auto inactiveColor = getDisabledColor(this->styleInfo.Stylecolor);
					if (inactiveColor == this->styleInfo.OutlineColor)
					{
						inactiveColor = brightenColor(inactiveColor, 50);
					}

					HPEN inactivePen =
						CreatePen(
							PS_SOLID, 1,
							inactiveColor
						);

					if (inactivePen != nullptr)
					{
						SelectObject(hdc_mem, inactivePen);

						// draw the inactive border
						if (ptp->mouseHover)
						{
							MoveToEx(hdc_mem, DPIScale(3), rc.bottom - DPIScale(2), nullptr);
							LineTo(hdc_mem, DPIScale(3), 0);
							LineTo(hdc_mem, rc.right - DPIScale(3), 0);
							LineTo(hdc_mem, rc.right - DPIScale(3), rc.bottom - DPIScale(1));
						}
						else
						{
							MoveToEx(hdc_mem, DPIScale(3), rc.bottom - DPIScale(3), nullptr);
							LineTo(hdc_mem, DPIScale(3), DPIScale(1));
							MoveToEx(hdc_mem, rc.right - DPIScale(3), rc.bottom - DPIScale(3), nullptr);
							LineTo(hdc_mem, rc.right - DPIScale(3), DPIScale(1));
						}
						DeleteObject(inactivePen);
					}
				}

				HICON dIcon;
				
				if (isActiveTab)
				{
					dIcon = ptp->menuButtonHover ? this->tcObj.cnc3_marked : this->tcObj.cnc3;
				}
				else
					dIcon = this->tcObj.cnc3_disabled;

				// draw icon
				DrawIconEx(
					hdc_mem,
					DPIScale(10),
					DPIScale(4),
					dIcon,
					DPIScale(16),
					DPIScale(16),
					0,
					NULL,
					DI_NORMAL
				);
				// select the original stock object
				SelectObject(hdc_mem, original);

				// blit the image to the screen DC
				BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdc_mem, 0, 0, SRCCOPY);

				SelectObject(hdc_mem, original_bitmap);
				DeleteObject(bkndBmp);
			}
			DeleteDC(hdc_mem);
		}
		EndPaint(Tab, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnDrawItem_inTab(HWND hWnd, LPARAM lParam, LPTABPROPERTY ptp)
{
	UNREFERENCED_PARAMETER(hWnd);

	LPDRAWITEMSTRUCT pdi = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
	if (pdi != nullptr)
	{
		HICON *icon = nullptr;
		bool isActiveTab = (ptp->TabIndex == this->iParam.dwActiveTab) ? true : false;

		if (ptp->Closebutton_highl)
			icon = &this->tcObj.tabclose_marked;
		else
		{
			if (ptp->Content_Changed)
				icon = &this->tcObj.tabclose_red;
			else
				icon = &this->tcObj.tabclose_green;
		}

		if (ptp->mouseHover)
		{
			HBRUSH bkgnd =
				CreateSolidBrush(
					brightenColor(this->styleInfo.Stylecolor, 5)
				);

			if (bkgnd)
			{
				FillRect(pdi->hDC, &pdi->rcItem, bkgnd);
				DeleteObject(bkgnd);
			}
		}
		else
		{
			if(isActiveTab)
				FillRect(pdi->hDC, &pdi->rcItem, this->tcObj.tabbkgnd);
			else
				FillRect(pdi->hDC, &pdi->rcItem, this->tcObj.framebkgnd);
		}

		DrawIconEx(
			pdi->hDC,
			0, 0,
			*icon,
			DPIScale(16),
			DPIScale(16),
			0,
			nullptr,
			DI_NORMAL
		);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnMousemove_inTab(HWND tab, LPARAM lParam)
{
	auto ptp =
		reinterpret_cast<LPTABPROPERTY>(
			GetWindowLongPtr(tab, GWLP_USERDATA)
		);

	if (this->iParam.dwActiveTab != ptp->TabIndex)// its a hidden tab
	{
		if (ptp->mouseHover != TRUE)
		{
			if (ptp != nullptr)
			{
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = tab;
				tme.dwHoverTime = HOVER_DEFAULT;

				TrackMouseEvent(&tme);

				ptp->mouseHover = TRUE;
				RedrawWindow(tab, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
			}
		}
	}
	else
	{
		// its the current foreground tab

		auto xTrack = GET_X_LPARAM(lParam);
		auto yTrack = GET_Y_LPARAM(lParam);

		if (((xTrack > DPIScale(10)) && (xTrack < DPIScale(26)) && (yTrack < DPIScale(20)) && (yTrack > DPIScale(4))))// mouse inside icon
		{
			if (!ptp->menuButtonHover)
			{
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = tab;
				tme.dwHoverTime = HOVER_DEFAULT;

				TrackMouseEvent(&tme);

				ptp->menuButtonHover = TRUE;
				RedrawWindow(tab, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
			}
		}
		else
		{
			if (ptp->menuButtonHover)
			{
				ptp->menuButtonHover = FALSE;
				RedrawWindow(tab, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
			}
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnCommand_inTab(HWND Tab, WPARAM wParam)
{
	switch (LOWORD(wParam))
	{
	case ID_TABCLOSE:
		this->CLOSE_Tab(Tab, TRUE);
		break;
	case EN_CHANGE:
		this->OnEditHasChanged(Tab);
		break;
	default:
		break;
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnSetAppStyle(LPARAM lParam)
{
	BOOL result = TRUE;

	LPAPPSTYLEINFO pStyleInfo = reinterpret_cast<LPAPPSTYLEINFO>(lParam);
	if (pStyleInfo != NULL)
	{
		HWND Tab;

		//TABBMPIDS ids;
		//this->SetBitmapIDs(&ids, pStyleInfo->StyleID);

		this->iParam.currentStyle = pStyleInfo->StyleID;

		copyAppStyleInfo(pStyleInfo, &this->styleInfo);

		DeleteObject(this->tcObj.framebkgnd);
		this->tcObj.framebkgnd = CreateSolidBrush(pStyleInfo->Stylecolor);

		DeleteObject(this->tcObj.tabbkgnd);
		this->tcObj.tabbkgnd = CreateSolidBrush(pStyleInfo->TabColor);

		DeleteObject(this->tcObj.toolbarbkgnd);
		this->tcObj.toolbarbkgnd = CreateSolidBrush(this->styleInfo.mainToolbarColor);

		this->setSelectedTabBackground();

		DeleteObject(this->tcObj.cPen);
		this->tcObj.cPen = CreatePen(PS_SOLID, 1, pStyleInfo->OutlineColor);

		this->tcObj.Textcolor = pStyleInfo->TextColor;

		if (!this->iParam.UseXMLFileForUserColors)
		{
			this->SetEditStyleColorsFromStyleInfo(&this->editStyleColors, pStyleInfo);
		}

		for (DWORD i = 0; i < this->TABCount; i++)
		{
			Tab = GetDlgItem(this->TCFrame, TAB_ID_START + i);

			LPTABPROPERTY ptp = reinterpret_cast<LPTABPROPERTY>(GetWindowLongPtr(Tab, GWLP_USERDATA));
			if (ptp != NULL)
			{
				RedrawWindow(Tab, NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE);

				ptp->Editcontrol->ConfigureComponent(&this->editStyleColors, NULL, TRUE, TRUE);
			}
		}
		if (this->iParam.descWnd_exists)
		{
			HWND descFrame = GetDlgItem(this->TCFrame, ID_DESCFRAME);
			if (descFrame)
			{
				HWND edit;

				for (int i = 0; i < 3; i++)
				{
					edit = GetDlgItem(descFrame, ID_PROPERTYEDIT_ONE + i);
					if (edit)
					{
						this->ConfigPropEdit(edit);
					}
				}

				for (int i = 0; i < 4; i++)
				{
					auto button = GetDlgItem(descFrame, IDM_DESCWND_COPY + i);
					if (button != nullptr)
					{
						auto cButton =
							reinterpret_cast<CustomButton*>(
								SendMessage(button, WM_GETWNDINSTANCE, 0, 0)
								);
						if (cButton != nullptr)
						{
							cButton->setColors(
								this->styleInfo.mainToolbarColor,
								makeSelectionColor(this->styleInfo.mainToolbarColor),
								makePressedColor(this->styleInfo.mainToolbarColor)
							);
							cButton->setBorder(TRUE, this->styleInfo.OutlineColor);
						}
					}
				}
			}
		}
		RedrawWindow(this->TCFrame, NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE);
	}
	else
		result = FALSE;

	return static_cast<LRESULT>(result);
}

LRESULT CnCS_TC::OnDrawItem_inFrame(LPARAM lParam)
{
	LPDRAWITEMSTRUCT pdi = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
	if (pdi != NULL)
	{
		switch (pdi->CtlID)
		{
		case IDM_UPPERCASE:
			if (this->iParam.uppercase_active)
				this->DrawTBButton(pdi, IDI_EDITTOOLBAR_UPPERCASE_GREEN);
			else
				this->DrawTBButton(pdi, IDI_EDITTOOLBAR_UPPERCASE_RED);
			break;
		case IDM_AUTOCOMPLETE:
			if (this->iParam.autocomplete_active)
				this->DrawTBButton(pdi, IDI_EDITTOOLBAR_AUTOCOMPLETE_GREEN);
			else
				this->DrawTBButton(pdi, IDI_EDITTOOLBAR_AUTOCOMPLETE_RED);
			break;
		case IDM_AUTONUM:
			if (this->iParam.autonum_active)
				this->DrawTBButton(pdi, IDI_EDITTOOLBAR_AUTONUM_GREEN);
			else
				this->DrawTBButton(pdi, IDI_EDITTOOLBAR_AUTONUM_RED);
			break;
		case IDM_TEXTCLR:
			if (this->iParam.textcolor_active)
				this->DrawTBButton(pdi, IDI_EDITTOOLBAR_TEXTCOLOR_GREEN);
			else
				this->DrawTBButton(pdi, IDI_EDITTOOLBAR_TEXTCOLOR_RED);
			break;
		case IDM_FOCUSRECT:
			if (this->iParam.focusmark_active)
				this->DrawTBButton(pdi, IDI_EDITTOOLBAR_FOCUSMARK_GREEN);
			else
				this->DrawTBButton(pdi, IDI_EDITTOOLBAR_FOCUSMARK_RED);
			break;
		case IDM_CTLPROPWND:
			if (this->iParam.descWnd_visible)
				this->DrawTBButton(pdi, IDI_EDITTOOLBAR_SHOWDESC);
			else
				this->DrawTBButton(pdi, IDI_EDITTOOLBAR_HIDEDESC);
			break;
		case IDM_NUMSEQUENCE:
			this->DrawTBButton(pdi, IDI_EDITTOOLBAR_NUMSEQUENCE);
			break;
		case IDM_ERRORCHECK:
			this->DrawTBButton(pdi, IDI_EDITTOOLBAR_ERRORCHECK);
			break;
		case IDM_ECONVERTALL:
			this->DrawTBButton(pdi, IDI_EDITTOOLBAR_RECONVERT);
			break;
		case IDM_AUTOSYNTAX:
			this->DrawTBButton(pdi, IDI_EDITTOOLBAR_AUTOSYNTAX);
			break;
		case IDM_FONTPROPERTIES:
			this->DrawTBButton(pdi, IDI_EDITTOOLBAR_TEXTPROPERTIES);
			break;
		case IDM_VT_UNDO:
			this->DrawTBButton(pdi, IDI_TCVT_UNDO);
			break;
		case IDM_VT_REDO:
			this->DrawTBButton(pdi, IDI_TCVT_REDO);
			break;
		default:
			break;

		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnValidateError(LPARAM lParam)
{
	BOOL ncsDelete = FALSE;

	LPEDSPSTRUCT pedsp = reinterpret_cast<LPEDSPSTRUCT>(lParam);
	if (pedsp != NULL)
	{
		if (pedsp->type == EDSP_INFO)
			ncsDelete = FALSE;
		else
		{

		}
	}
	return static_cast<LRESULT>(ncsDelete);
}

LRESULT CnCS_TC::OnPaint_inDescWnd(HWND descWnd)
{
	RECT rc;
	HDC hdc;
	int charcount = 0;
	PAINTSTRUCT ps;
	TCHAR *heading = getStringFromResource(UI_TABCTRL_DOCUMENTPROPERTIES);

	GetClientRect(descWnd, &rc);

	hdc = BeginPaint(descWnd, &ps);

	FillRect(hdc, &rc, this->tcObj.tabbkgnd);

	charcount = GetCharCount(this->iParam.DESC_property1);
	if (charcount != -1)
	{
		SIZE sz;

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, this->tcObj.Textcolor);

		HGDIOBJ original = SelectObject(hdc, this->tcObj.desc_propertyfont);

		GetTextExtentPoint32(hdc, this->iParam.DESC_property1, charcount, &sz);
		TextOut(hdc, (rc.right / 2) - (sz.cx / 2), DPIScale(70), this->iParam.DESC_property1, charcount);

		charcount = GetCharCount(this->iParam.DESC_property2);
		if (charcount != -1)
		{
			GetTextExtentPoint32(hdc, this->iParam.DESC_property2, charcount, &sz);
			TextOut(hdc, (rc.right / 2) - (sz.cx / 2), DPIScale(140), this->iParam.DESC_property2, charcount);

			charcount = GetCharCount(this->iParam.DESC_property3);
			if (charcount != -1)
			{
				GetTextExtentPoint32(hdc, this->iParam.DESC_property3, charcount, &sz);
				TextOut(hdc, (rc.right / 2) - (sz.cx / 2), DPIScale(210), this->iParam.DESC_property3, charcount);

				auto infoDrawn = 
					this->displayFileInfos(
						hdc,
						DPIScale(FILEINFOAREA_STARTHEIGHT + 32)
					);

				charcount = GetCharCount(heading);
				if (charcount != -1)
				{
					SelectObject(hdc, this->tcObj.desc_propertyfont);

					SetTextColor(hdc, this->styleInfo.specialTextcolor);//RGB(255, 128, 64));

					GetTextExtentPoint32(hdc, heading, charcount, &sz);
					TextOut(hdc, DPIScale(10), DPIScale(10), heading, charcount);

					if (infoDrawn)
					{
						charcount = GetCharCount(
							getStringFromResource(UI_FILEINFO)
						);
						GetTextExtentPoint32(hdc, getStringFromResource(UI_FILEINFO), charcount, &sz);
						TextOut(hdc, DPIScale(18), DPIScale(FILEINFOAREA_STARTHEIGHT + 3), getStringFromResource(UI_FILEINFO), charcount);
					}

					SelectObject(hdc, this->tcObj.cPen);
					MoveToEx(hdc, rc.right, DPIScale(32), NULL);
					LineTo(hdc, 0, DPIScale(32));
					LineTo(hdc, 0, rc.bottom);
					MoveToEx(hdc, rc.right, DPIScale(98), NULL);
					LineTo(hdc, 0, DPIScale(98));

					MoveToEx(hdc, rc.right, DPIScale(66), NULL);
					LineTo(hdc, 0, DPIScale(66));

					MoveToEx(hdc, rc.right, DPIScale(168), NULL);
					LineTo(hdc, 0, DPIScale(168));
					MoveToEx(hdc, rc.right, DPIScale(238), NULL);
					LineTo(hdc, 0, DPIScale(238));

					if (infoDrawn)
					{
						if (rc.right > DPIScale(300))
						{
							MoveToEx(
								hdc,
								rc.right - DPIScale(16),
								rc.bottom,
								nullptr
							);
							LineTo(
								hdc,
								rc.right - DPIScale(16),
								DPIScale(FILEINFOAREA_STARTHEIGHT + 1)
							);
						}
						else
						{
							MoveToEx(
								hdc,
								rc.right,
								DPIScale(FILEINFOAREA_STARTHEIGHT + 1),
								nullptr
							);
						}
						LineTo(
							hdc,
							DPIScale(20) + sz.cx,
							DPIScale(FILEINFOAREA_STARTHEIGHT + 1)
						);
						LineTo(
							hdc,
							DPIScale(20) + sz.cx,
							DPIScale(FILEINFOAREA_STARTHEIGHT + sz.cy + 2)
						);

						LineTo(
							hdc,
							DPIScale(16),
							DPIScale(FILEINFOAREA_STARTHEIGHT + sz.cy + 2)
						);
						LineTo(
							hdc,
							DPIScale(16),
							rc.bottom
						);
					}
				}
			}
		}
		SelectObject(hdc, original);		
	}
	EndPaint(descWnd, &ps);

	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnSize_inPropWnd(HWND propWnd)
{
	RECT rc;
	GetClientRect(propWnd, &rc);

	MoveWindow(
		GetDlgItem(propWnd, ID_PROPERTYEDIT_ONE),
		DPIScale(2),
		DPIScale(100),
		rc.right - DPIScale(2),
		DPIScale(30),
		TRUE
	);
	MoveWindow(
		GetDlgItem(propWnd, ID_PROPERTYEDIT_TWO),
		DPIScale(2),
		DPIScale(170),
		rc.right - DPIScale(2),
		DPIScale(30),
		TRUE
	);
	MoveWindow(
		GetDlgItem(propWnd, ID_PROPERTYEDIT_THREE),
		DPIScale(2),
		DPIScale(240),
		rc.right - DPIScale(2),
		DPIScale(100),
		TRUE
	);

	// resize buttons???

	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnNotify_inFrame(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LPTABPROPERTY ptb
		= reinterpret_cast<LPTABPROPERTY>(
			GetWindowLongPtr(this->iParam.hwndActiveTab, GWLP_USERDATA)
		);
	if (ptb != nullptr)
	{
		return static_cast<LRESULT>(
			ptb->Editcontrol->OnEditNotify(hWnd, wParam, lParam)
		);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnGetEditWndPos(LPARAM lParam)
{
	LPRECT pRc = reinterpret_cast<LPRECT>(lParam);
	if (pRc != nullptr)
	{
		auto ptp = this->GetActiveTabProperty();
		if (ptp != nullptr)
		{
			GetWindowRect(
				ptp->AssocEditWnd, pRc
			);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnMouseLeave_inTab(HWND tab)
{
	auto ptp =
		reinterpret_cast<LPTABPROPERTY>(
			GetWindowLongPtr(tab, GWLP_USERDATA)
			);

	if (ptp != nullptr)
	{
		if (!ptp->Closebutton_highl)
		{
			ptp->mouseHover = FALSE;
			RedrawWindow(tab, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
		}
		else
		{
			ptp->mouseHover = 2;// this means that the mouse is already inside the tab
								// but the mousetracker is offline due to entering the close-button area
								// the tracker must be reactivated to keep the desired painting behavior
		}

		if (ptp->menuButtonHover)
		{
			ptp->menuButtonHover = FALSE;
			RedrawWindow(tab, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
		}
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_TC::OnPaintInToolPopup(HWND hWnd)
{
	HDC hdc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(hWnd, &ps);
	if (hdc)
	{
		RECT rc;
		GetClientRect(hWnd, &rc);

		FillRect(hdc, &rc, this->tcObj.framebkgnd);

		EndPaint(hWnd, &ps);
	}
	return static_cast<LRESULT>(0);
}

void CnCS_TC::OnVerticalToolBar_Textcolor()
{
	LPTABPROPERTY ptb = reinterpret_cast<LPTABPROPERTY>(GetWindowLongPtr(this->iParam.hwndActiveTab, GWLP_USERDATA));
	if (ptb != NULL)
	{
		this->iParam.textcolor_active = (!this->iParam.textcolor_active);

		ptb->Editcontrol->SetAutoColor(this->iParam.textcolor_active);
		RedrawWindow(
			GetDlgItem(
				this->TCFrame, IDM_TEXTCLR),
			NULL, NULL, RDW_NOERASE | RDW_NOCHILDREN | RDW_INVALIDATE);

		auto dataContainer
			= reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer()
			);
		if (dataContainer != nullptr)
		{
			dataContainer->saveValue(DATAKEY_EDITOR_TEXTCOLOR, this->iParam.textcolor_active);
		}
		SetFocus(ptb->AssocEditWnd);
	}	
}

void CnCS_TC::OnVerticalToolBar_Autocomplete()
{
	auto ptb = this->GetActiveTabProperty();
	if (ptb != NULL)
	{
		this->iParam.autocomplete_active = (!this->iParam.autocomplete_active);
		ptb->Editcontrol->SetAutoComplete(this->iParam.autocomplete_active);
		RedrawWindow(
			GetDlgItem(
				this->TCFrame, IDM_AUTOCOMPLETE),
			NULL, NULL, RDW_NOERASE | RDW_NOCHILDREN | RDW_INVALIDATE);

		auto dataContainer
			= reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer()
				);
		if (dataContainer != nullptr)
		{
			dataContainer->saveValue(DATAKEY_EDITOR_AUTOCOMPLETE, this->iParam.autocomplete_active);
		}
		SetFocus(ptb->AssocEditWnd);
	}
}

void CnCS_TC::OnVerticalToolBar_Autonum()
{
	auto ptb = this->GetActiveTabProperty();
	if (ptb != NULL)
	{
		this->iParam.autonum_active = (!this->iParam.autonum_active);
		ptb->Editcontrol->SetAutoNum(this->iParam.autonum_active, -1);
		RedrawWindow(
			GetDlgItem(
				this->TCFrame, IDM_AUTONUM),
			NULL, NULL, RDW_NOERASE | RDW_NOCHILDREN | RDW_INVALIDATE);

		auto dataContainer
			= reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer()
				);
		if (dataContainer != nullptr)
		{
			dataContainer->saveValue(DATAKEY_EDITOR_AUTONUM, this->iParam.autonum_active);
		}
		SetFocus(ptb->AssocEditWnd);
	}
}

void CnCS_TC::OnVerticalToolBar_Uppercase()
{
	auto ptb = this->GetActiveTabProperty();
	if (ptb != NULL)
	{
		this->iParam.uppercase_active = (!this->iParam.uppercase_active);
		ptb->Editcontrol->SetUppercase(this->iParam.uppercase_active);
		RedrawWindow(
			GetDlgItem(
				this->TCFrame, IDM_UPPERCASE),
			NULL, NULL, RDW_NOERASE | RDW_NOCHILDREN | RDW_INVALIDATE);

		auto dataContainer
			= reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer()
				);
		if (dataContainer != nullptr)
		{
			dataContainer->saveValue(DATAKEY_EDITOR_UPPERCASE, this->iParam.uppercase_active);
		}
		SetFocus(ptb->AssocEditWnd);
	}
}

void CnCS_TC::OnVerticalToolBar_FocusRect()
{
	auto ptb = this->GetActiveTabProperty();
	if (ptb != NULL)
	{
		this->iParam.focusmark_active = (!this->iParam.focusmark_active);

		ptb->Editcontrol->SetFocusMark(this->iParam.focusmark_active);
		RedrawWindow(
			GetDlgItem(
				this->TCFrame, IDM_FOCUSRECT),
			NULL, NULL, RDW_NOERASE | RDW_NOCHILDREN | RDW_INVALIDATE);

		auto dataContainer
			= reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer()
				);
		if (dataContainer != nullptr)
		{
			dataContainer->saveValue(DATAKEY_EDITOR_FOCUSMARK, this->iParam.focusmark_active);
		}
		SetFocus(ptb->AssocEditWnd);
	}
}

void CnCS_TC::OnVerticalToolBar_eConvertAll()
{
	auto ptp = this->GetActiveTabProperty();
	if (ptp != nullptr)
	{
		ptp->Editcontrol->ConvertContent(
			ptp->Editcontrol->conversionFlagsFromObjectSetup()
		);
		SetFocus(ptp->AssocEditWnd);
	}
}

void CnCS_TC::OnVerticalToolBar_AutoSyntax()
{
	//TCHAR path[] = L"C:\\Users\\hans-\\Documents\\CnC Suite\\Projects\\Allgemein\\";

	//TCHAR path_to_substitute[] = L"C:\\Users\\hans-\\Documents\\CnC Suite";
	//TCHAR substitution[] = L"C:\\NewPath\\SuperNew\\New";

	//TCHAR str1[] = L"Das ist zum Testen!";

	//CHARSCOPE cs;
	//iString ipath(str1);

	//if (ipath.Contains(L"Das", &cs))
	//{
	//	ipath.SetSegment(L"Was", &cs);
	//}

	//MessageBox(NULL, ipath.GetData(), L"path action", MB_OK);

	//auto ptp = this->GetActiveTabProperty();
	//MessageBox(NULL, ptp->Path, L"Tab-Path", MB_OK);


	//SendMessage(this->Main, WM_COMMAND, MAKEWPARAM(ICOMMAND_TVREFRESH, 0), 0);

}

void CnCS_TC::OnVerticalToolBar_NumSequence()
{
	auto ptp = this->GetActiveTabProperty();
	if (ptp != nullptr)
	{
		ptp->Editcontrol->ConvertContent(HIDE_WHILE_CONVERTING | CONVERT_SYNTAX | CONVERT_CUR_COLORING | CONVERT_ADDTO_UNDOSTACK);
		ptp->Content_Changed = TRUE;
		this->RedrawTab_s(REDRAW_CURRENT);

		SetFocus(ptp->AssocEditWnd);
	}

}

void CnCS_TC::OnVerticalToolBar_ErrorCheck()
{
	CnC3FileManager mngr(this->Main);

	auto files = mngr.Open();
	if (files.Succeeded())
	{
		if (files.GetCount() > 0)
		{
			auto singleFile =
				files.GetAt(0);

			if (singleFile.Succeeded())
			{

			}
		}
	}


	//auto ptp = this->GetActiveTabProperty();
	//if (ptp != nullptr)
	//{
	//	TCHAR *buffer = nullptr;

	//	CnC3File file;
	//	file.AddProperty(CnC3File::PID_DESCRIPTION_ONE, ptp->DESC1);
	//	file.AddProperty(CnC3File::PID_DESCRIPTION_TWO, ptp->DESC2);
	//	file.AddProperty(CnC3File::PID_DESCRIPTION_THREE, ptp->DESC3);

	//	if (ptp->Editcontrol->GetTextContent(&buffer) > 0)
	//	{
	//		file.SetNCContent(buffer);

	//		CnC3FileManager mngr(this->Main);
	//		mngr.SetDialogText(L"Speichern unter", L"Speichern", L"myCnC3");

	//		auto savedFile = mngr.SaveAs(file);
	//		if (savedFile.Succeeded())
	//		{

	//		}
	//		SafeDeleteArray(&buffer);
	//	}
	//}
	//Xmap<iString, int> testmap;

	//testmap.Add(L"newkey123", 123);
	//testmap.Add(L"key987", 987);
	//testmap.Add(L"key456", 456);


	//auto data = testmap.GetDataForKey(L"key987");
	//if (data)
	//{
	//	testmap.Remove(L"key987");
	//}

	//TCHAR *folder = nullptr;
	//CnC3FileManager fMngr;

	//auto ptp = this->GetActiveTabProperty();
	//CopyStringToPtr(ptp->Path, &folder);

	//CreateBasicFPO()->RemoveFilenameFromPath(folder);

	//fMngr.SetDialogText(L"Testdialog", L"Öffnen", L"file");
	//fMngr.SetTargetFolder(folder);

	//auto files = fMngr.Open();
	//auto numFiles = files.GetCount();
	//if (numFiles > 0)
	//{
	//	for (int i = 0; i < numFiles; i++)
	//	{
	//		auto file = files.GetAt(i);

	//		MessageBox(this->Main, file.GetPath(), L"test", MB_OK);
	//	}
	//}

	//SYSTEMTIME past;
	//SYSTEMTIME present;
	//SYSTEMTIME future;

	//past.wDay = 28;
	//past.wHour = 23;
	//past.wMinute = 01;
	//past.wSecond = 45;
	//past.wMonth = 2;
	//past.wYear = 2005;
	//past.wMilliseconds = 422;

	//// 28.2.2005 23:01:45 422

	//present.wDay = 5;
	//present.wHour = 9;
	//present.wMinute = 28;
	//present.wMonth = 3;
	//present.wSecond = 21;
	//present.wYear = 2018;
	//present.wMilliseconds = 763;

	// 5.3.2018 9:28:21 763

	//future.wDay = 26;
	//future.wHour = 10;
	//future.wMinute = 0;
	//future.wMonth = 10;
	//future.wSecond = 0;
	//future.wYear = 2018;

	// 26.10.2018 10:00:00 000

	//DateTime _past_(&past);
	//DateTime _pres2_(&present);
	//DateTime _future_(&future);

	//if (_past_ < _pres1_)
	//{
	//	MessageBox(this->Main, L"past < pres1", L"test", MB_OK);
	//}

	//if (_past_ < _future_)
	//{
	//	MessageBox(this->Main, L"past < future", L"test", MB_OK);
	//}

	//if (_pres1_ == _pres2_)
	//{
	//	MessageBox(this->Main, L"pres1 == pres2", L"test", MB_OK);
	//}

	//if (_future_ > _pres2_)
	//{
	//	MessageBox(this->Main, L"future > pres2", L"test", MB_OK);
	//}

	//TimeSpan span;
	//span.FromTimeToTime(&present, &past);
	//auto srep = span.ToString();

	//MessageBox(this->Main, srep, L"test 1", MB_OK);

	//span.Clear();
	//span.FromTimeToTime(&past, &present);
	//srep = span.ToString();

	//MessageBox(this->Main, srep, L"test 2", MB_OK);
}

void CnCS_TC::OnVerticalToolBar_Undo()
{
	auto ptp = this->GetActiveTabProperty();
	if (ptp != nullptr)
	{
		ptp->Editcontrol->Undo();
		SetFocus(ptp->AssocEditWnd);
	}
}

void CnCS_TC::OnVerticalToolBar_Redo()
{
	auto ptp = this->GetActiveTabProperty();
	if (ptp != nullptr)
	{
		ptp->Editcontrol->Redo();
		SetFocus(ptp->AssocEditWnd);
	}
}

BOOL CnCS_TC::ADD_Tab(LPCTSTR Path)
{
	RECT rc;
	BOOL result = TRUE;
	POINT pt;
	GetClientRect(this->TCFrame, &rc);

	// get the absoluteRange of the new tab
	result = this->CalculateInsertTabrange(&pt);
	if (result)
	{
		// check if the maximum number of tabs is reached
		if ((result == TAB_MAXIMUM) && (this->TABCount > 0))
		{
			TCHAR text[100] = { 0 };
			StringCbPrintf(text, sizeof(text), L"Tab %i", this->TABCount + 1);

			DispatchEWINotification(EDSP_INFO, L"TC0001", getStringFromResource(INFO_MSG_MAXIMUMTABCOUNTREACHED), text);

			return TRUE;
		}
		// create the tab itself
		HWND Tab =
			CreateWindow(
				L"CSTABCLASS",
				NULL,
				WS_CHILD | WS_VISIBLE,
				pt.x,
				DPIScale(2),
				pt.y - pt.x,
				DPIScale(25),
				this->TCFrame,
				reinterpret_cast<HMENU>(MAKECTRLID(TAB_ID_START + this->TABCount)),			//800 + ( zero based index ) 
				this->hInstance,
				this
			);

		if (!Tab)
			return FALSE;
		else
		{
			this->iParam.ActiveTabRange.x = pt.x;
			this->iParam.ActiveTabRange.y = pt.y;

			// allocate the tab-property and store it in the tab-window
			LPTABPROPERTY tabProperty = new TABPROPERTY;
			SecureZeroMemory(tabProperty, sizeof(TABPROPERTY));

			tabProperty->toClass = reinterpret_cast<LONG_PTR>(this);
			tabProperty->TabIndex = this->TABCount;						// zero based index
			tabProperty->Content_Changed = FALSE;
			tabProperty->mouseHover = FALSE;
			tabProperty->Tab = Tab;

			if (Path != NULL)
			{
				CopyStringToPtr(Path, &tabProperty->Path);
				GetFilenameOutOfPath(Path, &tabProperty->displayname, TRUE);
			}
			else
			{
				CopyStringToPtr(getStringFromResource(UI_TABCTRL_NEWTAB), &tabProperty->displayname);
			}
			SetWindowLongPtr(Tab, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(tabProperty));

			RECT rc_tab;
			GetClientRect(Tab, &rc_tab);

			// create the close-button
			HWND button =
				CreateWindow(
					L"BUTTON",
					nullptr,
					WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
					rc_tab.right - DPIScale(28),
					rc_tab.bottom - DPIScale(21),
					DPIScale(16),
					DPIScale(16),
					Tab,
					(HMENU)ID_TABCLOSE,
					this->hInstance,
					nullptr
				);

			if (!button)
				return FALSE;
			else
			{
				// subclass the close-button
				result = SetWindowSubclass(button, CnCS_TC::CloseBtnSub, NULL, reinterpret_cast<DWORD_PTR>(tabProperty));
				if (result)
				{
					if (this->TABCount > 0)
					{
						// if another tab will be hidden make sure to save the visible data to the tab-property
						LPTABPROPERTY ptp_old =
							reinterpret_cast<LPTABPROPERTY>(
								GetWindowLongPtr(this->iParam.hwndActiveTab, GWLP_USERDATA)
							);

						if (ptp_old != NULL)
						{
							this->SaveDescritpionsToTabProperty(ptp_old);
							ShowWindow(ptp_old->AssocEditWnd, SW_HIDE);
						}
					}
					// create the associated editwindow
					tabProperty->AssocEditWnd =
						CreateWindowEx(
							0,
							MSFTEDIT_CLASS,
							nullptr,
							WS_CHILD | WS_CLIPSIBLINGS | ES_MULTILINE | ES_NOHIDESEL | ES_SELECTIONBAR | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL,
							0, 0, 0, 0,
							this->TCFrame,
							IDTOHMENU(TABEDIT_ID_START + this->TABCount),//reinterpret_cast<HMENU>(MAKECTRLID(TABEDIT_ID_START + this->TABCount)),
							this->hInstance,
							nullptr
						);

					if (!tabProperty->AssocEditWnd)
						return FALSE;
					else
					{
						// create the editcontrol instance and connect it to the rich-edit-box
						tabProperty->Editcontrol = CreateEditControlInstance(tabProperty->AssocEditWnd, Tab);

						result = (tabProperty->Editcontrol != nullptr) ? TRUE : FALSE;
						if (result)
						{
							// config the editcontrol object >>

							EDITCONTROLPROPERTIES ecs;
							this->SetEditControlProperties(&ecs);// includes the autosyntax settings...!

							// this could reduce efficiency????
							iString subprog, endprog, newline;
							autosyntaxManager::GetTriggerStrings(subprog, endprog, newline);
							tabProperty->Editcontrol->insertUserdefinedTriggerStrings(
								subprog.GetData(),
								endprog.GetData(),
								newline.GetData()
							);						

							result = tabProperty->Editcontrol->ConfigureComponent(&this->editStyleColors, &ecs, TRUE, TRUE);
							if (result)
							{
								auto appProperties
									= reinterpret_cast<CnCSuite_Property*>(
										getApplicationPropertyComponent()
									);

								if (appProperties != nullptr)
								{
									int cnt = 0;
									auto acStrings
										= reinterpret_cast<LPAUTOCOMPLETESTRINGS>(
											appProperties->GetAutocompleteStrings(&cnt)
										);
									tabProperty->Editcontrol->SetAutocompleteStrings(acStrings, cnt);
								}

								int Ewidth = rc.right - DPIScale(40);
								if (this->iParam.descWnd_visible)	// hold free section for desc window
									Ewidth = 2 * (Ewidth / 3);

								ShowScrollBar(tabProperty->AssocEditWnd, SB_BOTH, FALSE);

								SetWindowPos(
									tabProperty->AssocEditWnd,
									NULL,
									DPIScale(40),
									DPIScale(40),
									Ewidth,
									rc.bottom - DPIScale(40),
									SWP_NOZORDER | SWP_SHOWWINDOW
								);

								// load a path (if exist) or default values (no path)
								if (Path != nullptr)
								{
									CnC3File file;
									auto hr =
										file.Load(Path);

									if (SUCCEEDED(hr))
									{
										this->iParam.hwndActiveTab = Tab;

										this->OpenDisplayAndFormat(file, NO_EXECUTE);

										this->updateFileInfoArea();
									}
									else
									{
										iString errorMsg(
											getStringFromResource(ERROR_MSG_CNC3FILEOBJECT_ERROR)
										);
										errorMsg += iString::fromHex((uintX)hr);

										DispatchEWINotification(
											EDSP_ERROR,
											L"TC0005",
											errorMsg.GetData(),
											L"Method::ADD_Tab"
										);
									}
								}
								else
								{
									// its an empty tab, so set placeholder descriptions and default text if desired
									this->SetDescriptions(L"...", L"...", L"...");
									this->initDescriptionBuffer(tabProperty);

									this->eraseFileInfoArea(nullptr);

									auto dataContainer =
										reinterpret_cast<ApplicationData*>(
											getDefaultApplicationDataContainer()
										);
									if (dataContainer != nullptr)
									{
										auto insertDefaultText =
											dataContainer->getBooleanData(DATAKEY_SETTINGS_INSERTDEFAULTTEXT, true);

										if (insertDefaultText)
										{
											this->SetTabContentFocusAndPushCaret(
												tabProperty,
												this->getDefaultInsertText(),
												TRUE,
												CARET_TO_END_OF_TEXT
											);	
										}
									}
								}
								tabProperty->Editcontrol->UpdateFocusRect();
							}
						}
					}
				}
			}
			// update the internal control values
			this->TABCount++;
			this->iParam.previousTab = this->iParam.dwActiveTab;
			this->iParam.dwActiveTab = tabProperty->TabIndex;
			this->iParam.hwndActiveTab = Tab;

			this->RefreshTabAlignment();
		}
	}
	return result;
}

BOOL CnCS_TC::AddTab(const CnC3File & file)
{
	RECT rc;
	BOOL result = TRUE;
	POINT pt;
	GetClientRect(this->TCFrame, &rc);

	// get the absoluteRange of the new tab
	result = this->CalculateInsertTabrange(&pt);
	if (result)
	{
		// check if the maximum number of tabs is reached
		if ((result == TAB_MAXIMUM) && (this->TABCount > 0))
		{
			TCHAR text[100] = { 0 };
			StringCbPrintf(text, sizeof(text), L"Tab %i", this->TABCount + 1);

			DispatchEWINotification(EDSP_INFO, L"TC0001", getStringFromResource(INFO_MSG_MAXIMUMTABCOUNTREACHED), text);

			return TRUE;
		}

		// create the tab itself
		HWND Tab =
			CreateWindow(
				L"CSTABCLASS",
				NULL,
				WS_CHILD | WS_VISIBLE,
				pt.x,
				DPIScale(2),
				pt.y - pt.x,
				DPIScale(25),
				this->TCFrame,
				reinterpret_cast<HMENU>(MAKECTRLID(TAB_ID_START + this->TABCount)),			//800 + ( zero based index ) 
				this->hInstance,
				this
			);

		if (!Tab)
			return FALSE;
		else
		{
			this->iParam.ActiveTabRange.x = pt.x;
			this->iParam.ActiveTabRange.y = pt.y;

			// allocate the tab-property and store it in the tab-window
			LPTABPROPERTY tabProperty = new TABPROPERTY;
			SecureZeroMemory(tabProperty, sizeof(TABPROPERTY));

			tabProperty->toClass = reinterpret_cast<LONG_PTR>(this);
			tabProperty->TabIndex = this->TABCount;						// zero based index
			tabProperty->Content_Changed = FALSE;
			tabProperty->mouseHover = FALSE;
			tabProperty->Tab = Tab;

			if (file.HasPath())
			{
				CopyStringToPtr(
					file.GetPath(),
					&tabProperty->Path
				);
				GetFilenameOutOfPath(
					file.GetPath(),
					&tabProperty->displayname,
					TRUE
				);
			}
			else
			{
				CopyStringToPtr(
					getStringFromResource(UI_TABCTRL_NEWTAB),
					&tabProperty->displayname
				);
			}
			SetWindowLongPtr(Tab, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(tabProperty));

			RECT rc_tab;
			GetClientRect(Tab, &rc_tab);

			// create the close-button
			HWND button =
				CreateWindow(
					L"BUTTON",
					nullptr,
					WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
					rc_tab.right - DPIScale(28),
					rc_tab.bottom - DPIScale(21),
					DPIScale(16),
					DPIScale(16),
					Tab,
					(HMENU)ID_TABCLOSE,
					this->hInstance,
					nullptr
				);

			if (!button)
				return FALSE;
			else
			{
				// subclass the close-button
				result = SetWindowSubclass(button, CnCS_TC::CloseBtnSub, NULL, reinterpret_cast<DWORD_PTR>(tabProperty));
				if (result)
				{
					if (this->TABCount > 0)
					{
						// if another tab will be hidden make sure to save the visible data to the tab-property
						LPTABPROPERTY ptp_old =
							reinterpret_cast<LPTABPROPERTY>(
								GetWindowLongPtr(this->iParam.hwndActiveTab, GWLP_USERDATA)
								);

						if (ptp_old != NULL)
						{
							this->SaveDescritpionsToTabProperty(ptp_old);
							ShowWindow(ptp_old->AssocEditWnd, SW_HIDE);
						}
					}
					// create the associated editwindow
					tabProperty->AssocEditWnd =
						CreateWindowEx(
							0,
							MSFTEDIT_CLASS,
							nullptr,
							WS_CHILD | WS_CLIPSIBLINGS | ES_MULTILINE | ES_NOHIDESEL | ES_SELECTIONBAR | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL,
							0, 0, 0, 0,
							this->TCFrame,
							IDTOHMENU(TABEDIT_ID_START + this->TABCount),//reinterpret_cast<HMENU>(MAKECTRLID(TABEDIT_ID_START + this->TABCount)),
							this->hInstance,
							nullptr
						);

					if (!tabProperty->AssocEditWnd)
						return FALSE;
					else
					{
						// create the editcontrol instance and connect it to the rich-edit-box
						tabProperty->Editcontrol = CreateEditControlInstance(tabProperty->AssocEditWnd, Tab);

						result = (tabProperty->Editcontrol != nullptr) ? TRUE : FALSE;
						if (result)
						{
							// config the editcontrol object >>

							EDITCONTROLPROPERTIES ecs;
							this->SetEditControlProperties(&ecs);// includes the autosyntax settings...!

							// this could reduce efficiency????
							iString subprog, endprog, newline;
							autosyntaxManager::GetTriggerStrings(subprog, endprog, newline);
							tabProperty->Editcontrol->insertUserdefinedTriggerStrings(
								subprog.GetData(),
								endprog.GetData(),
								newline.GetData()
							);

							result = tabProperty->Editcontrol->ConfigureComponent(&this->editStyleColors, &ecs, TRUE, TRUE);
							if (result)
							{
								auto appProperties
									= reinterpret_cast<CnCSuite_Property*>(
										getApplicationPropertyComponent()
										);

								if (appProperties != nullptr)
								{
									int cnt = 0;
									auto acStrings
										= reinterpret_cast<LPAUTOCOMPLETESTRINGS>(
											appProperties->GetAutocompleteStrings(&cnt)
											);
									tabProperty->Editcontrol->SetAutocompleteStrings(acStrings, cnt);
								}

								int Ewidth = rc.right - DPIScale(40);
								if (this->iParam.descWnd_visible)	// hold free section for desc window
									Ewidth = 2 * (Ewidth / 3);

								ShowScrollBar(tabProperty->AssocEditWnd, SB_BOTH, FALSE);

								SetWindowPos(
									tabProperty->AssocEditWnd,
									NULL,
									DPIScale(40),
									DPIScale(40),
									Ewidth,
									rc.bottom - DPIScale(40),
									SWP_NOZORDER | SWP_SHOWWINDOW
								);

								// load a path (if exist) or default values (no path)
								if (file.HasPath())
								{
									this->iParam.hwndActiveTab = Tab;

									this->OpenDisplayAndFormat(file, NO_EXECUTE);

									this->updateFileInfoArea();
								}
								else
								{
									// its an empty tab, so set placeholder descriptions and default text if desired
									this->SetDescriptions(L"...", L"...", L"...");
									this->initDescriptionBuffer(tabProperty);

									this->eraseFileInfoArea(nullptr);

									auto dataContainer =
										reinterpret_cast<ApplicationData*>(
											getDefaultApplicationDataContainer()
											);
									if (dataContainer != nullptr)
									{
										auto insertDefaultText =
											dataContainer->getBooleanData(DATAKEY_SETTINGS_INSERTDEFAULTTEXT, true);

										if (insertDefaultText)
										{
											this->SetTabContentFocusAndPushCaret(
												tabProperty,
												this->getDefaultInsertText(),
												TRUE,
												CARET_TO_END_OF_TEXT
											);
										}
									}
								}
								tabProperty->Editcontrol->UpdateFocusRect();
							}
						}
					}
				}
			}
			// update the internal control values
			this->TABCount++;
			this->iParam.previousTab = this->iParam.dwActiveTab;
			this->iParam.dwActiveTab = tabProperty->TabIndex;
			this->iParam.hwndActiveTab = Tab;

			this->RefreshTabAlignment();
		}
	}
	return result;
}

BOOL CnCS_TC::CLOSE_Tab(HWND Tab, BOOL performSavecheck)
{
	BOOL result;
	// delete tabinfo and content

	LPTABPROPERTY ptp = reinterpret_cast<LPTABPROPERTY>(
		GetWindowLongPtr(Tab, GWLP_USERDATA)
		);
	
	result = (ptp != NULL) ? TRUE : FALSE;
	if (result)
	{
		setFileAccessTime(ptp->Path);

		if ((this->TABCount == 1) && (ptp->Path == nullptr)) // there is only one tab without a valid path -> notify the user
		{
			DispatchEWINotification(
				EDSP_INFO,
				L"TC0002\0",
				getStringFromResource(INFO_MSG_LASTTABNOCLOSE),
				getStringFromResource(UI_TABCONTROL)
			);

			return TRUE;
		}
		else if ((this->TABCount == 1) && (ptp->Path != nullptr))// there is only one tab with a valid path -> save-check + erase
		{
			auto erase =
				performSavecheck
				? this->SaveCheck_singleTab(ptp) : TRUE;

			if (erase)
			{
				this->eraseTab(ptp);
				ptp->Editcontrol->SetCaret(CARET_TO_END_OF_TEXT);
				SetFocus(ptp->AssocEditWnd);
			}
			return TRUE;
		}
		else // handle multiple tabs
		{
			auto close =
				performSavecheck
				? this->SaveCheck_singleTab(ptp) : TRUE;

			if (close)
			{
				HWND nextTab;
				LPTABPROPERTY nextPtp;
				int TabID = static_cast<int>(GetWindowLongPtr(Tab, GWLP_ID));

				SafeRelease(&ptp->Editcontrol);
				SafeDeleteArray(&ptp->Path);
				SafeDeleteArray(&ptp->displayname);

				if (this->iParam.hwndActiveTab == Tab)
					this->iParam.hwndActiveTab = NULL;
				else
				{
					if (ptp->TabIndex > this->iParam.dwActiveTab)
					{
						this->iParam.ChangeActiveTab = FALSE;
					}
					else
					{
						this->iParam.ChangeActiveTab = TRUE;
					}
				}
				DestroyWindow(ptp->AssocEditWnd);

				SafeDelete(&ptp);
				SetWindowLongPtr(Tab, GWLP_USERDATA, 0);

				DestroyWindow(Tab);

				if (this->TABCount > 0)
					this->TABCount--;

				while ((nextTab = GetDlgItem(this->TCFrame, TabID + 1)) != NULL)
				{
					nextPtp = reinterpret_cast<LPTABPROPERTY>(GetWindowLongPtr(nextTab, GWLP_USERDATA));
					if (nextPtp != NULL)
					{
						nextPtp->TabIndex--;
					}
					SetWindowLongPtr(nextTab, GWLP_ID, (LONG)TabID);

					TabID++;
				}
				this->SetActiveTab();
				this->RefreshTabAlignment();

				this->GetActiveTabProperty()
					->Editcontrol
					->UpdateFocusRect();

				this->updateFileInfoArea();
			}
		}
	}
	return result;
}

void CnCS_TC::selectTab(DWORD tabNr)
{
	auto Tab = GetDlgItem(this->TCFrame, TAB_ID_START + tabNr);

	if (Tab != this->iParam.hwndActiveTab && Tab != nullptr)
	{
		LPTABPROPERTY ptp_old = reinterpret_cast<LPTABPROPERTY>(GetWindowLongPtr(this->iParam.hwndActiveTab, GWLP_USERDATA));
		if (ptp_old != NULL)
		{
			ShowWindow(ptp_old->AssocEditWnd, SW_HIDE);
			this->SaveDescritpionsToTabProperty(ptp_old);
		}
		this->iParam.previousTab = this->iParam.dwActiveTab;

		this->iParam.hwndActiveTab = Tab;

		LPTABPROPERTY ptp_new = reinterpret_cast<LPTABPROPERTY>(GetWindowLongPtr(Tab, GWLP_USERDATA));
		if (ptp_new != NULL)
		{
			this->iParam.dwActiveTab = ptp_new->TabIndex;
			/////////////////////////////////////////////
			if (!this->iParam.descWnd_isMoving)
			{
				RECT rc;
				GetClientRect(this->TCFrame, &rc);

				int commonwidth;

				if (this->iParam.descWnd_visible)
					commonwidth = 2 * ((rc.right - 40) / 3);
				else
					commonwidth = rc.right - 40;

				MoveWindow(ptp_new->AssocEditWnd, 40, 40, commonwidth, rc.bottom - 40, TRUE);
			}
			///////////////////////////////////////////////////////////////////////
			EDITCONTROLPROPERTIES pCP;
			this->SetEditControlProperties(&pCP);
			ptp_new->Editcontrol->UpdateProperties(&pCP, FALSE);
			///////////////////////////////////////////////////////////////////////
			ShowWindow(ptp_new->AssocEditWnd, SW_SHOW);
			this->SetDescriptions(ptp_new->DESC1, ptp_new->DESC2, ptp_new->DESC3);
		}
		this->RefreshTabAlignment();
		ptp_new->Editcontrol->UpdateFocusRect();

		this->updateFileInfoArea();
	}
	else
	{
		auto ptp = this->GetActiveTabProperty();
		if (ptp != nullptr)
		{
			SetFocus(ptp->AssocEditWnd);

			// update focus rect
			ptp->Editcontrol->UpdateFocusRect();
		}
	}
}

void CnCS_TC::eraseTab(DWORD tabNr)
{
	auto ptp = this->GetTabProperty(tabNr);
	if (ptp != nullptr)
	{
		this->eraseTab(ptp);
	}
}

void CnCS_TC::eraseTab(LPTABPROPERTY ptp)
{
	auto dataContainer =
		reinterpret_cast<ApplicationData*>(
			getDefaultApplicationDataContainer()
		);

	SafeDeleteArray((void**)&ptp->Path);
	SafeDeleteArray((void**)&ptp->displayname);
	ptp->Content_Changed = FALSE;

	if (dataContainer != nullptr)
	{
		auto insertDefaultText = dataContainer->getBooleanData(DATAKEY_SETTINGS_INSERTDEFAULTTEXT, true);

		ptp->Editcontrol->DeleteAllandReset(
			insertDefaultText
			? this->getDefaultInsertText()
			: nullptr	
		);
	}
	SafeDeleteArray((void**)&ptp->DESC1);
	SafeDeleteArray((void**)&ptp->DESC2);
	SafeDeleteArray((void**)&ptp->DESC3);

	this->SetDescriptions(L"...", L"...", L"...");
	this->eraseFileInfoArea(nullptr);

	CopyStringToPtr(
		getStringFromResource(UI_TABCTRL_NEWTAB),
		&ptp->displayname
	);

	RedrawWindow(ptp->Tab, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_NOERASE);
}

void CnCS_TC::Open(LPTSTR path, BOOL openInNewTab, BOOL setFocus)
{
	if (this->isPathAlreadyOpen(path))
	{
		DispatchEWINotification(
			EDSP_INFO,
			L"TC0004",
			getStringFromResource(INFO_MSG_FILEISALREADYOPEN),
			getStringFromResource(UI_TABCONTROL)
		);
	}
	else
	{
		if (this->iParam.OpenNewPathInNewTab || openInNewTab)
		{
			if (this->ADD_Tab(path))
			{
				if (setFocus)
				{
					auto _async = new Async();
					if (_async != nullptr)
					{
						//async_smpl fct = (async_smpl)&focusToEdit;
						_async->callFunction(&focusToEdit);
					}
				}
			}
		}
		else
		{
			if (this->SaveCheck_singleTab(this->GetActiveTabProperty()))
			{
				CnC3File file;
				file.Load(path);

				if (file.Succeeded())
				{
					if (!this->OpenDisplayAndFormat(file, DO_CLEANUP | SET_DISPLAYNAME))
					{
						// handle error...
					}
					else
					{
						this->updateFileInfoArea();

						if (setFocus)
						{
							auto _async = new Async();
							if (_async != nullptr)
							{
								//async_smpl fct = (async_smpl)&focusToEdit;
								_async->callFunction(&focusToEdit);
							}
						}
					}
				}
				else
				{
					auto hr = file.GetStatus();

					iString errorMsg(
						getStringFromResource(ERROR_MSG_CNC3FILEOBJECT_ERROR)
					);
					errorMsg += iString::fromHex((uintX)hr);

					DispatchEWINotification(
						EDSP_ERROR,
						L"TC0005",
						errorMsg.GetData(),
						L"Method::Open"
					);

				}
			}
		}
	}
}

BOOL CnCS_TC::SaveAs(TCHAR ** path_out, LPTABPROPERTY _ptp)
{
	BOOL result = FALSE;

	LPTABPROPERTY ptp = nullptr;

	if (_ptp == nullptr)
		ptp = this->GetActiveTabProperty();
	else
		ptp = _ptp;

	if (ptp != nullptr)
	{
		CnC3FileManager cnc3FileManager(this->Main);

		cnc3FileManager.SetUserEventHandler(
			dynamic_cast<IFileDialogUserEvents*>(this)
		);

		// set the current directory and filename to make to process more conveniant for the user
		if (ptp->Path != nullptr)
		{
			auto bfpo = CreateBasicFPO();
			if (bfpo != nullptr)
			{
				TCHAR* folder = nullptr;

				if (bfpo->RemoveFilenameFromPath(ptp->Path, &folder))
				{
					cnc3FileManager.SetTargetFolder(folder);
					cnc3FileManager.SetDialogText(nullptr, nullptr, ptp->displayname);
				}
				SafeDeleteArray(&folder);
				SafeRelease(&bfpo);
			}
		}
		else
		{
			// TODO: get the last navigated folder and set as target folder
		}

		this->iParam.SkipNextFilesystemAction = TRUE;// prevent the filesystem-watcher from closing the tab

		TCHAR* editText = nullptr;

		if (GetRichEditContent(ptp->AssocEditWnd, &editText) == TRUE)
		{
			CnC3File file;
			file.SetNCContent(editText);
			file.AddProperty(CnC3File::PID_DESCRIPTION_ONE, ptp->DESC1);
			file.AddProperty(CnC3File::PID_DESCRIPTION_TWO, ptp->DESC2);
			file.AddProperty(CnC3File::PID_DESCRIPTION_THREE, ptp->DESC3);

			auto savedFile = cnc3FileManager.SaveAs(file);
			if (savedFile.Succeeded())
			{
				ptp->Content_Changed = FALSE;

				SafeDeleteArray(&ptp->Path);
				SafeDeleteArray(&ptp->displayname);

				if (CopyStringToPtr(
					savedFile.GetPath(),
					&ptp->Path)
					== TRUE)
				{
					if (GetFilenameOutOfPath(ptp->Path, &ptp->displayname, TRUE) == TRUE)
					{
						this->RedrawTab_s(REDRAW_CURRENT);
						this->updateFileInfoArea();

						result =
							(CopyStringToPtr(savedFile.GetPath(), path_out) == TRUE)
							? TRUE : FALSE;
					}
				}
			}
			else
			{
				// error saving the file -> Dispatch notification!!!!!!!!


			}
			SafeDeleteArray(&editText);	
		}
	}
	return result;
}

BOOL CnCS_TC::Save(DWORD mode, LPTABPROPERTY _ptp)
{
	BOOL result = TRUE;

	if (mode == TOS_SAVE)
	{
		LPTABPROPERTY ptp = nullptr;

		if (_ptp == nullptr)
			ptp = this->GetActiveTabProperty();
		else
			ptp = _ptp;

		if (ptp != nullptr)
		{
			if (ptp->Content_Changed)
			{
				TCHAR *editText = nullptr;

				if (GetRichEditContent(ptp->AssocEditWnd, &editText) == TRUE)
				{
					this->SaveDescritpionsToTabProperty(ptp);

					if (ptp->Path != nullptr)
					{
						CnC3File file;
						file.SetPath(ptp->Path);
						file.SetNCContent(editText);
						file.AddProperty(CnC3File::PID_DESCRIPTION_ONE, ptp->DESC1);
						file.AddProperty(CnC3File::PID_DESCRIPTION_TWO, ptp->DESC2);
						file.AddProperty(CnC3File::PID_DESCRIPTION_THREE, ptp->DESC3);

						auto hr = file.Save();
						if (SUCCEEDED(hr))
						{
							ptp->Content_Changed = FALSE;
							this->RedrawTab_s(REDRAW_CURRENT);
							this->updateFileInfoArea();
						}
					}
					else
					{
						result = FALSE;
					}
					SafeDeleteArray(&editText);
				}
			}
		}
	}
	else if (mode == TOS_SAVEALL)
	{
		this->SaveDescritpionsToTabProperty(
			this->GetActiveTabProperty()
		);

		LPTABPROPERTY ptp = NULL;
		TCHAR* editText = NULL;

		for (DWORD i = 0; i < this->TABCount; i++)
		{
			ptp = reinterpret_cast<LPTABPROPERTY>(
				GetWindowLongPtr(
					GetDlgItem(this->TCFrame, TAB_ID_START + i), GWLP_USERDATA)
				);

			if (ptp != NULL)
			{
				if (ptp->Content_Changed)
				{
					if (GetRichEditContent(ptp->AssocEditWnd, &editText) == TRUE)
					{
						if (ptp->Path != NULL)
						{
							CnC3File file;
							file.SetPath(ptp->Path);
							file.SetNCContent(editText);
							file.AddProperty(CnC3File::PID_DESCRIPTION_ONE, ptp->DESC1);
							file.AddProperty(CnC3File::PID_DESCRIPTION_TWO, ptp->DESC2);
							file.AddProperty(CnC3File::PID_DESCRIPTION_THREE, ptp->DESC3);

							auto hr = file.Save();
							if (SUCCEEDED(hr))
							{
								ptp->Content_Changed = FALSE;
							}
						}
						else
						{
							result = FALSE;
						}
						SafeDeleteArray(&editText);
					}
					if (i == this->iParam.dwActiveTab)
					{
						this->updateFileInfoArea();
					}
				}
			}
		}
		this->RedrawTab_s(REDRAW_ALL);
	}
	return result;
}

void CnCS_TC::Import(LPCTSTR content)
{
	auto ctp = this->GetActiveTabProperty();
	if (ctp != NULL)
	{
		ctp->Editcontrol->SetTextContent(content, TRUE, FALSE, TRUE);
		ctp->Content_Changed = TRUE;
		this->RedrawTab_s(REDRAW_CURRENT);
	}
}

void CnCS_TC::Insert(LPCTSTR text)
{
	auto ptp = this->GetTabProperty(this->iParam.dwActiveTab);
	if (ptp != nullptr)
	{
		ptp->Editcontrol->InsertText(text, TRUE);
		ptp->Content_Changed = TRUE;

		this->RedrawTab_s(REDRAW_CURRENT);

		auto async = new Async();
		if (async != nullptr)
		{
			//async_smpl fmt = (async_smpl)&focusToEdit;
			async->callFunction(&focusToEdit);
		}
	}
}

void CnCS_TC::OnFileDeleted(LPCTSTR path)
{
	HWND tab = NULL;

	for (DWORD i = 0; i < this->TABCount; i++)
	{
		tab = GetDlgItem(this->TCFrame, TAB_ID_START + i);
		if (tab)
		{
			auto ptp = reinterpret_cast<LPTABPROPERTY>(GetWindowLongPtr(tab, GWLP_USERDATA));
			if (ptp != NULL)
			{
				if (ptp->Path != nullptr)
				{
					if (CompareStringsB(ptp->Path, path))
					{
						this->CLOSE_Tab(tab, FALSE);
					}
				}
			}			
		}
	}
}

void CnCS_TC::OnFolderDeleted(LPCTSTR path)
{
	iString tabPath;

	int holder = (int)this->TABCount;

	for (int i = (holder - 1); i >= 0; i--)
	{
		auto ptp = this->GetTabProperty(i);
		if (ptp != nullptr)
		{
			if (ptp->Path != nullptr)
			{
				CHARSCOPE cs;

				tabPath.Replace(ptp->Path);

				if (tabPath.Contains(path, &cs))
				{
					if (cs.startChar == 0)
					{
						this->CLOSE_Tab(ptp->Tab, FALSE);
					}
				}
			}
		}	
	}
}

void CnCS_TC::OnFileRenamed(LPCTSTR path_new, LPCTSTR path_old)
{
	for (DWORD i = 0; i < this->TABCount; i++)
	{
		auto ptp = this->GetTabProperty(i);
		if (ptp != nullptr)
		{
			auto isEqual = CompareStringsB(ptp->Path, path_old);
			if (isEqual)
			{
				SafeDeleteArray(&ptp->Path);
				SafeDeleteArray(&ptp->displayname);

				CopyStringToPtr(path_new, &ptp->Path);
				GetFilenameOutOfPath(ptp->Path, &ptp->displayname, TRUE);

				RedrawWindow(ptp->Tab, nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE);
				break;
			}
		}
	}
}

void CnCS_TC::OnFolderRenamed(LPCTSTR path_new, LPCTSTR path_old)
{
	CHARSCOPE cs;
	iString tabPath;

	for (DWORD i = 0; i < this->TABCount; i++)
	{
		auto ptp = this->GetTabProperty(i);
		if (ptp != nullptr)
		{
			tabPath.Replace(ptp->Path);

			if (tabPath.Contains(path_old, &cs))
			{
				if (cs.startChar == 0)
				{
					tabPath.SetSegment(path_new, &cs);

					SafeDeleteArray((void**)&ptp->Path);
					CopyStringToPtr(tabPath.GetData(), &ptp->Path);
				}
			}
		}
	}
}

void CnCS_TC::OnFileSystemMoveOperation(LPFILESYSTEMOBJECT fso)
{
	for (DWORD d = 0; d < this->TABCount; d++)
	{
		auto ptp = this->GetTabProperty(d);
		if (ptp != nullptr)
		{
			if (fso->FileSystemObjectType == FSO_TYPE_FILE) // file
			{
				if (fso->OldPath.GetLength() > 0)
				{
					if (CompareStringsB(ptp->Path, fso->OldPath.GetPathData()))
					{
						// the moved file is opened in a tab -> so change tab-path
						SafeDeleteArray((void**)&ptp->Path);

						if (fso->Path.GetLength() > 0)
						{
							if (CopyStringToPtr(
								fso->Path.GetPathData(),
								&ptp->Path)
								!= TRUE)
							{
								// error ...
							}
						}
					}
				}
			}
			else // folder
			{
				if ((fso->OldPath.GetLength() > 0) && (fso->Path.GetLength() > 0))
				{
					CHARSCOPE cs;

					iString currentPath(ptp->Path);
					iString stringToFind(
						fso->OldPath.GetPathData()
					);
					iString newPath(
						fso->Path.GetPathData()
					);

					if (currentPath.Contains(stringToFind, &cs))
					{
						// the moved directory is a parent-directory of the opened file
						// -> substitute the path-segment to update the filepath in the tabstruct
						if (cs.startChar == 0)
						{
							auto len = currentPath.GetLength();
							cs.startChar = cs.endChar + 1;
							cs.endChar = len;

							auto seg = currentPath.GetSegment(&cs);
							newPath += seg;

							SafeDeleteArray((void**)&ptp->Path);

							if (
								CopyStringToPtr(
									newPath.GetData(),
									&ptp->Path)
								!= TRUE)
							{
								// error ...
							}
						}
					}
				}
			}
		}
	}
}

void CnCS_TC::OnFileSystemDeleteOperation(LPFILESYSTEMOBJECT fso)
{
	if (fso->FileSystemObjectType == FSO_TYPE_FILE)
	{
		this->OnFileDeleted(
			fso->Path.GetPathData()
		);
	}
	else if (fso->FileSystemObjectType == FSO_TYPE_FOLDER)
	{
		this->OnFolderDeleted(
			fso->Path.GetPathData()
		);
	}
	else // FSO_TYPE_UNKNOWN
	{
		if (isCnC3Path(fso->Path.GetPathData()))
		{
			this->OnFileDeleted(
				fso->Path.GetPathData()
			);
		}
		else
		{
			this->OnFolderDeleted(
				fso->Path.GetPathData()
			);
		}
	}
}

void CnCS_TC::OnFileSystemRenameOperation(LPFILESYSTEMOBJECT fso)
{
	if (fso->FileSystemObjectType == FSO_TYPE_FILE)
	{
		this->OnFileRenamed(
			fso->Path.GetPathData(),
			fso->OldPath.GetPathData()
		);
	}
	else if (fso->FileSystemObjectType == FSO_TYPE_FOLDER)
	{
		this->OnFolderRenamed(
			fso->Path.GetPathData(),
			fso->OldPath.GetPathData()
		);
	}
}


//void CnCS_TC::OnFileSystemMoveOperation(LPFILEOPERATIONINFO pFoInfo)
//{
//	for (DWORD d = 0; d < this->TABCount; d++)
//	{
//		auto ptp = this->GetTabProperty(d);
//		if (ptp != nullptr)
//		{
//			if (pFoInfo->fileType == FOI_TYPE_FILE)
//			{
//				if (pFoInfo->p1 != nullptr)
//				{
//					if (CompareStringsB(ptp->Path, pFoInfo->p1))
//					{
//						// the moved file is opened in a tab -> so change tab-path
//						SafeDeleteArray((void**)&ptp->Path);
//
//						if (pFoInfo->p2 != nullptr)
//						{
//							if (CopyStringToPtr(pFoInfo->p2, &ptp->Path) != TRUE)
//							{
//								// error ...
//							}
//						}
//					}
//				}
//			}
//			else // FOI_TYPE_FOLDER
//			{
//				if ((pFoInfo->p1 != nullptr) && (pFoInfo->p2 != nullptr))
//				{
//					CHARSCOPE cs;
//
//					iString currentPath(ptp->Path);
//					iString stringToFind(pFoInfo->p1);
//					iString newPath(pFoInfo->p2);
//
//					if (currentPath.Contains(stringToFind, &cs))
//					{
//						// the moved directory is a parent-directory of the opened file
//							// -> substitute the path-segment to update the filepath in the tabstruct
//						if (cs.startChar == 0)
//						{
//							auto len = currentPath.GetLength();
//							cs.startChar = cs.endChar + 1;
//							cs.endChar = len;
//
//							auto seg = currentPath.GetSegment(&cs);
//							newPath += seg;
//
//							SafeDeleteArray((void**)&ptp->Path);
//
//							if (
//								CopyStringToPtr(
//									newPath.GetData(),
//									&ptp->Path)
//								!= TRUE)
//							{
//								// error ...
//							}
//						}
//					}
//				}				
//			}
//		}
//	}
//}

void CnCS_TC::SetDescriptions(LPTSTR d1, LPTSTR d2, LPTSTR d3)
{
	HWND descFrame = GetDlgItem(this->TCFrame, ID_DESCFRAME);
	if (descFrame != NULL)
	{
		TCHAR defTExt[] = L"< --- >";
		LPTSTR setter = nullptr;

		SETTEXTEX stx;
		stx.codepage = 1200;
		stx.flags = ST_DEFAULT | ST_NEWCHARS;

		if (d1 == nullptr)
			setter = defTExt;
		else
			setter = d1;

		SendMessage(
			GetDlgItem(descFrame, ID_PROPERTYEDIT_ONE),
			EM_SETTEXTEX,
			reinterpret_cast<WPARAM>(&stx),
			reinterpret_cast<LPARAM>(setter)
		);

		if (d2 == nullptr)
			setter = defTExt;
		else
			setter = d2;

		SendMessage(
			GetDlgItem(descFrame, ID_PROPERTYEDIT_TWO),
			EM_SETTEXTEX,
			reinterpret_cast<WPARAM>(&stx),
			reinterpret_cast<LPARAM>(setter)
		);

		if (d3 == nullptr)
			setter = defTExt;
		else
			setter = d3;

		SendMessage(
			GetDlgItem(descFrame, ID_PROPERTYEDIT_THREE),
			EM_SETTEXTEX,
			reinterpret_cast<WPARAM>(&stx),
			reinterpret_cast<LPARAM>(setter)
		);
	}
}

void CnCS_TC::SaveDescritpionsToTabProperty(LPTABPROPERTY ptc)
{
	HWND descFrame = GetDlgItem(this->TCFrame, ID_DESCFRAME);
	if (descFrame != NULL)
	{
		SafeDeleteArray(&ptc->DESC1);
		SafeDeleteArray(&ptc->DESC2);
		SafeDeleteArray(&ptc->DESC3);

		GetRichEditContent(
			GetDlgItem(descFrame, ID_PROPERTYEDIT_ONE),
			&ptc->DESC1
		);
		GetRichEditContent(
			GetDlgItem(descFrame, ID_PROPERTYEDIT_TWO),
			&ptc->DESC2
		);
		GetRichEditContent(
			GetDlgItem(descFrame, ID_PROPERTYEDIT_THREE),
			&ptc->DESC3
		);
	}
}

void CnCS_TC::CleanUpTabStructForNewContent(LPTABPROPERTY ptc)
{
	ptc->Content_Changed = FALSE;
	SafeDeleteArray(&ptc->displayname);
	SafeDeleteArray(&ptc->Path);
	SafeDeleteArray(&ptc->DESC1);
	SafeDeleteArray(&ptc->DESC2);
	SafeDeleteArray(&ptc->DESC3);
}

BOOL CnCS_TC::onClose()
{
	auto appDataCont
		= reinterpret_cast<ApplicationData*>(
			getDefaultApplicationDataContainer()
		);
	if (appDataCont != nullptr)
	{
		auto saveCondition = appDataCont->getBooleanData(DATAKEY_SETTINGS_SAVE_TABWND_COND, true);
		if (saveCondition)
		{
			this->saveCondition();
		}
		auto saveUnsaved = appDataCont->getBooleanData(DATAKEY_SETTINGS_SAVE_UNSAVED_CONTENT, false);
		if (saveUnsaved)
		{
			for (DWORD i = 0; i < this->TABCount; i++)
			{
				auto ptp = this->GetTabProperty(i);
				if (ptp != nullptr)
				{
					if (ptp->Path != nullptr)
					{
						setFileAccessTime(ptp->Path);
					}
				}
			}
			return TRUE;
		}
	}

	int unsavedTabs = 0;
	int singleTabNr = -1;

	for (DWORD i = 0; i < this->TABCount; i++)
	{
		auto ptp = this->GetTabProperty(i);
		if (ptp != nullptr)
		{
			if (ptp->Content_Changed)
			{
				unsavedTabs++;
				singleTabNr = i;
			}
			if (ptp->Path != nullptr)
			{
				setFileAccessTime(ptp->Path);
			}
		}
	}

	if (unsavedTabs > 0)
	{
		if (unsavedTabs == 1)
		{
			auto ust_ptp = this->GetTabProperty(singleTabNr);
			if (ust_ptp != nullptr)
			{
				return this->SaveCheck_singleTab(ust_ptp);
			}
		}
		else
		{
			iString start;
			iString end(
				getStringFromResource(DLG_MSG_UNSAVEDCONTENTEXISTS));

			auto message = start + unsavedTabs + end;

			int result =
				MessageBox(
					this->TCFrame,
					message.GetData(),
					getStringFromResource(UI_TABCONTROL),
					MB_OKCANCEL | MB_ICONASTERISK | MB_DEFBUTTON2);

			if (result == IDOK)return TRUE;
			else return FALSE;
		}
		return TRUE;
	}
	else
		return TRUE;
}

void CnCS_TC::onSetNewColorScheme()
{
	auto dataContainer
		= reinterpret_cast<ApplicationData*>(
			getDefaultApplicationDataContainer()
		);
	if (dataContainer != nullptr)
	{
		auto box
			= reinterpret_cast<iBox<bool>*>(
				dataContainer->lookUp(L"useXmlColorScheme")
			);

		if (box != nullptr)
		{
			auto setUserdefined = box->get();

			if (setUserdefined)
			{
				this->iParam.UseXMLFileForUserColors = TRUE;

				this->SetEditStyleColorsFrom_XMLFile(&this->editStyleColors);
			}
			else
			{

				this->iParam.UseXMLFileForUserColors = FALSE;

				APPSTYLEINFO StyleInfo;
				SendMessage(this->Main, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&StyleInfo));

				this->SetEditStyleColorsFromStyleInfo(&this->editStyleColors, &StyleInfo);
			}

			for (DWORD i = 0; i < this->TABCount; i++)
			{
				auto ptp = this->GetTabProperty(static_cast<DWORD>(i));
				if (ptp != nullptr)
				{
					ptp->Editcontrol->ConfigureComponent(&this->editStyleColors, nullptr, TRUE, TRUE);					
				}
			}
		}
	}
}

void CnCS_TC::onKeydown(WPARAM vKey)
{
	auto ptp = this->GetActiveTabProperty();
	if (ptp != nullptr)
	{
		switch (vKey)
		{
		case VK_F1:
			this->OnVerticalToolBar_Uppercase();
			break;
		case VK_F2:
			this->OnVerticalToolBar_Autonum();
			break;
		case VK_F3:
			this->OnVerticalToolBar_Autocomplete();
			break;
		case VK_F4:
			this->OnVerticalToolBar_Textcolor();
			break;
		case VK_F5:
			this->OnVerticalToolBar_FocusRect();
			break;
		case VK_F6:
			ptp->Editcontrol->ConfigureComponent(nullptr, nullptr, TRUE, TRUE);
			break;
		case VK_F7:
			// sequence linenumbers
			break;
		default:
			break;
		}
	}
}

void CnCS_TC::onButtonClick(CustomButton * button, CTRLID ctrlID)
{
	UNREFERENCED_PARAMETER(button);

	switch (ctrlID)
	{
	case IDM_DESCWND_COPY:
		this->tagsToInternalClipboard();
		break;
	case IDM_DESCWND_INSERT:
		this->clipboardToTags();
		break;
	case IDM_DESCWND_DIAMETER:
		this->insertDiameterSymbol();
		break;
	case IDM_DESCWND_DELETE:
		this->deleteAllTags();
		break;
	default:
		break;
	}
}

void CnCS_TC::onTrackbarTracking(customTrackbar * _trackbar, int _trackPos)
{
	if (_trackbar != nullptr)
	{
		auto ptp = this->GetActiveTabProperty();
		if (ptp != nullptr)
		{
			auto trackbarId = _trackbar->getCtrlID();

			if (trackbarId == IDM_FONTHEIGHTTRACK)
			{
				this->iParam.FontHeight = 200 + _trackPos;

				ptp->Editcontrol->SetCharHeight(
					DPIScale(this->iParam.FontHeight)
				);
			}
			else if (trackbarId == IDM_LINESPACETRACK)
			{
				this->iParam.lineOffset = _trackPos;
				ptp->Editcontrol->SetLineOffset((LONG)_trackPos);
			}
		}
	}
}

void CnCS_TC::onTrackbarTrackEnd(customTrackbar * _trackbar, int _trackPos)
{
	if (_trackbar != nullptr)
	{
		auto dataContainer
			= reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer());

		if (dataContainer != nullptr)
		{
			auto ptp = this->GetActiveTabProperty();
			if (ptp != nullptr)
			{
				auto trackbarId = _trackbar->getCtrlID();

				if (trackbarId == IDM_FONTHEIGHTTRACK)
				{
					this->iParam.FontHeight = 200 + _trackPos;

					// update all tabs
					for (DWORD i = 0; i < this->TABCount; i++)
					{
						auto nptp = this->GetTabProperty(i);
						if (nptp != nullptr)
						{
							nptp->Editcontrol->SetCharHeight(
								DPIScale(this->iParam.FontHeight)
							);
						}
					}
					dataContainer->saveValue(DATAKEY_EDITOR_FONT_HEIGHT, this->iParam.FontHeight);
				}
				else if (trackbarId == IDM_LINESPACETRACK)
				{
					this->iParam.lineOffset = _trackPos;

					// update all tabs
					for (DWORD i = 0; i < this->TABCount; i++)
					{
						auto nptp = this->GetTabProperty(i);
						if (nptp != nullptr)
						{
							nptp->Editcontrol->SetLineOffset(this->iParam.lineOffset);
						}
					}
					dataContainer->saveValue(DATAKEY_EDITOR_LINEOFFSET, _trackPos);
				}			
			}
		}
	}
}

void CnCS_TC::onCheckboxChecked(CustomCheckbox * cbx, bool _newState)
{
	if (cbx != nullptr)
	{
		auto dataContainer =
			reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer()
			);

		if (dataContainer != nullptr)
		{
			auto cbxID = cbx->getCtrlID();

			switch (cbxID)
			{
			case IDM_FONTBOLDCHECKBX:
				this->iParam.isBold = _newState ? TRUE : FALSE;

				for (DWORD i = 0; i < this->TABCount; i++)
				{
					auto ptp = this->GetTabProperty(i);
					if (ptp != nullptr)
					{
						ptp->Editcontrol->SetBold(_newState);
					}
				}
				dataContainer->saveValue(DATAKEY_EDITOR_BOLDSTYLE, _newState);
				break;
			default:
				break;
			}
		}
	}
}

void CnCS_TC::onFileSystemChanged(LPFILESYSTEMOBJECT fso)
{
	if (this->iParam.SkipNextFilesystemAction == TRUE)
	{
		// skip one delete action, because the IFileDialog->Show() method triggers two filesystem actions in the save-dialog mode (system-side, cannot change this :(  ---)
		// at first there is an add action, which is not dispatched from the application class (makes no sense to inform the tabcontrol)
		// the second action is a delete action, this action must be skiped to prevent the closure of the current tab
		this->iParam.SkipNextFilesystemAction = FALSE;
		return;
	}

	switch (fso->FileSystemOperation)
	{
	case FSO_ITEM_CREATED:
		break;
	case FSO_ITEM_DELETED:
		this->OnFileSystemDeleteOperation(fso);
		break;
	case FSO_ITEM_MOVED:
		this->OnFileSystemMoveOperation(fso);
		break;
	case FSO_ITEM_RENAMED:
		this->OnFileSystemRenameOperation(fso);
		break;
	default:
		break;
	}
}

void CnCS_TC::onMenuButtonClick(CTRLID Id)
{
	auto ptp = this->GetActiveTabProperty();
	if (ptp != nullptr)
	{
		TCHAR* text = nullptr;

		switch (Id)
		{
		case IDM_PMENU_OPENINWINEXP:
			if (ptp->Path != nullptr)
			{
				TCHAR* npath = nullptr;

				if (CopyStringToPtr(ptp->Path, &npath) == TRUE)
				{
					auto bfpo = CreateBasicFPO();
					if (bfpo != nullptr)
					{
						if (bfpo->RemoveFilenameFromPath(npath))
						{
							ShellExecute(nullptr, L"open", npath, nullptr, nullptr, SW_SHOWDEFAULT);
						}
						SafeRelease(&bfpo);
					}
					SafeDeleteArray(&npath);
				}
			}
			break;
		case IDM_PMENU_COPYTONEWTAB:
			if (ptp->Editcontrol->GetTextContent(&text))
			{
				this->ADD_Tab(nullptr);

				auto newtabPtp = GetTabProperty(this->TABCount - 1);
				if (newtabPtp != nullptr)
				{
					newtabPtp->Editcontrol->SetTextContent(text, TRUE, FALSE, TRUE);
					newtabPtp->Content_Changed = TRUE;
					RedrawTab_s(REDRAW_CURRENT);
				}
				SafeDeleteArray(&text);
			}
			break;
		case IDM_PMENU_ERASETAB:
			this->eraseTab(ptp);
			ptp->Editcontrol->SetCaret(CARET_TO_END_OF_TEXT);
			SetFocus(ptp->AssocEditWnd);
			break;
		case IDM_PMENU_EXPANDTOFILE:
			if (ptp->Path != nullptr)
			{
				SendMessage(
					this->Main,
					WM_COMMAND,
					MAKEWPARAM(ICOMMAND_EXPANDPATHTOFILE, 0),
					reinterpret_cast<LPARAM>(ptp->Path)
				);
			}
			break;
		case IDM_PMENU_SETTEMPLATE:
			if (ptp->Editcontrol->GetTextContent(&text))
			{
				auto userStringContainer =
					reinterpret_cast<ApplicationData*>(
						getApplicationDataContainerFromFilekey(FILEKEY_USER_STRINGS)
						);

				if (userStringContainer != nullptr)
				{
					userStringContainer->saveValue(DATAKEY_USERSTRINGS_DEFAULTTABINSERTTEXT, text);		
				}
				SafeDeleteArray(&text);
			}
			break;
		default:
			break;
		}
	}
}

void CnCS_TC::onFocusRectOffsetChanged()
{
	auto dataContainer =
		reinterpret_cast<ApplicationData*>(
			getApplicationDataContainerFromFilekey(FILEKEY_INTERNAL_SETTINGS)
			);

	if (dataContainer != nullptr)
	{
		int top = dataContainer->getIntegerData(DATAKEY_INTSET_EDITOR_FOCUSRECTOFFSET_TOP, 1);
		int bottom = dataContainer->getIntegerData(DATAKEY_INTSET_EDITOR_FOCUSRECTOFFSET_BOTTOM, 5);

		for (DWORD i = 0; i < this->TABCount; i++)
		{
			auto ptp = this->GetTabProperty(i);
			if (ptp != nullptr)
			{
				ptp->Editcontrol->SetFocusMarkCorrectionValue(top, bottom);
			}
		}
	}
}

//BOOL CnCS_TC::_SetProp(LPTCPROPERTY tcp)
//{
//	BOOL result = (tcp != NULL) ? TRUE : FALSE;
//	if (result)
//	{
//		this->iParam.StartupAction = tcp->StartupAction;
//		this->iParam.OpenNewPathInNewTab = tcp->OpenNewPathInNewTab;
//		this->iParam.AutoSave = tcp->AutoSave;
//
//		result = this->_SavePropToFile();
//		if (result)
//		{
//
//		}
//	}
//	return result;
//}

BOOL CnCS_TC::_GetProp(LPTCPROPERTY tcp)
{
	BOOL result = (tcp != NULL) ? TRUE : FALSE;
	if (result)
	{
		APPSTYLEINFO StyleInfo;
		SendMessage(this->Main, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&StyleInfo));

		tcp->StartupAction = this->iParam.StartupAction;
		tcp->AutoSave = this->iParam.AutoSave;
		tcp->OpenNewPathInNewTab = this->iParam.OpenNewPathInNewTab;

		this->SetEditControlProperties(&tcp->ecp);
	
		if (this->iParam.UseXMLFileForUserColors)
			this->SetEditStyleColorsFrom_XMLFile(&tcp->esc);
		else
			this->SetEditStyleColorsFromStyleInfo(&tcp->esc, &StyleInfo);
	}
	return result;
}

void CnCS_TC::saveCondition()
{
	auto builder = new XML_Builder();
	if (builder != nullptr)
	{
		iXML_Tag tag;
		tag.hasProperties = true;
		tag.tagName = L"tab";

		XML_TAG_Property propertY1, propertY2;
		propertY1.propertyName = L"contentchanged";
		propertY2.propertyName = L"path";

		builder->setUp_DocumentDefinition(L"1.0", L"utf-8", L"cncstab_session", true);

		for (DWORD i = 0; i < this->TABCount; i++)
		{
			auto ptp = this->GetTabProperty(i);
			if (ptp != nullptr)
			{
				TCHAR* tBuffer = nullptr;

				if (GetRichEditContent(ptp->AssocEditWnd, &tBuffer))
				{
					tag.tagContent.Replace(tBuffer);
					SafeDeleteArray(&tBuffer);
				}
				else
					tag.tagContent.Replace(L"none");
				
				propertY1.propertyContent = ptp->Content_Changed ? L"true" : L"false";
				tag.tagProperties.AddItem(propertY1);
				
				if(ptp->Path != nullptr)
					propertY2.propertyContent.Replace(ptp->Path);
				else
					propertY2.propertyContent.Replace(L"none");

				tag.tagProperties.AddItem(propertY2);

				if (this->iParam.dwActiveTab == i)
				{
					XML_TAG_Property propertY3;
					propertY3.propertyName = L"isActive";
					propertY3.propertyContent = L"true";
					tag.tagProperties.AddItem(propertY3);
				}

				builder->AddTag(&tag);

				tag.tagProperties.Clear();
			}
		}
		AppPath aPath;
		auto fPath = aPath.Get(PATHID_FOLDER_CNCSUITE_APPDATA_SESSION);
		fPath.Append(L"\\TABimage.xml");

		builder->finalizeAsync(fPath, true);
	}
}

void CnCS_TC::restoreCondition()
{
	AppPath aPath;
	auto fPath = aPath.Get(PATHID_FOLDER_CNCSUITE_APPDATA_SESSION);
	fPath.Append(L"\\TABimage.xml");

	auto appDataCont =
		reinterpret_cast<ApplicationData*>(
			getDefaultApplicationDataContainer()
			);

	if (appDataCont != nullptr)
	{
		auto restoreUnsavedData = appDataCont->getBooleanData(DATAKEY_SETTINGS_SAVE_UNSAVED_CONTENT, false);

		XML_Parser parser;
		if (parser.OpenDocument(fPath))
		{
			if (parser.Parse())
			{
				DWORD selectedTab = 0;
				auto docStruct = parser.getDocumentStructure();

				for (int i = 0; i < docStruct.GetCount(); i++)
				{
					// add a TAB for each element...
					auto tag = docStruct.GetAt(i);

					// get the path and open it or create an empty tab
					auto pathtofile = tag.getPropertyContentFromName(L"path");
					if (pathtofile.Equals(L"none"))
					{
						this->ADD_Tab(nullptr);
					}
					else
					{
						CnC3File file;
						file.SetPath(
							pathtofile.GetData()
						);
						auto hr = file.Load();
						if (SUCCEEDED(hr))
						{
							this->AddTab(file);
						}
					}
					// look for the tab with the selection mark-property
					if (tag.hasSpecificProperty(L"isActive"))
					{
						selectedTab = i;
					}

					// restore the unsaved data
					if (restoreUnsavedData)
					{
						auto ptp = this->GetTabProperty(i);
						if (ptp != nullptr)
						{
							ptp->Content_Changed
								= tag.getPropertyContentFromName(L"contentchanged")
								.Equals(L"true")
								? TRUE : FALSE;

							if (ptp->Content_Changed)
							{
								ptp->Editcontrol->SetTextContent(
									tag.tagContent.GetData(),
									TRUE, TRUE, FALSE
								);
							}
						}
					}
				}
				this->selectTab(selectedTab);
			}
		}
		else
		{
			this->ADD_Tab(nullptr);
		}
	}
}

BOOL CnCS_TC::SaveCheck_singleTab(LPTABPROPERTY ptp)
{
	if (!ptp->Content_Changed)
		return TRUE;
	else
	{
		if (this->iParam.AutoSave && (ptp->Path != nullptr))
		{
			this->Save(TOS_SAVE, ptp);
			return TRUE;
		}
		else
		{
			iString message(ptp->displayname);
			message.Append(
				getStringFromResource(DLG_MSG_FILENOTSAVEDSAVENOW));

			int result = MessageBox(
				this->TCFrame,
				message.GetData(),
				ptp->displayname,
				MB_YESNOCANCEL | MB_ICONASTERISK | MB_DEFBUTTON1);

			if (result == IDNO)
				return TRUE;				// do not save and proceed...
			else if (result == IDCANCEL)
				return FALSE;				// return FALSE to prevent tab closing ...
			else if (result == IDYES)
			{								// save...
				if (ptp->Path != NULL)
				{
					this->Save(TOS_SAVE, ptp);
				}
				else
				{
					TCHAR* newPath = NULL;

					if (this->SaveAs(&newPath, ptp))
					{
						SafeDeleteArray(&newPath);
					}
				}
			}
			return TRUE;
		}		
	}
}

BOOL CnCS_TC::CalculateInsertTabrange(LPPOINT pt)
{
	BOOL result = TRUE;
	RECT rc;
	GetClientRect(this->TCFrame, &rc);

	int calWidth = (rc.right - DPIScale(42)) / (this->TABCount + 1);		//int calWidth = (rc.right - 4) / (this->TABCount + 1);
	if (calWidth >= COMMONTABWIDTH)
	{
		pt->x = (this->TABCount*COMMONTABWIDTH) + DPIScale(40);			//pt->x = (this->TABCount*COMMONTABWIDTH) + 2;
		pt->y = ((this->TABCount + 1)*COMMONTABWIDTH) + DPIScale(40);		//pt->y = ((this->TABCount + 1)*COMMONTABWIDTH) + 2;

		result = COMMONTABWIDTH;
	}
	else if ((calWidth < COMMONTABWIDTH) && (calWidth > MINIMUMTABWIDTH))
	{
		pt->x = (this->TABCount*calWidth) + DPIScale(40);					//pt->x = (this->TABCount*calWidth) + 2;
		pt->y = ((this->TABCount + 1)*calWidth) + DPIScale(40);			//pt->y = ((this->TABCount + 1)*calWidth) + 2;

		result = calWidth;
	}
	else if (calWidth <= MINIMUMTABWIDTH)
	{
		pt->x = (this->TABCount*MINIMUMTABWIDTH) + DPIScale(40);			//pt->x = (this->TABCount*MINIMUMTABWIDTH) + 2;
		pt->y = ((this->TABCount + 1)*MINIMUMTABWIDTH) + DPIScale(40);	//pt->y = ((this->TABCount + 1)*MINIMUMTABWIDTH) + 2;

		result = TAB_MAXIMUM;
	}
	else
		return FALSE;

	return result;
}

BOOL CnCS_TC::DrawTabText(HDC hdc, HWND Tab, LPTSTR nText)
{
	int c;
	size_t len;
	SIZE sz;
	RECT rc;
	BOOL result = TRUE;
	TCHAR* text = NULL;

	result = CopyStringToPtrA(nText, &text);
	if (result)
	{
		GetClientRect(Tab, &rc);

		result = SUCCEEDED(StringCbLength(text, STRSAFE_MAX_CCH, &len)) ? TRUE : FALSE;
		if (result)
		{
			c = (int)(len / sizeof(TCHAR));

			result = (c > 0) ? TRUE : FALSE;
			if (result)
			{
				result = GetTextExtentPoint32(hdc, text, c, &sz);
				if (result)
				{
					int freeSection = DPIScale(70);

					if (sz.cx > (rc.right - freeSection))
					{
						// the text is wider than the draw-area
						// set it as tooltip
						AddToolipToControl(Tab, this->Main, text, this->hInstance, nullptr);

						while (sz.cx > (rc.right - freeSection))
						{
							c--;
							if (c == 0)
								break;

							result = GetTextExtentPoint32(hdc, text, c, &sz);
							if (!result)
								break;
						}
						if (c > 3)
						{
							text[c - 1] = '.';
							text[c - 2] = '.';
							text[c - 3] = '.';
						}
						else if (c == 3)
						{
							text[c - 1] = '.';
							text[c - 2] = '.';
						}

					}
					result = TextOut(hdc, DPIScale(35), ((rc.bottom / 2) - (sz.cy / 2)), text, c);
				}
			}
		}
		delete[] text;
	}
	return result;
}

BOOL CnCS_TC::DrawTBButton(LPDRAWITEMSTRUCT pdi, int iconID)
{
	BOOL result = TRUE;

	HICON icon =
		(HICON)LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(iconID),
			IMAGE_ICON,
			DPIScale(24),
			DPIScale(24),
			LR_DEFAULTCOLOR
		);

	if (icon)
	{
		HBRUSH bkgnd = this->GetTBButtonBkgndBrush(pdi->CtlID);
		if (bkgnd)
		{
			FillRect(pdi->hDC, &pdi->rcItem, bkgnd);

			result =
				DrawIconEx(
					pdi->hDC,
					0, 0,
					icon,
					DPIScale(24),
					DPIScale(24),
					0,
					bkgnd,
					DI_NORMAL
				);

			HGDIOBJ origin = SelectObject(pdi->hDC, this->tcObj.cPen);

			MoveToEx(pdi->hDC, 0, 0, NULL);
			LineTo(pdi->hDC, pdi->rcItem.right - 1, 0);
			LineTo(pdi->hDC, pdi->rcItem.right - 1, pdi->rcItem.bottom - 1);
			LineTo(pdi->hDC, 0, pdi->rcItem.bottom - 1);
			LineTo(pdi->hDC, 0, 0);

			SelectObject(pdi->hDC, origin);

			DeleteObject(bkgnd);
		}
		DestroyIcon(icon);
	}
	else
		result = FALSE;

	return result;
}

void CnCS_TC::RefreshTabAlignment()
{
	HWND Tab;
	int x;

	RECT rc;
	GetClientRect(this->TCFrame, &rc);

	DWORD width = this->GetTabWidth(this->TABCount);

	for (DWORD i = 0; i < this->TABCount; i++)		// tab count one-based
	{
		Tab = GetDlgItem(this->TCFrame, TAB_ID_START + i);
		if (Tab != NULL)
		{
			x = DPIScale(40) + (i*width);									//x = 2 + (i*width);
			if (i == this->iParam.dwActiveTab)
			{
				this->iParam.ActiveTabRange.x = x;
				this->iParam.ActiveTabRange.y = x + width;
			}
			InvalidateRect(Tab, NULL, FALSE);
			MoveWindow(Tab, x, DPIScale(2), width, DPIScale(25), TRUE);
		}
	}
	rc.bottom = DPIScale(52);
	RedrawWindow(this->TCFrame, &rc, NULL, RDW_NOCHILDREN | RDW_NOERASE | RDW_INVALIDATE);
}

void CnCS_TC::RefreshTBButtons()
{
	for (int i = 0; i < 5; i++)
	{
		RedrawWindow(
			GetDlgItem(
				this->TCFrame,
				IDM_UPPERCASE + i),
			NULL, NULL, RDW_INVALIDATE | RDW_NOCHILDREN | RDW_NOERASE);
	}
}

void CnCS_TC::RedrawTab_s(DWORD mode)
{
	if (mode == REDRAW_CURRENT)
	{
		RedrawWindow(this->iParam.hwndActiveTab, NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_NOERASE);
	}
	else if (mode == REDRAW_ALL)
	{
		for (DWORD i = 0; i < this->TABCount; i++)
		{
			RedrawWindow(
				GetDlgItem(this->TCFrame, TAB_ID_START + i)
				, NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_NOERASE);
		}
	}
}

void CnCS_TC::SetActiveTab()
{
	BOOL change = FALSE;

	if (this->iParam.previousTab >= (this->TABCount - 1))		// zero-based >= (one-based - 1)
	{
		this->iParam.previousTab = 0;

		if (this->iParam.hwndActiveTab == NULL)
		{
			this->iParam.dwActiveTab = (this->TABCount - 1);	// zero-based = (one-based - 1)
			this->iParam.hwndActiveTab = GetDlgItem(this->TCFrame, TAB_ID_START + this->iParam.dwActiveTab);

			change = TRUE;
		}
		else
		{
			if (this->iParam.ChangeActiveTab)
			{
				if (this->iParam.dwActiveTab > 0)
					this->iParam.dwActiveTab--;
			}
		}
	}
	else
	{
		if (this->iParam.hwndActiveTab == NULL)
		{
			this->iParam.dwActiveTab = this->iParam.previousTab;
			this->iParam.hwndActiveTab = GetDlgItem(this->TCFrame, TAB_ID_START + this->iParam.dwActiveTab);

			change = TRUE;
		}
		else
		{
			if (this->iParam.ChangeActiveTab)
			{
				if (this->iParam.dwActiveTab > 0)
					this->iParam.dwActiveTab--;
			}
		}
	}
	if (change)
	{
		LPTABPROPERTY ptp =
			reinterpret_cast<LPTABPROPERTY>(
				GetWindowLongPtr(this->iParam.hwndActiveTab, GWLP_USERDATA)
			);

		if (ptp != NULL)
		{
			// move the edit-wnd!!!!
			this->ResizeEditWindow(ptp->AssocEditWnd);

			ShowWindow(ptp->AssocEditWnd, SW_SHOW);
			this->SetDescriptions(ptp->DESC1, ptp->DESC2, ptp->DESC3);
		}
	}
}

void CnCS_TC::OnEditHasChanged(HWND Tab)
{
	LPTABPROPERTY ptp = reinterpret_cast<LPTABPROPERTY>(GetWindowLongPtr(Tab, GWLP_USERDATA));
	if (ptp != NULL)
	{
		ptp->Content_Changed = TRUE;

		RedrawWindow(
			GetDlgItem(Tab, ID_TABCLOSE),
			NULL, NULL, RDW_NOCHILDREN | RDW_NOERASE | RDW_INVALIDATE);
	}
}

void CnCS_TC::ControlPropertyWindow()
{
	if (!this->iParam.descWnd_isMoving)
	{
		if (this->iParam.descWnd_visible)
		{
			this->PropWndSoftControl(PROPWND_SOFTOUT);
			this->iParam.descWnd_visible = FALSE;
			this->ControlPropertyButton(PROPBUTTON_UPDATE);
		}
		else
		{
			if (this->iParam.descWnd_exists)
			{
				this->iParam.descWnd_visible = TRUE;
				this->PropWndSoftControl(PROPWND_SOFTIN);
				this->ControlPropertyButton(PROPBUTTON_UPDATE);
			}
			else
			{
				int Mode = DESCFRAME_CREATEDYNAMIC;

				this->iParam.descWnd_visible = TRUE;

				if (Mode == DESCFRAME_CREATESTATIC)
					SendMessage(this->TCFrame, WM_SIZE, 0, 0);

				this->CreateDescFrame(Mode);
			}
		}
	}
}

void CnCS_TC::ChangeColorAsync()
{
	HANDLE hThread;
	DWORD threadID;

	hThread = CreateThread(nullptr, 0, CnCS_TC::changeColorProc, reinterpret_cast<LPVOID>(this), 0, &threadID);
	if (hThread)
	{
		WaitForSingleObject(hThread, 20);
		CloseHandle(hThread);
	}
}

DWORD CnCS_TC::GetTabWidth(DWORD Count)
{
	if (Count == 0)
		return COMMONTABWIDTH;

	RECT rc;
	GetClientRect(this->TCFrame, &rc);

	int calWidth = (rc.right - 42) / Count;		//int calWidth = (rc.right - 4) / Count;
	if (calWidth > COMMONTABWIDTH)
	{
		return COMMONTABWIDTH;
	}
	else if ((calWidth < COMMONTABWIDTH) && (calWidth > MINIMUMTABWIDTH))
	{
		return calWidth;
	}
	else if (calWidth < MINIMUMTABWIDTH)
	{
		return MINIMUMTABWIDTH;
	}
	else
		return 0;
}

HBRUSH CnCS_TC::GetTBButtonBkgndBrush(int IDdrawCtrl)
{
	COLORREF color;

	if (IDdrawCtrl == this->iParam.TBButtonIDdrawFor)
	{
		color = RGB(0, 120, 180);
	}
	else
	{
		switch (this->iParam.currentStyle)
		{
		case STYLEID_GREEN:
			color = RGB(40, 90, 90);
			break;
		case STYLEID_BLACK:
			color = RGB(0, 0, 0);
			break;
		case STYLEID_DARKGRAY:
			color = RGB(80, 80, 80);
			break;
		case STYLEID_LIGHTGRAY:
			color = RGB(100, 100, 100);
			break;
		default:
			return (HBRUSH)NULL;
		}
	}
	return CreateSolidBrush(color);
}

//void CnCS_TC::SetBitmapIDs(LPTABBMPIDS ids, int StyleID)
//{
	//UNREFERENCED_PARAMETER(ids);
	//UNREFERENCED_PARAMETER(StyleID);

	//if (StyleID == STYLEID_BLACK)
	//{
	//	ids->tl = IDB_TABBKGND_LEFT_BLACK;
	//	ids->tm = IDB_TABBKGND_MID_BLACK;
	//	ids->tr = IDB_TABBKGND_RIGHT_BLACK;

	//	ids->tl_h = IDB_TABBKGND_LEFT_BLACK_HGL;
	//	ids->tm_h = IDB_TABBKGND_MID_BLACK_HGL;
	//	ids->tr_h = IDB_TABBKGND_RIGHT_BLACK_HGL;
	//}
	//else if (StyleID == STYLEID_GREEN)
	//{
	//	ids->tl = IDB_TABBKGND_LEFT_GREEN;
	//	ids->tm = IDB_TABBKGND_MID_GREEN;
	//	ids->tr = IDB_TABBKGND_RIGHT_GREEN;

	//	ids->tl_h = IDB_TABBKGND_LEFT_GREEN_HGL;
	//	ids->tm_h = IDB_TABBKGND_MID_GREEN_HGL;
	//	ids->tr_h = IDB_TABBKGND_RIGHT_GREEN_HGL;

	//}
	//else if (StyleID == STYLEID_DARKGRAY)
	//{
	//	ids->tl = IDB_TABBKGND_LEFT_DARKGRAY;
	//	ids->tm = IDB_TABBKGND_MID_DARKGRAY;
	//	ids->tr = IDB_TABBKGND_RIGHT_DARKGRAY;

	//	ids->tl_h = IDB_TABBKGND_LEFT_DARKGRAY_HGL;
	//	ids->tm_h = IDB_TABBKGND_MID_DARKGRAY_HGL;
	//	ids->tr_h = IDB_TABBKGND_RIGHT_DARKGRAY_HGL;

	//}
	//else if (StyleID == STYLEID_LIGHTGRAY)
	//{
	//	ids->tl = IDB_TABBKGND_LEFT_LIGHTGRAY;
	//	ids->tm = IDB_TABBKGND_MID_LIGHTGRAY;
	//	ids->tr = IDB_TABBKGND_RIGHT_LIGHTGRAY;

	//	ids->tl_h = IDB_TABBKGND_LEFT_LIGHTGRAY_HGL;
	//	ids->tm_h = IDB_TABBKGND_MID_LIGHTGRAY_HGL;
	//	ids->tr_h = IDB_TABBKGND_RIGHT_LIGHTGRAY_HGL;
	//}
	//else
	//{
	//	ids->tl = IDB_TABBKGND_LEFT_LIGHTGRAY;
	//	ids->tm = IDB_TABBKGND_MID_LIGHTGRAY;
	//	ids->tr = IDB_TABBKGND_RIGHT_LIGHTGRAY;

	//	ids->tl_h = IDB_TABBKGND_LEFT_LIGHTGRAY_HGL;
	//	ids->tm_h = IDB_TABBKGND_MID_LIGHTGRAY_HGL;
	//	ids->tr_h = IDB_TABBKGND_RIGHT_LIGHTGRAY_HGL;
	//}
//}

void CnCS_TC::SetEditStyleColorsFromStyleInfo(LPEDITSTYLECOLORS esc, LPAPPSTYLEINFO pStyleInfo)
{
	int defaultSchemeIndex = 1;

	auto dataContainer =
		reinterpret_cast<ApplicationData*>(
			getDefaultApplicationDataContainer()
			);
	if (dataContainer != nullptr)
	{
		defaultSchemeIndex = dataContainer->getIntegerData(DATAKEY_SETTINGS_COLORSCHEME_DEFINDEX, 1);
	}

	if (defaultSchemeIndex == 0)
	{
		esc->background = pStyleInfo->TabColor;
		esc->defaultTextcolor = InvertColor(pStyleInfo->TabColor);

		switch (pStyleInfo->StyleID)
		{
		case STYLEID_BLACK:
			esc->A = RGB(200, 200, 0);
			esc->B = RGB(88, 160, 167);
			esc->C = RGB(240, 240, 240);// unused?
			esc->D = RGB(240, 240, 240);// unused?
			esc->E = RGB(197, 58, 145);
			esc->F = RGB(55, 200, 146);
			esc->G = RGB(34, 137, 221);
			esc->H = RGB(221, 123, 34);
			esc->I = RGB(215, 156, 100);
			esc->J = RGB(215, 156, 100);
			esc->K = RGB(215, 156, 100);
			esc->L = RGB(255, 0, 128);
			esc->M = RGB(255, 255, 0);
			esc->N = RGB(128, 255, 128);
			esc->O = RGB(240, 240, 240);// unused?
			esc->P = RGB(200, 200, 200);
			esc->Q = RGB(226, 191, 156);
			esc->R = RGB(97, 128, 158);
			esc->S = RGB(55, 200, 146);
			esc->T = RGB(255, 0, 0);
			esc->U = RGB(240, 240, 240);// unused?
			esc->V = RGB(0, 255, 255);
			esc->W = RGB(240, 240, 240);
			esc->X = RGB(220, 220, 200);
			esc->Y = RGB(220, 220, 180);
			esc->Z = RGB(220, 220, 160);
			esc->Annotation = RGB(0, 255, 255);
			esc->LineNumber = esc->defaultTextcolor;
			break;
		case STYLEID_DARKGRAY:
			esc->A = RGB(200, 200, 0);
			esc->B = RGB(88, 160, 167);
			esc->C = RGB(240, 240, 240);// unused?
			esc->D = RGB(240, 240, 240);// unused?
			esc->E = RGB(197, 58, 145);
			esc->F = RGB(55, 200, 146);
			esc->G = RGB(34, 137, 221);
			esc->H = RGB(221, 123, 34);
			esc->I = RGB(215, 156, 100);
			esc->J = RGB(215, 156, 100);
			esc->K = RGB(215, 156, 100);
			esc->L = RGB(255, 0, 128);
			esc->M = RGB(220, 220, 0);
			esc->N = RGB(128, 255, 128);
			esc->O = RGB(240, 240, 240);// unused?
			esc->P = RGB(200, 200, 200);
			esc->Q = RGB(226, 191, 156);
			esc->R = RGB(97, 128, 158);
			esc->S = RGB(55, 200, 146);
			esc->T = RGB(200, 50, 50);
			esc->U = RGB(240, 240, 240);// unused?
			esc->V = RGB(0, 255, 255);
			esc->W = RGB(240, 240, 240);
			esc->X = RGB(220, 220, 200);
			esc->Y = RGB(220, 220, 180);
			esc->Z = RGB(220, 220, 160);
			esc->Annotation = RGB(0, 255, 255);
			esc->LineNumber = esc->defaultTextcolor;
			break;
		//case STYLEID_GREEN:
		//	esc->A = RGB(200, 200, 0);
		//	esc->B = RGB(150, 100, 220);
		//	esc->C = RGB(240, 240, 240);// unused?
		//	esc->D = RGB(240, 240, 240);// unused?
		//	esc->E = RGB(197, 58, 145);
		//	esc->F = RGB(55, 200, 146);
		//	esc->G = RGB(34, 137, 221);
		//	esc->H = RGB(221, 123, 34);
		//	esc->I = RGB(255, 128, 128);
		//	esc->J = RGB(255, 128, 128);
		//	esc->K = RGB(255, 128, 128);
		//	esc->L = RGB(255, 0, 128);
		//	esc->M = RGB(255, 255, 0);
		//	esc->N = RGB(128, 255, 128);
		//	esc->O = RGB(240, 240, 240);// unused?
		//	esc->P = RGB(200, 200, 200);
		//	esc->Q = RGB(226, 191, 156);
		//	esc->R = RGB(97, 128, 158);
		//	esc->S = RGB(55, 200, 146);
		//	esc->T = RGB(200, 50, 50);
		//	esc->U = RGB(240, 240, 240);// unused?
		//	esc->V = RGB(0, 255, 255);
		//	esc->W = RGB(240, 240, 240);
		//	esc->X = RGB(20, 20, 100);
		//	esc->Y = RGB(20, 20, 80);
		//	esc->Z = RGB(20, 20, 60);
		//	esc->Annotation = RGB(0, 255, 255);
		//	esc->LineNumber = esc->defaultTextcolor;
		//	break;
		case STYLEID_LIGHTGRAY:
			esc->A = RGB(160, 160, 20);
			esc->B = RGB(88, 160, 167);
			esc->C = RGB(240, 240, 240);// unused?
			esc->D = RGB(240, 240, 240);// unused?
			esc->E = RGB(197, 58, 145);
			esc->F = RGB(0, 0, 128);
			esc->G = RGB(34, 137, 221);
			esc->H = RGB(221, 123, 34);
			esc->I = RGB(255, 128, 128);
			esc->J = RGB(255, 128, 128);
			esc->K = RGB(255, 128, 128);
			esc->L = RGB(255, 0, 128);
			esc->M = RGB(230, 230, 145);
			esc->N = RGB(128, 180, 128);
			esc->O = RGB(240, 240, 240);// unused?
			esc->P = RGB(80, 80, 80);
			esc->Q = RGB(126, 91, 56);
			esc->R = RGB(97, 128, 158);
			esc->S = RGB(0, 0, 128);
			esc->T = RGB(255, 0, 0);
			esc->U = RGB(240, 240, 240);// unused?
			esc->V = RGB(0, 255, 255);
			esc->W = RGB(240, 240, 240);
			esc->X = RGB(60, 60, 0);
			esc->Y = RGB(80, 80, 20);
			esc->Z = RGB(100, 100, 40);
			esc->Annotation = RGB(0, 220, 220);
			esc->LineNumber = esc->defaultTextcolor;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (pStyleInfo->StyleID)
		{
		case STYLEID_BLACK:
			esc->A = 0x98b1ae;
			esc->B = 0x857d91;
			esc->C = 0xd7d7d7;
			esc->D = 0xd7d7d7;
			esc->E = 0xd7d7d7;
			esc->F = 0xa5a5a5;
			esc->G = 0xe1cac4;
			esc->H = 0xd7d7d7;
			esc->I = 0x969696;
			esc->J = 0xd7d7d7;
			esc->K = 0x969696;
			esc->L = 0xd7d7d7;
			esc->M = 0xc8e4e6;
			esc->N = 0x77b578;
			esc->O = 0xd7d7d7;
			esc->P = 0xd7d7d7;
			esc->Q = 0xd7d7d7;
			esc->R = 0xd7d7d7;
			esc->S = 0x9987a3;
			esc->T = 0x86799b;
			esc->U = 0xd7d7d7;
			esc->V = 0xd7d7d7;
			esc->W = 0xd7d7d7;
			esc->X = 0xf0f0f0;
			esc->Y = 0x677385;
			esc->Z = 0xb4b4b4;
			esc->Annotation = 0x838383;
			esc->defaultTextcolor = 0xffffff;
			esc->LineNumber = 0x4b4b4b;
			esc->background = 0x282828;
			break;
		case STYLEID_DARKGRAY:
			esc->A = 0xc0c0c0;
			esc->B = 0xa7a058;
			esc->C = 0xf0f0f0;
			esc->D = 0xf0f0f0;
			esc->E = 0xa5a5a5;
			esc->F = 0xd8ebb8;
			esc->G = 0xf1d0a9;
			esc->H = 0xb9d6f4;
			esc->I = 0xd7e4f4;
			esc->J = 0xb7d0ec;
			esc->K = 0xcedff2;
			esc->L = 0xd9b3ff;
			esc->M = 0xb7ffff;
			esc->N = 0xd2ffd2;
			esc->O = 0xf0f0f0;
			esc->P = 0xc8c8c8;
			esc->Q = 0x9cbfe2;
			esc->R = 0xe1e1e1;
			esc->S = 0xd5eab0;
			esc->T = 0xafafeb;
			esc->U = 0xf0f0f0;
			esc->V = 0xa6a600;
			esc->W = 0xf0f0f0;
			esc->X = 0xc8dcdc;
			esc->Y = 0xb4dcdc;
			esc->Z = 0xd2d2d2;
			esc->Annotation = 0xeeeeaf;
			esc->defaultTextcolor = 0x9b9b9b;
			esc->LineNumber = 0x9b9b9b;
			esc->background = 0x646464;
			break;
		case STYLEID_LIGHTGRAY:
			esc->A = 0x32485f;
			esc->B = 0x857d91;
			esc->C = 0xd7d7d7;
			esc->D = 0xd7d7d7;
			esc->E = 0x51613f;
			esc->F = 0x969696;
			esc->G = 0x633c32;
			esc->H = 0xd7d7d7;
			esc->I = 0x5c5c5c;
			esc->J = 0xd7d7d7;
			esc->K = 0x585858;
			esc->L = 0xd7d7d7;
			esc->M = 0x346d72;
			esc->N = 0x87a58a;
			esc->O = 0xd7d7d7;
			esc->P = 0x544a5e;
			esc->Q = 0x3c4f41;
			esc->R = 0x696969;
			esc->S = 0x6a5973;
			esc->T = 0x635775;
			esc->U = 0xd7d7d7;
			esc->V = 0xd7d7d7;
			esc->W = 0x4e2c4e;
			esc->X = 0x646464;
			esc->Y = 0x5b5b5b;
			esc->Z = 0x8d8d8d;
			esc->Annotation = 0xa5a55a;
			esc->defaultTextcolor = 0x1e1e1e;
			esc->LineNumber = 0xa5a5a5;
			esc->background = 0xc8c8c8;
			break;
		default:
			break;
		}
	}
}

bool CnCS_TC::SetEditStyleColorsFrom_XMLFile(LPEDITSTYLECOLORS esc)
{
	auto dataContainer
		= reinterpret_cast<ApplicationData*>(
			getDefaultApplicationDataContainer()
		);
	if (dataContainer != nullptr)
	{
		auto box
			= reinterpret_cast<iBox<iString>*>(
				dataContainer->lookUp(DATAKEY_SETTINGS_COLORSCHEME_CUR_NAME)
			);
		if (box != nullptr)
		{
			AppPath pathManager;
			auto path = pathManager.Get(PATHID_FOLDER_CNCSUITE_USERSTYLES_EDITOR);
			path += L"\\\0";
			path += box->get();
			path += L".xml";

			XML_Parser parser;
			
			if (parser.OpenDocument(path))
			{
				if (parser.Parse())
				{
					auto xmlStruct = parser.getDocumentStructure();
					if (xmlStruct.GetCount() > 0)
					{
						if (colorSchemeManager::xmlToColorStruct(&xmlStruct, esc))
						{
							// ...

							return true;
						}
					}
				}
			}
			SafeRelease(&box);
		}
	}
	return false;
}

void CnCS_TC::SetEditControlProperties(LPEDITCONTROLPROPERTIES pECS)
{
	auto internalDataContainer =
		reinterpret_cast<ApplicationData*>(
			getApplicationDataContainerFromFilekey(FILEKEY_INTERNAL_SETTINGS)
			);

	if (internalDataContainer != nullptr)
	{
		pECS->focusmarkCor_top =
			internalDataContainer->getIntegerData(DATAKEY_INTSET_EDITOR_FOCUSRECTOFFSET_TOP, 1);
		pECS->focusmarkCor_bottom =
			internalDataContainer->getIntegerData(DATAKEY_INTSET_EDITOR_FOCUSRECTOFFSET_BOTTOM, 5);
	}

	pECS->autocolor = this->iParam.textcolor_active;
	pECS->autocomplete = this->iParam.autocomplete_active;
	pECS->autonum = this->iParam.autonum_active;
	pECS->uppercase = this->iParam.uppercase_active;
	pECS->focusmark = this->iParam.focusmark_active;
	pECS->charHeight = DPIScale(this->iParam.FontHeight);
	pECS->lineOffset = this->iParam.lineOffset;
	pECS->isBold = this->iParam.isBold;
	pECS->autosyntax = this->iParam.autoSyntax;

	if ((this->availableFonts.GetCount() > 0) && (this->iParam.FontFamilyIndex >= 0))
	{
		StringCbCopy(
			pECS->charSet,
			sizeof(pECS->charSet),
			this->availableFonts.GetAt(
				this->iParam.FontFamilyIndex
			).GetData()
		);
	}
	else
	{
		StringCbCopy(
			pECS->charSet,
			sizeof(pECS->charSet),
			L"Consolas"
		);
	}
}

BOOL CnCS_TC::InitializeValues(LPTCSTARTUPINFO tcInfo)
{
	if (tcInfo != nullptr)
	{
		// copy the working directory
		if (CopyStringToPtrA(tcInfo->WorkingDirectory, &this->iParam.WorkingDirectory) == TRUE)
		{
			// get the data container
			auto dataContainer
				= reinterpret_cast<ApplicationData*>(
					getDefaultApplicationDataContainer()
					);

			if (dataContainer != nullptr)
			{
				bool styleIsSet = false;

				// save the startupinfo ????
				// this->iParam.StartupAction = tcInfo.mode; ??
				this->iParam.StartupAction = TCP_NEWTAB;

				// set the current style
				this->iParam.currentStyle = this->styleInfo.StyleID;

				// look for the inital source of the editcolor-struct				
				this->iParam.UseXMLFileForUserColors
					= dataContainer->getBooleanData(DATAKEY_SETTINGS_COLORSCHEME_USAGE, false)
					? TRUE : FALSE;

				// set autosyntax
				this->setAutosyntaxProperties(&this->iParam.autoSyntax);

				// set edit colors
				if (this->iParam.UseXMLFileForUserColors)
				{
					if (this->SetEditStyleColorsFrom_XMLFile(&this->editStyleColors))
					{
						styleIsSet = true;
					}
				}							
				if (!styleIsSet)
				{
					this->SetEditStyleColorsFromStyleInfo(&this->editStyleColors, &this->styleInfo);
				}

				// initially - show/hide - the description window:
				this->iParam.descWnd_visible
					= dataContainer->getBooleanData(DATAKEY_SETTINGS_SHOW_DESCWND, true)
					? TRUE : FALSE;

				// execute open request in a new tab or the current visible tab
				this->iParam.OpenNewPathInNewTab
					= dataContainer->getBooleanData(DATAKEY_SETTINGS_OPEN_IN_NEW_TAB, true)
					? TRUE : FALSE;

				// auto save content
				this->iParam.AutoSave
					= dataContainer->getBooleanData(DATAKEY_SETTINGS_AUTOSAVE, false)
					? TRUE : FALSE;

				// Editor properties:
				this->iParam.autocomplete_active
					= dataContainer->getBooleanData(DATAKEY_EDITOR_AUTOCOMPLETE, true)
					? TRUE : FALSE;

				this->iParam.autonum_active
					= dataContainer->getBooleanData(DATAKEY_EDITOR_AUTONUM, true)
					? TRUE : FALSE;

				this->iParam.textcolor_active
					= dataContainer->getBooleanData(DATAKEY_EDITOR_TEXTCOLOR, true)
					? TRUE : FALSE;

				this->iParam.uppercase_active
					= dataContainer->getBooleanData(DATAKEY_EDITOR_UPPERCASE, true)
					? TRUE : FALSE;

				this->iParam.focusmark_active
					= dataContainer->getBooleanData(DATAKEY_EDITOR_FOCUSMARK, true)
					? TRUE : FALSE;

				// font properties
				this->iParam.FontHeight = dataContainer->getIntegerData(DATAKEY_EDITOR_FONT_HEIGHT, 275);
				this->iParam.FontFamilyIndex = dataContainer->getIntegerData(DATAKEY_EDITOR_FONT_FAMILY_INDEX, 0);
				this->iParam.lineOffset = dataContainer->getIntegerData(DATAKEY_EDITOR_LINEOFFSET, 72);
				this->iParam.isBold =
					dataContainer->getBooleanData(DATAKEY_EDITOR_BOLDSTYLE, true)
					? TRUE : FALSE;

				return TRUE;
			}
		}
	}
	return FALSE;
}

void CnCS_TC::LoadFontResource()
{
	// set the default font:
	this->availableFonts.AddItem(L"Consolas");

	// load the fonts dynamically from user directory
	bool exit = false;

	AppPath aPath;
	auto path =
		aPath.Get(PATHID_FOLDER_CNCSUITE_APPLICATIONFOLDER_FONTS);

	iString wcPath(L"\\\\?\\");
	wcPath.Append(path);
	wcPath.Append(L"\\*");

	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA wfd;

	auto bfpo = CreateBasicFPO();
	if (bfpo != nullptr)
	{
		hFind = FindFirstFile(wcPath.GetData(), &wfd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			while (wfd.cFileName[0] == L'.')
			{
				if (FindNextFile(hFind, &wfd) == 0)
				{
					exit = true;
					break;
				}
			}
			if (!exit)
			{
				do
				{
					TCHAR *ext = nullptr;
					bfpo->GetFileExtension(wfd.cFileName, &ext);

					if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						iString _ext_(ext);
						if (_ext_.Equals(L".ttf") || _ext_.Equals(L".otf"))
						{
							auto pathtofile = path + L"\\" + wfd.cFileName;
							auto fontname =
								GetFontNameFromFile(
									pathtofile.GetData()
								);

							if (fontname.GetLength() > 0)
							{
								auto result =
									AddFontResourceEx(
										pathtofile.GetData(),
										FR_PRIVATE,
										0
									);
								if (result != 0)
								{
									// check if the name already exists (->bold & regular could be listed twice)

									this->availableFonts.AddItem(fontname);
								}
							}
						}
						else
						{
							// it is no font or the font is not in the right format
						}
					}
					else
					{
						// it's a directory so do nothing...
					}

				} while (FindNextFile(hFind, &wfd) != 0);
			}
			FindClose(hFind);
		}
		SafeRelease(&bfpo);
	}
}

BOOL CnCS_TC::LoadDESCPropertyNames()
{
	AppPath appPath;

	auto bfpo = CreateBasicFPO();
	auto res =
		(bfpo != nullptr)
		? TRUE : FALSE;

	if (res)
	{
		auto pathToProp = appPath.Get(PATHID_FILE_DESCPROP_ONE);
		res =
			pathToProp.succeeded()
			? TRUE : FALSE;

		if (res)
		{
			SafeDeleteArray(&this->iParam.DESC_property1);

			if (!bfpo->LoadBufferFmFileAsUtf8(
				&this->iParam.DESC_property1,
				pathToProp.GetData() ))
			{
				res =
					(CopyStringToPtrA(
						getStringFromResource(UI_TABCTRL_DESCPLACEHOLDER_1),
						&this->iParam.DESC_property1) == TRUE)
					? TRUE : FALSE;
			}
			if (res)
			{
				pathToProp = appPath.Get(PATHID_FILE_DESCPROP_TWO);
				res =
					pathToProp.succeeded()
					? TRUE : FALSE;

				if (res)
				{
					SafeDeleteArray(&this->iParam.DESC_property2);

					if (!bfpo->LoadBufferFmFileAsUtf8(
						&this->iParam.DESC_property2,
						pathToProp.GetData() ))
					{
						res =
							(CopyStringToPtrA(
								getStringFromResource(UI_TABCTRL_DESCPLACEHOLDER_2),
								&this->iParam.DESC_property2) == TRUE)
							? TRUE : FALSE;
					}
					if (res)
					{
						pathToProp = appPath.Get(PATHID_FILE_DESCPROP_THREE);
						res =
							pathToProp.succeeded()
							? TRUE : FALSE;

						if (res)
						{
							SafeDeleteArray(&this->iParam.DESC_property3);

							if (!bfpo->LoadBufferFmFileAsUtf8(
								&this->iParam.DESC_property3,
								pathToProp.GetData()))
							{
								res =
									(CopyStringToPtrA(
										getStringFromResource(UI_TABCTRL_DESCPLACEHOLDER_3),
										&this->iParam.DESC_property3) == TRUE)
									? TRUE : FALSE;
							}
							if (res)
							{
								// ...
							}
						}
					}
				}
			}
		}
	}
	return res;
}

BOOL CnCS_TC::SaveDESCPropertyNames()
{
	auto bfpo =
		CreateBasicFPO();

	auto result =
		(bfpo != nullptr)
		? TRUE : FALSE;

	if (result)
	{
		AppPath appPath;

		auto pathToFile =
			appPath.Get(PATHID_FILE_DESCPROP_ONE);

		result =
			pathToFile.succeeded()
			? TRUE : FALSE;

		if (result)
		{
			result =
				bfpo->SaveBufferToFileAsUtf8(
					this->iParam.DESC_property1,
					pathToFile.GetData()
				);
			if (result)
			{
				pathToFile =
					appPath.Get(PATHID_FILE_DESCPROP_TWO);

				result =
					pathToFile.succeeded()
					? TRUE : FALSE;

				if (result)
				{
					result =
						bfpo->SaveBufferToFileAsUtf8(
							this->iParam.DESC_property2,
							pathToFile.GetData()
						);
					if (result)
					{
						pathToFile =
							appPath.Get(PATHID_FILE_DESCPROP_THREE);

						result =
							pathToFile.succeeded()
							? TRUE : FALSE;

						if (result)
						{
							result =
								bfpo->SaveBufferToFileAsUtf8(
									this->iParam.DESC_property3,
									pathToFile.GetData()
								);
							if (result)
							{
								// ...
							}
						}
					}
				}
			}
		}
		SafeRelease(&bfpo);
	}
	return result;
}

BOOL CnCS_TC::PropWndSoftControl(DWORD Mode)
{
	HANDLE hThread;
	DWORD threadID;
	TCTHREADDATA tdata;
	tdata.Mode = Mode;
	tdata.additional_val = 0;

	hThread = CreateThread(NULL, 0, CnCS_TC::PropWndSoftControlProc, (LPVOID)&tdata, 0, &threadID);
	if (hThread)
	{
		this->iParam.descWnd_isMoving = TRUE;

		WaitForSingleObject(hThread, 50);
	}
	else
		return FALSE;

	return TRUE;
}

HWND CnCS_TC::GetCurrentVisibleEditbox(HWND tcFrame)
{
	__try
	{
		// tcFrame can be NULL, but only in the same thread;
		if (tcFrame == NULL)tcFrame = this->TCFrame;

		CnCS_TC* pTC = reinterpret_cast<CnCS_TC*>(GetWindowLongPtr(tcFrame, GWLP_USERDATA));
		if (pTC != NULL)
		{
			HWND tab = pTC->iParam.hwndActiveTab;
			if (tab != NULL)
			{
				LPTABPROPERTY ptb = reinterpret_cast<LPTABPROPERTY>(GetWindowLongPtr(tab, GWLP_USERDATA));
				if (ptb != NULL)
				{
					return ptb->AssocEditWnd;
				}
				else
					return (HWND)NULL;
			}
			else
				return (HWND)NULL;
		}
		else
			return (HWND)NULL;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return (HWND)NULL;
	}
}

LPTABPROPERTY CnCS_TC::GetActiveTabProperty()
{
	return reinterpret_cast<LPTABPROPERTY>(GetWindowLongPtr(this->iParam.hwndActiveTab, GWLP_USERDATA));
}

LPTABPROPERTY CnCS_TC::GetTabProperty(DWORD tabNr)
{
	HWND tab = GetDlgItem(
		this->TCFrame,
		TAB_ID_START + tabNr
	);

	if (tab)
	{
		return reinterpret_cast<LPTABPROPERTY>(
			GetWindowLongPtr(
				tab, GWLP_USERDATA	));
	}
	else
		return nullptr;
}

BOOL CnCS_TC::ConfigPropEdit(HWND edit)
{
	if (edit == nullptr)
		return FALSE;

	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_CHARSET | CFM_COLOR | CFM_SIZE | CFM_FACE;
	cf.bCharSet = ANSI_CHARSET;
	cf.dwEffects = 0;
	cf.yHeight =
		ConvertYPixToTwips(
			edit,
			DPIScale(20)	// charheight
		);
	cf.crTextColor = this->styleInfo.specialTextcolor;
	StringCbCopy(cf.szFaceName, sizeof(cf.szFaceName), L"Segoe UI\0");

	SendMessage(edit, EM_SETBKGNDCOLOR, (WPARAM)0, (LPARAM)this->styleInfo.Stylecolor);
	SendMessage(edit, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, reinterpret_cast<LPARAM>(&cf));

	return SetWindowSubclass(
		edit,
		CnCS_TC::DescEditSub,
		0,
		reinterpret_cast<DWORD_PTR>(this)
	);
}

//BOOL CnCS_TC::OpenDisplayAndFormat(LPCTSTR path, DWORD flags)
//{
//	BOOL result = FALSE;
//
//	if (path != NULL)
//	{
//		Open_Save_CTRL *osc = new Open_Save_CTRL();
//
//		result = (osc == NULL) ? FALSE : TRUE;
//		if (result)
//		{
//			HWND Tab = this->iParam.hwndActiveTab;
//			if (Tab != NULL)
//			{
//				LPTABPROPERTY ptc = reinterpret_cast<LPTABPROPERTY>(GetWindowLongPtr(Tab, GWLP_USERDATA));
//				if (ptc != NULL)
//				{
//					TCHAR* text = NULL;
//
//					if (flags & DO_CLEANUP)
//					{
//						// clean old content
//						this->CleanUpTabStructForNewContent(ptc);
//					}
//					if (flags & SET_DISPLAYNAME)
//					{
//						// set new content
//						CopyStringToPtr(path, &ptc->Path);
//						GetFilenameOutOfPath(path, &ptc->displayname, TRUE);
//					}
//
//					osc->OpenWithoutLoading(
//						path,
//						&text,
//						&ptc->DESC1,
//						&ptc->DESC2,
//						&ptc->DESC3);
//
//					ptc->Editcontrol->SetTextContent(text, TRUE, TRUE, FALSE);
//					SafeDeleteArray(&text);
//
//					this->SetDescriptions(ptc->DESC1, ptc->DESC2, ptc->DESC3);
//
//					RedrawWindow(Tab, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_ALLCHILDREN);
//				}
//			}
//			SafeRelease(&osc);
//		}
//	}
//	return result;
//}

BOOL CnCS_TC::OpenDisplayAndFormat(const CnC3File & file, DWORD flags)
{
	if (file.HasPath())
	{
		HWND Tab = this->iParam.hwndActiveTab;
		if (Tab != NULL)
		{
			LPTABPROPERTY ptc =
				reinterpret_cast<LPTABPROPERTY>(
					GetWindowLongPtr(Tab, GWLP_USERDATA)
				);

			if (ptc != NULL)
			{

				if (flags & DO_CLEANUP)
				{
					// clean old content
					this->CleanUpTabStructForNewContent(ptc);
				}
				if (flags & SET_DISPLAYNAME)
				{
					// set new content
					CopyStringToPtr(
						file.GetPath(),
						&ptc->Path
					);
					GetFilenameOutOfPath(
						file.GetPath(),
						&ptc->displayname,
						TRUE
					);
				}

				ptc->Editcontrol->SetTextContent(
					file.GetNCContent(),
					TRUE, TRUE, FALSE
				);

				file.GetProperty(CnC3File::PID_DESCRIPTION_ONE, &ptc->DESC1);
				file.GetProperty(CnC3File::PID_DESCRIPTION_TWO, &ptc->DESC2);
				file.GetProperty(CnC3File::PID_DESCRIPTION_THREE, &ptc->DESC3);

				this->SetDescriptions(
					ptc->DESC1,
					ptc->DESC2,
					ptc->DESC3
				);

				RedrawWindow(Tab, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_ALLCHILDREN);
			}
		}
	}
	return TRUE;
}

BOOL CnCS_TC::getCurrentSelectedText(LPSELECTIONINFO selInfo)
{
	BOOL result;

	HWND edit = this->GetCurrentVisibleEditbox(NULL);
	result = (edit) ? TRUE : FALSE;
	if(result)
	{
		SendMessage(edit, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&selInfo->cr));

		if (selInfo->cr.cpMax > selInfo->cr.cpMin)
		{
			// get selection-absoluteRange
			selInfo->selContent = new TCHAR[(sizeof(TCHAR)*((selInfo->cr.cpMax - selInfo->cr.cpMin) + 2))];

			result = (selInfo->selContent != nullptr) ? TRUE : FALSE;
			if (result)
			{
				GETTEXTEX gtx;
				gtx.cb = (sizeof(TCHAR)*((selInfo->cr.cpMax - selInfo->cr.cpMin) + 2));
				gtx.codepage = 1200;
				gtx.flags = GT_SELECTION;
				gtx.lpDefaultChar = nullptr;
				gtx.lpUsedDefChar = nullptr;

				selInfo->numChars =		//x64
					static_cast<int>(
						SendMessage(
							edit,
							EM_GETTEXTEX,
							reinterpret_cast<WPARAM>(&gtx),
							reinterpret_cast<LPARAM>(selInfo->selContent)
							)
						);

				result = (selInfo->numChars > 0) ? TRUE : FALSE;
				if (result)
				{
					// ...
				}
			}
		}
		else if (selInfo->cr.cpMin == 0 && selInfo->cr.cpMax == -1)
		{
			// all is selected
			result = GetRichEditContent(edit, &selInfo->selContent);
			if (result)
			{
				// ...

			}
		}
		else
		{
			// nothing is selected
			result = FALSE;
		}
	}
	return result;
}

BOOL CnCS_TC::onGetDescriptionNames(LPDESCRIPTIONINFO dInfo)
{
	BOOL result;

	result = (dInfo != nullptr) ? TRUE : FALSE;
	if (result)
	{
		result =
			(CopyStringToPtr(this->iParam.DESC_property1, &dInfo->desc1) == TRUE)
			? TRUE : FALSE;

		if (result)
		{
			result =
				(CopyStringToPtr(this->iParam.DESC_property2, &dInfo->desc2) == TRUE)
				? TRUE : FALSE;

			if (result)
			{
				result =
					(CopyStringToPtr(this->iParam.DESC_property3, &dInfo->desc3) == TRUE)
					? TRUE : FALSE;
				
				if (result)
				{
					// ...
				}
			}
		}
	}
	return result;
}

void CnCS_TC::onSetDescriptionNames(LPDESCRIPTIONINFO dInfo)
{
	if (dInfo != nullptr)
	{
		if (dInfo->desc1 != nullptr)
		{
			SafeDeleteArray(&this->iParam.DESC_property1);
			CopyStringToPtr(dInfo->desc1, &this->iParam.DESC_property1);
		}
		if (dInfo->desc2 != nullptr)
		{
			SafeDeleteArray(&this->iParam.DESC_property2);
			CopyStringToPtr(dInfo->desc2, &this->iParam.DESC_property2);
		}
		if (dInfo->desc3 != nullptr)
		{
			SafeDeleteArray(&this->iParam.DESC_property3);
			CopyStringToPtr(dInfo->desc3, &this->iParam.DESC_property3);
		}
		if (this->iParam.descWnd_exists)
		{
			RedrawWindow(
				GetDlgItem(this->TCFrame, ID_DESCFRAME),
				nullptr, nullptr, RDW_INVALIDATE);
		}
		this->SaveDESCPropertyNames();
	}
	else
	{
		this->LoadDESCPropertyNames();

		if (this->iParam.descWnd_exists)
		{
			RedrawWindow(
				GetDlgItem(this->TCFrame, ID_DESCFRAME),
				nullptr, nullptr, RDW_INVALIDATE);
		}
	}
}

void CnCS_TC::getEditcontrolProperties(LPEDITSTYLECOLORS esc, LPEDITCONTROLPROPERTIES ecp)
{
	APPSTYLEINFO StyleInfo;
	SendMessage(this->Main, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&StyleInfo));

	this->SetEditStyleColorsFromStyleInfo(esc, &StyleInfo);
	this->SetEditControlProperties(ecp);
}

void CnCS_TC::setAutosyntaxProperties(LPAUTOSYNTAXPROPERTY pSyntax)
{
	if (pSyntax != nullptr)
	{
		autosyntaxManager::GetAutosyntaxSettings(pSyntax);

		// TODO: load it from settings later!!!

		//pSyntax->AutoLevitation = FALSE;			// what is that!?
		//pSyntax->eraseLinenumberOnBackspace = TRUE;
		//pSyntax->IsOn = FALSE;
		//pSyntax->MainLevelNumLength = 3;
		//pSyntax->noSpaceInLineNumber = FALSE;
		//pSyntax->SubLevelNumLength = 2;
		//pSyntax->UseDifferentNumLengthInSubProg = TRUE;
		//pSyntax->autoinsertBrackets = TRUE;
		//pSyntax->autonumStartLine = 2;
		//pSyntax->maximumLinenumber = 9999;
		//pSyntax->noSpaceBetweenBraces = TRUE;		// invert??
		//pSyntax->newEvenLineNumberOnTrigger = TRUE;
		//pSyntax->lineNumberStartValue = 10;
		//pSyntax->numStepInSubProgramm = 1;
		//pSyntax->useNoDifferentLinenumbersInSubprogram = FALSE; // set this to invertion of: pSyntax->UseDifferentNumLengthInSubProg ?
		//pSyntax->useMultilineAnnotations = FALSE;
		//pSyntax->useEndProgDetection = TRUE;
	}
}

void CnCS_TC::updateAutosyntaxSettings()
{
	iString subprog, endprog, newline;
	autosyntaxManager::GetTriggerStrings(subprog, endprog, newline);

	this->setAutosyntaxProperties(&this->iParam.autoSyntax);

	for (DWORD i = 0; i < this->TABCount; i++)
	{
		auto ptp = this->GetTabProperty(i);
		if (ptp != nullptr)
		{
			ptp->Editcontrol->SetAutosyntaxProperty(&this->iParam.autoSyntax);
			ptp->Editcontrol->insertUserdefinedTriggerStrings(
				subprog.GetData(),
				endprog.GetData(),
				newline.GetData()
			);

			// convert syntax ????
			// if as->isON -> convert???
		}
	}

}

void CnCS_TC::initDescriptionBuffer(LPTABPROPERTY ptp)
{
	if (ptp->DESC1 == nullptr)
		CopyStringToPtr(L"...", &ptp->DESC1);
	if (ptp->DESC2 == nullptr)
		CopyStringToPtr(L"...", &ptp->DESC2);
	if (ptp->DESC3 == nullptr)
		CopyStringToPtr(L"...", &ptp->DESC3);
}

void CnCS_TC::updateAutocompleteData()
{
	auto appProperties
		= reinterpret_cast<CnCSuite_Property*>(
			getApplicationPropertyComponent()
			);
	if (appProperties != nullptr)
	{
		int cnt = 0;

		auto acStrings
			= reinterpret_cast<LPAUTOCOMPLETESTRINGS>(
				appProperties->GetAutocompleteStrings(&cnt)
				);

		for (DWORD i = 0; i < this->TABCount; i++)
		{
			auto ptp = this->GetTabProperty(i);
			if (ptp != nullptr)
			{
				ptp->Editcontrol->SetAutocompleteStrings(acStrings, cnt);
			}
		}
	}
}

void CnCS_TC::comboboxSelChange(comboBox * cBox, int selindex)
{
	auto Id = cBox->getCtrlId();

	auto dataContainer
		= reinterpret_cast<ApplicationData*>(
			getDefaultApplicationDataContainer());

	if (dataContainer != nullptr)
	{
		auto ptp = this->GetActiveTabProperty();
		if (ptp != nullptr)
		{
			switch (Id)
			{
			case IDM_FONTFAMILYCOMBO:
				this->iParam.FontFamilyIndex = selindex;
				ptp->Editcontrol->SetFont(
					this->availableFonts.GetAt(selindex)
					.GetData()
				);
				dataContainer->saveValue(DATAKEY_EDITOR_FONT_FAMILY_INDEX, selindex);
				break;
			case IDM_FONTHEIGHTTRACK:
				this->iParam.FontHeight = 8 + (selindex * 2);
				ptp->Editcontrol->SetCharHeight(this->iParam.FontHeight);
				dataContainer->saveValue(DATAKEY_EDITOR_FONT_HEIGHT, selindex);
				break;
			default:
				break;
			}
		}
	}
}

BOOL CnCS_TC::isPathAlreadyOpen(LPTSTR path)
{
	for (DWORD i = 0; i < this->TABCount; i++)
	{
		auto ptp = this->GetTabProperty(i);
		if (ptp != nullptr)
		{
			auto isEqual = CompareStringsB(ptp->Path, path);
			if (isEqual)
				return TRUE;
		}
	}
	return FALSE;
}

bool CnCS_TC::isAlreadyOpened(const CnC3File& file)
{
	for (DWORD i = 0; i < this->TABCount; i++)
	{
		auto ptp = this->GetTabProperty(i);
		if (ptp != nullptr)
		{
			auto isEqual =
				CompareStringsB(
					ptp->Path,
					file.GetPath()
				);

			if (isEqual)
				return true;
		}
	}
	return false;
}


void CnCS_TC::setFocusOncurEdit()
{
	bool detachThreads = false;

	DWORD threadIdentifier =
		GetCurrentThreadId();

	if (threadIdentifier != this->_thisThreadId)
	{
		if (AttachThreadInput(threadIdentifier, this->_thisThreadId, TRUE))
		{
			detachThreads = true;
		}
	}

	auto edit = this->GetCurrentVisibleEditbox(this->TCFrame);
	SetFocus(edit);

	if (detachThreads)
	{
		AttachThreadInput(threadIdentifier, this->_thisThreadId, FALSE);
	}
}

void CnCS_TC::insertDiameterSymbol()
{
	auto ptp = this->GetActiveTabProperty();
	if (ptp != nullptr)
	{
		auto hWnd = GetFocus();
		if (hWnd != nullptr)
		{
			auto ID = static_cast<int>(GetWindowLong(hWnd, GWL_ID));

			switch (ID)
			{
			case ID_PROPERTYEDIT_ONE:
				SetRichEditContent(hWnd, L"\u00D8", ST_SELECTION | ST_NEWCHARS);
				break;
			case ID_PROPERTYEDIT_TWO:
				SetRichEditContent(hWnd, L"\u00D8", ST_SELECTION | ST_NEWCHARS);
				break;
			case ID_PROPERTYEDIT_THREE:
				SetRichEditContent(hWnd, L"\u00D8", ST_SELECTION | ST_NEWCHARS);
				break;
			default:
				return;
			}
			ptp->Content_Changed = TRUE;
			this->RedrawTab_s(REDRAW_CURRENT);
		}
	}
}

void CnCS_TC::tagsToInternalClipboard()
{
	HWND dFrame = GetDlgItem(this->TCFrame, ID_DESCFRAME);
	if (dFrame != nullptr)
	{
		SafeDeleteArray(&this->iParam.descClipboard.d1);
		SafeDeleteArray(&this->iParam.descClipboard.d2);
		SafeDeleteArray(&this->iParam.descClipboard.d3);

		GetRichEditContent(
			GetDlgItem(dFrame, ID_PROPERTYEDIT_ONE),
			&this->iParam.descClipboard.d1
		);
		GetRichEditContent(
			GetDlgItem(dFrame, ID_PROPERTYEDIT_TWO),
			&this->iParam.descClipboard.d2
		);
		GetRichEditContent(
			GetDlgItem(dFrame, ID_PROPERTYEDIT_THREE),
			&this->iParam.descClipboard.d3
		);
	}
}

void CnCS_TC::clipboardToTags()
{
	// tab changed
	auto ptp = this->GetActiveTabProperty();
	if (ptp != nullptr)
	{
		ptp->Content_Changed = TRUE;

		HWND dFrame = GetDlgItem(this->TCFrame, ID_DESCFRAME);
		if (dFrame != nullptr)
		{
			SetRichEditContent(
				GetDlgItem(dFrame, ID_PROPERTYEDIT_ONE),
				this->iParam.descClipboard.d1,
				ST_DEFAULT
			);
			SetRichEditContent(
				GetDlgItem(dFrame, ID_PROPERTYEDIT_TWO),
				this->iParam.descClipboard.d2,
				ST_DEFAULT
			);
			SetRichEditContent(
				GetDlgItem(dFrame, ID_PROPERTYEDIT_THREE),
				this->iParam.descClipboard.d3,
				ST_DEFAULT
			);

			this->RedrawTab_s(REDRAW_CURRENT);
		}
	}
}

void CnCS_TC::deleteAllTags()
{
	// tab changed
	auto ptp = this->GetActiveTabProperty();
	if (ptp != nullptr)
	{
		ptp->Content_Changed = TRUE;

		HWND dFrame = GetDlgItem(this->TCFrame, ID_DESCFRAME);
		if (dFrame != nullptr)
		{
			SetRichEditContent(
				GetDlgItem(dFrame, ID_PROPERTYEDIT_ONE),
				L"",
				ST_DEFAULT
			);
			SetRichEditContent(
				GetDlgItem(dFrame, ID_PROPERTYEDIT_TWO),
				L"",
				ST_DEFAULT
			);
			SetRichEditContent(
				GetDlgItem(dFrame, ID_PROPERTYEDIT_THREE),
				L"",
				ST_DEFAULT
			);

			this->RedrawTab_s(REDRAW_CURRENT);
		}
	}
}

void CnCS_TC::setSelectedTabBackground()
{
	if (this->tcObj.selTabbkgnd != nullptr)
		DeleteObject(this->tcObj.selTabbkgnd);

	switch (this->styleInfo.StyleID)
	{
	case STYLEID_BLACK:
		this->tcObj.selTabbkgnd = CreateSolidBrush(RGB(60, 60, 60));
		break;
	case STYLEID_DARKGRAY:
		this->tcObj.selTabbkgnd = CreateSolidBrush(RGB(127, 127, 127));
		break;
	case STYLEID_LIGHTGRAY:
		this->tcObj.selTabbkgnd = CreateSolidBrush(RGB(195, 195, 195));
		break;
	case STYLEID_GREEN:
		this->tcObj.selTabbkgnd = CreateSolidBrush(RGB(141, 186, 206));
		break;
	default:
		this->tcObj.selTabbkgnd = CreateSolidBrush(RGB(195, 195, 195));
		break;
	}
}

LPCTSTR CnCS_TC::getDefaultInsertText()
{
	auto userStringContainer =
		reinterpret_cast<ApplicationData*>(
			getApplicationDataContainerFromFilekey(FILEKEY_USER_STRINGS)
			);

	if (userStringContainer != nullptr)
	{
		this->defaultInsertText =
			userStringContainer->getStringData(
				DATAKEY_USERSTRINGS_DEFAULTTABINSERTTEXT,
				STARTUP_TEXT_PLACEHOLDER
			);

		return this->defaultInsertText.GetData();
	}
	return STARTUP_TEXT_PLACEHOLDER;
}

bool CnCS_TC::displayFileInfos(HDC hdc, int fromPos)
{
	auto ptp =
		this->GetActiveTabProperty();

	if (ptp != nullptr)
	{
		if (ptp->Path != nullptr)
		{
			SYSTEMTIME lastAccess, lastWrite, creationTime, local;

			auto bfpo = CreateBasicFPO();
			if (bfpo != nullptr)
			{
				if (bfpo->GetFileTimes(ptp->Path, &creationTime, &lastAccess, &lastWrite))
				{
					auto font = CreateScaledFont(14, FW_MEDIUM, L"Consolas");
					if (font)
					{
						SelectObject(hdc, font);

						DateTime dateTime;
						dateTime.SetLangID(
							(LANGID)getCurrentAppLanguage()
						);
						SystemTimeToTzSpecificLocalTime(nullptr, &creationTime, &local);
						dateTime.SetTime(&local);

						iString outString(
							getStringFromResource(UI_FILETIME_CREATED)
						);
						outString += L" ";
						outString += dateTime.SimpleDateAsString();
						outString += L"  ";
						outString += dateTime.SimpleTimeAsString();

						TextOut(
							hdc,
							DPIScale(30),
							fromPos,
							outString.GetData(),
							outString.GetLength()
						);

						fromPos += DPIScale(20);// next line

						dateTime.Clear();
						dateTime.SetLangID(
							(LANGID)getCurrentAppLanguage()
						);
						SystemTimeToTzSpecificLocalTime(nullptr, &lastAccess, &local);
						dateTime.SetTime(&local);

						outString.Replace(
							getStringFromResource(UI_FILETIME_LASTACCESS)
						);

						outString += L" ";
						outString += dateTime.SimpleDateAsString();
						outString += L"  ";
						outString += dateTime.SimpleTimeAsString();

						TextOut(
							hdc,
							DPIScale(30),
							fromPos,
							outString.GetData(),
							outString.GetLength()
						);

						fromPos += DPIScale(20);// next line

						dateTime.Clear();
						dateTime.SetLangID(
							(LANGID)getCurrentAppLanguage()
						);
						SystemTimeToTzSpecificLocalTime(nullptr, &lastWrite, &local);
						dateTime.SetTime(&local);

						outString.Replace(
							getStringFromResource(UI_FILETIME_LASTWRITE)
						);
						outString += L" ";
						outString += dateTime.SimpleDateAsString();
						outString += L"  ";
						outString += dateTime.SimpleTimeAsString();

						TextOut(
							hdc,
							DPIScale(30),
							fromPos,
							outString.GetData(),
							outString.GetLength()
						);

						fromPos += DPIScale(20);// next line

						// display filesize
						auto li = getFileSizeX(ptp->Path);

						outString.Replace(
							getStringFromResource(UI_FILESIZE)
						);
						outString += L" ";
						outString += li.LowPart;
						outString += L" Bytes";

						TextOut(
							hdc,
							DPIScale(30),
							fromPos,
							outString.GetData(),
							outString.GetLength()
						);

						fromPos += DPIScale(20);// next line

						// display output-size
						auto textLen = ptp->Editcontrol->GetTextLength();

						outString.Replace(
							getStringFromResource(UI_OUTPUTSIZE)
						);
						outString += L" ";
						outString += textLen;
						outString += L" Bytes";

						TextOut(
							hdc,
							DPIScale(30),
							fromPos,
							outString.GetData(),
							outString.GetLength()
						);

						DeleteObject(font);

						return true;
					}
				}
				SafeRelease(&bfpo);
			}
		}
		else
		{
			this->eraseFileInfoArea(hdc);
		}
	}
	return false;
}

void CnCS_TC::updateFileInfoArea()
{
	if (this->iParam.descWnd_exists)
	{
		auto descWnd =
			GetDlgItem(this->TCFrame, ID_DESCFRAME);

		if (descWnd)
		{
			RECT rc;
			GetClientRect(descWnd, &rc);

			rc.top = DPIScale(FILEINFOAREA_STARTHEIGHT);

			InvalidateRect(descWnd, &rc, FALSE);

			RedrawWindow(descWnd, nullptr, nullptr, RDW_NOCHILDREN | RDW_NOERASE | RDW_UPDATENOW);
		}
	}
}

void CnCS_TC::eraseFileInfoArea(HDC hdc)
{
	bool wasAllocated = false;

	auto descWnd =
		GetDlgItem(this->TCFrame, ID_DESCFRAME);

	if (descWnd)
	{
		if (hdc == nullptr)
		{
			hdc =
				GetDC(descWnd);

			wasAllocated = true;
		}
		if (hdc)
		{
			RECT rc;
			GetClientRect(descWnd, &rc);
			rc.left = DPIScale(10);
			rc.top = DPIScale(FILEINFOAREA_STARTHEIGHT);

			FillRect(hdc, &rc, this->tcObj.tabbkgnd);

			if (wasAllocated)
				ReleaseDC(descWnd, hdc);
		}
	}
}

void CnCS_TC::_createDpiDependendResources()
{
	if (this->tcObj.cnc3 != nullptr)
		DestroyIcon(this->tcObj.cnc3);

	this->tcObj.cnc3 =
		(HICON)LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(IDI_CNC3_FILEICON_SQ32),
			IMAGE_ICON,
			DPIScale(16),
			DPIScale(16),
			LR_DEFAULTCOLOR
		);

	if (this->tcObj.cnc3_marked != nullptr)
		DeleteObject(this->tcObj.cnc3_marked);

	this->tcObj.cnc3_marked =
		(HICON)LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(IDI_CNC3_FILEICON_SQ32_MARK),
			IMAGE_ICON,
			DPIScale(16),
			DPIScale(16),
			LR_DEFAULTCOLOR
		);

	if (this->tcObj.cnc3_disabled != nullptr)
		DeleteObject(this->tcObj.cnc3_disabled);

	this->tcObj.cnc3_disabled =
		(HICON)LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(IDI_CNC3_FILEICON_SQ32_DSBL),
			IMAGE_ICON,
			DPIScale(16),
			DPIScale(16),
			LR_DEFAULTCOLOR
		);

	if (this->tcObj.Tabfont != nullptr)
		DeleteObject(this->tcObj.Tabfont);

	this->tcObj.Tabfont = CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT);
		//CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Segoe UI\0");

	if (this->tcObj.desc_propertyfont != nullptr)
		DeleteObject(this->tcObj.desc_propertyfont);

	this->tcObj.desc_propertyfont = CreateScaledFont(20, FW_BOLD, APPLICATION_PRIMARY_FONT);
		//CreateFont(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Segoe UI\0");

	if (this->tcObj.tabclose_red != nullptr)
		DestroyIcon(this->tcObj.tabclose_red);

	this->tcObj.tabclose_red =
		(HICON)LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(IDI_TC_CLOSERED),
			IMAGE_ICON,
			DPIScale(16),
			DPIScale(16),
			LR_DEFAULTCOLOR
		);

	if (this->tcObj.tabclose_green != nullptr)
		DestroyIcon(this->tcObj.tabclose_green);

	this->tcObj.tabclose_green =
		(HICON)LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(IDI_TC_CLOSEGREEN),
			IMAGE_ICON,
			DPIScale(16),
			DPIScale(16),
			LR_DEFAULTCOLOR
		);

	if (this->tcObj.tabclose_marked != nullptr)
		DestroyIcon(this->tcObj.tabclose_marked);

	this->tcObj.tabclose_marked =
		(HICON)LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(IDI_TC_CLOSEMARKED),
			IMAGE_ICON,
			DPIScale(16),
			DPIScale(16),
			LR_DEFAULTCOLOR
		);
}

void CnCS_TC::DpiChanged()
{
	this->_createDpiDependendResources();

	int x = DPIScale(2);
	int y = 32;// startpoint of first button

	for (int i = IDM_CTLPROPWND; i <= IDM_FONTPROPERTIES; i++)
	{
		auto button = GetDlgItem(this->TCFrame, i);
		if (button)
		{
			MoveWindow(button, x, DPIScale(y), DPIScale(24), DPIScale(24), TRUE);
		}

		// jump to create a gap
		switch (i)
		{
		case IDM_CTLPROPWND:
			y = 69;
			break;
		case IDM_VT_REDO:
			y = 133;
			break;
		case IDM_FOCUSRECT:
			y = 278;
			break;
		default:
			y += 27;
			break;
		}
	}

	for (DWORD i = 0; i < this->TABCount; i++)
	{
		auto ptp = this->GetTabProperty(i);
		if (ptp != nullptr)
		{
			ptp->Editcontrol->SetCharHeight(
				DPIScale(this->iParam.FontHeight)
			);
		}
	}

	auto descFrame = GetDlgItem(this->TCFrame, ID_DESCFRAME);
	if (descFrame != nullptr)
	{
		DestroyWindow(descFrame);
		this->iParam.descWnd_exists = FALSE;
		this->iParam.descWnd_isMoving = FALSE;
		this->iParam.descWnd_visible = TRUE;

		this->CreateDescFrame(DESCFRAME_CREATESTATIC);
	}
}

void CnCS_TC::ResizeEditWindow(HWND editWnd)
{
	RECT rc;
	GetClientRect(this->TCFrame, &rc);

	int Ewidth = rc.right - DPIScale(40);
	if (this->iParam.descWnd_visible)	// hold free section for desc window
		Ewidth = 2 * (Ewidth / 3);

	SetWindowPos(
		editWnd,
		NULL,
		DPIScale(40),
		DPIScale(40),
		Ewidth,
		rc.bottom - DPIScale(40),
		SWP_NOZORDER// | SWP_SHOWWINDOW
	);
}

void CnCS_TC::SetTabContentFocusAndPushCaret(LPTABPROPERTY ptp, LPCTSTR defaultText, BOOL setFocus, CaretIndex cIndex)
{
	if (ptp != nullptr)
	{
		if (defaultText != nullptr)
		{
			ptp->Editcontrol->SetTextContent(defaultText, TRUE, TRUE, FALSE);
		}
		ptp->Editcontrol->SetCaret(cIndex);

		if (setFocus)
		{
			SetFocus(ptp->AssocEditWnd);
		}
	}
}

void CnCS_TC::closeTabIfPathIsEqual(LPCTSTR path)
{
	for (DWORD i = 0; i < this->TABCount; i++)
	{
		auto ptp = GetTabProperty(i);
		if (ptp != nullptr)
		{
			iString _path_(path);
			if (_path_.Equals(ptp->Path))
			{
				auto tab = GetDlgItem(this->TCFrame, TAB_ID_START + i);
				if (tab != nullptr)
				{
					this->CLOSE_Tab(tab, FALSE);
				}
			}
		}
	}
}

void CnCS_TC::clearToolWindowPopupControlParameterAndDestroy()
{
	ReleaseCapture();

	DestroyWindow(this->iParam.currentOpenedPopUp);
	this->iParam.excludeRectIsValid = FALSE;
	this->iParam.PopupIsOpen = FALSE;
	this->iParam.popupType = 0;
	this->iParam.currentOpenedPopUp = nullptr;
}

HRESULT CnCS_TC::RegisterTCClasses()
{
	HRESULT hr;

	WNDCLASSEX wcx;
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = sizeof(LPTABPROPERTY);
	wcx.lpfnWndProc = CnCS_TC::TabProc;
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.hbrBackground = NULL;
	wcx.hIcon = NULL;
	wcx.hInstance = this->hInstance;
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = L"CSTABCLASS";
	wcx.hIconSm = NULL;

	hr = (RegisterClassEx(&wcx) == 0) ? E_FAIL : S_OK;
	if (SUCCEEDED(hr))
	{
		WNDCLASSEX wcxx;
		wcxx.cbSize = sizeof(WNDCLASSEX);
		wcxx.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wcxx.cbClsExtra = 0;
		wcxx.cbWndExtra = sizeof(LONG_PTR);
		wcxx.lpfnWndProc = CnCS_TC::DescWndProc;
		wcxx.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcxx.hbrBackground = NULL;
		wcxx.hIcon = NULL;
		wcxx.hInstance = this->hInstance;
		wcxx.lpszMenuName = NULL;
		wcxx.lpszClassName = L"DESCCLASSX";
		wcxx.hIconSm = NULL;

		hr = (RegisterClassEx(&wcxx) == 0) ? E_FAIL : S_OK;
		if (SUCCEEDED(hr))
		{
			WNDCLASSEX wcxxx;
			wcxxx.cbSize = sizeof(WNDCLASSEX);
			wcxxx.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
			wcxxx.cbClsExtra = 0;
			wcxxx.cbWndExtra = sizeof(LONG_PTR);
			wcxxx.lpfnWndProc = CnCS_TC::ToolPopupProc;
			wcxxx.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcxxx.hbrBackground = NULL;
			wcxxx.hIcon = NULL;
			wcxxx.hInstance = this->hInstance;
			wcxxx.lpszMenuName = NULL;
			wcxxx.lpszClassName = TOOLWINDOWPOPUPCLASS;
			wcxxx.hIconSm = NULL;

			hr = (RegisterClassEx(&wcxxx) == 0) ? E_FAIL : S_OK;
			if (SUCCEEDED(hr))
			{
				// ...

			}
		}
	}
	return hr;
}

HRESULT CnCS_TC::SaveStartupInfo(LPTCSTARTUPINFO tcInfo)
{
	HRESULT hr = (tcInfo != NULL) ? S_OK : E_POINTER;
	if (SUCCEEDED(hr))
	{
		hr = (CopyStringToPtrA(tcInfo->WorkingDirectory, &this->iParam.WorkingDirectory) == TRUE) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			this->iParam.UseXMLFileForUserColors = tcInfo->useXMLColorScheme;



		}
	}
	return hr;
}

HRESULT CnCS_TC::CreateTBButton(DWORD IDbutton, int x, int y, PTSTR tooltip)
{
	HRESULT hr;

	HWND button = CreateWindow(
		L"BUTTON",
		NULL,
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		DPIScale(x),
		DPIScale(y),
		DPIScale(24),
		DPIScale(24),
		this->TCFrame,
		IDTOHMENU(IDbutton),//reinterpret_cast<HMENU>(IDbutton),
		this->hInstance,
		NULL
	);

	hr = (button != NULL) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		hr = SetWindowSubclass(button, CnCS_TC::TBBtnSub, NULL, reinterpret_cast<DWORD_PTR>(this)) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			// ...

			if (tooltip != NULL)
			{
				HWND tip = NULL;

				AddToolipToControl(button, this->TCFrame, tooltip, this->hInstance, &tip);

				if (IDbutton == IDM_CTLPROPWND)
					this->iParam.propTooltip = tip;
			}
		}
	}
	return hr;
}

HRESULT CnCS_TC::CreateDescFrame(DWORD Mode)
{
	if (!this->iParam.descWnd_visible)
		return S_OK;
	else
	{
		// Modes:  DESCFRAME_CREATEDYNAMIC / DESCFRAME_CREATESTATIC

		HRESULT hr;
		RECT rc_tab;
		GetClientRect(this->TCFrame, &rc_tab);

		int x, y, cx, cy;

		if (Mode == DESCFRAME_CREATEDYNAMIC)
		{
			x = rc_tab.right;
		}
		else
		{
			x = 2 * ((rc_tab.right - DPIScale(40)) / 3);
			x += DPIScale(40);
		}
		y = DPIScale(27);
		cx = (rc_tab.right - DPIScale(40)) / 3;
		cy = rc_tab.bottom - DPIScale(27);

		HWND descFrame = CreateWindowEx(
			WS_EX_COMPOSITED,
			L"DESCCLASSX",
			nullptr,
			WS_CHILD | WS_VISIBLE,
			x, y, cx, cy,
			this->TCFrame,
			(HMENU)ID_DESCFRAME,
			this->hInstance,
			reinterpret_cast<LPVOID>(this)
		);

		hr = (descFrame != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			this->iParam.descWnd_exists = TRUE;

			HWND propDesc1 = CreateWindow(
				MSFTEDIT_CLASS,
				NULL,
				WS_CHILD | WS_VISIBLE | ES_CENTER | ES_AUTOHSCROLL,
				DPIScale(2),
				DPIScale(100),
				cx-DPIScale(2),
				DPIScale(30),
				descFrame,
				(HMENU)ID_PROPERTYEDIT_ONE,
				this->hInstance,
				nullptr
			);

			hr = (propDesc1 != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				this->ConfigPropEdit(propDesc1);

				HWND propDesc2 = CreateWindow(
					MSFTEDIT_CLASS,
					NULL,
					WS_CHILD | WS_VISIBLE | ES_CENTER | ES_AUTOHSCROLL,
					DPIScale(2),
					DPIScale(170),
					cx - DPIScale(2),
					DPIScale(30),
					descFrame,
					(HMENU)ID_PROPERTYEDIT_TWO,
					this->hInstance,
					nullptr
				);

				hr = (propDesc2 != nullptr) ? S_OK : E_FAIL;
				if (SUCCEEDED(hr))
				{
					this->ConfigPropEdit(propDesc2);

					HWND propDesc3 = CreateWindow(
						MSFTEDIT_CLASS,
						NULL,
						WS_CHILD | WS_VISIBLE | ES_CENTER | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
						DPIScale(2),
						DPIScale(240),
						cx - DPIScale(2),
						DPIScale(100),
						descFrame,
						(HMENU)ID_PROPERTYEDIT_THREE,
						this->hInstance,
						nullptr
					);

					hr = (propDesc3 != nullptr) ? S_OK : E_FAIL;
					if (SUCCEEDED(hr))
					{
						this->ConfigPropEdit(propDesc3);

						POINT pt;
						pt.x = DPIScale(10);
						pt.y = DPIScale(36);

						SIZE sz;
						sz.cx = DPIScale(28);
						sz.cy = DPIScale(28);

						auto copyButton = new CustomButton(descFrame, BUTTONMODE_ICON, &pt, &sz, IDM_DESCWND_COPY, this->hInstance);
						if (copyButton != nullptr)
						{
							iString tooltip(
								getStringFromResource(UI_TABCTRL_TTBUTTONCOPYDESC)
							);

							copyButton->setAppearance_onlyIcon(IDI_EXP_COPYELEMENT, DPIScale(24));
							copyButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
							copyButton->setColors(
								this->styleInfo.mainToolbarColor,
								makeSelectionColor(this->styleInfo.mainToolbarColor),
								makePressedColor(this->styleInfo.mainToolbarColor)
							);
							copyButton->setBorder(TRUE, this->styleInfo.OutlineColor);
							copyButton->workWithCompositedWindows(true);
							copyButton->setTooltip(tooltip);

							hr = copyButton->Create();
							if (SUCCEEDED(hr))
							{
								pt.x += DPIScale(32);

								auto insertButton = new CustomButton(descFrame, BUTTONMODE_ICON, &pt, &sz, IDM_DESCWND_INSERT, this->hInstance);
								if (insertButton != nullptr)
								{
									tooltip.Replace(
										getStringFromResource(UI_TABCTRL_TTBUTTONINSERTDESC)
									);
									insertButton->setAppearance_onlyIcon(IDI_EXP_INSERTELEMENT, DPIScale(24));
									insertButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
									insertButton->setColors(
										this->styleInfo.mainToolbarColor,
										makeSelectionColor(this->styleInfo.mainToolbarColor),
										makePressedColor(this->styleInfo.mainToolbarColor)
									);
									insertButton->setBorder(TRUE, this->styleInfo.OutlineColor);
									insertButton->workWithCompositedWindows(true);
									insertButton->setTooltip(tooltip);

									hr = insertButton->Create();
									if (SUCCEEDED(hr))
									{

										pt.x += DPIScale(32);

										auto deleteButton = new CustomButton(descFrame, BUTTONMODE_ICON, &pt, &sz, IDM_DESCWND_DELETE, this->hInstance);
										if (deleteButton != nullptr)
										{
											tooltip.Replace(
												getStringFromResource(UI_TABCTRL_TTBUTTONDELETEDESC)
											);
											deleteButton->setAppearance_onlyIcon(IDI_EXP_DELETEELEMENT, DPIScale(24));
											deleteButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
											deleteButton->setColors(
												this->styleInfo.mainToolbarColor,
												makeSelectionColor(this->styleInfo.mainToolbarColor),
												makePressedColor(this->styleInfo.mainToolbarColor)
											);
											deleteButton->setBorder(TRUE, this->styleInfo.OutlineColor);
											deleteButton->workWithCompositedWindows(true);
											deleteButton->setTooltip(tooltip);

											hr = deleteButton->Create();
											if (SUCCEEDED(hr))
											{
												pt.x += DPIScale(32);

												auto diameterButton = new CustomButton(descFrame, BUTTONMODE_TEXT, &pt, &sz, IDM_DESCWND_DIAMETER, this->hInstance);
												if (diameterButton != nullptr)
												{
													tooltip.Replace(
														getStringFromResource(UI_TABCTRL_TTBUTTONSETDIAMETER)
													);

													iString dia(L"\u00D8");
													diameterButton->setAppearance_onlyText(dia, FALSE);
													diameterButton->setTextColor(RGB(255,255,255));
													diameterButton->setFont(
														//CreateFont(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Segoe UI Symbol\0")
														CreateScaledFont(32, FW_NORMAL, L"Segoe UI Symbol")
													);
													diameterButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
													diameterButton->setColors(
														this->styleInfo.mainToolbarColor,
														makeSelectionColor(this->styleInfo.mainToolbarColor),
														makePressedColor(this->styleInfo.mainToolbarColor)
													);
													diameterButton->setBorder(TRUE, this->styleInfo.OutlineColor);
													diameterButton->setTextColor(RGB(15, 83, 151));
													diameterButton->workWithCompositedWindows(true);
													diameterButton->setTooltip(tooltip);
													diameterButton->setContentOffset(0, -1);

													hr = diameterButton->Create();
													if (SUCCEEDED(hr))
													{
														// ...

														auto ptp = this->GetActiveTabProperty();
														if (ptp != nullptr)
														{
															this->SetDescriptions(ptp->DESC1, ptp->DESC2, ptp->DESC3);
														}

														if (Mode == DESCFRAME_CREATEDYNAMIC)
														{
															this->iParam.descWnd_visible = FALSE;

															this->ControlPropertyWindow();
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		return hr;
	}
}

HRESULT CnCS_TC::InitFrameProperty()
{
	this->LoadDESCPropertyNames();
	return S_OK;
}

HRESULT CnCS_TC::ControlPropertyButton(DWORD Mode)
{
	HRESULT hr = S_OK;

	if (Mode == PROPBUTTON_CREATE)
	{
		if (this->iParam.descWnd_visible)
			hr = this->CreateTBButton(IDM_CTLPROPWND, 2, 32, getStringFromResource(UI_EDITTOOLBAR_HIDEDOCPROPERTY));
		else
			hr = this->CreateTBButton(IDM_CTLPROPWND, 2, 32, getStringFromResource(UI_EDITTOOLBAR_SHOWDOCPROPERTY));
	}
	else
	{
		HWND button = GetDlgItem(this->TCFrame, IDM_CTLPROPWND);
		hr = (button != NULL) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			hr =
				RedrawWindow(
					button,
					NULL, NULL, RDW_INVALIDATE | RDW_ERASE)
				? S_OK : E_FAIL;

			if (this->iParam.descWnd_visible)
				hr = ChangeTooltipText(this->iParam.propTooltip, button, this->TCFrame, getStringFromResource(UI_EDITTOOLBAR_HIDEDOCPROPERTY), this->hInstance) ? S_OK : E_FAIL;
			else
				hr = ChangeTooltipText(this->iParam.propTooltip, button, this->TCFrame, getStringFromResource(UI_EDITTOOLBAR_SHOWDOCPROPERTY), this->hInstance) ? S_OK : E_FAIL;
		}
	}
	return hr;
}

HRESULT CnCS_TC::CreateFontPropertyBox()
{
	if (this->iParam.PopupIsOpen)
	{
		return E_FAIL;
	}
	else
	{
		HRESULT hr;

		hr = this->CreateToolHostWindowFromButtonPosition(
			IDM_FONTPROPERTIES,
			DPIScale(200),
			DPIScale(300)
		);
		if (SUCCEEDED(hr))
		{
			this->iParam.popupType = POPUPTYPE_FONTPROPERTY;
			this->iParam.PopupIsOpen = TRUE;

			auto FontFamilyCombo =
				new comboBox(
					this->hInstance,
					this->iParam.currentOpenedPopUp,
					COMBOTYPE_DROPDOWNLIST,
					IDM_FONTFAMILYCOMBO,
					DPIScale(30),
					DPIScale(5),
					DPIScale(140),
					DPIScale(22)
				);

			hr = FontFamilyCombo->Succeded() ? S_OK : E_FAIL;
			if(SUCCEEDED(hr))
			{
				for (int i = 0; i < this->availableFonts.GetCount(); i++)
				{
					auto font = this->availableFonts.GetAt(i);
					FontFamilyCombo->Items->AddItem(font);
				}
				FontFamilyCombo->setSelectedIndex(this->iParam.FontFamilyIndex);
				FontFamilyCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));

				auto fontheightTrackbar = new customTrackbar();

				hr = (fontheightTrackbar != nullptr) ? S_OK : E_FAIL;
				if(SUCCEEDED(hr))
				{
					iString text(
						getStringFromResource(UI_TABCTRL_FONTHEIGHT)
					);

					POINT pt;
					SIZE sz;

					pt.x = 0;
					pt.y = DPIScale(40);

					sz.cx = DPIScale(100);
					sz.cy = DPIScale(200);

					fontheightTrackbar->setText(text);
					fontheightTrackbar->setFont(
						CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
					);
					fontheightTrackbar->setColors(this->styleInfo.Stylecolor, this->styleInfo.OutlineColor, this->styleInfo.TextColor);
					fontheightTrackbar->setTrackRange(0, 300);
					fontheightTrackbar->setTrackbuttonImages(IDI_TRACKBARBUTTON_NORMAL, IDI_TRACKBARBUTTON_PRESSED, 24);
					fontheightTrackbar->setType(customTrackbar::TRACKBARTYPE_VERTICAL);
					fontheightTrackbar->setBorder(true);
					fontheightTrackbar->setBarAppearence(2, false, false, 0);
					fontheightTrackbar->setEventHandler(dynamic_cast<trackBarEventSink*>(this));
					fontheightTrackbar->setTrackPosition(this->iParam.FontHeight - 200);

					hr = fontheightTrackbar->create(this->hInstance, this->iParam.currentOpenedPopUp, &pt, &sz, IDM_FONTHEIGHTTRACK);
					if (SUCCEEDED(hr))
					{
						pt.x = DPIScale(100);

						text.Replace(
							getStringFromResource(UI_TABCTRL_LINEOFFSET)
						);

						auto lineOffsetTrackbar = new customTrackbar();

						hr = (lineOffsetTrackbar != nullptr) ? S_OK : E_FAIL;
						if(SUCCEEDED(hr))
						{
							lineOffsetTrackbar->setText(text);
							lineOffsetTrackbar->setFont(
								CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
							);
							lineOffsetTrackbar->setColors(this->styleInfo.Stylecolor, this->styleInfo.OutlineColor, this->styleInfo.TextColor);
							lineOffsetTrackbar->setTrackRange(0, 200);
							lineOffsetTrackbar->setTrackbuttonImages(IDI_TRACKBARBUTTON_NORMAL, IDI_TRACKBARBUTTON_PRESSED, 24);
							lineOffsetTrackbar->setType(customTrackbar::TRACKBARTYPE_VERTICAL);
							lineOffsetTrackbar->setBorder(true);
							lineOffsetTrackbar->setBarAppearence(2, false, false, 0);
							lineOffsetTrackbar->setEventHandler(dynamic_cast<trackBarEventSink*>(this));
							lineOffsetTrackbar->setTrackPosition(this->iParam.lineOffset);

							hr = lineOffsetTrackbar->create(this->hInstance, this->iParam.currentOpenedPopUp, &pt, &sz, IDM_LINESPACETRACK);
							if (SUCCEEDED(hr))
							{
								pt.x = 0;
								pt.y = DPIScale(250);
								sz.cx = DPIScale(198);
								sz.cy = DPIScale(30);

								auto boldCBX = new CustomCheckbox(this->hInstance, this->iParam.currentOpenedPopUp, &pt, &sz, IDM_FONTBOLDCHECKBX);

								hr = (boldCBX != nullptr) ? S_OK : E_FAIL;
								if (SUCCEEDED(hr))
								{
									text.Replace(
										getStringFromResource(UI_TABCTRL_FONTBOLD)
									);

									boldCBX->setColors(this->styleInfo.Stylecolor, this->styleInfo.TextColor);
									boldCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
									boldCBX->setChecked(this->iParam.isBold ? true : false);
									boldCBX->setText(text);
									boldCBX->setFont(
										CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
									);
									boldCBX->setConstraints(10, 10);
									boldCBX->setAlignment(CBX_ALIGN_CENTER);

									hr = boldCBX->Create();
									if (SUCCEEDED(hr))
									{
										// ...
									}
								}
							}
						}
					}
				}
			}
		}
		return hr;
	}
}

HRESULT CnCS_TC::CreateToolHostWindowFromButtonPosition(int Id_Button, int cx, int cy)
{
	RECT buttonRect, mainWndRect;

	GetWindowRect(this->Main, &mainWndRect);
	GetWindowRect(
		GetDlgItem(this->TCFrame, Id_Button),
		&buttonRect
	);

	CopyRect(&this->iParam.excludeRect, &buttonRect);
	this->iParam.excludeRectIsValid = TRUE;

	int yPos;

	if ((mainWndRect.bottom - buttonRect.top) < cy)
	{
		int corr = cy - (mainWndRect.bottom - buttonRect.top);
		yPos = buttonRect.top - corr;
	}
	else
	{
		yPos = buttonRect.bottom - (buttonRect.bottom - buttonRect.top);
	}

	this->iParam.currentOpenedPopUp =

		CreateWindow(
			TOOLWINDOWPOPUPCLASS,
			NULL,
			WS_POPUP | WS_VISIBLE | WS_BORDER,
			buttonRect.right + 20,
			yPos,
			cx, cy,
			this->TCFrame,
			NULL,
			this->hInstance,
			reinterpret_cast<LPVOID>(this)
		);

	return (this->iParam.currentOpenedPopUp) ? S_OK : E_FAIL;
}

HRESULT CnCS_TC::OpenTabMenu()
{
	// save as..	??
	// export as..	??

	auto menu = new CustomPopUpMenu(this->hInstance);

	auto hr = (menu != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		auto ptp = this->GetActiveTabProperty();

		hr = (ptp != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			menu->setControlFont(
				CreateScaledFont(16, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
			);
			menu->setEventListener(
				dynamic_cast<customPopUpMenuEventSink*>(this)
			);

			MenuEntry entry;

			if (ptp->Path != nullptr)
			{
				this->_defineMenuEntry(entry, IDM_PMENU_EXPANDTOFILE, IDI_TC_TABMENU_EXPAND, DPIScale(24), getStringFromResource(UI_TABCTRL_EXPANDPATHTOFILE));
				menu->addMenuEntry(&entry);
				this->_defineMenuEntry(entry, IDM_PMENU_OPENINWINEXP, IDI_TC_TABMENU_OPENINWIN, DPIScale(24), getStringFromResource(UI_GNRL_SHOWINWINEXPLORER));
				menu->addMenuEntry(&entry);
			}

			this->_defineMenuEntry(entry, IDM_PMENU_SETTEMPLATE, IDI_TC_TABMENU_ADDASTEMPL, DPIScale(24), getStringFromResource(UI_TABCTRL_SETCONTENTASTEMPLATE));
			menu->addMenuEntry(&entry);


			this->_defineMenuEntry(entry, IDM_PMENU_COPYTONEWTAB, IDI_TC_TABMENU_COPYTOTAB, DPIScale(24), getStringFromResource(UI_TABCTRL_COPYCONTENTTONEWTAB));
			menu->addMenuEntry(&entry);
			this->_defineMenuEntry(entry, IDM_PMENU_ERASETAB, IDI_TC_TABMENU_RESET, DPIScale(24), getStringFromResource(UI_TABCTRL_ERASETABCONTENT));
			menu->addMenuEntry(&entry);
		
			POINT ul, lr;
			ul.x = DPIScale(10);
			ul.y = DPIScale(4);
			lr.x = DPIScale(26);
			lr.y = DPIScale(20);

			ClientToScreen(ptp->Tab, &ul);
			ClientToScreen(ptp->Tab, &lr);

			RECT excludeRect;
			SetRect(&excludeRect, ul.x, ul.y, lr.x, lr.y);

			menu->setCloseExclusionArea(&excludeRect);

			RECT rc;
			GetWindowRect(ptp->Tab, &rc);
			POINT Pos = { rc.left, rc.bottom };

			hr = menu->Show(this->TCFrame, &Pos, COMMONTABWIDTH, this->styleInfo.Background);
		}
	}
	return hr;
}

void CnCS_TC::_defineMenuEntry(MenuEntry &entry, int eID, int iconID, int iconSQsize, LPCTSTR text)
{
	entry.Clear();
	entry.setID(eID);
	entry.setIcon(iconID, iconSQsize);
	entry.setText(text);
	entry.setColors(
		this->styleInfo.MenuPopUpColor,
		this->styleInfo.TextColor,
		RGB(0, 150, 200),
		RGB(0, 130, 180)
	);
}

