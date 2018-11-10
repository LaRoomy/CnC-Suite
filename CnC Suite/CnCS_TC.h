#pragma once
#include"CnCSuite_Tabcontrol.h"
#include"EditControl.h"
#include"HelperF.h"
#include"BasicFPO.h"
#include<StringClass.h>
#include<ComboBox.h>
#include<CustomButton.h>
#include<CustomTrackbar.h>
#include<CustomCheckbox.h>
#include<CustomPopUpMenu.h>
#include"Async.h"

#define		TAB_MAXIMUM			-1

#define		NO_EXECUTE			0x00
#define		DO_CLEANUP			0x01
#define		SET_DISPLAYNAME		0x02

#define		REDRAW_CURRENT		3
#define		REDRAW_ALL			5

#define		ID_TABCLOSE					20
#define		ID_DESCFRAME				30
#define		ID_PROPERTYEDIT_ONE			34
#define		ID_PROPERTYEDIT_TWO			35
#define		ID_PROPERTYEDIT_THREE		36

#define		COMMONTABWIDTH		DPIScale(260)
#define		MINIMUMTABWIDTH		DPIScale(100)


#define		IDM_CTLPROPWND		300
#define		IDM_VT_UNDO			301
#define		IDM_VT_REDO			302
#define		IDM_UPPERCASE		303
#define		IDM_AUTONUM			304
#define		IDM_AUTOCOMPLETE	305
#define		IDM_TEXTCLR			306
#define		IDM_FOCUSRECT		307
#define		IDM_ECONVERTALL		308
#define		IDM_NUMSEQUENCE		309
#define		IDM_ERRORCHECK		310
#define		IDM_AUTOSYNTAX		311
#define		IDM_FONTPROPERTIES	312

#define		IDMX_FILESYSTEMCHANGED		315


#define		IDM_LINESPACETRACK	320
#define		IDM_FONTBOLDCHECKBX	321
#define		IDM_FONTHEIGHTTRACK	322
#define		IDM_FONTFAMILYCOMBO	323


//////////////////////////////////////////
// keep the successive ID's for iteration!
#define		IDM_DESCWND_COPY		330
#define		IDM_DESCWND_INSERT		331
#define		IDM_DESCWND_DIAMETER	332
#define		IDM_DESCWND_DELETE		333
//////////////////////////////////////////

#define		IDM_PMENU_EXPANDTOFILE	350
#define		IDM_PMENU_OPENINWINEXP	351
#define		IDM_PMENU_ERASETAB		352
#define		IDM_PMENU_EXPORTAS		353
#define		IDM_PMENU_COPYTONEWTAB	354
#define		IDM_PMENU_SETTEMPLATE	355


#define		DESCFRAME_CREATESTATIC	400
#define		DESCFRAME_CREATEDYNAMIC	401

#define		PROPBUTTON_CREATE		410
#define		PROPBUTTON_UPDATE		411

#define		PROPWND_SOFTOUT			412
#define		PROPWND_SOFTIN			413

#define		POPUPTYPE_FONTPROPERTY	420

#define		FILEINFOAREA_STARTHEIGHT 350

#define		TOOLWINDOWPOPUPCLASS		L"TOOLPOPUPCLASS"

typedef struct _TCGDIOBJECTS {

	HFONT Tabfont;
	HFONT desc_propertyfont;

	HICON tabclose_red;
	HICON tabclose_green;
	HICON tabclose_marked;
	HICON cnc3;
	HICON cnc3_marked;
	HICON cnc3_disabled;

	HBRUSH tabbkgnd;
	HBRUSH selTabbkgnd;
	HBRUSH framebkgnd;
	HBRUSH toolbarbkgnd;

	HPEN cPen;

	COLORREF Textcolor;

}TCGDIOBJECTS, *LPTCGDIOBJECTS;

typedef struct _TABPROPERTY {

	LONG_PTR toClass;
	HWND Tab;
	HWND AssocEditWnd;
	TCHAR* Path;
	TCHAR* displayname;
	DWORD TabIndex;
	EditControl* Editcontrol;
	BOOL Content_Changed;
	BOOL Closebutton_highl;
	BOOL mouseHover;
	BOOL menuButtonHover;

	TCHAR *DESC1;
	TCHAR *DESC2;
	TCHAR *DESC3;

}TABPROPERTY, *LPTABPROPERTY;

typedef struct _DESCCLIPBOARD {

	TCHAR* d1;
	TCHAR* d2;
	TCHAR* d3;

}DESCCLIPBOARD, *LPDESCCLIPBOARD;

