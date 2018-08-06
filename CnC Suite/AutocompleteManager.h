#pragma once
#include"external.h"
#include"CommonControls\CustomButton.h"
#include"CommonControls\SingleLineEdit.h"
#include<cListView.h>
#include"autocompleteStrings.h"

#define			AUTOCOMPLETEMANAGERCLASS		L"AUTOCMPLMNGRCLASS"

#define			CTRLID_EXITBUTTON			20
#define			CTRLID_ADDBUTTON			21
#define			CTRLID_DELETEBUTTON			22
#define			CTRLID_TRIGGEREDIT			23
#define			CTRLID_APPENDIXEDIT			24
#define			CTRLID_PREVIEWEDIT			25
#define			CTRLID_ACLISTVIEW			26

#define			TYPE_TRIGGERINPUT			1
#define			TYPE_APPENDIXINPUT			2

class autocompleteManager
	: public ObjectRelease<autocompleteManager>,
	public customButtonEventSink,
	public singleLineEditEventSink,
	public listViewEventSink
{
public:
	//autocompleteManager();
	autocompleteManager(HINSTANCE hInst, HWND mainWnd);
	~autocompleteManager();

	HRESULT Init(LPCTRLCREATIONSTRUCT pcs, autocompleteStrings* pac);

	void autocompleteManager::onCustomButtonClick(cObject sender, CTRLID ctrlID){
		this->onButtonClick(
			reinterpret_cast<CustomButton*>(sender),
			ctrlID
		);
	}

	void autocompleteManager::onEditboxContentChanged(cObject sender, CTRLID ctrlID){
		this->onEditContentChanged(reinterpret_cast<singleLineEdit*>(sender), ctrlID);
	}
	void autocompleteManager::onTabKeyWasPressed(cObject sender, CTRLID ctrlID) {
		
		UNREFERENCED_PARAMETER(sender);

		switch (ctrlID)
		{
		case CTRLID_TRIGGEREDIT:
			this->appendixEdit->setFocus();
			break;
		case CTRLID_APPENDIXEDIT:
			this->triggerEdit->setFocus();
			break;
		default:
			break;
		}
	}
	void autocompleteManager::onListviewItemSelected(cObject sender, int selectedRowIndex){
		this->onListviewSelectionChanged(reinterpret_cast<cListView*>(sender), selectedRowIndex);
	}

	static const CTRLID AutoCompWindowID = 1086;

private:
	HWND acmWnd;
	HWND MainWnd;
	HWND parent;
	HINSTANCE hInstance;
	HFONT ctrlFont;
	DWORD opMode;

	//int selectedRow;

	singleLineEdit* triggerEdit;
	singleLineEdit* appendixEdit;
	singleLineEdit* previewEdit;

	autocompleteStrings* pAcStrings;

	APPSTYLEINFO sInfo;
	cListView* acListView;

	HRESULT createControls();
	HRESULT registerClass();

	static LRESULT CALLBACK acmProc(HWND, UINT, WPARAM, LPARAM);

	LRESULT onPaint(HWND);

	void beginNewAddSession();
	void beginNewAddSessionWithContent(int type, const TCHAR* content);

	void onButtonClick(CustomButton* sender, CTRLID ctrlID);
	void onEditContentChanged(singleLineEdit* edit, CTRLID ctrlID);
	void onListviewSelectionChanged(cListView* view, int selectedIndex);

	void updatePreviewEdit();
};