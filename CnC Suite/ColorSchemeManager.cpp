#include"ColorSchemeManager.h"
#include"HelperF.h"
#include"AppPath.h"
#include"ExtendedResources.h"

colorSchemeManager::colorSchemeManager(HINSTANCE hInst)
	:hInstance(hInst),
	colorSchemes(nullptr),
	currentTriggerCharacter(L'A'),
	currentColorForCharacter(RGB(255, 255, 255)),
	editControl(nullptr)
{
	this->colorSchemes = new itemCollection<iString>();
	if (this->colorSchemes != nullptr)
	{
		loadSchemeList(this->colorSchemes);
	}
}

colorSchemeManager::~colorSchemeManager()
{
	if (this->colorSchemes != nullptr)
	{
		delete this->colorSchemes;
	}
	if (this->editControl != nullptr)
	{
		SafeRelease(&this->editControl);
	}
}

HRESULT colorSchemeManager::init(HWND Parent, HWND mainWindow)
{
	this->parent = Parent;
	this->MainWindow = mainWindow;
	SendMessage(mainWindow, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&this->sInfo));

	HRESULT hr = this->registerSchemeManagerClass();

	if (SUCCEEDED(hr))
	{
		RECT rc_client, rc_window;
		GetClientRect(this->parent, &rc_client);
		GetWindowRect(this->parent, &rc_window);

		this->hwndSchemeManager
			= CreateWindow(
				SCHEMEMANAGERCLASS,
				nullptr,
				WS_POPUP | WS_BORDER,
				rc_window.left + 1,
				rc_window.top + 1,
				rc_client.right,
				rc_client.bottom,
				Parent,
				nullptr,
				this->hInstance,
				reinterpret_cast<LPVOID>(this)
			);

		hr = (this->hwndSchemeManager != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			hr = this->createControls();
			if (SUCCEEDED(hr))
			{
				ShowWindow(this->hwndSchemeManager, SW_SHOW);
			}
		}
	}
	return hr;
}

HRESULT colorSchemeManager::registerSchemeManagerClass()
{
	WNDCLASSEX wcx;
	HRESULT hr = S_OK;

	if (!GetClassInfoEx(this->hInstance, SCHEMEMANAGERCLASS, &wcx))
	{
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcx.lpfnWndProc = colorSchemeManager::schemeProc;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = sizeof(LONG_PTR);
		wcx.hbrBackground = nullptr;
		wcx.hIcon = nullptr;
		wcx.hInstance = this->hInstance;
		wcx.hIconSm = nullptr;
		wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcx.lpszClassName = SCHEMEMANAGERCLASS;
		wcx.lpszMenuName = nullptr;

		hr = (RegisterClassEx(&wcx) != 0) ? S_OK : E_FAIL;
	}
	return hr;
}

