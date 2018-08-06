#pragma once
#include"..//external.h"
#include"..//HelperF.h"

#define		CX_ICON				24
#define		CY_ICON				24
#define		NUM_ICONS			3

#define		A__FILE				7
#define		A__FOLDER			8
#define		A__CNC3FILE			9
#define		A__EMPTYITEM		10
#define		EMPTY_DIR			11
#define		ROOT_DIR			12

#define		PARENT__ITEM		13
#define		CHILD__ITEM			14
#define		CNC3__ITEM			15
#define		EMPTY__ITEM			16
#define		ROOT__ITEM			17

#define		INIT_MODE			18
#define		RELOAD_MODE			19

#define		COUNT				30
#define		READ				31
#define		FOP_MOVE			32
#define		FOP_COPY			33
#define		FOP_RENAME			34
#define		FOP_DELETE			35
#define		FOP_NEWFILE			36
#define		FOP_NEWFOLDER		37
#define		FOP_INSERT			38
#define		FOP_CONVERT			39
#define		FOP_IMPORT			40

#define		DYNAMICMODE_GETPARENTITEMFROMLEVEL		50
#define		DYNAMICMODE_SAVEITEMHANDLE				51

#define		SQC_ROOT			60
#define		SQC_FOLDER			61

#define		INVALID_TYPE_INFO		-5

#define		TV_CTRL_OPENPATH		200
#define		TV_CTRL_SETCURFOLDER	201
#define		TV_CTRL_CREATETOOLWND	202
#define		TV_CTRL_CONVERTTOCNC3	203
#define		TV_CTRL_IMPORTFILE		204

#define		MAX_HEADING_LEN			100
#define		MAX_FILEPATH_LEN		(32767 * sizeof( TCHAR ))

#define		ID_TVDRAGSCROLL_TIMER		100

typedef void STD_NOTIMPLMETHOD;

typedef struct _MDA{ TCHAR name[ MAX_HEADING_LEN ]; }MDA, *LPMDA;
typedef struct _HTI { HTREEITEM hPrev; }HPREVITEM, *LPHPREVITEM;

typedef struct _FOPCLIPBOARDINFO{

	int type_forCopy;
	int type_forMove;
	int type_forNew;
	DWORD lastaction;
	DWORD dwFlagsAfterOP;
	TCHAR* renamepath;
	TCHAR* movepath_source;
	TCHAR* movepath_target;
	TCHAR* copypath;
	TCHAR* postOperationBuffer;
	TCHAR* preOperationBuffer;
	BOOL preOperationBuffer_valid;
	BOOL postOperationBuffer_valid;
	BOOL copypath_valid;
	BOOL movepath_source_valid;
	BOOL movepath_target_valid;
	BOOL renamepath_valid;
	HTREEITEM labelItem;
	HTREEITEM moveItem;
	int DragCursorType;
	BOOL CursorOutOfValidArea;
	BOOL CursorOverEditBox;

}FOPCLIPBOARDINFO,*LPFOPCLIPBOARDINFO;

typedef struct{
	TCHAR Heading[ MAX_HEADING_LEN ];
	int Level;
	int type;
}HEADING, *LPHEADING;

typedef struct {
	LPHPREVITEM dynamic_array;
	int nCount;
}HDYNAMICLEVEL, *LPHDYNAMICLEVEL;

typedef struct {
	LONG_PTR toClass;
	LPCTSTR root;
	int mode;
}TVTHREADDATA,*LPTVTHREADDATA;

typedef struct {
	HCURSOR cnc3;
	HCURSOR	file;
	HCURSOR folder;
	HCURSOR forbidden;
	HCURSOR insert;
}OBJECTSTORAGE, *LPOBJECTSTORAGE;

__interface ITreeViewItemLoadingProgress
{
public:
	void dataCollectionStarted();
	void dataCollectionFinished(int ItemCount);
	void startInitializingItems(int nCount);
	void itemAdded();
	void operationComplete();
	void operationFailed();
};

__interface ITreeViewUserEvents
{
public:
	void onSelectionChanged(int type, LPCTSTR itemPath);
	void onItemMoved(int type, LPCTSTR new_path, LPCTSTR old_path);
	void onItemRenamed(int type, LPCTSTR new_path, LPCTSTR old_path);
	void onItemDeleted(int type, LPCTSTR itemPath);
};

