#pragma once
#include"..//external.h"
#include"..//CommonControls/CustomButton.h"
#include"..//XML Parsing/XML_Parsing.h"
#include"..//EditControl.h"
#include"..//HelperF.h"
#include"..//Graphics/workingIndicator.h"
#include"..//CommonControls/CustomPopUpMenu.h"

#define			TV_IMG_INDEX_FILTERCLOSED		0
#define			TV_IMG_INDEX_FILTEROPEN			1
#define			TV_IMG_INDEX_SAMPLEFILE			2
#define			TV_IMG_INDEX_FILTERCLOSED_SEL	3
#define			TV_IMG_INDEX_FILTEROPEN_SEL		4
#define			TV_IMG_INDEX_SAMPLEFILE_SEL		5

#define			ITEMTYPE_FILTER					20
#define			ITEMTYPE_SAMPLE					21

#define			NO_EXIST_LOOKUP				0x01
#define			DO_NOT_INSERT_IN_VIEW		0x02
#define			ELEMENT_WAS_CHANGED			0x04

#define			SMPMNGR_MAX_HEADING_LEN			96

#define			IDMM_SAMPLETREE			99

#define			IDMM_ADDSAMPLE			100
#define			IDMM_ADDFILTER			101
#define			IDMM_CCOPYSAMPLE		102
#define			IDMM_CHANGEELEMENT		103
#define			IDMM_REMOVESAMPLE		104
#define			IDMM_INSERTSAMPLE		105
#define			IDMM_SAVESAMPLE			106
#define			IDMM_CHANGESAMPLE		107
//#define			IDMM_CCOPYSAMPLE		108
// reserved for toolbar button ids
#define			IDMM_CLOSE				109
#define			IDMM_CLOSESMPEDITOR		110

#define			IDM_NAMEEDIT			111
#define			IDM_DESCEDIT			112
#define			IDM_CONTENTEDIT			113

#define		MAX_FILEPATH_LEN		(32767 * sizeof( TCHAR ))

#define		SAMPLEMANAGER_WINDOWCLASS	L"SMPMNGR_CLASS\0"

typedef struct _SMNGROBJ {
	HFONT font;
	HFONT tvFont;
	HBRUSH background;
	HPEN outlinePen;

	HWND saveButton;
	HWND rejectButton;

	HWND nameEdit;
	HWND descEdit;
	HWND content;

	HCURSOR samplefile;

}SMNGROBJ, *LPSMNGROBJ;

typedef struct _SAMPLEINSERTARRAY {
	TCHAR heading[SMPMNGR_MAX_HEADING_LEN];
	int level;
	int type;
	int state;
} SAMPLEINSERTARRAY, *LPSAMPLEINSERTARRAY;

typedef struct _CURRENTSELECTEDELEMENT {
	iString path;
	iString name;
	int type;
	HTREEITEM item;
}CURRENTSELECTEDELEMENT, *LPCURRENTSELECTEDELEMENT;

typedef struct _SMPDRAGINFORMATION {
	bool isValid;
	TVITEM source;
	TCHAR sourceName[SMPMNGR_MAX_HEADING_LEN];
}SMPDRAGINFORMATION, *LPSMPDRAGINFORMATION;

typedef struct _SAMPLEPROPERTY {
	iString path;
	iString name;
	iString description;
	iString content;

}SAMPLEPROPERTY, *LPSAMPLEPROPERTY;

typedef struct _ELEMENTCHANGEINFO {
	bool isValid = false;
	iString path;
	iString name;
	HTREEITEM item;
}ELEMENTCHANGEINFO, *LPELEMENTCHANGEINFO;

