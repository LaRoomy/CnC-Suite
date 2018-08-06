#include"ResetDialog.h"
#include"HelperF.h"
#include"AppPath.h"

ResetDialog::ResetDialog()
	: resetDlgWnd(nullptr),
	hInstance(nullptr),
	displayRestartMessage(false)
{
	SecureZeroMemory(&this->cStates, sizeof(CHECKSTATES));
	this->font = CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT);
}

ResetDialog::~ResetDialog()
{
	DeleteObject(this->font);
}

const TCHAR* ResetDialog::RESET_DLG_CLASS_ID = L"RESETDLGCLASSNAME13453";

HRESULT ResetDialog::Create(HINSTANCE hInst, HWND hWndMain)
{
	auto hr =
		((hInst != nullptr) && (hWndMain != nullptr))
		? S_OK : E_INVALIDARG;

	if (SUCCEEDED(hr))
	{
		this->MainWindow = hWndMain;
		this->hInstance = hInst;

		hr = this->registerResetWnd();
		if (SUCCEEDED(hr))
		{
			auto cx = GetSystemMetrics(SM_CXSCREEN);
			auto cy = GetSystemMetrics(SM_CYSCREEN);


			this->resetDlgWnd =
				CreateWindow(
					ResetDialog::RESET_DLG_CLASS_ID,
					nullptr,
					WS_POPUP | WS_BORDER,
					cx / 2 - DPIScale(150),
					cy / 2 - DPIScale(200),
					DPIScale(300),
					DPIScale(400),
					hWndMain,
					nullptr,
					hInst,
					reinterpret_cast<LPVOID>(this)
				);

			hr = (this->resetDlgWnd != nullptr)
				? S_OK : E_HANDLE;

			if (SUCCEEDED(hr))
			{
				hr = this->createControls();
				if (SUCCEEDED(hr))
				{
					ShowWindow(this->resetDlgWnd, SW_SHOW);
					UpdateWindow(this->resetDlgWnd);

					EnableWindow(this->MainWindow, FALSE);
				}
			}
		}
	}
	return hr;
}

void ResetDialog::SetColors(COLORREF background, COLORREF text)
{
	this->backgroundColor = background;
	this->textColor = text;
}

void ResetDialog::CloseDialog()
{
	EnableWindow(this->MainWindow, TRUE);
	DestroyWindow(this->resetDlgWnd);
}

void ResetDialog::onCustomButtonClick(cObject sender, CTRLID ctrlID)
{
	switch (ctrlID)
	{
	case RDLG_CTRLID_EXITBUTTON:
		this->CloseDialog();
		break;
	case RDLG_CTRLID_CANCELBUTTON:
		this->CloseDialog();
		break;
	case RDLG_CTRLID_RESETBUTTON:
		this->executeReset();
		break;
	default:
		break;
	}
}

void ResetDialog::onCustomCheckboxChecked(cObject sender, bool newState)
{
	switch (
		reinterpret_cast<CustomCheckbox*>(sender)->getCtrlID()
		)
	{
	case RDLG_CTRLID_COMMONSETTINGS_CBX:
		this->cStates.commonSettings = newState;
		break;
	case RDLG_CTRLID_COMPARAMETER_CBX:
		this->cStates.interfaceParameter = newState;
		break;
	case RDLG_CTRLID_AUTOSYNTAXSETUP_CBX:
		this->cStates.autosyntaxSetup = newState;
		break;
	case RDLG_CTRLID_USERTEXT_CBX:
		this->cStates.usertext = newState;
		break;
	case RDLG_CTRLID_INTERNALSETUP_CBX:
		this->cStates.internalSetup = newState;
		break;
	case RDLG_CTRLID_AUTOCOMPLETESTRINGS_CBX:
		this->cStates.autocompleteSetup = newState;
		break;
	case RDLG_CTRLID_APPDATA_CBX:
		this->cStates.appdataSetup = newState;
		break;
	case RDLG_CTRLID_SESSIONDATA_CBX:
		this->cStates.sessionData = newState;
		break;
	default:
		break;
	}

	this->displayRestartMessage = (this->cStates.appdataSetup || this->cStates.sessionData) ? true : false;
	RedrawWindow(this->resetDlgWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_NOERASE);
}

