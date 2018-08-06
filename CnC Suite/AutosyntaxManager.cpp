#include"AutosyntaxManager.h"
#include"HelperF.h"

autosyntaxManager::autosyntaxManager(HINSTANCE hInst)
	:hInstance(hInst),
	ausynWnd(nullptr),
	Parent(nullptr),
	syntaxHasChanged(false),
	msgBlocker(false),
	currentScrollPosition(0)
{
	SendMessage(_MAIN_WND_, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&this->styleInfo));

	this->ctrlFont = CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT);
}

autosyntaxManager::~autosyntaxManager()
{
	if (this->ctrlFont != nullptr)
	{
		DeleteObject(this->ctrlFont);
	}
}

const TCHAR* autosyntaxManager::AutoSyntaxManagerWindowCLASS = L"AUTOSYNTAXMANAGERWINDOWCLASS";

HRESULT autosyntaxManager::Init(LPCTRLCREATIONSTRUCT pcr)
{
	HRESULT hr = (pcr != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		hr = this->_registerASClass();
		if (SUCCEEDED(hr))
		{
			this->Parent = pcr->parent;

			this->ausynWnd
				= CreateWindow(
					autosyntaxManager::AutoSyntaxManagerWindowCLASS,
					nullptr,
					WS_CHILD | WS_VSCROLL | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
					pcr->pos.x,
					pcr->pos.y,
					pcr->size.cx,
					pcr->size.cy,
					pcr->parent,
					reinterpret_cast<HMENU>(this->ASWindow_ID),
					this->hInstance,
					reinterpret_cast<LPVOID>(this)			
				);
			hr = (this->ausynWnd != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				hr = this->_createControls();
				if (SUCCEEDED(hr))
				{
					SCROLLINFO sci;
					sci.cbSize = sizeof(SCROLLINFO);
					sci.fMask = SIF_RANGE | SIF_PAGE;
					sci.nMax = DPIScale(840);
					sci.nMin = 0;
					sci.nPage = DPIScale(600);

					SetScrollInfo(this->ausynWnd, SB_VERT, &sci, TRUE);
					ShowWindow(this->ausynWnd, SW_SHOW);
				}
			}
		}
	}
	return hr;
}

void autosyntaxManager::Close()
{
	SendMessage(this->ausynWnd, WM_CLOSE, 0, 0);
}

bool autosyntaxManager::GetAutosyntaxSettings(LPAUTOSYNTAXPROPERTY asp)
{
	auto syntaxSettings =
		reinterpret_cast<ApplicationData*>(
			getApplicationDataContainerFromFilekey(FILEKEY_AUTOSYNTAX_SETTINGS)
		);
	if (syntaxSettings != nullptr)
	{
		asp->IsOn =
			syntaxSettings->getBooleanData(DATAKEY_SETTINGS_AUTOSYNTAX_ISON, false)
			? TRUE : FALSE;
		asp->UseDifferentNumLengthInSubProg =
			syntaxSettings->getBooleanData(DATAKEY_SETTINGS_AUTOSYNTAX_USEDIFFNUMLENINSUB, false)
			? TRUE : FALSE;

		asp->MainLevelNumLength = syntaxSettings->getIntegerData(DATAKEY_SETTINGS_AUTOSYNTAX_NUMLENGTHMAIN, 1);
		asp->SubLevelNumLength = syntaxSettings->getIntegerData(DATAKEY_SETTINGS_AUTOSYNTAX_NUMLENGTHSUB, 1);

		asp->noSpaceInLineNumber =
			syntaxSettings->getBooleanData(DATAKEY_SETTINGS_AUTOSYNTAX_NOSPACEINLINENUMBER, false)
			? TRUE : FALSE;
		asp->eraseLinenumberOnBackspace =
			syntaxSettings->getBooleanData(DATAKEY_SETTINGS_AUTOSYNTAX_ERASELINENUMBER, false)
			? TRUE : FALSE;
		asp->autoinsertBrackets =
			syntaxSettings->getBooleanData(DATAKEY_SETTINGS_AUTOSYNTAX_AUTOCOMPBRACKETS, true)
			? TRUE : FALSE;
		asp->noSpaceBetweenBraces =
			syntaxSettings->getBooleanData(DATAKEY_SETTINGS_AUTOSYNTAX_INSERTSPACESINBRAC, false)
			? TRUE : FALSE;

		asp->autonumStartLine = syntaxSettings->getIntegerData(DATAKEY_SETTINGS_AUTOSYNTAX_AUTONUMSTARTLINE, 2);
		asp->maximumLinenumber = syntaxSettings->getIntegerData(DATAKEY_SETTINGS_AUTOSYNTAX_MAXLINENUMBER, 9999);
		asp->numStepInSubProgramm = syntaxSettings->getIntegerData(DATAKEY_SETTINGS_AUTOSYNTAX_NUMSTEPSUB, 1);
		asp->numStepInMainProgramm = syntaxSettings->getIntegerData(DATAKEY_SETTINGS_AUTOSYNTAX_AUTONUMSTEPMAIN, 1);

		asp->newEvenLineNumberOnTrigger =
			syntaxSettings->getBooleanData(DATAKEY_SETTINGS_AUTOSYNTAX_NEWEVENLINEONTRIGG, false)
			? TRUE : FALSE;

		asp->lineNumberStartValue = syntaxSettings->getIntegerData(DATAKEY_SETTINGS_AUTOSYNTAX_MINLINENUMBER, 1);

		asp->useNoDifferentLinenumbersInSubprogram = !asp->UseDifferentNumLengthInSubProg; // confusing...!

		asp->useMultilineAnnotations =
			syntaxSettings->getBooleanData(DATAKEY_SETTINGS_AUTOSYNTAX_USEMULTILINEANNOTNS, false)
			? TRUE : FALSE;
		asp->useEndProgDetection =
			syntaxSettings->getBooleanData(DATAKEY_SETTINGS_AUTOSYNTAX_USEENDPROGDETECTION, true)
			? TRUE : FALSE;

		// for future:
		asp->AutoLevitation = FALSE;

		return true;
	}
	return false;
}

