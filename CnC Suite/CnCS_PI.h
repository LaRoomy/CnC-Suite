#pragma once
#include"CnCSuite_Property.h"
#include"CommonControls\CustomButton.h"
#include"CommonControls\CustomCheckbox.h"
#include"CommonControls\ComboBox.h"
#include"CommonControls\SingleLineEdit.h"
#include"DataExchange\DataExchange.h"
#include"UpdateAgent.h"
#include"BasicFPO.h"
#include"autocompleteStrings.h"
#include"Graphics\workingIndicator.h"

#define			CNCSPROP_PAGE_CLASS			L"CNCSPROPERTY_PAGE_CLASS\0"

#define			NUM_SETUP_PAGES				6

#define			UPDATERESULT_APPUPTODATE		10
#define			UPDATERESULT_APPDEPRECATED		11
#define			UPDATERESULT_ERRORNOTCONNECTED	12

#define			ASYNCMSG_CREATEDWNLBUTTON		50

#define			IDMM_PROPWINDOWEXIT			100

// !! Note that menubutton and page -ID's must have consecutive values for incrementation !!

#define			IDMM_MENUSETTINGS			105
#define			IDMM_MENUTABCTRL			106
#define			IDMM_MENUFILENAV			107
#define			IDMM_MENUEDITOR				108
#define			IDMM_MENUEXCHANGE			109
#define			IDMM_MENUINFO				110

// MAINPAGES:
#define			PAGE_ID_GENERAL				150
#define			PAGE_ID_TABCTRL				151
#define			PAGE_ID_FILENAV				152
#define			PAGE_ID_EDITOR				153
#define			PAGE_ID_EXCHANGE			154
#define			PAGE_ID_INFO				155
#define			PAGE_ID_INC_END				156

// SUBPAGES:
#define			NAVPAGE_ID_OUTPUTSETTINGS	190

// Functions related to Navigation:
// - onButtonClick(..			(initally shows the pages)
// - onNavigateBack(..			(only responsible for the control of the windows created in this component)
// - controlPageNavigation(..	(as the name says, it controls the between main and subpages - the method is called from showPage(...)!)
// - _hideSubPages(...			(hides the subpages when another subpage must be shown)

