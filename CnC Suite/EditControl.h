#pragma once
#include"external.h"
#include"autocompleteStrings.h"

//#define		CONTENTHASCHANGED		1000

#define		GETCURRENTCHARRANGE(hWnd, cr)		SNDMSG((HWND)(hWnd), EM_EXGETSEL, (WPARAM)0, (LPARAM)(cr))

#define		CONVERT_LINEFEED			0x0001
#define		CONVERT_TOMULTICOLOR		0x0002
#define		CONVERT_TOSINGLECOLOR		0x0004
#define		CONVERT_SYNTAX				0x0008
#define		CONVERT_CHARHEIGHT			0x0010
#define		CONVERT_MARKCOLOR			0x0020
#define		CONVERT_CUR_COLORING		0x0040
#define		CONVERT_ADDTO_UNDOSTACK		0x0080

// ...

#define		HIDE_WHILE_CONVERTING		0x800

#define		ENABLE_ALL_CONVERSION_FLAGS			CONVERT_TOMULTICOLOR | CONVERT_SYNTAX | CONVERT_CHARHEIGHT | CONVERT_MARKCOLOR | HIDE_WHILE_CONVERTING

#define		MAX_UNDO_STACK				30

#define		WM_CHECKENVIRONMENT		WM_USER + 1
#define		WM_DETECTINSIDE_N		WM_USER + 2
#define		WM_UPDATEFOCUSRECT		WM_USER + 3
#define		WM_UPDATELINE			WM_USER + 4		// WPARAM == POS FROM CHAR

#define		FROM_CARET_POSITION				-1

#define		UNDOACTION_CONTENT_DELETED		1
#define		UNDOACTION_BACKSPACE			2
#define		UNDOACTION_NEWCHAR_ADDED		3
#define		UNDOACTION_TEXT_INSERTED		4
#define		UNDOACTION_SELECTION_REPLACED	5
#define		UNDOACTION_CONTENT_REPLACED		6

#define		LN_MODE_TRIGGER					10
#define		LN_MODE_SUB						11
#define		LN_MODE_MAIN					12

#define		ANNOTATION_AUTOINSERT_EXECUTED	15
#define		ANNOTATION_VARIABLE_STATEMENT	16
#define		ANNOTATION_CLOSEMARK_DETECTED	17

#define		PERFORM_RELEASE_ASYNC			20
#define		EXECUTE_WITH_RE_INITALIZATION	21

#define		SUBPROG_TRIGGER_DEFAULTSTRING	L"G1 ;G2 ;G3 ;G40;G41;G42;G4 ;"
#define		ENDPROG_TRIGGER_DEFAULTSTRING	L"M30;M91;M99;"
#define		NEWLINE_TRIGGER_DEFAULTSTRING	L"T;"

#define		STARTUP_TEXT_PLACEHOLDER		L"  \n\nN   1 "

#define		CARET_TO_END_OF_TEXT			-50
#define		CARET_TO_FIRST_POSITION			-51

#define		CORR_TYPE_TOP		1
#define		CORR_TYPE_BOTTOM	0

typedef int CaretIndex;

class ColorConversion
	: public iCollectable<ColorConversion>
{
public:
	ColorConversion()
	: start(0), end(0), color(0)
	{}

	ColorConversion(const ColorConversion& ccv)
	: start(0), end(0), color(0)
	{
		this->start = ccv.getStart();
		this->end = ccv.getEnd();
		this->color = ccv.getColor();
	}

	// getter
	int getStart() const {
		return this->start;
	}
	int getEnd() const {
		return this->end;
	}
	COLORREF getColor() const {
		return this->color;
	}

	// setter
	void setStart(int _start) {
		if (_start < 0)
			this->start = 0;
		else
			this->start = _start;
	}
	void setEnd(int _end) {
		if (_end < 0)
			this->end = 0;
		else
			this->end = _end;
	
	}
	void setColor(COLORREF _color) {
		this->color = _color;
	}

	// other
	void clear()
	{
		this->start = 0;
		this->end = 0;
		this->color = 0;
	}
	ColorConversion& operator= (const ColorConversion& ccv)
	{
		this->start = ccv.getStart();
		this->end = ccv.getEnd();
		this->color = ccv.getColor();
		return *this;
	}
	bool operator== (const ColorConversion& ccv) {
		return
			((this->color == ccv.color) && (this->end == ccv.end) && (this->start == ccv.start))
			? true : false;
	}

	ColorConversion* getInstance() {
		return this;
	}

private:
	int start;
	int end;
	COLORREF color;
};