bool autosyntaxManager::GetTriggerStrings(iString & subProg, iString & endProg, iString & newLine)
{
	auto syntaxSettings =
		reinterpret_cast<ApplicationData*>(
			getApplicationDataContainerFromFilekey(FILEKEY_AUTOSYNTAX_SETTINGS)
			);
	if (syntaxSettings != nullptr)
	{
		subProg = syntaxSettings->getStringData(
			DATAKEY_SETTINGS_AUTOSYNTAX_SUBPROGTRIGGEREDIT,
			SUBPROG_TRIGGER_DEFAULTSTRING
		);
		endProg = syntaxSettings->getStringData(
			DATAKEY_SETTINGS_AUTOSYNTAX_ENDPROGTRIGGEREDIT,
			ENDPROG_TRIGGER_DEFAULTSTRING
		);
		newLine = syntaxSettings->getStringData(
			DATAKEY_SETTINGS_AUTOSYNTAX_EVENLINETRIGGEREDIT,
			NEWLINE_TRIGGER_DEFAULTSTRING
		);
		return true;
	}
	return false;
}

HRESULT autosyntaxManager::_registerASClass()
{
	HRESULT hr = S_OK;
	WNDCLASSEX wcx;

	if (GetClassInfoEx(this->hInstance, autosyntaxManager::AutoSyntaxManagerWindowCLASS, &wcx) == 0)
	{
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = sizeof(LONG_PTR);
		wcx.hbrBackground = nullptr;
		wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcx.hInstance = this->hInstance;
		wcx.lpfnWndProc = autosyntaxManager::autosynProc;
		wcx.hIcon = nullptr;
		wcx.hIconSm = nullptr;
		wcx.lpszClassName = autosyntaxManager::AutoSyntaxManagerWindowCLASS;
		wcx.lpszMenuName = nullptr;

		hr = (RegisterClassEx(&wcx) == 0) ? E_FAIL : S_OK;
	}
	return hr;
}