HRESULT ResetDialog::registerResetWnd()
{
	HRESULT hr = S_OK;
	WNDCLASSEX wcx;

	if (GetClassInfoEx(this->hInstance, ResetDialog::RESET_DLG_CLASS_ID, &wcx) == 0)
	{
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wcx.lpfnWndProc = ResetDialog::resetProc;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = sizeof(LONG_PTR);
		wcx.hInstance = this->hInstance;
		wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcx.hIcon = nullptr;
		wcx.hIconSm = nullptr;
		wcx.lpszClassName = ResetDialog::RESET_DLG_CLASS_ID;
		wcx.lpszMenuName = nullptr;
		wcx.hbrBackground = nullptr;

		hr = (RegisterClassEx(&wcx) == 0)
			? E_FAIL : S_OK;
	}
	return hr;
}

HRESULT ResetDialog::createControls()
{
	POINT pos;
	SIZE sz;
	RECT rc;

	GetClientRect(this->resetDlgWnd, &rc);

	pos.x = rc.right - DPIScale(26);
	pos.y = DPIScale(2);

	sz.cx = DPIScale(24);
	sz.cy = sz.cx;

	auto exitButton = new CustomButton(this->resetDlgWnd, BUTTONMODE_ICON, &pos, &sz, RDLG_CTRLID_EXITBUTTON, this->hInstance);
	auto hr =
		(exitButton != nullptr)
		? S_OK : E_POINTER;
	if (SUCCEEDED(hr))
	{
		exitButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
		exitButton->setColors(this->backgroundColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
		exitButton->setAppearance_onlyIcon(IDI_GNRL_CANCEL_YELLOW, 24);

		hr = exitButton->Create();
		if (SUCCEEDED(hr))
		{
			pos.x = DPIScale(20);
			pos.y = DPIScale(50);

			sz.cx = rc.right - 40;
			sz.cy = 30;

			iString cbText(
				getStringFromResource(UI_RESETWND_COMMONSETTINGS)
			);

			// add tooltip to checkbox class!!!

			auto commonSettings = new CustomCheckbox(this->hInstance, this->resetDlgWnd, &pos, &sz, RDLG_CTRLID_COMMONSETTINGS_CBX);

			hr = (commonSettings != nullptr) ? S_OK : E_POINTER;
			if (SUCCEEDED(hr))
			{
				commonSettings->setColors(this->backgroundColor, this->textColor);
				commonSettings->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
				commonSettings->setText(cbText);
				commonSettings->setConstraints(10, 10);
				commonSettings->setFont(
					CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
				);

				hr = commonSettings->Create();
				if (SUCCEEDED(hr))
				{
					cbText.Replace(
						getStringFromResource(UI_RESETWND_COMPARAMETER)
					);
					pos.y += DPIScale(35);

					auto interfaceSetupCBX = new CustomCheckbox(this->hInstance, this->resetDlgWnd, &pos, &sz, RDLG_CTRLID_COMPARAMETER_CBX);

					hr = (interfaceSetupCBX != nullptr) ? S_OK : E_POINTER;
					if (SUCCEEDED(hr))
					{
						interfaceSetupCBX->setColors(this->backgroundColor, this->textColor);
						interfaceSetupCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
						interfaceSetupCBX->setText(cbText);
						interfaceSetupCBX->setConstraints(10, 10);
						interfaceSetupCBX->setFont(
							CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
						);

						hr = interfaceSetupCBX->Create();
						if (SUCCEEDED(hr))
						{
							cbText.Replace(
								getStringFromResource(UI_RESETWND_AUTOSYNTAXSETUP)
							);
							pos.y += DPIScale(35);

							auto autosyntaxCBX = new CustomCheckbox(this->hInstance, this->resetDlgWnd, &pos, &sz, RDLG_CTRLID_AUTOSYNTAXSETUP_CBX);

							hr = (autosyntaxCBX != nullptr) ? S_OK : E_POINTER;
							if (SUCCEEDED(hr))
							{
								autosyntaxCBX->setColors(this->backgroundColor, this->textColor);
								autosyntaxCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
								autosyntaxCBX->setText(cbText);
								autosyntaxCBX->setConstraints(10, 10);
								autosyntaxCBX->setFont(
									CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
								);

								hr = autosyntaxCBX->Create();
								if (SUCCEEDED(hr))
								{
									cbText.Replace(
										getStringFromResource(UI_RESETWND_USERTEXT)
									);
									pos.y += DPIScale(35);

									auto usertextCBX = new CustomCheckbox(this->hInstance, this->resetDlgWnd, &pos, &sz, RDLG_CTRLID_USERTEXT_CBX);

									hr = (usertextCBX != nullptr) ? S_OK : E_POINTER;
									if (SUCCEEDED(hr))
									{
										usertextCBX->setColors(this->backgroundColor, this->textColor);
										usertextCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
										usertextCBX->setText(cbText);
										usertextCBX->setConstraints(10, 10);
										usertextCBX->setFont(
											CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
										);

										hr = usertextCBX->Create();
										if (SUCCEEDED(hr))
										{
											cbText.Replace(
												getStringFromResource(UI_RESETWND_INTERNALSETUP)
											);
											pos.y += DPIScale(35);

											auto internalSetupCBX = new CustomCheckbox(this->hInstance, this->resetDlgWnd, &pos, &sz, RDLG_CTRLID_INTERNALSETUP_CBX);

											hr = (internalSetupCBX != nullptr) ? S_OK : E_POINTER;
											if (SUCCEEDED(hr))
											{
												internalSetupCBX->setColors(this->backgroundColor, this->textColor);
												internalSetupCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
												internalSetupCBX->setText(cbText);
												internalSetupCBX->setConstraints(10, 10);
												internalSetupCBX->setFont(
													CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
												);

												hr = internalSetupCBX->Create();
												if (SUCCEEDED(hr))
												{
													cbText.Replace(
														getStringFromResource(UI_RESETWND_AUTOCOMPLETESETUP)
													);
													pos.y += DPIScale(35);

													auto autocompleteCBX = new CustomCheckbox(this->hInstance, this->resetDlgWnd, &pos, &sz, RDLG_CTRLID_AUTOCOMPLETESTRINGS_CBX);

													hr = (autocompleteCBX != nullptr) ? S_OK : E_POINTER;
													if (SUCCEEDED(hr))
													{
														autocompleteCBX->setColors(this->backgroundColor, this->textColor);
														autocompleteCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
														autocompleteCBX->setText(cbText);
														autocompleteCBX->setConstraints(10, 10);
														autocompleteCBX->setFont(
															CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
														);

														hr = autocompleteCBX->Create();
														if (SUCCEEDED(hr))
														{
															cbText.Replace(
																getStringFromResource(UI_RESETWND_APPDATA)
															);
															pos.y += DPIScale(35);

															auto appDataCBX = new CustomCheckbox(this->hInstance, this->resetDlgWnd, &pos, &sz, RDLG_CTRLID_APPDATA_CBX);

															hr = (appDataCBX != nullptr) ? S_OK : E_POINTER;
															if (SUCCEEDED(hr))
															{
																appDataCBX->setColors(this->backgroundColor, this->textColor);
																appDataCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
																appDataCBX->setText(cbText);
																appDataCBX->setConstraints(10, 10);
																appDataCBX->setFont(
																	CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																);

																hr = appDataCBX->Create();
																if (SUCCEEDED(hr))
																{
																	cbText.Replace(
																		getStringFromResource(UI_RESETWND_SESSIONDATA)
																	);
																	pos.y += DPIScale(35);

																	auto sessionDataCbx = new CustomCheckbox(this->hInstance, this->resetDlgWnd, &pos, &sz, RDLG_CTRLID_SESSIONDATA_CBX);

																	hr = (sessionDataCbx != nullptr) ? S_OK : E_POINTER;
																	if (SUCCEEDED(hr))
																	{
																		sessionDataCbx->setColors(this->backgroundColor, this->textColor);
																		sessionDataCbx->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
																		sessionDataCbx->setText(cbText);
																		sessionDataCbx->setConstraints(10, 10);
																		sessionDataCbx->setFont(
																			CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																		);

																		hr = sessionDataCbx->Create();
																		if (SUCCEEDED(hr))
																		{
																			cbText.Replace(L"Reset");

																			pos.x = 0;
																			pos.y = rc.bottom - DPIScale(25);

																			sz.cx = rc.right / 2;
																			sz.cy = DPIScale(25);

																			auto resetButton = new CustomButton(this->resetDlgWnd, BUTTONMODE_TEXT, &pos, &sz, RDLG_CTRLID_RESETBUTTON, this->hInstance);
																			hr =
																				(resetButton != nullptr)
																				? S_OK : E_FAIL;
																			if (SUCCEEDED(hr))
																			{
																				resetButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
																				resetButton->setAppearance_onlyText(cbText, FALSE);
																				resetButton->setBorder(TRUE, this->textColor);
																				resetButton->setColors(this->backgroundColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
																				resetButton->setTextHighlight(TRUE, RGB(255, 255, 255));
																				resetButton->setFont(
																					CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																				);
																				resetButton->setTextColor(this->textColor);

																				hr = resetButton->Create();
																				if (SUCCEEDED(hr))
																				{
																					cbText.Replace(
																						getStringFromResource(UI_GNRL_CANCEL)
																					);

																					pos.x = rc.right / 2;

																					auto cancelButton = new CustomButton(this->resetDlgWnd, BUTTONMODE_TEXT, &pos, &sz, RDLG_CTRLID_CANCELBUTTON, this->hInstance);
																					hr =
																						(cancelButton != nullptr)
																						? S_OK : E_FAIL;
																					if (SUCCEEDED(hr))
																					{
																						cancelButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
																						cancelButton->setAppearance_onlyText(cbText, FALSE);
																						cancelButton->setBorder(TRUE, this->textColor);
																						cancelButton->setColors(this->backgroundColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
																						cancelButton->setTextHighlight(TRUE, RGB(255, 255, 255));
																						cancelButton->setFont(
																							CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																						);
																						cancelButton->setTextColor(this->textColor);

																						hr = cancelButton->Create();
																						if (SUCCEEDED(hr))
																						{

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

LRESULT ResetDialog::resetProc(HWND resetWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ResetDialog* pDlg = nullptr;

	if (message == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		if (pcr != nullptr)
		{
			pDlg = reinterpret_cast<ResetDialog*>(pcr->lpCreateParams);
			if (pDlg != nullptr)
			{
				SetWindowLongPtr(resetWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDlg));
			}
		}
		return 1;
	}
	else
	{
		pDlg = reinterpret_cast<ResetDialog*>(GetWindowLongPtr(resetWnd, GWLP_USERDATA));
		if (pDlg != nullptr)
		{
			switch (message)
			{
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_PAINT:
				return pDlg->onPaint(resetWnd);
			case WM_DESTROY:
				SafeRelease(&pDlg);
				return 0;
			default:
				break;
			}
		}
		return DefWindowProc(resetWnd, message, wParam, lParam);
	}
}

LRESULT ResetDialog::onPaint(HWND hDlg)
{
	HDC hdc;
	RECT rc;
	PAINTSTRUCT ps;

	GetClientRect(hDlg, &rc);

	hdc = BeginPaint(hDlg, &ps);
	if (hdc)
	{
		HBRUSH backgroundBrush = CreateSolidBrush(this->backgroundColor);
		if (backgroundBrush)
		{
			FillRect(hdc, &rc, backgroundBrush);

			TextOutDC(
				hdc,
				getStringFromResource(UI_RESETWND_CHOOSECOMPONENT),
				DPIScale(20),
				DPIScale(20),
				this->font,
				this->textColor
			);

			if (this->displayRestartMessage)
			{
				TextOutDC(
					hdc,
					getStringFromResource(UI_RESETWND_RESTARTMESSAGE),
					DPIScale(10),
					DPIScale(rc.bottom - 55),
					this->font,
					RGB(255,255,0)
				);
			}

			DeleteObject(backgroundBrush);
		}
		HPEN pen = CreatePen(PS_SOLID, 1, COLOR_BUTTON_MOUSE_OVER);
		if (pen)
		{
			auto origin = SelectObject(hdc, pen);

			MoveToEx(hdc, 0, 0, nullptr);
			LineTo(hdc, rc.right - 1, 0);
			LineTo(hdc, rc.right - 1, rc.bottom - 1);
			LineTo(hdc, 0, rc.bottom - 1);
			LineTo(hdc, 0, 0);

			SelectObject(hdc, origin);
			DeleteObject(pen);
		}

		EndPaint(hDlg, &ps);
	}
	return static_cast<LRESULT>(0);
}

void ResetDialog::executeReset()
{
	bool mustRestart = false;
	AppPath appPath;

	if (this->cStates.appdataSetup)// if this is selected the application must be relaunched
	{
		auto pathToFile = appPath.Get(PATHID_FILE_APPSTYLE);
		if (pathToFile.succeeded())
		{
			DeleteFile(
				pathToFile.GetData()
			);
		}
		pathToFile = appPath.Get(PATHID_FILE_NAVROOT);
		if (pathToFile.succeeded())
		{
			DeleteFile(
				pathToFile.GetData()
			);
		}
		pathToFile = appPath.Get(PATHID_FILE_WINDOW_DATA);
		if (pathToFile.succeeded())
		{
			DeleteFile(
				pathToFile.GetData()
			);
		}
		pathToFile = appPath.Get(PATHID_FILE_SEARCHSETUP);
		if (pathToFile.succeeded())
		{
			DeleteFile(
				pathToFile.GetData()
			);
		}
		mustRestart = true;
	}
	if (this->cStates.sessionData)// if this is selected the application must be relaunched
	{
		auto pathToFile = appPath.Get(PATHID_FILE_TABIMAGE);

		if (pathToFile.succeeded())
		{
			DeleteFile(
				pathToFile.GetData()
			);
		}
		pathToFile = appPath.Get(PATHID_FILE_TREEVIEWIMAGE);

		if (pathToFile.succeeded())
		{
			DeleteFile(
				pathToFile.GetData()
			);
		}
		mustRestart = true;
	}
	if (this->cStates.autocompleteSetup)
	{
		auto pathToFile = appPath.Get(PATHID_FILE_AUTOCOMPLETE);

		if (pathToFile.succeeded())
		{
			DeleteFile(
				pathToFile.GetData()
			);
		}
	}
	if (this->cStates.autosyntaxSetup)
	{
		auto pathToFile = appPath.Get(PATHID_FILE_AUTOSYNTAX);

		if (pathToFile.succeeded())
		{
			DeleteFile(
				pathToFile.GetData()
			);
		}
	}
	if (this->cStates.commonSettings)
	{
		auto pathTofile = appPath.Get(PATHID_FILE_SETTINGS);

		if (pathTofile.succeeded())
		{
			DeleteFile(
				pathTofile.GetData()
			);
		}

		pathTofile = appPath.Get(PATHID_FILE_EXSETTINGS);

		if (pathTofile.succeeded())
		{
			DeleteFile(
				pathTofile.GetData()
			);
		}
	}
	if (this->cStates.interfaceParameter)
	{
		auto pathToFile = appPath.Get(PATHID_FILE_COMSETUP);

		if (pathToFile.succeeded())
		{
			DeleteFile(
				pathToFile.GetData()
			);
		}
	}
	if (this->cStates.internalSetup)
	{
		auto pathToFile = appPath.Get(PATHID_FILE_INTERNALSETUP);

		if (pathToFile.succeeded())
		{
			DeleteFile(
				pathToFile.GetData()
			);
		}
	}
	if (this->cStates.usertext)
	{
		auto pathToFile = appPath.Get(PATHID_FILE_USERTEXT);

		if (pathToFile.succeeded())
		{
			DeleteFile(
				pathToFile.GetData()
			);
		}

		//delete property descriptions
		pathToFile = appPath.Get(PATHID_FOLDER_CNCSUITE_APPDATA_STRINGS);
		if (pathToFile.succeeded())
		{
			for (int i = 1; i < 4; i++)
			{
				auto path = pathToFile;
				path += L"\\";

				iString propName(L"Dproperty");
				propName += i;
				propName += L".dat";

				path += propName;

				DeleteFile(
					path.GetData()
				);
			}
		}
		// update the description-window!
		_MSG_TO_MAIN(WM_SETDESCRIPTIONS, 0, (LPARAM)nullptr);
	}
	this->CloseDialog();

	if (mustRestart)
	{
		_MSG_TO_MAIN(WM_RESTARTAPPLICATION, (WPARAM)RESTART_DONOTSAVE, 0);
	}
}