class TreeViewCTRL
	: public IFileOperationProgressSink
{
public:					TreeViewCTRL( HWND, HINSTANCE, int, LPTSTR );
						~TreeViewCTRL( void );
						/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						///// IUNKNOWN + IFILEOPERATIONPROGRESSSINK METHODS ////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable : 4838)
						IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
						{
							static const QITAB rgqit[] = {
								QITABENT(TreeViewCTRL,IFileOperationProgressSink),
								{0}
							};
							return QISearch(this, rgqit, riid, ppv);
						}
#pragma warning(pop)
						IFACEMETHODIMP_(ULONG) AddRef()
						{
							return InterlockedIncrement(&_cRef);
						}
						IFACEMETHODIMP_(ULONG) Release()
						{
							long cRef = InterlockedDecrement(&_cRef);
							if (!cRef)
								delete this;
							return cRef;
						}
						IFACEMETHODIMP PostCopyItem(DWORD dwFlags, IShellItem*, IShellItem*, LPCWSTR pszNewName, HRESULT, IShellItem*)
						{
							return this->ProcessPostCopyInfo(dwFlags, pszNewName);
						}
						IFACEMETHODIMP PostDeleteItem(DWORD dwFlags, IShellItem*, HRESULT, IShellItem*)
						{
							return this->ProcessPostDeleteInfo(dwFlags);
						}
						IFACEMETHODIMP PostMoveItem(DWORD dwFlags, IShellItem*, IShellItem*, LPCWSTR pszNewName, HRESULT, IShellItem*)
						{
							return this->ProcessPostMoveInfo(dwFlags, pszNewName);
						}
						IFACEMETHODIMP PostNewItem(DWORD dwFlags,IShellItem*,LPCWSTR newName,LPCWSTR,DWORD fileAttributes,HRESULT,IShellItem*)
						{
							return this->ProcessPostNewFile(dwFlags, newName, fileAttributes);
						}
						IFACEMETHODIMP PostRenameItem(DWORD dwFlags, IShellItem*, LPCWSTR newName, HRESULT, IShellItem*)
						{
							return this->ProcessPostRename(dwFlags, newName);
						}
						IFACEMETHODIMP PreCopyItem(DWORD, IShellItem*, IShellItem*, LPCWSTR) { return S_OK; }
						IFACEMETHODIMP PreDeleteItem(DWORD, IShellItem*) { return S_OK; }
						IFACEMETHODIMP PreMoveItem(DWORD, IShellItem*, IShellItem*, LPCWSTR) { return S_OK; }
						IFACEMETHODIMP PreNewItem(DWORD, IShellItem*, LPCWSTR) { return S_OK; }
						IFACEMETHODIMP PreRenameItem(DWORD, IShellItem*, LPCWSTR) { return S_OK; }

						IFACEMETHODIMP FinishOperations(HRESULT) { return S_OK; }
						IFACEMETHODIMP PauseTimer() { return S_OK; }
						IFACEMETHODIMP ResetTimer() { return S_OK; }
						IFACEMETHODIMP ResumeTimer() { return S_OK; }
						IFACEMETHODIMP StartOperations() { return S_OK; }
						IFACEMETHODIMP UpdateProgress(UINT, UINT) { return S_OK; }
						/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						TCHAR* Root_Folder;

						HRESULT InitTreeView( COLORREF, HFONT, COLORREF, LPRECT, DWORD );// Create the treeview control
						BOOL InitTreeViewItems( LPCTSTR, int );// load a new root folder
						void InitTreeViewItemsAsync(LPCTSTR root);
						BOOL OnTreeViewNotify( LPARAM );// add to the parent's message handler...
						void OnTreeViewTimer(WPARAM);
						int OnMouseMove(LPARAM);// add to the parent's message handler...
						int OnLButtonUp();// add to the parent's message handler...

						void updateFont(HFONT font);
						void updateImageList();

						void PerformFileOperation(int mode);

						void InsertExistingFileToView(LPTSTR);
						BOOL ExpandPathToItem( LPCTSTR );
						void Reload(bool recoverScrollPosition);
						BOOL SaveRootFolderSelection();
						BOOL SaveExpandImage( LPCTSTR );
						BOOL LoadExpandImage( LPCTSTR );
						void clearExpandImage(LPCTSTR pth) {
							this->DeleteImageFile(pth);
						}

						BOOL GetSelectedItemPath(TCHAR ** path_out);
						HWND GetTreeViewHandle() { return this->hwndTV; }
						BOOL canPasteObject() { return this->FOPInfo.copypath_valid; }
						BOOL IsDragging_CursorCTRL(LPPOINT);// Add it to MouseProc(hook)
						BOOL Recolor(COLORREF textcolor, COLORREF background);

						void SaveScroll(); // saves the scrollinfo to an internal structure
						void SetScrollPosition(LPSCROLLINFO sInfo); // sets the scroll to sInfo; if (sInfo == nullptr) the function resets the scrollinfo from an internal structure (SaveScroll() must be called before)
						BOOL GetTVScrollInfo(LPSCROLLINFO sInfo); // fills the SCROLLINFO-Struct (cbSize and fMask will also be set by the method)

						// if this was an internal operation and it was a move operation, the method returns false!
						bool FileSystem_RemoveItemFromTree(LPCTSTR path);// the path is relative to the rootfolderpath!
						void FileSystem_AddItemToTree(LPCTSTR path);// the path is relative to the rootfolderpath!
						void FileSystem_RenameItemInTree(LPCTSTR old_path, LPCTSTR new_path);// the path's are relative to the rootfolderpath!
						void FileSystem_RefreshAllItems(); // this method calls 'ReloadAsync()'

						void ReloadAsync();

						void Clear(){
							TreeView_DeleteAllItems(this->hwndTV);
						}

						void infoTipEnable(BOOL enable);
						void openNewFileEnable(BOOL enable)
						{
							this->openNewFile = enable;
						}
						void setEventListener(ITreeViewItemLoadingProgress* eventListener)
						{
							this->iTVProgress = eventListener;
						}
						ITreeViewItemLoadingProgress* getEventListener()
						{
							return this->iTVProgress;
						}
						void setInfoSink(ITreeViewUserEvents* iSink)
						{
							this->iTVUserEvents = iSink;
						}
private:
	long _cRef;// Instance counter -> set to 1 by creation
	HINSTANCE hInstance;
	HWND hwndTV;
	HWND TVFrame;
	int language_;// remove// OBSOLETE !!! 
	int ico_index[ 5 ];
	TCHAR* Root_Name;
	TCHAR* Working_Dir;
	HFONT tvFont;
	FOPCLIPBOARDINFO FOPInfo;
	HDYNAMICLEVEL DynamicLevel;
	SCROLLINFO internalScrollInfo;
	DWORD threadID;
	BOOL cSuccess;
	BOOL disableFOPConfirmation;
	BOOL DragUnderway;
	BOOL useInfoTips;
	BOOL openNewFile;
	BOOL folderCopyInProgress;
	BOOL internalFileOperationInProgress;
	BOOL moveOperationInProgress;
	BOOL CursorScrollTimerActivated;
	int ScrollDirection;				// -1 == down; 0 == no scroll; 1 == up

	OBJECTSTORAGE Obj;
	ITreeViewItemLoadingProgress* iTVProgress;
	ITreeViewUserEvents* iTVUserEvents;

	ITreeViewUserEvents* temporaryPtr;

	BOOL InitTreeViewImageLists( HWND );
	HTREEITEM AddItemToTree( HWND, LPTSTR, int, int );

	HTREEITEM _dynamic_level_provider_(DWORD, int, HTREEITEM);
	int CountItems( LPCTSTR );
	HRESULT CountItemsNextLevel(HANDLE, LPWIN32_FIND_DATA, TCHAR*, int&);

	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static DWORD __stdcall FOP_Proc(LPVOID);
	static DWORD __stdcall initAsycProc(LPVOID);
	static DWORD __stdcall reloadProc(LPVOID lParam);

	BOOL SaveRoot();
	BOOL LoadRoot();
	HRESULT GenerateRoot();
	HRESULT InitNextLevelItems(HANDLE, LPWIN32_FIND_DATA, LPHEADING, TCHAR*,int, int, int&);

	void DisplayErrorBox( LPTSTR );
	void LeachRootName( LPCTSTR, LPTSTR );

	BOOL CompareHeading( LPTSTR, LPTSTR );
	BOOL IsRootinPath(LPTSTR);
	int CheckItemType( HTREEITEM );
	int _getSelectedItemPath( TCHAR** );
	int GetItemPath(HTREEITEM, TCHAR**);
	int GetItemLevel(HTREEITEM);
	int GetUserPermission(int);

	// this method returns the handle to the item, which is representing the path in the treeView
	// When (returnLastValidHandle == true)-> if the path is pointing to a location which is not yet represented in the treeView,
	// the method returns the handle to the last parent item which was found in the path queue and type_out is -1 !
	// -> if 'returnLastValidHandle == true' and the return value is nullptr, the last valid item must be the root item (use TreeView_GetRoot() to get it)
	// -> if 'returnLastValidHandle == false' and the return value is nullptr, the item does not exist
	HTREEITEM getItemHandleFromRelativePath(LPCTSTR path, int* type_out, bool returnLastValidHandle);

    BOOL Sequencing( int, LPHEADING, LPHEADING, int );
    inline int SequenceNextLevel( int, int, LPHEADING, LPHEADING, int, int );
	int CheckFileType( LPTSTR );
	BOOL CheckForNumber( TCHAR );
	void DisplayItemstruct(LPHEADING, int);
	inline BOOL ExpandNextLevel( HTREEITEM, int, LPMDA, int );

	int GetExpandImage( LPHEADING, DWORD ); // modes are: READ / COUNT ->in count-mode LPHEADING parameter can be nullptr
	BOOL SetExpandImage( LPHEADING, DWORD ); // what is max_index????
	int LoadExpandImageFromFile( LPCTSTR, LPHEADING, DWORD );
	BOOL SaveExpandImageToFile( LPCTSTR, LPHEADING, int );

	inline BOOL GetNextLevel_Image( LPHEADING, HTREEITEM, DWORD, int& );
	inline BOOL SetNextLevel_Image( LPHEADING, HTREEITEM, DWORD&, DWORD );
	BOOL DeleteImageFile( LPCTSTR );
	BOOL Read_Image_From_Buffer( LPHEADING, LPTSTR, int&, DWORD, int );

	BOOL RemoveWildcard(TCHAR*);
	BOOL RemoveFilename(TCHAR*);
	BOOL RemoveFileExt(int, WCHAR*);	

	HTREEITEM FindInsertAfter(HTREEITEM, int, LPCTSTR);
	BOOL InsertFolder(HTREEITEM);
	BOOL InsertEmptyItem(HTREEITEM);
	HTREEITEM SortItem(HTREEITEM);

	int OnItemExpanding(LPNMTREEVIEW);
	int OnItemExpanded(LPNMTREEVIEW);
	int OnSelchanged(LPNMTREEVIEW);
	int OnDblClick();
	int OnRgtClick();
	int OnBeginDrag(LPNMTREEVIEW);
	int OnEndLabelEdit(LPNMTVDISPINFO);
	int OnBeginLabelEdit(LPNMTVDISPINFO);
	int OnGetInfoTip(LPNMTVGETINFOTIP);

	BOOL PrepareForCopyOperation();
	BOOL StartLabelEdit();
	
	HRESULT CreateFOP_Thread(int);
	HRESULT	_executeCopyOP();
	HRESULT _executeDeleteOP();
	HRESULT _executeMoveOP();
	HRESULT _executeRenameOP(LPCWSTR);
	HRESULT _executeNewOP(int);

	BOOL SendConvertInstruction();
	BOOL SendImportInstruction();

	HRESULT ProcessPostCopyInfo(DWORD,LPCWSTR);
	HRESULT ProcessPostDeleteInfo(DWORD);
	HRESULT ProcessPostMoveInfo(DWORD, LPCWSTR);
	HRESULT ProcessPostNewFile(DWORD, LPCWSTR, DWORD);
	HRESULT ProcessPostRename(DWORD, LPCWSTR);

	void stepProgressIfApplicable();
	void enableSelchangeNotification(bool enable);
};