HRESULT autosyntaxManager::_createControls()
{
	HRESULT hr;
	RECT rc;
	GetClientRect(this->ausynWnd, &rc);

	POINT pt;
	pt.x = DPIScale(30);
	pt.y = DPIScale(790);

	SIZE sz;
	sz.cx = DPIScale(160);
	sz.cy = DPIScale(26);

	auto exitbutton = new CustomButton(this->ausynWnd, BUTTONMODE_TEXT, &pt, &sz, autosyntaxManager::ctrlID_exitButton, this->hInstance);

	hr = (exitbutton != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		iString ttp(
			getStringFromResource(UI_GNRL_NAVIGATEBACK)
		);

		exitbutton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
		exitbutton->setAppearance_onlyText(ttp, FALSE);
		exitbutton->setBorder(TRUE, this->styleInfo.OutlineColor);
		exitbutton->setTextColor(this->styleInfo.TextColor);
		exitbutton->setColors(this->styleInfo.mainToolbarColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
		exitbutton->setFont(
			CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
		);

		hr = exitbutton->Create();
		if (SUCCEEDED(hr))
		{
			auto syntaxSettings =
				reinterpret_cast<ApplicationData*>(
					getApplicationDataContainerFromFilekey(FILEKEY_AUTOSYNTAX_SETTINGS)
				);

			if (syntaxSettings != nullptr)
			{
				pt.x = DPIScale(20);
				pt.y = DPIScale(10);
				sz.cx = rc.right - DPIScale(140);
				sz.cy = DPIScale(30);

				iString ctrlText(
					getStringFromResource(UI_PROPWND_CONVERTSYNTAXONOPEN)
				);

				auto isOnCheckbox = new CustomCheckbox(this->hInstance, this->ausynWnd, &pt, &sz, autosyntaxManager::ctrlID_isOnCheckbox);

				hr = (isOnCheckbox != nullptr) ? S_OK : E_FAIL;
				if (SUCCEEDED(hr))
				{
					isOnCheckbox->setEventHandler(
						dynamic_cast<customCheckboxEventSink*>(this)
					);
					isOnCheckbox->setText(ctrlText);
					isOnCheckbox->setChecked(
						syntaxSettings->getBooleanData(DATAKEY_SETTINGS_AUTOSYNTAX_ISON, false)
					);
					isOnCheckbox->setColors(this->styleInfo.TabColor, this->styleInfo.TextColor);
					isOnCheckbox->setConstraints(
						DPIScale(10),
						DPIScale(10)
					);
					isOnCheckbox->setFont(
						CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
					);

					hr = isOnCheckbox->Create();
					if (SUCCEEDED(hr))
					{
						/*						
						don't forget:
							- triggerlines
							- num step main


							logic order:

							1.Linenumber properties
							- autonum startline EDIT
							- num step main EDIT
							- num length main EDIT

							- minimum linenumber EDIT
							- maximum linenumber EDIT

							- erase linenumber on backspace CBX
							- no space in linenumber CBX

							- new even linenumber on trigger CBX
							- even linenumber trigger EDIT


							2.SubProg detection

							- sub prog trigger EDIT

							- use different num length in sub prog CBX
							- num step sub  EDIT
							- num lenght sub EDIT

							3.EndProg detection
							- Use Endprog detection CBX
							- EndProgTrigger EDIT

							4.Annotations and Variable statements
							- autocomplete brackets CBX
							- insert spaces CBX
							- use multiline annotations CBX						
						*/

						hr = this->_createSinglelineEditWithIntegerDataBinding(
							syntaxSettings,
							DATAKEY_SETTINGS_AUTOSYNTAX_AUTONUMSTARTLINE,
							UI_PROPWND_AUTONUMSTARTLINE,
							autosyntaxManager::ctrlID_autonumStartline,
							DPIScale(40),
							DPIScale(90),
							2
						);

						if (SUCCEEDED(hr))
						{
							hr = this->_createSinglelineEditWithIntegerDataBinding(
								syntaxSettings,
								DATAKEY_SETTINGS_AUTOSYNTAX_AUTONUMSTEPMAIN,
								UI_PROPWND_NUMSTEPMAIN,
								autosyntaxManager::ctrlID_autoNumStepMain,
								DPIScale(40),
								DPIScale(115),
								5
							);

							if (SUCCEEDED(hr))
							{
								hr = this->_createSinglelineEditWithIntegerDataBinding(
									syntaxSettings,
									DATAKEY_SETTINGS_AUTOSYNTAX_NUMLENGTHMAIN,
									UI_PROPWND_NUMLENGTHMAIN,
									autosyntaxManager::ctrlID_numLenghtMain,
									DPIScale(40),
									DPIScale(140),
									3
								);

								if (SUCCEEDED(hr))
								{
									hr = this->_createSinglelineEditWithIntegerDataBinding(
										syntaxSettings,
										DATAKEY_SETTINGS_AUTOSYNTAX_MINLINENUMBER,
										UI_PROPWND_MINIMUMLINENUMBER,
										autosyntaxManager::ctrlID_minLinenumber,
										DPIScale(40),
										DPIScale(165),
										10
									);

									if (SUCCEEDED(hr))
									{
										hr = this->_createSinglelineEditWithIntegerDataBinding(
											syntaxSettings,
											DATAKEY_SETTINGS_AUTOSYNTAX_MAXLINENUMBER,
											UI_PROPWND_MAXIMUMLINENUMBER,
											autosyntaxManager::ctrlID_maxLinenumber,
											DPIScale(40),
											DPIScale(190),
											9999
										);

										if (SUCCEEDED(hr))
										{
											hr = this->_createCheckboxWithBooleanDataBinding(
												syntaxSettings,
												DATAKEY_SETTINGS_AUTOSYNTAX_ERASELINENUMBER,
												UI_PROPWND_ERASELINENRONBACKSPACE,
												autosyntaxManager::ctrlID_eraseLinenumber,
												DPIScale(40),
												DPIScale(220),
												false
											);
											if (SUCCEEDED(hr))
											{
												hr = this->_createCheckboxWithBooleanDataBinding(
													syntaxSettings,
													DATAKEY_SETTINGS_AUTOSYNTAX_NOSPACEINLINENUMBER,
													UI_PROPWND_NOSPACEINLINENUMBER,
													autosyntaxManager::ctrlID_noSpaceInLinenumber,
													DPIScale(40),
													DPIScale(250),
													false
												);
												if (SUCCEEDED(hr))
												{
													hr = this->_createCheckboxWithBooleanDataBinding(
														syntaxSettings,
														DATAKEY_SETTINGS_AUTOSYNTAX_NEWEVENLINEONTRIGG,
														UI_PROPWND_NEWEVENLINENUMBERRONTRIGGER,
														autosyntaxManager::ctrlID_newEvenLinenumberOnTrigger,
														DPIScale(40),
														DPIScale(290),
														true
													);

													if (SUCCEEDED(hr))
													{
														hr = this->_createSinglelineEditWithStringDataBinding(
															syntaxSettings,
															DATAKEY_SETTINGS_AUTOSYNTAX_EVENLINETRIGGEREDIT,
															autosyntaxManager::ctrlID_evenLinenumberTriggerEdit,
															DPIScale(40),
															DPIScale(320),
															NEWLINE_TRIGGER_DEFAULTSTRING
														);

														if (SUCCEEDED(hr))
														{
															hr = this->_createSinglelineEditWithStringDataBinding(
																syntaxSettings,
																DATAKEY_SETTINGS_AUTOSYNTAX_SUBPROGTRIGGEREDIT,
																autosyntaxManager::ctrlID_subProgTriggerEdit,
																DPIScale(20),
																DPIScale(400),
																SUBPROG_TRIGGER_DEFAULTSTRING
															);

															if (SUCCEEDED(hr))
															{
																hr = this->_createCheckboxWithBooleanDataBinding(
																	syntaxSettings,
																	DATAKEY_SETTINGS_AUTOSYNTAX_USEDIFFNUMLENINSUB,
																	UI_PROPWND_USEDIFFERENTLINENUMBERINSUB,
																	autosyntaxManager::ctrlID_useDifferentLinenumberInSub,
																	DPIScale(40),
																	DPIScale(440),
																	true
																);

																if (SUCCEEDED(hr))
																{
																	hr = this->_createSinglelineEditWithIntegerDataBinding(
																		syntaxSettings,
																		DATAKEY_SETTINGS_AUTOSYNTAX_NUMSTEPSUB,
																		UI_PROPWND_NUMSTEPSUB,
																		autosyntaxManager::ctrlID_numStepSub,
																		DPIScale(80),
																		DPIScale(470),
																		1
																	);

																	if (SUCCEEDED(hr))
																	{
																		hr = this->_createSinglelineEditWithIntegerDataBinding(
																			syntaxSettings,
																			DATAKEY_SETTINGS_AUTOSYNTAX_NUMLENGTHSUB,
																			UI_PROPWND_NUMLENGTHSUB,
																			autosyntaxManager::ctrlID_numLengthSub,
																			DPIScale(80),
																			DPIScale(495),
																			2
																		);

																		if (SUCCEEDED(hr))
																		{
																			hr = this->_createCheckboxWithBooleanDataBinding(
																				syntaxSettings,
																				DATAKEY_SETTINGS_AUTOSYNTAX_USEENDPROGDETECTION,
																				UI_PROPWND_USEENDPROGDETECTION,
																				autosyntaxManager::ctrlID_useEndprogDetection,
																				DPIScale(40),
																				DPIScale(570),
																				true
																			);

																			if (SUCCEEDED(hr))
																			{
																				hr = this->_createSinglelineEditWithStringDataBinding(
																					syntaxSettings,
																					DATAKEY_SETTINGS_AUTOSYNTAX_ENDPROGTRIGGEREDIT,
																					autosyntaxManager::ctrlID_endProgTriggerEdit,
																					DPIScale(60),
																					DPIScale(600),
																					ENDPROG_TRIGGER_DEFAULTSTRING
																				);

																				if (SUCCEEDED(hr))
																				{
																					hr = this->_createCheckboxWithBooleanDataBinding(
																						syntaxSettings,
																						DATAKEY_SETTINGS_AUTOSYNTAX_AUTOCOMPBRACKETS,
																						UI_PROPWND_AUTOCOMPLETEBRACKETS,
																						autosyntaxManager::ctrlID_autoCompleteBrackets,
																						DPIScale(40),
																						DPIScale(680),
																						true
																					);

																					if (SUCCEEDED(hr))
																					{
																						hr = this->_createCheckboxWithBooleanDataBinding(
																							syntaxSettings,
																							DATAKEY_SETTINGS_AUTOSYNTAX_INSERTSPACESINBRAC,
																							UI_PROPWND_INSERTSPACESINAUTOCMPLTBARCKET,
																							autosyntaxManager::ctrlID_insertSpacesInBrackets,
																							DPIScale(40),
																							DPIScale(710),
																							true
																						);

																						if (SUCCEEDED(hr))
																						{
																							hr = this->_createCheckboxWithBooleanDataBinding(
																								syntaxSettings,
																								DATAKEY_SETTINGS_AUTOSYNTAX_USEMULTILINEANNOTNS,
																								UI_PROPWND_USEMULTILINEANNOTATIONS,
																								autosyntaxManager::ctrlID_useMultilineAnnotations,
																								DPIScale(40),
																								DPIScale(740),
																								true
																							);

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

HRESULT autosyntaxManager::_createSinglelineEditWithIntegerDataBinding(ApplicationData* aData, DATAKEY key, int descriptionResourceID, int ctrlID, int x, int y, int defaultValue)
{
	HRESULT hr;

	auto sle = new singleLineEdit(this->hInstance);
	hr = (sle != nullptr) ? S_OK : E_FAIL;
	if(SUCCEEDED(hr))
	{
		iString ctrlText(
			getStringFromResource(descriptionResourceID)
		);
		sle->setType(
			SLE_TYPE_WITHDESCRIPTION,
			DPIScale(300)
		);
		sle->setDimensions(x, y, DPIScale(350));
		sle->setDescriptionText(ctrlText);
		sle->setAdditionalEditStyles(ES_CENTER | ES_AUTOHSCROLL);
		sle->setColors(this->styleInfo.TabColor, this->styleInfo.Stylecolor, this->styleInfo.TextColor, this->styleInfo.TextColor);
		sle->setBorder(true, this->styleInfo.OutlineColor);
		sle->setEventHandler(
			dynamic_cast<singleLineEditEventSink*>(this)
		);
		sle->restrictedContent(SLE_RESTRICTTYPE_ONLYNUMBERS);
		sle->setCtrlFont(
			CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
		);

		this->setValidRangeFromControlID(sle, ctrlID);

		sle->setEditFontProperties(
			APPLICATION_PRIMARY_FONT,
			DPIScale(16),
			DPIScale(25)
		);
		sle->setExtendedFlag(SLE_EX_NOEVENT_ON_INVALIDCONTENT);

		hr = sle->Create(this->ausynWnd, ctrlID);
		if (SUCCEEDED(hr))
		{
			sle->setContentWithEventBlocker(
				iString::fromInt(
					aData->getIntegerData(key, defaultValue)
				)
			);
		}
	}
	return hr;
}

HRESULT autosyntaxManager::_createSinglelineEditWithStringDataBinding(ApplicationData * aData, DATAKEY key, int ctrlID, int x, int y, LPCTSTR defaultValue)
{
	RECT rc;
	HRESULT hr;

	GetClientRect(this->ausynWnd, &rc);

	auto edit_ = new singleLineEdit(this->hInstance);
	hr = (edit_ != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		iString ctrlText(
			getStringFromResource(UI_PROPWND_TRIGGER)
		);
		edit_->setType(
			SLE_TYPE_WITHDESCRIPTION,
			DPIScale(80)
		);
		edit_->setDimensions(x, y, rc.right - DPIScale(100));
		edit_->setDescriptionText(ctrlText);
		edit_->setAdditionalEditStyles(ES_LEFT | ES_AUTOHSCROLL);
		edit_->setColors(this->styleInfo.TabColor, this->styleInfo.Stylecolor, this->styleInfo.TextColor, this->styleInfo.TextColor);
		edit_->setBorder(true, this->styleInfo.OutlineColor);
		edit_->setSpacing(
			DPIScale(10)
		);
		edit_->setAlignment(DESC_ALIGN_RIGHT);
		edit_->setEventHandler(
			dynamic_cast<singleLineEditEventSink*>(this)
		);
		edit_->setCtrlFont(
			CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
		);
		edit_->setEditFontProperties(
			APPLICATION_PRIMARY_FONT,
			DPIScale(16),
			DPIScale(25)
		);

		hr = edit_->Create(this->ausynWnd, ctrlID);
		if (SUCCEEDED(hr))
		{
			iString defaultString(defaultValue);
			auto stringToSet = aData->getStringData(key, defaultString);
			edit_->setContentWithEventBlocker(stringToSet);
		}
	}
	return hr;
}

HRESULT autosyntaxManager::_createCheckboxWithBooleanDataBinding(ApplicationData * aData, DATAKEY key, int UItextResourceID, int ctrlID, int x, int y, bool defaultvalue)
{
	POINT pt;
	SIZE sz;
	RECT rc;
	HRESULT hr;

	GetClientRect(this->ausynWnd, &rc);

	pt.x = x;
	pt.y = y;
	sz.cx = rc.right - (DPIScale(10) + x);
	sz.cy = DPIScale(30);

	iString ctrlText(
		getStringFromResource(UItextResourceID)
	);

	auto cbx = new CustomCheckbox(this->hInstance, this->ausynWnd, &pt, &sz, ctrlID);

	hr = (cbx != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		cbx->setEventHandler(
			dynamic_cast<customCheckboxEventSink*>(this)
		);
		cbx->setText(ctrlText);
		cbx->setChecked(
			aData->getBooleanData(key, defaultvalue)
		);
		cbx->setColors(this->styleInfo.TabColor, this->styleInfo.TextColor);
		cbx->setConstraints(
			DPIScale(10),
			DPIScale(10)
		);
		cbx->setFont(
			CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
		);

		hr = cbx->Create();
	}
	return hr;
}

LRESULT autosyntaxManager::autosynProc(HWND asWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	autosyntaxManager* aManager = nullptr;

	if (message == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		aManager = reinterpret_cast<autosyntaxManager*>(pcr->lpCreateParams);
		SetWindowLongPtr(asWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(aManager));
		return 1;
	}
	else
	{
		aManager = reinterpret_cast<autosyntaxManager*>(GetWindowLongPtr(asWnd, GWLP_USERDATA));
		if (aManager != nullptr)
		{
			switch (message)
			{
			case WM_MOUSEWHEEL:
				return aManager->onMouseWheel(wParam);
			case WM_VSCROLL:
				return aManager->onVScroll(wParam);
			case WM_PAINT:
				return aManager->onPaint(asWnd);
			case WM_CLOSE:
				DestroyWindow(asWnd);
				return 0;
			case WM_DESTROY:			
				if (aManager->syntaxHasChanged)
				{
					// if the setup has changed -> dispatch message to tabcontrol
					PostMessage(_MAIN_WND_, WM_INTERNALCOMMAND, MAKEWPARAM(ICOMMAND_AUTOSYNTAX_DATA_CHANGED, 0), 0);
				}
				// tell the prop-window that the manager is destroyed
				PostMessage(aManager->Parent, WM_COMMAND, MAKEWPARAM(ICOMMAND_AUTOSYNTAXMANGER_CLOSED, 0), 0);
				// release the component
				SafeRelease(&aManager);
				return 0;
			default:
				break;
			}
		}
		return DefWindowProc(asWnd, message, wParam, lParam);
	}
}

LRESULT autosyntaxManager::onPaint(HWND hWnd)
{
	HDC hdc;
	HPEN pen;
	HGDIOBJ original;
	PAINTSTRUCT ps;

	hdc = BeginPaint(hWnd, &ps);
	if (hdc)
	{
		HBRUSH brush = CreateSolidBrush(this->styleInfo.TabColor);
		if (brush)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			FillRect(hdc, &rc, brush);

			TextOutDC(hdc,
				getStringFromResource(UI_PROPWND_LINENUMBERS),
				DPIScale(20),
				DPIScale(60) - this->currentScrollPosition,
				this->ctrlFont,
				this->styleInfo.TextColor
			);
			TextOutDC(hdc,
				getStringFromResource(UI_PROPWND_SUBPROGDETECTION),
				DPIScale(20),
				DPIScale(370) - this->currentScrollPosition,
				this->ctrlFont,
				this->styleInfo.TextColor
			);
			TextOutDC(hdc,
				getStringFromResource(UI_PROPWND_ENDPROGDETECTION),
				DPIScale(20),
				DPIScale(540) - this->currentScrollPosition,
				this->ctrlFont,
				this->styleInfo.TextColor
			);

			pen = CreatePen(PS_SOLID, 1, this->styleInfo.TextColor);
			if (pen)
			{
				original = SelectObject(hdc, pen);

				MoveToEx(
					hdc,
					DPIScale(20),
					DPIScale(80) - this->currentScrollPosition,
					nullptr
				);
				LineTo(
					hdc,
					rc.right - DPIScale(30),
					DPIScale(80) - this->currentScrollPosition
				);

				MoveToEx(
					hdc,
					DPIScale(20),
					DPIScale(390) - this->currentScrollPosition,
					nullptr
				);
				LineTo(
					hdc,
					rc.right - DPIScale(30),
					DPIScale(390) - this->currentScrollPosition
				);

				MoveToEx(
					hdc,
					DPIScale(20),
					DPIScale(560) - this->currentScrollPosition,
					nullptr
				);
				LineTo(
					hdc,
					rc.right - DPIScale(30),
					DPIScale(560) - this->currentScrollPosition
				);

				SelectObject(hdc, original);
				DeleteObject(pen);
			}

			DeleteObject(brush);
		}

		if (this->currentScrollPosition > 0)
		{
			this->redrawVolatileWindowPortion(hdc, this->currentScrollPosition);
		}

		EndPaint(hWnd, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT autosyntaxManager::onVScroll(WPARAM wParam)
{
	SCROLLINFO sci;
	sci.cbSize = sizeof(SCROLLINFO);
	sci.fMask = SIF_ALL;
	GetScrollInfo(this->ausynWnd, SB_VERT, &sci);

	int yPos = sci.nPos;

	switch (LOWORD(wParam))
	{
	case SB_LINEDOWN:
		sci.nPos += DPIScale(16);
		break;
	case SB_LINEUP:
		sci.nPos -= DPIScale(16);
		break;
	case SB_PAGEDOWN:
		sci.nPos = sci.nMax;
		break;
	case SB_PAGEUP:
		sci.nPos = sci.nMin;
		break;
	case SB_THUMBTRACK:
		sci.nPos = sci.nTrackPos;
		if (!this->msgBlocker) // make sure the message is only sent once
		{
			SendMessage(this->Parent, WM_COMMAND, MAKEWPARAM(ICOMMAND_SETCANCLOSE, FALSE), 0);
			this->msgBlocker = true;
		}
		break;
	case SB_THUMBPOSITION:
		SendMessage(this->Parent, WM_COMMAND, MAKEWPARAM(ICOMMAND_SETCANCLOSE, TRUE), 0);
		this->msgBlocker = false;
		break;
	default:
		break;
	}

	sci.fMask = SIF_POS;
	SetScrollInfo(this->ausynWnd, SB_VERT, &sci, TRUE);
	GetScrollInfo(this->ausynWnd, SB_VERT, &sci);

	this->currentScrollPosition = sci.nPos;

	if (sci.nPos != yPos)
	{
		ScrollWindow(this->ausynWnd, 0, yPos - sci.nPos, nullptr, nullptr);
		redrawVolatileWindowPortion(nullptr, sci.nPos);
	}
	return static_cast<LRESULT>(0);
}

LRESULT autosyntaxManager::onMouseWheel(WPARAM wParam)
{
	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

	if (zDelta > 0)
		this->onVScroll(
			MAKEWPARAM(SB_LINEUP, 0)
		);
	else
		this->onVScroll(
			MAKEWPARAM(SB_LINEDOWN, 0)
		);

	return static_cast<LRESULT>(0);
}

void autosyntaxManager::onButtonClick(CustomButton * cButton, CTRLID ctrlID)
{
	UNREFERENCED_PARAMETER(cButton);

	switch (ctrlID)
	{
	case autosyntaxManager::ctrlID_exitButton:
		this->Close();
		break;
	default:
		break;
	}
}

void autosyntaxManager::onCheckboxChecked(CustomCheckbox * cCheckbox, bool _newState)
{
	this->syntaxHasChanged = true;

	auto syntaxSettings =
		reinterpret_cast<ApplicationData*>(
			getApplicationDataContainerFromFilekey(FILEKEY_AUTOSYNTAX_SETTINGS)
		);

	if (syntaxSettings != nullptr)
	{
		switch (cCheckbox->getCtrlID())
		{
		case autosyntaxManager::ctrlID_isOnCheckbox:
			syntaxSettings->saveValue(DATAKEY_SETTINGS_AUTOSYNTAX_ISON, _newState);
			break;
		case autosyntaxManager::ctrlID_autoCompleteBrackets:
			syntaxSettings->saveValue(DATAKEY_SETTINGS_AUTOSYNTAX_AUTOCOMPBRACKETS, _newState);
			break;
		case autosyntaxManager::ctrlID_eraseLinenumber:
			syntaxSettings->saveValue(DATAKEY_SETTINGS_AUTOSYNTAX_ERASELINENUMBER, _newState);
			break;
		case autosyntaxManager::ctrlID_insertSpacesInBrackets:
			syntaxSettings->saveValue(DATAKEY_SETTINGS_AUTOSYNTAX_INSERTSPACESINBRAC, _newState);
			break;
		case autosyntaxManager::ctrlID_newEvenLinenumberOnTrigger:
			syntaxSettings->saveValue(DATAKEY_SETTINGS_AUTOSYNTAX_NEWEVENLINEONTRIGG, _newState);
			break;
		case autosyntaxManager::ctrlID_noSpaceInLinenumber:
			syntaxSettings->saveValue(DATAKEY_SETTINGS_AUTOSYNTAX_NOSPACEINLINENUMBER, _newState);
			break;
		case autosyntaxManager::ctrlID_useDifferentLinenumberInSub:
			syntaxSettings->saveValue(DATAKEY_SETTINGS_AUTOSYNTAX_USEDIFFNUMLENINSUB, _newState);
			break;
		case autosyntaxManager::ctrlID_useEndprogDetection:
			syntaxSettings->saveValue(DATAKEY_SETTINGS_AUTOSYNTAX_USEENDPROGDETECTION, _newState);
			break;
		case autosyntaxManager::ctrlID_useMultilineAnnotations:
			syntaxSettings->saveValue(DATAKEY_SETTINGS_AUTOSYNTAX_USEMULTILINEANNOTNS, _newState);
			break;
		default:
			break;
		}
	}
}

void autosyntaxManager::onEditContentChanged(singleLineEdit * edit, CTRLID ctrlID)
{
	this->syntaxHasChanged = true;

	auto syntaxSettings =
		reinterpret_cast<ApplicationData*>(
			getApplicationDataContainerFromFilekey(FILEKEY_AUTOSYNTAX_SETTINGS)
			);

	if (syntaxSettings != nullptr)
	{
		switch (ctrlID)
		{
		case autosyntaxManager::ctrlID_autonumStartline:
			syntaxSettings->saveValue(
				DATAKEY_SETTINGS_AUTOSYNTAX_AUTONUMSTARTLINE,
				edit->getContent()
				.getAsInt()
			);
			break;
		case autosyntaxManager::ctrlID_autoNumStepMain:
			syntaxSettings->saveValue(
				DATAKEY_SETTINGS_AUTOSYNTAX_AUTONUMSTEPMAIN,
				edit->getContent()
				.getAsInt()
			);
			break;
		case autosyntaxManager::ctrlID_numStepSub:
			syntaxSettings->saveValue(
				DATAKEY_SETTINGS_AUTOSYNTAX_NUMSTEPSUB,
				edit->getContent()
				.getAsInt()
			);
			break;
		case autosyntaxManager::ctrlID_endProgTriggerEdit:
			syntaxSettings->saveValue(
				DATAKEY_SETTINGS_AUTOSYNTAX_ENDPROGTRIGGEREDIT,
				edit->getContent()
			);
			break;
		case autosyntaxManager::ctrlID_evenLinenumberTriggerEdit:
			syntaxSettings->saveValue(
				DATAKEY_SETTINGS_AUTOSYNTAX_EVENLINETRIGGEREDIT,
				edit->getContent()
			);
			break;
		case autosyntaxManager::ctrlID_maxLinenumber:
			syntaxSettings->saveValue(
				DATAKEY_SETTINGS_AUTOSYNTAX_MAXLINENUMBER,
				edit->getContent()
				.getAsInt()
			);
			break;
		case autosyntaxManager::ctrlID_minLinenumber:
			syntaxSettings->saveValue(
				DATAKEY_SETTINGS_AUTOSYNTAX_MINLINENUMBER,
				edit->getContent()
				.getAsInt()
			);
			break;
		case autosyntaxManager::ctrlID_numLenghtMain:
			syntaxSettings->saveValue(
				DATAKEY_SETTINGS_AUTOSYNTAX_NUMLENGTHMAIN,
				edit->getContent()
				.getAsInt()
			);
			break;
		case autosyntaxManager::ctrlID_numLengthSub:
			syntaxSettings->saveValue(
				DATAKEY_SETTINGS_AUTOSYNTAX_NUMLENGTHSUB,
				edit->getContent()
				.getAsInt()
			);
			break;
		case autosyntaxManager::ctrlID_subProgTriggerEdit:
			syntaxSettings->saveValue(
				DATAKEY_SETTINGS_AUTOSYNTAX_SUBPROGTRIGGEREDIT,
				edit->getContent()
			);
			break;
		default:
			break;
		}
	}
}

void autosyntaxManager::redrawVolatileWindowPortion(HDC hdcExtern, int scrollPosition)
{
	HDC hdc = (hdcExtern == nullptr) ? GetDC(this->ausynWnd) : hdcExtern;
	if (hdc)
	{
		RECT rc;
		GetClientRect(this->ausynWnd, &rc);

		HBRUSH brush = CreateSolidBrush(this->styleInfo.TabColor);
		if (brush)
		{
			int yLinenumberText = DPIScale(60) - scrollPosition;
			int yAnnotationText = DPIScale(650) - scrollPosition;

			if (yLinenumberText > -(DPIScale(20)))
			{
				RECT fRect;
				SetRect(&fRect, 0, yLinenumberText - DPIScale(1), rc.right, yLinenumberText + DPIScale(41));
				FillRect(hdc, &fRect, brush);

				TextOutDC(
					hdc,
					getStringFromResource(UI_PROPWND_LINENUMBERS),
					DPIScale(20),
					yLinenumberText,
					this->ctrlFont,
					this->styleInfo.TextColor
				);

				HPEN pen = CreatePen(PS_SOLID, 1, this->styleInfo.TextColor);
				if (pen)
				{
					HGDIOBJ original = SelectObject(hdc, pen);

					MoveToEx(hdc, DPIScale(20), DPIScale(80) - scrollPosition, nullptr);
					LineTo(hdc, rc.right - DPIScale(30), DPIScale(80) - scrollPosition);

					SelectObject(hdc, original);
					DeleteObject(pen);
				}
			}
			if (yAnnotationText < DPIScale(600))
			{
				RECT fRect;
				SetRect(&fRect, 0, yAnnotationText - DPIScale(1), rc.right, yAnnotationText + DPIScale(21));
				FillRect(hdc, &fRect, brush);

				TextOutDC(
					hdc,
					getStringFromResource(UI_PROPWND_ANNOTATIONANDVARIABLESTATEMENTS),
					DPIScale(20),
					yAnnotationText,
					this->ctrlFont,
					this->styleInfo.TextColor
				);

				HPEN pen = CreatePen(PS_SOLID, 1, this->styleInfo.TextColor);
				if (pen)
				{
					HGDIOBJ original = SelectObject(hdc, pen);

					MoveToEx(hdc, DPIScale(20), DPIScale(670) - scrollPosition, nullptr);
					LineTo(hdc, rc.right - DPIScale(30), DPIScale(670) - scrollPosition);

					SelectObject(hdc, original);
					DeleteObject(pen);
				}

			}
			DeleteObject(brush);
		}
		if(hdcExtern == nullptr)
			ReleaseDC(this->ausynWnd, hdc);
	}
}

void autosyntaxManager::setValidRangeFromControlID(singleLineEdit * Sle, int ctrlID)
{
	if (Sle != nullptr)
	{
		switch (ctrlID)
		{
		case autosyntaxManager::ctrlID_autonumStartline:
			Sle->setValidContentRange(1, 100000);
			break;
		case autosyntaxManager::ctrlID_autoNumStepMain:
			Sle->setValidContentRange(1, 1000);
			break;
		case autosyntaxManager::ctrlID_maxLinenumber:
			Sle->setValidContentRange(1, 1000000);
			break;
		case autosyntaxManager::ctrlID_minLinenumber:
			Sle->setValidContentRange(1, 1000000);
			break;
		case autosyntaxManager::ctrlID_numLenghtMain:
			Sle->setValidContentRange(1, 10);
			break;
		case autosyntaxManager::ctrlID_numLengthSub:
			Sle->setValidContentRange(1, 10);
			break;
		case autosyntaxManager::ctrlID_numStepSub:
			Sle->setValidContentRange(1, 1000);
			break;
		default:
			break;
		}
	}
}