//typedef struct _MARKCOLORSTRUCT {
//
//	TCHAR start[56];
//	TCHAR end[56];
//	COLORREF refColor;
//
//}MARKCOLORSTRUCT, *LPMARKCOLORSTRUCT;			// for later :)

typedef struct _EDITSTYLECOLORS {

	COLORREF background;
	COLORREF defaultTextcolor;
	COLORREF A;
	COLORREF B;
	COLORREF C;
	COLORREF D;
	COLORREF E;
	COLORREF F;
	COLORREF G;
	COLORREF H;
	COLORREF I;
	COLORREF J;
	COLORREF K;
	COLORREF L;
	COLORREF M;
	COLORREF N;
	COLORREF O;
	COLORREF P;
	COLORREF Q;
	COLORREF R;
	COLORREF S;
	COLORREF T;
	COLORREF U;
	COLORREF V;
	COLORREF W;
	COLORREF X;
	COLORREF Y;
	COLORREF Z;
	COLORREF Annotation;
	COLORREF LineNumber;
}EDITSTYLECOLORS, *LPEDITSTYLECOLORS;

typedef struct _AUTOSYNTAXPROPERTY {

	BOOL IsOn;// when this is set to TRUE, the text will initally converted(by opening)
	BOOL UseDifferentNumLengthInSubProg;
	int MainLevelNumLength;
	int SubLevelNumLength;// limited to 3! (3 figures)
	BOOL noSpaceInLineNumber;
	BOOL eraseLinenumberOnBackspace;
	BOOL autoinsertBrackets;
	BOOL noSpaceBetweenBraces;
	int autonumStartLine;
	int maximumLinenumber;
	int numStepInSubProgramm;
	int numStepInMainProgramm;

	BOOL newEvenLineNumberOnTrigger;
	int lineNumberStartValue;
	BOOL useNoDifferentLinenumbersInSubprogram;	//this is confusing, but it is responsible for the conversion, not the insertion
	BOOL useMultilineAnnotations;
	BOOL useEndProgDetection;

	BOOL AutoLevitation;// not used, yet

	_AUTOSYNTAXPROPERTY& _AUTOSYNTAXPROPERTY::operator= (const _AUTOSYNTAXPROPERTY& as)
	{
		this->IsOn = as.IsOn;
		this->UseDifferentNumLengthInSubProg = as.UseDifferentNumLengthInSubProg;
		this->MainLevelNumLength = as.MainLevelNumLength;
		this->SubLevelNumLength = as.SubLevelNumLength;
		this->noSpaceInLineNumber = as.noSpaceInLineNumber;
		this->eraseLinenumberOnBackspace = as.eraseLinenumberOnBackspace;
		this->autoinsertBrackets = as.autoinsertBrackets;
		this->noSpaceBetweenBraces = as.noSpaceBetweenBraces;
		this->autonumStartLine = as.autonumStartLine;
		this->maximumLinenumber = as.maximumLinenumber;
		this->AutoLevitation = as.AutoLevitation;
		this->newEvenLineNumberOnTrigger = as.newEvenLineNumberOnTrigger;
		this->lineNumberStartValue = as.lineNumberStartValue;
		this->numStepInSubProgramm = as.numStepInSubProgramm;
		this->useNoDifferentLinenumbersInSubprogram = as.useNoDifferentLinenumbersInSubprogram;
		this->useMultilineAnnotations = as.useMultilineAnnotations;
		this->useEndProgDetection = as.useEndProgDetection;
		this->numStepInMainProgramm = as.numStepInMainProgramm;

		return *this;
	}

}AUTOSYNTAXPROPERTY, *LPAUTOSYNTAXPROPERTY;

