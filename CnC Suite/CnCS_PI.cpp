#include "CnCS_PI.h"
#include"HelperF.h"
#include"AppPath.h"
#include"ApplicationData.h"
#include"ColorSchemeManager.h"
#include"AutocompleteManager.h"
#include"CnCSuite_FileNavigator.h"
#include"Searchcontrol\SearchControl.h"
#include"AutosyntaxManager.h"
#include"DPI_Helper.h"
#include"CommonControls\textField.h"
#include"ResetDialog.h"
#include"splashScreen.h"
#include"history.h"
#include"Url.h"

CnCS_PI::CnCS_PI()
	: Success(TRUE), propWnd(nullptr), hInstance(nullptr), wki(nullptr)
{
	SecureZeroMemory(&this->sInfo, sizeof(APPSTYLEINFO));
	SecureZeroMemory(&this->menuButtons, sizeof(CNCSPROP_MENUBUTTONS));
	SecureZeroMemory(&this->iParam, sizeof(CNCSPROP_PARAMS));
	this->setInitialControlParameter();

	this->_createDpiDependendResources();	
}

CnCS_PI::~CnCS_PI()
{
	// delete objects...???
	// unhook mouse hook
}

HRESULT CnCS_PI::_createPropertyWindow(HWND Main)
{
	HRESULT hr;

	hr = ((!this->Success) || (!Main) || (!this->hInstance))
		? E_INVALIDARG : S_OK;

	if (SUCCEEDED(hr))
	{
		WNDCLASSEX wcx;

		if (!GetClassInfoEx(this->hInstance, CS_PROPERTYWINDOW_CLASS, &wcx))
		{
			wcx.cbSize = sizeof(WNDCLASSEX);
			wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
			wcx.cbClsExtra = 0;
			wcx.cbWndExtra = sizeof(LONG_PTR);
			wcx.lpfnWndProc = CnCS_PI::pWProc;
			wcx.hInstance = this->hInstance;
			wcx.hIcon = nullptr;
			wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wcx.hbrBackground = nullptr;
			wcx.lpszClassName = CS_PROPERTYWINDOW_CLASS;
			wcx.lpszMenuName = nullptr;
			wcx.hIconSm = nullptr;

			hr = (RegisterClassEx(&wcx) != 0) ? S_OK : E_FAIL;
		}
		if (SUCCEEDED(hr))
		{
			RECT rc;
			GetWindowRect(Main, &rc);

			hr = SendMessage(Main, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&this->sInfo)) ? S_OK : E_FAIL;

			if (SUCCEEDED(hr))
			{
				this->createStyleParams();
				this->registerPageWndClass();

				this->propWnd
					= CreateWindow(
						CS_PROPERTYWINDOW_CLASS,
						nullptr,
						WS_POPUP | WS_BORDER,
						rc.left,
						rc.top + DPIScale(26),
						DPIScale(800),
						DPIScale(600),
						Main, nullptr,
						this->hInstance,
						reinterpret_cast<LPVOID>(this)
					);

				hr = (this->propWnd != nullptr) ? S_OK : E_FAIL;

				if (SUCCEEDED(hr))
				{
					hr = this->_createFrameControls();

					if (SUCCEEDED(hr))
					{
						hr = this->showPage(PAGE_ID_GENERAL);

						// set first menu selection
						this->menuButtons.settingsButton->setButtonDown();

						if (SUCCEEDED(hr))
						{
							auto threadId = GetCurrentThreadId();
							this->iParam.mouseHook
								= SetWindowsHookEx(WH_MOUSE, CnCS_PI::mouseProc, this->hInstance, threadId);

							ShowWindow(this->propWnd, SW_SHOW);
							UpdateWindow(this->propWnd);

							SendMessage(this->mainWnd, WM_BLOCKINPUT, (WPARAM)TRUE, 0);
						}
					}
				}
			}
		}
	}
	return hr;
}

