#pragma once
#include"CnCSuite_FileNavigator.h"
#include"TVComp\TreeViewClass.h"
#include"OpenSave\Open_Save_CTRL.h"
#include"Searchcontrol\SearchControl.h"
#include"BasicFPO.h"
#include"AppPath.h"
#include"SampleManager\SampleManager.h"
#include<CustomButton.h>
#include<CustomPopUpMenu.h>

#define		ID_SLNEWROOTFOLDER		49
#define		ID_NEWPROGRAMM			50
#define		ID_NEWFOLDER			51
#define		ID_COPY					52
#define		ID_INSERT				53
#define		ID_DELETE				54
#define		ID_CONVERT				55
#define		ID_IMPORT				56
#define		ID_SEARCH				57
#define		ID_RENAME				58
#define		ID_OPEN					59
#define		ID_OPEN_IN_NEW_TAB		60

#define		ID_EXPLORER				61
#define		ID_TEMPLATE				62

#define		ID_DATACOLLECTIONSTARTED	71
//#define		ID_DATACOLLECTIONFINISHED	62
#define		ID_DATAINITIALIZATIONSTARTED	73
//#define		ID_DATAPROGRESSSTEP			64

#define		ID_PROGRESSDIALOG_SHUTDOWMTIMER		10


#define ID_TOOLWINDOW	99
#define	TOOLWNDCLASS	L"CTVTOOLWND_CLASS"

#define		MAX_TBBUTTON		9

#define		Button_SetFont(hButton,hFont)		SNDMSG((HWND)hButton,WM_SETFONT,(WPARAM)(HFONT)hFont,MAKELPARAM(TRUE,0))

typedef struct _FILENAVPARAMS {
	HWND Frame;
	HWND Main;
	HFONT font;
	HFONT buttonfont;
	HFONT tabFont;
	HHOOK hHook;
	HBRUSH background;
	HBRUSH markBrush;
	HBRUSH buttonbrush;
	HBRUSH toolbarBrush;

	BOOL UseInfoTip;
	BOOL toolbarDualLine;
	BOOL dataCollectionComplete;

	UINT32 interruptFileSystemWatcher;

	HICON searchico;
	HICON newfolderico;

}FILENAVPARAMS, *LPFILENAVPARAMS;

typedef struct _FILESYSTEMWATCHERTHREADDATA
{
	TCHAR* root_path;
	LONG_PTR pToClass;

}FILESYSTEMWATCHERTHREADDATA, *LPFILESYSTEMWATCHERTHREADDATA;