// CONTROL-IDs:
#define			CTRLID_LANGCOMBO			200
#define			CTRLID_DESIGNCOMBO			201
#define			CTRLID_UPDATECHECKBOX		202
#define			CTRLID_UPDATEBUTTON			203
#define			CTRLID_DOWNLOADBUTTON		204
#define			CTRLID_SAVEEXPLORERCBX		205
#define			CTRLID_SAVETABWNDCONDCBX	206
#define			CTRLID_SAVEUNSAVEDCBX		207
#define			CTRLID_AUTOSAVECBX			208
#define			CTRLID_NEWTABCBX			209
#define			CTRLID_SHOWPROPWNDCBX		210
#define			CTRLID_DESCRIPTORONE		211
#define			CTRLID_DESCRIPTORTWO		212
#define			CTRLID_DESCRIPTORTHREE		213
#define			CTRLID_ALWAYSEXPANDPATHCBX	214
#define			CTRLID_COLORSCHEMECOMBO		215
#define			CTRLID_ACEDITBUTTON			216
#define			CTRLID_SEARCHFORERRORCBX	217
#define			CTRLID_COMEDIT				218
#define			CTRLID_BAUDRATECOMBO		219
#define			CTRLID_PARITYCOMBO			220
#define			CTRLID_DATABITSCOMBO		221
#define			CTRLID_STOPBITSCOMBO		222
#define			CTRLID_HANDSHAKECOMBO		223
#define			CTRLID_PARITYCHECKCBX		224
#define			CTRLID_ABORTONERRORCBX		225
#define			CTRLID_REPLACEERRORCBX		226
#define			CTRLID_DTRFLOWCOMBO			227
#define			CTRLID_RTSFLOWCOMBO			228
#define			CTRLID_XONCHAREDIT			229
#define			CTRLID_XOFFCHAREDIT			230
#define			CTRLID_ERRORCHAREDIT		231
#define			CTRLID_EOFCHAREDIT			232
#define			CTRLID_EVENTCHAREDIT		233
#define			CTRLID_READINTERVALEDIT		234
#define			CTRLID_READTOTALMPLEDIT		235
#define			CTRLID_READTOTALCSTEDIT		236
#define			CTRLID_WRITETOTALMPLEDIT	237
#define			CTRLID_WRITETOTALCSTEDIT	238
#define			CTRLID_MANAGECOLORSCHEMES	239
#define			CTRLID_OPENCREATEDFILECBX	240
#define			CTRLID_USEFILETAGSASTOOLTIP	241
#define			CTRLID_SEARCHCBX_MULTIFILE	242
#define			CTRLID_SEARCHCBX_EXPANDPATH	243
#define			CTRLID_SEARCHCBX_CLOSENOPEN	244
#define			CTRLID_SEARCHCBX_FILENAME	245
#define			CTRLID_SEARCHCBX_FOLDERNAME	246
#define			CTRLID_SEARCHCBX_DESC1		247
#define			CTRLID_SEARCHCBX_DESC2		248
#define			CTRLID_SEARCHCBX_DESC3		249
#define			CTRLID_AUTOSYNTAXBUTTON		250
#define			CTRLID_STDTEMPLATE_CBX		251
#define			CTRLID_STDTEMPLATE_EDIT		252
#define			CTRLID_DEVICELISTINGBTN		253
#define			CTRLID_OUTPUTSETTINGS		254
#define			CTRLID_NAVIGATEBACKBTN		255
#define			CTRLID_ANCHORINOUTWNDCBX	256
#define			CTRLID_INOUTVERBOSECBX		257
#define			CTRLID_INOUTMONITORCBX		258
#define			CTRLID_INOUTAUTOCLOSECBX	259
#define			CTRLID_RESETAPPBUTTON		260
#define			CTRLID_SERIAL_LINEENDFORMAT	261
#define			CTRLID_EXPORT_LINEENDFORMAT	262
#define			CTRLID_DELETECURRENTSCHEME	263
#define			CTRLID_EXPORT_REMOVEAPTCOMMENT	264
#define			CTRLID_EXPORT_REMOVEBRCTCOMMENT	265
#define			CTRLID_EXPORT_REMOVESPACES		266
#define			CTRLID_SAVEHISTORY				267
#define			CTRLID_DELETEHISTORYNOW			268


typedef struct _CNCSPROP_MENUBUTTONS {

	CustomButton* settingsButton;
	CustomButton* tabctrlButton;
	CustomButton* filenavButton;
	CustomButton* editorButton;
	CustomButton* exchangeButton;
	CustomButton* infoButton;
	CustomButton* exitButton;

}CNCSPROP_MENUBUTTONS, *LPCNCSMENUBUTTONS;

typedef struct _CNCSPROP_PARAMS {

	HBRUSH staticBkgnd;
	HFONT clientFont;
	int currentPageId;

	bool localUpdateSearchEnabled;
	bool editChangeBlockerEnabled;
	bool languageChanged;

	HWND generalPage;
	HWND tabctrlPage;
	HWND fileNavPage;
	HWND editorPage;
	HWND exchangePage;
	HWND outputSettingsPage;
	HWND infoPage;

	HHOOK mouseHook;
	BOOL canClose;
	BOOL deviceSelectionWindowIsCreated;
	BOOL outputSettingsWindowIsVisible;
	BOOL autocompleteWindowIsCreated;
	BOOL autosyntaxWindowIsCreated;
	BOOL inputCaptured;

}CNCSPROP_PARAMS, *LPCNCSPROP_PARAMS;