typedef struct _TCINTERNALPARAMETER {

	POINT ActiveTabRange;
	DWORD dwActiveTab;
	HWND hwndActiveTab;
	DWORD previousTab;
	BOOL ChangeActiveTab;
	BOOL UseXMLFileForUserColors;
	BOOL OpenNewPathInNewTab;
	BOOL AutoSave;
	BOOL PopupIsOpen;
	BOOL PopupCloseBlocker;
	BOOL SkipNextFilesystemAction;
	DWORD popupType;

	int StartupAction;

	TCHAR *WorkingDirectory;

	BOOL uppercase_active;
	BOOL autocomplete_active;
	BOOL autonum_active;
	BOOL textcolor_active;
	BOOL focusmark_active;

	BOOL descWnd_visible;
	BOOL descWnd_exists;
	BOOL descWnd_isMoving;

	int currentStyle;
	int TBButtonIDdrawFor;

	TCHAR *DESC_property1;
	TCHAR *DESC_property2;
	TCHAR *DESC_property3;

	HWND propTooltip;
	HWND currentOpenedPopUp;

	HHOOK mouseHook;

	int FontHeight;
	int FontFamilyIndex;
	int lineOffset;
	BOOL isBold;

	AUTOSYNTAXPROPERTY autoSyntax;
	DESCCLIPBOARD descClipboard;

	RECT excludeRect;
	BOOL excludeRectIsValid;

}TCINTERNALPARAMETER, *LPINTERNALPARAMETER;

typedef struct _TCTHREADDATA {
	DWORD Mode;
	int additional_val;
}TCTHREADDATA, *LPTCTHREADDATA;

//typedef struct { int tl; int tr; int tm; int tl_h; int tr_h; int tm_h; }TABBMPIDS, *LPTABBMPIDS;