typedef struct _EDITCONTROLPROPERTIES {
	BOOL uppercase;
	BOOL autocomplete;
	BOOL autonum;
	BOOL autocolor;
	BOOL focusmark;
	// BOOL markcolor;

	BOOL isBold;
	LONG lineOffset;
	
	int charHeight;
	TCHAR charSet[128];
	AUTOSYNTAXPROPERTY autosyntax;

	int focusmarkCor_top;
	int focusmarkCor_bottom;

	_EDITCONTROLPROPERTIES& _EDITCONTROLPROPERTIES::operator= (const _EDITCONTROLPROPERTIES& ecp)
	{
		this->uppercase = ecp.uppercase;
		this->autocomplete = ecp.autocomplete;
		this->autonum = ecp.autonum;
		this->autocolor = ecp.autocolor;
		this->focusmark = ecp.focusmark;
		this->isBold = ecp.isBold;
		this->lineOffset = ecp.lineOffset;
		this->charHeight = ecp.charHeight;
		this->autosyntax = ecp.autosyntax;
		this->focusmarkCor_top = ecp.focusmarkCor_top;
		this->focusmarkCor_bottom = ecp.focusmarkCor_bottom;
		StringCbCopy(this->charSet, sizeof(this->charSet), ecp.charSet);

		return *this;
	}

}EDITCONTROLPROPERTIES, *LPEDITCONTROLPROPERTIES;

typedef struct _DELAYTHREADDATA {
	int ctrlID;
	DWORD milliSeconds;
	UINT message;
	WPARAM wParam;
}DELAYTHREADDATA, *LPDELAYTHREADDATA;

typedef struct _LNFMTDATA {
	// linenumber format data
	bool subLevelActive;
	int lastNumber;
}LNFMTDATA, *PLNFMTDATA;

typedef struct _UNDOREDOACTIONS {

	CHARRANGE range;
	TCHAR* content;
	int action;

	TCHAR* replacedContent;
	CHARRANGE replacedRange;

}UNDOREDOACTIONS, *LPUNDOREDOACTIONS;

class TriggerWordContainer
	: public iCollectable<TriggerWordContainer>
{
public:
	TriggerWordContainer(){}
	TriggerWordContainer(const TriggerWordContainer& twc)
	{
		this->words = twc.words;
	}
	~TriggerWordContainer(){}

	void Release() { delete this; }

	bool isWordInside(LPCTSTR word);

	// note: the call of this method clears the content in any case, even though tStr is nullptr
	bool setContentFromTriggerString(LPCTSTR tStr);

	bool checkBufferForWordMatches(LPCTSTR buffer);

	int getWordCount() const { return this->words.GetCount(); }
	itemCollection<iString> getWords() const { return this->words; }

	void clear() {
		this->words.Clear();
	}

	TriggerWordContainer& operator= (const TriggerWordContainer& twc)
	{
		this->words = twc.words;
		return *this;
	}
	bool operator== (const TriggerWordContainer& twc) {
		auto nWords = twc.getWordCount();
		for (int i = 0; i < nWords; i++)
		{
			if (this->words.GetAt(i) != twc.words.GetAt(i))
			{
				return false;
			}
		}
		return true;
	}
	TriggerWordContainer* getInstance() { return this; }

private:
	itemCollection<iString> words;
};

class UndoRedoChain
{
public:
	UndoRedoChain()
		: currentActionIndex(0),
		availableActions(0)
	{
		this->Initalize();
	}
	~UndoRedoChain(){
		this->Clear();
	}

	void Clear() {

		for (int i = 0; i < MAX_UNDO_STACK; i++)
		{
			if (this->actions[i].content != nullptr)
			{
				delete[] this->actions[i].content;
				this->actions[i].content = nullptr;
			}
		}
		this->availableActions = 0;
		this->currentActionIndex = 0;
	}
	// add a new UNDO action to the stack
	void addNewUndoAction(LPUNDOREDOACTIONS action_in);
	// if this function returns true UNDO could be executed otherwise the stack is empty
	bool UndoRequest(LPUNDOREDOACTIONS action_out);
	// if this function returns true REDO could be executed otherwise the stack is empty
	bool RedoRequest(LPUNDOREDOACTIONS action_out);

	// special functions - use with care!
	void getActionAt(int index, LPUNDOREDOACTIONS action_out);
	void replaceActionAt(int index, LPUNDOREDOACTIONS action_in);

private:
	int currentActionIndex;
	int availableActions;

	UNDOREDOACTIONS actions[MAX_UNDO_STACK];

