#pragma once
#include"external.h"
#include<CustomButton.h>
#include<CustomCheckbox.h>
#include<SingleLineEdit.h>
#include"ApplicationData.h"
#include"EditControl.h"

class autosyntaxManager
	: public customButtonEventSink,
	public customCheckboxEventSink,
	public singleLineEditEventSink
{
public:
	autosyntaxManager(HINSTANCE hInst);
	~autosyntaxManager();

	static const TCHAR* AutoSyntaxManagerWindowCLASS;

	static const CTRLID ASWindow_ID = 1126;
	static const int ctrlID_exitButton = 10;
	static const int ctrlID_isOnCheckbox = 11;
	static const int ctrlID_autonumStartline = 30;
	static const int ctrlID_autoNumStepMain = 31;
	static const int ctrlID_numLenghtMain = 32;
	static const int ctrlID_minLinenumber = 33;
	static const int ctrlID_maxLinenumber = 34;
	static const int ctrlID_eraseLinenumber = 35;
	static const int ctrlID_noSpaceInLinenumber = 36;
	static const int ctrlID_numStepSub = 37;
	static const int ctrlID_numLengthSub = 38;
	static const int ctrlID_useDifferentLinenumberInSub = 39;
	static const int ctrlID_subProgTriggerEdit = 40;
	static const int ctrlID_newEvenLinenumberOnTrigger = 41;
	static const int ctrlID_evenLinenumberTriggerEdit = 42;
	static const int ctrlID_useEndprogDetection = 43;
	static const int ctrlID_endProgTriggerEdit = 44;
	static const int ctrlID_autoCompleteBrackets = 45;
	static const int ctrlID_insertSpacesInBrackets = 46;
	static const int ctrlID_useMultilineAnnotations = 47;

	HRESULT Init(LPCTRLCREATIONSTRUCT pcr);

	void Release() {
		delete this;
	}
	// when the window is closed, the object will be automatically released
	void Close();

	static bool GetAutosyntaxSettings(LPAUTOSYNTAXPROPERTY asp);
	static bool GetTriggerStrings(iString& subProg, iString& endProg, iString& newLine);

	// button event handler ////////////////////////////////////////////////////
	void autosyntaxManager::onCustomButtonClick(cObject sender, CTRLID ctrlID) {
		this->onButtonClick(
			reinterpret_cast<CustomButton*>(sender),
			ctrlID
		);
	}
	////////////////////////////////////////////////////////////////////////////
	// checkbox event handler //////////////////////////////////////////////////
	void autosyntaxManager::onCustomCheckboxChecked(cObject sender, bool newState)
	{
		this->onCheckboxChecked(reinterpret_cast<CustomCheckbox*>(sender), newState);
	}
	////////////////////////////////////////////////////////////////////////////
	// singleline edit event handler ///////////////////////////////////////////
	void autosyntaxManager::onEditboxContentChanged(cObject sender, CTRLID ctrlID)
	{
		this->onEditContentChanged(
			reinterpret_cast<singleLineEdit*>(sender),
			ctrlID
		);
	}
	void autosyntaxManager::onTabKeyWasPressed(cObject sender, CTRLID ctrlID)
	{
		UNREFERENCED_PARAMETER(sender);
		UNREFERENCED_PARAMETER(ctrlID);
	}
	////////////////////////////////////////////////////////////////////////////
private:
	HINSTANCE hInstance;
	HWND ausynWnd;
	HWND Parent;
	HFONT ctrlFont;

	bool syntaxHasChanged;
	bool msgBlocker;

	int currentScrollPosition;

	APPSTYLEINFO styleInfo;

	HRESULT _registerASClass();
	HRESULT _createControls();
	HRESULT _createSinglelineEditWithIntegerDataBinding(ApplicationData* aData, DATAKEY key, int descriptionResourceID, int ctrlID, int x, int y, int defaultValue);
	HRESULT _createSinglelineEditWithStringDataBinding(ApplicationData* aData, DATAKEY key, int ctrlID, int x, int y, LPCTSTR defaultValue);
	HRESULT _createCheckboxWithBooleanDataBinding(ApplicationData* aData, DATAKEY key, int UItextResourceID, int ctrlID, int x, int y, bool defaultvalue);

	static LRESULT CALLBACK autosynProc(HWND, UINT, WPARAM, LPARAM);

	LRESULT onPaint(HWND hWnd);
	LRESULT onVScroll(WPARAM wParam);
	LRESULT onMouseWheel(WPARAM wParam);

	void onButtonClick(CustomButton* cButton, CTRLID ctrlID);
	void onCheckboxChecked(CustomCheckbox* cCheckbox, bool _newState);
	void onEditContentChanged(singleLineEdit* edit, CTRLID ctrlID);

	void redrawVolatileWindowPortion(HDC hdcExtern, int scrollPosition);

	void setValidRangeFromControlID(singleLineEdit* Sle, int ctrlID);
};