HRESULT CnCS_PI::_createFrameControls()
{
	HRESULT hr;

	RECT rc;
	GetClientRect(this->propWnd, &rc);

	POINT pt;
	pt.x = rc.right - DPIScale(32);
	pt.y = DPIScale(4);

	SIZE sz;
	sz.cx = DPIScale(28);
	sz.cy = DPIScale(28);

	this->menuButtons.exitButton = new CustomButton(this->propWnd, BUTTONMODE_ICON, &pt, &sz, IDMM_PROPWINDOWEXIT, this->hInstance);

	hr = (this->menuButtons.exitButton != nullptr) ? S_OK : E_FAIL;
	if(SUCCEEDED(hr))
	{
		this->menuButtons.exitButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
		this->menuButtons.exitButton->setAppearance_onlyIcon(IDI_GNRL_CANCEL_YELLOW, 24);
		this->menuButtons.exitButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);

		hr = this->menuButtons.exitButton->Create();
		if (SUCCEEDED(hr))
		{
			pt.x = 0;
			pt.y = DPIScale(100);
			sz.cx = DPIScale(200);
			sz.cy = DPIScale(40);

			this->menuButtons.settingsButton = new CustomButton(this->propWnd, BUTTONMODE_ICONTEXT, &pt, &sz, IDMM_MENUSETTINGS, this->hInstance);
			
			hr = (this->menuButtons.settingsButton != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				iString text(
					getStringFromResource(UI_PROPWND_SETTINGSBUTTON)
				);

				this->defineMenuButton(this->menuButtons.settingsButton, IDI_PROP_SETTINGS, text);

				hr = this->menuButtons.settingsButton->Create();
				if (SUCCEEDED(hr))
				{
					pt.y = DPIScale(140);
					
					this->menuButtons.tabctrlButton = new CustomButton(this->propWnd, BUTTONMODE_ICONTEXT, &pt, &sz, IDMM_MENUTABCTRL, this->hInstance);

					hr = (this->menuButtons.tabctrlButton != nullptr) ? S_OK : E_FAIL;
					if (SUCCEEDED(hr))
					{
						text.Replace(
							getStringFromResource(UI_PROPWND_TABCTRLBUTTON)
						);

						this->defineMenuButton(this->menuButtons.tabctrlButton, IDI_PROP_TABCTRL, text);

						hr = this->menuButtons.tabctrlButton->Create();
						if (SUCCEEDED(hr))
						{
							pt.y = DPIScale(180);

							this->menuButtons.filenavButton = new CustomButton(this->propWnd, BUTTONMODE_ICONTEXT, &pt, &sz, IDMM_MENUFILENAV, this->hInstance);

							hr = (this->menuButtons.filenavButton != nullptr) ? S_OK : E_FAIL;
							if (SUCCEEDED(hr))
							{
								text.Replace(
									getStringFromResource(UI_PROPWND_FILENAVIGATOR)
								);

								this->defineMenuButton(this->menuButtons.filenavButton, IDI_PROP_FILENAV, text);

								hr = this->menuButtons.filenavButton->Create();
								if (SUCCEEDED(hr))
								{
									pt.y = DPIScale(220);

									this->menuButtons.editorButton = new CustomButton(this->propWnd, BUTTONMODE_ICONTEXT, &pt, &sz, IDMM_MENUEDITOR, this->hInstance);

									hr = (this->menuButtons.editorButton != nullptr) ? S_OK : E_FAIL;
									if (SUCCEEDED(hr))
									{
										text.Replace(
											getStringFromResource(UI_PROPWND_EDITOR)
										);

										this->defineMenuButton(this->menuButtons.editorButton, IDI_PROP_EDITOR, text);

										hr = this->menuButtons.editorButton->Create();
										if (SUCCEEDED(hr))
										{
											pt.y = DPIScale(260);

											this->menuButtons.exchangeButton = new CustomButton(this->propWnd, BUTTONMODE_ICONTEXT, &pt, &sz, IDMM_MENUEXCHANGE, this->hInstance);

											hr = (this->menuButtons.exchangeButton != nullptr) ? S_OK : E_FAIL;
											if (SUCCEEDED(hr))
											{
												text.Replace(
													getStringFromResource(UI_PROPWND_EXCHANGE)
												);

												this->defineMenuButton(this->menuButtons.exchangeButton, IDI_PROP_EXCHANGE, text);

												hr = this->menuButtons.exchangeButton->Create();
												if (SUCCEEDED(hr))
												{
													pt.y = DPIScale(300);

													this->menuButtons.infoButton = new CustomButton(this->propWnd, BUTTONMODE_ICONTEXT, &pt, &sz, IDMM_MENUINFO, this->hInstance);

													hr = (this->menuButtons.infoButton != nullptr) ? S_OK : E_FAIL;
													if (SUCCEEDED(hr))
													{
														text.Replace(
															getStringFromResource(UI_PROPWND_INFO)
														);

														this->defineMenuButton(this->menuButtons.infoButton, IDI_PROP_INFO, text);

														hr = this->menuButtons.infoButton->Create();
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
	return hr;
}

HRESULT CnCS_PI::_createPropWindowPage(int pageId)
{
	switch (pageId)
	{
	case PAGE_ID_GENERAL:
		return this->_createGeneralPage();
	case PAGE_ID_TABCTRL:
		return this->_createTabCtrlPage();
	case PAGE_ID_FILENAV:
		return this->_createFileNavPage();
	case PAGE_ID_EDITOR:
		return this->_createEditorPage();
	case PAGE_ID_EXCHANGE:
		return this->_createDataExPage();
	case PAGE_ID_INFO:
		return this->_createInfoPage();
	default:
		return E_FAIL;
	}
}

HRESULT CnCS_PI::_createGeneralPage()
{
	HRESULT hr;

	RECT rc;
	GetClientRect(this->propWnd, &rc);

	this->iParam.generalPage = CreateWindow(
		CNCSPROP_PAGE_CLASS,
		nullptr,
		WS_CHILD | WS_VISIBLE,
		DPIScale(210),
		DPIScale(10),
		rc.right - DPIScale(250),
		rc.bottom - DPIScale(20),
		this->propWnd,
		reinterpret_cast<HMENU>(PAGE_ID_GENERAL),
		this->hInstance,
		reinterpret_cast<LPVOID>(this)
	);

	hr = (this->iParam.generalPage != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		GetClientRect(this->iParam.generalPage, &rc);

		auto dataContainer =
			reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer()
			);
		auto extendedDataContainer =
			reinterpret_cast<ApplicationData*>(
				getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
			);

		if ((dataContainer == nullptr) || (extendedDataContainer == nullptr))
			return E_POINTER;

		// language selection >>
		iString syslang(getStringFromResource(UI_PROPWND_SYSTEMLANGUAGE));
		iString english(getStringFromResource(UI_PROPWND_ENGLISH));
		iString german(getStringFromResource(UI_PROPWND_GERMAN));
		auto langCombo =
			new comboBox(
				this->hInstance,
				this->iParam.generalPage,
				COMBOTYPE_DROPDOWNLIST,
				CTRLID_LANGCOMBO,
				DPIScale(30),
				DPIScale(45),
				DPIScale(250),
				0
			);
		if (langCombo != nullptr)
		{
			langCombo->setFont(
				CreateScaledFont(18, FW_NORMAL, APPLICATION_PRIMARY_FONT)
			);

			langCombo->Items->AddItem(syslang);
			langCombo->Items->AddItem(english);
			langCombo->Items->AddItem(german);
			langCombo->setSelectedIndex(
				dataContainer->getIntegerData(DATAKEY_SETTINGS_LANGUAGE, 0));
			langCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
		}
		else
			hr = E_FAIL;

		// design selection
		iString black(getStringFromResource(UI_PROPWND_DSG_BLACK));
		iString grey(getStringFromResource(UI_PROPWND_DSG_GREY));
		iString light(getStringFromResource(UI_PROPWND_DSG_LIGHT));
		//iString green(getStringFromResource(UI_PROPWND_DSG_GREEN));

		auto designCombo = new comboBox(
			this->hInstance,
			this->iParam.generalPage,
			COMBOTYPE_DROPDOWNLIST,
			CTRLID_DESIGNCOMBO,
			DPIScale(30),
			DPIScale(125),
			DPIScale(250),
			0
		);
		if (designCombo != nullptr)
		{
			designCombo->setFont(
				CreateScaledFont(18, FW_NORMAL, APPLICATION_PRIMARY_FONT)
			);

			designCombo->Items->AddItem(black);
			designCombo->Items->AddItem(grey);
			designCombo->Items->AddItem(light);
			//designCombo->Items->AddItem(green);
			designCombo->setSelectedIndex(
				dataContainer->getIntegerData(DATAKEY_SETTINGS_DESIGN, 0));
			designCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
		}
		else
			hr = E_FAIL;

		// update search checkbox
		POINT pt;
		pt.x = DPIScale(20);
		pt.y = DPIScale(210);

		SIZE sz;
		sz.cx = DPIScale(200);//rc.right - DPIScale(50);
		sz.cy = DPIScale(30);

		iString cb_text(
			getStringFromResource(UI_PROPWND_AUTOUPDATESEARCH)
		);

		auto updateCheckbox = new CustomCheckbox(this->hInstance, this->iParam.generalPage, &pt, &sz, CTRLID_UPDATECHECKBOX);
		if (updateCheckbox != nullptr)
		{
			updateCheckbox->setChecked(
				dataContainer->getBooleanData(DATAKEY_SETTINGS_SEARCHFORUPDATES, true));
			updateCheckbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
			updateCheckbox->setText(cb_text);
			updateCheckbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
			updateCheckbox->setConstraints(10, 10);
			updateCheckbox->setFont(
				CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
			);
			updateCheckbox->Create();

			__pragma(warning(push));
			__pragma(warning(disable: 4127));

			if (AppType == APPTYPE_STOREAPP)
				updateCheckbox->setEnabledState(false);

			__pragma(warning(pop));
		}
		else
			hr = E_FAIL;

		pt.x = rc.right - DPIScale(240);
		sz.cx = DPIScale(180);
		sz.cy = DPIScale(30);

		auto updateButton = new CustomButton(this->iParam.generalPage, BUTTONMODE_ICONTEXT, &pt, &sz, CTRLID_UPDATEBUTTON, this->hInstance);
		if (updateButton != nullptr)
		{
			iString button_text(getStringFromResource(UI_PROPWND_SEARCHFORUPDATESNOW));

			updateButton->setAppearance_IconText(IDI_GNRL_SYNC, DPIScale(24), button_text);
			updateButton->setEventListener(
				dynamic_cast<customButtonEventSink*>(this)
			);
			updateButton->setColors(
				this->sInfo.TabColor,
				makeSelectionColor(this->sInfo.TabColor),
				makePressedColor(this->sInfo.TabColor)
			);
			updateButton->setTextColor(this->sInfo.TextColor);
			updateButton->setBorder(TRUE, this->sInfo.OutlineColor);
			updateButton->setDisabledIcon(IDI_PROP_UPDATEARROWS_DSBL);
			updateButton->setDisabledColor(this->sInfo.TabColor);
			updateButton->setConstraints(5, 10);
			updateButton->setFont(
				CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
			);
			updateButton->Create();

			__pragma(warning(push));
			__pragma(warning(disable: 4127));

			if (AppType == APPTYPE_STOREAPP)
				updateButton->setEnabledState(false);

			__pragma(warning(pop));
		}
		else
			hr = E_FAIL;

		pt.x = DPIScale(20);
		pt.y = DPIScale(300);
		sz.cx = rc.right - DPIScale(50);
		sz.cy = DPIScale(30);

		cb_text.Replace(
			getStringFromResource(UI_PROPWND_SAVEEXPLORERCOND)
		);

		// save options checkboxes
		auto sExplorerCheckbox = new CustomCheckbox(this->hInstance, this->iParam.generalPage, &pt, &sz, CTRLID_SAVEEXPLORERCBX);
		if (sExplorerCheckbox != nullptr)
		{
			sExplorerCheckbox->setChecked(
				dataContainer->getBooleanData(DATAKEY_SETTINGS_SAVE_EXPLORER_COND, true)
			);
				
			sExplorerCheckbox->setEventHandler(
				dynamic_cast<customCheckboxEventSink*>(this)
			);
			sExplorerCheckbox->setText(cb_text);
			sExplorerCheckbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
			sExplorerCheckbox->setConstraints(10, 10);
			sExplorerCheckbox->setFont(
				CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
			);
			sExplorerCheckbox->Create();
		}
		pt.y = DPIScale(340);

		cb_text.Replace(
			getStringFromResource(UI_PROPWND_SAVETABWNDCOND)
		);

		auto sTabcondCheckbox = new CustomCheckbox(this->hInstance, this->iParam.generalPage, &pt, &sz, CTRLID_SAVETABWNDCONDCBX);
		if (sTabcondCheckbox != nullptr)
		{
			sTabcondCheckbox->setChecked(
				dataContainer->getBooleanData(DATAKEY_SETTINGS_SAVE_TABWND_COND, true)
			);
			sTabcondCheckbox->setEventHandler(
				dynamic_cast<customCheckboxEventSink*>(this)
			);
			sTabcondCheckbox->setText(cb_text);
			sTabcondCheckbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
			sTabcondCheckbox->setConstraints(10, 10);
			sTabcondCheckbox->setFont(
				CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
			);
			sTabcondCheckbox->Create();
		}
		else
			hr = E_FAIL;

		pt.y = DPIScale(380);

		cb_text.Replace(
			getStringFromResource(UI_PROPWND_SAVEUNSAVEDCONTENT)
		);

		auto sUnsavedCheckbox = new CustomCheckbox(this->hInstance, this->iParam.generalPage, &pt, &sz, CTRLID_SAVEUNSAVEDCBX);
		if (sUnsavedCheckbox != nullptr)
		{
			sUnsavedCheckbox->setChecked(
				dataContainer->getBooleanData(DATAKEY_SETTINGS_SAVE_UNSAVED_CONTENT, false)
			);
			sUnsavedCheckbox->setEventHandler(
				dynamic_cast<customCheckboxEventSink*>(this)
			);
			sUnsavedCheckbox->setText(cb_text);
			sUnsavedCheckbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
			sUnsavedCheckbox->setConstraints(10, 10);
			sUnsavedCheckbox->setFont(
				CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
			);
			sUnsavedCheckbox->Create();
		}
		else
			hr = E_FAIL;

		pt.y = DPIScale(470);
		sz.cx = DPIScale(200);

		cb_text.Replace(
			getStringFromResource(UI_PROPWND_SAVEHISTORY)
		);

		auto saveHistoryCheckbox = new CustomCheckbox(this->hInstance, this->iParam.generalPage, &pt, &sz, CTRLID_SAVEHISTORY);
		if (saveHistoryCheckbox != nullptr)
		{
			saveHistoryCheckbox->setChecked(
				extendedDataContainer->getBooleanData(DATAKEY_EXSETTINGS_HISTORY_SAVEHISTORY, true)
			);
			saveHistoryCheckbox->setEventHandler(
				dynamic_cast<customCheckboxEventSink*>(this)
			);
			saveHistoryCheckbox->setText(cb_text);
			saveHistoryCheckbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
			saveHistoryCheckbox->setConstraints(10, 10);
			saveHistoryCheckbox->setFont(
				CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
			);
			saveHistoryCheckbox->Create();
		}
		else
			hr = E_FAIL;

		pt.x = rc.right - DPIScale(240);
		sz.cx = DPIScale(180);
		sz.cy = DPIScale(30);

		auto deleteHistoryButton = new CustomButton(this->iParam.generalPage, BUTTONMODE_ICONTEXT, &pt, &sz, CTRLID_DELETEHISTORYNOW, this->hInstance);
		if (deleteHistoryButton != nullptr)
		{
			iString button_text(
				getStringFromResource(UI_PROPWND_DELETEHISTORYNOW)
			);
			deleteHistoryButton->setBorder(TRUE, this->sInfo.OutlineColor);
			deleteHistoryButton->setAppearance_IconText(IDI_EXP_DELETEELEMENT, DPIScale(24), button_text);
			deleteHistoryButton->setEventListener(
				dynamic_cast<customButtonEventSink*>(this)
			);
			deleteHistoryButton->setColors(
				this->sInfo.TabColor,
				makeSelectionColor(this->sInfo.TabColor),
				makePressedColor(this->sInfo.TabColor)
			);
			deleteHistoryButton->setTextColor(this->sInfo.TextColor);
			deleteHistoryButton->setConstraints(5, 10);
			deleteHistoryButton->setFont(
				CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
			);
			deleteHistoryButton->Create();

		}
		else
			hr = E_FAIL;

	}
	return hr;
}

HRESULT CnCS_PI::_createTabCtrlPage()
{
	HRESULT hr;
	DESCRIPTIONINFO dInfo;
	SecureZeroMemory(&dInfo, sizeof(DESCRIPTIONINFO));

	SendMessage(this->mainWnd, WM_GETDESCRIPTIONS, 0, reinterpret_cast<LPARAM>(&dInfo));

	RECT rc;
	GetClientRect(this->propWnd, &rc);

	this->iParam.tabctrlPage = CreateWindow(
		CNCSPROP_PAGE_CLASS,
		nullptr,
		WS_CHILD | WS_VISIBLE,
		DPIScale(210),
		DPIScale(10),
		rc.right - DPIScale(250),
		rc.bottom - DPIScale(20),
		this->propWnd,
		reinterpret_cast<HMENU>(PAGE_ID_TABCTRL),
		this->hInstance,
		reinterpret_cast<LPVOID>(this)
	);

	hr = (this->iParam.tabctrlPage != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		GetClientRect(this->iParam.tabctrlPage, &rc);

		auto dataContainer =
			reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer()
			);
		auto extendedDataContainer =
			reinterpret_cast<ApplicationData*>(
				getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
			);

		hr = ((dataContainer != nullptr) && (extendedDataContainer != nullptr)) ? S_OK : E_UNEXPECTED;
		if (SUCCEEDED(hr))
		{
			// autosave checkbox
			POINT pt;
			pt.x = DPIScale(20);
			pt.y = DPIScale(20);
			SIZE sz;
			sz.cx = rc.right - DPIScale(50);
			sz.cy = DPIScale(30);
			iString cb_text(
				getStringFromResource(UI_PROPWND_AUTOSAVE));

			auto autosaveCheckbox = new CustomCheckbox(this->hInstance, this->iParam.tabctrlPage, &pt, &sz, CTRLID_AUTOSAVECBX);
			if (autosaveCheckbox != nullptr)
			{
				autosaveCheckbox->setChecked(
					dataContainer->getBooleanData(DATAKEY_SETTINGS_AUTOSAVE, false));
				autosaveCheckbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
				autosaveCheckbox->setText(cb_text);
				autosaveCheckbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
				autosaveCheckbox->setConstraints(10, 10);
				autosaveCheckbox->setFont(
					CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
				);

				hr = autosaveCheckbox->Create();
			}
			else
				hr = E_FAIL;

			if (SUCCEEDED(hr))
			{
				pt.y += DPIScale(40);
				cb_text.Replace(
					getStringFromResource(UI_PROPWND_OPENNEWFILEINNEWTAB));

				auto newtabCheckbox = new CustomCheckbox(this->hInstance, this->iParam.tabctrlPage, &pt, &sz, CTRLID_NEWTABCBX);
				if (newtabCheckbox != nullptr)
				{
					newtabCheckbox->setChecked(
						dataContainer->getBooleanData(DATAKEY_SETTINGS_OPEN_IN_NEW_TAB, true)
					);
					newtabCheckbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
					newtabCheckbox->setText(cb_text);
					newtabCheckbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
					newtabCheckbox->setConstraints(10, 10);
					newtabCheckbox->setFont(
						CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
					);

					hr = newtabCheckbox->Create();
				}
				else
					hr = E_FAIL;

				if (SUCCEEDED(hr))
				{
					pt.y += DPIScale(40);
					cb_text.Replace(
						getStringFromResource(UI_PROPWND_SHOWDOCPROPERTIES));

					auto dPropWndCheckbox = new CustomCheckbox(this->hInstance, this->iParam.tabctrlPage, &pt, &sz, CTRLID_SHOWPROPWNDCBX);
					if (dPropWndCheckbox != nullptr)
					{
						dPropWndCheckbox->setChecked(
							dataContainer->getBooleanData(DATAKEY_SETTINGS_SHOW_DESCWND, true)
						);
						dPropWndCheckbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
						dPropWndCheckbox->setText(cb_text);
						dPropWndCheckbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
						dPropWndCheckbox->setConstraints(10, 10);
						dPropWndCheckbox->setFont(
							CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
						);

						hr = dPropWndCheckbox->Create();
					}
					else
						hr = E_FAIL;

					if (SUCCEEDED(hr))
					{
						this->iParam.editChangeBlockerEnabled = true;

						auto descriptorEdit1 = new singleLineEdit(this->hInstance);

						hr = (descriptorEdit1 != nullptr) ? S_OK : E_FAIL;
						if (SUCCEEDED(hr))
						{
							iString desc(getStringFromResource(UI_PROPWND_PROPDESCRIPTOR));
							iString counter(L" 1\0");
							iString d1 = desc + counter;

							descriptorEdit1->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
							descriptorEdit1->setType(
								SLE_TYPE_WITHDESCRIPTION,
								DPIScale(100)
							);
							descriptorEdit1->setCtrlFont(
								CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
							);
							descriptorEdit1->setDescriptionText(d1);
							descriptorEdit1->setBorder(true, this->sInfo.TextColor);
							descriptorEdit1->setAlignment(DESC_ALIGN_LEFT);
							descriptorEdit1->setDimensions(
								DPIScale(40),
								DPIScale(200),
								rc.right - DPIScale(150)
							);
							descriptorEdit1->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);

							hr = descriptorEdit1->Create(this->iParam.tabctrlPage, CTRLID_DESCRIPTORONE);
							if (SUCCEEDED(hr))
							{
								descriptorEdit1->setContent(dInfo.desc1);

								auto descriptorEdit2 = new singleLineEdit(this->hInstance);

								hr = (descriptorEdit2 != nullptr) ? S_OK : E_FAIL;
								if (SUCCEEDED(hr))
								{
									counter.Replace(L" 2\0");
									iString d2 = desc + counter;

									descriptorEdit2->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
									descriptorEdit2->setType(
										SLE_TYPE_WITHDESCRIPTION,
										DPIScale(100)
									);
									descriptorEdit2->setCtrlFont(
										CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
									);
									descriptorEdit2->setDescriptionText(d2);
									descriptorEdit2->setBorder(true, this->sInfo.TextColor);
									descriptorEdit2->setAlignment(DESC_ALIGN_LEFT);
									descriptorEdit2->setDimensions(
										DPIScale(40),
										DPIScale(240),
										rc.right - DPIScale(150)
									);
									descriptorEdit2->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);

									hr = descriptorEdit2->Create(this->iParam.tabctrlPage, CTRLID_DESCRIPTORTWO);
									if (SUCCEEDED(hr))
									{
										descriptorEdit2->setContent(dInfo.desc2);

										auto descriptorEdit3 = new singleLineEdit(this->hInstance);

										hr = (descriptorEdit3 != nullptr) ? S_OK : E_FAIL;
										if (SUCCEEDED(hr))
										{
											counter.Replace(L" 3\0");
											iString d3 = desc + counter;

											descriptorEdit3->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
											descriptorEdit3->setType(
												SLE_TYPE_WITHDESCRIPTION,
												DPIScale(100)
											);
											descriptorEdit3->setCtrlFont(
												CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
											);
											descriptorEdit3->setDescriptionText(d3);
											descriptorEdit3->setBorder(true, this->sInfo.TextColor);
											descriptorEdit3->setAlignment(DESC_ALIGN_LEFT);
											descriptorEdit3->setDimensions(
												DPIScale(40),
												DPIScale(280),
												rc.right - DPIScale(150)
											);
											descriptorEdit3->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);

											hr = descriptorEdit3->Create(this->iParam.tabctrlPage, CTRLID_DESCRIPTORTHREE);
											if (SUCCEEDED(hr))
											{
												descriptorEdit3->setContent(dInfo.desc3);

												this->iParam.editChangeBlockerEnabled = false;

												pt.y = DPIScale(390);

												cb_text.Replace(
													getStringFromResource(UI_PROPWND_INSERTSTANDARDTEMPLATE));

												auto stdTemplateCbx = new CustomCheckbox(this->hInstance, this->iParam.tabctrlPage, &pt, &sz, CTRLID_STDTEMPLATE_CBX);
												if (stdTemplateCbx != nullptr)
												{
													stdTemplateCbx->setChecked(
														dataContainer->getBooleanData(DATAKEY_SETTINGS_INSERTDEFAULTTEXT, true)
													);
													stdTemplateCbx->setEventHandler(
														dynamic_cast<customCheckboxEventSink*>(this)
													);
													stdTemplateCbx->setText(cb_text);
													stdTemplateCbx->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
													stdTemplateCbx->setConstraints(10, 10);
													stdTemplateCbx->setFont(
														CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
													);

													hr = stdTemplateCbx->Create();
												}
												else
													hr = E_FAIL;

												if (SUCCEEDED(hr))
												{
													pt.x = DPIScale(130);
													pt.y = DPIScale(488);
													sz.cx = DPIScale(100);

													auto lineEndCombo = new comboBox(
														this->hInstance, this->iParam.tabctrlPage,
														COMBOTYPE_DROPDOWNLIST, CTRLID_EXPORT_LINEENDFORMAT,
														pt.x, pt.y, sz.cx, DPIScale(30)
													);
													hr = (lineEndCombo != nullptr)
														? S_OK : E_FAIL;
													if (SUCCEEDED(hr))
													{
														if (lineEndCombo->Succeded())
														{
															iString item(L"CR/LF");
															lineEndCombo->Items->AddItem(item);
															item.Replace(L"CR");
															lineEndCombo->Items->AddItem(item);
															item.Replace(L"LF");
															lineEndCombo->Items->AddItem(item);

															lineEndCombo->setFont(
																CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
															);

															lineEndCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
															lineEndCombo->setSelectedIndex(
																extendedDataContainer->getIntegerData(DATAKEY_EXSETTINGS_EXPORT_LINEENDFORMAT, 0)
															);

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
	SafeDeleteArray(&dInfo.desc1);
	SafeDeleteArray(&dInfo.desc2);
	SafeDeleteArray(&dInfo.desc3);

	return hr;
}

HRESULT CnCS_PI::_createFileNavPage()
{
	HRESULT hr;

	RECT rc;
	GetClientRect(this->propWnd, &rc);

	this->iParam.fileNavPage = CreateWindow(
		CNCSPROP_PAGE_CLASS,
		nullptr,
		WS_CHILD | WS_VISIBLE,
		DPIScale(210),
		DPIScale(10),
		rc.right - DPIScale(250),
		rc.bottom - DPIScale(20),
		this->propWnd,
		reinterpret_cast<HMENU>(PAGE_ID_FILENAV),
		this->hInstance,
		reinterpret_cast<LPVOID>(this)
	);

	hr = (this->iParam.fileNavPage != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		GetClientRect(this->iParam.fileNavPage, &rc);

		auto dataContainer = reinterpret_cast<ApplicationData*>(getDefaultApplicationDataContainer());

		// expand path checkbox
		POINT pt;
		pt.x = DPIScale(20);
		pt.y = DPIScale(20);
		SIZE sz;
		sz.cx = rc.right - DPIScale(50);
		sz.cy = DPIScale(30);
		iString cb_text(
			getStringFromResource(UI_PROPWND_ALWAYSEXPANDPATH));

		auto expandPathCheckbox = new CustomCheckbox(this->hInstance, this->iParam.fileNavPage, &pt, &sz, CTRLID_ALWAYSEXPANDPATHCBX);
		if (expandPathCheckbox != nullptr)
		{
			expandPathCheckbox->setChecked(
				dataContainer->getBooleanData(L"alwaysexpandpathcbx", true)
			);
			expandPathCheckbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
			expandPathCheckbox->setText(cb_text);
			expandPathCheckbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
			expandPathCheckbox->setConstraints(10, 10);
			expandPathCheckbox->setFont(
				CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
			);

			hr = expandPathCheckbox->Create();
		}
		else
			hr = E_FAIL;

		if (SUCCEEDED(hr))
		{
			pt.y += DPIScale(40);

			cb_text.Replace(
				getStringFromResource(UI_PROPWND_OPENCREATEDFILE)
			);

			auto openCreatedFileCheckbox = new CustomCheckbox(this->hInstance, this->iParam.fileNavPage, &pt, &sz, CTRLID_OPENCREATEDFILECBX);
			if (openCreatedFileCheckbox != nullptr)
			{
				openCreatedFileCheckbox->setChecked(
					dataContainer->getBooleanData(DATAKEY_SETTINGS_OPENCREATEDFILE, true)
				);
				openCreatedFileCheckbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
				openCreatedFileCheckbox->setText(cb_text);
				openCreatedFileCheckbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
				openCreatedFileCheckbox->setConstraints(10, 10);
				openCreatedFileCheckbox->setFont(
					CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
				);

				hr = openCreatedFileCheckbox->Create();
			}
			else
				hr = E_FAIL;

			if (SUCCEEDED(hr))
			{
				pt.y += DPIScale(40);

				cb_text.Replace(
					getStringFromResource(UI_PROPWND_USEFILETAGSASTOOLTIP)
				);

				auto useFileTagsAsTooltipCheckbox = new CustomCheckbox(this->hInstance, this->iParam.fileNavPage, &pt, &sz, CTRLID_USEFILETAGSASTOOLTIP);
				if (useFileTagsAsTooltipCheckbox != nullptr)
				{
					useFileTagsAsTooltipCheckbox->setChecked(
						dataContainer->getBooleanData(DATAKEY_SETTINGS_USEFILETAGSASTOOLTIP, true)
					);
					useFileTagsAsTooltipCheckbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
					useFileTagsAsTooltipCheckbox->setText(cb_text);
					useFileTagsAsTooltipCheckbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
					useFileTagsAsTooltipCheckbox->setConstraints(10, 10);
					useFileTagsAsTooltipCheckbox->setFont(
						CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
					);

					hr = useFileTagsAsTooltipCheckbox->Create();
				}
				else
					hr = E_FAIL;

				if (SUCCEEDED(hr))
				{
					AppPath aPath;					
					auto sPath = aPath.Get(PATHID_FOLDER_CNCSUITE_APPDATA);

/*					 search settings >>

					allow opening of multiple files
					expand path to file
					close after opening

					search in:
					 - foldername
					 - filename
					 - description 1
					 - description 2
					 - description 3
*/
					auto sc = new Searchcontrol(nullptr, nullptr, nullptr, sPath.GetData(), nullptr, 0);

					hr = (sc != nullptr) ? S_OK : E_FAIL;
					if (SUCCEEDED(hr))
					{
						SRCHSET settings;

						if (sc->getSettings(&settings))
						{
							pt.y = DPIScale(190);

							cb_text.Replace(
								getStringFromResource(UI_PROPWND_SRCH_OPENMULTIFILE)
							);

							auto multifile_checkbox = new CustomCheckbox(this->hInstance, this->iParam.fileNavPage, &pt, &sz, CTRLID_SEARCHCBX_MULTIFILE);
							if (multifile_checkbox != nullptr)
							{
								multifile_checkbox->setChecked(
									(settings.EnableMultipleFileOpening == 1)
									? true : false
								);
								multifile_checkbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
								multifile_checkbox->setText(cb_text);
								multifile_checkbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
								multifile_checkbox->setConstraints(10, 10);
								multifile_checkbox->setFont(
									CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
								);

								hr = multifile_checkbox->Create();
							}
							else
								hr = E_FAIL;

							if (SUCCEEDED(hr))
							{
								pt.y += DPIScale(40);

								cb_text.Replace(
									getStringFromResource(UI_PROPWND_SRCH_CLOSEONOPEN)
								);

								auto closeonopen_checkbox = new CustomCheckbox(this->hInstance, this->iParam.fileNavPage, &pt, &sz, CTRLID_SEARCHCBX_CLOSENOPEN);
								if (closeonopen_checkbox != nullptr)
								{
									closeonopen_checkbox->setChecked(
										(settings.CloseAfterOpening == 1)
										? true : false
									);
									closeonopen_checkbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
									closeonopen_checkbox->setText(cb_text);
									closeonopen_checkbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
									closeonopen_checkbox->setConstraints(10, 10);
									closeonopen_checkbox->setFont(
										CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
									);

									hr = closeonopen_checkbox->Create();
								}
								else
									hr = E_FAIL;

								if (SUCCEEDED(hr))
								{

									pt.y += DPIScale(40);

									cb_text.Replace(
										getStringFromResource(UI_PROPWND_SRCH_EXPANDPATH)
									);

									auto SRexpandPath_checkbox = new CustomCheckbox(this->hInstance, this->iParam.fileNavPage, &pt, &sz, CTRLID_SEARCHCBX_EXPANDPATH);
									if (SRexpandPath_checkbox != nullptr)
									{
										SRexpandPath_checkbox->setChecked(
											(settings.ExpandPathToFile == 1)
											? true : false
										);
										SRexpandPath_checkbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
										SRexpandPath_checkbox->setText(cb_text);
										SRexpandPath_checkbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
										SRexpandPath_checkbox->setConstraints(10, 10);
										SRexpandPath_checkbox->setFont(
											CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
										);

										hr = SRexpandPath_checkbox->Create();
									}
									else
										hr = E_FAIL;

									if (SUCCEEDED(hr))
									{
										pt.y += DPIScale(70);
										pt.x = DPIScale(70);

										cb_text.Replace(
											getStringFromResource(UI_PROPWND_SEARCHFORFILENAME)
										);

										auto filename_checkbox = new CustomCheckbox(this->hInstance, this->iParam.fileNavPage, &pt, &sz, CTRLID_SEARCHCBX_FILENAME);
										if (filename_checkbox != nullptr)
										{
											filename_checkbox->setChecked(
												(settings.SF_Filename == 1)
												? true : false
											);
											filename_checkbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
											filename_checkbox->setCustomImages(IDI_PROPRADIOBUTTON_CHECKED, IDI_PROPRADIOBUTTON_UNCHECKED, 16);
											filename_checkbox->setText(cb_text);
											filename_checkbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
											filename_checkbox->setConstraints(10, 10);
											filename_checkbox->setFont(
												CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
											);

											hr = filename_checkbox->Create();
										}
										else
											hr = E_FAIL;

										if (SUCCEEDED(hr))
										{
											pt.y += DPIScale(35);

											cb_text.Replace(
												getStringFromResource(UI_PROPWND_SEARCHFORFOLDERNAME)
											);

											auto foldername_checkbox = new CustomCheckbox(this->hInstance, this->iParam.fileNavPage, &pt, &sz, CTRLID_SEARCHCBX_FOLDERNAME);
											if (foldername_checkbox != nullptr)
											{
												foldername_checkbox->setChecked(
													(settings.SF_Projectname == 1)
													? true : false
												);
												foldername_checkbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
												foldername_checkbox->setCustomImages(IDI_PROPRADIOBUTTON_CHECKED, IDI_PROPRADIOBUTTON_UNCHECKED, 16);
												foldername_checkbox->setText(cb_text);
												foldername_checkbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
												foldername_checkbox->setConstraints(10, 10);
												foldername_checkbox->setFont(
													CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
												);

												hr = foldername_checkbox->Create();
											}
											else
												hr = E_FAIL;

											if (SUCCEEDED(hr))
											{
												DESCRIPTIONINFO dInfo;
												SecureZeroMemory(&dInfo, sizeof(DESCRIPTIONINFO));
												SendMessage(this->mainWnd, WM_GETDESCRIPTIONS, 0, reinterpret_cast<LPARAM>(&dInfo));

												pt.y += DPIScale(35);

												cb_text.Replace(dInfo.desc1);

												auto desc1_checkbox = new CustomCheckbox(this->hInstance, this->iParam.fileNavPage, &pt, &sz, CTRLID_SEARCHCBX_DESC1);
												if (desc1_checkbox != nullptr)
												{
													desc1_checkbox->setChecked(
														(settings.SF_Description1 == 1)
														? true : false
													);
													desc1_checkbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
													desc1_checkbox->setCustomImages(IDI_PROPRADIOBUTTON_CHECKED, IDI_PROPRADIOBUTTON_UNCHECKED, 16);
													desc1_checkbox->setText(cb_text);
													desc1_checkbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
													desc1_checkbox->setConstraints(10, 10);
													desc1_checkbox->setFont(
														CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
													);

													hr = desc1_checkbox->Create();
												}
												else
													hr = E_FAIL;

												if (SUCCEEDED(hr))
												{
													pt.y += DPIScale(35);

													cb_text.Replace(dInfo.desc2);

													auto desc2_checkbox = new CustomCheckbox(this->hInstance, this->iParam.fileNavPage, &pt, &sz, CTRLID_SEARCHCBX_DESC2);
													if (desc2_checkbox != nullptr)
													{
														desc2_checkbox->setChecked(
															(settings.SF_Description2 == 1)
															? true : false
														);
														desc2_checkbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
														desc2_checkbox->setText(cb_text);
														desc2_checkbox->setCustomImages(IDI_PROPRADIOBUTTON_CHECKED, IDI_PROPRADIOBUTTON_UNCHECKED, 16);
														desc2_checkbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
														desc2_checkbox->setConstraints(10, 10);
														desc2_checkbox->setFont(
															CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
														);

														hr = desc2_checkbox->Create();
													}
													else
														hr = E_FAIL;

													if (SUCCEEDED(hr))
													{
														pt.y += DPIScale(35);

														cb_text.Replace(dInfo.desc3);

														auto desc3_checkbox = new CustomCheckbox(this->hInstance, this->iParam.fileNavPage, &pt, &sz, CTRLID_SEARCHCBX_DESC3);
														if (desc3_checkbox != nullptr)
														{
															desc3_checkbox->setChecked(
																(settings.SF_Description3 == 1)
																? true : false
															);
															desc3_checkbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
															desc3_checkbox->setText(cb_text);
															desc3_checkbox->setCustomImages(IDI_PROPRADIOBUTTON_CHECKED, IDI_PROPRADIOBUTTON_UNCHECKED, 16);
															desc3_checkbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
															desc3_checkbox->setConstraints(10, 10);
															desc3_checkbox->setFont(
																CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
															);

															hr = desc3_checkbox->Create();
														}
														else
															hr = E_FAIL;

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
						SafeRelease(&sc);
					}
				}
			}
		}
	}
	return hr;
}

HRESULT CnCS_PI::_createEditorPage()
{
	HRESULT hr;

	RECT rc;
	GetClientRect(this->propWnd, &rc);

	this->iParam.editorPage = CreateWindow(
		CNCSPROP_PAGE_CLASS,
		nullptr,
		WS_CHILD | WS_VISIBLE,
		DPIScale(210),
		DPIScale(10),
		rc.right - DPIScale(250),
		rc.bottom - DPIScale(20),
		this->propWnd,
		reinterpret_cast<HMENU>(PAGE_ID_EDITOR),
		this->hInstance,
		reinterpret_cast<LPVOID>(this)
	);

	hr = (this->iParam.editorPage != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		GetClientRect(this->iParam.editorPage, &rc);

		auto dataContainer =
			reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer()
				);

		if (dataContainer == nullptr)
			return E_FAIL;

		// color scheme selection >>
		iString normal(
			getStringFromResource(UI_PROPWND_NORMAL)
		);
		iString discreet(
			getStringFromResource(UI_PROPWND_DISCREET)
		);

		auto schemeCombo =
			new comboBox(
				this->hInstance,
				this->iParam.editorPage,
				COMBOTYPE_DROPDOWNLIST,
				CTRLID_COLORSCHEMECOMBO,
				DPIScale(30),
				DPIScale(45),
				DPIScale(250),
				0
			);
		if (schemeCombo != nullptr)
		{
			iString separator(L"--------------");
			schemeCombo->Items->AddItem(normal);
			schemeCombo->Items->AddItem(discreet);
			schemeCombo->Items->AddItem(separator);

			auto schemelist = new itemCollection<iString>();
			if (schemelist != nullptr)
			{
				colorSchemeManager::loadSchemeList(schemelist);

				for (int i = 0; i < schemelist->GetCount(); i++)
				{
					auto str = schemelist->GetAt(i);
					schemeCombo->Items->AddItem(str);
				}
				SafeRelease(&schemelist);
			}
			auto schemeComboSelectedIndex =
				dataContainer->getIntegerData(DATAKEY_SETTINGS_COLORSCHEME, 1);

			schemeCombo->setSelectedIndex(
				schemeComboSelectedIndex
			);
			schemeCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
			schemeCombo->setFont(
				CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
			);

			POINT npt = {
				DPIScale(290),
				DPIScale(45)
			};
			SIZE nsz = {
				DPIScale(190),
				DPIScale(26)
			};
			iString ntext(getStringFromResource(UI_PROPWND_MANAGECOLORSCHEMES));

			auto colorSchemeButton = new CustomButton(this->iParam.editorPage, BUTTONMODE_ICONTEXT, &npt, &nsz, CTRLID_MANAGECOLORSCHEMES, this->hInstance);
			if (colorSchemeButton != nullptr)
			{
				colorSchemeButton->setAppearance_IconText(IDI_PROP_SCEMEMANAGER, 24, ntext);
				colorSchemeButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
				colorSchemeButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
				colorSchemeButton->setTextColor(this->sInfo.TextColor);
				colorSchemeButton->setAlignment(BAL_LEFT);
				colorSchemeButton->setConstraints(5, 10);
				colorSchemeButton->setFont(
					CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
				);
				if (this->sInfo.StyleID == STYLEID_LIGHTGRAY)
				{
					colorSchemeButton->setTextHighlight(TRUE, RGB(255, 255, 255));
				}

				hr = colorSchemeButton->Create();

				if (SUCCEEDED(hr))
				{
					POINT pt = {
						DPIScale(30),
						DPIScale(125)
					};
					SIZE sz = {
						DPIScale(220),
						DPIScale(30)
					};

					auto autocompleteButton = new CustomButton(this->iParam.editorPage, BUTTONMODE_ICONTEXT, &pt, &sz, CTRLID_ACEDITBUTTON, this->hInstance);

					hr = (autocompleteButton != nullptr) ? S_OK : E_FAIL;
					if (SUCCEEDED(hr))
					{
						iString button_text(getStringFromResource(UI_PROPWND_EDITACSTRINGS));

						autocompleteButton->setAppearance_IconText(IDI_PROP_AUTOCOMPLETEMANAGER, 16, button_text);
						autocompleteButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
						autocompleteButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
						autocompleteButton->setTextColor(this->sInfo.TextColor);
						autocompleteButton->setAlignment(BAL_LEFT);
						autocompleteButton->setConstraints(5, 10);
						autocompleteButton->setFont(
							CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
						);
						if (this->sInfo.StyleID == STYLEID_LIGHTGRAY)
						{
							autocompleteButton->setTextHighlight(TRUE, RGB(255, 255, 255));
						}

						hr = autocompleteButton->Create();

						if (SUCCEEDED(hr))
						{
							pt.y = DPIScale(205);

							auto autosyntaxButton = new CustomButton(this->iParam.editorPage, BUTTONMODE_ICONTEXT, &pt, &sz, CTRLID_AUTOSYNTAXBUTTON, this->hInstance);

							hr = (autosyntaxButton != nullptr) ? S_OK : E_FAIL;
							if (SUCCEEDED(hr))
							{
								button_text.Replace(getStringFromResource(UI_PROPWND_SETAUTOSYNTAX));

								autosyntaxButton->setAppearance_IconText(IDI_EDITTOOLBAR_AUTOSYNTAX, 16, button_text);
								autosyntaxButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
								autosyntaxButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
								autosyntaxButton->setTextColor(this->sInfo.TextColor);
								autosyntaxButton->setAlignment(BAL_LEFT);
								autosyntaxButton->setConstraints(5, 10);
								autosyntaxButton->setFont(
									CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
								);
								if (this->sInfo.StyleID == STYLEID_LIGHTGRAY)
								{
									autosyntaxButton->setTextHighlight(TRUE, RGB(255, 255, 255));
								}

								hr = autosyntaxButton->Create();

								if (SUCCEEDED(hr))
								{
									//// error-search checkbox
									//pt.x = DPIScale(20);
									//pt.y = DPIScale(440);
									//sz.cx = rc.right - DPIScale(50);
									//sz.cy = DPIScale(30);
									//iString cb_text(
									//	getStringFromResource(UI_PROPWND_SEARCHFORERRORS));

									//auto searchForErrorCheckbox = new CustomCheckbox(this->hInstance, this->iParam.editorPage, &pt, &sz, CTRLID_SEARCHFORERRORCBX);
									//hr = (searchForErrorCheckbox != nullptr)
									//	? S_OK : E_FAIL;

									//if (SUCCEEDED(hr))
									//{
									//	searchForErrorCheckbox->setChecked(
									//		dataContainer->getBooleanData(DATAKEY_SETTINGS_SEARCH_FOR_ERRORS, false)
									//	);
									//	searchForErrorCheckbox->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
									//	searchForErrorCheckbox->setText(cb_text);
									//	searchForErrorCheckbox->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
									//	searchForErrorCheckbox->setConstraints(10, 10);
									//	searchForErrorCheckbox->setFont(
									//		CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
									//	);

									//	hr = searchForErrorCheckbox->Create();

									//	if (SUCCEEDED(hr))
									//	{
									pt.x = DPIScale(490);
									pt.y = DPIScale(45);
									sz.cx = DPIScale(26);
									sz.cy = DPIScale(26);

									auto delButton = new CustomButton(this->iParam.editorPage, BUTTONMODE_ICON, &pt, &sz, CTRLID_DELETECURRENTSCHEME, this->hInstance);
									if (delButton != nullptr)
									{
										iString tooltip(
											getStringFromResource(UI_PROPWND_DELETECURRENTSCHEME)
										);

										delButton->setAppearance_onlyIcon(IDI_TOOLBAR_DELETE, DPIScale(24));
										delButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
										delButton->setColors(RGB(200, 100, 100), RGB(230, 50, 50), RGB(200, 50, 50));
										delButton->setBorder(TRUE, this->sInfo.OutlineColor);
										delButton->setDisabledColors(RGB(80, 80, 80), 0);
										delButton->setDisabledIcon(IDI_TOOLBAR_DELETE_PRESSED);
										delButton->setTooltip(tooltip);

										if (schemeComboSelectedIndex < 3)
											delButton->setEnabledState(false);

										hr = delButton->Create();
										if (SUCCEEDED(hr))
										{
											//...
										}
									}
									//	}
									//}
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

HRESULT CnCS_PI::_createDataExPage()
{
	HRESULT hr;

	RECT rc;
	GetClientRect(this->propWnd, &rc);

	this->iParam.exchangePage = CreateWindow(
		CNCSPROP_PAGE_CLASS,
		nullptr,
		WS_CHILD | WS_VISIBLE,
		DPIScale(210),
		DPIScale(10),
		rc.right - DPIScale(250),
		rc.bottom - DPIScale(20),
		this->propWnd,
		reinterpret_cast<HMENU>(PAGE_ID_EXCHANGE),
		this->hInstance,
		reinterpret_cast<LPVOID>(this)
	);

	hr = (this->iParam.exchangePage != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		GetClientRect(this->iParam.exchangePage, &rc);

		this->iParam.editChangeBlockerEnabled = true;

		AppPath appPath;
		iString path = appPath.Get(PATHID_FILE_COMSETUP);

		SerialComm dTraf;
		SERIAL_CONFIG config;

		dTraf.getConfiguration(path.GetData(), &config);

		auto comEdit = new singleLineEdit(this->hInstance);

		hr = (comEdit != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			iString com(getStringFromResource(UI_PROPWND_COMPORTNUMBER));

			comEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
			comEdit->setType(
				SLE_TYPE_WITHDESCRIPTION,
				DPIScale(200)
			);
			comEdit->setDescriptionText(com);
			comEdit->setEditFontProperties(
				iString(APPLICATION_PRIMARY_FONT),
				DPIScale(14),
				DPIScale(22)
			);
			comEdit->setCtrlFont(
				CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
			);
			comEdit->setBorder(true, this->sInfo.TextColor);
			comEdit->setAdditionalEditStyles(ES_CENTER);
			comEdit->setAlignment(DESC_ALIGN_RIGHT);
			comEdit->setSpacing(
				DPIScale(20)
			);
			comEdit->setDimensions(
				DPIScale(20),
				DPIScale(50),
				DPIScale(250)
			);
			comEdit->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);
			comEdit->restrictedContent(SLE_RESTRICTTYPE_ONLYNUMBERS);
			comEdit->setValidContentRange(1, 1024);
			comEdit->setExtendedFlag(SLE_EX_NOEVENT_ON_INVALIDCONTENT);

			hr = comEdit->Create(this->iParam.exchangePage, CTRLID_COMEDIT);
			if (SUCCEEDED(hr))
			{
				auto comPort = iString::FromInt(config.Active_port);
				comEdit->setContent(comPort->GetData());
				comPort->Release();

				auto BaudCombo =
					new comboBox(
						this->hInstance,
						this->iParam.exchangePage,
						COMBOTYPE_DROPDOWNLIST,
						CTRLID_BAUDRATECOMBO,
						DPIScale(220),
						DPIScale(90),
						DPIScale(150),
						0
					);
				if (BaudCombo != nullptr)
				{
					for (int i = 0; i < BAUDRATE_STRUCTSIZE; i++)
					{
						auto str = iString::FromInt(_baud[i]);
						
						BaudCombo->Items->AddItem(str);
						SafeRelease(&str);
					}
					BaudCombo->setSelectedIndex(config.baud_index);
					BaudCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
					BaudCombo->setFont(
						CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
					);
				}
				else
					hr = E_FAIL;

				if (SUCCEEDED(hr))
				{
					auto ParityCombo =
						new comboBox(
							this->hInstance,
							this->iParam.exchangePage,
							COMBOTYPE_DROPDOWNLIST,
							CTRLID_PARITYCOMBO,
							DPIScale(220),
							DPIScale(120),
							DPIScale(150),
							0
						);
					if (ParityCombo != nullptr)
					{
						for (int i = 0; i < 5; i++)
						{
							iString str(getStringFromResource(UI_PROPWND_NOPARITY + i));

							ParityCombo->Items->AddItem(str);
							
						}
						ParityCombo->setSelectedIndex(config.parity_index);
						ParityCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
						ParityCombo->setFont(
							CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
						);
					}
					else
						hr = E_FAIL;

					if (SUCCEEDED(hr))
					{
						auto DatabitCombo =
							new comboBox(
								this->hInstance,
								this->iParam.exchangePage,
								COMBOTYPE_DROPDOWNLIST,
								CTRLID_DATABITSCOMBO,
								DPIScale(220),
								DPIScale(150),
								DPIScale(150),
								0
							);
						if (DatabitCombo != nullptr)
						{
							for (int i = 0; i < 4; i++)
							{
								auto str1 = iString::FromInt(i + 5);
								str1->Append(L" bits\0");

								DatabitCombo->Items->AddItem(str1);

								SafeRelease(&str1);
							}
							DatabitCombo->setSelectedIndex(config.databit_index);
							DatabitCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
							DatabitCombo->setFont(
								CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
							);
						}
						else
							hr = E_FAIL;

						if (SUCCEEDED(hr))
						{
							auto StopbitCombo =
								new comboBox(
									this->hInstance,
									this->iParam.exchangePage,
									COMBOTYPE_DROPDOWNLIST,
									CTRLID_STOPBITSCOMBO,
									DPIScale(220),
									DPIScale(180),
									DPIScale(150),
									0
								);
							if (StopbitCombo != nullptr)
							{
								iString oneSb(L"1 bit\0");
								StopbitCombo->Items->AddItem(oneSb);
								iString onePointFiveSb(L"1.5 bits\0");
								StopbitCombo->Items->AddItem(onePointFiveSb);
								iString twoSb(L"2 bits\0");
								StopbitCombo->Items->AddItem(twoSb);

								StopbitCombo->setSelectedIndex(config.stopbit_index);
								StopbitCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
								StopbitCombo->setFont(
									CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
								);
							}
							else
								hr = E_FAIL;

							if (SUCCEEDED(hr))
							{
								auto HandshakeCombo =
									new comboBox(
										this->hInstance,
										this->iParam.exchangePage,
										COMBOTYPE_DROPDOWNLIST,
										CTRLID_HANDSHAKECOMBO,
										DPIScale(220),
										DPIScale(210),
										DPIScale(150),
										0
									);
								if (HandshakeCombo != nullptr)
								{
									for (int i = 0; i < 3; i++)
									{
										iString str(getStringFromResource(UI_PROPWND_FLOWCTRL_NONE + i));
										HandshakeCombo->Items->AddItem(str);
									}
									HandshakeCombo->setSelectedIndex(config.XON_XOFF);
									HandshakeCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
									HandshakeCombo->setFont(
										CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
									);
								}
								else
									hr = E_FAIL;


								// paritycheck checkbox
								POINT pt;
								pt.x = DPIScale(25);
								pt.y = DPIScale(250);
								SIZE sz;
								sz.cx = rc.right - DPIScale(300);
								sz.cy = DPIScale(30);
								iString cb_text(
									getStringFromResource(UI_PROPWND_PERFORMPARITYCHECK));

								auto parityCBX = new CustomCheckbox(this->hInstance, this->iParam.exchangePage, &pt, &sz, CTRLID_PARITYCHECKCBX);
								hr = (parityCBX != nullptr)
									? S_OK : E_FAIL;

								if (SUCCEEDED(hr))
								{
									parityCBX->setChecked((config.Paritycheck == TRUE));
									parityCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
									parityCBX->setText(cb_text);
									parityCBX->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
									parityCBX->setConstraints(10, 10);
									parityCBX->setFont(
										CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
									);

									hr = parityCBX->Create();

									if (SUCCEEDED(hr))
									{
										pt.y = DPIScale(275);
										cb_text.Replace(getStringFromResource(UI_PROPWND_ABORTONERROR));

										auto abortCBX = new CustomCheckbox(this->hInstance, this->iParam.exchangePage, &pt, &sz, CTRLID_ABORTONERRORCBX);
										hr = (abortCBX != nullptr)
											? S_OK : E_FAIL;

										if (SUCCEEDED(hr))
										{
											abortCBX->setChecked((config.Abort_on_error == TRUE));
											abortCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
											abortCBX->setText(cb_text);
											abortCBX->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
											abortCBX->setConstraints(10, 10);
											abortCBX->setFont(
												CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
											);

											hr = abortCBX->Create();

											if (SUCCEEDED(hr))
											{
												pt.y = DPIScale(300);
												cb_text.Replace(getStringFromResource(UI_PROPWND_REPLACEPARITYERROR));

												auto replaceCBX = new CustomCheckbox(this->hInstance, this->iParam.exchangePage, &pt, &sz, CTRLID_REPLACEERRORCBX);
												hr = (replaceCBX != nullptr)
													? S_OK : E_FAIL;

												if (SUCCEEDED(hr))
												{
													replaceCBX->setChecked((config.ErrorChar_replace == TRUE));
													replaceCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
													replaceCBX->setText(cb_text);
													replaceCBX->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
													replaceCBX->setConstraints(10, 10);
													replaceCBX->setFont(
														CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
													);

													hr = replaceCBX->Create();

													if (SUCCEEDED(hr))
													{
														auto dtrFlowCombo =
															new comboBox(
																this->hInstance,
																this->iParam.exchangePage,
																COMBOTYPE_DROPDOWNLIST,
																CTRLID_DTRFLOWCOMBO,
																DPIScale(220),
																DPIScale(380),
																DPIScale(150),
																0
															);
														if (dtrFlowCombo != nullptr)
														{
															iString dtr(L"Disable\0");
															dtrFlowCombo->Items->AddItem(dtr);
															dtr.Replace(L"Enable\0");
															dtrFlowCombo->Items->AddItem(dtr);
															dtr.Replace(L"Handshake\0");
															dtrFlowCombo->Items->AddItem(dtr);

															dtrFlowCombo->setSelectedIndex(config.DTR_control_index);
															dtrFlowCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
															dtrFlowCombo->setFont(
																CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
															);
														}
														else
															hr = E_FAIL;

														if (SUCCEEDED(hr))
														{
															auto rtsFlowCombo =
																new comboBox(
																	this->hInstance,
																	this->iParam.exchangePage,
																	COMBOTYPE_DROPDOWNLIST,
																	CTRLID_RTSFLOWCOMBO,
																	DPIScale(220),
																	DPIScale(410),
																	DPIScale(150),
																	0
																);
															if (rtsFlowCombo != nullptr)
															{
																iString dtr(L"Disable\0");
																rtsFlowCombo->Items->AddItem(dtr);
																dtr.Replace(L"Enable\0");
																rtsFlowCombo->Items->AddItem(dtr);
																dtr.Replace(L"Handshake\0");
																rtsFlowCombo->Items->AddItem(dtr);
																dtr.Replace(L"Toggle\0");
																rtsFlowCombo->Items->AddItem(dtr);

																rtsFlowCombo->setSelectedIndex(config.RTS_control_index);
																rtsFlowCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
																rtsFlowCombo->setFont(
																	CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																);
															}
															else
																hr = E_FAIL;


															if (SUCCEEDED(hr))
															{
																auto xonCharEdit = new singleLineEdit(this->hInstance);

																hr = (xonCharEdit != nullptr) ? S_OK : E_FAIL;
																if (SUCCEEDED(hr))
																{
																	iString xon(L"Xon");

																	xonCharEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
																	xonCharEdit->setType(
																		SLE_TYPE_WITHDESCRIPTION,
																		DPIScale(50)
																	);
																	xonCharEdit->setDescriptionText(xon);
																	xonCharEdit->setEditFontProperties(
																		iString(L"Segoe UI\0"),
																		DPIScale(12),
																		DPIScale(18)
																	);
																	xonCharEdit->setCtrlFont(
																		CreateScaledFont(14, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																	);
																	xonCharEdit->setBorder(true, this->sInfo.TextColor);
																	xonCharEdit->setAdditionalEditStyles(ES_CENTER);
																	xonCharEdit->setAlignment(DESC_ALIGN_LEFT);
																	xonCharEdit->setSpacing(10);
																	xonCharEdit->setDimensions(
																		DPIScale(40),
																		DPIScale(475),
																		DPIScale(80)
																	);
																	xonCharEdit->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);

																	hr = xonCharEdit->Create(this->iParam.exchangePage, CTRLID_XONCHAREDIT);
																	if (SUCCEEDED(hr))
																	{
																		TCHAR xonchar[2] = { config.XonChar, L'\0' };
																		xonCharEdit->setContent(
																			iString(xonchar));

																		auto xoffCharEdit = new singleLineEdit(this->hInstance);

																		hr = (xoffCharEdit != nullptr) ? S_OK : E_FAIL;
																		if (SUCCEEDED(hr))
																		{
																			iString xoff(L"Xoff");

																			xoffCharEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
																			xoffCharEdit->setType(
																				SLE_TYPE_WITHDESCRIPTION,
																				DPIScale(50)
																			);
																			xoffCharEdit->setDescriptionText(xoff);
																			xoffCharEdit->setEditFontProperties(
																				iString(L"Segoe UI\0"),
																				DPIScale(12),
																				DPIScale(18)
																			);
																			xoffCharEdit->setCtrlFont(
																				CreateScaledFont(14, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																			);
																			xoffCharEdit->setBorder(true, this->sInfo.TextColor);
																			xoffCharEdit->setAdditionalEditStyles(ES_CENTER);
																			xoffCharEdit->setAlignment(DESC_ALIGN_LEFT);
																			xoffCharEdit->setSpacing(10);
																			xoffCharEdit->setDimensions(
																				DPIScale(40),
																				DPIScale(495),
																				DPIScale(80)
																			);
																			xoffCharEdit->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);

																			hr = xoffCharEdit->Create(this->iParam.exchangePage, CTRLID_XOFFCHAREDIT);
																			if (SUCCEEDED(hr))
																			{
																				TCHAR xoffchar[2] = { config.XoffChar, L'\0' };
																				xoffCharEdit->setContent(
																					iString(xoffchar));

																				auto errorCharEdit = new singleLineEdit(this->hInstance);

																				hr = (errorCharEdit != nullptr) ? S_OK : E_FAIL;
																				if (SUCCEEDED(hr))
																				{
																					iString error(L"Error");

																					errorCharEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
																					errorCharEdit->setType(
																						SLE_TYPE_WITHDESCRIPTION,
																						DPIScale(50)
																					);
																					errorCharEdit->setDescriptionText(error);
																					errorCharEdit->setEditFontProperties(
																						iString(L"Segoe UI\0"),
																						DPIScale(12),
																						DPIScale(18)
																					);
																					errorCharEdit->setCtrlFont(
																						CreateScaledFont(14, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																					);
																					errorCharEdit->setBorder(true, this->sInfo.TextColor);
																					errorCharEdit->setAdditionalEditStyles(ES_CENTER);
																					errorCharEdit->setAlignment(DESC_ALIGN_LEFT);
																					errorCharEdit->setSpacing(10);
																					errorCharEdit->setDimensions(
																						DPIScale(40),
																						DPIScale(515),
																						DPIScale(80)
																					);
																					errorCharEdit->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);

																					hr = errorCharEdit->Create(this->iParam.exchangePage, CTRLID_ERRORCHAREDIT);
																					if (SUCCEEDED(hr))
																					{
																						TCHAR errorchar[2] = { config.ErrorChar, L'\0' };
																						errorCharEdit->setContent(
																							iString(errorchar));

																						auto eofCharEdit = new singleLineEdit(this->hInstance);

																						hr = (eofCharEdit != nullptr) ? S_OK : E_FAIL;
																						if (SUCCEEDED(hr))
																						{
																							iString eof(L"Eof\0");

																							eofCharEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
																							eofCharEdit->setType(
																								SLE_TYPE_WITHDESCRIPTION,
																								DPIScale(50)
																							);
																							eofCharEdit->setDescriptionText(eof);
																							eofCharEdit->setEditFontProperties(
																								iString(L"Segoe UI\0"),
																								DPIScale(12),
																								DPIScale(18)
																							);
																							eofCharEdit->setCtrlFont(
																								CreateScaledFont(14, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																							);
																							eofCharEdit->setBorder(true, this->sInfo.TextColor);
																							eofCharEdit->setAdditionalEditStyles(ES_CENTER);
																							eofCharEdit->setAlignment(DESC_ALIGN_LEFT);
																							eofCharEdit->setSpacing(10);
																							eofCharEdit->setDimensions(
																								DPIScale(40),
																								DPIScale(535),
																								DPIScale(80)
																							);
																							eofCharEdit->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);

																							hr = eofCharEdit->Create(this->iParam.exchangePage, CTRLID_EOFCHAREDIT);
																							if (SUCCEEDED(hr))
																							{
																								TCHAR eofchar[2] = { config.EofChar, L'\0' };
																								eofCharEdit->setContent(
																									iString(eofchar));

																								auto eventCharEdit = new singleLineEdit(this->hInstance);

																								hr = (eventCharEdit != nullptr) ? S_OK : E_FAIL;
																								if (SUCCEEDED(hr))
																								{
																									iString eventC(L"Event");

																									eventCharEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
																									eventCharEdit->setType(
																										SLE_TYPE_WITHDESCRIPTION,
																										DPIScale(50)
																									);
																									eventCharEdit->setDescriptionText(eventC);
																									eventCharEdit->setEditFontProperties(
																										iString(L"Segoe UI\0"),
																										DPIScale(12),
																										DPIScale(18)
																									);
																									eventCharEdit->setCtrlFont(
																										CreateScaledFont(14, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																									);
																									eventCharEdit->setBorder(true, this->sInfo.TextColor);
																									eventCharEdit->setAdditionalEditStyles(ES_CENTER);
																									eventCharEdit->setAlignment(DESC_ALIGN_LEFT);
																									eventCharEdit->setSpacing(10);
																									eventCharEdit->setDimensions(
																										DPIScale(40),
																										DPIScale(555),
																										DPIScale(80)
																									);
																									eventCharEdit->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);

																									hr = eventCharEdit->Create(this->iParam.exchangePage, CTRLID_EVENTCHAREDIT);
																									if (SUCCEEDED(hr))
																									{
																										TCHAR eventchar[2] = { config.EventChar, L'\0' };
																										eventCharEdit->setContent(
																											iString(eventchar));

																										auto readIntervalEdit = new singleLineEdit(this->hInstance);

																										hr = (readIntervalEdit != nullptr) ? S_OK : E_FAIL;
																										if (SUCCEEDED(hr))
																										{
																											iString readInterval(L"Interval");

																											readIntervalEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
																											readIntervalEdit->setType(
																												SLE_TYPE_WITHDESCRIPTION,
																												DPIScale(100)
																											);
																											readIntervalEdit->setDescriptionText(readInterval);
																											readIntervalEdit->setEditFontProperties(
																												iString(APPLICATION_PRIMARY_FONT),
																												DPIScale(12),
																												DPIScale(18)
																											);
																											readIntervalEdit->setCtrlFont(
																												CreateScaledFont(14, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																											);
																											readIntervalEdit->setBorder(true, this->sInfo.TextColor);
																											readIntervalEdit->setAdditionalEditStyles(ES_CENTER);
																											readIntervalEdit->setAlignment(DESC_ALIGN_LEFT);
																											readIntervalEdit->setSpacing(10);
																											readIntervalEdit->setDimensions(
																												DPIScale(353),
																												DPIScale(475),
																												DPIScale(160)
																											);
																											readIntervalEdit->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);
																											readIntervalEdit->restrictedContent(SLE_RESTRICTTYPE_ONLYNUMBERS);

																											hr = readIntervalEdit->Create(this->iParam.exchangePage, CTRLID_READINTERVALEDIT);
																											if (SUCCEEDED(hr))
																											{																					
																												readIntervalEdit->setContent(
																													iString::fromInt(config.RI_Timeout));

																												auto readMplEdit = new singleLineEdit(this->hInstance);

																												hr = (readMplEdit != nullptr) ? S_OK : E_FAIL;
																												if (SUCCEEDED(hr))
																												{
																													iString readMpl(L"Total-Multiplier");

																													readMplEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
																													readMplEdit->setType(
																														SLE_TYPE_WITHDESCRIPTION,
																														DPIScale(100)
																													);
																													readMplEdit->setDescriptionText(readMpl);
																													readMplEdit->setEditFontProperties(
																														iString(L"Segoe UI\0"),
																														DPIScale(12),
																														DPIScale(18)
																													);
																													readMplEdit->setCtrlFont(
																														CreateScaledFont(14, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																													);
																													readMplEdit->setBorder(true, this->sInfo.TextColor);
																													readMplEdit->setAdditionalEditStyles(ES_CENTER);
																													readMplEdit->setAlignment(DESC_ALIGN_LEFT);
																													readMplEdit->setSpacing(10);
																													readMplEdit->setDimensions(
																														DPIScale(353),
																														DPIScale(495),
																														DPIScale(160)
																													);
																													readMplEdit->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);
																													readMplEdit->restrictedContent(SLE_RESTRICTTYPE_ONLYNUMBERS);

																													hr = readMplEdit->Create(this->iParam.exchangePage, CTRLID_READTOTALMPLEDIT);
																													if (SUCCEEDED(hr))
																													{
																														readMplEdit->setContent(
																															iString::fromInt(config.RT_Timeout_mpl));

																														auto readCstEdit = new singleLineEdit(this->hInstance);

																														hr = (readCstEdit != nullptr) ? S_OK : E_FAIL;
																														if (SUCCEEDED(hr))
																														{
																															iString readCst(L"Total-Constant");

																															readCstEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
																															readCstEdit->setType(
																																SLE_TYPE_WITHDESCRIPTION,
																																DPIScale(100)
																															);
																															readCstEdit->setDescriptionText(readCst);
																															readCstEdit->setEditFontProperties(
																																iString(L"Segoe UI\0"),
																																DPIScale(12),
																																DPIScale(18)
																															);
																															readCstEdit->setCtrlFont(
																																CreateScaledFont(14, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																															);
																															readCstEdit->setBorder(true, this->sInfo.TextColor);
																															readCstEdit->setAdditionalEditStyles(ES_CENTER);
																															readCstEdit->setAlignment(DESC_ALIGN_LEFT);
																															readCstEdit->setSpacing(10);
																															readCstEdit->setDimensions(
																																DPIScale(353),
																																DPIScale(515),
																																DPIScale(160)
																															);
																															readCstEdit->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);
																															readCstEdit->restrictedContent(SLE_RESTRICTTYPE_ONLYNUMBERS);

																															hr = readCstEdit->Create(this->iParam.exchangePage, CTRLID_READTOTALCSTEDIT);
																															if (SUCCEEDED(hr))
																															{																																
																																readCstEdit->setContent(
																																	iString::fromInt(config.RT_Timeout_cst));

																																auto writeMplEdit = new singleLineEdit(this->hInstance);

																																hr = (writeMplEdit != nullptr) ? S_OK : E_FAIL;
																																if (SUCCEEDED(hr))
																																{
																																	iString writeMpl(L"Total-Multiplier");

																																	writeMplEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
																																	writeMplEdit->setType(
																																		SLE_TYPE_WITHDESCRIPTION,
																																		DPIScale(100)
																																	);
																																	writeMplEdit->setDescriptionText(writeMpl);
																																	writeMplEdit->setEditFontProperties(
																																		iString(L"Segoe UI\0"),
																																		DPIScale(12),
																																		DPIScale(18)
																																	);
																																	writeMplEdit->setCtrlFont(
																																		CreateScaledFont(14, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																																	);
																																	writeMplEdit->setBorder(true, this->sInfo.TextColor);
																																	writeMplEdit->setAdditionalEditStyles(ES_CENTER);
																																	writeMplEdit->setAlignment(DESC_ALIGN_LEFT);
																																	writeMplEdit->setSpacing(10);
																																	writeMplEdit->setDimensions(
																																		DPIScale(353),
																																		DPIScale(535),
																																		DPIScale(160)
																																	);
																																	writeMplEdit->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);
																																	writeMplEdit->restrictedContent(SLE_RESTRICTTYPE_ONLYNUMBERS);

																																	hr = writeMplEdit->Create(this->iParam.exchangePage, CTRLID_WRITETOTALMPLEDIT);
																																	if (SUCCEEDED(hr))
																																	{
																																		writeMplEdit->setContent(
																																			iString::fromInt(config.WT_Timeout_mpl));

																																		auto writeCstEdit = new singleLineEdit(this->hInstance);

																																		hr = (writeCstEdit != nullptr) ? S_OK : E_FAIL;
																																		if (SUCCEEDED(hr))
																																		{
																																			iString writeCst(L"Total-Constant");

																																			writeCstEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
																																			writeCstEdit->setType(
																																				SLE_TYPE_WITHDESCRIPTION,
																																				DPIScale(100)
																																			);
																																			writeCstEdit->setDescriptionText(writeCst);
																																			writeCstEdit->setEditFontProperties(
																																				iString(L"Segoe UI\0"),
																																				DPIScale(12),
																																				DPIScale(18)
																																			);
																																			writeCstEdit->setCtrlFont(
																																				CreateScaledFont(14, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																																			);
																																			writeCstEdit->setBorder(true, this->sInfo.TextColor);
																																			writeCstEdit->setAdditionalEditStyles(ES_CENTER);
																																			writeCstEdit->setAlignment(DESC_ALIGN_LEFT);
																																			writeCstEdit->setSpacing(10);
																																			writeCstEdit->setDimensions(
																																				DPIScale(353),
																																				DPIScale(555),
																																				DPIScale(160)
																																			);
																																			writeCstEdit->setColors(this->sInfo.TabColor, this->sInfo.mainToolbarColor, this->sInfo.TextColor, this->sInfo.TextColor);

																																			hr = writeCstEdit->Create(this->iParam.exchangePage, CTRLID_WRITETOTALCSTEDIT);
																																			if (SUCCEEDED(hr))
																																			{
																																				if (config.WT_Timeout_cst == -1)
																																				{
																																					writeCstEdit->setContent(
																																						iString(L"Auto")
																																					);
																																				}
																																				else
																																				{
																																					writeCstEdit->setContent(
																																						iString::fromInt(config.WT_Timeout_cst)
																																					);
																																				}
																																				if (SUCCEEDED(hr))
																																				{
																																					pt.x = DPIScale(290);
																																					pt.y = DPIScale(50);
																																					sz.cx = DPIScale(200);
																																					sz.cy = DPIScale(24);

																																					auto devListButton = new CustomButton(this->iParam.exchangePage, BUTTONMODE_TEXT, &pt, &sz, CTRLID_DEVICELISTINGBTN, this->hInstance);

																																					hr = (devListButton != nullptr) ? S_OK : E_FAIL;
																																					if (SUCCEEDED(hr))
																																					{
																																						iString button_text(
																																							getStringFromResource(UI_PROPWND_CREATEDEVICELISTING)
																																						);

																																						devListButton->setAppearance_onlyText(button_text, FALSE);
																																						devListButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
																																						devListButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
																																						devListButton->setTextColor(this->sInfo.TextColor);
																																						devListButton->setBorder(TRUE, this->sInfo.OutlineColor);
																																						devListButton->setFont(
																																							CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																																						);
																																						if (this->sInfo.StyleID == STYLEID_LIGHTGRAY)
																																						{
																																							devListButton->setTextHighlight(TRUE, RGB(255, 255, 255));
																																						}



																																						hr = devListButton->Create();

																																						if (SUCCEEDED(hr))
																																						{
																																							pt.x = rc.right - DPIScale(240);
																																							pt.y = DPIScale(300);
																																							sz.cx = DPIScale(220);
																																							
																																							auto outputButton = new CustomButton(this->iParam.exchangePage, BUTTONMODE_TEXT, &pt, &sz, CTRLID_OUTPUTSETTINGS, this->hInstance);
																																							hr = (outputButton != nullptr)
																																								? S_OK : E_FAIL;

																																							if (SUCCEEDED(hr))
																																							{
																																								button_text.Replace(
																																									getStringFromResource(UI_PROPWND_OUTPUTSETTINGS)
																																								);

																																								outputButton->setAppearance_onlyText(button_text, FALSE);
																																								outputButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
																																								outputButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
																																								outputButton->setTextColor(this->sInfo.TextColor);
																																								outputButton->setBorder(TRUE, this->sInfo.OutlineColor);
																																								outputButton->setFont(
																																									CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																																								);
																																								if (this->sInfo.StyleID == STYLEID_LIGHTGRAY)
																																								{
																																									outputButton->setTextHighlight(TRUE, RGB(255, 255, 255));
																																								}



																																								hr = outputButton->Create();
																																								if (SUCCEEDED(hr))
																																								{
																																									// ...

																																									this->iParam.editChangeBlockerEnabled = false;
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

HRESULT CnCS_PI::_createOutputSettingsPage()
{
	RECT rc;
	GetClientRect(this->propWnd, &rc);

	this->iParam.outputSettingsPage = CreateWindow(
		CNCSPROP_PAGE_CLASS,
		nullptr,
		WS_CHILD | WS_VISIBLE,
		DPIScale(210),
		DPIScale(10),
		rc.right - DPIScale(250),
		rc.bottom - DPIScale(20),
		this->propWnd,
		reinterpret_cast<HMENU>(NAVPAGE_ID_OUTPUTSETTINGS),
		this->hInstance,
		reinterpret_cast<LPVOID>(this)
	);

	HRESULT hr =
		(this->iParam.outputSettingsPage != nullptr)
		? S_OK : E_FAIL;

	if (SUCCEEDED(hr))
	{
		GetClientRect(this->iParam.outputSettingsPage, &rc);

		POINT pos;
		SIZE sz;
		pos.x = DPIScale(20);
		pos.y = rc.bottom - DPIScale(40);
		sz.cx = DPIScale(160);
		sz.cy = DPIScale(26);

		iString buttontext(
			getStringFromResource(UI_GNRL_NAVIGATEBACK)
		);

		auto backbutton = new CustomButton(this->iParam.outputSettingsPage, BUTTONMODE_TEXT, &pos, &sz, CTRLID_NAVIGATEBACKBTN, this->hInstance);
		hr = (backbutton != nullptr)
			? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			backbutton->setAppearance_onlyText(buttontext, FALSE);
			backbutton->setBorder(TRUE, this->sInfo.OutlineColor);
			backbutton->setColors(this->sInfo.mainToolbarColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
			backbutton->setTextColor(this->sInfo.TextColor);
			backbutton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
			backbutton->setFont(
				CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
			);
			if (this->sInfo.StyleID == STYLEID_LIGHTGRAY)
			{
				backbutton->setTextHighlight(TRUE, RGB(255, 255, 255));
			}

			hr = backbutton->Create();
			if (SUCCEEDED(hr))
			{
				auto dataContainer =
					reinterpret_cast<ApplicationData*>(
						getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
						);

				hr = (dataContainer != nullptr)
					? S_OK : E_FAIL;

				if (SUCCEEDED(hr))
				{
					POINT pt;
					pt.x = DPIScale(20);
					pt.y = DPIScale(50);
					
					sz.cx = rc.right - DPIScale(100);
					sz.cy = DPIScale(30);

					iString cb_text(
						getStringFromResource(UI_PROPWND_ANCHOROUTPUTWINDOW));

					auto anchorWindowCBX =
						new CustomCheckbox(this->hInstance, this->iParam.outputSettingsPage, &pt, &sz, CTRLID_ANCHORINOUTWNDCBX);

					hr = (anchorWindowCBX != nullptr) ? S_OK : E_FAIL;
					if(SUCCEEDED(hr))
					{
						anchorWindowCBX->setChecked(
							dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXCHANGEWND_ANCHORWND, true)
						);
						anchorWindowCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
						anchorWindowCBX->setText(cb_text);
						anchorWindowCBX->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
						anchorWindowCBX->setConstraints(10, 10);
						anchorWindowCBX->setFont(
							CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
						);

						hr = anchorWindowCBX->Create();
						if (SUCCEEDED(hr))
						{
							pt.y += DPIScale(35);
							cb_text.Replace(
								getStringFromResource(UI_PROPWND_MONITORTRANSMISSON)
							);

							auto monitorOutputCBX =
								new CustomCheckbox(this->hInstance, this->iParam.outputSettingsPage, &pt, &sz, CTRLID_INOUTMONITORCBX);

							hr = (monitorOutputCBX != nullptr) ? S_OK : E_FAIL;
							if (SUCCEEDED(hr))
							{
								monitorOutputCBX->setChecked(
									dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXCHANGEWND_MONITORTRANSMISSION, true)
								);
								monitorOutputCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
								monitorOutputCBX->setText(cb_text);
								monitorOutputCBX->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
								monitorOutputCBX->setConstraints(10, 10);
								monitorOutputCBX->setFont(
									CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
								);

								hr = monitorOutputCBX->Create();
								if (SUCCEEDED(hr))
								{
									pt.y += DPIScale(35);

									cb_text.Replace(
										getStringFromResource(UI_PROPWND_VERBOSETRANSMISSION)
									);

									auto verboseOutputCBX =
										new CustomCheckbox(this->hInstance, this->iParam.outputSettingsPage, &pt, &sz, CTRLID_INOUTVERBOSECBX);

									hr = (verboseOutputCBX != nullptr) ? S_OK : E_FAIL;
									if (SUCCEEDED(hr))
									{
										verboseOutputCBX->setChecked(
											dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXCHANGEWND_VERBOSETRANSMISSION, false)
										);
										verboseOutputCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
										verboseOutputCBX->setText(cb_text);
										verboseOutputCBX->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
										verboseOutputCBX->setConstraints(10, 10);
										verboseOutputCBX->setFont(
											CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
										);

										hr = verboseOutputCBX->Create();
										if (SUCCEEDED(hr))
										{
											pt.y += DPIScale(35);

											cb_text.Replace(
												getStringFromResource(UI_PROPWND_AUTOCLOSE)
											);

											auto autoCloseCBX =
												new CustomCheckbox(this->hInstance, this->iParam.outputSettingsPage, &pt, &sz, CTRLID_INOUTAUTOCLOSECBX);

											hr = (autoCloseCBX != nullptr) ? S_OK : E_FAIL;
											if (SUCCEEDED(hr))
											{
												autoCloseCBX->setChecked(
													dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXCHANGEWND_AUTOCLOSE, true)
												);
												autoCloseCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
												autoCloseCBX->setText(cb_text);
												autoCloseCBX->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
												autoCloseCBX->setConstraints(10, 10);
												autoCloseCBX->setFont(
													CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
												);

												hr = autoCloseCBX->Create();
												if (SUCCEEDED(hr))
												{
													pt.x = DPIScale(130);
													pt.y += DPIScale(103);
													sz.cx = DPIScale(100);

													// output format settings:
													// - line-end format combobox (cr,lf,cr/lf)
													// - translation-codepage (utf-8, ANSI)
													// - remove spaces
													// - linenumber-format (no linennbr, N-Word, only number)??
													// - start/stop character-index ??
													// - monitor-format (text, decimal, hexadecimal, binary)


													auto lineEndCombo = new comboBox(
														this->hInstance, this->iParam.outputSettingsPage,
														COMBOTYPE_DROPDOWNLIST, CTRLID_SERIAL_LINEENDFORMAT,
														pt.x, pt.y, sz.cx, DPIScale(30)
													);
													hr = (lineEndCombo != nullptr)
														? S_OK : E_FAIL;
													if (SUCCEEDED(hr))
													{
														if (lineEndCombo->Succeded())
														{
															iString item(L"CR/LF");
															lineEndCombo->Items->AddItem(item);
															item.Replace(L"CR");
															lineEndCombo->Items->AddItem(item);
															item.Replace(L"LF");
															lineEndCombo->Items->AddItem(item);

															lineEndCombo->setFont(
																CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
															);

															lineEndCombo->setEventHandler(dynamic_cast<comboBoxEventSink*>(this));
															lineEndCombo->setSelectedIndex(
																dataContainer->getIntegerData(DATAKEY_EXSETTINGS_EXCHANGEWND_LINEENDFORMAT, 0)
															);

															pt.x = DPIScale(20);
															pt.y += DPIScale(45);
															sz.cx = rc.right - 20;
															sz.cy = DPIScale(30);

															cb_text = getStringFromResource(UI_PROPWND_REMOVEAPOSTROPHECOMMENT);

															auto removeAPTcommentCBX =
																new CustomCheckbox(this->hInstance, this->iParam.outputSettingsPage, &pt, &sz, CTRLID_EXPORT_REMOVEAPTCOMMENT);

															hr = (removeAPTcommentCBX != nullptr) ? S_OK : E_FAIL;
															if (SUCCEEDED(hr))
															{
																removeAPTcommentCBX->setChecked(
																	dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXPORT_REMOVEAPOSTROPHECOMMENT, true)
																);
																removeAPTcommentCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
																removeAPTcommentCBX->setText(cb_text);
																removeAPTcommentCBX->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
																removeAPTcommentCBX->setConstraints(10, 10);
																removeAPTcommentCBX->setFont(
																	CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																);

																hr = removeAPTcommentCBX->Create();
																if (SUCCEEDED(hr))
																{
																	pt.y += DPIScale(35);

																	cb_text = getStringFromResource(UI_PROPWND_REMOVEBRACKETCOMMENT);

																	auto removeBRKTcommentCBX =
																		new CustomCheckbox(this->hInstance, this->iParam.outputSettingsPage, &pt, &sz, CTRLID_EXPORT_REMOVEBRCTCOMMENT);

																	hr = (removeBRKTcommentCBX != nullptr) ? S_OK : E_FAIL;
																	if (SUCCEEDED(hr))
																	{
																		removeBRKTcommentCBX->setChecked(
																			dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXPORT_REMOVEBRACKETCOMMENT, false)
																		);
																		removeBRKTcommentCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
																		removeBRKTcommentCBX->setText(cb_text);
																		removeBRKTcommentCBX->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
																		removeBRKTcommentCBX->setConstraints(10, 10);
																		removeBRKTcommentCBX->setFont(
																			CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																		);

																		hr = removeBRKTcommentCBX->Create();
																		if (SUCCEEDED(hr))
																		{
																			pt.y += DPIScale(35);

																			cb_text = getStringFromResource(UI_PROPWND_REMOVESPACES);

																			auto removeSpacesCBX =
																				new CustomCheckbox(this->hInstance, this->iParam.outputSettingsPage, &pt, &sz, CTRLID_EXPORT_REMOVESPACES);

																			hr = (removeSpacesCBX != nullptr) ? S_OK : E_FAIL;
																			if (SUCCEEDED(hr))
																			{
																				removeSpacesCBX->setChecked(
																					dataContainer->getBooleanData(DATAKEY_EXSETTINGS_EXPORT_REMOVESPACES, false)
																				);
																				removeSpacesCBX->setEventHandler(dynamic_cast<customCheckboxEventSink*>(this));
																				removeSpacesCBX->setText(cb_text);
																				removeSpacesCBX->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
																				removeSpacesCBX->setConstraints(10, 10);
																				removeSpacesCBX->setFont(
																					CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																				);

																				hr = removeSpacesCBX->Create();
																				if (SUCCEEDED(hr))
																				{
																					pt.y += DPIScale(45);

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

HRESULT CnCS_PI::_createInfoPage()
{
	RECT rc;
	GetClientRect(this->propWnd, &rc);

	this->iParam.infoPage = CreateWindow(
		CNCSPROP_PAGE_CLASS,
		nullptr,
		WS_CHILD | WS_VISIBLE,
		DPIScale(210),
		DPIScale(10),
		rc.right - DPIScale(250),
		rc.bottom - DPIScale(20),
		this->propWnd,
		reinterpret_cast<HMENU>(PAGE_ID_INFO),
		this->hInstance,
		reinterpret_cast<LPVOID>(this)
	);

	HRESULT hr = (this->iParam.infoPage != nullptr)
		? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		GetClientRect(this->iParam.infoPage, &rc);

		POINT pos;
		pos.x = DPIScale(40);
		pos.y = DPIScale(20);

		auto appNameTextField = new Textfield();
		hr =
			(appNameTextField != nullptr)
			? S_OK : E_FAIL;

		if (SUCCEEDED(hr))
		{
			appNameTextField->setText(APPLICATION_FULLVERSION_NAME);
			appNameTextField->setColors(this->sInfo.TextColor, this->sInfo.TabColor);
			appNameTextField->setFont(
				CreateScaledUnderlinedFont(26, FW_BOLD, APPLICATION_PRIMARY_FONT)
			);
			appNameTextField->setCreationFlags(Textfield::CF_APPLYSIZETOCONTEXT);

			hr = appNameTextField->Create(this->hInstance, this->iParam.infoPage, &pos, nullptr);
			if (SUCCEEDED(hr))
			{
				pos.y = DPIScale(54);

				auto copyrightTextfield = new Textfield();
				hr =
					(copyrightTextfield != nullptr)
					? S_OK : E_FAIL;

				if (SUCCEEDED(hr))
				{
					copyrightTextfield->setText(
						getStringFromResource(UI_GNRL_COPYRIGHT)
					);
					copyrightTextfield->setColors(this->sInfo.TextColor, this->sInfo.TabColor);
					copyrightTextfield->setFont(
						CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
					);
					copyrightTextfield->setCreationFlags(Textfield::CF_APPLYSIZETOCONTEXT);

					hr = copyrightTextfield->Create(this->hInstance, this->iParam.infoPage, &pos, nullptr);
					if (SUCCEEDED(hr))
					{
						pos.y = DPIScale(76);

						auto rightsReservedTextfield = new Textfield();
						hr =
							(rightsReservedTextfield != nullptr)
							? S_OK : E_FAIL;

						if (SUCCEEDED(hr))
						{
							rightsReservedTextfield->setText(
								getStringFromResource(UI_GNRL_RIGHTS)
							);
							rightsReservedTextfield->setColors(this->sInfo.TextColor, this->sInfo.TabColor);
							rightsReservedTextfield->setFont(
								CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
							);
							rightsReservedTextfield->setCreationFlags(Textfield::CF_APPLYSIZETOCONTEXT);

							hr = rightsReservedTextfield->Create(this->hInstance, this->iParam.infoPage, &pos, nullptr);
							if (SUCCEEDED(hr))
							{
								pos.y = DPIScale(98);

								auto versionTextfield = new Textfield();
								hr =
									(versionTextfield != nullptr)
									? S_OK : E_FAIL;

								if (SUCCEEDED(hr))
								{
									versionTextfield->setText(APPLICATION_VERSION);
									versionTextfield->setColors(this->sInfo.TextColor, this->sInfo.TabColor);
									versionTextfield->setFont(
										CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
									);
									versionTextfield->setCreationFlags(Textfield::CF_APPLYSIZETOCONTEXT);

									hr = versionTextfield->Create(this->hInstance, this->iParam.infoPage, &pos, nullptr);
									if (SUCCEEDED(hr))
									{
										pos.y = DPIScale(120);

										auto builddateTextfield = new Textfield();
										hr =
											(builddateTextfield != nullptr)
											? S_OK : E_FAIL;

										if (SUCCEEDED(hr))
										{
											builddateTextfield->setText(CNCSUITE_BUILDDATE);
											builddateTextfield->setColors(this->sInfo.TextColor, this->sInfo.TabColor);
											builddateTextfield->setFont(
												CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
											);
											builddateTextfield->setCreationFlags(Textfield::CF_APPLYSIZETOCONTEXT);

											hr = builddateTextfield->Create(this->hInstance, this->iParam.infoPage, &pos, nullptr);
											if (SUCCEEDED(hr))
											{
												pos.y = DPIScale(142);

												auto insttypeTextfield = new Textfield();
												hr =
													(insttypeTextfield != nullptr)
													? S_OK : E_FAIL;

												if (SUCCEEDED(hr))
												{
													iString insttype(L"Installation-Type: ");
#ifndef CNCSUITE_USERINSTALLATION
													insttype += L"admin";
#else
													insttype += L"user";
#endif // !CNCSUITE_USERINSTALLATION
													insttypeTextfield->setText(
														insttype.GetData()
													);
													insttypeTextfield->setColors(this->sInfo.TextColor, this->sInfo.TabColor);
													insttypeTextfield->setFont(
														CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
													);
													insttypeTextfield->setCreationFlags(Textfield::CF_APPLYSIZETOCONTEXT);

													hr = insttypeTextfield->Create(this->hInstance, this->iParam.infoPage, &pos, nullptr);
													if (SUCCEEDED(hr))
													{

														pos.y = rc.bottom - DPIScale(50);
														pos.x = DPIScale(25);

														SIZE sz;
														sz.cx = rc.right - DPIScale(50);
														sz.cy = DPIScale(30);

														iString bText(
															getStringFromResource(UI_PROPWND_RESETBUTTON)
														);

														auto resetButton = new CustomButton(this->iParam.infoPage, BUTTONMODE_TEXT, &pos, &sz, CTRLID_RESETAPPBUTTON, this->hInstance);
														hr =
															(resetButton != nullptr)
															? S_OK : E_FAIL;

														if (SUCCEEDED(hr))
														{
															resetButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
															resetButton->setAppearance_onlyText(bText, FALSE);
															resetButton->setBorder(TRUE, this->sInfo.OutlineColor);
															resetButton->setFont(
																CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
															);
															resetButton->setTextColor(this->sInfo.TextColor);

															if (this->sInfo.StyleID == STYLEID_LIGHTGRAY)
															{
																resetButton->setTextHighlight(
																	TRUE,
																	RGB(255, 255, 255)
																);
															}
															resetButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));

															hr = resetButton->Create();
															if (SUCCEEDED(hr))
															{
																bText = getStringFromResource(UI_GNRL_MANUAL);

																pos.x = DPIScale(60);
																pos.y = DPIScale(220);
																sz.cx = DPIScale(160);
																sz.cy = DPIScale(58);

																auto manualButton = new CustomButton(this->iParam.infoPage, BUTTONMODE_ICONTEXT, &pos, &sz, CTRLID_MANUALBUTTON, this->hInstance);
																hr =
																	(manualButton != nullptr)
																	? S_OK : E_FAIL;

																if (SUCCEEDED(hr)) {

																	auto mB_id = (this->sInfo.StyleID == STYLEID_LIGHTGRAY) ? IDI_HELPMARK_PRESSED : IDI_HELPMARK_MARKED;

																	manualButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
																	manualButton->setAppearance_IconText(mB_id, 48, bText);
																	manualButton->setBorder(TRUE, this->sInfo.OutlineColor);
																	manualButton->setFont(
																		CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																	);
																	manualButton->setTextColor(this->sInfo.TextColor);
																	manualButton->setAlignment(BAL_LEFT);
																	manualButton->setConstraints(
																		DPIScale(10), DPIScale(20)
																	);

																	if (this->sInfo.StyleID == STYLEID_LIGHTGRAY)
																	{
																		manualButton->setTextHighlight(
																			TRUE,
																			RGB(255, 255, 255)
																		);
																	}
																	manualButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));

																	hr = manualButton->Create();
																	if (SUCCEEDED(hr)) {

																		bText = getStringFromResource(UI_GNRL_WEBSITE);

																		pos.y = 320;

																		auto websiteButton = new CustomButton(this->iParam.infoPage, BUTTONMODE_ICONTEXT, &pos, &sz, CTRLID_WEBSITEBUTTON, this->hInstance);
																		hr =
																			(websiteButton != nullptr)
																			? S_OK : E_FAIL;
																		
																		if (SUCCEEDED(hr)) {

																			auto wB_id = (this->sInfo.StyleID == STYLEID_LIGHTGRAY) ? IDI_WEB_PRESSED : IDI_WEB_MARKED;

																			websiteButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
																			websiteButton->setAppearance_IconText(wB_id, 48, bText);
																			websiteButton->setBorder(TRUE, this->sInfo.OutlineColor);
																			websiteButton->setFont(
																				CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
																			);
																			websiteButton->setTextColor(this->sInfo.TextColor);
																			websiteButton->setAlignment(BAL_LEFT);
																			websiteButton->setConstraints(
																				DPIScale(10), DPIScale(20)
																			);

																			if (this->sInfo.StyleID == STYLEID_LIGHTGRAY)
																			{
																				websiteButton->setTextHighlight(
																					TRUE,
																					RGB(255, 255, 255)
																				);
																			}
																			websiteButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));

																			hr = websiteButton->Create();
																			if (SUCCEEDED(hr)) {

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
	return hr;
}

LRESULT CnCS_PI::pWProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CnCS_PI* pProp = nullptr;

	if (msg == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pProp = reinterpret_cast<CnCS_PI*>(pcr->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pProp));
		return 1;
	}
	else
	{
		pProp = reinterpret_cast<CnCS_PI*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (pProp != nullptr)
		{
			switch (msg)
			{
			case WM_COMMAND:
				return pProp->onCommandInHost(hWnd, wParam, lParam);
			case WM_CTLCOLORSTATIC:
				return reinterpret_cast<LRESULT>(pProp->iParam.staticBkgnd);
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_PAINT:
				return pProp->onPaint(hWnd);
			case WM_GETWNDINSTANCE:
				return reinterpret_cast<LRESULT>(pProp);
			case WM_CLOSE:
				return pProp->onClose();
			case WM_DESTROY:
				return pProp->onDestroy();
			default:
				return DefWindowProc(hWnd, msg, wParam, lParam);
			}
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

LRESULT CnCS_PI::pageWndProc(HWND page, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CnCS_PI* pProp = nullptr;

	if (msg == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pProp = reinterpret_cast<CnCS_PI*>(pcr->lpCreateParams);
		SetWindowLongPtr(page, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pProp));
		return 1;
	}
	else
	{
		pProp = reinterpret_cast<CnCS_PI*>(GetWindowLongPtr(page, GWLP_USERDATA));

		if (pProp != nullptr)
		{
			switch (msg)
			{
			case WM_COMMAND:
				return pProp->onCommandInPage(page, wParam, lParam);
			case WM_PAINT:
				return pProp->onPaintInPageWnd(page);
			case WM_GETWNDINSTANCE:
				return reinterpret_cast<LRESULT>(pProp);
			default:
				return DefWindowProc(page, msg, wParam, lParam);
			}
		}
		return DefWindowProc(page, msg, wParam, lParam);
	}
}

LRESULT CnCS_PI::mouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);
	else
	{
		auto mhs = reinterpret_cast<LPMOUSEHOOKSTRUCT>(lParam);
		if (mhs != nullptr)
		{
			auto pWnd = FindWindow(CS_PROPERTYWINDOW_CLASS, nullptr);
			if (pWnd != nullptr)
			{
				auto pProp = reinterpret_cast<CnCS_PI*>(GetWindowLongPtr(pWnd, GWLP_USERDATA));
				if (pProp != nullptr)
				{
					RECT rc;
					GetWindowRect(pWnd, &rc);

					if ((mhs->pt.x < rc.left) || (mhs->pt.x > rc.right) || (mhs->pt.y < rc.top) || (mhs->pt.y > rc.bottom))
					{
						if (!pProp->iParam.inputCaptured)
						{
							// only set the capture if the colorscheme-manager is not visible
							auto ecm = FindWindow(SCHEMEMANAGERCLASS, nullptr);
							if (ecm == nullptr)
							{
								SetCapture(pProp->propWnd);
								pProp->iParam.inputCaptured = TRUE;
							}
						}

						SHORT keystate1, keystate2;
						keystate1 = GetAsyncKeyState(VK_LBUTTON);
						keystate2 = GetAsyncKeyState(VK_RBUTTON);

						if ((keystate1 & 0x8000) || (keystate2 & 0x8000))
						{
							// only close the property-window when the colorscheme-manager is not visible
							auto ecm = FindWindow(SCHEMEMANAGERCLASS, nullptr);
							if (ecm == nullptr)
							{
								if (pProp->canClose()) // check if the window could be closed!
								{
									RECT propButton;
									pProp->getSettingsButtonRect(&propButton);

									// exclude the settings-button from the close-area
									if ((mhs->pt.x < propButton.left) || (mhs->pt.x > propButton.right) || (mhs->pt.y < propButton.top) || (mhs->pt.y > propButton.bottom))
									{
										PostMessage(pWnd, WM_CLOSE, 0, 0);
									}
								}
							}
						}
					}
					else
					{
						if (pProp->iParam.inputCaptured)
						{
							ReleaseCapture();
							pProp->iParam.inputCaptured = FALSE;
						}
					}
				}
			}
		}
		return CallNextHookEx(nullptr, nCode, wParam, lParam);
	}
}

LRESULT CnCS_PI::onPaint(HWND hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	HDC hdc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(hWnd, &ps);
	if (hdc)
	{
		HBRUSH bkgnd = CreateSolidBrush(this->sInfo.TabColor);
		if (bkgnd)
		{
			HBRUSH accent = CreateSolidBrush(this->sInfo.Background);
			if (accent)
			{
				RECT accent_fill;
				RECT client_fill;

				SetRect(
					&accent_fill,
					0, 0,
					DPIScale(200),
					rc.bottom
				);
				SetRect(
					&client_fill,
					DPIScale(200),
					0, rc.right, rc.bottom
				);
				FillRect(hdc, &accent_fill, accent);
				FillRect(hdc, &client_fill, bkgnd);

				if (this->sInfo.StyleID == STYLEID_DARKGRAY)
				{
					HGDIOBJ origin;

					HPEN pen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
					if (pen)
					{
						origin = SelectObject(hdc, pen);

						MoveToEx(hdc, DPIScale(200), rc.bottom - 1, nullptr);
						LineTo(hdc, rc.right - 1, rc.bottom - 1);
						LineTo(hdc, rc.right - 1, rc.top);
						LineTo(hdc, DPIScale(200), rc.top);

						SelectObject(hdc, origin);
						DeleteObject(pen);
					}
				}

				HICON icon
					= (HICON)LoadImage(
						this->hInstance, MAKEINTRESOURCE(IDI_CNCSUITE_NORM), IMAGE_ICON, DPIScale(64), DPIScale(64), LR_DEFAULTCOLOR);

				if (icon)
				{
					if (DrawIconEx(hdc, DPIScale(20), 15, icon, DPIScale(64), DPIScale(64), 0, nullptr, DI_NORMAL))
					{
						HFONT font = CreateScaledFont(34, FW_BOLD, APPLICATION_PRIMARY_FONT);

						if (font)
						{
							TextOutDC(
								hdc,
								L"CnC\0", //FND_APPNAME
								DPIScale(110),
								DPIScale(20),
								font,
								this->sInfo.TextColor
							);
							TextOutDC(
								hdc,
								L"Suite\0", //FND_APPNAME
								DPIScale(105),
								DPIScale(45),
								font,
								this->sInfo.TextColor
							);

							DeleteObject(font);
						}
					}
					DestroyIcon(icon);
				}
				DeleteObject(accent);
			}
			DeleteObject(bkgnd);
		}
		EndPaint(hWnd, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_PI::onPaintInPageWnd(HWND hWnd)
{
	HDC hdc;
	RECT rc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(hWnd, &ps);
	if (hdc)
	{
		GetClientRect(hWnd, &rc);

		HBRUSH bkgnd = CreateSolidBrush(this->sInfo.TabColor);
		if (bkgnd)
		{
			FillRect(hdc, &rc, bkgnd);

			int pageID = (int)GetWindowLongPtr(hWnd, GWLP_ID);

			switch (pageID)
			{
			case PAGE_ID_GENERAL:
				this->drawGeneralPage(hdc, &rc);
				break;
			case PAGE_ID_TABCTRL:
				this->drawTabCtrlPage(hdc, &rc);
				break;
			case PAGE_ID_FILENAV:
				this->drawFileNavPage(hdc, &rc);
				break;
			case PAGE_ID_EDITOR:
				this->drawEditorPage(hdc, &rc);
				break;
			case PAGE_ID_EXCHANGE:
				this->drawExchangePage(hdc, &rc);
				break;
			case PAGE_ID_INFO:
				this->drawInfoPage(hdc, &rc);
				break;
			case NAVPAGE_ID_OUTPUTSETTINGS:
				this->drawOutputSettingsPage(hdc, &rc);
				break;
			default:
				break;
			}
			DeleteObject(bkgnd);
		}
		EndPaint(hWnd, &ps);
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_PI::onCommandInPage(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	auto pageID = GetWindowLongPtr(hWnd, GWLP_ID);

	switch (pageID)
	{
	case PAGE_ID_GENERAL:
		switch (LOWORD(wParam))
		{
		case ASYNCMSG_CREATEDWNLBUTTON:
			this->createDownloadButton();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_PI::onCommandInHost(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case CMD_REFRESH_COLORSCHEMELIST:
		this->reloadColorSchemes();
		return 0;
	case CMD_SELECT_NEWCOLORSCHEME:
		this->selectColorSchemeFromName(
			reinterpret_cast<LPCTSTR>(lParam)
		);
		return 0;
	case ICOMMAND_SETCANCLOSE:
		this->iParam.canClose =
			static_cast<BOOL>(
				HIWORD(wParam)
			);
		break;
	case ICOMMAND_AUTOSYNTAXMANGER_CLOSED:
		this->iParam.autosyntaxWindowIsCreated = FALSE;
		if (this->iParam.currentPageId == PAGE_ID_EDITOR)
		{
			ShowWindow(this->iParam.editorPage, SW_SHOW);
		}
		return 0;
	case ICOMMAND_AUTOCOMPLETEMANAGER_CLOSED:
		this->iParam.autocompleteWindowIsCreated = FALSE;
		if (this->iParam.currentPageId == PAGE_ID_EDITOR)
		{
			ShowWindow(this->iParam.editorPage, SW_SHOW);
		}
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, WM_COMMAND, wParam, lParam);
}

LRESULT CnCS_PI::onDestroy()
{
	this->propWnd = nullptr;

	PostMessage(
		this->mainWnd,
		WM_COMMAND,
		MAKEWPARAM(ICOMMAND_PROPERTYWINDOW_CLOSED, 0),
		0
	);

	this->_resetPropWindowControlParameter();

	return static_cast<LRESULT>(0);
}

LRESULT CnCS_PI::onClose()
{
	ReleaseCapture();
	this->iParam.inputCaptured = FALSE;

	SendMessage(this->mainWnd, WM_BLOCKINPUT, (WPARAM)FALSE, 0);

	UnhookWindowsHookEx(this->iParam.mouseHook);
	this->iParam.mouseHook = nullptr;

	DestroyWindow(this->propWnd);

	return static_cast<LRESULT>(0);
}

BOOL CnCS_PI::_VerifyAppFileSystem()
{
	iString path;
	AppPath pathManager;

	auto result = this->checkSplashScreenExitCondition();
	if (result)
	{
		auto bfpo = CreateBasicFPO();
		if (bfpo != nullptr)
		{
			//this->stringToSplashScreen(
			//	getStringFromResource(UI_SSCREEN_VERIFYFILESYSTEM)
			//);
			Sleep(800); // remove!!!

	////////// check directory structure //////////////////////////////////////
			for (
				int i = PATHID_FOLDER_CNCSUITE_USERFOLDER;
				i < PATHID_ITERATION_FINAL;
				i++)
			{
				path.Replace(
					pathManager.Get(i)
				);

				if (!PathFileExists(path.GetData()))
				{
					if (!CreateDirectory(path.GetData(), nullptr))
					{
						return FALSE;
					}
				}
			}
			////////// check files ///////////////////////////////////////////////////

					// settings-file for the serial configuration >>
			path.Replace(
				pathManager.Get(PATHID_FILE_COMSETUP)
			);
			if (!PathFileExists(
				path.GetData())
				)
			{
				SerialComm serialCom;
				serialCom.setDefaultConfiguration(
					path.GetData()
				);
			}
			// appstyle id >>	// this is disabled because the styleinfo must be loaded in the constructor of the Application class -> this is before this method is executed (so this has no effect)
			//path.Replace(
			//	pathManager.get(PATHID_FILE_APPSTYLE)
			//);
			//if (!PathFileExists(path.GetData()))
			//{
			//	bfpo->SaveBufferToFileAsUtf8(
			//		L"6",
			//		path.GetData()
			//	);
			//}
			// default navigation folder
			path.Replace(
				pathManager.Get(PATHID_FILE_NAVROOT)
			);
			if (!PathFileExists(
				path.GetData()
			))
			{
				TCHAR* knownFolder = nullptr;

				if (bfpo->GetKnownFolderPath(&knownFolder, FOLDERID_Documents))
				{
					iString preceder(knownFolder);
					preceder += NAVROOT_DEFAULTFILE_CONTENT;

					bfpo->SaveBufferToFileAsUtf8(
						preceder.GetData(),
						path.GetData()
					);
					SafeDeleteArray(&knownFolder);
				}
			}

			// check for first use >>
			// ! = this must be the last action in this method, because of the return-value
			path.Replace(
				pathManager.Get(PATHID_FILE_ALREADYINSTALLED)
			);

			if (!PathFileExists(
				path.GetData()
			))
			{
				result = APPLICATION_ISFIRSTUSE;

				bfpo->SaveBufferToFileAsUtf8(
					CNCSUITE_GUID,
					path.GetData()
				);
			}
			// clean up:
			SafeRelease(&bfpo);
		}
		else
			result = FALSE;
	}
	return result;
}

BOOL CnCS_PI::_AcquireData()
{
////temp:
//	iString scheme1(L"Grey shades\0");
//	iString scheme2(L"Specific EPL2\0");
//
//	this->additionalColorSchemes.AddItem(scheme1);
//	this->additionalColorSchemes.AddItem(scheme2);
//
	auto result = this->checkSplashScreenExitCondition();
	if (result)
	{
		HRESULT hr = initDefaultApplicationDataContainer();
		if (SUCCEEDED(hr))
		{
			hr = initExtendedApplicationDataContainer();
			if (SUCCEEDED(hr))
			{
				//hr = this->stringToSplashScreen(
				//	getStringFromResource(UI_SSCREEN_INITAPPDATA)
				//);
				Sleep(800); // remove!!!

				if (SUCCEEDED(hr))
				{
					auto dataContainer
						= reinterpret_cast<ApplicationData*>(getDefaultApplicationDataContainer());

					if (dataContainer != nullptr)
					{
						int langIndex = dataContainer->getIntegerData(DATAKEY_SETTINGS_LANGUAGE, 0);
						switch (langIndex)
						{
						case 0:
							this->langID = this->getSysLanguage();
							break;
						case 1:
							this->langID = LANG_ENGLISH;
							break;
						case 2:
							this->langID = LANG_GERMAN;
							break;
						default:
							return FALSE;
						}


						// exit marks for cancellation from the user (pressing the cancel button in the startup wnd)
						// notify user in startupWnd



						//hr = this->stringToSplashScreen(
						//	getStringFromResource(UI_SSCREEN_LOADEDITRESOURCES)
						//);
						Sleep(800); // remove!!!

						if (SUCCEEDED(hr))
						{
							// initialize all edit resources

							this->autocompleteStr.fromFile(AC_EXECUTE_INLINE);


							return TRUE;
						}
					}
				}
			}
		}
	}
	// search for xml-color-schemes!!!!!!!!!!!!!
	// load autocomplete strings!!!!!!!!!!!!!!!!
	// load samples/templates!!!!!!!!!!


	return result;
}

BOOL CnCS_PI::_CheckForUpdates()
{
	__pragma(warning(suppress : 4127));
	if (AppType == APPTYPE_FREEAPP)
	{
		UpdateAgent* pUA = new UpdateAgent(this->hInstance);
		if (pUA != NULL)
		{
			pUA->setEventHandler(dynamic_cast<updateAgentEventSink*>(this));
			pUA->StartUpdateSearch(TRUE);
			return TRUE;
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}

int CnCS_PI::getSysLanguage()
{
	auto langId = GetUserDefaultLangID();
	auto primLangId = PRIMARYLANGID(langId);
	return primLangId;
}

void CnCS_PI::onButtonClick(CustomButton* sender, CTRLID ctrlID)
{
	switch (ctrlID)
	{
	case IDMM_PROPWINDOWEXIT:
		PostMessage(this->propWnd, WM_CLOSE, 0, 0);
		break;
	case IDMM_MENUSETTINGS:
		this->ReleaseButtons(ctrlID);
		this->showPage(PAGE_ID_GENERAL);
		break;
	case IDMM_MENUTABCTRL:
		this->ReleaseButtons(ctrlID);
		this->showPage(PAGE_ID_TABCTRL);
		break;
	case IDMM_MENUFILENAV:
		this->ReleaseButtons(ctrlID);
		this->showPage(PAGE_ID_FILENAV);
		break;
	case IDMM_MENUEDITOR:
		this->ReleaseButtons(ctrlID);
		this->showPage(PAGE_ID_EDITOR);
		break;
	case IDMM_MENUEXCHANGE:
		this->ReleaseButtons(ctrlID);
		this->showPage(PAGE_ID_EXCHANGE);
		break;
	case IDMM_MENUINFO:
		this->ReleaseButtons(ctrlID);
		this->showPage(PAGE_ID_INFO);
		break;
	case CTRLID_UPDATEBUTTON:
		this->onUpdateButtonClicked(sender);
		break;
	case CTRLID_NAVIGATEBACKBTN:
		this->onNavigateBack();
		break;
	case CTRLID_DELETECURRENTSCHEME:
		this->deleteSelectedScheme();
		break;
	case CTRLID_DELETEHISTORYNOW:
		PostMessage(this->propWnd, WM_CLOSE, 0, 0);
		this->deleteHistory();
		break;
	case CTRLID_MANAGECOLORSCHEMES:
	{
		auto cmanager = new colorSchemeManager(this->hInstance);
		if (cmanager != nullptr)
		{
			if (SUCCEEDED(
				cmanager->init(this->propWnd, this->mainWnd)))
			{
				this->iParam.canClose = FALSE;
			}
		}
	}
		break;
	case CTRLID_ACEDITBUTTON:
	{
		auto acManager = new autocompleteManager(this->hInstance, this->mainWnd);
		if (acManager != nullptr)
		{
			RECT rc;
			GetClientRect(this->propWnd, &rc);

			CTRLCREATIONSTRUCT cs;
			cs.parent = this->propWnd;
			cs.pos.x = DPIScale(210);
			cs.pos.y = DPIScale(10);
			cs.size.cx = rc.right - DPIScale(250);
			cs.size.cy = rc.bottom - DPIScale(20);

			auto hr = acManager->Init(&cs, &this->autocompleteStr);
			if (SUCCEEDED(hr))
			{
				this->iParam.autocompleteWindowIsCreated = TRUE;
				ShowWindow(this->iParam.editorPage, SW_HIDE);
			}
		}
	}
		break;
	case CTRLID_AUTOSYNTAXBUTTON:
	{
		auto asManager = new autosyntaxManager(this->hInstance);
		if (asManager != nullptr)
		{
			if (this->iParam.editorPage != nullptr)
			{
				RECT rc;
				GetClientRect(this->propWnd, &rc);

				CTRLCREATIONSTRUCT cs;
				cs.parent = this->propWnd;
				cs.pos.x = DPIScale(210);
				cs.pos.y = DPIScale(10);
				cs.size.cx = rc.right - DPIScale(250);
				cs.size.cy = rc.bottom - DPIScale(20);

				auto hr = asManager->Init(&cs);
				if (SUCCEEDED(hr))
				{
					this->iParam.autosyntaxWindowIsCreated = TRUE;
					ShowWindow(this->iParam.editorPage, SW_HIDE);
				}
			}
		}
	}
		break;
	case CTRLID_DEVICELISTINGBTN:
		if (this->iParam.exchangePage != nullptr)
		{
			auto deviceListing = new SerialComm(this->propWnd, this->hInstance, nullptr);
			if (deviceListing != nullptr)
			{
				RECT rc;
				POINT pos;
				SIZE size;

				GetClientRect(this->propWnd, &rc);
				pos.x = DPIScale(210);
				pos.y = DPIScale(10);
				size.cx = rc.right - DPIScale(250);
				size.cy = rc.bottom - DPIScale(20);

				auto hr = deviceListing->CreateDeviceListingWindow(
					&pos, &size,
					dynamic_cast<IDeviceSelectionProtocol*>(this)
				);
				if (SUCCEEDED(hr))
				{
					this->iParam.deviceSelectionWindowIsCreated = TRUE;
					ShowWindow(this->iParam.exchangePage, SW_HIDE);
				}
			}
		}
		break;
	case CTRLID_OUTPUTSETTINGS:
		if (this->iParam.exchangePage != nullptr)
		{
			HRESULT hr;

			if (this->iParam.outputSettingsPage == nullptr)
			{
				hr =
					this->_createOutputSettingsPage();
			}
			else
			{
				hr = S_OK;
				ShowWindow(this->iParam.outputSettingsPage, SW_SHOW);
			}

			if (SUCCEEDED(hr))
			{
				this->iParam.outputSettingsWindowIsVisible = TRUE;
				ShowWindow(this->iParam.exchangePage, SW_HIDE);
			}
		}
		break;
	case CTRLID_RESETAPPBUTTON:
		{
			auto resetAppDialog = new ResetDialog();
			if (resetAppDialog != nullptr)
			{
				resetAppDialog->SetColors(
					this->sInfo.MenuPopUpColor,
					/*(this->sInfo.StyleID == STYLEID_DARKGRAY) ? RGB(220,220,220) :*/ this->sInfo.TextColor
				);

				auto hr = resetAppDialog->Create(this->hInstance, this->mainWnd);
				if (SUCCEEDED(hr))
				{
					SendMessage(this->propWnd, WM_CLOSE, 0, 0);
				}
			}
		}
		break;
	case CTRLID_MANUALBUTTON:
		{
			auto lang = getCurrentAppLanguage();

#ifdef COMPILE_FOR_WINSTORE_DISTRIBUTION

			TCHAR path_buffer[4096] = { 0 };

			auto nSize = GetModuleFileName(nullptr, path_buffer, (DWORD)4096);
			if (nSize > 0) {

				auto bfpo = CreateBasicFPO();
				if (bfpo != nullptr) {

					if (bfpo->RemoveFilenameFromPath(path_buffer))
					{
						auto hr = PathCchRemoveFileSpec(path_buffer, (size_t)4096);
						if (SUCCEEDED(hr)) {
							iString manualPath(path_buffer);
							Url manualUrl;

							if (lang == LANG_GERMAN) {
								manualPath += L"\\manual\\CnC Suite Handbuch.html";
							}
							else
							{
								manualPath += L"\\manual\\CnC Suite Manual.html";
							}
							manualUrl.SetUrlFromLocalPath(manualPath);

							ShellExecute(
								nullptr, nullptr,
								manualUrl.GetUrl(),
								nullptr, nullptr,
								SW_SHOW
							);
						}
						SafeRelease(&bfpo);
					}
				}
			}

#else
			AppPath appPath;
			iString pathToManual;
			Url manualUrl;

			switch (lang)
			{
			case LANG_GERMAN:
				pathToManual = appPath.Get(PATHID_FILE_HELPHTML_GERMAN);
				manualUrl.SetUrlFromLocalPath(
					pathToManual
				);
				ShellExecute(
					nullptr, nullptr,
					manualUrl.GetUrl(),
					nullptr, nullptr,
					SW_SHOW
				);
				break;
			case LANG_ENGLISH:
				pathToManual = appPath.Get(PATHID_FILE_HELPHTML_ENGLISH);
				manualUrl.SetUrlFromLocalPath(
					pathToManual
				);
				ShellExecute(
					nullptr, nullptr,
					manualUrl.GetUrl(),
					nullptr, nullptr,
					SW_SHOW
				);
				break;
			default:
				break;
			}
#endif
	}
		break;
	case CTRLID_WEBSITEBUTTON:
		ShellExecute(nullptr, nullptr, L"http://cnc-suite.blogspot.com/", nullptr, nullptr, SW_SHOW);
		break;
	default:
		break;
	}
}

void CnCS_PI::onCheckboxChecked(CustomCheckbox* checkBox, bool newState)
{
	auto ctrlID = checkBox->getCtrlID();

	if ((ctrlID >= CTRLID_PARITYCHECKCBX) && (ctrlID <= CTRLID_REPLACEERRORCBX))
	{
		// handle com-setup checkboxes

		SerialComm dataT;
		SERIAL_CONFIG config;
		AppPath appPath;
		iString path = appPath.Get(PATHID_FILE_COMSETUP);
		SecureZeroMemory(&config, sizeof(SERIAL_CONFIG));
		dataT.getConfiguration(path.GetData(), &config);

		switch (ctrlID)
		{
		case CTRLID_PARITYCHECKCBX:
			config.Paritycheck = newState ? TRUE : FALSE;
			break;
		case CTRLID_ABORTONERRORCBX:
			config.Abort_on_error = newState ? TRUE : FALSE;
			break;
		case CTRLID_REPLACEERRORCBX:
			config.ErrorChar_replace = newState ? TRUE : FALSE;
			break;
		default:
			break;
		}
		dataT.setConfiguration(nullptr, &config);
	}
	else if ((ctrlID >= CTRLID_SEARCHCBX_MULTIFILE) && (ctrlID <= CTRLID_SEARCHCBX_DESC3))
	{
		// handle searchcontrol checkboxes

		AppPath aPath;
		auto sPath = aPath.Get(PATHID_FOLDER_CNCSUITE_APPDATA);

		auto sc = new Searchcontrol(nullptr, nullptr, nullptr, sPath.GetData(), nullptr, 0);
		if (sc != nullptr)
		{
			SRCHSET _Ssettings;

			if (sc->getSettings(&_Ssettings))
			{
				LRESULT val = newState ? 1 : 0;

				switch (ctrlID)
				{
				case CTRLID_SEARCHCBX_MULTIFILE:
					_Ssettings.EnableMultipleFileOpening = val;
					break;
				case CTRLID_SEARCHCBX_EXPANDPATH:
					_Ssettings.ExpandPathToFile = val;
					break;
				case CTRLID_SEARCHCBX_CLOSENOPEN:
					_Ssettings.CloseAfterOpening = val;
					break;
				case CTRLID_SEARCHCBX_FILENAME:
					_Ssettings.SF_Filename = val;
					break;
				case CTRLID_SEARCHCBX_FOLDERNAME:
					_Ssettings.SF_Projectname = val;
					break;
				case CTRLID_SEARCHCBX_DESC1:
					_Ssettings.SF_Description1 = val;
					break;
				case CTRLID_SEARCHCBX_DESC2:
					_Ssettings.SF_Description2 = val;
					break;
				case CTRLID_SEARCHCBX_DESC3:
					_Ssettings.SF_Description3 = val;
					break;
				default:
					break;
				}
				sc->setSettings(&_Ssettings);
			}
			SafeRelease(&sc);
		}
	}
	else
	{
		// handle other checkboxes
		auto dataContainer = reinterpret_cast<ApplicationData*>(getDefaultApplicationDataContainer());
		if (dataContainer != nullptr)
		{
			switch (ctrlID)
			{
			case CTRLID_UPDATECHECKBOX:
				dataContainer->saveValue(DATAKEY_SETTINGS_SEARCHFORUPDATES, newState);
				break;
			case CTRLID_SAVEEXPLORERCBX:
				dataContainer->saveValue(DATAKEY_SETTINGS_SAVE_EXPLORER_COND, newState);
				break;
			case CTRLID_SAVETABWNDCONDCBX:
				dataContainer->saveValue(DATAKEY_SETTINGS_SAVE_TABWND_COND, newState);
				break;
			case CTRLID_SAVEUNSAVEDCBX:
				dataContainer->saveValue(DATAKEY_SETTINGS_SAVE_UNSAVED_CONTENT, newState);
				break;
			case CTRLID_AUTOSAVECBX:
				dataContainer->saveValue(DATAKEY_SETTINGS_AUTOSAVE, newState);
				break;
			case CTRLID_NEWTABCBX:
				dataContainer->saveValue(DATAKEY_SETTINGS_OPEN_IN_NEW_TAB, newState);
				break;
			case CTRLID_SHOWPROPWNDCBX:
				dataContainer->saveValue(DATAKEY_SETTINGS_SHOW_DESCWND, newState);
				break;
			case CTRLID_ALWAYSEXPANDPATHCBX:
				dataContainer->saveValue(DATAKEY_SETTINGS_ALWAYS_EXPAND_PATH, newState);
				break;
			case CTRLID_SEARCHFORERRORCBX:
				dataContainer->saveValue(DATAKEY_SETTINGS_SEARCH_FOR_ERRORS, newState);
				break;
			case CTRLID_OPENCREATEDFILECBX:
				dataContainer->saveValue(DATAKEY_SETTINGS_OPENCREATEDFILE, newState);
				this->switchFileExplorerNewFileOpenSetting(newState);
				break;
			case CTRLID_USEFILETAGSASTOOLTIP:
				dataContainer->saveValue(DATAKEY_SETTINGS_USEFILETAGSASTOOLTIP, newState);
				this->switchFileExplorerInfoTipSetting(newState);
				break;
			case CTRLID_STDTEMPLATE_CBX:
				dataContainer->saveValue(DATAKEY_SETTINGS_INSERTDEFAULTTEXT, newState);
				break;
			case CTRLID_ANCHORINOUTWNDCBX:
				((ApplicationData*)(getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)))
					->saveValue(DATAKEY_EXSETTINGS_EXCHANGEWND_ANCHORWND, newState);
				break;
			case CTRLID_INOUTMONITORCBX:
				((ApplicationData*)(getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)))
					->saveValue(DATAKEY_EXSETTINGS_EXCHANGEWND_MONITORTRANSMISSION, newState);
				break;
			case CTRLID_INOUTVERBOSECBX:
				((ApplicationData*)(getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)))
					->saveValue(DATAKEY_EXSETTINGS_EXCHANGEWND_VERBOSETRANSMISSION, newState);
				break;
			case CTRLID_INOUTAUTOCLOSECBX:
				((ApplicationData*)(getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)))
					->saveValue(DATAKEY_EXSETTINGS_EXCHANGEWND_AUTOCLOSE, newState);
				break;
			case CTRLID_SAVEHISTORY:
				((ApplicationData*)(getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)))
					->saveValue(DATAKEY_EXSETTINGS_HISTORY_SAVEHISTORY, newState);
				break;
			default:
				break;
			}
		}
	}
}

void CnCS_PI::onComboboxSelChange(comboBox* combo, int selectedIndex)
{
	DWORD ctrlID = combo->getCtrlId();

	if ((ctrlID >= CTRLID_BAUDRATECOMBO) && (ctrlID <= CTRLID_RTSFLOWCOMBO))
	{
		SerialComm dataT;
		SERIAL_CONFIG config;
		AppPath appPath;
		iString path = appPath.Get(PATHID_FILE_COMSETUP);
		SecureZeroMemory(&config, sizeof(SERIAL_CONFIG));
		dataT.getConfiguration(path.GetData(), &config);

		switch (ctrlID)
		{
		case CTRLID_BAUDRATECOMBO:
			config.baud_index = selectedIndex;
			break;
		case CTRLID_PARITYCOMBO:
			config.parity_index = selectedIndex;
			break;
		case CTRLID_DATABITSCOMBO:
			config.databit_index = selectedIndex;
			break;
		case CTRLID_STOPBITSCOMBO:
			config.stopbit_index = selectedIndex;
			break;
		case CTRLID_HANDSHAKECOMBO:
			config.XON_XOFF = selectedIndex;
			break;
		case CTRLID_DTRFLOWCOMBO:
			config.DTR_control_index = selectedIndex;
			break;
		case CTRLID_RTSFLOWCOMBO:
			config.RTS_control_index = selectedIndex;
			break;
		default:
			break;
		}
		dataT.setConfiguration(nullptr, &config);
	}
	else
	{
		// handle other comboboxes

		auto dataContainer = reinterpret_cast<ApplicationData*>(getDefaultApplicationDataContainer());
		if (dataContainer != nullptr)
		{
			switch (ctrlID)
			{
			case CTRLID_DESIGNCOMBO:
				dataContainer->saveValue(DATAKEY_SETTINGS_DESIGN, selectedIndex);
				changeAppStyle(selectedIndex + 6);
				SendMessage(this->mainWnd, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&this->sInfo));
				this->menuButtons.editorButton->setColors(this->sInfo.Background, COLOR_BUTTON_MOUSE_OVER, this->sInfo.TabColor);
				this->menuButtons.editorButton->setTextColor(this->sInfo.TextColor);

				this->menuButtons.exchangeButton->setColors(this->sInfo.Background, COLOR_BUTTON_MOUSE_OVER, this->sInfo.TabColor);
				this->menuButtons.exchangeButton->setTextColor(this->sInfo.TextColor);

				this->menuButtons.filenavButton->setColors(this->sInfo.Background, COLOR_BUTTON_MOUSE_OVER, this->sInfo.TabColor);
				this->menuButtons.filenavButton->setTextColor(this->sInfo.TextColor);

				this->menuButtons.infoButton->setColors(this->sInfo.Background, COLOR_BUTTON_MOUSE_OVER, this->sInfo.TabColor);
				this->menuButtons.infoButton->setTextColor(this->sInfo.TextColor);

				this->menuButtons.settingsButton->setColors(this->sInfo.Background, COLOR_BUTTON_MOUSE_OVER, this->sInfo.TabColor);
				this->menuButtons.settingsButton->setTextColor(this->sInfo.TextColor);

				this->menuButtons.tabctrlButton->setColors(this->sInfo.Background, COLOR_BUTTON_MOUSE_OVER, this->sInfo.TabColor);
				this->menuButtons.tabctrlButton->setTextColor(this->sInfo.TextColor);

				this->menuButtons.exitButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
				this->menuButtons.exitButton->setTextColor(this->sInfo.TextColor);

				RedrawWindow(this->propWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);

				for (int i = PAGE_ID_GENERAL; i < PAGE_ID_INC_END; i++)
				{
					HWND page = GetDlgItem(this->propWnd, i);
					if (page)
					{
						DestroyWindow(page);
					}
				}
				this->_createGeneralPage();
				break;
			case CTRLID_LANGCOMBO:
				dataContainer->saveValue(DATAKEY_SETTINGS_LANGUAGE, selectedIndex);

				switch (selectedIndex)
				{
				case 0:
					this->langID = this->getSysLanguage();
					break;
				case 1:
					this->langID = LANG_ENGLISH;
					break;
				case 2:
					this->langID = LANG_GERMAN;
					break;
				default:
					this->langID = LANG_ENGLISH;
					break;
				}
				this->iParam.languageChanged = true;
				RedrawWindow(this->propWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);

				DispatchEWINotification(
					EDSP_INFO,
					L"SMP001",
					getStringFromResource(INFO_MSG_CHANGESNEEDRESTART),
					getStringFromResource(UI_APPPROPERTIES)
				);

				break;
			case CTRLID_COLORSCHEMECOMBO:
				if (selectedIndex == 2)
				{
					combo->setSelectedIndex(1);
					selectedIndex = 1;
				}
				if (selectedIndex < 2)
				{
					dataContainer->deleteValue(DATAKEY_SETTINGS_COLORSCHEME_CUR_NAME);
					dataContainer->saveValue(DATAKEY_SETTINGS_COLORSCHEME_USAGE, false);
					dataContainer->saveValue(DATAKEY_SETTINGS_COLORSCHEME_DEFINDEX, selectedIndex);
					SendMessage(this->mainWnd, WM_UPDATEEDITCOLORS, (WPARAM)FALSE, 0);

					reinterpret_cast<CustomButton*>(
						CUSTOMBUTTONCLASS_FROMID(this->iParam.editorPage, CTRLID_DELETECURRENTSCHEME)
						)->setEnabledState(false);
				}
				else
				{
					auto schemeName = combo->Items->GetAt(selectedIndex);

					dataContainer->saveValue(DATAKEY_SETTINGS_COLORSCHEME_CUR_NAME, schemeName);
					dataContainer->saveValue(DATAKEY_SETTINGS_COLORSCHEME_USAGE, true);

					SendMessage(this->mainWnd, WM_UPDATEEDITCOLORS, (WPARAM)TRUE, 0);

					reinterpret_cast<CustomButton*>(
						CUSTOMBUTTONCLASS_FROMID(this->iParam.editorPage, CTRLID_DELETECURRENTSCHEME)
						)->setEnabledState(true);
				}
				dataContainer->saveValue(DATAKEY_SETTINGS_COLORSCHEME, selectedIndex);
				break;
			case CTRLID_SERIAL_LINEENDFORMAT:
				reinterpret_cast<ApplicationData*>(
					getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
						)->saveValue(
							DATAKEY_EXSETTINGS_EXCHANGEWND_LINEENDFORMAT,
							selectedIndex
					);
				break;
			case CTRLID_EXPORT_LINEENDFORMAT:
				reinterpret_cast<ApplicationData*>(
					getApplicationDataContainerFromFilekey(FILEKEY_EXTENDED_SETTINGS)
					)->saveValue(
						DATAKEY_EXSETTINGS_EXPORT_LINEENDFORMAT,
						selectedIndex
					);
				break;
			default:
				break;
			}
		}
	}
}


void CnCS_PI::onUpdateButtonClicked(CustomButton *sender)
{
	HWND gPage = GetDlgItem(this->propWnd, PAGE_ID_GENERAL);
	if (gPage)
	{
		RECT pgRc;
		GetClientRect(gPage, &pgRc);

		RedrawWindow(
			gPage,
			nullptr, nullptr,
			RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_NOERASE);

		sender->setEnabledState(false);

		this->iParam.localUpdateSearchEnabled = true;

		this->wki = new workingIndicator(this->hInstance);

		if (this->wki != nullptr)
		{
			POINT pos;
			pos.x = DPIScale(20);
			pos.y = DPIScale(241);

			this->wki->setExtendedAnimationProperty(
				ANIMATIONMODE_LINEAR_EXTENDED,
				this->sInfo.TabColor,
				IDI_WKI_MOVER_SQ5_ORANGE
			);
			this->wki->showExtendedAnimation(
				gPage,
				&pos,
				pgRc.right - DPIScale(20)
			);
		}
		this->_CheckForUpdates();
	}
}

void CnCS_PI::onUpdateSrcComplete(bool isUpdateAvailable)
{
	if (this->iParam.localUpdateSearchEnabled)
	{
		int rslt = isUpdateAvailable ? UPDATERESULT_APPDEPRECATED : UPDATERESULT_APPUPTODATE;
	
		this->resetUpdateSearchIndication();
		this->displayUpdateSearchResult(rslt);
	}
	else
	{
		if (isUpdateAvailable)
		{
			DispatchEWINotification(
				EDSP_INFO,
				L"UA0001\0",
				getStringFromResource(INFO_MSG_UPDATEAVAILABLE),
				getStringFromResource(UI_APPPROPERTIES));
		}
	}
}

void CnCS_PI::onUpdateSrcFailed(int errorcode)
{
	if (iParam.localUpdateSearchEnabled)
	{		
		this->resetUpdateSearchIndication();	
		this->displayUpdateSearchResult(UPDATERESULT_ERRORNOTCONNECTED);
	}
	else
	{
		switch (errorcode)
		{
		case UAERR_NOINTERNETCONNECTION:
			DispatchEWINotification(
				EDSP_WARNING,
				L"UA0002",
				getStringFromResource(WARNING_MSG_UPDATESEARCH_FAILED),
				L"---\0");
			break;
		default:
			break;
		}
	}
}

void CnCS_PI::onEditContentChanged(singleLineEdit *editbox, CTRLID ctrlID)
{
	if (!this->iParam.editChangeBlockerEnabled)
	{
		// change descriptions live!
		if ((ctrlID >= CTRLID_DESCRIPTORONE) && (ctrlID <= CTRLID_DESCRIPTORTHREE))
		{
			DESCRIPTIONINFO dInfo;
			SecureZeroMemory(&dInfo, sizeof(DESCRIPTIONINFO));

			for (int i = 0; i < 3; i++)
			{
				auto edit_ = GetDlgItem(this->iParam.tabctrlPage, CTRLID_DESCRIPTORONE + i);
				if (edit_)
				{
					auto sle
						= reinterpret_cast<singleLineEdit*>(
							GetWindowLongPtr(edit_, GWLP_USERDATA));

					if (sle != nullptr)
					{
						auto content = sle->getContent();

						switch (i)
						{
						case 0:
							CopyStringToPtr(
								content.GetData(),
								&dInfo.desc1
							);
							break;
						case 1:
							CopyStringToPtr(
								content.GetData(),
								&dInfo.desc2
							);
							break;
						case 2:
							CopyStringToPtr(
								content.GetData(),
								&dInfo.desc3
							);
							break;
						default:
							break;
						}
					}
				}
			}
			SendMessage(this->mainWnd, WM_SETDESCRIPTIONS, 0, reinterpret_cast<LPARAM>(&dInfo));

			SafeDeleteArray(&dInfo.desc1);
			SafeDeleteArray(&dInfo.desc2);
			SafeDeleteArray(&dInfo.desc3);
		}
		else
		{
			if ((ctrlID >= CTRLID_COMEDIT) && (ctrlID <= CTRLID_WRITETOTALCSTEDIT))
			{
				auto str = editbox->getContent();
				if (
					(str.GetLength() == 0) || (editbox->isContentInvalid()))
					return;

				SerialComm dataT;
				SERIAL_CONFIG config;
				AppPath appPath;
				iString path = appPath.Get(PATHID_FILE_COMSETUP);
				SecureZeroMemory(&config, sizeof(SERIAL_CONFIG));
				dataT.getConfiguration(path.GetData(), &config);
				auto dataCont =
					reinterpret_cast<ApplicationData*>(
						getDefaultApplicationDataContainer()
					);

				switch (ctrlID)
				{
				case CTRLID_COMEDIT:
					config.Active_port
						=_wtoi(
							str.GetData());
					if (dataCont != nullptr)
					{
						dataCont->saveValue(DATAKEY_SETTINGS_CURRENTCOMPORT, config.Active_port);

						STBUPDATE sbu;
						sbu.action = 0;
						sbu.part = 1;
						StringCbPrintf(sbu.text, sizeof(sbu.text), L"COM %i", config.Active_port);
						SendMessage(this->mainWnd, WM_UPDATESTATUSBAR, 0, reinterpret_cast<LPARAM>(&sbu));
					}
					break;
				case CTRLID_READINTERVALEDIT:
					config.RI_Timeout
						= _wtoi(editbox->getContent().GetData());
					break;
				case CTRLID_READTOTALMPLEDIT:
					config.RT_Timeout_mpl
						= _wtoi(editbox->getContent().GetData());
					break;
				case CTRLID_READTOTALCSTEDIT:
					config.RT_Timeout_cst
						= _wtoi(editbox->getContent().GetData());
					break;
				case CTRLID_WRITETOTALMPLEDIT:
					config.WT_Timeout_mpl
						= _wtoi(editbox->getContent().GetData());
					break;
				case CTRLID_WRITETOTALCSTEDIT:
					if (editbox->getContent().Equals(L"Auto"))
					{
						config.WT_Timeout_cst = -1;
					}
					else
					{
						// TODO: check for only digits

						config.WT_Timeout_cst
							= _wtoi(
								editbox->getContent().GetData());
					}
					break;
				case CTRLID_XONCHAREDIT:
					config.XonChar = editbox->getContent().getCharAt(0);
					break;
				case CTRLID_XOFFCHAREDIT:
					config.XoffChar = editbox->getContent().getCharAt(0);
					break;
				case CTRLID_ERRORCHAREDIT:
					config.ErrorChar = editbox->getContent().getCharAt(0);
					break;
				case CTRLID_EOFCHAREDIT:
					config.EofChar = editbox->getContent().getCharAt(0);
					break;
				case CTRLID_EVENTCHAREDIT:
					config.EventChar = editbox->getContent().getCharAt(0);
					break;
				}
				dataT.setConfiguration(nullptr, &config);
			}
			else
			{
				// handle other editboxes

			}
		}
	}
}

void CnCS_PI::onNavigateBack()
{
	if (this->iParam.currentPageId == PAGE_ID_EXCHANGE)
	{
		if (IsWindowVisible(this->iParam.outputSettingsPage))
		{
			ShowWindow(this->iParam.outputSettingsPage, SW_HIDE);
			this->iParam.outputSettingsWindowIsVisible = FALSE;
			ShowWindow(this->iParam.exchangePage, SW_SHOW);
		}
	}
}

void CnCS_PI::reloadColorSchemes()
{
	auto schemes = new itemCollection<iString>();
	if (schemes != nullptr)
	{
		if (colorSchemeManager::loadSchemeList(schemes))
		{
			if (this->iParam.editorPage != nullptr)
			{
				iString normal(
					getStringFromResource(UI_PROPWND_NORMAL)
				);
				iString discreet(
					getStringFromResource(UI_PROPWND_DISCREET)
				);
				iString separator(L"--------------");

				auto combo
					= reinterpret_cast<comboBox*>(
						SendMessage(
							GetDlgItem(this->iParam.editorPage, CTRLID_COLORSCHEMECOMBO),
							WM_GETWNDINSTANCE, 0, 0)
						);
				if (combo != nullptr)
				{
					auto selIndex = combo->getSelectedIndex();

					combo->Items->Clear();
					combo->Items->AddItem(normal);
					combo->Items->AddItem(discreet);
					combo->Items->AddItem(separator);
					for (int i = 0; i < schemes->GetCount(); i++)
					{
						auto str = schemes->GetAt(i);
						combo->Items->AddItem(str);
					}

					if (selIndex < combo->Items->GetCount())
						combo->setSelectedIndex(selIndex);
					else
					{
						// update the new selected scheme in tabcontrol + save
						auto dataContainer =
							reinterpret_cast<ApplicationData*>(
								getDefaultApplicationDataContainer()
								);
						if (dataContainer != nullptr)
						{
							combo->setSelectedIndex(
								combo->Items->GetCount() - 1
							);

							dataContainer->saveValue(
								DATAKEY_SETTINGS_COLORSCHEME_CUR_NAME,
								combo->Items->GetAt(
									combo->Items->GetCount() - 1)
								.GetData()
							);								
							dataContainer->saveValue(DATAKEY_SETTINGS_COLORSCHEME_USAGE, true);

							SendMessage(this->mainWnd, WM_UPDATEEDITCOLORS, (WPARAM)TRUE, 0);
						}
					}
				}
			}
		}
	}
}

void CnCS_PI::selectColorSchemeFromName(LPCTSTR name)
{
	auto combo
		= reinterpret_cast<comboBox*>(
			SendMessage(
				GetDlgItem(this->iParam.editorPage, CTRLID_COLORSCHEMECOMBO),
				WM_GETWNDINSTANCE, 0, 0)
			);
	if (combo != nullptr)
	{
		auto itemCount = combo->Items->GetCount();
		int i;

		for (i = 3; i < itemCount; i++)
		{
			if (combo->Items->GetAt(i).Equals(name))
				break;
		}
		if (combo->Items->GetAt(i).Equals(name))//make sure this was a match, not an overflow
		{			
			// update the new selected scheme in tabcontrol + save
			auto dataContainer =
				reinterpret_cast<ApplicationData*>(
					getDefaultApplicationDataContainer()
					);
			if (dataContainer != nullptr)
			{
				combo->setSelectedIndex(i);

				dataContainer->saveValue(DATAKEY_SETTINGS_COLORSCHEME_CUR_NAME, name);
				dataContainer->saveValue(DATAKEY_SETTINGS_COLORSCHEME_USAGE, true);

				SendMessage(this->mainWnd, WM_UPDATEEDITCOLORS, (WPARAM)TRUE, 0);

				reinterpret_cast<CustomButton*>(
					CUSTOMBUTTONCLASS_FROMID(this->iParam.editorPage, CTRLID_DELETECURRENTSCHEME)
					)->setEnabledState(true);
			}
		}
	}
}

void CnCS_PI::selectColorSchemeFromIndex(int index)
{
	if (index == 2)
		index = 1;

	if (index > 2)
	{
		auto combo
			= reinterpret_cast<comboBox*>(
				SendMessage(
					GetDlgItem(this->iParam.editorPage, CTRLID_COLORSCHEMECOMBO),
					WM_GETWNDINSTANCE, 0, 0)
				);
		if (combo != nullptr)
		{
			if (index < combo->Items->GetCount())
			{
				auto sName = combo->Items->GetAt(index);

				this->selectColorSchemeFromName(
					sName.GetData()
				);
			}
		}
	}
	else
	{
		// update the new selected scheme in tabcontrol + save
		auto dataContainer =
			reinterpret_cast<ApplicationData*>(
				getDefaultApplicationDataContainer()
				);
		if (dataContainer != nullptr)
		{

			dataContainer->deleteValue(DATAKEY_SETTINGS_COLORSCHEME_CUR_NAME);
			dataContainer->saveValue(DATAKEY_SETTINGS_COLORSCHEME_USAGE, false);
			dataContainer->saveValue(DATAKEY_SETTINGS_COLORSCHEME_DEFINDEX, 1);

			SendMessage(this->mainWnd, WM_UPDATEEDITCOLORS, (WPARAM)TRUE, 0);

			reinterpret_cast<CustomButton*>(
				CUSTOMBUTTONCLASS_FROMID(this->iParam.editorPage, CTRLID_DELETECURRENTSCHEME)
				)->setEnabledState(false);
		}
	}
}

void CnCS_PI::deleteSelectedScheme()
{
	auto combo
		= reinterpret_cast<comboBox*>(
			SendMessage(
				GetDlgItem(this->iParam.editorPage, CTRLID_COLORSCHEMECOMBO),
				WM_GETWNDINSTANCE, 0, 0)
			);
	if (combo != nullptr)
	{
		auto selIndex = combo->getSelectedIndex();
		auto schemeName = combo->Items->GetAt(selIndex);

		if (schemeName.succeeded())
		{
			AppPath appPath;
			auto path = appPath.Get(PATHID_FOLDER_CNCSUITE_USERSTYLES_EDITOR);
			if (path.succeeded())
			{
				path = path + L"\\" + schemeName + L".xml";

				DeleteFile(path.GetData());

				combo->setSelectedIndex(1);

				this->reloadColorSchemes();
				this->selectColorSchemeFromIndex(1);

				reinterpret_cast<ApplicationData*>(getDefaultApplicationDataContainer())->saveValue(DATAKEY_SETTINGS_COLORSCHEME, 1);
			}
		}
	}
}

void CnCS_PI::switchFileExplorerInfoTipSetting(bool enable)
{
	auto fileExplorer =
		reinterpret_cast<CnCSuite_FileNavigator*>(
			getComponentHandle(COMP_ID_FILE_EXPLORER)
		);
	if (fileExplorer != nullptr)
	{
		fileExplorer->EnableInfoTip(enable ? TRUE : FALSE);
	}
}

void CnCS_PI::switchFileExplorerNewFileOpenSetting(bool enable)
{
	auto fileExplorer =
		reinterpret_cast<CnCSuite_FileNavigator*>(
			getComponentHandle(COMP_ID_FILE_EXPLORER)
			);
	if (fileExplorer != nullptr)
	{
		fileExplorer->EnableNewFileOpening(enable ? TRUE : FALSE);
	}
}

void CnCS_PI::ReleaseButtons(CTRLID ID_exclude)
{
	if (this->menuButtons.settingsButton != nullptr)
	{
		if (this->menuButtons.settingsButton->getID() != ID_exclude)
		{
			this->menuButtons.settingsButton->buttonDownRelease();
		}
	}
	if (this->menuButtons.tabctrlButton != nullptr)
	{
		if (this->menuButtons.tabctrlButton->getID() != ID_exclude)
		{
			this->menuButtons.tabctrlButton->buttonDownRelease();
		}
	}
	if (this->menuButtons.filenavButton != nullptr)
	{
		if (this->menuButtons.filenavButton->getID() != ID_exclude)
		{
			this->menuButtons.filenavButton->buttonDownRelease();
		}
	}
	if (this->menuButtons.editorButton != nullptr)
	{
		if (this->menuButtons.editorButton->getID() != ID_exclude)
		{
			this->menuButtons.editorButton->buttonDownRelease();
		}
	}
	if (this->menuButtons.exchangeButton != nullptr)
	{
		if (this->menuButtons.exchangeButton->getID() != ID_exclude)
		{
			this->menuButtons.exchangeButton->buttonDownRelease();
		}
	}
	if (this->menuButtons.infoButton != nullptr)
	{
		if (this->menuButtons.infoButton->getID() != ID_exclude)
		{
			this->menuButtons.infoButton->buttonDownRelease();
		}
	}
}

HRESULT CnCS_PI::showPage(int pageID)
{
	HRESULT hr = S_OK;
	this->iParam.currentPageId = pageID;

	for (
		int i = PAGE_ID_GENERAL;
		i < PAGE_ID_INC_END;
		i++)
	{
		HWND page = GetDlgItem(this->propWnd, i);
		if (i == pageID)
		{
			if (this->controlPageNavigation(pageID))// this method is responsible for inner page navigation
			{
				if (page)
					ShowWindow(page, SW_SHOW);
				else
					hr = this->_createPropWindowPage(pageID);
			}
		}
		else
		{
			if(page)
				ShowWindow(page, SW_HIDE);
		}
	}
	return hr;
}

void CnCS_PI::defineMenuButton(CustomButton* button, int iconID, iString text)
{
	button->setEventListener(dynamic_cast<customButtonEventSink*>(this));
	button->setAppearance_IconText(iconID, 32, text);											// TODO: dpi-awareness!
	button->setColors(this->sInfo.Background, COLOR_BUTTON_MOUSE_OVER, this->sInfo.TabColor);
	button->setTextColor(this->sInfo.TextColor);
	button->setConstraints(10, 10);
	button->setAlignment(BAL_LEFT);
	button->setTouchdownLock(true);
	button->setFont(
		CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
	);

	button->setTextHighlight(TRUE, RGB(255,255,255));
}

bool CnCS_PI::controlPageNavigation(int pageID)
{
	// if the user navigated to the next site from the main site, the main site should remain hidden and the active page should be shown
	// to prevent the method 'showPage()' from showing the page return false (and show the current page! do not forget to hide other subpages!)

	if (pageID == PAGE_ID_EXCHANGE)
	{
		if (this->iParam.deviceSelectionWindowIsCreated)
		{
			auto hwndDevSel = GetDlgItem(this->propWnd, CTRLID_DEVICESELECTIONWINDOW);
			if (hwndDevSel != nullptr)
			{
				this->_hideSubPages(CTRLID_DEVICESELECTIONWINDOW);
				ShowWindow(hwndDevSel, SW_SHOW);
				return false;
			}
		}
		else if (this->iParam.outputSettingsWindowIsVisible)
		{
			this->_hideSubPages(NAVPAGE_ID_OUTPUTSETTINGS);
			ShowWindow(this->iParam.outputSettingsPage, SW_SHOW);
			return false;
		}
	}
	else if (pageID == PAGE_ID_EDITOR)
	{
		if (this->iParam.autosyntaxWindowIsCreated)
		{
			auto syntaxWindow = GetDlgItem(this->propWnd, autosyntaxManager::ASWindow_ID);
			if (syntaxWindow != nullptr)
			{
				this->_hideSubPages(autosyntaxManager::ASWindow_ID);
				ShowWindow(syntaxWindow, SW_SHOW);
				return false;
			}
			else
			{
				this->iParam.autosyntaxWindowIsCreated = FALSE;
			}
		}
		else if (this->iParam.autocompleteWindowIsCreated)
		{
			auto acWindow = GetDlgItem(this->propWnd, autocompleteManager::AutoCompWindowID);
			if (acWindow != nullptr)
			{
				this->_hideSubPages(autocompleteManager::AutoCompWindowID);
				ShowWindow(acWindow, SW_SHOW);
				return false;
			}
			else
			{
				this->iParam.autocompleteWindowIsCreated = FALSE;
			}
		}
	}

	// default: ! must always be executed !
	if (this->iParam.deviceSelectionWindowIsCreated)
	{
		auto hwndDevSel = GetDlgItem(this->propWnd, CTRLID_DEVICESELECTIONWINDOW);
		if (hwndDevSel != nullptr)
		{
			ShowWindow(hwndDevSel, SW_HIDE);
		}		
	}
	if (this->iParam.outputSettingsWindowIsVisible)
	{
		ShowWindow(this->iParam.outputSettingsPage, SW_HIDE);
	}
	if (this->iParam.autosyntaxWindowIsCreated)
	{
		auto syntaxWindow = GetDlgItem(this->propWnd, autosyntaxManager::ASWindow_ID);
		if (syntaxWindow != nullptr)
		{
			ShowWindow(syntaxWindow, SW_HIDE);
		}
		else
		{
			this->iParam.autosyntaxWindowIsCreated = FALSE;
		}
	}
	if (this->iParam.autocompleteWindowIsCreated)
	{
		auto autocompWnd = GetDlgItem(this->propWnd, autocompleteManager::AutoCompWindowID);
		if (autocompWnd != nullptr)
		{
			ShowWindow(autocompWnd, SW_HIDE);
		}
		else
		{
			this->iParam.autocompleteWindowIsCreated = FALSE;
		}
	}
	return true;
}

void CnCS_PI::createStyleParams()
{
	this->iParam.staticBkgnd = CreateSolidBrush(this->sInfo.TabColor);
}

void CnCS_PI::createDownloadButton()
{
	RECT rc;
	GetClientRect(this->iParam.generalPage, &rc);
	DestroyWindow(
		GetDlgItem(this->iParam.generalPage, CTRLID_UPDATEBUTTON)
	);// auto release the old button

	iString btn_text(L"\x2192 Update\0");

	POINT pt = {
		rc.right - DPIScale(240),
		DPIScale(210)
	};
	SIZE sz = {
		DPIScale(180),
		DPIScale(30)
	};

	auto downloadButton = new CustomButton(this->iParam.generalPage, BUTTONMODE_TEXT, &pt, &sz, CTRLID_DOWNLOADBUTTON, this->hInstance);
	downloadButton->setEventListener(
		dynamic_cast<customButtonEventSink*>(this)
	);
	downloadButton->setBorder(TRUE, this->sInfo.OutlineColor);
	downloadButton->setAppearance_onlyText(btn_text, FALSE);
	downloadButton->setColors(
		this->sInfo.TabColor,
		makeSelectionColor(this->sInfo.TabColor),
		makePressedColor(this->sInfo.TabColor)
	);
	downloadButton->setTextColor(RGB(0, 255, 0));

	downloadButton->Create();
}

void CnCS_PI::registerPageWndClass()
{
	WNDCLASSEX wcx;

	if (!GetClassInfoEx(this->hInstance, CNCSPROP_PAGE_CLASS, &wcx))
	{
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = sizeof(LONG_PTR);
		wcx.lpfnWndProc = CnCS_PI::pageWndProc;
		wcx.hInstance = this->hInstance;
		wcx.hIcon = nullptr;
		wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcx.hbrBackground = nullptr;
		wcx.lpszClassName = CNCSPROP_PAGE_CLASS;
		wcx.lpszMenuName = nullptr;
		wcx.hIconSm = nullptr;

		RegisterClassEx(&wcx);
	}
}

BOOL CnCS_PI::getSettingsButtonRect(LPRECT prc)
{
	auto btn = GetDlgItem(this->mainWnd, IDM_APPSETTINGS);
	if (btn)
	{
		GetWindowRect(btn, prc);
		return TRUE;
	}
	else
		return FALSE;
}

void CnCS_PI::displayUpdateSearchResult(int type)
{
	switch (type)
	{
	case UPDATERESULT_APPUPTODATE:
		TextOutWnd(
			this->iParam.generalPage,
			getStringFromResource(UI_PROPWND_APPUPTODATE),
			DPIScale(160),
			DPIScale(180),
			this->iParam.clientFont,
			RGB(0, 255, 0)
		);
		break;
	case UPDATERESULT_APPDEPRECATED:
		PostMessage(this->iParam.generalPage, WM_COMMAND, MAKEWPARAM(ASYNCMSG_CREATEDWNLBUTTON, 0), 0);
		TextOutWnd(
			this->iParam.generalPage,
			getStringFromResource(UI_PROPWND_UPDATEAVAILABLE),
			DPIScale(160),
			DPIScale(180),
			this->iParam.clientFont,
			RGB(0, 255, 0)
		);
		break;
	case UPDATERESULT_ERRORNOTCONNECTED:
		TextOutWnd(
			this->iParam.generalPage,
			getStringFromResource(UI_PROPWND_ERRORNOINTERNETCON),
			DPIScale(160),
			DPIScale(180),
			this->iParam.clientFont,
			RGB(255, 0, 0)
		);
		break;
	default:
		break;
	}
}

void CnCS_PI::resetUpdateSearchIndication()
{
	if (this->wki != nullptr)
	{
		this->wki->killEx(0);
		this->wki = nullptr;
	}

	auto updateButton
		= reinterpret_cast<CustomButton*>(
			SendMessage(
				GetDlgItem(this->iParam.generalPage, CTRLID_UPDATEBUTTON),
				WM_GETWNDINSTANCE,
				0, 0
			)
		);

	if (updateButton != nullptr)
	{
		updateButton->setEnabledState(true);
	}
	this->iParam.localUpdateSearchEnabled = false;
}

void CnCS_PI::setInitialControlParameter()
{
	this->iParam.canClose = TRUE;
	this->iParam.inputCaptured = FALSE;
}

void CnCS_PI::deleteHistory()
{
	auto res =
		MessageBox(
			this->mainWnd,
			getStringFromResource(UI_PROPWND_CONFIRMHISTORYDELETITION),
			L"History",
			MB_OKCANCEL | MB_ICONEXCLAMATION
		);

	if (res == IDOK)
	{
		auto fileHistory =
			reinterpret_cast<UIHistory*>(
				getComponentHandle(COMP_ID_HISTORY)
				);
		if (fileHistory != nullptr)
		{
			AppPath path;

			auto historyPath =
				path.Get(PATHID_FILE_HISTORY);

			if (historyPath.succeeded())
			{
				// delete the file and clear the UI
				DeleteFile(
					historyPath.GetData()
				);
				fileHistory->ClearCompleteHistory();
			}
		}
	}
}

void CnCS_PI::drawGeneralPage(HDC hdc, LPRECT rc)
{
	HGDIOBJ original;
	HPEN pen;

	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_LANGUAGE),
		DPIScale(20),
		DPIScale(20),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);

	if (this->iParam.languageChanged)
	{
		//iString dsp(
		//	getStringFromResource(UI_PROPWND_PROGRAMRESTARTNECCESSARY)
		//);
		
		TextOutDC(
			hdc,
			getStringFromResource(UI_PROPWND_PROGRAMRESTARTNECCESSARY),
			DPIScale(300),
			DPIScale(48),
			this->iParam.clientFont,
			RGB(255, 255, 0)
		);


		//if(this->appSetup.language == LANG_GERMAN)
		//	TextOutDC(hdc, L"Programm-neustart erforderlich", 300, 48, this->iParam.clientFont, RGB(255,255,0));
		//else if (this->appSetup.language == LANG_ENGLISH)
		//	TextOutDC(hdc, L"Program-restart neccessary", 300, 48, this->iParam.clientFont, RGB(255, 255, 0));
	}

	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_DESIGN),
		DPIScale(20),
		DPIScale(100),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_UPDATING),
		DPIScale(20),
		DPIScale(180),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_ACTIONONLAUNCH),
		DPIScale(20),
		DPIScale(270),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);

	TextOutDC(
		hdc,
		getStringFromResource(UI_GNRL_HISTORY),
		DPIScale(20),
		DPIScale(440),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);


	pen = CreatePen(PS_SOLID, 1, this->sInfo.TextColor);
	if (pen)
	{
		original = SelectObject(hdc, pen);

		MoveToEx(
			hdc,
			DPIScale(20),
			DPIScale(40),
			nullptr
		);
		LineTo(hdc, rc->right - DPIScale(30), DPIScale(40));

		MoveToEx(hdc, DPIScale(20), DPIScale(120), nullptr);
		LineTo(hdc, rc->right - DPIScale(30), DPIScale(120));

		MoveToEx(hdc, DPIScale(20), DPIScale(200), nullptr);
		LineTo(hdc, rc->right - DPIScale(30), DPIScale(200));

		MoveToEx(hdc, DPIScale(20), DPIScale(290), nullptr);
		LineTo(hdc, rc->right - DPIScale(30), DPIScale(290));

		MoveToEx(hdc, DPIScale(20), DPIScale(460), nullptr);
		LineTo(hdc, rc->right - DPIScale(30), DPIScale(460));

		SelectObject(hdc, original);
		DeleteObject(pen);
	}
}

void CnCS_PI::drawTabCtrlPage(HDC hdc, LPRECT rc)
{
	HGDIOBJ original;
	HPEN pen;

	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_DOCPROPTAGDESC),
		DPIScale(20),
		DPIScale(160),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);

	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_STANDARDTEMPLATE),
		DPIScale(20),
		DPIScale(350),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);

	TextOutDC(
		hdc,
		L"Export",
		DPIScale(20),
		DPIScale(450),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);

	TextOutDC(
		hdc,
		getStringFromResource(UI_GNRL_LINEENDFORMAT),
		DPIScale(40),
		DPIScale(490),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);



	pen = CreatePen(PS_SOLID, 1, this->sInfo.TextColor);
	if (pen)
	{
		original = SelectObject(hdc, pen);

		MoveToEx(
			hdc,
			DPIScale(20),
			DPIScale(180),
			nullptr
		);
		LineTo(
			hdc,
			rc->right - DPIScale(30),
			DPIScale(180)
		);
		MoveToEx(
			hdc,
			DPIScale(20),
			DPIScale(370),
			nullptr
		);
		LineTo(
			hdc,
			rc->right - DPIScale(30),
			DPIScale(370)
		);
		MoveToEx(
			hdc,
			DPIScale(20),
			DPIScale(470),
			nullptr
		);
		LineTo(
			hdc,
			rc->right - DPIScale(30),
			DPIScale(470)
		);

		SelectObject(hdc, original);
		DeleteObject(pen);
	}
}

void CnCS_PI::drawFileNavPage(HDC hdc, LPRECT rc)
{
	HGDIOBJ original;
	HPEN pen;

	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_SEARCHSETTINGS),
		DPIScale(20),
		DPIScale(150),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_SEARCH_IN),
		DPIScale(60),
		DPIScale(310),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);

	pen = CreatePen(PS_SOLID, 1, this->sInfo.TextColor);
	if (pen)
	{
		original = SelectObject(hdc, pen);

		MoveToEx(
			hdc,
			DPIScale(20),
			DPIScale(170),
			nullptr
		);
		LineTo(
			hdc,
			rc->right - DPIScale(30),
			DPIScale(170)
		);

		MoveToEx(
			hdc,
			DPIScale(60),
			DPIScale(330),
			nullptr
		);
		LineTo(
			hdc,
			rc->right - DPIScale(120),
			DPIScale(330)
		);
		SelectObject(hdc, original);
		DeleteObject(pen);
	}
}

void CnCS_PI::drawEditorPage(HDC hdc, LPRECT rc)
{
	HGDIOBJ original;
	HPEN pen;

	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_COLORSCHEME),
		DPIScale(20),
		DPIScale(20),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_AUTOCOMPLETESTRINGS),
		DPIScale(20),
		DPIScale(100),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_AUTOSYNTAX),
		DPIScale(20),
		DPIScale(180),
		this->iParam.clientFont,
		this->sInfo.TextColor
	);
	//TextOutDC(
	//	hdc,
	//	getStringFromResource(UI_PROPWND_ERRORCONTROL),
	//	DPIScale(20),
	//	DPIScale(410),
	//	this->iParam.clientFont,
	//	this->sInfo.TextColor
	//);

	pen = CreatePen(PS_SOLID, 1, this->sInfo.TextColor);
	if (pen)
	{
		original = SelectObject(hdc, pen);

		MoveToEx(
			hdc,
			DPIScale(20),
			DPIScale(40),
			nullptr
		);
		LineTo(
			hdc,
			rc->right - DPIScale(30),
			DPIScale(40)
		);
		MoveToEx(
			hdc,
			DPIScale(20),
			DPIScale(120),
			nullptr
		);
		LineTo(
			hdc,
			rc->right - DPIScale(30),
			DPIScale(120)
		);
		MoveToEx(
			hdc,
			DPIScale(20),
			DPIScale(200),
			nullptr
		);
		LineTo(
			hdc,
			rc->right - DPIScale(30),
			DPIScale(200)
		);
		//MoveToEx(
		//	hdc,
		//	DPIScale(20),
		//	DPIScale(430),
		//	nullptr
		//);
		//LineTo(
		//	hdc,
		//	rc->right - DPIScale(30),
		//	DPIScale(430)
		//);
		SelectObject(hdc, original);
		DeleteObject(pen);
	}

}

void CnCS_PI::drawExchangePage(HDC hdc, LPRECT rc)
{
	HGDIOBJ original;
	HPEN pen;

	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_INTERFACECONFIG),
		DPIScale(20),
		DPIScale(20),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_BAUDRATE),
		DPIScale(60),
		DPIScale(92),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_PARITY), 
		DPIScale(60),
		DPIScale(122),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_DATABITS),
		DPIScale(60),
		DPIScale(152),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_STOPBITS),
		DPIScale(60),
		DPIScale(182),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_FLOWCTRL),
		DPIScale(60),
		DPIScale(212),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_GNRL_ADVANCED),
		DPIScale(20),
		DPIScale(350),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_DTRFLOWCONTROL),
		DPIScale(40),
		DPIScale(382),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_RTSFLOWCONTROL),
		DPIScale(40),
		DPIScale(412),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc, 
		getStringFromResource(UI_PROPWND_CHARACTERS), 
		DPIScale(40),
		DPIScale(450),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		L"Timeouts (ms)",
		DPIScale(220),
		DPIScale(450),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		L"Read",
		DPIScale(250),
		DPIScale(485),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		L"Write",
		DPIScale(250),
		DPIScale(535),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	pen = CreatePen(PS_SOLID, 1, this->sInfo.TextColor);
	if (pen)
	{
		original = SelectObject(hdc, pen);

		MoveToEx(hdc, DPIScale(20), DPIScale(40), nullptr);
		LineTo(hdc, rc->right - DPIScale(30), DPIScale(40));

		MoveToEx(hdc, DPIScale(60), DPIScale(88), nullptr);
		LineTo(hdc, DPIScale(370), DPIScale(88));

		MoveToEx(hdc, DPIScale(60), DPIScale(238), nullptr);
		LineTo(hdc, DPIScale(370), DPIScale(238));

		MoveToEx(hdc, DPIScale(20), DPIScale(370), nullptr);
		LineTo(hdc, rc->right - DPIScale(30), DPIScale(370));

		MoveToEx(hdc, DPIScale(40), DPIScale(470), nullptr);
		LineTo(hdc, DPIScale(125), DPIScale(470));
		LineTo(hdc, DPIScale(125), rc->bottom - DPIScale(5));

		MoveToEx(hdc, DPIScale(220), DPIScale(470), nullptr);
		LineTo(hdc, rc->right - DPIScale(30), DPIScale(470));
		LineTo(hdc, rc->right - DPIScale(30), rc->bottom - DPIScale(5));

		MoveToEx(hdc, DPIScale(250), DPIScale(505), nullptr);
		LineTo(hdc, DPIScale(350), DPIScale(505));
		MoveToEx(hdc, DPIScale(350), DPIScale(480), nullptr);
		LineTo(hdc, DPIScale(350), DPIScale(530));
		LineTo(hdc, DPIScale(353), DPIScale(530));
		MoveToEx(hdc, DPIScale(350), DPIScale(480), nullptr);
		LineTo(hdc, DPIScale(353), DPIScale(480));

		MoveToEx(hdc, DPIScale(250), DPIScale(555), nullptr);
		LineTo(hdc, DPIScale(350), DPIScale(555));
		MoveToEx(hdc, DPIScale(350), DPIScale(540), nullptr);
		LineTo(hdc, DPIScale(350), DPIScale(570));
		LineTo(hdc, DPIScale(353), DPIScale(570));
		MoveToEx(hdc, DPIScale(350), DPIScale(540), nullptr);
		LineTo(hdc, DPIScale(353), DPIScale(540));

		SelectObject(hdc, original);
		DeleteObject(pen);
	}
}

void CnCS_PI::drawOutputSettingsPage(HDC hdc, LPRECT rc)
{
	HGDIOBJ original;
	HPEN pen;

	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_OUTWINDOWSETTINGS),
		DPIScale(20),
		DPIScale(20),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_PROPWND_OUTFORMATSETTINGS),
		DPIScale(20),
		DPIScale(220),
		this->iParam.clientFont, this->sInfo.TextColor
	);
	TextOutDC(
		hdc,
		getStringFromResource(UI_GNRL_LINEENDFORMAT),
		DPIScale(40),
		DPIScale(260),
		this->iParam.clientFont, this->sInfo.TextColor
	);


	pen = CreatePen(PS_SOLID, 1, this->sInfo.TextColor);
	if (pen)
	{
		original = SelectObject(hdc, pen);

		MoveToEx(hdc, DPIScale(20), DPIScale(40), nullptr);
		LineTo(hdc, rc->right - DPIScale(30), DPIScale(40));

		MoveToEx(hdc, DPIScale(20), DPIScale(240), nullptr);
		LineTo(hdc, rc->right - DPIScale(30), DPIScale(240));

		SelectObject(hdc, original);
		DeleteObject(pen);
	}
}

void CnCS_PI::drawInfoPage(HDC hdc, LPRECT rc)
{
	// TODO: draw lines!!!!

	// temp
	UNREFERENCED_PARAMETER(hdc);
	UNREFERENCED_PARAMETER(rc);
}

HRESULT CnCS_PI::stringToSplashScreen(LPTSTR string_)
{
	HWND sScreen = FindWindow(SPLASHSCREENCLASS, nullptr);
	if (sScreen != nullptr)
	{
		auto result =
			SendMessage(
			sScreen,
			WM_COMMAND,
			MAKEWPARAM(ICOMMAND_SETOUTPUT_STRING, 0),
			reinterpret_cast<LPARAM>(string_)
		);
		return result ? S_OK : E_ABORT;
	}
	return S_OK;
}

BOOL CnCS_PI::checkSplashScreenExitCondition()
{
	HWND sScreen = FindWindow(SPLASHSCREENCLASS, nullptr);
	if (sScreen != nullptr)
	{
		auto result =
			SendMessage(
				sScreen,
				WM_COMMAND,
				MAKEWPARAM(ICOMMAND_GETEXITCONDITION, 0),
				0
			);
		return static_cast<BOOL>(!result); // if the user has pressed the exitbutton -> return FALSE
	}
	return FALSE;
}

void CnCS_PI::_createDpiDependendResources()
{
	if (this->iParam.clientFont != nullptr)
		DeleteObject(this->iParam.clientFont);

	this->iParam.clientFont = CreateScaledFont(20, FW_MEDIUM, APPLICATION_PRIMARY_FONT);
}

void CnCS_PI::_resetPropWindowControlParameter()
{
	this->propWnd = nullptr;
	this->iParam.generalPage = nullptr;
	this->iParam.fileNavPage = nullptr;
	this->iParam.tabctrlPage = nullptr;
	this->iParam.editorPage = nullptr;
	this->iParam.exchangePage = nullptr;
	this->iParam.outputSettingsPage = nullptr;
	this->iParam.infoPage = nullptr;
	this->iParam.deviceSelectionWindowIsCreated = FALSE;
	this->iParam.outputSettingsWindowIsVisible = FALSE;
	this->iParam.autocompleteWindowIsCreated = FALSE;
	this->iParam.autosyntaxWindowIsCreated = FALSE;

	SecureZeroMemory(&this->menuButtons, sizeof(CNCSPROP_MENUBUTTONS));
}

void CnCS_PI::_hideSubPages(int excludeId)
{
	if (excludeId != CTRLID_DEVICESELECTIONWINDOW)
	{
		if (this->iParam.deviceSelectionWindowIsCreated)
		{
			auto hwndDevSel = GetDlgItem(this->propWnd, CTRLID_DEVICESELECTIONWINDOW);
			if (hwndDevSel != nullptr)
			{
				ShowWindow(hwndDevSel, SW_HIDE);
			}
		}
	}
	if (excludeId != NAVPAGE_ID_OUTPUTSETTINGS)
	{
		if (this->iParam.outputSettingsWindowIsVisible)
		{
			ShowWindow(this->iParam.outputSettingsPage, SW_HIDE);
		}
	}
	if (excludeId != autosyntaxManager::ASWindow_ID)
	{
		if (this->iParam.autosyntaxWindowIsCreated)
		{
			auto syntaxWindow = GetDlgItem(this->propWnd, autosyntaxManager::ASWindow_ID);
			if (syntaxWindow != nullptr)
			{
				ShowWindow(syntaxWindow, SW_HIDE);
			}
		}
	}
	if (excludeId != autocompleteManager::AutoCompWindowID)
	{
		if (this->iParam.autocompleteWindowIsCreated)
		{
			auto acWindow = GetDlgItem(this->propWnd, autocompleteManager::AutoCompWindowID);
			if (acWindow != nullptr)
			{
				ShowWindow(acWindow, SW_HIDE);
			}
		}
	}
}