	void Initalize() {
		
		for (int i = 0; i < MAX_UNDO_STACK; i++)
		{
			this->actions[i].action = -1;
			this->actions[i].content = nullptr;
			this->actions[i].range.cpMax = 0;
			this->actions[i].range.cpMin = 0;

			this->actions[i].replacedContent = nullptr;
			this->actions[i].replacedRange.cpMax = 0;
			this->actions[i].replacedRange.cpMin = 0;
		}
	}
	void clearAtIndex(int index) {

		if (index < MAX_UNDO_STACK)
		{
			this->actions[index].action = -1;
			this->actions[index].range.cpMax = 0;
			this->actions[index].range.cpMin = 0;
			SafeDeleteArray(&this->actions[index].content);

			this->actions[index].replacedRange.cpMax = 0;
			this->actions[index].replacedRange.cpMin = 0;
			SafeDeleteArray(&this->actions[index].replacedContent);
		}
	}
	void removeRedoAndReformat();
};

class EditControl {
public:
	EditControl(HWND edit,HWND connector);
	~EditControl();

	void Release() {
		
		BOOL isThreadActive =
			(BOOL)InterlockedCompareExchange(
				(LONG*)&this->focusMarkVisibilityCheckupThreadActive,
				(LONG)PERFORM_RELEASE_ASYNC,
				TRUE
			);

		if (!isThreadActive)
			delete this;
		else if (isThreadActive == 2)
			InterlockedExchange((LONG*)&this->focusMarkVisibilityCheckupThreadActive, (LONG)PERFORM_RELEASE_ASYNC);
	}
	BOOL ConfigureComponent(LPEDITSTYLECOLORS styleColors, LPEDITCONTROLPROPERTIES properties, BOOL redraw, BOOL convertText);
	void UpdateProperties(LPEDITCONTROLPROPERTIES properties, BOOL forceConversion);

	void ChangeVAR_Reset();
	void GetColorInfo(LPEDITSTYLECOLORS);
	void ConvertContent(DWORD conversionFlags);
	void PerformSyntaxErrorCheck();

	void Undo();
	void Redo();

	void SetTextContent(LPCTSTR text, BOOL convertIt, BOOL resetUndoStack, BOOL addToUndoStack);

	// this method returns the amount of TCHAR's copied into 'text' or a negative value to indicate an error
	// NOTE: the caller is responsible for freeing the string!!!
	intX GetTextContent(TCHAR** text);
	int GetTextLength();
	void InsertText(LPCTSTR text, BOOL addToUndoStack);
	void DeleteAllandReset(LPCTSTR defaultText);
	void SetSelection(CHARRANGE* cr, BOOL activateBlocker);

	void SetCaret(CaretIndex index);
	void UpdateFocusRect();
	void UpdateFocusRectAsync(DWORD delay);

	int OnEditNotify(HWND, WPARAM, LPARAM);
	int OnEnChange(HWND, WPARAM, LPARAM);

	void SetAutoColor(BOOL);
	void SetAutoComplete(BOOL);
	void SetAutoNum(BOOL isOn, int num_Step);
	void SetUppercase(BOOL);
	void SetFocusMark(BOOL);
	void SetMarkcolor(BOOL);
	void SetCharHeight(int);
	void SetFont(const TCHAR*);
	void SetBold(BOOL enable);
	void SetLineOffset(LONG offset);
	void SetAutosyntaxProperty(LPAUTOSYNTAXPROPERTY pSyntax);
	void SetFocusMarkCorrectionValue(int top, int bottom);

	void insertUserdefinedTriggerStrings(LPCTSTR subProg, LPCTSTR endProg, LPCTSTR newLine);

	void SetAutocompleteStrings(LPAUTOCOMPLETESTRINGS ACstrings, int count);
	//void SetMarkcolorInfo(LPMARKCOLORSTRUCT mcs, int size, BOOL convert);			// for later :)

	static void initEditStyleColorsWithSingleColor(LPEDITSTYLECOLORS esc, COLORREF color);

	DWORD conversionFlagsFromObjectSetup();

private:
	HWND EditWnd;
	HWND AssocConnect;

	DWORD inputCounter;
	BOOL N_open;
	BOOL N_inside; // localize ???
	TCHAR inputBuffer[1024];
	TCHAR* editboxContent;
	TCHAR backspaceChar;
	int numOfAutocompleteStrings;
	//int scrollPosition;