class CnCS_PI
	: public CnCSuite_Property,
	public customButtonEventSink,
	public customCheckboxEventSink,
	public comboBoxEventSink,
	public updateAgentEventSink,
	public singleLineEditEventSink,
	public autocompleteStringLoadingEventSink,
	public IDeviceSelectionProtocol
{
public:
	CnCS_PI();
	~CnCS_PI();

	// interface methods
	void CnCS_PI::Release() { delete this; }
	HRESULT CnCS_PI::CreatePropertyWindow(HWND Main, HINSTANCE hInst) {
		this->hInstance = hInst;
		this->mainWnd = Main;
		return this->_createPropertyWindow(Main);
	}
	BOOL CnCS_PI::VerifyApplicationFileSystem() { return this->_VerifyAppFileSystem(); }
	BOOL CnCS_PI::AcquireData() { return this->_AcquireData(); }
	LanguageID CnCS_PI::GetUserLanguage() {
		return this->langID;
	}
	BOOL CnCS_PI::CheckForUpdates() { return this->_CheckForUpdates(); }
	LPVOID CnCS_PI::GetAutocompleteStrings(int* count) {
		*count = this->autocompleteStr.GetCount();
		return reinterpret_cast<LPVOID>(this->autocompleteStr.getReferenceToStruct());
	}
	HRESULT CnCS_PI::StringToSplashScreen(LPTSTR string_) { return this->stringToSplashScreen(string_); }
	BOOL CnCS_PI::isPropWindowOpen(HWND *hwndOut) {
		*hwndOut = this->propWnd;
		return (this->propWnd != nullptr) ? TRUE : FALSE;
	}
	void CnCS_PI::onDpiChanged() {

		if (this->propWnd != nullptr)
		{
			SendMessage(this->propWnd, WM_CLOSE, 0, 0);
		}
		this->_createDpiDependendResources();
	}
	BOOL canClose() { return this->iParam.canClose; }


	// button eventhandler
	void CnCS_PI::onCustomButtonClick(cObject sender, CTRLID ID) {
		this->onButtonClick(
			reinterpret_cast<CustomButton*>(sender), ID);
	}

	// checkbox eventhandler
	void CnCS_PI::onCustomCheckboxChecked(cObject sender, bool newState) {
		this->onCheckboxChecked(
			reinterpret_cast<CustomCheckbox*>(sender), newState);
	}

	// comboBox eventhandler
	void CnCS_PI::onComboBoxSelectionChanged(cObject sender, int selIndex) {
		this->onComboboxSelChange(
			reinterpret_cast<comboBox*>(sender), selIndex);
	}

	// updateAgent eventhandler
	void CnCS_PI::onUpdateSearchComplete(bool isUpdateAvailable) {
		this->onUpdateSrcComplete(isUpdateAvailable);
	}
	void CnCS_PI::onUpdateSearchFailed(int errorcode) {
		this->onUpdateSrcFailed(errorcode);
	}

	// editbox events
	void CnCS_PI::onEditboxContentChanged(cObject sender, CTRLID ctrlID) {
		this->onEditContentChanged(
			reinterpret_cast<singleLineEdit*>(sender), ctrlID);
	}
	void CnCS_PI::onTabKeyWasPressed(cObject sender, CTRLID ctrlID) {
		
		UNREFERENCED_PARAMETER(sender);
		UNREFERENCED_PARAMETER(ctrlID);
	}

	// autocomplete eventhandler
	void CnCS_PI::acStringLoadingComplete(cObject sender) {
		UNREFERENCED_PARAMETER(sender);
	}

	// device-selction events
	void OnDeviceSelected(cObject sender, iString& friendlyName) {
		 auto port = SerialComm::ExtractComPortFromDeviceName(friendlyName);
		 if(port > 0)
		 {
			 auto comEdit = singleLineEdit::GetSingleLineEditObject(this->iParam.exchangePage, CTRLID_COMEDIT);
			 if (comEdit != nullptr)
			 {
				 auto portStr = iString::fromInt(port);
				 comEdit->setContent(portStr);
			 }
		 }
		 UNREFERENCED_PARAMETER(sender);
	}

	void OnQuit(cObject sender) {
		reinterpret_cast<SerialComm*>(sender)->DestroyDeviceListingWindow();
		this->iParam.deviceSelectionWindowIsCreated = FALSE;
		ShowWindow(this->iParam.exchangePage, SW_SHOW);
	}

private:
	BOOL Success;
	HWND mainWnd;
	HWND propWnd;
	HINSTANCE hInstance;

	LanguageID langID;

	APPSTYLEINFO sInfo;
	CNCSPROP_MENUBUTTONS menuButtons;
	CNCSPROP_PARAMS iParam;

	autocompleteStrings autocompleteStr;
	workingIndicator *wki;

	HRESULT _createPropertyWindow(HWND);
	HRESULT _createFrameControls();
	HRESULT _createPropWindowPage(int pageId);

	HRESULT showPage(int);

	HRESULT _createGeneralPage();
	HRESULT _createTabCtrlPage();
	HRESULT _createFileNavPage();
	HRESULT _createEditorPage();
	HRESULT _createDataExPage();
	HRESULT _createOutputSettingsPage();
	HRESULT _createInfoPage();

	static LRESULT CALLBACK pWProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK pageWndProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK mouseProc(int, WPARAM, LPARAM);

	LRESULT onPaint(HWND);
	LRESULT onPaintInPageWnd(HWND);
	LRESULT onCommandInPage(HWND, WPARAM, LPARAM);
	LRESULT onCommandInHost(HWND, WPARAM, LPARAM);
	LRESULT onDestroy();
	LRESULT onClose();

	BOOL _VerifyAppFileSystem();
	BOOL _AcquireData();
	BOOL _CheckForUpdates();

	int getSysLanguage();

	void onButtonClick(CustomButton*, CTRLID);
	void onCheckboxChecked(CustomCheckbox* checkBox, bool);
	void onComboboxSelChange(comboBox* combo, int);
	void onUpdateButtonClicked(CustomButton*);
	void onUpdateSrcComplete(bool);
	void onUpdateSrcFailed(int);
	void onEditContentChanged(singleLineEdit*, CTRLID);
	void onNavigateBack();

	void reloadColorSchemes();
	void selectColorSchemeFromName(LPCTSTR name);
	void selectColorSchemeFromIndex(int index);
	void deleteSelectedScheme();
	void switchFileExplorerInfoTipSetting(bool);
	void switchFileExplorerNewFileOpenSetting(bool);

	void ReleaseButtons(CTRLID);

	void defineMenuButton(CustomButton*, int, iString);
	bool controlPageNavigation(int pageID);

	void createStyleParams();
	void createDownloadButton();
	void registerPageWndClass();
	BOOL getSettingsButtonRect(LPRECT);
	
	void displayUpdateSearchResult(int type);
	void resetUpdateSearchIndication();
	void setInitialControlParameter();

	void drawGeneralPage(HDC, LPRECT);
	void drawTabCtrlPage(HDC, LPRECT);
	void drawFileNavPage(HDC, LPRECT);
	void drawEditorPage(HDC, LPRECT);
	void drawExchangePage(HDC, LPRECT);
	void drawOutputSettingsPage(HDC, LPRECT);
	void drawInfoPage(HDC, LPRECT);

	HRESULT stringToSplashScreen(LPTSTR);
	BOOL checkSplashScreenExitCondition();// returns FALSE if the user has pressed the exitbutton

	void _createDpiDependendResources();
	void _resetPropWindowControlParameter();
	void _hideSubPages(int excludeId);
};

CnCSuite_Property* CreateCnCSuiteProperty()
{
	return new CnCS_PI();
}