HRESULT colorSchemeManager::createControls()
{
	HRESULT hr;
	RECT rc;
	GetClientRect(this->hwndSchemeManager, &rc);

	POINT pt;
	pt.x = DPIScale(10);
	pt.y = rc.bottom - DPIScale(40);

	SIZE sz;
	sz.cx = DPIScale(220);
	sz.cy = DPIScale(34);

	auto exitbutton = new CustomButton(this->hwndSchemeManager, BUTTONMODE_ICONTEXT, &pt, &sz, CTRLID_EXIT, this->hInstance);

	hr = (exitbutton != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		iString exitbuttonText(
			getStringFromResource(UI_SMP_REJECT)
		);

		exitbutton->setAppearance_IconText(IDI_SMP_CLOSEMANAGER, 24, exitbuttonText);
		exitbutton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
		exitbutton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
		exitbutton->setBorder(TRUE, this->sInfo.OutlineColor);
		exitbutton->setTextColor(this->sInfo.TextColor);
		exitbutton->setFont(
			CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
		);

		hr = exitbutton->Create();
		if (SUCCEEDED(hr))
		{
			iString btext(getStringFromResource(UI_PROPWND_CHOOSECOLOR));

			pt.x = DPIScale(10);
			pt.y = DPIScale(120);
			sz.cx = DPIScale(220);
			sz.cy = DPIScale(34);

			auto chooseColorButton = new CustomButton(this->hwndSchemeManager, BUTTONMODE_ICONTEXT, &pt, &sz, CTRLID_CHOOSECOLOR, this->hInstance);

			hr = (chooseColorButton != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				chooseColorButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
				chooseColorButton->setAppearance_IconText(IDI_PROP_CHOOSECOLOR, 32, btext);
				chooseColorButton->setTextColor(this->sInfo.TextColor);
				chooseColorButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
				chooseColorButton->setFont(
					CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
				);

				hr = chooseColorButton->Create();
				if (SUCCEEDED(hr))
				{
					iString firstEntry(getStringFromResource(UI_PROPWND_NEWSCHEME));
					iString separator(L"--------------");

					auto schemeCombo =
						new comboBox(
							this->hInstance,
							this->hwndSchemeManager,
							COMBOTYPE_DROPDOWNLIST,
							CTRLID_SCHEMECOMBO,
							DPIScale(10),
							DPIScale(10),
							DPIScale(220),
							0
						);
					if (schemeCombo != nullptr)
					{
						schemeCombo->Items->AddItem(firstEntry);
						schemeCombo->Items->AddItem(separator);

						if (this->colorSchemes != nullptr)
						{
							for (int i = 0; i < this->colorSchemes->GetCount(); i++)
							{
								auto str = this->colorSchemes->GetAt(i);
								schemeCombo->Items->AddItem(str);
							}
						}
						schemeCombo->setSelectedIndex(0);
						schemeCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
						schemeCombo->setFont(
							CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
						);
					}
					else
						hr = E_FAIL;

					if (SUCCEEDED(hr))
					{
						auto nameEdit = new singleLineEdit(this->hInstance);

						hr = (nameEdit != nullptr) ? S_OK : E_FAIL;
						if (SUCCEEDED(hr))
						{
							iString name(L"Name:");

							//nameEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));

							nameEdit->setType(
								SLE_TYPE_WITHDESCRIPTION,
								DPIScale(50)
							);
							nameEdit->setEditFontProperties(
								iString(APPLICATION_PRIMARY_FONT),
								DPIScale(14),
								DPIScale(22)
							);
							nameEdit->setDescriptionText(name);
							nameEdit->setCtrlFont(
								CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
							);
							nameEdit->setBorder(true, this->sInfo.TextColor);
							nameEdit->setAdditionalEditStyles(ES_CENTER | ES_NOHIDESEL);
							nameEdit->setAlignment(DESC_ALIGN_LEFT);
							nameEdit->setSpacing(0);
							nameEdit->setDimensions(
								DPIScale(10),
								DPIScale(50),
								DPIScale(220)
							);
							nameEdit->setColors(this->sInfo.TabColor, this->sInfo.OutlineColor, this->sInfo.TextColor, this->sInfo.TextColor);
							nameEdit->restrictedContent(SLE_RESTRICTTYPE_FORFILENAMES);

							hr = nameEdit->Create(this->hwndSchemeManager, CTRLID_NAMEEDIT);
							if (SUCCEEDED(hr))
							{
								nameEdit->setContent(getStringFromResource(UI_PROPWND_NEWSCHEME));
								nameEdit->selectAll();

								TCHAR buffer[2] = { L'\0' };

								iString Entry;
								
								auto triggerCombo =
									new comboBox(
										this->hInstance,
										this->hwndSchemeManager,
										COMBOTYPE_DROPDOWNLIST,
										CTRLID_TRIGGERCOMBO,
										DPIScale(60),
										DPIScale(90),
										DPIScale(170),
										0
									);
								if (triggerCombo != nullptr)
								{
									iString Char;

									for (int i = 65; i < 91; i++)
									{
										buffer[0] = static_cast<TCHAR>(i);
										Char.Replace(buffer);
										triggerCombo->Items->AddItem(Char);
									}

									for (int i = 0; i < 4; i++)
									{
										Entry.Replace(
											getStringFromResource(UI_PROPWND_ANNOTATION + i)
										);
										triggerCombo->Items->AddItem(Entry);
									}
									triggerCombo->setSelectedIndex(0);
									triggerCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
									triggerCombo->setFont(
										CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
									);
								}
								else
									hr = E_FAIL;

								pt.x = DPIScale(10);
								pt.y = rc.bottom - DPIScale(80);
								sz.cx = DPIScale(220);
								sz.cy = DPIScale(34);

								btext.Replace(getStringFromResource(UI_GNRL_PLAIN_SAVE));

								auto saveButton = new CustomButton(this->hwndSchemeManager, BUTTONMODE_ICONTEXT, &pt, &sz, CTRLID_SAVESCHEME, this->hInstance);

								hr = (saveButton != nullptr) ? S_OK : E_FAIL;
								if (SUCCEEDED(hr))
								{
									saveButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
									saveButton->setAppearance_IconText(IDI_SMP_SAVESAMPLE, 24, btext);
									saveButton->setTextColor(this->sInfo.TextColor);
									saveButton->setBorder(TRUE, this->sInfo.OutlineColor);
									saveButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
									saveButton->setFont(
										CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
									);

									hr = saveButton->Create();
									if (SUCCEEDED(hr))
									{
										this->edit = CreateWindow(
											MSFTEDIT_CLASS,
											nullptr,
											WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_NOHIDESEL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL,
											DPIScale(240),
											DPIScale(10),
											rc.right - DPIScale(242),
											rc.bottom - DPIScale(20),
											this->hwndSchemeManager,
											reinterpret_cast<HMENU>(CTRLID_SHOWEDIT),
											this->hInstance, nullptr
										);

										hr = (this->edit != nullptr) ? S_OK : E_FAIL;
										if (SUCCEEDED(hr))
										{
											this->editControl = CreateEditControlInstance(this->edit, this->hwndSchemeManager);

											hr = (this->editControl != nullptr) ? S_OK : E_FAIL;
											if (SUCCEEDED(hr))
											{
												
												EditControl::initEditStyleColorsWithSingleColor(
													&this->editStyleColors,
													InvertColor(this->sInfo.TabColor)
												);
												SendMessage(
													this->MainWindow,
													WM_GETEDITCONTROLPROPERIES,
													reinterpret_cast<WPARAM>(&this->editStyleColors),
													reinterpret_cast<LPARAM>(&this->editControlProperties));

												this->currentColorForCharacter = this->editStyleColors.A;
												RedrawWindow(this->hwndSchemeManager, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);

												this->editControlProperties.focusmark = FALSE;
																								
												this->editControl->ConfigureComponent(&this->editStyleColors, &this->editControlProperties, TRUE, TRUE);

												LPCWSTR stowBuffer = nullptr;

												stowBuffer =
													(getCurrentAppLanguage() == LANG_GERMAN)
													? EDITCOLORSAMPLEFILE_CONTENT_DE
													: EDITCOLORSAMPLEFILE_CONTENT_EN;

												this->editControl->InsertText(stowBuffer, FALSE);

												pt.x = DPIScale(10);
												pt.y = rc.bottom - DPIScale(160);
												sz.cx = DPIScale(220);
												sz.cy = DPIScale(22);

												btext.Replace(getStringFromResource(UI_PROPWND_RESETTEXTCOLOR));

												auto resetTextButton = new CustomButton(this->hwndSchemeManager, BUTTONMODE_TEXT, &pt, &sz, CTRLID_RESETTEXTCOLOR, this->hInstance);

												hr = (resetTextButton != nullptr) ? S_OK : E_FAIL;
												if (SUCCEEDED(hr))
												{
													resetTextButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
													resetTextButton->setAppearance_onlyText(btext, FALSE);
													resetTextButton->setTextColor(this->sInfo.TextColor);
													resetTextButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
													resetTextButton->setFont(
														CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
													);

													hr = resetTextButton->Create();
													if (SUCCEEDED(hr))
													{
														pt.x = DPIScale(10);
														pt.y = rc.bottom - DPIScale(130);
														sz.cx = DPIScale(220);
														sz.cy = DPIScale(22);

														btext.Replace(getStringFromResource(UI_PROPWND_RESETBACKGROUND));

														auto resetBkgndButton = new CustomButton(this->hwndSchemeManager, BUTTONMODE_TEXT, &pt, &sz, CTRLID_RESETBACKGROUND, this->hInstance);

														hr = (resetBkgndButton != nullptr) ? S_OK : E_FAIL;
														if (SUCCEEDED(hr))
														{
															resetBkgndButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
															resetBkgndButton->setAppearance_onlyText(btext, FALSE);
															resetBkgndButton->setTextColor(this->sInfo.TextColor);
															resetBkgndButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
															resetBkgndButton->setFont(
																CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
															);

															hr = resetBkgndButton->Create();
															if (SUCCEEDED(hr))
															{
																nameEdit->setFocus();
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

LRESULT colorSchemeManager::schemeProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	colorSchemeManager* csm = nullptr;

	if (message == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		csm = reinterpret_cast<colorSchemeManager*>(pcr->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(csm));
		return 1;
	}
	else
	{
		csm = reinterpret_cast<colorSchemeManager*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (csm != nullptr)
		{
			switch (message)
			{
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_PAINT:
				return csm->onPaint(hWnd);
			case WM_DESTROY:
				SafeRelease(&csm);
				return 0;
			default:
				break;
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}	
}

LRESULT colorSchemeManager::onPaint(HWND hWnd)
{
	RECT rc;
	PAINTSTRUCT ps;
	HDC hdc;

	hdc = BeginPaint(hWnd, &ps);
	if (hdc)
	{
		GetClientRect(hWnd, &rc);

		HBRUSH brush = CreateSolidBrush(this->sInfo.TabColor);
		if (brush)
		{
			FillRect(hdc, &rc, brush);

			HBRUSH charcolorBrush = CreateSolidBrush(this->currentColorForCharacter);
			if (charcolorBrush)
			{
				RECT rc_charcolor = {
					DPIScale(10),
					DPIScale(90),
					DPIScale(50),
					DPIScale(115)
				};
				FillRect(hdc, &rc_charcolor, charcolorBrush);

				HPEN pen = CreatePen(PS_SOLID, 1, this->sInfo.TextColor);

				if (pen)
				{
					HGDIOBJ original = SelectObject(hdc, pen);

					MoveToEx(hdc, DPIScale(10), DPIScale(40), nullptr);
					LineTo(hdc, DPIScale(230), DPIScale(40));

					MoveToEx(hdc, DPIScale(10), DPIScale(80), nullptr);
					LineTo(hdc, DPIScale(230), DPIScale(80));

					MoveToEx(hdc, DPIScale(10), DPIScale(160), nullptr);
					LineTo(hdc, DPIScale(230), DPIScale(160));

					MoveToEx(hdc, DPIScale(235), DPIScale(5), nullptr);
					LineTo(hdc, DPIScale(235), rc.bottom - DPIScale(5));

					if (this->sInfo.StyleID == STYLEID_DARKGRAY)
					{
						MoveToEx(hdc, 0, 0, nullptr);
						LineTo(hdc, rc.right - 1, 0);
						LineTo(hdc, rc.right - 1, rc.bottom - 1);
						LineTo(hdc, 0, rc.bottom - 1);
						LineTo(hdc, 0, 0);
					}
					SelectObject(hdc, original);
					DeleteObject(pen);

				}
				DeleteObject(charcolorBrush);
			}
			DeleteObject(brush);
		}
		EndPaint(hWnd, &ps);
	}
	return static_cast<LRESULT>(0);
}

void colorSchemeManager::onButtonClicked(CustomButton * sender, CTRLID ctrlID)
{
	UNREFERENCED_PARAMETER(sender);

	switch (ctrlID)
	{
	case CTRLID_EXIT:
		this->Close();
		break;
	case CTRLID_SAVESCHEME:
		if (this->saveCurrentScheme())
		{
			iString schemeName =
				reinterpret_cast<singleLineEdit*>(
					GetWindowLongPtr(
						GetDlgItem(this->hwndSchemeManager, CTRLID_NAMEEDIT), GWLP_USERDATA)
					)->getContent();

			SendMessage(
				this->parent,
				WM_COMMAND,
				MAKEWPARAM(CMD_REFRESH_COLORSCHEMELIST, 0),
				0
			);
			SendMessage(
				this->parent,
				WM_COMMAND,
				MAKEWPARAM(CMD_SELECT_NEWCOLORSCHEME, 0),
				reinterpret_cast<LPARAM>(schemeName.GetData())
			);

			this->Close();
		}
		else
		{
			// TODO: notify user !!!
		}
		break;
	case CTRLID_CHOOSECOLOR:
		this->currentColorForCharacter =
			this->getColorFromUser(this->currentColorForCharacter);
		this->currentColorToEditbox();
		RedrawWindow(this->hwndSchemeManager, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
		break;
	case CTRLID_RESETBACKGROUND:
		this->resetBackground();
		break;
	case CTRLID_RESETTEXTCOLOR:
		this->resetText();
		break;
	default:
		break;
	}
}

void colorSchemeManager::onComboSelchanged(comboBox * sender, int selIndex)
{
	auto ctrlID = sender->getCtrlId();

	switch (ctrlID)
	{
	case CTRLID_SCHEMECOMBO:
		if (selIndex == 1)
		{
			sender->setSelectedIndex(0);
			return;
		}
		else
		{
			// load specific existing scheme or clear all for new scheme

			if (selIndex == 0)
			{
				this->resetBackground();
				this->resetText();
				auto editbox
					= reinterpret_cast<singleLineEdit*>(
						GetWindowLongPtr(
							GetDlgItem(this->hwndSchemeManager, CTRLID_NAMEEDIT),
							GWLP_USERDATA)
						);
				if (editbox != nullptr)
				{
					editbox->setContent(
						getStringFromResource(UI_PROPWND_NEWSCHEME)
					);
					editbox->selectAll();
				}
			}
			else
			{
				if (selIndex < sender->Items->GetCount())
				{
					AppPath pathManager;
					auto schemeName = sender->Items->GetAt(selIndex);

					auto path = pathManager.Get(PATHID_FOLDER_CNCSUITE_USERSTYLES_EDITOR);
					path.Append(L"\\");
					path.Append(schemeName);
					path.Append(L".xml");

					auto parser = new XML_Parser(dynamic_cast<XMLParsingEventSink*>(this));
					if (parser != nullptr)
					{
						if (parser->OpenDocument(path))
						{
							parser->ParseAsync();
						}
					}
				}
			}
		}
		break;
	case CTRLID_TRIGGERCOMBO:
		this->setTriggerCharFromSelIndex(selIndex);
		break;
	default:
		break;
	}
}


COLORREF colorSchemeManager::getColorFromUser(COLORREF current)
{
	int rgb_val = 255;
	COLORREF custClr[16];
	for (int i = 0; i < 16; i++)
	{
		rgb_val = 255 - (i * 15);
		custClr[i] = RGB(rgb_val, rgb_val, rgb_val);
	}

	CHOOSECOLOR cclr;
	SecureZeroMemory(&cclr, sizeof(CHOOSECOLOR));
	cclr.lStructSize = sizeof(cclr);
	cclr.hwndOwner = this->hwndSchemeManager;
	cclr.Flags = CC_FULLOPEN | CC_RGBINIT;
	cclr.lpCustColors = (LPDWORD)custClr;
	cclr.rgbResult = current;

	ChooseColor(&cclr);

	return cclr.rgbResult;
}

void colorSchemeManager::setTriggerCharFromSelIndex(int selectedIndex)
{
	if (selectedIndex < 26)
	{
		this->currentTriggerCharacter = static_cast<TCHAR>(65 + selectedIndex);
	}
	else
	{
		this->currentTriggerCharacter = static_cast<TCHAR>(48 + (selectedIndex - 26));
	}
	this->currentColorForCharacter = this->colorFromIndex(selectedIndex);
	RedrawWindow(this->hwndSchemeManager, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void colorSchemeManager::currentColorToEditbox()
{
	switch (this->currentTriggerCharacter)
	{
	case L'A':
		this->editStyleColors.A = this->currentColorForCharacter;
		break;
	case L'B':
		this->editStyleColors.B = this->currentColorForCharacter;
		break;
	case L'C':
		this->editStyleColors.C = this->currentColorForCharacter;
		break;
	case L'D':
		this->editStyleColors.D = this->currentColorForCharacter;
		break;
	case L'E':
		this->editStyleColors.E = this->currentColorForCharacter;
		break;
	case L'F':
		this->editStyleColors.F = this->currentColorForCharacter;
		break;
	case L'G':
		this->editStyleColors.G = this->currentColorForCharacter;
		break;
	case L'H':
		this->editStyleColors.H = this->currentColorForCharacter;
		break;
	case L'I':
		this->editStyleColors.I = this->currentColorForCharacter;
		break;
	case L'J':
		this->editStyleColors.J = this->currentColorForCharacter;
		break;
	case L'K':
		this->editStyleColors.K = this->currentColorForCharacter;
		break;
	case L'L':
		this->editStyleColors.L = this->currentColorForCharacter;
		break;
	case L'M':
		this->editStyleColors.M = this->currentColorForCharacter;
		break;
	case L'N':
		this->editStyleColors.N = this->currentColorForCharacter;
		break;
	case L'O':
		this->editStyleColors.O = this->currentColorForCharacter;
		break;
	case L'P':
		this->editStyleColors.P = this->currentColorForCharacter;
		break;
	case L'Q':
		this->editStyleColors.Q = this->currentColorForCharacter;
		break;
	case L'R':
		this->editStyleColors.R = this->currentColorForCharacter;
		break;
	case L'S':
		this->editStyleColors.S = this->currentColorForCharacter;
		break;
	case L'T':
		this->editStyleColors.T = this->currentColorForCharacter;
		break;
	case L'U':
		this->editStyleColors.U = this->currentColorForCharacter;
		break;
	case L'V':
		this->editStyleColors.V = this->currentColorForCharacter;
		break;
	case L'W':
		this->editStyleColors.W = this->currentColorForCharacter;
		break;
	case L'X':
		this->editStyleColors.X = this->currentColorForCharacter;
		break;
	case L'Y':
		this->editStyleColors.Y = this->currentColorForCharacter;
		break;
	case L'Z':
		this->editStyleColors.Z = this->currentColorForCharacter;
		break;
	case L'0':
		this->editStyleColors.Annotation = this->currentColorForCharacter;
		break;
	case L'1':
		this->editStyleColors.defaultTextcolor = this->currentColorForCharacter;
		break;
	case L'2':
		this->editStyleColors.LineNumber = this->currentColorForCharacter;
		break;
	case L'3':
		this->editStyleColors.background = this->currentColorForCharacter;
		break;
	default:
		return;
	}
	this->editControl->ConfigureComponent(
		&this->editStyleColors,
		&this->editControlProperties,
		TRUE, TRUE
	);
}

void colorSchemeManager::setCurrentColorForCharacter()
{
	// ????
}

void colorSchemeManager::resetText()
{
	auto bkColor = this->editStyleColors.background;
	auto newColor = InvertColor(bkColor);
	EditControl::initEditStyleColorsWithSingleColor(&this->editStyleColors, newColor);
	this->editStyleColors.background = bkColor;

	if (this->currentTriggerCharacter != L'3')
	{
		this->currentColorForCharacter = newColor;
		RedrawWindow(this->hwndSchemeManager, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
	this->redrawEditbox();
}

void colorSchemeManager::resetBackground()
{
	this->editStyleColors.background = this->sInfo.TabColor;

	if (this->currentTriggerCharacter == L'3')
	{
		this->currentColorForCharacter = this->sInfo.TabColor;
		RedrawWindow(this->hwndSchemeManager, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
	this->redrawEditbox();
}

void colorSchemeManager::redrawEditbox()
{
	this->editControl->ConfigureComponent(&this->editStyleColors, &this->editControlProperties, TRUE, TRUE);
}

COLORREF colorSchemeManager::colorFromIndex(int index)
{
	switch (index)
	{
	case 0:
		return this->editStyleColors.A;
		break;
	case 1:
		return this->editStyleColors.B;
		break;
	case 2:
		return this->editStyleColors.C;
		break;
	case 3:
		return this->editStyleColors.D;
		break;
	case 4:
		return this->editStyleColors.E;
		break;
	case 5:
		return this->editStyleColors.F;
		break;
	case 6:
		return this->editStyleColors.G;
		break;
	case 7:
		return this->editStyleColors.H;
		break;
	case 8:
		return this->editStyleColors.I;
		break;
	case 9:
		return this->editStyleColors.J;
		break;
	case 10:
		return this->editStyleColors.K;
		break;
	case 11:
		return this->editStyleColors.L;
		break;
	case 12:
		return this->editStyleColors.M;
		break;
	case 13:
		return this->editStyleColors.N;
		break;
	case 14:
		return this->editStyleColors.O;
		break;
	case 15:
		return this->editStyleColors.P;
		break;
	case 16:
		return this->editStyleColors.Q;
		break;
	case 17:
		return this->editStyleColors.R;
		break;
	case 18:
		return this->editStyleColors.S;
		break;
	case 19:
		return this->editStyleColors.T;
		break;
	case 20:
		return this->editStyleColors.U;
		break;
	case 21:
		return this->editStyleColors.V;
		break;
	case 22:
		return this->editStyleColors.W;
		break;
	case 23:
		return this->editStyleColors.X;
		break;
	case 24:
		return this->editStyleColors.Y;
		break;
	case 25:
		return this->editStyleColors.Z;
		break;
	case 26:
		return this->editStyleColors.Annotation;
		break;
	case 27:
		return this->editStyleColors.defaultTextcolor;
		break;
	case 28:
		return this->editStyleColors.LineNumber;
		break;
	case 29:
		return this->editStyleColors.background;
		break;
	default:
		return RGB(255, 255, 255);
	}
}

bool colorSchemeManager::saveCurrentScheme()
{
	auto builder = new XML_Builder();
	if (builder != nullptr)
	{
		AppPath pathManager;
		auto path = pathManager.Get(PATHID_FOLDER_CNCSUITE_USERSTYLES_EDITOR);

		iXML_Tag tag;
		XML_TAG_Property prop;

		iString schemeName =
			reinterpret_cast<singleLineEdit*>(
				GetWindowLongPtr(
					GetDlgItem(this->hwndSchemeManager, CTRLID_NAMEEDIT), GWLP_USERDATA)
				)->getContent();

		if (schemeName.GetLength() == 0)
		{
			// notify user ???

			return false;
		}

		path += L"\\\0";
		path += schemeName;
		path += L".xml\0";

		builder->setUp_DocumentDefinition(L"1.0", L"utf-8", L"eclr", true);

		tag.tagName.Replace(L"color");
		tag.hasProperties = true;
		prop.propertyName = L"forType\0";

		TCHAR buffer[2] = { L'\0' };

		// char-colors
		for (int i = 0; i < 26; i++)
		{
			tag.tagContent.Replace(
				iString::fromHex(this->colorFromIndex(i)));
			buffer[0] = static_cast<TCHAR>(65 + i);
			prop.propertyContent = buffer;
			tag.tagProperties.Clear();
			tag.tagProperties.AddItem(prop);
			builder->AddTag(&tag);
		}
		// annotation
		tag.tagContent.Replace(
			iString::fromHex(this->editStyleColors.Annotation));
		prop.propertyContent = L"annotation\0";
		tag.tagProperties.Clear();
		tag.tagProperties.AddItem(prop);
		builder->AddTag(&tag);

		// defaulttext
		tag.tagContent.Replace(
			iString::fromHex(this->editStyleColors.defaultTextcolor));
		prop.propertyContent = L"defaulttext\0";
		tag.tagProperties.Clear();
		tag.tagProperties.AddItem(prop);
		builder->AddTag(&tag);

		// linenumber
		tag.tagContent.Replace(
			iString::fromHex(this->editStyleColors.LineNumber));
		prop.propertyContent = L"linenumber\0";
		tag.tagProperties.Clear();
		tag.tagProperties.AddItem(prop);
		builder->AddTag(&tag);

		// background
		tag.tagContent.Replace(
			iString::fromHex(this->editStyleColors.background));
		prop.propertyContent = L"background\0";
		tag.tagProperties.Clear();
		tag.tagProperties.AddItem(prop);
		builder->AddTag(&tag);

		builder->finalizeAsync(path, true);

		return true;
	}
	return false;
}

void colorSchemeManager::Close()
{
	SendMessage(this->parent, WM_COMMAND, MAKEWPARAM(ICOMMAND_SETCANCLOSE, TRUE), 0);
	DestroyWindow(this->hwndSchemeManager);
}

bool colorSchemeManager::loadSchemeList(itemCollection<iString>* list_out)
{
	AppPath pathManager;
	iString path(L"\\\\?\\");
	path.Append(
		pathManager.Get(PATHID_FOLDER_CNCSUITE_USERSTYLES_EDITOR));
	path.Append(L"\\*");

	WIN32_FIND_DATA wfd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	hFind = FindFirstFile(path.GetData(), &wfd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		while (wfd.cFileName[0] == '.')
		{
			if (FindNextFile(hFind, &wfd) == 0)
			{
				FindClose(hFind);
				return false;
			}
		}
		iString schemeName;
		auto bfpo = CreateBasicFPO();

		if (bfpo != nullptr)
		{
			do
			{
				if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					TCHAR *extension = nullptr;

					if (bfpo->RemoveFileExtension(wfd.cFileName, &extension))
					{
						if (CompareStringsB(extension, L".xml"))
						{
							schemeName.Replace(wfd.cFileName);
							list_out->AddItem(schemeName);
						}
						SafeDeleteArray(&extension);
					}
				}
			} while (FindNextFile(hFind, &wfd) != 0);

			SafeRelease(&bfpo);
		}
		FindClose(hFind);

		return true;
	}
	return false;
}

bool colorSchemeManager::xmlToColorStruct(itemCollection<iXML_Tag>* xmlColorStructure, LPEDITSTYLECOLORS esc)
{
	if (xmlColorStructure->GetCount() < 30)
		return false;
	else
	{
		iString compStr;

		for (int i = 0; i < 30; i++)
		{
			if (i < 26)
			{
				TCHAR letterForColor[2] = { static_cast<TCHAR>(65 + i), L'\0' };
				compStr.Replace(letterForColor);
			}
			else
			{
				switch (i)
				{
				case 26:
					compStr.Replace(L"annotation");
					break;
				case 27:
					compStr.Replace(L"defaulttext");
					break;
				case 28:
					compStr.Replace(L"linenumber");
					break;
				case 29:
					compStr.Replace(L"background");
					break;
				default:
					return false;
				}
			}
			auto tag = xmlColorStructure->GetAt(i);
			if (
				tag
				.tagProperties
				.GetAt(0)
				.propertyContent
				.Equals(compStr))
			{
				COLORREF color;
				int value;
				iString valueToConvert(L"0x");
				valueToConvert += tag.tagContent;

				if (StrToIntEx(valueToConvert.GetData(), STIF_SUPPORT_HEX, &value))
				{
					color = (COLORREF)value;
					colorSchemeManager::indexToColor(i, color, esc);
				}
				else
					return false;
			}
		}
	}
	return true;
}

void colorSchemeManager::indexToColor(int index, COLORREF color, LPEDITSTYLECOLORS esc)
{
	switch (index)
	{
	case 0:
		esc->A = color;
		break;
	case 1:
		esc->B = color;
		break;
	case 2:
		esc->C = color;
		break;
	case 3:
		esc->D = color;
		break;
	case 4:
		esc->E = color;
		break;
	case 5:
		esc->F = color;
		break;
	case 6:
		esc->G = color;
		break;
	case 7:
		esc->H = color;
		break;
	case 8:
		esc->I = color;
		break;
	case 9:
		esc->J = color;
		break;
	case 10:
		esc->K = color;
		break;
	case 11:
		esc->L = color;
		break;
	case 12:
		esc->M = color;
		break;
	case 13:
		esc->N = color;
		break;
	case 14:
		esc->O = color;
		break;
	case 15:
		esc->P = color;
		break;
	case 16:
		esc->Q = color;
		break;
	case 17:
		esc->R = color;
		break;
	case 18:
		esc->S = color;
		break;
	case 19:
		esc->T = color;
		break;
	case 20:
		esc->U = color;
		break;
	case 21:
		esc->V = color;
		break;
	case 22:
		esc->W = color;
		break;
	case 23:
		esc->X = color;
		break;
	case 24:
		esc->Y = color;
		break;
	case 25:
		esc->Z = color;
		break;
	case 26:
		esc->Annotation = color;
		break;
	case 27:
		esc->defaultTextcolor = color;
		break;
	case 28:
		esc->LineNumber = color;
		break;
	case 29:
		esc->background = color;
		break;
	default:
		break;
	}
}