	BOOL Success;
	BOOL ContentChanged;
	BOOL ConversionNotificationBlocker;
	BOOL focusWasInvisible;
	BOOL focusMarkVisibilityCheckupThreadActive;
	BOOL selectionColorErased;
	BOOL suppressSelectionNotification;
	BOOL skipOnClearExecution;
	BOOL suppressLineNumberRemoval;
	BOOL lineNumberIsAboutToBeRemoved;
	BOOL annotationMarkWasRemoved;
	BOOL blockLinenumberRemoval;

	CHARFORMAT textFormat;
	int oldFocusCaretPosition;

	COLORREF sysHighlightColors[2];

	EDITSTYLECOLORS editStyleColors;
	EDITCONTROLPROPERTIES editControlProperties;
	LPAUTOCOMPLETESTRINGS pAcStrings;
	LNFMTDATA lnFormat;
	SCROLLINFO scrollInfo;

	//LPMARKCOLORSTRUCT pMcolor;

	UndoRedoChain UndoStack;

	TriggerWordContainer subProgTrigger;
	TriggerWordContainer endProgTrigger;
	TriggerWordContainer newLineTrigger;

	static LRESULT CALLBACK EditSub(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
	static DWORD WINAPI DelayThreadProc(LPVOID);
	static DWORD WINAPI focusMarkProc(LPVOID);

	LRESULT OnChar(HWND, WPARAM, LPARAM);
	LRESULT OnLButtonDown();
	LRESULT OnKeydown(WPARAM, LPARAM);
	LRESULT OnSize();
	LRESULT OnUpdateLine(WPARAM wParam);
	LRESULT OnMousewheel(WPARAM);

	void NewUndoAction(int, WPARAM);
	void onClear();
	void onPaste();
	void onCut();
	void onDeleteKeyWasPressed();

	void EditChangeCTRL();
	void FormatText();
	void FormatLinefeeds();
	void FormatAll();
	void FormatLine(int lineNbr);
	void ResetInputBuffer();
	void AdaptExistingFormat();
	void ConvertSyntax(BOOL addToUndoStack);
	void ConvertCharHeight();
	void ConvertBoldEffect();
	void ConvertLineOffset();
	//void ConvertMarkcolor();			// for later :) ->initalize in constructor

	void SetColor(COLORREF);
	void SetWordColor(COLORREF);
	void ColorUpdate();
	COLORREF GetCharColor(TCHAR);
	// if( buffer_out == nullptr ) -> the function returns the size of the buffer for the converted text or a negative value if failed
	// if buffer_out is a valid pointer, the return-value is nonzero by success, otherwise it is zero
	int executeSyntaxConversion(_In_ LPCTSTR buffer_in, _Inout_opt_ LPTSTR buffer_out);

	void SetDefaultStyleColors();
	void SetDefaultEditProperties();
	
	void ProcessLinefeed();
	void ProcessBackspaceAfter();
	void ProcessBackspaceBefore();

	WPARAM makeUppercase(WPARAM);

	BOOL ControlInputBuffer(WPARAM);

	BOOL AnnotationControl(WPARAM, TCHAR*);		// (buffer allocation v)
	void WordcolorControl(TCHAR*);				// (buffer allocation v)
	BOOL AutocompleteControl();
	void AutoNumControl(TCHAR*);				// (buffer allocation v) + (internal selection)

	BOOL CheckForNumber(TCHAR);
	BOOL CheckForLetter(TCHAR);
	intX GetLineFromCurrentPosition();

	void CheckPosition(COLORREF, TCHAR*);		// (buffer allocation v) + (internal selection)
	void CheckEnvironment(TCHAR*);				// (buffer allocation v) + (internal selection)
	BOOL CheckLineNumberRemoval(TCHAR*);		// (buffer allocation v)
	BOOL DetectInsideN(TCHAR*);					// (buffer allocation v)
	void CheckAnnotationMarkRemoval(LPCTSTR buffer);
	void CheckFocusmarkVisibility();

	void CheckDelayAsync(UINT, DWORD);
	void CheckDelayAsync(UINT, DWORD, WPARAM);
	void CheckFocusMarkVisibilityAsync();

	void saveScrollPosition();
	void restoreSavedScrollPosition();

	// focus-rect control methods
	/*void UpdateFocusRect();*/
	void UpdateFocusRect_noFlicker();
	void setFocusRect();
	void getFocusRect(LPRECT, int);
	void eraseFocusRect();

	int getFocusMarkCorrectionValue(int type);
	TCHAR getCharAtCaret(CHARRANGE*);

	void eraseSystemHighlightColor();
	void restoreSystemHighlightColor();

	// undo methods
	void undoBackspace(CHARRANGE* cr, LPCTSTR content);
	void undoDeletedContent(CHARRANGE* cr, LPCTSTR content);
	void undoAddedNewchar(CHARRANGE* cr);
	void undoInsertedText(CHARRANGE* cr);
	void undoReplacedSelection(LPUNDOREDOACTIONS action);
	void undoReplacedContent(LPUNDOREDOACTIONS action);
	// redo methods
	void redoBackspace(CHARRANGE* cr);
	void redoDeletedContent(CHARRANGE* cr);
	void redoAddedNewchar(CHARRANGE* cr, LPCTSTR content);
	void redoInsertedText(CHARRANGE* cr, LPCTSTR content);
	void redoReplacedSelection(LPUNDOREDOACTIONS action);
	void redoReplacedContent(LPUNDOREDOACTIONS action);

	void applyColorToSelection(CHARRANGE*, CHARRANGE*, COLORREF, BOOL, BOOL);
	COLORREF getFocusMarkColor();
	void moveCursor(int offset);
	bool changeLinenumberInCurrentLine(int pos_from, LPCTSTR buffer, DWORD mode, int* selectionOffset_out);
	// -1 means there is no linenumber or the buffer was invalid or the counter reached zero
	// if succeeded: pcr_out contains the position of the linenumber
	int getLinenumberFromPreviousLine(int pos, LPCTSTR buffer, CHARRANGE* pcr_out);
	// -1 means there is no linenumber or the buffer was invalid or the counter reached zero
	// if succeeded: pcr_out contains the position of the linenumber
	int getLinenumberFromCurrentLine(int pos, LPCTSTR buffer, CHARRANGE* pcr_out);

	BOOL checkAutoInsertBrackets(WPARAM wParam);
	// if this function returns false, no autonum should be executed!
	BOOL checkCurrentLineForSpecialTrigger(TCHAR* buffer);															// (buffer allocation v)
	COLORREF getCurrentColor(DWORD mode);
	int addLineNumberToBufferWithCurrentConfig(int num, LPTSTR buffer, int from);
	int roundOffLineNumber(int currentNumber);
	int checkNextLineForTrigger(int pos_from, LPCTSTR buffer, int currentLinenumber);
	bool isLineInSubProgram(int pos_from, LPCTSTR buffer, int max);
	// return value: -1 -> error or not found
	int getLastMainLevelLinenumber(int pos_from, LPCTSTR buffer);
	bool isValidNCWordContent(TCHAR L);
	bool getAnnotationScope(const int pos_from, LPCTSTR buffer, CHARRANGE *scope_out,const int max, bool multiline);
	// the method returns an End in any case, even if the line-end was reached(not multiline) or
	// the end of the buffer was reached(multiline) -> aFound is true when an appropriate closing tag was found
	// if the method fails the return value is -1
	int getAnnotationEnd(const int pos_from, TCHAR type, LPCTSTR buffer, const int max, bool multiline, bool* aFound);
	bool createConverterStructFromBuffer(LPCTSTR buffer, itemCollection<ColorConversion> &converterImage);

	void _setsel(CHARRANGE*);
	void _getsel(CHARRANGE*);
	intX _lineFromCharIndex(int);
	void _restoreContent(bool restoreScroll);
	void _removeCommentFromLineBuffer(LPTSTR buffer);
	void getCurrentFormat(CHARFORMAT* ccf);
	bool isEmptyLine(LPCTSTR buffer, int len);

	void updateContentBuffer();
	void eraseContentBuffer();

	bool isVariableStatement(int pos_from, LPCTSTR buffer, COLORREF *color_out);
	bool isLineNumber(int pos_from, LPCTSTR buffer, int caretPos, bool actionWasBackspace);
	bool isFirst_N_inLine(LPCTSTR buffer, int pos);

	// temp methods
	void showValues();
};

EditControl* CreateEditControlInstance(HWND EditWnd, HWND AssocConnect);