class CnCS_FN
	: public CnCSuite_FileNavigator,
	public ITreeViewItemLoadingProgress,
	public customButtonEventSink,
	public customPopUpMenuEventSink,
	public ITreeViewUserEvents
{
public:
	CnCS_FN(HINSTANCE,HWND);
	~CnCS_FN();

	HRESULT CnCS_FN::Init(HWND NavFrame, LPTSTR AppDirectory) { return this->_Init(NavFrame, AppDirectory); }
	void CnCS_FN::AddNewFileToView(LPTSTR path, BOOL expand) { this->addNewFileToView(path, expand); }
	void CnCS_FN::EnableInfoTip(BOOL enable) { this->onEnableInfoTip(enable); }
	void CnCS_FN::EnableNewFileOpening(BOOL enable) { this->onEnableNewFileOpening(enable); }

	void CnCS_FN::ExpandPathToFile(LPCTSTR path) { this->_expandPath(path); }

	void CnCS_FN::SaveCondition() {
		if (this->pTV != nullptr)
		{
			AppPath aPath;			
			this->pTV->SaveExpandImage(
				aPath.Get(PATHID_FOLDER_CNCSUITE_APPDATA_SESSION)
				.GetData()
			);
		}
	}

	void CnCS_FN::LoadCondition() {
		if (this->pTV != nullptr)
		{
			AppPath aPath;
			this->pTV->LoadExpandImage(
				aPath.Get(PATHID_FOLDER_CNCSUITE_APPDATA_SESSION)
				.GetData()
			);
		}
	}

	void CnCS_FN::ClearCondition() {
		if (this->pTV != nullptr)
		{
			AppPath aPath;
			this->pTV->clearExpandImage(
				aPath.Get(PATHID_FOLDER_CNCSUITE_APPDATA_SESSION)
				.GetData()
			);
		}
	}
	void CnCS_FN::Release() { delete this; }

	void CnCS_FN::Reload() {
		this->pTV->Reload(true);
	}

	void CnCS_FN::ReloadAsync(){
		this->pTV->ReloadAsync();
	}

	// treeview interface methods
	void CnCS_FN::dataCollectionStarted(){
		PostMessage(
			this->fnParam.Frame,
			WM_COMMAND,
			MAKEWPARAM(ID_DATACOLLECTIONSTARTED, 0),
			0
		);
	}
	void CnCS_FN::dataCollectionFinished(int ItemCount){
		this->onTVItemDataCollectionFinished(ItemCount);
	}
	void CnCS_FN::startInitializingItems(int nCount){
		PostMessage(
			this->fnParam.Frame,
			WM_COMMAND,
			MAKEWPARAM(ID_DATAINITIALIZATIONSTARTED, 0),
			static_cast<LPARAM>(nCount)
		);
	}
	void CnCS_FN::itemAdded(){
		this->onTVItemAdded();
	}
	void CnCS_FN::operationComplete(){
		this->onTVOperationComplete();
	}
	void CnCS_FN::operationFailed(){
		this->onTVOperationFailed();
	}
	
	void CnCS_FN::onDpiChanged() { this->OnDpiChanged(); }

	void CnCS_FN::onCustomButtonClick(cObject sender, CTRLID ctrlID){
		this->onButtonClick(
			reinterpret_cast<CustomButton*>(sender),
			ctrlID
		);
	}
	
	void CnCS_FN::onPopUpMenuButtonClicked(CTRLID Id){

		SendMessage(
			this->fnParam.Frame,
			WM_COMMAND,
			MAKEWPARAM(Id, 0),
			0
		);
	}

	void CnCS_FN::onSelectionChanged(int type, LPCTSTR itemPath)
	{
		UNREFERENCED_PARAMETER(itemPath);
		this->setButtonEnabledStateFromSelectionType(type);
	}

	void CnCS_FN::onItemMoved(int type, LPCTSTR new_path, LPCTSTR old_path) {

		if (this->FileSystemEvents != nullptr)
		{
			FILESYSTEMOBJECT fso;
			fso.Path = new_path;
			fso.OldPath = old_path;
			fso.FileSystemObjectType = (type == PARENT__ITEM) ? FSO_TYPE_FOLDER : FSO_TYPE_FILE;
			fso.FileSystemOperation = FSO_ITEM_MOVED;

			this->FileSystemEvents->onFilesysItemMoved(
				reinterpret_cast<cObject>(this),
				&fso
			);
		}
	}

	void CnCS_FN::onItemRenamed(int type, LPCTSTR new_path, LPCTSTR old_path) {

		if (this->FileSystemEvents != nullptr)
		{
			FILESYSTEMOBJECT fso;
			fso.Path = new_path;
			fso.OldPath = old_path;
			fso.FileSystemObjectType = (type == PARENT__ITEM) ? FSO_TYPE_FOLDER : FSO_TYPE_FILE;
			fso.FileSystemOperation = FSO_ITEM_RENAMED;

			this->FileSystemEvents->onFilesysItemRenamed(
				reinterpret_cast<cObject>(this),
				&fso
			);
		}
	}

	void CnCS_FN::onItemDeleted(int type, LPCTSTR itemPath) {

		UNREFERENCED_PARAMETER(type);
		UNREFERENCED_PARAMETER(itemPath);

		// do nothing here, because the filesystem-watcher will call the event !!!

		// NOTE:	if a filesystem-item will be moved, there is no 'move' notification. Instead there will be a delete + add notification
		//			-> so if an item will be moved from the inside of this application, the filesystem-watcher must be blocked!!!


		//if (this->FileSystemEvents != nullptr)
		//{
		//	FILESYSTEMOBJECT fso;
		//	fso.Path = itemPath;
		//	fso.FileSystemObjectType = (type == PARENT__ITEM) ? FSO_TYPE_FOLDER : FSO_TYPE_FILE;
		//	fso.FileSystemOperation = FSO_ITEM_DELETED;

		//	this->FileSystemEvents->onFilesysItemDeleted(
		//		reinterpret_cast<cObject>(this),
		//		&fso
		//	);

		//}
	}

	void CnCS_FN::SetEventHandler(IFileSystemModificationProtocoll* FileSystemEvents_) {
		this->FileSystemEvents = FileSystemEvents_;
	}

private:
	HINSTANCE hInstance;
	FILENAVPARAMS fnParam;

	TreeViewCTRL* pTV;
	Searchcontrol* pSRCH;
	SampleManager* smpManager;

	APPSTYLEINFO styleInfo;

	IFileSystemModificationProtocoll* FileSystemEvents;

	HRESULT _Init(HWND, LPTSTR);
	HRESULT CreateCtrlButtons();
	HRESULT CreateSampleManager();
	HRESULT CreateTBButtons();

	static LRESULT CALLBACK FileNavProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
	static LRESULT CALLBACK MouseProc(int, WPARAM, LPARAM);
	static DWORD __stdcall fileSystemWatcher(LPVOID lParam);

	LRESULT OnSize();
	LRESULT OnConvert(LPARAM);
	LRESULT OnCommand(HWND, WPARAM, LPARAM);
	LRESULT OnEraseBkgnd(HWND,WPARAM);
	LRESULT OnCleanUp(WPARAM);
	LRESULT OnSetAppStyle(LPARAM);
	LRESULT OnValidateError(LPARAM);
	LRESULT OnTimer(WPARAM);

	void OnCreatePopupMenu(int);
	BOOL StartSearch();
	void addNewFileToView(LPTSTR,BOOL);
	void startFileSystemWatcher(LPCTSTR root);

	void SelectNewRootFolder();
	void InsertNewProgram();
	void InsertNewFolder();
	void OnCopyItem();
	void OnInsertItem();
	void OnConvertItem();
	void OnRenameItem();
	void OnImportItem();
	void OnDeleteItem();
	void OnOpenItem(bool openInNewTab);
	void OnDpiChanged();

	void onEnableInfoTip(BOOL);
	void onEnableNewFileOpening(BOOL);

	void _expandPath(LPCTSTR path);
	
	void hideSampleManager();
	void showSampleManager();
	void resizeToolbar();

	void onTVItemDataCollectionStarted();
	void onTVItemDataCollectionFinished(int nItems);
	void onTVItemInitalizingStarted(int nItems);
	void onTVItemAdded();
	void onTVOperationComplete();
	void onTVOperationFailed();

	void onButtonClick(CustomButton* button, CTRLID ctrlID);

	void _createDpiDependendResources();
	void _defineMenuEntry(MenuEntry &entry, int eID, int iconID, int iconSQsize, LPCTSTR text);

	void setButtonEnabledStateFromSelectionType(int type);
	DWORD getDWEnableInfo(int type);

	void onFileSystemChanged(BYTE* buffer, DWORD max_buffer);
	void onRootFolderInvalidated();
};

CnCSuite_FileNavigator* CreateFileNavigator(HINSTANCE hInst, HWND MainWindow) { return new CnCS_FN(hInst, MainWindow); }