class SampleManager
	: public customButtonEventSink,
	  public XMLParsingEventSink,
	  public customPopUpMenuEventSink
{
public:
	SampleManager(HWND host, HWND Main, const TCHAR* pathToSamples, HINSTANCE hInst);
	~SampleManager();

	HRESULT Init(LPRECT rc, int nCmdShow);
	HRESULT InitSampleEditor(int Mode, LPSAMPLEPROPERTY psp);

	void Release() { 
		DestroyWindow(this->smpEditWnd);
		DestroyWindow(this->sampleWnd); 
		delete this; }

	void onQuit();
	void onDpiChanged();

	// toolbar button event handler
	void SampleManager::onCustomButtonClick(cObject sender, CTRLID ID_button) {
		UNREFERENCED_PARAMETER(sender);
		this->onTBButtonClick(ID_button);
	}
	// xml-parsing result handler
	void SampleManager::ParsingCompleted(cObject sender, itemCollection<iXML_Tag>* xmlTag) {
		this->onParsingCompleted(sender, xmlTag);
	}
	void SampleManager::ParsingFailed(cObject sender, LPPARSINGERROR ppErr) {
		UNREFERENCED_PARAMETER(sender);
		this->translateParsingError(ppErr);
	}
	// menu-event handler
	void SampleManager::onPopUpMenuButtonClicked(CTRLID ID_menubutton) {
		this->onTBButtonClick(ID_menubutton);
	}

private:
	HWND sampleWnd;
	HWND smpEditWnd;
	HWND mainWnd;
	HWND hostWnd;
	HWND treeView;
	HINSTANCE hInstance;

	iString workingDir;
	SMNGROBJ obj;
	CURRENTSELECTEDELEMENT curSelElement;
	ELEMENTCHANGEINFO elementChangeInfo;
	SMPDRAGINFORMATION dragInfo;
	EditControl* edc;
	workingIndicator* pWI;

	APPSTYLEINFO StyleInfo;

	BOOL startupSucceeded;
	BOOL draggingUnderway;
	BOOL toolbarMultiline;

	static LRESULT CALLBACK sampleProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK sampleEditProc(HWND, UINT, WPARAM, LPARAM);
	static DWORD WINAPI filterImageProc(LPVOID);

	HRESULT createContent();
	HRESULT CreateToolbar();
	HRESULT CreateTreeview();
	HRESULT CreateSampleEditor(int);
	HRESULT CreateSampleEditorChilds(int, LPSAMPLEPROPERTY);

	LRESULT OnSize();
	LRESULT OnSizeInAddWnd();
	LRESULT OnNotify(LPARAM);
	LRESULT OnSetAppStyle(LPARAM);
	LRESULT OnPaint(HWND);
	LRESULT OnPaintInAddWnd(HWND);
	LRESULT OnClose(HWND);
	LRESULT OnDblClick();
	LRESULT OnRClick();
	LRESULT OnMousemove(WPARAM, LPARAM);
	LRESULT OnLButtonUp();

	void onTBButtonClick(CTRLID ID);

	BOOL initTreeViewImageList();
	BOOL initTreeViewItems(LPSAMPLEINSERTARRAY, int);
	HTREEITEM addItemToTree(LPTSTR, int, int, int);

	BOOL loadFilterImage();
	BOOL resolveFilterImage(itemCollection<iXML_Tag>*, int);
	int resolveFilterImageNextLevel(itemCollection<iXML_Tag>*, LPSAMPLEINSERTARRAY, int, int, int);
	BOOL saveFilterImage();
	BOOL saveFilterImageNextLevel(itemCollection<iXML_Tag>*, HTREEITEM);
	void createFilterImageSaverThread();

	void onItemExpandStatusChanged(LPARAM);
	void onGetInfoTip(LPARAM);
	void onSelchange(LPARAM);
	BOOL onEndLabelEdit(LPARAM);
	BOOL onBeginLabelEdit(LPARAM);

	BOOL GetInfoTip(LPTSTR,TCHAR**);
	BOOL ResolveFilter();
	BOOL ResolveFilter_NextLevel(HTREEITEM,HTREEITEM);
	BOOL GetSamplePropertyFromSelectedItem(LPSAMPLEPROPERTY);

	void AddNewFilter();
	void ChangeElement();
	void RemoveElement();
	void CloseSMPEditor(int);
	void SetSMPEditorContent(LPSAMPLEPROPERTY);

	int getItemType(HTREEITEM);
	int getItemInfo(HTREEITEM, LPCURRENTSELECTEDELEMENT);
	int getItemPathEx(HTREEITEM, iString*);

	BOOL saveSample(DWORD);
	iString loadSample();

	BOOL verifySampleName(LPTSTR);
	BOOL insertSampleInView(LPTSTR name);
	BOOL sendSampleInsertRequest();
	bool copySampleToClipboard();
	void insertRootFilter();
	int getIconIDFromMenuId(int menu_id, bool disabled);
	void setTBButtonEnabledState();

	void translateParsingError(LPPARSINGERROR);

	void onParsingCompleted(cObject, itemCollection<iXML_Tag>*);
	void onBeginDrag(LPARAM);

	HTREEITEM FindInsertAfter(HTREEITEM, int, iString);
	void moveSubItems(HTREEITEM, HTREEITEM);
	void updateSelectedItem();
	void setTVFont();
	BOOL isSelectionValid();

	void _createDpiDependendResources();
	DWORD getDWButtonEnableInfo(int type);
};