class CnCS_TC
	: public CnCSuite_Tabcontrol,
	public comboBoxEventSink,
	public customButtonEventSink,
	public trackBarEventSink,
	public customCheckboxEventSink,
	public customPopUpMenuEventSink,
	public IFileDialogUserEvents
{
public:
	CnCS_TC(HINSTANCE, HWND);
	~CnCS_TC();

	HRESULT CnCS_TC::Init(HWND Frame, LPTCSTARTUPINFO tc_info) { return this->_Init(Frame, tc_info); }
	BOOL CnCS_TC::GetProperty(LPTCPROPERTY TabControlProp) { return this->_GetProp(TabControlProp); }
	void CnCS_TC::GetCurrentCnC3File(CnC3File& file);
	BOOL CnCS_TC::GetCurrentTabContent(TCHAR** content) {
		return GetRichEditContent(
			this->GetCurrentVisibleEditbox(this->TCFrame), content);
	}
	BOOL CnCS_TC::GetCurrentSelectedText(LPSELECTIONINFO selInfo) { return this->getCurrentSelectedText(selInfo); }
	LPCTSTR CnCS_TC::GetCurrentFilename() {
		return
			this->GetActiveTabProperty()->displayname;
	}
	void CnCS_TC::UserRequest_AddNewTab() { this->ADD_Tab(nullptr); }
	void CnCS_TC::UserRequest_Open(LPTSTR path, BOOL ForceOpenInNewTab, BOOL setFocus) { this->Open(path, ForceOpenInNewTab, setFocus); }

	void CnCS_TC::UserRequest_Open(const CnC3File& file, bool forceOpenInNewTab, bool setFocus);

	BOOL CnCS_TC::UserRequest_SaveAs(TCHAR** path) {  return this->SaveAs(path, nullptr); }
	BOOL CnCS_TC::UserRequest_Save(DWORD mode) { return this->Save(mode, nullptr); }
	void CnCS_TC::UserRequest_Import(LPCTSTR content) { this->Import(content); }
	void CnCS_TC::UserRequest_InsertText(LPTSTR text) { this->Insert(text); }
	BOOL CnCS_TC::UserRequest_CloseApp() { return this->onClose(); }
	void CnCS_TC::UserRequest_SetNewColorScheme(BOOL setUserdefinedColor) {
		UNREFERENCED_PARAMETER(setUserdefinedColor);
		this->ChangeColorAsync();
	}

	void CnCS_TC::UserAction_Copy() {
		SendMessage(
			this->GetCurrentVisibleEditbox(nullptr),
			WM_COPY, 0, 0
		);	
	}
	void CnCS_TC::UserAction_Clear() {
		SendMessage(
			this->GetCurrentVisibleEditbox(nullptr),
			WM_CLEAR, 0, 0
		);	
	}
	void CnCS_TC::UserAction_Cut() {
		SendMessage(
			this->GetCurrentVisibleEditbox(nullptr),
			WM_CUT, 0, 0
		);		
	}
	void CnCS_TC::UserAction_Paste() {
		SendMessage(
			this->GetCurrentVisibleEditbox(nullptr),
			WM_PASTE, 0, 0
		);
	}
	void CnCS_TC::UserAction_SelectAll() {
		CHARRANGE cr = { 0,-1 };
		SendMessage(
			this->GetCurrentVisibleEditbox(nullptr),
			EM_EXSETSEL,
			0,
			reinterpret_cast<LPARAM>(&cr)
		);
	}
	void CnCS_TC::UserAction_Keyboard(WPARAM vKey) { this->onKeydown(vKey); }
	BOOL CnCS_TC::GetDescriptionNames(LPDESCRIPTIONINFO dInfo) { return this->onGetDescriptionNames(dInfo); }
	void CnCS_TC::SetDescriptionNames(LPDESCRIPTIONINFO dInfo) { this->onSetDescriptionNames(dInfo); }
	void CnCS_TC::SetFocusOnCurrentVisibleEditbox() {
		this->setFocusOncurEdit();
	}
	HWND CnCS_TC::GetFrameHandle() {
		return this->TCFrame;
	}
	HWND CnCS_TC::GetCurrentVisibleEditboxHandle() {
		return this->GetCurrentVisibleEditbox(this->TCFrame);
	}
	void CnCS_TC::GetEditControlProperties(UINT_PTR editstylecolors, LONG_PTR editcontrolproperties) {
		this->getEditcontrolProperties(
			reinterpret_cast<LPEDITSTYLECOLORS>(editstylecolors),
			reinterpret_cast<LPEDITCONTROLPROPERTIES>(editcontrolproperties)
		);
	}
	void CnCS_TC::ICommand(WPARAM wParam, LPARAM lParam) {
		this->OnCommand_inFrame(wParam, lParam);
	}
	void CnCS_TC::GetCurrentTabDataAsStringCollection(itemCollection<iString>& data);
	void CnCS_TC::UpdateAutocompleteData() { this->updateAutocompleteData(); }
	void CnCS_TC::UpdateAutosyntaxSettings() { this->updateAutosyntaxSettings(); }
	void CnCS_TC::SaveTabCondition() { this->saveCondition(); }
	void CnCS_TC::RestoreTabCondition() { this->restoreCondition(); }
	void CnCS_TC::onDpiChanged() { this->DpiChanged(); }
	void CnCS_TC::Release() { delete this; }

	// combobox event
	void CnCS_TC::onComboBoxSelectionChanged(cObject sender, int selIndex){
		this->comboboxSelChange(reinterpret_cast<comboBox*>(sender), selIndex);
	}

	// button events
	void CnCS_TC::onCustomButtonClick(cObject sender, CTRLID ctrlID){
		this->onButtonClick(
			reinterpret_cast<CustomButton*>(sender),
			ctrlID
		);
	}

	// trackbar events
	void CnCS_TC::trackBar_TrackEnd(cObject sender, int trackPos) {
		this->onTrackbarTrackEnd(
			reinterpret_cast<customTrackbar*>(sender),
			trackPos
		);
	}
	void __fastcall CnCS_TC::trackBar_Tracking(cObject sender, int trackPos) {
		this->onTrackbarTracking(
			reinterpret_cast<customTrackbar*>(sender),
			trackPos
		);
	}
	void CnCS_TC::trackBar_TrackStarted(cObject sender, int trackPos) {
		UNREFERENCED_PARAMETER(sender);
		UNREFERENCED_PARAMETER(trackPos);
	}

	// checkbox events
	void CnCS_TC::onCustomCheckboxChecked(cObject sender, bool newState) {
		this->onCheckboxChecked(
			reinterpret_cast<CustomCheckbox*>(sender),
			newState
		);
	}

	// pop-menu events
	void CnCS_TC::onPopUpMenuButtonClicked(CTRLID Id){
		this->onMenuButtonClick(Id);
	}

	// Filesystem modification
	void CnCS_TC::FileSystemChanged(LPFILESYSTEMOBJECT fso) {

		if (fso != nullptr)
		{
			auto threadID = GetCurrentThreadId();

			if (threadID != this->_thisThreadId)
			{
				SendMessage(
					this->TCFrame,
					WM_COMMAND,
					MAKEWPARAM(IDMX_FILESYSTEMCHANGED, 0),
					reinterpret_cast<LPARAM>(fso)
				);
			}
			else
			{
				this->onFileSystemChanged(fso);
			}
		}
	}

	// IFileDialogUserEvent Base
	void SaveAsPathWasSelected(cObject sender, LPCTSTR path);

private:
	HINSTANCE hInstance;
	HWND TCFrame;
	HWND Main;
	DWORD _thisThreadId;

	DWORD TABCount;

	TCGDIOBJECTS tcObj;
	TCINTERNALPARAMETER iParam;
	EDITSTYLECOLORS editStyleColors;
	APPSTYLEINFO styleInfo;
	iString defaultInsertText;

	itemCollection<iString> availableFonts;

	HRESULT _Init(HWND,LPTCSTARTUPINFO);
	HRESULT InitComponents(LPTCSTARTUPINFO);
	HRESULT RegisterTCClasses();
	HRESULT SaveStartupInfo(LPTCSTARTUPINFO);
	HRESULT CreateTBButton(DWORD, int, int, PTSTR);
	HRESULT CreateDescFrame(DWORD);
	HRESULT InitFrameProperty();
	HRESULT ControlPropertyButton(DWORD);
	HRESULT CreateFontPropertyBox();
	HRESULT CreateToolHostWindowFromButtonPosition(int,int,int);
	HRESULT OpenTabMenu();

	static LRESULT CALLBACK TabFrameProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
	static LRESULT CALLBACK TabProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK ToolPopupProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK CloseBtnSub(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
	static LRESULT CALLBACK TBBtnSub(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
	static LRESULT CALLBACK DescEditSub(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
	static LRESULT CALLBACK DescWndProc(HWND, UINT, WPARAM, LPARAM);
	static DWORD WINAPI PropWndSoftControlProc(LPVOID);
	static DWORD WINAPI changeColorProc(LPVOID);
	static LRESULT CALLBACK MouseProc(int, WPARAM, LPARAM);

	LRESULT OnSize_inFrame();
	LRESULT OnPaint_inFrame(HWND);
	LRESULT OnCommand_inFrame(WPARAM, LPARAM);
	LRESULT OnSize_inTab(HWND);
	LRESULT OnLButtonDown_inTab(HWND, LPARAM);
	LRESULT OnPaint_inTab(HWND, LPTABPROPERTY);
	LRESULT OnDrawItem_inTab(HWND, LPARAM, LPTABPROPERTY);
	LRESULT OnMousemove_inTab(HWND,LPARAM);
	LRESULT OnCommand_inTab(HWND, WPARAM);
	LRESULT OnSetAppStyle(LPARAM);
	LRESULT OnDrawItem_inFrame(LPARAM);
	LRESULT OnValidateError(LPARAM);
	LRESULT OnPaint_inDescWnd(HWND);
	LRESULT OnSize_inPropWnd(HWND);
	LRESULT OnNotify_inFrame(HWND, WPARAM, LPARAM);
	LRESULT OnGetEditWndPos(LPARAM);
	LRESULT OnMouseLeave_inTab(HWND);
	LRESULT OnPaintInToolPopup(HWND);

	void OnVerticalToolBar_Textcolor();
	void OnVerticalToolBar_Autocomplete();
	void OnVerticalToolBar_Autonum();
	void OnVerticalToolBar_Uppercase();
	void OnVerticalToolBar_FocusRect();
	void OnVerticalToolBar_eConvertAll();
	void OnVerticalToolBar_AutoSyntax();
	void OnVerticalToolBar_NumSequence();
	void OnVerticalToolBar_ErrorCheck();
	void OnVerticalToolBar_Undo();
	void OnVerticalToolBar_Redo();

	BOOL ADD_Tab(LPCTSTR path);				// old!
	BOOL AddTab(const CnC3File& file);		// new!

	BOOL CLOSE_Tab(HWND Tab, BOOL performSavecheck);

	void selectTab(DWORD tabNr);
	void eraseTab(DWORD tabNr);
	void eraseTab(LPTABPROPERTY ptp);

	void Open(LPTSTR path, BOOL openInNewTab, BOOL setFocus);
	BOOL SaveAs(TCHAR** path, LPTABPROPERTY);
	BOOL Save(DWORD, LPTABPROPERTY);
	void Import(LPCTSTR);
	void Insert(LPCTSTR);
	void OnFileDeleted(LPCTSTR path);
	void OnFolderDeleted(LPCTSTR path);
	void OnFileRenamed(LPCTSTR path_new, LPCTSTR path_old);
	void OnFolderRenamed(LPCTSTR path_new, LPCTSTR path_old);
	void OnFileSystemMoveOperation(LPFILESYSTEMOBJECT fso);
	void OnFileSystemDeleteOperation(LPFILESYSTEMOBJECT fso);
	void OnFileSystemRenameOperation(LPFILESYSTEMOBJECT fso);
	void SetDescriptions(LPTSTR, LPTSTR, LPTSTR);
	void SaveDescritpionsToTabProperty(LPTABPROPERTY);
	void CleanUpTabStructForNewContent(LPTABPROPERTY);
	BOOL onClose();
	void onSetNewColorScheme();
	void onKeydown(WPARAM);
	void onButtonClick(CustomButton* button, CTRLID ctrlID);
	void __fastcall onTrackbarTracking(customTrackbar* _trackbar, int _trackPos);
	void onTrackbarTrackEnd(customTrackbar* _trackbar, int _trackPos);
	void onCheckboxChecked(CustomCheckbox* cbx, bool _newState);
	void onFileSystemChanged(LPFILESYSTEMOBJECT fso);
	void onMenuButtonClick(CTRLID Id);
	void onFocusRectOffsetChanged();

	BOOL _GetProp(LPTCPROPERTY);

	void saveCondition();
	void restoreCondition();

	BOOL SaveCheck_singleTab(LPTABPROPERTY);
	BOOL CalculateInsertTabrange(LPPOINT);
	BOOL DrawTabText(HDC, HWND, LPTSTR);
	BOOL DrawTBButton(LPDRAWITEMSTRUCT, int);

	void RefreshTabAlignment();
	void RefreshTBButtons();
	void RedrawTab_s(DWORD mode);
	void SetActiveTab();
	void OnEditHasChanged(HWND);
	void ControlPropertyWindow();
	void ChangeColorAsync();

	DWORD GetTabWidth(DWORD);
	HBRUSH GetTBButtonBkgndBrush(int);
	
	//void SetBitmapIDs(LPTABBMPIDS, int);
	void SetEditStyleColorsFromStyleInfo(LPEDITSTYLECOLORS, LPAPPSTYLEINFO);
	bool SetEditStyleColorsFrom_XMLFile(LPEDITSTYLECOLORS);
	void SetEditControlProperties(LPEDITCONTROLPROPERTIES);

	BOOL InitializeValues(LPTCSTARTUPINFO);
	void LoadFontResource();

	BOOL LoadDESCPropertyNames();
	BOOL SaveDESCPropertyNames();

	BOOL PropWndSoftControl(DWORD Mode);

	// the hwnd parameter can be nullptr, but only in the same thread!!!
	HWND GetCurrentVisibleEditbox(HWND);

	LPTABPROPERTY GetActiveTabProperty();
	LPTABPROPERTY GetTabProperty(DWORD);
	BOOL ConfigPropEdit(HWND edit);

	//BOOL OpenDisplayAndFormat(LPCTSTR, DWORD);		// old!
	BOOL OpenDisplayAndFormat(const CnC3File& file, DWORD flags);
	BOOL getCurrentSelectedText(LPSELECTIONINFO);

	BOOL onGetDescriptionNames(LPDESCRIPTIONINFO dInfo);
	void onSetDescriptionNames(LPDESCRIPTIONINFO dInfo);

	void getEditcontrolProperties(LPEDITSTYLECOLORS, LPEDITCONTROLPROPERTIES);
	void setAutosyntaxProperties(LPAUTOSYNTAXPROPERTY pSyntax);
	void updateAutosyntaxSettings();
	void initDescriptionBuffer(LPTABPROPERTY);

	void updateAutocompleteData();
	void comboboxSelChange(comboBox* cBox, int selindex);

	BOOL isPathAlreadyOpen(LPTSTR path);			// old!
	bool isAlreadyOpened(const CnC3File& file);		// new!

	void setFocusOncurEdit();
	void insertDiameterSymbol();
	void tagsToInternalClipboard();
	void clipboardToTags();
	void deleteAllTags();
	void setSelectedTabBackground();
	LPCTSTR getDefaultInsertText();
	bool displayFileInfos(HDC hdc, int fromPos);
	void updateFileInfoArea();
	void eraseFileInfoArea(HDC hdc);

	void _createDpiDependendResources();
	void _defineMenuEntry(MenuEntry &entry, int eID, int iconID, int iconSQsize, LPCTSTR text);
	void DpiChanged();
	void ResizeEditWindow(HWND);
	void SetTabContentFocusAndPushCaret(LPTABPROPERTY ptp, LPCTSTR defaultText, BOOL setFocus, CaretIndex cIndex);
	void closeTabIfPathIsEqual(LPCTSTR path);
	void clearToolWindowPopupControlParameterAndDestroy();
};

CnCSuite_Tabcontrol* CreateTabcontrol(HINSTANCE hInst, HWND MainWindow) { return new CnCS_TC(hInst, MainWindow); }
