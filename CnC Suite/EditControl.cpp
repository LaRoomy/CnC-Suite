#include "EditControl.h"
#include "HelperF.h"
#include "Error dispatcher.h"
#include "Async.h"

void _updatefocusrect(LPVOID lParam)
{
	reinterpret_cast<EditControl*>(lParam)->UpdateFocusRect();
}

EditControl::EditControl(HWND Editwnd, HWND Connector)
	: EditWnd(Editwnd),
	AssocConnect(Connector),
	Success(FALSE),
	ContentChanged(FALSE),
	inputCounter(0),
	N_open(FALSE),
	N_inside(FALSE),
	pAcStrings(nullptr),
	//scrollPosition(-1),
	ConversionNotificationBlocker(FALSE),
	oldFocusCaretPosition(0),
	focusWasInvisible(FALSE),
	focusMarkVisibilityCheckupThreadActive(FALSE),
	selectionColorErased(FALSE),
	suppressSelectionNotification(FALSE),
	skipOnClearExecution(FALSE),
	suppressLineNumberRemoval(FALSE),
	lineNumberIsAboutToBeRemoved(FALSE),
	annotationMarkWasRemoved(FALSE),
	blockLinenumberRemoval(FALSE),
	backspaceChar(L'\0'),
	editboxContent(nullptr),
	isTextSelected(FALSE),
	currentSelectedLine(-1),
	previousSelectedLine(-1)
{
	SecureZeroMemory(&this->inputBuffer, sizeof(this->inputBuffer));
	SecureZeroMemory(&this->editStyleColors, sizeof(EDITSTYLECOLORS));
	SecureZeroMemory(&this->editControlProperties, sizeof(EDITCONTROLPROPERTIES));
	SecureZeroMemory(&this->lnFormat, sizeof(LNFMTDATA));

	this->SetDefaultEditProperties();
	this->SetDefaultStyleColors();

	this->subProgTrigger.setContentFromTriggerString(SUBPROG_TRIGGER_DEFAULTSTRING);
	this->endProgTrigger.setContentFromTriggerString(ENDPROG_TRIGGER_DEFAULTSTRING);
	this->newLineTrigger.setContentFromTriggerString(NEWLINE_TRIGGER_DEFAULTSTRING);

	this->Success = SetWindowSubclass(Editwnd, EditControl::EditSub, NULL, reinterpret_cast<DWORD_PTR>(this));
	if (this->Success)
	{
		SendMessage(Editwnd, EM_SETEVENTMASK, 0, static_cast<LPARAM>(ENM_CHANGE | ENM_SELCHANGE | ENM_CLIPFORMAT | ENM_KEYEVENTS));
		SetFocus(Editwnd);
	}
}

EditControl::~EditControl()
{
	this->eraseContentBuffer();

	if (this->pAcStrings != nullptr)
	{
		if (this->numOfAutocompleteStrings == 1)
			delete this->pAcStrings;
		else
			delete[] this->pAcStrings;
	}
}

BOOL EditControl::ConfigureComponent(LPEDITSTYLECOLORS pESC, LPEDITCONTROLPROPERTIES pECS, BOOL redraw, BOOL convertText)
{
	if (pESC != nullptr)
	{
		SecureZeroMemory(&this->editStyleColors, sizeof(EDITSTYLECOLORS));

		this->editStyleColors.background = pESC->background;
		this->editStyleColors.defaultTextcolor = pESC->defaultTextcolor;
		this->editStyleColors.Annotation = pESC->Annotation;
		this->editStyleColors.LineNumber = pESC->LineNumber;
		this->editStyleColors.A = pESC->A;
		this->editStyleColors.B = pESC->B;
		this->editStyleColors.C = pESC->C;
		this->editStyleColors.D = pESC->D;
		this->editStyleColors.E = pESC->E;
		this->editStyleColors.F = pESC->F;
		this->editStyleColors.G = pESC->G;
		this->editStyleColors.H = pESC->H;
		this->editStyleColors.I = pESC->I;
		this->editStyleColors.J = pESC->J;
		this->editStyleColors.K = pESC->K;
		this->editStyleColors.L = pESC->L;
		this->editStyleColors.M = pESC->M;
		this->editStyleColors.N = pESC->N;
		this->editStyleColors.O = pESC->O;
		this->editStyleColors.P = pESC->P;
		this->editStyleColors.Q = pESC->Q;
		this->editStyleColors.R = pESC->R;
		this->editStyleColors.S = pESC->S;
		this->editStyleColors.T = pESC->T;
		this->editStyleColors.U = pESC->U;
		this->editStyleColors.V = pESC->V;
		this->editStyleColors.W = pESC->W;
		this->editStyleColors.X = pESC->X;
		this->editStyleColors.Y = pESC->Y;
		this->editStyleColors.Z = pESC->Z;

		this->ColorUpdate();
	}
	if (pECS != nullptr)
	{
		this->editControlProperties.autocolor = pECS->autocolor;
		this->editControlProperties.autocomplete = pECS->autocomplete;
		this->editControlProperties.autonum = pECS->autonum;
		this->editControlProperties.uppercase = pECS->uppercase;
		this->editControlProperties.focusmark = pECS->focusmark;
		this->editControlProperties.charHeight = pECS->charHeight;
		this->editControlProperties.lineOffset = pECS->lineOffset;
		this->editControlProperties.isBold = pECS->isBold;
		this->textFormat.dwEffects = pECS->isBold ? CFE_BOLD : 0;
		this->textFormat.yOffset = pECS->lineOffset;
		this->textFormat.yHeight = pECS->charHeight;
		StringCbCopy(this->editControlProperties.charSet, sizeof(this->editControlProperties.charSet), pECS->charSet);
		StringCbCopy(this->textFormat.szFaceName, sizeof(this->textFormat.szFaceName), pECS->charSet);

		this->editControlProperties.autosyntax = pECS->autosyntax;
	}
	if (convertText)
	{
		DWORD conversionFlags = HIDE_WHILE_CONVERTING | CONVERT_CHARHEIGHT;

		if (this->editControlProperties.autocolor)
			conversionFlags |= CONVERT_TOMULTICOLOR;
		else
			conversionFlags |= CONVERT_TOSINGLECOLOR;

		if (this->editControlProperties.autosyntax.IsOn)
			conversionFlags |= CONVERT_SYNTAX;

		this->ConvertContent(conversionFlags);
	}
	if (redraw)
	{		
		RedrawWindow(this->EditWnd, NULL, NULL, RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE);
	}
	return TRUE;
}

void EditControl::UpdateProperties(LPEDITCONTROLPROPERTIES properties, BOOL forceConversion)
{
	if (properties != nullptr)
	{
		BOOL propChanged = FALSE;

		if (this->editControlProperties.autocolor != properties->autocolor)
			propChanged = TRUE;
		if (this->editControlProperties.autocomplete != properties->autocomplete)
			propChanged = TRUE;
		if (this->editControlProperties.autonum != properties->autonum)
			propChanged = TRUE;
		if (this->editControlProperties.uppercase != properties->uppercase)
			propChanged = TRUE;
		//if (this->editControlProperties.numStep != properties->numStep)
		//	propChanged = TRUE;
		if (this->editControlProperties.focusmark != properties->focusmark)
			propChanged = TRUE;
		if (this->editControlProperties.charHeight != properties->charHeight)
			propChanged = TRUE;
		if (!CompareStringsB(this->editControlProperties.charSet, properties->charSet))
			propChanged = TRUE;

		if (propChanged || forceConversion)
		{
			this->editControlProperties.autocolor = properties->autocolor;
			this->editControlProperties.autocomplete = properties->autocomplete;
			this->editControlProperties.autonum = properties->autonum;
			this->editControlProperties.uppercase = properties->uppercase;
			//this->editControlProperties.numStep = properties->numStep;
			this->editControlProperties.focusmark = properties->focusmark;
			this->editControlProperties.charHeight = properties->charHeight;
			this->textFormat.yHeight = properties->charHeight;//ConvertXPixToTwips(this->EditWnd, properties->charHeight);
			StringCbCopy(this->editControlProperties.charSet, sizeof(this->editControlProperties.charSet), properties->charSet);
			StringCbCopy(this->textFormat.szFaceName, sizeof(this->textFormat.szFaceName), properties->charSet);

			DWORD conversionFlags = HIDE_WHILE_CONVERTING | CONVERT_CHARHEIGHT;

			if (this->editControlProperties.autocolor)conversionFlags |= CONVERT_TOMULTICOLOR;
			else conversionFlags |= CONVERT_TOSINGLECOLOR;
			if (this->editControlProperties.autosyntax.IsOn)conversionFlags |= CONVERT_SYNTAX;

			this->ConvertContent(conversionFlags);
		}
	}
}

void EditControl::ChangeVAR_Reset()
{
	this->ContentChanged = FALSE;
}

void EditControl::GetColorInfo(LPEDITSTYLECOLORS esc)
{
	esc->A = this->editStyleColors.A;
	esc->background = this->editStyleColors.background;
	esc->defaultTextcolor = this->editStyleColors.defaultTextcolor;

	// ...... more
}

LRESULT EditControl::EditSub(HWND Edit, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR RefData)
{
	UNREFERENCED_PARAMETER(uID);

	EditControl* pEdit = reinterpret_cast<EditControl*>(RefData);
	if (pEdit != NULL)
	{
		switch (message)
		{
		case WM_SIZE:
			return pEdit->OnSize();
		case WM_KEYDOWN:
			pEdit->OnKeydown(wParam, lParam);
			break;
		case WM_CHAR:
			return pEdit->OnChar(Edit, wParam, lParam);
		case WM_CHECKENVIRONMENT:
			pEdit->CheckEnvironment(nullptr);
			return 0;
		case WM_DETECTINSIDE_N:
			pEdit->DetectInsideN(nullptr);
			return 0;
		case WM_UPDATEFOCUSRECT:
			pEdit->UpdateFocusRect();
			return 0;
		case WM_VSCROLL:
			pEdit->CheckFocusMarkVisibilityAsync();
			break;
		case WM_LBUTTONDOWN:
			pEdit->OnLButtonDown();
			break;
		case WM_UPDATELINE:
			return pEdit->OnUpdateLine(wParam);
		case WM_MOUSEWHEEL:
			if (pEdit->OnMousewheel(wParam))
				return 0;
			else
				break;

		//case WM_SETFOCUS:
			//pEdit->UpdateFocusRect();
		//	pEdit->eraseSystemHighlightColor();
			//break;
		//case WM_KILLFOCUS:
			//pEdit->eraseFocusRect();
		//	pEdit->restoreSystemHighlightColor();
			//break;
		//case WM_RBUTTONDOWN:// temp !!!!!!!!!!!!!!!!!!!!!!
		//	pEdit->showValues();
		//	break;
		case WM_CLEAR:
			pEdit->onClear();
			break;
		case WM_PASTE:
			pEdit->onPaste();
			return 0;
		default:
			break;
		}
	}
	return DefSubclassProc(Edit, message, wParam, lParam);
}

DWORD EditControl::DelayThreadProc(LPVOID lParam)
{
	LPDELAYTHREADDATA pdt =
		reinterpret_cast<LPDELAYTHREADDATA>(lParam);
	if (pdt != nullptr)
	{
		Sleep(pdt->milliSeconds);

		HWND Main = FindWindow(IDSEX_APPLICATIONCLASS, nullptr);
		if (Main != nullptr)
		{
			HWND tabFrame = GetDlgItem(Main, ID_TABCTRLFRAME);
			if (tabFrame != nullptr)
			{
				HWND thisEdit = GetDlgItem(tabFrame, pdt->ctrlID);
				if (thisEdit != nullptr)
				{
					SendMessage(thisEdit, pdt->message, pdt->wParam, 0);
				}
			}
		}
		SafeDelete(&pdt);
	}
	return 0;
}

DWORD EditControl::focusMarkProc(LPVOID lParam)
{
	auto _this = reinterpret_cast<EditControl*>(lParam);
	if (_this != nullptr)
	{
		//InterlockedExchange((LONG*)&_this->focusMarkVisibilityCheckupThreadActive, (LONG)TRUE);

		do
		{
			InterlockedExchange((LONG*)&_this->focusMarkVisibilityCheckupThreadActive, (LONG)TRUE);

			Sleep(300);

			_this->CheckFocusmarkVisibility();

		} while (_this->focusMarkVisibilityCheckupThreadActive == 2);

		if (_this->focusMarkVisibilityCheckupThreadActive == PERFORM_RELEASE_ASYNC)
		{
			delete _this;
			return 0;
		}

		InterlockedExchange((LONG*)&_this->focusMarkVisibilityCheckupThreadActive, (LONG)FALSE);
	}
	return 0;
}

LRESULT EditControl::OnChar(HWND Edit, WPARAM wParam, LPARAM lParam)
{
	// if a special char will be inserted with the alt-gr key, the control key state will be also true (why?)
	// to get this working the alt key must also be checked
	auto ctrlKeyState = GetKeyState(VK_CONTROL);
	auto ctrlKeyIsPressed = (ctrlKeyState & 0x8000) ? true : false;
	auto altKeyState = GetKeyState(VK_MENU);
	auto altKeyIsPressed = (altKeyState & 0x8000) ? true : false;
	
	if (ctrlKeyIsPressed && !altKeyIsPressed)
		return 0;	// this was a ctrl command, don't insert it

	this->EditChangeCTRL();

	switch (wParam)
	{
	case 0x0D:
		this->ProcessLinefeed();
		break;
	case 0x08:
		this->ProcessBackspaceAfter();
		break;
	default:
		// check uppercase
		wParam = this->makeUppercase(wParam);

		this->NewUndoAction(UNDOACTION_NEWCHAR_ADDED, wParam);

		if (this->ControlInputBuffer(wParam))
		{
			return 0;
		}		
		break;
	}
	return DefSubclassProc(Edit, WM_CHAR, wParam, lParam);
}

LRESULT EditControl::OnLButtonDown()
{
	this->ResetInputBuffer();

	this->DetectInsideN(nullptr);

	return static_cast<LRESULT>(0);
}

LRESULT EditControl::OnKeydown(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (wParam)
	{
	case VK_BACK:
		this->ProcessBackspaceBefore();
		//NewUndoAction(UNDOACTION_BACKSPACE, (WPARAM)(0x08));
		break;
	//case VK_F8: // temp
	//{
	//	// temp
	//	this->skipOnClearExecution = FALSE;
	//}
	//	break;
	default:
		break;
	}
	return static_cast<LRESULT>(0);
}

LRESULT EditControl::OnSize()
{
	if (this->editControlProperties.focusmark)
	{
		//this->UpdateFocusRect();
		this->setFocusRect();
	}
	return static_cast<LRESULT>(0);
}

LRESULT EditControl::OnUpdateLine(WPARAM wParam)
{
	this->FormatLine(
		(int)this->_lineFromCharIndex((int)wParam)
	);
	this->UpdateFocusRect();

	return static_cast<LRESULT>(0);
}

LRESULT EditControl::OnMousewheel(WPARAM wParam)
{
	this->CheckFocusMarkVisibilityAsync();

	auto kState = LOWORD(wParam);
	auto ctrlIsPressed = (kState & 0x08) ? true : false;
	if (ctrlIsPressed)
	{
		return static_cast<LRESULT>(TRUE);
	}
	return static_cast<LRESULT>(0);
}

void EditControl::NewUndoAction(int action, WPARAM wParam)
{
	TCHAR cont[2] = { L'\0' };
	cont[0] = (TCHAR)wParam;

	CHARRANGE curRange;
	GETCURRENTCHARRANGE(this->EditWnd, &curRange);

	switch (action)
	{
	case UNDOACTION_CONTENT_DELETED:
		break;
	case UNDOACTION_BACKSPACE:
		if (curRange.cpMax == curRange.cpMin)
		{
			curRange.cpMax -= 1;
			curRange.cpMin -= 1;

			UNDOREDOACTIONS ura;
			ura.action = action;
			ura.range = curRange;
			ura.content = cont;
			ura.replacedContent = nullptr;
			ura.replacedRange = { 0,0 };

			this->UndoStack.addNewUndoAction(&ura);
		}
		else
		{			
			this->onClear();
			this->suppressLineNumberRemoval = TRUE;
		}
		break;
	case UNDOACTION_NEWCHAR_ADDED:
		curRange.cpMin += 1;
		curRange.cpMax += 1;

		UNDOREDOACTIONS ura;
		ura.action = action;
		ura.range = curRange;
		ura.content = cont;
		ura.replacedContent = nullptr;
		ura.replacedRange = { 0,0 };

		this->UndoStack.addNewUndoAction(&ura);
		break;
	case UNDOACTION_TEXT_INSERTED:
		break;
	default:
		break;
	}
}

void EditControl::onClear()
{
	if (this->skipOnClearExecution)
	{
		this->skipOnClearExecution = FALSE;
	}
	else
	{
		TCHAR* selection = nullptr;

		if (GetRichEditSelectionContent(this->EditWnd, &selection) == TRUE)
		{
			CHARRANGE cr;
			GETCURRENTCHARRANGE(this->EditWnd, &cr);

			if (this->lineNumberIsAboutToBeRemoved)
			{
				// this is the remove action of the linenumber
				// so reassemble the linenumber-string and replace it on the undo-stack
				UNDOREDOACTIONS action;
				SecureZeroMemory(&action, sizeof(UNDOREDOACTIONS));

				// first get the preliminary added backspace-action
				this->UndoStack.getActionAt(0, &action);

				if (action.action == UNDOACTION_BACKSPACE)
				{
					iString _selectn(selection);

					int insertPnt = action.range.cpMin - cr.cpMin;

					if(action.content != nullptr)
						_selectn.insertCharAt(insertPnt, action.content[0]);

					action.action = UNDOACTION_CONTENT_DELETED;
					action.range.cpMin = cr.cpMin;
					action.range.cpMax = cr.cpMax + 1;

					SafeDeleteArray(&action.content);
					CopyStringToPtr(_selectn.GetData(), &action.content);

					this->UndoStack.replaceActionAt(0, &action);

					SafeDeleteArray(&action.content);
				}
				this->lineNumberIsAboutToBeRemoved = FALSE;
			}
			else
			{
				UNDOREDOACTIONS ura;
				ura.action = UNDOACTION_CONTENT_DELETED;
				ura.range = cr;
				ura.content = selection;
				ura.replacedContent = nullptr;
				ura.replacedRange = { 0, 0 };

				this->UndoStack.addNewUndoAction(&ura);
			}
			SafeDeleteArray(&selection);
		}
	}
}

void EditControl::onPaste()
{
	if (IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		if (OpenClipboard(this->AssocConnect))
		{
			auto hglb = GetClipboardData(CF_UNICODETEXT);

			if (hglb)
			{
				LPTSTR lptstr = reinterpret_cast<LPTSTR>(GlobalLock(hglb));

				if (lptstr != nullptr)
				{
					this->InsertText(lptstr, TRUE);

					GlobalUnlock(hglb);
				}
			}
			CloseClipboard();
		}
	}
}

void EditControl::onCut()
{
	CHARRANGE cr;
	this->_getsel(&cr);

	UNDOREDOACTIONS action;
	action.action = UNDOACTION_CONTENT_DELETED;
	action.range = cr;
	action.content = nullptr;
	action.replacedContent = nullptr;
	action.replacedRange = { 0 };

	GetRichEditSelectionContent(this->EditWnd, &action.content);

	this->UndoStack.addNewUndoAction(&action);
}

void EditControl::onDeleteKeyWasPressed()
{
	CHARRANGE cr;
	GETCURRENTCHARRANGE(this->EditWnd, &cr);

	if (cr.cpMax == cr.cpMin)
	{
		TCHAR cont[2] = { L'\0' };
		cont[0] = this->getCharAtCaret(&cr);

		UNDOREDOACTIONS ura;
		ura.action = UNDOACTION_BACKSPACE;
		ura.range = cr;
		ura.content = cont;

		this->UndoStack.addNewUndoAction(&ura);
	}
	else
	{
		this->onClear();
	}
}

int EditControl::OnEditNotify(HWND Parent, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(Parent);
	UNREFERENCED_PARAMETER(wParam);

	//MSGFILTER* msgFilter = nullptr;
	//SELCHANGE* pSel = nullptr;

	switch (((LPNMHDR)lParam)->code)
	{
	case EN_SELCHANGE:
	{
		if (this->ConversionNotificationBlocker)
			return 0;
		if (this->suppressSelectionNotification)
			return 0;

		SELCHANGE* pSel = reinterpret_cast<SELCHANGE*>(lParam);

		if (pSel->nmhdr.hwndFrom == this->EditWnd)
		{
			if(pSel->seltyp == SEL_EMPTY)
			{
				auto index = pSel->chrg.cpMax;
				auto line = static_cast<int>(SendMessage(this->EditWnd, EM_EXLINEFROMCHAR, 0, (LPARAM)index));
				auto lineindex = index - SendMessage(this->EditWnd, EM_LINEINDEX, (WPARAM)-1, 0);

				this->previousSelectedLine = this->currentSelectedLine;
				this->currentSelectedLine = line;

				if ((line >= 0) && (lineindex >= 0))
				{
					HRESULT hr;

					STBUPDATE sbUp;
					SecureZeroMemory(&sbUp, sizeof(sbUp));
					sbUp.part = 2;

					hr = StringCbPrintf(
						sbUp.text,
						sizeof(sbUp.text),
						L"  %s %i     %s %i",
						getStringFromResource(UI_EDITCONTROL_LINE),
						line + 1,
						getStringFromResource(UI_EDITCONTROL_COLUMN),
						(int)(lineindex + 1));

					if (SUCCEEDED(hr))
					{
						_MSG_TO_MAIN(WM_UPDATESTATUSBAR, 0, reinterpret_cast<LPARAM>(&sbUp));
					}

					if (this->isTextSelected)
					{
						this->setFocusRect();
						this->isTextSelected = FALSE;
					}
					else
					{
						if (this->previousSelectedLine != -1)
						{
							if (this->previousSelectedLine != this->currentSelectedLine)
							{
								this->UpdateFocusRect();
							}
						}
						else
						{
							this->UpdateFocusRect();
						}
					}
				}
			}
			else
			{
				if (pSel->seltyp & SEL_TEXT)
				{
					HRESULT hr;

					STBUPDATE sbUp;
					SecureZeroMemory(&sbUp, sizeof(sbUp));
					sbUp.part = 2;

					hr = StringCbPrintf(sbUp.text, sizeof(sbUp.text), L" < %s >", getStringFromResource(UI_EDITCONTROL_SELECTIONRANGE));

					if (SUCCEEDED(hr))
					{
						_MSG_TO_MAIN(WM_UPDATESTATUSBAR, 0, reinterpret_cast<LPARAM>(&sbUp));
					}

					//this->UpdateFocusRect();
					if (!this->isTextSelected)
					{
						this->eraseFocusRect();
						this->isTextSelected = TRUE;
					}
				}
				//if (pSel->seltyp & SEL_MULTICHAR)
				//{
				//	//showText(L"multichar");
				//}
			}
		}
	}	break;

	case EN_CLIPFORMAT:
	{
		DWORD flags
			= (this->editControlProperties.autocolor)
			? CONVERT_TOMULTICOLOR : CONVERT_TOSINGLECOLOR;

		flags |= HIDE_WHILE_CONVERTING;

		if (this->editControlProperties.autosyntax.IsOn)flags |= CONVERT_SYNTAX;
		//if (this->editControlProperties.markcolor)flags |= CONVERT_MARKCOLOR;

		this->ConvertContent(flags);
		this->EditChangeCTRL();

		SetFocus(this->EditWnd);
	}	break;
	case EN_MSGFILTER:
	{
		// return 1 to prevent default processing

		auto msgFilter = reinterpret_cast<MSGFILTER*>(lParam);
		if (msgFilter != nullptr)
		{
			if (msgFilter->msg == WM_KEYDOWN)
			{
				auto ctrlKeyState = GetKeyState(VK_CONTROL);
				auto ctrlKeyIsPressed = (ctrlKeyState & 0x8000) ? true : false;

				auto shiftKeyState = GetKeyState(VK_SHIFT);
				auto shiftKeyIsPressed = (shiftKeyState & 0x8000) ? true : false;

				if (ctrlKeyIsPressed)
				{
					switch (msgFilter->wParam)
					{
					case 0x58: // ctrl + x (cut)
						this->EditChangeCTRL();
						this->onCut();
						break;
					case 0x52:	// ctrl + r
						return 1;
					case 0x5A:	// ctrl + z (undo)
						this->Undo();
						return 1;
					case 0x59:	// ctrl + y (redo)
						this->Redo();
						return 1;
					case 0x56:	// ctrl + v (paste)
						SendMessage(this->EditWnd, WM_PASTE, 0, 0);
						this->UpdateFocusRect();
						return 1;
					default:
						break;
					}
				}
				else
				{
					if (msgFilter->wParam == VK_DELETE)	// del was pressed
					{
						if (!shiftKeyIsPressed)
						{
							this->onDeleteKeyWasPressed();
							this->EditChangeCTRL();
						}
					}
				}
			}
		}
	}	break;
	default:
		break;
	}
	return 0;
}

int EditControl::OnEnChange(HWND frame, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(frame);
	UNREFERENCED_PARAMETER(wParam);

	if (((HWND)lParam) == this->EditWnd)
	{
		//showText(L"en_change !");

		this->UpdateFocusRect();

		//this->CheckDelayAsync(WM_UPDATEFOCUSRECT, 50);
	}
	return 0;
}

void EditControl::SetAutoColor(BOOL ac)
{
	this->editControlProperties.autocolor = ac;
	 
	if (ac)
	{
		this->ConvertContent(CONVERT_TOMULTICOLOR | HIDE_WHILE_CONVERTING);
	}
	else
	{
		this->ConvertContent(CONVERT_TOSINGLECOLOR | HIDE_WHILE_CONVERTING);
	}
	this->UpdateFocusRect();
}

void EditControl::SetAutoComplete(BOOL ac)
{
	this->editControlProperties.autocomplete = ac;
}

void EditControl::SetAutoNum(BOOL isOn, int num_Step)
{
	this->editControlProperties.autonum = isOn;

	if (num_Step > 0)
	{
		//this->editControlProperties.numStep = num_Step;
		this->editControlProperties.autosyntax.numStepInMainProgramm = num_Step;
	}
}

void EditControl::SetUppercase(BOOL uc)
{
	this->editControlProperties.uppercase = uc;
}

void EditControl::SetFocusMark(BOOL frect)
{
	this->editControlProperties.focusmark = frect;

	// erase or set the rect
	if (!frect)
	{
		this->eraseFocusRect();
	}
	else
	{
		this->setFocusRect();
	}
}

void EditControl::SetMarkcolor(BOOL mc)
{
	//this->editControlProperties.markcolor = mc;

	if (mc)
	{
		//convertBackground
	}
	else
	{
		// emptyBackground
	}
}

void EditControl::EditChangeCTRL()
{
	if (!this->ContentChanged)
		this->ContentChanged = TRUE;

	SendMessage(this->AssocConnect, WM_COMMAND, MAKEWPARAM(EN_CHANGE, 0), 0);
}

void EditControl::FormatText()
{
	this->textFormat.dwMask = CFM_ALL | CFM_COLOR;
	this->textFormat.cbSize = sizeof(CHARFORMAT);
	this->textFormat.bCharSet = ANSI_CHARSET;
	this->textFormat.crTextColor = this->editStyleColors.defaultTextcolor;
	StringCbCopy(this->textFormat.szFaceName, sizeof(this->textFormat.szFaceName), this->editControlProperties.charSet);
	this->textFormat.yHeight = this->editControlProperties.charHeight;//ConvertXPixToTwips(this->EditWnd, this->editControlProperties.charHeight);
	this->textFormat.yOffset = this->editControlProperties.lineOffset;
	//this->textFormat.dwEffects = CFE_BOLD;

	SendMessage(
		this->EditWnd,
		EM_SETCHARFORMAT,
		static_cast<WPARAM>(SCF_ALL),
		reinterpret_cast<LPARAM>(&this->textFormat));

	SendMessage(
		this->EditWnd,
		EM_SETBKGNDCOLOR,
		static_cast<WPARAM>(0),
		static_cast<LPARAM>(this->editStyleColors.background));
}

void EditControl::SetCharHeight(int height)
{
	this->editControlProperties.charHeight = height;
	this->textFormat.yHeight = height;// ConvertXPixToTwips(this->EditWnd, height);
	this->ConvertCharHeight();
}

void EditControl::SetFont(const TCHAR * charSet)
{
	HRESULT hr = StringCbCopy(this->editControlProperties.charSet, sizeof(this->editControlProperties.charSet), charSet);
	if (SUCCEEDED(hr))
	{
		hr = StringCbCopy(this->textFormat.szFaceName, sizeof(this->textFormat.szFaceName), charSet);
		if (SUCCEEDED(hr))
		{
			CHARFORMAT cfm;
			cfm.cbSize = sizeof(CHARFORMAT);
			cfm.dwMask = CFM_FACE;
			StringCbCopy(cfm.szFaceName, sizeof(cfm.szFaceName), charSet);

			SendMessage(this->EditWnd, EM_SETCHARFORMAT, static_cast<WPARAM>(SCF_ALL), reinterpret_cast<LPARAM>(&cfm));
		}
	}
}

void EditControl::SetBold(BOOL enable)
{
	this->editControlProperties.isBold = enable;

	if (enable)
		this->textFormat.dwEffects = CFE_BOLD;
	else
		this->textFormat.dwEffects = 0;

	this->ConvertBoldEffect();
}

void EditControl::SetLineOffset(LONG offset)
{
	this->textFormat.yOffset = offset;
	this->editControlProperties.lineOffset = offset;
	this->ConvertLineOffset();
}

void EditControl::SetAutosyntaxProperty(LPAUTOSYNTAXPROPERTY pSyntax)
{
	if (pSyntax != nullptr)
	{
		this->editControlProperties.autosyntax = *pSyntax;
	}
}

void EditControl::SetFocusMarkCorrectionValue(int top, int bottom)
{
	this->editControlProperties.focusmarkCor_bottom = bottom;
	this->editControlProperties.focusmarkCor_top = top;

	this->UpdateFocusRect();
}

void EditControl::insertUserdefinedTriggerStrings(LPCTSTR subProg, LPCTSTR endProg, LPCTSTR newLine)
{
	if (subProg != nullptr)
	{
		this->subProgTrigger.setContentFromTriggerString(subProg);
	}
	if (endProg != nullptr)
	{
		this->endProgTrigger.setContentFromTriggerString(endProg);
	}
	if (newLine != nullptr)
	{
		this->newLineTrigger.setContentFromTriggerString(newLine);
	}
}

void EditControl::SetAutocompleteStrings(LPAUTOCOMPLETESTRINGS ACstrings, int count)
{
	__try
	{
		this->numOfAutocompleteStrings = count;
		if (this->pAcStrings != nullptr)
		{
			// delete old struct
			if (this->numOfAutocompleteStrings == 1)
				delete this->pAcStrings;
			else
				delete[] this->pAcStrings;
		}
		if (count > 0)
		{
			this->pAcStrings = new AUTOCOMPLETESTRINGS[this->numOfAutocompleteStrings];
			if (this->pAcStrings != nullptr)
			{
				// copy new struct!
				for (int i = 0; i < this->numOfAutocompleteStrings; i++)
				{
					StringCbCopy(this->pAcStrings[i].appendix, sizeof(TCHAR) * 56, ACstrings[i].appendix);
					StringCbCopy(this->pAcStrings[i].trigger, sizeof(TCHAR) * 56, ACstrings[i].trigger);
					this->pAcStrings[i].length = ACstrings[i].length;
				}
			}
		}
	}
	__except (
		GetExceptionCode()
		== EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return;
	}
}

//void EditControl::SetMarkcolorInfo(LPMARKCOLORSTRUCT mcs, int size, BOOL convert)
//{
//	__try
//	{
//		if (this->pMcolor != nullptr)
//		{
//			delete[] this->pMcolor;
//			this->pMcolor = nullptr;
//		}
//		this->pMcolor = new MARKCOLORSTRUCT[size];
//		if (this->pMcolor != nullptr)
//		{
//			for (int i = 0; i < size; i++)
//			{
//				StringCbCopy(this->pMcolor[i].start, sizeof(TCHAR) * 56, mcs[i].start);
//				StringCbCopy(this->pMcolor[i].end, sizeof(TCHAR) * 56, mcs[i].end);
//				this->pMcolor[i].refColor = mcs[i].refColor;
//			}
//		}
//		if (convert)
//		{
//			this->ConvertMarkcolor();
//		}
//	}
//	__except (
//		GetExceptionCode()
//		== EXCEPTION_ACCESS_VIOLATION
//		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
//	{
//		return;
//	}
//}

void EditControl::initEditStyleColorsWithSingleColor(LPEDITSTYLECOLORS esc, COLORREF color)
{	
	esc->Annotation = color;	
	esc->background = color;
	esc->defaultTextcolor = color;
	esc->LineNumber = color;
	esc->A = color;
	esc->B = color;
	esc->C = color;
	esc->D = color;
	esc->E = color;
	esc->F = color;
	esc->G = color;
	esc->H = color;
	esc->I = color;
	esc->J = color;
	esc->K = color;
	esc->L = color;
	esc->M = color;
	esc->N = color;
	esc->O = color;
	esc->P = color;
	esc->Q = color;
	esc->R = color;
	esc->S = color;
	esc->T = color;
	esc->U = color;
	esc->V = color;
	esc->W = color;
	esc->X = color; 
	esc->Y = color;
	esc->Z = color;
}

DWORD EditControl::conversionFlagsFromObjectSetup()
{
	DWORD flags = HIDE_WHILE_CONVERTING;

	if (this->editControlProperties.autosyntax.IsOn)
		flags |= CONVERT_SYNTAX;
		
	flags |= this->editControlProperties.autocolor ? CONVERT_TOMULTICOLOR : CONVERT_TOSINGLECOLOR;

	return flags;
}

void EditControl::ConvertContent(DWORD conversionFlags)
{
	auto isWndVisible = IsWindowVisible(this->EditWnd);

	if (conversionFlags & CONVERT_CUR_COLORING)
	{
		// set the color-flag with the appropriate setting
		conversionFlags |= this->editControlProperties.autocolor
			? CONVERT_TOMULTICOLOR : CONVERT_TOSINGLECOLOR;
	}
	if (isWndVisible)
	{
		if (conversionFlags & HIDE_WHILE_CONVERTING)
		{
			ShowWindow(this->EditWnd, SW_HIDE);
		}
	}
	///////////////////////////////////////////////////
	this->saveScrollPosition();
	///////////////////////////////////////////////////
	if (conversionFlags & CONVERT_LINEFEED)
	{
		// old -> do not use!
		this->FormatLinefeeds();
	}
	//////////////////////////////////////////////////
	if (conversionFlags & CONVERT_SYNTAX)
	{
		CHARRANGE crSel;
		this->_getsel(&crSel);

		this->ConvertSyntax(
			(conversionFlags & CONVERT_ADDTO_UNDOSTACK) ? TRUE : FALSE
		);

		this->_setsel(&crSel);
		this->UpdateFocusRectAsync(300);
	}
	//////////////////////////////////////////////////
	if (conversionFlags & CONVERT_TOMULTICOLOR)
	{
		__try
		{
			this->FormatAll();
		}
		__except (
			GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
			? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			if (isWndVisible)
			{
				ShowWindow(this->EditWnd, SW_SHOW);

				// temp!
				DispatchEWINotification(
					EDSP_ERROR,
					L"EDC0001",
					L"Critical Error - [EXCEPTION_ACCESS_VIOLATION] - in Editcontrol:FormatAll",
					L"Editcontrol"
				);
			}
			return;
		}

		SendMessage(this->EditWnd, EM_EMPTYUNDOBUFFER, 0, 0);
	}
	else if (conversionFlags & CONVERT_TOSINGLECOLOR)
	{
		this->textFormat.crTextColor = this->editStyleColors.defaultTextcolor;

		SendMessage(
			this->EditWnd,
			EM_SETCHARFORMAT,
			static_cast<WPARAM>(SCF_ALL),
			reinterpret_cast<LPARAM>(&this->textFormat));
	}
	//////////////////////////////////////////////////
	if (conversionFlags & CONVERT_CHARHEIGHT)
	{
		this->ConvertCharHeight();
	}
	///////////////////////////////////////////////////
	this->restoreSavedScrollPosition();
	///////////////////////////////////////////////////
	if (isWndVisible)
	{
		if (conversionFlags & HIDE_WHILE_CONVERTING)
		{
			ShowWindow(this->EditWnd, SW_SHOW);
		}
	}
}

void EditControl::PerformSyntaxErrorCheck()
{
	// TODO: ...
}

void EditControl::Undo()
{
	UNDOREDOACTIONS urActions;

	auto canUndo = this->UndoStack.UndoRequest(&urActions);
	if (canUndo)
	{
		switch (urActions.action)
		{
		case UNDOACTION_BACKSPACE:
			this->undoBackspace(&urActions.range, urActions.content);
			break;
		case UNDOACTION_CONTENT_DELETED:
			this->undoDeletedContent(&urActions.range, urActions.content);
			break;
		case UNDOACTION_NEWCHAR_ADDED:
			this->undoAddedNewchar(&urActions.range);
			break;
		case UNDOACTION_TEXT_INSERTED:
			this->undoInsertedText(&urActions.range);
			break;
		case UNDOACTION_SELECTION_REPLACED:
			this->undoReplacedSelection(&urActions);
			break;
		case UNDOACTION_CONTENT_REPLACED:
			this->undoReplacedContent(&urActions);
			break;
		default:
			break;
		}
		SafeDeleteArray(&urActions.content);
	}
}

void EditControl::Redo()
{
	UNDOREDOACTIONS urActions;

	auto canRedo = this->UndoStack.RedoRequest(&urActions);
	if (canRedo)
	{
		switch (urActions.action)
		{
		case UNDOACTION_BACKSPACE:
			this->redoBackspace(&urActions.range);
			break;
		case UNDOACTION_CONTENT_DELETED:
			this->redoDeletedContent(&urActions.range);
			break;
		case UNDOACTION_NEWCHAR_ADDED:
			this->redoAddedNewchar(&urActions.range, urActions.content);
			break;
		case UNDOACTION_TEXT_INSERTED:
			this->redoInsertedText(&urActions.range, urActions.content);
			break;
		case UNDOACTION_SELECTION_REPLACED:
			this->redoReplacedSelection(&urActions);
			break;
		case UNDOACTION_CONTENT_REPLACED:
			this->redoReplacedContent(&urActions);
			break;
		default:
			break;
		}
		SafeDeleteArray(&urActions.content);
	}
}

void EditControl::SetTextContent(LPCTSTR text, BOOL convertIt, BOOL resetUndoStack, BOOL addToUndoStack)
{
	if (convertIt)
		ShowWindow(this->EditWnd, SW_HIDE);

	SETTEXTEX stx;
	stx.codepage = 1200;
	stx.flags = ST_DEFAULT | ST_NEWCHARS;

	SendMessage(
		this->EditWnd,
		EM_SETTEXTEX,
		reinterpret_cast<WPARAM>(&stx),
		reinterpret_cast<LPARAM>(text));

	if (convertIt)
	{
		DWORD convFlags = HIDE_WHILE_CONVERTING;

		if (this->editControlProperties.autocolor)
			convFlags |= CONVERT_TOMULTICOLOR;
		if (this->editControlProperties.autosyntax.IsOn)
			convFlags |= CONVERT_SYNTAX;
		if (addToUndoStack)
			convFlags |= CONVERT_ADDTO_UNDOSTACK;

		this->ConvertContent(convFlags);

		ShowWindow(this->EditWnd, SW_SHOW);

		if (this->editControlProperties.focusmark)
		{
			// make sure the focusmark is initally visible ...

			CHARRANGE cr = { 1, 1 };
			this->SetSelection(&cr, TRUE);
		}
	}

	if (resetUndoStack)
	{
		// clear all undo actions
		this->UndoStack.Clear();
	}

	// update the content buffer
	this->updateContentBuffer();
}

intX EditControl::GetTextContent(TCHAR ** text)
{
	LRESULT res = -1; // error 'text' already referenced

	if (*text == nullptr)
	{
		GETTEXTLENGTHEX gtlx;
		gtlx.codepage = 1200;
		gtlx.flags = GTL_DEFAULT;

		res =
			SendMessage(
				this->EditWnd,
				EM_GETTEXTLENGTHEX,
				reinterpret_cast<WPARAM>(&gtlx),
				0
			);

		if (res > 0) // only proceed if there is text in the richedit
		{
			*text = new TCHAR[res + 1];

			res = (*text != nullptr) ? res : -2; // error out of memory

			if (res)
			{
				GETTEXTEX gtx;
				gtx.cb = (DWORD)(sizeof(TCHAR)* (res + 1));
				gtx.codepage = 1200;
				gtx.flags = GT_DEFAULT;
				gtx.lpDefaultChar = nullptr;
				gtx.lpUsedDefChar = nullptr;

				res =
					SendMessage(
						this->EditWnd,
						EM_GETTEXTEX,
						reinterpret_cast<WPARAM>(&gtx),
						reinterpret_cast<LPARAM>(*text)
					);
			}
		}
	}
	return res;
}

int EditControl::GetTextLength()
{
	this->updateContentBuffer();
	return _lengthOfString(this->editboxContent);
}

void EditControl::InsertText(LPCTSTR text, BOOL addToUndoStack)
{
	if (text != nullptr)
	{
		DWORD flags = 0;
		auto linef = countLinefeed(text);
		if (linef < 0)// proceed by error ?
			linef = 0;
	
		int len = _lengthOfString(text);

		CHARRANGE cr;
		SendMessage(this->EditWnd, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&cr));


		// Undo-Redo-Section one //////////////////////////////////////////////////////////////////////////////////////////////////////////
		UNDOREDOACTIONS ura;
		SecureZeroMemory(&ura, sizeof(UNDOREDOACTIONS));

		if (addToUndoStack)
		{
			if (cr.cpMax != cr.cpMin)// if the selection is not empty save the undo action as replaced-content
			{
				//SendMessage(this->EditWnd, WM_CLEAR, 0, 0);
				//SendMessage(this->EditWnd, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&cr));

				ura.action = UNDOACTION_SELECTION_REPLACED;
				ura.replacedRange = cr;
				GetRichEditSelectionContent(this->EditWnd, &ura.replacedContent);
			}
		}
		// Undo-Redo-Section END //////////////////////////////////////////////////////////////////////////////////////////////////////////


		if (this->editControlProperties.autocolor)flags |= CONVERT_TOMULTICOLOR;
		//if (this->editControlProperties.markcolor)flags |= CONVERT_MARKCOLOR;
		if (this->editControlProperties.autosyntax.IsOn)flags |= CONVERT_SYNTAX;

		if (flags != 0)
			ShowWindow(this->EditWnd, SW_HIDE);

		// set the text
		SETTEXTEX stx;
		stx.codepage = 1200;
		stx.flags = ST_NEWCHARS | ST_SELECTION;
		SendMessage(this->EditWnd, EM_SETTEXTEX, reinterpret_cast<WPARAM>(&stx), reinterpret_cast<LPARAM>(text));


		// Undo-Redo-Section two //////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (addToUndoStack)
		{
			if (ura.action != UNDOACTION_SELECTION_REPLACED)
			{
				// add the action to the undo stack
				CHARRANGE scope;
				scope.cpMin = cr.cpMin;
				scope.cpMax = (cr.cpMin + len) - linef;

				ura.action = UNDOACTION_TEXT_INSERTED;
				ura.range = scope;

				CopyStringToPtr(text, &ura.content);

				this->UndoStack.addNewUndoAction(&ura);

				SafeDeleteArray(&ura.content);
			}
			else
			{
				CHARRANGE scope;
				scope.cpMin = cr.cpMin;
				scope.cpMax = (scope.cpMin + len) - linef;

				ura.range = scope;

				CopyStringToPtr(text, &ura.content);

				this->UndoStack.addNewUndoAction(&ura);

				SafeDeleteArray(&ura.content);
			}
			SafeDeleteArray(&ura.replacedContent);
		}
		// Undo-Redo-Section END //////////////////////////////////////////////////////////////////////////////////////////////////////////


		// convert the text
		this->ConvertContent(flags);

		if (flags != 0)
			ShowWindow(this->EditWnd, SW_SHOW);

		// restore selection to the end of the selected text
		cr.cpMax = (cr.cpMin + len) - linef;
		cr.cpMin = cr.cpMax;
		SendMessage(this->EditWnd, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&cr));

		//update the content buffer
		this->updateContentBuffer();

		// mark the content as changed
		EditChangeCTRL();

		SetFocus(this->EditWnd);
	}
}

void EditControl::DeleteAllandReset(LPCTSTR defaultText)
{
	if (defaultText != nullptr)
		this->SetTextContent(defaultText, TRUE, FALSE, FALSE);
	else
		this->SetTextContent(L" ", TRUE, FALSE, FALSE);

	this->UndoStack.Clear();
}

void EditControl::SetSelection(CHARRANGE * cr, BOOL activateBlocker)
{
	if (activateBlocker)
		this->ConversionNotificationBlocker = TRUE;

	if ((cr->cpMax != cr->cpMin) && this->editControlProperties.focusmark)
		this->eraseFocusRect();

	SendMessage(this->EditWnd, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(cr));

	if (activateBlocker)
		this->ConversionNotificationBlocker = FALSE;
}

void EditControl::SetCaret(CaretIndex index)
{
	CHARRANGE cicr = { 0,0 };

	if (index == CARET_TO_FIRST_POSITION)
	{
		cicr.cpMax = 0;
		cicr.cpMin = 0;
	}
	else if (index == CARET_TO_END_OF_TEXT)
	{
		TCHAR* text = nullptr;

		if (GetRichEditContent(this->EditWnd, &text) == TRUE)
		{
			auto len = _lengthOfString(text);
			if (len > 0)
			{
				cicr.cpMax = len;
				cicr.cpMin = len;
			}
			SafeDeleteArray(&text);
		}
	}
	else
	{
		cicr.cpMin = index;
		cicr.cpMax = index;
	}
	this->SetSelection(&cicr, TRUE);
}

void EditControl::FormatLinefeeds()
{
	// old bullshit
	//int i = 0, result;
	//BOOL ctrl = FALSE;
	//TCHAR buffer[24000] = { 0 };

	//result = MessageBox(edit,
	//	L"Zeilenumbruchformatierung durchführen?",
	//	L"Text-Formatierung",
	//	MB_OKCANCEL | MB_ICONASTERISK);

	//if (result == IDCANCEL)
	//{
	//	return;
	//}

	//SendMessage(edit,
	//	WM_GETTEXT,
	//	static_cast<WPARAM>(24000),
	//	reinterpret_cast<LPARAM>(buffer));


	//while (buffer[i] != '\0')
	//{
	//	if ((buffer[i] == 'G') &&
	//		(buffer[i + 1] == '6') &&
	//		(buffer[i + 2] == '1'))
	//	{
	//		ctrl = TRUE;
	//	}
	//	else if (buffer[i] == 'N')
	//	{
	//		if (!ctrl)
	//		{
	//			buffer[i - 1] = 0x0D;
	//		}
	//		ctrl = FALSE;
	//	}
	//	else if (buffer[i] == '{')
	//	{
	//		while (buffer[i] != '}')
	//		{
	//			i++;
	//		}
	//	}
	//	else if (buffer[i] == '(')
	//	{
	//		while (buffer[i] != ')')
	//		{
	//			i++;
	//		}
	//	}
	//	i++;
	//}
	//SendMessage(edit,
	//	WM_SETTEXT,
	//	static_cast<WPARAM>(0),
	//	reinterpret_cast<LPARAM>(buffer));
}

void EditControl::FormatAll()
{
	int i = 0, pos = 0;
	TCHAR* buffer = nullptr;
	CHARRANGE cr;

	SendMessage(this->EditWnd, EM_SETEVENTMASK, 0, static_cast<LPARAM>(ENM_NONE));

	// make sure the whole content has the same format
	this->_restoreContent(false);

	if (GetRichEditContent(this->EditWnd, &buffer) == TRUE)
	{
		if (buffer != nullptr)
		{
			auto max_buffer = _lengthOfString(buffer);
			if (max_buffer > 1)
			{
				ColorConversion cList;
				itemCollection<ColorConversion> converterImage;

				this->_getsel(&cr);
				pos = cr.cpMax;

				// read the buffer and record the color-structure
				while (buffer[i] != L'\0')
				{
					if (this->CheckForLetter(buffer[i]))
					{
						// linenumber conversion
						if (buffer[i] == L'N')
						{
							cList.setStart(i);
							cList.setEnd(i + 1);
							cList.setColor(this->editStyleColors.N);

							converterImage.AddItem(cList);

							while (!this->CheckForNumber(buffer[i]))
							{
								if ((buffer[i] == L'\0') || (buffer[i] == L'\n') || (buffer[i] == 0x0D))
								{
									break;
								}
								i++;
							}
							cList.setStart(i);

							while (this->CheckForNumber(buffer[i]))
							{
								if ((buffer[i] == L'\0') || (buffer[i] == L'\n') || (buffer[i] == 0x0D))
								{
									break;
								}
								i++;
							}
							cList.setEnd(i + 1);
							cList.setColor(this->editStyleColors.LineNumber);

							converterImage.AddItem(cList);
						}
						else if (buffer[i] == L'V')
						{
							// 'V' is a special case ->variable statement
							cList.setStart(i);

							// but if 'V' is not followed by a bracket, treat it as a normal word!
							if (buffer[i + 1] != L'{')
							{
								while (buffer[i] != L' ')
								{
									if ((buffer[i] == L'\0') || (buffer[i] == L'\n') || (buffer[i] == 0x0D))
									{
										break;
									}
									i++;
								}
							}
							cList.setEnd(i + 1);
							cList.setColor(this->editStyleColors.V);

							converterImage.AddItem(cList);
						}
						else
						{
							// handle normal chars
							auto col = this->GetCharColor(buffer[i]);
							bool _ex = false;

							cList.setStart(i);

							i++;

							while (buffer[i] != L' ')
							{
								if ((buffer[i] == L'\0') || (buffer[i] == L'\n') || (buffer[i] == 0x0D))
								{
									break;
								}
								if (this->CheckForLetter(buffer[i]))// there is no space, the next letter is direct on the last word
								{
									i--;
									break;
								}
								if (buffer[i] == L'(')
								{
									i--;
									break;
								}
								else if (buffer[i] == L';')
								{
									i--;
									break;
								}
								else if (buffer[i] == L'{')
								{
									// must be a variable statement
									_ex = true;
									TCHAR bracketType = buffer[i];

									// search the end of the statement
									while ((bracketType == L'{') && (buffer[i] != L'}'))
									{
										if ((buffer[i] == L'\0') || (buffer[i] == 0x0D) || (i >= max_buffer))
										{
											_ex = true;
											break;
										}
										i++;
									}
								}
								if (_ex)
									break;
								i++;
							}
							cList.setEnd(i + 1);
							cList.setColor(col);

							converterImage.AddItem(cList);
						}
					}
					else
					{
						if ((buffer[i] == L'(') || (buffer[i] == L'{'))
						{
							cList.setStart(i);

							if (buffer[i] == L'(')
							{
								while (buffer[i] != L')')
								{
									if (buffer[i] == L'\0')
									{
										break;
									}
									if (!this->editControlProperties.autosyntax.useMultilineAnnotations)
									{
										if (buffer[i] == 0x0D)
										{
											break;
										}
									}
									i++;
								}
							}
							else
							{
								while (buffer[i] != L'}')
								{
									if (buffer[i] == L'\0')
									{
										break;
									}
									if (!this->editControlProperties.autosyntax.useMultilineAnnotations)
									{
										if (buffer[i] == 0x0D)
										{
											break;
										}
									}
									i++;
								}

							}
							cList.setEnd(i + 1);
							cList.setColor(this->editStyleColors.Annotation);

							converterImage.AddItem(cList);
						}
						else if (buffer[i] == L';')
						{
							cList.setStart(i);

							while (buffer[i] != 0x0D)
							{
								if (buffer[i] == L'\0')
								{
									break;
								}
								i++;
							}
							cList.setEnd(i + 1);
							cList.setColor(this->editStyleColors.Annotation);

							converterImage.AddItem(cList);
						}
					}
					i++;
				}
				// record-loop finished... //
				i = 0;
				int array_size = converterImage.GetCount();

				// get scroll position:
				SCROLLINFO psi;
				psi.cbSize = sizeof(SCROLLINFO);
				psi.fMask = SIF_POS;
				GetScrollInfo(this->EditWnd, SB_VERT, &psi);

				// block notifications
				this->ConversionNotificationBlocker = TRUE;

				// apply the color-structure
				while (i < array_size)
				{
					auto cConv = converterImage.GetAt(i);

					cr.cpMin = cConv.getStart();
					cr.cpMax = cConv.getEnd();
					this->textFormat.crTextColor = cConv.getColor();

					this->_setsel(&cr);

					SendMessage(
						this->EditWnd,
						EM_SETCHARFORMAT,
						static_cast<WPARAM>(SCF_SELECTION),
						reinterpret_cast<LPARAM>(&this->textFormat));

					i++;
				}

				// reset the selection
				cr.cpMax = pos;
				cr.cpMin = pos;
				this->_setsel(&cr);

				this->textFormat.crTextColor = this->editStyleColors.defaultTextcolor;

				SendMessage(
					this->EditWnd,
					EM_SETCHARFORMAT,
					static_cast<WPARAM>(SCF_SELECTION),
					reinterpret_cast<LPARAM>(&this->textFormat));

				//// reset scroll position
				//SetScrollInfo(this->EditWnd, SB_VERT, &psi, TRUE);
				//SendMessage(this->EditWnd, WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, psi.nPos), 0);

				// free notificationblocker
				this->ConversionNotificationBlocker = FALSE;
			}
			SafeDeleteArray(&buffer);
		}
	}
	SendMessage(this->EditWnd, EM_SETEVENTMASK, 0, static_cast<LPARAM>(ENM_CHANGE | ENM_SELCHANGE | ENM_CLIPFORMAT | ENM_KEYEVENTS));
}

void EditControl::FormatLine(int lineNbr)
{
	// get selection
	CHARRANGE cr_current;
	this->_getsel(&cr_current);

	if (lineNbr == FROM_CARET_POSITION)
		lineNbr = (int)this->_lineFromCharIndex(cr_current.cpMax);

	TCHAR* buffer = nullptr;
	// get content
	if (GetRichEditContent(this->EditWnd, &buffer) == TRUE)
	{
		// get maximum length
		auto max_array = _lengthOfString(buffer);
		if (max_array > 0)
		{
			bool lFound = false;
			int counter = 0;
			int linef = 0;
			int absoluteStartPosition = 0;

			// step forward to the desired line
			while (counter <= max_array)
			{
				if (buffer[counter] == 0x0D)
					linef++;
				if (linef == lineNbr)
				{
					lFound = true;
					absoluteStartPosition = counter + 1; //do not overrun the N on the beginning of the line!!!
					break;
				}
				counter++;
			}
			counter++;

			if (lFound)
			{
				int saver = counter;
				int array_size = 0;

				// count the chars in the line
				while (counter <= max_array)
				{
					if (buffer[counter] == 0x0D)
						break;

					array_size++;
					counter++;
				}

				if (array_size > 0)
				{
					// allocate the line-buffer
					TCHAR* lineBuffer = new TCHAR[array_size + 2];
					if (lineBuffer != nullptr)
					{
						SecureZeroMemory(lineBuffer, sizeof(TCHAR)* (array_size + 2));

						// recover the start-position
						counter = saver;
						saver = array_size;
						array_size = 0;

						// record the line
						while (counter <= max_array)
						{
							if (array_size == saver)
								break;

							lineBuffer[array_size] = buffer[counter];

							if (buffer[counter] == 0x0D)
								break;

							array_size++;
							counter++;
						}
						lineBuffer[array_size + 1] = L'\0';// not neccessary!!!

						itemCollection<ColorConversion> cConv;

						if (this->createConverterStructFromBuffer(lineBuffer, cConv))
						{
							int i = 0;
							CHARRANGE cr;

							int colorCount = cConv.GetCount();

							// get scroll position:
							SCROLLINFO psi;
							psi.cbSize = sizeof(SCROLLINFO);
							psi.fMask = SIF_POS;
							GetScrollInfo(this->EditWnd, SB_VERT, &psi);

							// block notifications
							this->ConversionNotificationBlocker = TRUE;

							// apply the color-structure
							while (i < colorCount)
							{
								auto _Cconv = cConv.GetAt(i);

								cr.cpMin = absoluteStartPosition + _Cconv.getStart();
								cr.cpMax = absoluteStartPosition + _Cconv.getEnd();
								this->textFormat.crTextColor = _Cconv.getColor();

								this->_setsel(&cr);

								SendMessage(
									this->EditWnd,
									EM_SETCHARFORMAT,
									static_cast<WPARAM>(SCF_SELECTION),
									reinterpret_cast<LPARAM>(&this->textFormat));

								i++;
							}

							// reset the selection
							this->_setsel(&cr_current);

							this->textFormat.crTextColor = this->editStyleColors.defaultTextcolor;

							SendMessage(
								this->EditWnd,
								EM_SETCHARFORMAT,
								static_cast<WPARAM>(SCF_SELECTION),
								reinterpret_cast<LPARAM>(&this->textFormat));

							// reset scroll position
							SetScrollInfo(this->EditWnd, SB_VERT, &psi, TRUE);
							SendMessage(this->EditWnd, WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, psi.nPos), 0);

							// free notificationblocker
							this->ConversionNotificationBlocker = FALSE;							
						}
						SafeDeleteArray(&lineBuffer);
					}
				}
			}
		}
		SafeDeleteArray(&buffer);
	}
	// recover selection
	this->_setsel(&cr_current);
}

void EditControl::ResetInputBuffer()
{
	SecureZeroMemory(this->inputBuffer, sizeof(this->inputBuffer));
	this->inputCounter = 0;
}

void EditControl::AdaptExistingFormat()
{
	CHARFORMAT _cf;
	_cf.cbSize = sizeof(CHARFORMAT);

	SendMessage(
		this->EditWnd,
		EM_GETCHARFORMAT,
		static_cast<WPARAM>(SCF_SELECTION),
		reinterpret_cast<LPARAM>(&_cf));
	SendMessage(
		this->EditWnd,
		EM_SETCHARFORMAT,
		static_cast<WPARAM>(SCF_SELECTION),
		reinterpret_cast<LPARAM>(&_cf));
}

void EditControl::ConvertSyntax(BOOL addToUndoStack)
{
	TCHAR* textToConvert = nullptr;

	if (GetRichEditContent(this->EditWnd, &textToConvert) == TRUE)
	{
		TCHAR* convertedText = nullptr;

		auto bufSize = this->executeSyntaxConversion(textToConvert, nullptr);
		if (bufSize > 0)
		{
			convertedText = new TCHAR[bufSize + 1];
			if (convertedText != nullptr)
			{
				SecureZeroMemory(convertedText, sizeof(TCHAR)*(bufSize + 1));

				if (this->executeSyntaxConversion(textToConvert, convertedText))
				{
					this->SetTextContent(convertedText, FALSE, FALSE, FALSE);

					if (addToUndoStack)
					{
						CHARRANGE cr_del, cr_ins;
						cr_ins.cpMax = -1;
						cr_ins.cpMin = 0;
						cr_del.cpMax = -1;
						cr_del.cpMin = 0;

						UNDOREDOACTIONS actions;
						actions.action = UNDOACTION_CONTENT_REPLACED;
						actions.range = cr_ins;
						actions.replacedRange = cr_del;
						actions.content = convertedText;
						actions.replacedContent = textToConvert;

						this->UndoStack.addNewUndoAction(&actions);
					}
				}
				SafeDeleteArray(&convertedText);
			}
		}
		SafeDeleteArray(&textToConvert);
	}
}

void EditControl::ConvertCharHeight()
{
	CHARFORMAT cfm;
	cfm.cbSize = sizeof(CHARFORMAT);
	cfm.dwMask = CFM_SIZE;
	cfm.yHeight = this->editControlProperties.charHeight;//ConvertXPixToTwips(this->EditWnd, this->editControlProperties.charHeight);

	SendMessage(
		this->EditWnd,
		EM_SETCHARFORMAT,
		static_cast<WPARAM>(SCF_ALL),
		reinterpret_cast<LPARAM>(&cfm)
	);
}

void EditControl::ConvertBoldEffect()
{
	CHARFORMAT cfm;
	cfm.cbSize = sizeof(CHARFORMAT);
	cfm.dwMask = CFM_BOLD;
	cfm.dwEffects = this->editControlProperties.isBold ? CFE_BOLD : 0;

	SendMessage(
		this->EditWnd,
		EM_SETCHARFORMAT,
		static_cast<WPARAM>(SCF_ALL),
		reinterpret_cast<LPARAM>(&cfm)
	);
}

void EditControl::ConvertLineOffset()
{
	CHARFORMAT cfm;
	cfm.cbSize = sizeof(CHARFORMAT);
	cfm.dwMask = CFM_OFFSET;
	cfm.yOffset = this->editControlProperties.lineOffset;

	SendMessage(
		this->EditWnd,
		EM_SETCHARFORMAT,
		static_cast<WPARAM>(SCF_ALL),
		reinterpret_cast<LPARAM>(&cfm)
	);
}

//void EditControl::ConvertMarkcolor()
//{
//	TCHAR* buffer = nullptr;
//
//	if (GetRichEditContent(this->EditWnd, &buffer) == TRUE)
//	{
//		POINTL ptl;
//		
//		int startline = 0;
//		int startdraw = 0;
//		int endline = 0;
//		int enddraw = 0;
//
//		CHARSCOPE cs;
//		iString content(buffer);
//
//		if (content.Contains(L"G818", &cs, 0, true))
//		{
//			startline = SendMessage(this->EditWnd, EM_EXLINEFROMCHAR, 0, (LPARAM)cs.startChar);
//			SendMessage(this->EditWnd, EM_POSFROMCHAR, reinterpret_cast<WPARAM>(&ptl), static_cast<LPARAM>(cs.startChar));
//			startdraw = ptl.y;
//
//			if (content.Contains(L"G80", &cs, cs.endChar, true))
//			{
//				endline = SendMessage(this->EditWnd, EM_EXLINEFROMCHAR, 0, (LPARAM)cs.endChar);
//				SendMessage(this->EditWnd, EM_POSFROMCHAR, reinterpret_cast<WPARAM>(&ptl), static_cast<LPARAM>(cs.startChar));
//				enddraw = ptl.y;
//			}
//		}
//		if (endline > startline)
//		{
//			RECT rc;
//			GetClientRect(this->EditWnd, &rc);
//			//show_integer(startdraw, enddraw);
//			HDC hdc = GetDC(this->EditWnd);
//			if (hdc)
//			{
//				HBRUSH brush = CreateSolidBrush(RGB(255, 255, 0));
//				if (brush)
//				{
//					RECT rcFill = { 0, startdraw, rc.right, startdraw + 20 };
//					//FillRect(hdc, &rcFill, brush);
//					DrawFocusRect(hdc, &rcFill);
//
//					DeleteObject(brush);
//				}
//
//				ReleaseDC(this->EditWnd, hdc);
//			}			
//		}
//		delete[] buffer;
//	}
//}

void EditControl::SetColor(COLORREF colorToSet)
{
	this->textFormat.crTextColor = colorToSet;

	SendMessage(
		this->EditWnd,
		EM_SETCHARFORMAT,
		static_cast<WPARAM>(SCF_SELECTION),
		reinterpret_cast<LPARAM>(&this->textFormat)
	);	
}

void EditControl::SetWordColor(COLORREF colorToSet)
{
	this->textFormat.crTextColor = colorToSet;

	SendMessage(
		this->EditWnd,
		EM_SETCHARFORMAT,
		static_cast<WPARAM>(SCF_WORD),
		reinterpret_cast<LPARAM>(&this->textFormat)
	);
}

void EditControl::ColorUpdate()
{
	this->FormatText();
}

int EditControl::executeSyntaxConversion(_In_ LPCTSTR buffer_in, _Inout_opt_ LPTSTR buffer_out)
{
	__try
	{
		size_t len;
		auto hr = StringCbLength(buffer_in, STRSAFE_MAX_CCH, &len);
		if (SUCCEEDED(hr))
		{
			const int bufSize = (int)(len / sizeof(TCHAR));

			int pos = 0;
			int line = 1;
			int lineNumber = this->editControlProperties.autosyntax.lineNumberStartValue;
			int newBufSize = 0;
			SecureZeroMemory(&this->lnFormat, sizeof(LNFMTDATA));

			// process buffer
			while (pos < bufSize)
			{
				if ((buffer_in[pos] == L'\0') || (pos >= bufSize))
					break;

				// start processing a new line
				if (this->CheckForNumber(buffer_in[pos]) ||
					(buffer_in[pos] == L' ') ||
					(buffer_in[pos] == L'N'))
				{
					// iterate the line and discard numbers, N-letters and spaces
					// so do nothing here!!
				}
				else
				{
					// record the valid part of the line
					while (1)
					{
						if ((buffer_in[pos] == L'\0') || (pos >= bufSize))
							break;

						// record to target buffer
						if (buffer_out != nullptr)
						{
							buffer_out[newBufSize] = buffer_in[pos];
						}
						newBufSize++;

						if (buffer_in[pos] == 0x0D)
						{
							if (line >= this->editControlProperties.autosyntax.autonumStartLine)
							{
								// check next line for special linenumber actions
								lineNumber = this->checkNextLineForTrigger(pos + 1, buffer_in, lineNumber);

								if (lineNumber < 0) // position is out of scope! Stop execution of this line!
								{
									break;
								}

								// add new number and increase
								newBufSize += this->addLineNumberToBufferWithCurrentConfig(lineNumber, buffer_out, newBufSize);

								if (this->lnFormat.subLevelActive)
								{
									if (!this->editControlProperties.autosyntax.useNoDifferentLinenumbersInSubprogram)
										lineNumber += this->editControlProperties.autosyntax.numStepInSubProgramm;
									else
										lineNumber += this->editControlProperties.autosyntax.numStepInMainProgramm;
								}
								else
									lineNumber += this->editControlProperties.autosyntax.numStepInMainProgramm;
							}
							line++;
							break;
						}
						pos++;
					}
				}
				pos++;
			}
			SecureZeroMemory(&this->lnFormat, sizeof(LNFMTDATA));

			return newBufSize + 1;
		}
		return 0;
	}
	__except (
		GetExceptionCode()
		== EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH
		)
	{
		return 0;
	}
}

void EditControl::SetDefaultStyleColors()
{
	this->editStyleColors.A = RGB(0, 212, 170);
	this->editStyleColors.B = RGB(150, 190, 50);
	this->editStyleColors.C = RGB(150, 150, 150);
	this->editStyleColors.D = RGB(150, 150, 150);
	this->editStyleColors.E = RGB(255, 255, 255);
	this->editStyleColors.F = RGB(132, 210, 81);
	this->editStyleColors.G = RGB(132, 191, 232);
	this->editStyleColors.H = RGB(255, 255, 255);
	this->editStyleColors.I = RGB(100, 255, 0);
	this->editStyleColors.J = RGB(255, 255, 255);
	this->editStyleColors.K = RGB(255, 255, 255);
	this->editStyleColors.L = RGB(255, 255, 255);
	this->editStyleColors.M = RGB(255, 255, 0);
	this->editStyleColors.N = RGB(0, 255, 0);
	this->editStyleColors.O = RGB(150, 150, 150);
	this->editStyleColors.P = RGB(255, 255, 255);
	this->editStyleColors.Q = RGB(255, 255, 255);
	this->editStyleColors.R = RGB(255, 255, 255);
	this->editStyleColors.S = RGB(132, 210, 81);
	this->editStyleColors.T = RGB(230, 100, 100);
	this->editStyleColors.U = RGB(150, 150, 150);
	this->editStyleColors.V = RGB(120, 120, 120);
	this->editStyleColors.W = RGB(255, 255, 255);
	this->editStyleColors.X = RGB(245, 212, 170);
	this->editStyleColors.Y = RGB(150, 150, 150);
	this->editStyleColors.Z = RGB(204, 255, 170);
	this->editStyleColors.Annotation = RGB(0, 255, 255);
	this->editStyleColors.background = RGB(0, 0, 0);
	this->editStyleColors.defaultTextcolor = RGB(255, 255, 255);
	this->editStyleColors.LineNumber = RGB(255, 255, 255);

	// save system color values
	this->sysHighlightColors[0] = GetSysColor(COLOR_HIGHLIGHT);
	this->sysHighlightColors[1] = GetSysColor(COLOR_HIGHLIGHTTEXT);
}

void EditControl::SetDefaultEditProperties()
{
	this->editControlProperties.autocolor = TRUE;
	this->editControlProperties.autocomplete = TRUE;
	this->editControlProperties.autonum = TRUE;
	this->editControlProperties.focusmark = TRUE;
	this->editControlProperties.uppercase = TRUE;
	this->editControlProperties.charHeight = 20;

	StringCbCopy(
		this->editControlProperties.charSet,
		sizeof(this->editControlProperties.charSet),
		L"Consolas\0"
	);

	this->editControlProperties.focusmarkCor_top = 1;
	this->editControlProperties.focusmarkCor_bottom = 6;

	////////AUTOSYNTAX///////////////////////////////////////////////
	this->editControlProperties.autosyntax.AutoLevitation = FALSE;
	this->editControlProperties.autosyntax.IsOn = TRUE;
	this->editControlProperties.autosyntax.MainLevelNumLength = 3;
	this->editControlProperties.autosyntax.SubLevelNumLength = 2;
	this->editControlProperties.autosyntax.UseDifferentNumLengthInSubProg = TRUE;
	this->editControlProperties.autosyntax.autoinsertBrackets = TRUE;
	this->editControlProperties.autosyntax.noSpaceBetweenBraces = TRUE;
	this->editControlProperties.autosyntax.noSpaceInLineNumber = TRUE;
	this->editControlProperties.autosyntax.autonumStartLine = 2;
	this->editControlProperties.autosyntax.maximumLinenumber = 9999;
	this->editControlProperties.autosyntax.numStepInMainProgramm = 5;
}

COLORREF EditControl::GetCharColor(TCHAR c)
{
	switch (c)
	{
	case L'A':
		return this->editStyleColors.A;
	case L'B':
		return this->editStyleColors.B;
	case L'C':
		return this->editStyleColors.C;
	case L'D':
		return this->editStyleColors.D;
	case L'E':
		return this->editStyleColors.E;
	case L'F':
		return this->editStyleColors.F;
	case L'G':
		return this->editStyleColors.G;
	case L'H':
		return this->editStyleColors.H;
	case L'I':
		return this->editStyleColors.I;
	case L'J':
		return this->editStyleColors.J;
	case L'K':
		return this->editStyleColors.K;
	case L'L':
		return this->editStyleColors.L;
	case L'M':
		return this->editStyleColors.M;
	case L'N':
		return this->editStyleColors.N;
	case L'O':
		return this->editStyleColors.O;
	case L'P':
		return this->editStyleColors.P;
	case L'Q':
		return this->editStyleColors.Q;
	case L'R':
		return this->editStyleColors.R;
	case L'S':
		return this->editStyleColors.S;
	case L'T':
		return this->editStyleColors.T;
	case L'U':
		return this->editStyleColors.U;
	case L'V':
		return this->editStyleColors.V;
	case L'W':
		return this->editStyleColors.W;
	case L'X':
		return this->editStyleColors.X;
	case L'Y':
		return this->editStyleColors.Y;
	case L'Z':
		return this->editStyleColors.Z;
	default:
		return this->editStyleColors.defaultTextcolor;
	}
}

void EditControl::ProcessLinefeed()
{
	this->ResetInputBuffer();

	if(!this->editControlProperties.autonum)
		this->NewUndoAction(UNDOACTION_NEWCHAR_ADDED, (WPARAM)(0x0D));
	else
	{
		TCHAR* buffer = nullptr;

		if (GetRichEditContent(this->EditWnd, &buffer) == TRUE)
		{
			auto execute = this->checkCurrentLineForSpecialTrigger(buffer);

			if (execute)
			{
				if (execute == EXECUTE_WITH_RE_INITALIZATION)
				{
					this->AutoNumControl(nullptr);
				}
				else
				{
					this->AutoNumControl(buffer);
				}
			}
			SafeDeleteArray(&buffer);
		}
	}
}

void EditControl::ProcessBackspaceAfter()
{
	TCHAR* buffer = nullptr;

	if (GetRichEditContent(this->EditWnd, &buffer) == TRUE)
	{
		// check if the linenumber could be removed
		auto lineNbrWasRemoved = this->CheckLineNumberRemoval(buffer);
		if (!lineNbrWasRemoved)
		{
			// linenumber was not removed -> make sure the format is correct

			BOOL buf_Underflow = FALSE;

			if (this->inputCounter > 0)
			{
				// decrease the buffer-counter
				this->inputCounter--;
			}
			else
			{
				buf_Underflow = TRUE;
			}

			if (this->editControlProperties.autocolor)
			{
				if (this->annotationMarkWasRemoved)
				{
					// reformat the complete line
					this->FormatLine(FROM_CARET_POSITION);

					this->annotationMarkWasRemoved = FALSE;
				}
				this->CheckEnvironment(buffer);
			}
			// terminate the last array field
			this->inputBuffer[this->inputCounter] = L'\0';
		}
		SafeDeleteArray(&buffer);
	}
}

void EditControl::ProcessBackspaceBefore()
{
	CHARRANGE cr;
	this->_getsel(&cr);

	// only execute if the selection is empty
	if (cr.cpMax == cr.cpMin)
	{
		cr.cpMax--;
		cr.cpMin--;

		// retrieve the char which is about to be removed
		auto _tchar_ = this->getCharAtCaret(&cr);
		// save it
		this->backspaceChar = _tchar_;

		// add the action to the undo-stack
		NewUndoAction(UNDOACTION_BACKSPACE, (WPARAM)_tchar_);

		// when an annotation mark is about to be removed schedule a checkup
		if ((_tchar_ == L'(') || (_tchar_ == L'{') || (_tchar_ == L')') || (_tchar_ == L'}') || (_tchar_ == L';'))
		{
			this->annotationMarkWasRemoved = TRUE;
		}
	}
	else
	{
		this->onClear();

		backspaceChar = L'\0';
		this->blockLinenumberRemoval = TRUE;
	}
}

WPARAM EditControl::makeUppercase(WPARAM wParam)
{
	if (this->editControlProperties.uppercase)
	{
		if (wParam == 'a')wParam = 'A';
		else if (wParam == 'b')wParam = 'B';
		else if (wParam == 'c')wParam = 'C';
		else if (wParam == 'd')wParam = 'D';
		else if (wParam == 'e')wParam = 'E';
		else if (wParam == 'f')wParam = 'F';
		else if (wParam == 'g')wParam = 'G';
		else if (wParam == 'h')wParam = 'H';
		else if (wParam == 'i')wParam = 'I';
		else if (wParam == 'j')wParam = 'J';
		else if (wParam == 'k')wParam = 'K';
		else if (wParam == 'l')wParam = 'L';
		else if (wParam == 'm')wParam = 'M';
		else if (wParam == 'n')wParam = 'N';
		else if (wParam == 'o')wParam = 'O';
		else if (wParam == 'p')wParam = 'P';
		else if (wParam == 'q')wParam = 'Q';
		else if (wParam == 'r')wParam = 'R';
		else if (wParam == 's')wParam = 'S';
		else if (wParam == 't')wParam = 'T';
		else if (wParam == 'u')wParam = 'U';
		else if (wParam == 'v')wParam = 'V';
		else if (wParam == 'w')wParam = 'W';
		else if (wParam == 'x')wParam = 'X';
		else if (wParam == 'y')wParam = 'Y';
		else if (wParam == 'z')wParam = 'Z';
		else if (wParam == static_cast<WPARAM>(0x00DF))wParam = '?';
	}
	return wParam;
}

BOOL EditControl::ControlInputBuffer(WPARAM wParam)
{
	// if this function returns FALSE the default processing will be executed
	// -> otherwise (TRUE) the default behavior will be blocked

	// first save current input
	this->inputBuffer[this->inputCounter] = static_cast<TCHAR>(wParam);

	// check input counter
	if (this->inputCounter >= 255)
	{
		// maximum permitted linelength is reached -> reset
		this->ResetInputBuffer();
	}

	// update the content buffer before calling methods who depends on the buffer
	this->updateContentBuffer();

	// check if the caret is inside of an annotation
	if (this->editControlProperties.autocolor)
	{
		auto aResult = this->AnnotationControl(wParam, this->editboxContent);		// buffer consumer

		if (aResult)						
		{
			if (aResult == TRUE)
			{
				// annotation is active -> stop further processing
				// and keep the default processing
				return FALSE;
			}
			else if (aResult == ANNOTATION_VARIABLE_STATEMENT)
			{
				// variable statement is active, the appropriate color is already applied by the method 'AnnotationControl(..)'
				// stop further processing of this method an keep the default processing of the calling method
				return FALSE;
			}
			else if (aResult == ANNOTATION_AUTOINSERT_EXECUTED)
			{
				// annotation mark were inserted by the annotation control
				// -> stop further processing and block the default behavior
				return TRUE;
			}
			else if (aResult == ANNOTATION_CLOSEMARK_DETECTED)
			{
				CHARRANGE chr;
				this->_getsel(&chr);
				// a closing bracket was detected, keep the default behavior, but check the environment async
				this->CheckDelayAsync(WM_UPDATELINE, 150, chr.cpMax);
			}
		}
	}

	// this is supposed to be a word-end //WORD-END >> RESET >> BUFFER + COUNTER + COLOR
	if (this->inputBuffer[this->inputCounter] == ' ')
	{
		if (this->editControlProperties.autocolor)
		{
			// check if the caret is inside the linenumber-scope
			if (this->DetectInsideN(this->editboxContent))								// buffer consumer
			{
				this->SetColor(this->editStyleColors.LineNumber);
			}

			// schedule the environment checkup
			this->CheckDelayAsync(WM_CHECKENVIRONMENT, 10);
		}

		// reset the input-buffer and keep the default processing
		this->ResetInputBuffer();
		return FALSE;
	}
	else
	{
		// this is the normal color checkup-chain
		// -> always executed -> unless it is a word end (or the color is switched off)
		if (this->editControlProperties.autocolor)
		{
			if (this->CheckForLetter((TCHAR)wParam))
			{
				BOOL chkENV = FALSE;
				
				if ((TCHAR)wParam == L'N')
				{
					chkENV = TRUE;
					this->N_open = TRUE;
				}
				else
					this->N_open = FALSE;

				COLORREF col = this->GetCharColor((TCHAR)wParam);

				if (chkENV)
					CheckEnvironment(this->editboxContent);							// buffer consumer
				else
				{
					if((TCHAR)wParam != L'V')
						this->CheckPosition(col, this->editboxContent);					// buffer consumer
				}

				this->SetColor(col);
			}
			else
			{
				if (this->DetectInsideN(this->editboxContent))						// buffer consumer
				{
					if (!this->CheckForNumber((TCHAR)wParam))
					{
						this->SetColor(this->editStyleColors.defaultTextcolor);
					}
					else
					{
						this->SetColor(this->editStyleColors.LineNumber);
					}
				}
				else
				{
					this->WordcolorControl(this->editboxContent);					// buffer consumer

					// note: WordcolorControl also checks the linenumber-scope, which is useless here (will never be executed)!
				}
			}
		}
	}
	// process autocomplete
	if (this->editControlProperties.autocomplete)
	{
		if (this->AutocompleteControl())
		{
			// an autocomplete-string was inserted so
			// reset the input-buffer and prevent the default processing
			this->ResetInputBuffer();
			return TRUE;
		}
	}
	// increase input counter
	this->inputCounter++;
	// keep default processing
	return FALSE;
}

BOOL EditControl::AnnotationControl(WPARAM wParam, TCHAR* buffer)
{
	// if this function returns TRUE the calling function 'ControlInputBuffer' will be exited immediately (and keeps the default processing)
	bool bufAlloc = false;
	BOOL result = FALSE;

	if (buffer == nullptr)
	{
		bufAlloc =
			(GetRichEditContent(this->EditWnd, &buffer) == TRUE)
			? true : false;
	}

	if (buffer != nullptr)
	{
		CHARRANGE cr;
		this->_getsel(&cr);

		auto max_buffer = _lengthOfString(buffer);

		if (((TCHAR)wParam == L'(') || ((TCHAR)wParam == L'{') || ((TCHAR)wParam == L';'))
		{
			// an annotation entry symbol was inserted
			result = TRUE;

			//-> set annotation color
			this->SetColor(this->editStyleColors.Annotation);

			// only execute when selection is empty
			if (cr.cpMax == cr.cpMin)
			{
				bool annotationFound;

				// search for an existing closing mark
				auto annotationEnd =
					this->getAnnotationEnd(cr.cpMax, (TCHAR)wParam, buffer, max_buffer, false, &annotationFound);

				if (annotationEnd >= 0)
				{
					if (!annotationFound)
					{
						// no closing mark found -> insert it when requested

						// check autoinsertbrackets
						if (this->checkAutoInsertBrackets(wParam))
						{
							// block the default processing
							// + block the execution of the calling method
							result = ANNOTATION_AUTOINSERT_EXECUTED;
						}
					}
					else
					{
						// closing mark found -> apply annotation color
						CHARRANGE annoT;
						annoT.cpMax = annotationEnd + 1;
						annoT.cpMin = cr.cpMax;
						_BELOW_ZERO_SETTOZERO(annoT.cpMin);

						this->applyColorToSelection(&annoT, &cr, this->editStyleColors.Annotation, TRUE, FALSE);

						this->SetColor(this->editStyleColors.Annotation);
					}
				}
			}
		}
		else if ((TCHAR)wParam == L')' || (TCHAR)wParam == L'}')
		{
			result = ANNOTATION_CLOSEMARK_DETECTED;
		}

		if (!result)
		{
			if (max_buffer == 0)
			{
				result = FALSE;
			}
			else
			{
				int i = 0;

				if (cr.cpMax != cr.cpMin)
				{
					// if selection is not empty ->return and continue execution of calling method
					result = FALSE;
				}
				else
				{
					i = cr.cpMax - 1;
					bool carry_detected = false;

					// step down and search for annotation marks
					while (i >= 0)
					{
						if (!this->editControlProperties.autosyntax.useMultilineAnnotations)
						{
							if (buffer[i] == 0x0D)// disabled multiline annotations and line-end is reached
							{
								result = FALSE;// enable further processing of the calling method
								break;// exit loop
							}
						}
						else
						{
							if (buffer[i] == 0x0D)
							{
								carry_detected = true;
							}
						}

						// this is an annotation-mark class 2
						if (buffer[i] == L')')
						{
							// look for an class 1 mark
							while (i >= 0)
							{
								if (buffer[i] == L'{')
								{
									COLORREF tCol;
									// check variable statement
									bool wasVariableStatement = this->isVariableStatement(i, buffer, &tCol);

									if (wasVariableStatement)
									{
										this->SetColor(tCol);
										result = ANNOTATION_VARIABLE_STATEMENT;
									}
									else
									{
										// caret must be in annotation
										this->SetColor(this->editStyleColors.Annotation);
										result = TRUE;
									}
									// exit loop and block further processing of calling method
									
									break;
								}
								else if (buffer[i] == L'}')
								{
									// no annotation -> exit loop and continue further processing of calling method
									break;
								}
								i--;
							}
							break;
						}
						else if (buffer[i] == L'}')
						{
							// no annotation -> exit loop and continue further processing of calling method
							break;
						}
						else if ((buffer[i] == '(') || (buffer[i] == '{'))
						{
							COLORREF tCol;

							bool wasVariableStatement = this->isVariableStatement(i, buffer, &tCol);

							if (wasVariableStatement)
							{
								this->SetColor(tCol);
								result = ANNOTATION_VARIABLE_STATEMENT;
							}
							else
							{
								// caret must be in annotation -> exit and block further processing of calling method
								this->SetColor(this->editStyleColors.Annotation);
								result = TRUE;
							}							
							break;
						}
						else if (buffer[i] == L';')
						{
							if (!carry_detected)
							{
								this->SetColor(this->editStyleColors.Annotation);
								result = TRUE;
							}
						}

						if (result)
						{
							// exiter
							break;
						}
						i--;
					}
				}
			}
		}
		if (bufAlloc)
			SafeDeleteArray(&buffer);
	}
	return result;
}

void EditControl::WordcolorControl(TCHAR* buffer)
{
	// this function determines which color should be applied at the position of the caret
	// based on the current position inside a word
	// -> the function only sets this format (no existing text will be formatted)
	bool bufAlloc = false;
	
	if (buffer == nullptr)// if there is no buffer -> allocate it!
	{
		bufAlloc =
			(GetRichEditContent(this->EditWnd, &buffer) == TRUE)
			? true : false;
	}

	if (buffer != nullptr)
	{
		CHARRANGE cr;
		this->_getsel(&cr);

		auto max_buffer = _lengthOfString(buffer);

		if (cr.cpMax <= max_buffer)// check array bounds
		{
			if (cr.cpMax == cr.cpMin)// only execute if the selection is empty
			{
				int i = 0;
				BOOL exit = FALSE;// blocks the further processing
				BOOL L_check = FALSE;// determines if the word beginns with a letter or not
				int s_pos = cr.cpMax;

				i = s_pos - 1;

				// step back and search for the beginning of the word
				while ((buffer[i] != L' ') && (buffer[i] != 0x0D))
				{
					if (i == 0)
					{
						// lower buffer end without word-break -> exit immediately and
						// set the defaultcolor at the beginning of the buffer
						this->SetColor(this->editStyleColors.defaultTextcolor);
						exit = TRUE;
						break;
					}
					if (this->CheckForLetter(buffer[i]))
					{
						// a letter was found -> check it
						L_check = TRUE;
						break;
					}
					i--;
				}
				/*
				there are four posibilities at this position:

				 1.) i == 0 -> exit function
				 2.) the beginning of the word is a letter (L_check == TRUE)
				 3.) the word was over and the loop exited with a space
				 4.) the word was over and the loop exited with a linefeed

				*/
				if (!exit)
				{
					int j = i;

					if (!L_check)
					{
						// no letter at the beginning of the word

						// check if it is the linenumber-construct
						while (!this->CheckForLetter(buffer[j]))
						{
							if (j == 0)
							{
								break;
							}
							if (this->CheckForNumber(buffer[j]))
							{
								// it is not the linenumber-construct -> it must be a single number, so set the defaultcolor
								this->SetColor(this->editStyleColors.defaultTextcolor);
								exit = TRUE;
								break;
							}
							j--;
						}
						if (!exit)
						{
							if ((buffer[j] == 'N'))
							{
								// the caret must be in the linenumber scope
								this->SetColor(this->editStyleColors.LineNumber);// old: this->editStyleColors.N
								exit = TRUE;
							}
						}
					}
					if (!exit)
					{
						// -> retrieve the color and set it!
						if (this->CheckForLetter(buffer[i]))
						{
							// it is a letter
							auto col = this->GetCharColor(buffer[i]);
							this->SetColor(col);
						}
						else if (this->CheckForNumber(buffer[i]))
						{
							// it is a number
							this->SetColor(this->editStyleColors.defaultTextcolor);
						}
						else
						{
							// reset ??
						}
					}
				}
			}
		}
		if(bufAlloc)
			delete[] buffer;
	}
}

BOOL EditControl::AutocompleteControl()
{
	if (this->pAcStrings != nullptr)
	{
		for (int i = 0; i < this->numOfAutocompleteStrings; i++)
		{
			for (int j = 0; j < this->pAcStrings[i].length; j++)
			{
				if (j == 56)return FALSE;
				else
				{
					if (this->pAcStrings[i].trigger[j]
						== this->inputBuffer[j])
					{
						if ((j + 1) == this->pAcStrings[i].length)
						{
							TCHAR buf[57] = { L'\0' };
							buf[0] = this->pAcStrings[i].trigger[j];
							int cSet = 1, cGet = 0;

							while (this->pAcStrings[i].appendix[cGet] != L'\0')
							{
								if ((cSet > 56) || (cGet > 55))
									return FALSE;
								buf[cSet] = this->pAcStrings[i].appendix[cGet];
								cSet++;
								cGet++;
							}
							this->InsertText(buf, TRUE);
							return TRUE;
						}
					}
					else
						break;
				}
			}
		}
	}
	return FALSE;
}

void EditControl::AutoNumControl(TCHAR* buffer)
{
	// if the current line is lower than the startline -> exit
	if (this->GetLineFromCurrentPosition() < this->editControlProperties.autosyntax.autonumStartLine)
		return;

	bool bufAlloc = false;

	if (buffer == nullptr)//if there is no buffer -> allocate it!
	{
		bufAlloc =
			(GetRichEditContent(this->EditWnd, &buffer) == TRUE)
			? true : false;
	}

	if (buffer != nullptr)
	{
		TCHAR temp[20] = { 0 };
		int auto_num = 0;
		CHARRANGE cr;

		// set the color of the linenumber
		SETTEXTEX ste;
		ste.codepage = 1200;
		ste.flags = ST_SELECTION | ST_NEWCHARS; // remove new chars?????

		if (this->editControlProperties.autocolor)
		{
			this->textFormat.crTextColor = this->editStyleColors.LineNumber;
		}
		else
		{
			this->textFormat.crTextColor = this->editStyleColors.defaultTextcolor;
		}
		uintX res = SendMessage(
			this->EditWnd,
			EM_SETCHARFORMAT,
			static_cast<WPARAM>(SCF_SELECTION),
			reinterpret_cast<LPARAM>(&this->textFormat));
		_NOT_USED(res);

		int i = 0;

		this->_getsel(&cr);

		// if the letter on the back of the caret is an N, do not insert an additional Linenumber
		if (buffer[cr.cpMax] != L'N')
		{
			i = cr.cpMax - 2;
			int pos1 = cr.cpMax;

			// step back to the last line-number
			while (1)
			{
				bool _ex = false;

				while (buffer[i] != L'N')
				{
					if (i == 0)
					{
						auto_num = 0;
						_ex = true;
						break;
					}
					else
					{
						// annotation is opened -> ignore content
						if ((buffer[i] == L'}') || (buffer[i] == L')'))
						{
							TCHAR bracketType = buffer[i];

							while (
								((bracketType == L'}') && (buffer[i] != L'{'))
								|| ((bracketType == L')') && (buffer[i] != L'('))
								)
							{
								if (i == 0)
								{
									_ex = true;
									break;
								}
								if (buffer[i] == 0x0D)// exit on carriage return
								{
									_ex = true;
									break;
								}
								i--;
							}
						}
						else if (buffer[i] == 0x0D)
						{
							_ex = true;
							break;
						}
					}
					if (_ex)
						break;

					i--;
				}
				if (_ex)
					break;
				else
				{
					if (buffer[i] == L'N')
					{
						// make sure this is the first linenumber in this line
						int d = i - 1;
						bool restart = false;

						while (d >= 0)
						{
							if (buffer[d] == L'N')
							{
								restart = true;
								break;
							}
							else if (buffer[d] == 0x0D)
							{
								_ex = true;
								break;
							}
							d--;
						}
						if (restart)
						{
							i--;
							continue;
						}
						else
							break;
					}
					else
						break;
				}
			}

			// record the number and translate to integer
			if (buffer[i] == L'N')
			{
				while (!CheckForNumber(buffer[i]))
				{
					if (buffer[i] == L'\0')
					{
						break;
					}
					i++;
				}
				int j = 0;

				while (CheckForNumber(buffer[i]))
				{
					temp[j] = buffer[i];
					i++;
					j++;
				}
				// translate and add the jump-distance
				swscanf_s(temp, L"%i", &j);							// use _wtoi instead ???
				auto_num = j + this->editControlProperties.autosyntax.numStepInMainProgramm;	// JUMP DISTANCE
			}
			// validate line-number
			if (auto_num == 0)
			{
				auto_num = this->editControlProperties.autosyntax.numStepInMainProgramm;
			}
			else if (auto_num > this->editControlProperties.autosyntax.maximumLinenumber)
			{
				auto_num = this->editControlProperties.autosyntax.maximumLinenumber;
			}
			SecureZeroMemory(&temp, sizeof(temp));

			// control indention with spaces between N and number
			if (this->editControlProperties.autosyntax.noSpaceInLineNumber)
			{
				StringCbPrintf(temp, sizeof(temp), L"N%i ", auto_num);
			}
			else
			{
				if ((auto_num > 9) && (auto_num < 100))
				{
					StringCbPrintf(temp, sizeof(temp), L"N   %i ", auto_num);
				}
				else if ((auto_num > 99) && (auto_num < 1000))
				{
					StringCbPrintf(temp, sizeof(temp), L"N  %i ", auto_num);
				}
				else if ((auto_num > 999) && (auto_num < 10000))
				{
					StringCbPrintf(temp, sizeof(temp), L"N %i ", auto_num);
				}
				else
				{
					StringCbPrintf(temp, sizeof(temp), L"N    %i ", auto_num);
				}
			}
			int pos2 = 0;

			// insert the linenumber
			SendMessage(
				this->EditWnd,
				EM_SETTEXTEX,
				reinterpret_cast<WPARAM>(&ste),
				reinterpret_cast<LPARAM>(&temp));

			// add the new content to the undo-stack: ////////////////////////////////////////////////////////////////////////
			// --------------------------------------------
			// the linefeed who triggered this, is not recorded to the undo-stack
			// instead the range is extended on the bottom-side of the range with 1
			// -> so the undo-action will remove the linefeed with the linenumber
			// -> for the redo-action the linefeed must be added at the first position of the buffer save in the undo-stack
			auto length = _lengthOfString(temp);// get the length of the added linenumber (without the preceding /r)

			TCHAR nbuf[20] = { 0 };
			StringCbPrintf(nbuf, sizeof(nbuf), L"\r%s", temp); // add the linefeed (carriage return - i know!)

			CHARRANGE undoRange;
			undoRange = cr;
			undoRange.cpMax += length; // add the length of the numbuffer to the current postion

			if (undoRange.cpMin > 0)
				undoRange.cpMin -= 1;// decrease the min-range to include the linefeed

			UNDOREDOACTIONS ura;
			ura.action = UNDOACTION_TEXT_INSERTED;
			ura.range = undoRange;
			ura.content = nbuf;
			ura.replacedContent = nullptr;
			ura.replacedRange = { 0,0 };

			this->UndoStack.addNewUndoAction(&ura);// add it to the stack
			/// END undo-redo stack /////////////////////////////////////////////////////////////////////////////////////////////

			this->_getsel(&cr);

			if (cr.cpMax != cr.cpMin)
			{
				return;
			}
			else
			{
				pos2 = cr.cpMax;
			}
			// make sure the right coloring is applied
			if (this->editControlProperties.autocolor)
			{
				cr.cpMin = pos1;
				cr.cpMax = pos1 + 1;
				this->textFormat.crTextColor = this->editStyleColors.N;

				this->_setsel(&cr);

				SendMessage(
					this->EditWnd,
					EM_SETCHARFORMAT,
					static_cast<WPARAM>(SCF_SELECTION),
					reinterpret_cast<LPARAM>(&this->textFormat));

				cr.cpMin = pos2;
				cr.cpMax = pos2;

				this->_setsel(&cr);

				this->textFormat.crTextColor = this->editStyleColors.defaultTextcolor;

				SendMessage(
					this->EditWnd,
					EM_SETCHARFORMAT,
					static_cast<WPARAM>(SCF_SELECTION),
					reinterpret_cast<LPARAM>(&this->textFormat));
			}
		}
		else
		{
			UNDOREDOACTIONS action;
			action.action = UNDOACTION_NEWCHAR_ADDED;
			action.range = cr;
			action.content = L"\r";

			this->UndoStack.addNewUndoAction(&action);
		}

		if (bufAlloc)
			SafeDeleteArray(&buffer);
	}
}

BOOL EditControl::CheckForNumber(TCHAR LT)
{
	BOOL result = FALSE;

	if ((LT == L'.') ||
		(LT == L'0') ||
		(LT == L'1') ||
		(LT == L'2') ||
		(LT == L'3') ||
		(LT == L'4') ||
		(LT == L'5') ||
		(LT == L'6') ||
		(LT == L'7') ||
		(LT == L'8') ||
		(LT == L'9'))
	{
		result = TRUE;
	}
	return result;
}

BOOL EditControl::CheckForLetter(TCHAR LT)
{
	BOOL result = FALSE;

	if ((LT == 'A') ||
		(LT == 'B') ||
		(LT == 'C') ||
		(LT == 'D') ||
		(LT == 'E') ||
		(LT == 'F') ||
		(LT == 'G') ||
		(LT == 'H') ||
		(LT == 'I') ||
		(LT == 'J') ||
		(LT == 'K') ||
		(LT == 'L') ||
		(LT == 'M') ||
		(LT == 'N') ||
		(LT == 'O') ||
		(LT == 'P') ||
		(LT == 'Q') ||
		(LT == 'R') ||
		(LT == 'S') ||
		(LT == 'T') ||
		(LT == 'U') ||
		(LT == 'V') ||
		(LT == 'W') ||
		(LT == 'X') ||
		(LT == 'Y') ||
		(LT == 'Z'))
	{
		result = TRUE;
	}
	return result;
}

intX EditControl::GetLineFromCurrentPosition()
{
	CHARRANGE cr;

	SendMessage(
		this->EditWnd,
		EM_EXGETSEL,
		static_cast<WPARAM>(0),
		reinterpret_cast<LPARAM>(&cr));

	return SendMessage(
		this->EditWnd,
		EM_EXLINEFROMCHAR,
		static_cast<WPARAM>(0),
		static_cast<LPARAM>(cr.cpMax));
}

void EditControl::CheckPosition(COLORREF color, TCHAR* buffer)
{
	// if a new letter is entered and there is no gap behind the letter (e.g. 'G'900 - G is entered)
	// than this function colors all charaters up to the next letter or space
	bool bufAlloc = false;

	// if there is no buffer->allocate the buffer
	if (buffer == nullptr)
	{
		bufAlloc =
			(GetRichEditContent(this->EditWnd, &buffer) == TRUE)
			? true : false;
	}
	if (buffer != nullptr)
	{
		CHARRANGE cr, cr_old;
		this->_getsel(&cr);

		if (cr.cpMax == cr.cpMin)// only execute if the selection is empty
		{
			// save old selection
			cr_old.cpMax = cr.cpMax;
			cr_old.cpMin = cr.cpMin;

			int s_pos = cr.cpMax;

			// search the end of the word
			if (!((buffer[s_pos] == L' ') || (buffer[s_pos] == 0x0D) || (buffer[s_pos] == L'\0')||(buffer[s_pos] == L';')))
			{
				if (!(this->CheckForLetter(buffer[s_pos]) || (buffer[s_pos] == L'\0')))
				{
					int i = 0;

					while (!this->CheckForLetter(buffer[s_pos + i]))
					{
						if ((buffer[s_pos + i] == L' ') || (buffer[s_pos + i] == 0x0D) || (buffer[s_pos] == L'\0'))
							break;
						else
						{
							if (buffer[s_pos + i] == L'{')
							{
								bool annotationEndFound;

								int end =
									this->getAnnotationEnd(
										s_pos + i,
										buffer[s_pos + i],
										buffer,
										_lengthOfString(buffer),
										false,
										&annotationEndFound
									);

								if (end >= 0)
								{
									cr.cpMax = end + 1;// save end position
									i = -1;
									break;
								}
							}
							else if (buffer[s_pos + i] == L'(')
							{
								i--;
								break;
							}							
						}
						i++;
					}
					if(i >= 0)
						cr.cpMax = cr.cpMax + i;// save end-position

					// apply the color to the evaluated scope
					this->applyColorToSelection(&cr, &cr_old, color, TRUE, FALSE);
				}
			}
		}
		if (bufAlloc)
		{
			delete[] buffer;
		}
	}
}

void EditControl::CheckEnvironment(TCHAR* buffer)
{
	bool bufAlloc = false;

	// if there is no buffer -> allocate the buffer
	if (buffer == nullptr)
	{
		bufAlloc = 
			(GetRichEditContent(this->EditWnd, &buffer) == TRUE)
			? true : false;
	}

	if (buffer != nullptr)
	{
		CHARRANGE cr, cr_old;
		this->_getsel(&cr);

		int max_buffer = _lengthOfString(buffer);

		if (cr.cpMax == cr.cpMin)// only execute if the selection is empty
		{
			// safe old selection
			cr_old.cpMax = cr.cpMax;
			cr_old.cpMin = cr.cpMin;

			int s_pos = cr.cpMax;
			int i = s_pos - 1;

			BOOL noFurtherProcessing = FALSE;
			BOOL executeAgain = TRUE;

			// step back from current position
			while ((buffer[i] != 0x0D) && (buffer[i] != L'\0'))// terminator comparison useless !?
			{
				// lower buffer end reached -> break
				if (i == 0)
					break;

				// the caret is in an open annotation
				if ((buffer[i] == L'(') || (buffer[i] == L'{') || (buffer[i] == L';'))
				{
					// check if the current coloring is the annotation color
					auto color =
						this->getCurrentColor(SCF_SELECTION);

					if (color != this->editStyleColors.Annotation)
					{
						// color is wrong -> get scope and apply!
						CHARRANGE cr_annotation;

						if (this->getAnnotationScope(cr.cpMax, buffer, &cr_annotation, max_buffer, false))
						{
							COLORREF tCol;

							// check for variable statement
							auto _isVariableStatement = this->isVariableStatement(cr_annotation.cpMin, buffer, &tCol);
							if (_isVariableStatement)
							{
								this->applyColorToSelection(&cr_annotation, &cr, tCol, TRUE, FALSE);
							}
							else
							{
								this->applyColorToSelection(&cr_annotation, &cr, this->editStyleColors.Annotation, TRUE, FALSE);
							}
						}
					}
					this->N_inside = FALSE;
					noFurtherProcessing = TRUE;
					break;
				}

				if (executeAgain)
				{
					// if the first letter in negative direction is not an N -> the caret is not in linenumber-scope
					// as a result the execution of this part will be deactivated (executeAgain)

					if (this->CheckForLetter(buffer[i]))
					{
						// letter detected
						if (buffer[i] == L'N')
						{
							// make sure the the caret is really inside N and not at the end with a space in front of it
							if (this->isLineNumber(i, buffer, s_pos, false))
							{
								// N detected, but this could also be an annotation
								// so check it
								CHARRANGE cr_temp;

								if (!this->getAnnotationScope(i, buffer, &cr_temp, max_buffer, false))
								{
									// it is no annotation so the caret must be in linenumber-scope
									// -> stop further processing
									noFurtherProcessing = TRUE;
									this->N_inside = TRUE;
									break;
								}
							}
						}
						executeAgain = FALSE;
					}
				}
				i--;
			}
			/*
			posssibilities at this position:

			 -> the caret is in the first line (i == 0)																				=> further processing
			 -> the caret is inside or on the back of a normal word																	=> further processing
			 -> N was detected as the first letter in negative direction (noFurtherProcessing == TRUE && this->N_inside == TRUE)	=> the linenumber-part will be executed
			 -> annotation detected (noFurtherProcessing == TRUE && this->N_inside == FALSE)										=> the method jumps to the end (no processing in this function)		
			*/


			// if the action was backspace and the letter of the first word in the line was executed,
			// the remaining number must be converted to the defaulttextcolor!

			// -> the checkup-part for the linenumber-scope blocks this, because if the action was backspace, there is no letter to check for!
			// -> solve this man

			if (!noFurtherProcessing)//noelse
			{
				if (s_pos > 0)//noelse
				{
					// the char in front of the caret is  -NOT-  a linefeed or a space
					if (!((buffer[s_pos - 1] == L' ') || (buffer[s_pos - 1] == 0x0D)))
					{
						int j = s_pos - 1;
						BOOL exit = FALSE;

						// step back to get the first letter in negative direction
						while (1)
						{
							if ((buffer[j] == L' ') || (buffer[j] == 0x0D) || (j == 0))
							{
								// there's no letter in negative direction -> exit
								exit = TRUE;
								break;
							}
							if (this->CheckForLetter(buffer[j]))
							{
								break;
							}
							j--;
						}
						// when a letter was found it is located on -> buffer[ j ]

						if (!exit)
						{
							// a letter was found

							// set the defaultcolor (useless ?)
							COLORREF color = this->editStyleColors.defaultTextcolor;

							if (this->CheckForLetter(buffer[j]))
							{
								// handle special case 'V' and a following {...} statement
								if ((buffer[j] == L'V') && (buffer[j + 1] == L'{'))
								{
									color = this->editStyleColors.Annotation;
									j++;
								}
								else
								{
									// set the appropriate letter-color
									color = this->GetCharColor(buffer[j]);
								}
							}

							cr.cpMin = j;// set the lower selection point

							if ((buffer[s_pos] != L' ') && (buffer[s_pos] != 0x0D) && (buffer[s_pos] != L'\0') && (buffer[s_pos] !=  L';'))
							{
								//->the caret is inside the word
								//->try to find the end of the word
								if (!this->CheckForLetter(buffer[s_pos]))
								{
									int p = 0;

									// when a variable-statement is opened -> convert it to word-color
									if (buffer[s_pos + p] == L'{') 
									{
										if (this->CheckForLetter(buffer[s_pos - 1]))
										{
											while ((buffer[s_pos + p] != L'}') && (buffer[s_pos + p] != 0x0D))
											{
												if ((s_pos + p) >= max_buffer)
													break;
												if (buffer[s_pos + p] == L'\0')
													break;

												p++;
											}
											p++;
											cr.cpMax = s_pos + p;
										}
										else
										{
											cr.cpMax = s_pos - 1;
										}
									}
									// when an annotation is opened -> set color end, even if there is no word-break
									else if (buffer[s_pos + p] == L'(')
									{
										cr.cpMax = s_pos;
									}
									else
									{
										int k = 1;


										if ((s_pos + k) < max_buffer)		// C6385 suppressor
										{
											// step forward to the end of the word
											while (this->CheckForNumber(buffer[s_pos + k]))
											{
												k++;

												if ((s_pos + k) >= max_buffer)		// C6385 suppressor
													break;
												if (buffer[s_pos + k] == L'\0')
													break;
											}

											// when a variable-statement is opened -> convert it to word-color
											if (buffer[s_pos + k] == L'{')
											{
												if (this->CheckForLetter(buffer[s_pos + (k - 1)]))// if the char before the bracket is no letter, this must be a missing space -> do not proceed
												{
													while ((buffer[s_pos + k] != L'}') && (buffer[s_pos + k] != 0x0D))
													{
														if ((s_pos + k) >= max_buffer)
															break;
														if (buffer[s_pos + k] == L'\0')
															break;

														k++;
													}
												}
											}
										}
										cr.cpMax = s_pos + k;
									}
								}
								else
								{
									// the caret is in front of a letter -> must be next word
									cr.cpMax = s_pos - 1;									
								}
							}
							else
							{
								// the caret must be at the end of the word
								cr.cpMax = s_pos;
							}
							this->applyColorToSelection(&cr, &cr_old, color, TRUE, FALSE);
						}
						else
						{
							// no letter was found in negative direction
							int k = s_pos;							

							if (k < max_buffer)
							{
								if (this->CheckForNumber(buffer[k]))
								{
									// the caret is on the beginning of a plain number (no preceding letter)

									COLORREF color = this->editStyleColors.defaultTextcolor;
									cr.cpMin = k;// set lower selection point

									// step forward to find the end of the number
									while (buffer[k] != L'\0')
									{
										if ((buffer[k] == L' ') || (buffer[k] == 0x0D))
										{
											break;
										}
										if (this->CheckForLetter(buffer[k]))
										{
											break;
										}
										k++;

										if (k >= max_buffer)		// C6385 suppressor
											break;
									}
									cr.cpMax = k;// set higher selection point

									// set the color
									this->applyColorToSelection(&cr, &cr_old, color, TRUE, FALSE);
									// this is the last action in this method (inside this iteration!) (if N_inside is FALSE - what should be ->otherwise error?)
									// -> jump to end
								}
							}
						}
					}
					else// the char in front of the caret is a linefeed or a space
					{
						// handle a space - this could be for example, when a user separates two strings -> the second part must be converted
						if (buffer[s_pos - 1] == L' ')
						{
							// check next word in positive direction
							BOOL stop_exec = FALSE;
							int l = s_pos;
							cr.cpMin = s_pos;

							COLORREF color = this->editStyleColors.defaultTextcolor;

							// the caret is already at the beginning of the next word
							if (this->CheckForLetter(buffer[s_pos]))
							{
								// the caret is in front of an N -> stop execution (must be a jump function (G61) ?? )
								if (buffer[s_pos] == L'N')
									stop_exec = TRUE;
								else if (buffer[s_pos] == L'V')
								{
									stop_exec = TRUE;

									// maybe convert the letter???

									this->FormatLine(
										(int)this->_lineFromCharIndex(s_pos)
									);
									this->UpdateFocusRect();
								}

								color = this->GetCharColor(buffer[s_pos]);
							}
							else
							{
								// it is the beginning of an annotation -> convert it!
								if ((buffer[s_pos] == L'(') || (buffer[s_pos] == L'{'))
								{
									int f = 0;

									while (
										((buffer[s_pos] == L'(') && (buffer[s_pos + f] != L')')) ||
										((buffer[s_pos] == L'{') && (buffer[s_pos + f] != L'}'))
										)
									{
										if ((s_pos + f) >= max_buffer)
											break;
										if (buffer[s_pos + f] == L'\0')
											break;
										if (buffer[s_pos + f] == 0x0D)
											break;

										f++;
									}
									f++;
									cr.cpMax = s_pos + f;
									stop_exec = TRUE;
									this->applyColorToSelection(&cr, &cr_old, this->editStyleColors.Annotation, TRUE, FALSE);
								}
								else if (buffer[s_pos] == L';')
								{
									auto f = s_pos;

									while ((buffer[f] != 0x0D) && (buffer[f] != 0x0A) && (buffer[f] != L'\0'))
									{
										if (f >= max_buffer)
											break;

										f++;
									}
									f++;
									cr.cpMax = f;
									stop_exec = TRUE;
									this->applyColorToSelection(&cr, &cr_old, this->editStyleColors.Annotation, TRUE, FALSE);
								}
								else
								{
									//search for the beginning of the next word...?
								}
							}

							if (!stop_exec)
							{
								// search for the end of the word
								while (buffer[l] != L'\0')
								{
									if ((buffer[l] == L' ') || (buffer[l] == 0x0D))
									{
										break;
									}
									l++;
								}
								cr.cpMax = l;

								// change the system selection color ??? >>
								//this->eraseSystemHighlightColor();

								if (cr.cpMax != cr.cpMin)// only set the color if the selection is not empty (there is no word!)
								{
									this->_setsel(&cr);
									this->SetColor(color);
									this->_setsel(&cr_old);
								}

								// check last word:
								l = s_pos - 1;

								// step back to find the end of the word
								while ((buffer[l] == L' ') || (buffer[l] == 0x0D))
								{
									if (l == 0)break;
									l--;
								}
								cr.cpMax = l;// save end

								// step back further to find the beginning of the word
								while ((buffer[l] != L' ') && (buffer[l] != 0x0D))
								{
									if (l == 0)break;
									l--;
								}
								l++;
								cr.cpMin = l;// save beginning

								bool apply = true;

								// get the right color
								if (this->CheckForLetter(buffer[l]))
								{
									color = this->GetCharColor(buffer[l]);
								}
								else
								{
									apply = false;
								}

								// and apply the color
								if(apply)
									this->applyColorToSelection(&cr, &cr_old, color, TRUE, FALSE);
							}
						}
					}
				}
			}
			if (this->N_inside)// the caret must be inside the scope of a linenumber
			{
				if (s_pos < max_buffer) // C6385 suppressor
				{
					if (buffer[s_pos - 1] == L' ')		// maybe handle linefeed too??		|| (buffer[s_pos - 1] == 0x0D))
					{
						//the preciding char is a space
						// -> so this must be the beginning of N
						// -> or the beginning of the number
						// -> or the space between the number and the first word in the line
					
						// first check next word:
						int l = s_pos;

						cr.cpMin = s_pos;// set start point

						// set the linenumber color as default
						COLORREF color = this->editStyleColors.LineNumber;

						if (this->CheckForLetter(buffer[s_pos]))
						{
							// the caret position is a letter ->(in an optimal case it's 'N')
							// so update the color
							color = this->GetCharColor(buffer[s_pos]);
						}

						// step forward to the end of the word ->(in an optimal case it's the space between 'N' and the linenumber
						while (buffer[l] != L'\0')
						{
							if ((buffer[l] == L' ') || (buffer[l] == 0x0D) || this->CheckForLetter(buffer[l]))// || this->CheckForNumber(buffer[l]))// check for number ??
							{
								l++;
								break;
							}
							l++;
						}
						cr.cpMax = l;// set endpoint

						//SendMessage(this->EditWnd, EM_EXSETSEL, static_cast<WPARAM>(0), reinterpret_cast<LPARAM>(&cr));

						// if there is no space between N and the linenumber, the color for N will be applied to the whole word -> not desired!!!!!

						this->_setsel(&cr);
						this->SetColor(color);

						// check last word

						l = s_pos - 1;

						// step back to the end of the last word
						while ((buffer[l] == L' ') || (buffer[l] == 0x0D))
						{
							if (l == 0)break;
							l--;
						}
						l--;
						cr.cpMax = l; // save the end position

						// step back further to the beginning of the last word
						while ((buffer[l] != L' ') && (buffer[l] != 0x0D))
						{
							if (l == 0)break;
							l--;
						}
						l++;
						cr.cpMin = l;// save start position

						// retrieve the color
						if (this->CheckForLetter(buffer[l]))
						{
							// the caret position is a letter ->(in an optimal case it's 'N')
							// so update the color
							color = this->GetCharColor(buffer[l]);
						}
						else
						{
							color = this->editStyleColors.LineNumber;
						}
						// and apply the color
						this->applyColorToSelection(&cr, &cr_old, color, TRUE, FALSE);
					}
					else
					{
						// ?...

					}
				}
				this->N_inside = FALSE;
			}
		}
		if (bufAlloc)
		{
			delete[] buffer;
		}
	}
	return;
}

BOOL EditControl::CheckLineNumberRemoval(TCHAR* buffer)
{
	// note: the backspace key removes the char before this method could be executed!
	// this function returns TRUE if the line number was removed
	BOOL removed = FALSE;

	// the removal was a seletion so cancel execution
	if (this->blockLinenumberRemoval)
	{
		this->blockLinenumberRemoval = FALSE;
		return removed;
	}

	// do not erase the linenumber
	if (!this->editControlProperties.autosyntax.eraseLinenumberOnBackspace)
		return removed;

	if (this->editControlProperties.autonum)
	{
		// check for removeable linenumber...
		BOOL executeRemoval = TRUE;
		int posOfN = -1;
		bool bufAlloc = false;

		// if there is no buffer -> allocate the buffer
		if (buffer == nullptr)
		{
			bufAlloc =
				(GetRichEditContent(this->EditWnd, &buffer) == TRUE)
				? true : false;
		}

		if (buffer != nullptr)
		{
			BOOL insideN = FALSE;
			CHARRANGE cr_current, cr_delete;

			// get current selection
			this->_getsel(&cr_current);

			// remove the linenumber only when the selection is empty
			if (!this->suppressLineNumberRemoval)
			{
				cr_delete.cpMax = cr_current.cpMax;
				cr_delete.cpMin = cr_current.cpMin;

				int pos = cr_current.cpMax -1; //???????????

				if (pos > 2)
				{
					// cursor is in front of N
					if (buffer[pos + 1] == L'N')
					{
						insideN = TRUE;

						// save position of N
						posOfN = pos + 1;

						// deletition start point
						cr_delete.cpMin = pos + 1;

						// set new positon after removing
						cr_current.cpMax = pos + 1;
						cr_current.cpMin = pos + 1;
					}
					else
					{
						int SpacesDetected = 0;

						while (pos >= 0)
						{
							// step back to look for N
							if (this->CheckForLetter(buffer[pos]))
							{
								if (buffer[pos] == L'N')// N found
								{
									if (this->isLineNumber(pos, buffer, cr_current.cpMax, true))
									{
										insideN = TRUE;

										// save position of N
										posOfN = pos;

										while (buffer[pos] != 0x0D)
										{
											pos--;
											if (pos == 0)break;
										}

										// deletition start point
										cr_delete.cpMin = pos;

										// set new positon after removing
										cr_current.cpMax = pos;
										cr_current.cpMin = pos;
									}
									break;
								}
								else if ((buffer[pos] == L'(') || (buffer[pos] == L')') || (buffer[pos] == L'{') || (buffer[pos] == L'}'))// must be a comment in any case
								{
									insideN = FALSE;
									break;
								}
								else
								{
									insideN = FALSE;
									break;
								}
							}
							else if (buffer[pos] == L' ')
							{
								if (SpacesDetected == 1)
								{
									insideN = FALSE;
									break;
								}
								else
								{
									while (buffer[pos] == L' ')
									{
										if (pos == 1)break;

										pos--;
									}
									pos++;
									SpacesDetected++;
								}
								// set new position???
							}
							if (buffer[pos] == 0x0D)
							{
								insideN = TRUE;

								// deletition start point
								cr_delete.cpMin = pos;

								// set new positon after removing
								cr_current.cpMax = pos;
								cr_current.cpMin = pos;

								break;
							}
							pos--;
						}
					}
					if (insideN)
					{
						BOOL firstSpaceDetected = FALSE;

						// reset the position to location of N
						if (posOfN != -1)
						{
							// make sure the linenumber is the first element in this line
							int cnt = posOfN - 1;

							while (cnt > 0)
							{
								if (buffer[cnt] == 0x0D)
									break;
								if (buffer[cnt] != L' ')
								{
									executeRemoval = FALSE;
									break;
								}
								cnt--;
							}							
							pos = posOfN + 1;
						}
						else
							pos += 2;

						if (executeRemoval)
						{
							// search the end of the linenumber
							while (1)
							{
								if (buffer[pos] == 0x0D)
								{
									cr_delete.cpMax = pos;
									break;
								}
								else if (buffer[pos] == L'\0')
								{
									cr_delete.cpMax = pos;
									break;
								}
								else if (this->CheckForLetter(buffer[pos]))
								{
									cr_delete.cpMax = pos;
									break;
								}
								else if (buffer[pos] == L' ')
								{
									while (buffer[pos] == L' ')pos++;

									if (firstSpaceDetected)
									{
										if (buffer[pos + 1] == 0x0D)cr_delete.cpMax = pos + 1;
										else cr_delete.cpMax = pos;

										break;
									}
									firstSpaceDetected = TRUE;
								}
								else
								{
									if (!this->CheckForNumber(buffer[pos]))
									{
										cr_delete.cpMax = pos;
										break;
									}
								}
								pos++;
							}
							// set selection to delete
							SendMessage(this->EditWnd, EM_EXSETSEL, static_cast<WPARAM>(0), reinterpret_cast<LPARAM>(&cr_delete));
							// trigger the reassembly of the correct string
							this->lineNumberIsAboutToBeRemoved = TRUE;
							// WM_CLEAR removes the selection and adds the removed content to the undo-stack
							SendMessage(this->EditWnd, WM_CLEAR, 0, 0);
							// reset selection
							SendMessage(this->EditWnd, EM_EXSETSEL, static_cast<WPARAM>(0), reinterpret_cast<LPARAM>(&cr_current));

							this->ResetInputBuffer();

							removed = TRUE;
						}
						else
							removed = FALSE;
					}
				}
			}
			else
			{
				this->suppressLineNumberRemoval = FALSE;
			}
			if (bufAlloc)
				delete[] buffer;
		}
	}
	return removed;
}

BOOL EditControl::DetectInsideN(TCHAR* buffer)
{
	bool bufAlloc = false;
	BOOL isN = FALSE;

	// if there is no buffer -> allocate the buffer
	if (buffer == nullptr)
	{
		bufAlloc =
			(GetRichEditContent(this->EditWnd, &buffer) == TRUE)
			? true : false;
	}

	if (buffer != nullptr)
	{
		int pos = 0;
		CHARRANGE cr;
		this->_getsel(&cr);
			
		if(cr.cpMax > 0)pos = cr.cpMax - 1;

		int i = pos;

		//bool num_open = false, num_closed = false;
		bool spaceDetected = false;

		// step back and search for N, exit on carriage return
		while (buffer[i] != 0x0D)
		{		
			// a letter was detected
			if (this->CheckForLetter(buffer[i]))
			{
				if (buffer[i] == L'N')
				{
					// the letter is N -> caret must be inside linenumber-scope
					isN = TRUE;
					break;
				}
				else
				{
					// the letter is not N -> caret is not inside the linenumber
					isN = FALSE;
					break;
				}
			}
			else
			{
				if (buffer[i] == L' ')
					spaceDetected = true;
				else
				{
					if (IsNumber(buffer[i]))
					{
						if (spaceDetected)
						{
							isN = FALSE;
							break;
						}
					}
				}

				//if (IsNumber(buffer[i]))
				//{
				//	if (num_closed)
				//	{
				//		isN = FALSE;
				//		break;
				//	}
				//	num_open = true;
				//}

				//if (num_open && (buffer[i] == L' '))
				//{
				//	num_closed = true;
				//}
			}

			if (i == 0)break;
			i--;
		}
		this->N_open = isN;

		if (bufAlloc)
			delete[] buffer;
	}
	return isN;
}

void EditControl::CheckAnnotationMarkRemoval(LPCTSTR buffer)
{
	if (buffer != nullptr)
	{


	}
}

void EditControl::CheckFocusmarkVisibility()
{
	if (this->editControlProperties.focusmark)
	{
		RECT fRect, clientRect;
		this->getFocusRect(&fRect, -1);
		GetClientRect(this->EditWnd, &clientRect);

		if ((fRect.bottom >= clientRect.bottom) || (fRect.top <= clientRect.top))
		{
			this->focusWasInvisible = TRUE;
		}
		else
		{
			if (this->focusWasInvisible)
			{
				this->UpdateFocusRect();

				this->focusWasInvisible = FALSE;
			}
		}
	}
}

void EditControl::CheckDelayAsync(UINT message, DWORD milliSeconds)
{
	DWORD dwThreadId;
	HANDLE hThread;

	auto dtd = new DELAYTHREADDATA;
	if (dtd != nullptr)
	{
		dtd->ctrlID = (int)GetWindowLongPtr(this->EditWnd, GWLP_ID);
		dtd->message = message;
		dtd->milliSeconds = milliSeconds;
		dtd->wParam = 0;

		hThread =
			CreateThread(
				nullptr, 0,
				EditControl::DelayThreadProc,
				reinterpret_cast<LPVOID>(dtd),
				0, &dwThreadId
			);
		if (hThread != nullptr)
		{
			CloseHandle(hThread);
		}
	}
}

void EditControl::CheckDelayAsync(UINT message, DWORD milliSeconds, WPARAM wParam)
{
	DWORD dwThreadId;
	HANDLE hThread;

	auto dtd = new DELAYTHREADDATA;
	if (dtd != nullptr)
	{
		dtd->ctrlID = (int)GetWindowLongPtr(this->EditWnd, GWLP_ID);
		dtd->message = message;
		dtd->milliSeconds = milliSeconds;
		dtd->wParam = wParam;

		hThread =
			CreateThread(
				nullptr, 0,
				EditControl::DelayThreadProc,
				reinterpret_cast<LPVOID>(dtd),
				0, &dwThreadId
			);
		if (hThread != nullptr)
		{
			CloseHandle(hThread);
		}
	}
}

void EditControl::CheckFocusMarkVisibilityAsync()
{
	if (this->editControlProperties.focusmark)
	{
		BOOL isThreadActive = (BOOL)InterlockedCompareExchange((LONG*)&this->focusMarkVisibilityCheckupThreadActive, (LONG)2, TRUE);

		if (!isThreadActive)
		{
			DWORD threadID;
			HANDLE hThread;

			hThread = CreateThread(nullptr, 0, EditControl::focusMarkProc, reinterpret_cast<LPVOID>(this), 0, &threadID);
			if (hThread != nullptr)
			{
				CloseHandle(hThread);
			}
		}
	}
}

void EditControl::saveScrollPosition()
{
	SecureZeroMemory(&this->scrollInfo, sizeof(SCROLLINFO));
	this->scrollInfo.cbSize = sizeof(SCROLLINFO);
	this->scrollInfo.fMask = SIF_POS | SIF_PAGE | SIF_TRACKPOS | SIF_RANGE;

	GetScrollInfo(this->EditWnd, SB_VERT, &this->scrollInfo);
}

void EditControl::restoreSavedScrollPosition()
{
	if (this->scrollInfo.nPos == this->scrollInfo.nMin)
	{
		SendMessage(this->EditWnd, WM_VSCROLL, MAKEWPARAM(SB_TOP, 0), 0);
	}
	else if (this->scrollInfo.nPos == this->scrollInfo.nMax)
	{
		SendMessage(this->EditWnd, WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
	}
	else
	{
		SCROLLINFO tempInfo;
		tempInfo.cbSize = sizeof(SCROLLINFO);
		tempInfo.fMask = SIF_POS;

		int lastPos = -1;

		GetScrollInfo(this->EditWnd, SB_VERT, &tempInfo);

		if (tempInfo.nPos > this->scrollInfo.nPos)
		{
			while (1)
			{
				GetScrollInfo(this->EditWnd, SB_VERT, &tempInfo);

				if (tempInfo.nPos > this->scrollInfo.nPos)
					SendMessage(this->EditWnd, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				else
					break;

				if (lastPos == tempInfo.nPos)
					break;

				lastPos = tempInfo.nPos;
			}
		}
		else if (tempInfo.nPos < this->scrollInfo.nPos)
		{
			while (1)
			{
				GetScrollInfo(this->EditWnd, SB_VERT, &tempInfo);

				if (tempInfo.nPos < this->scrollInfo.nPos)
					SendMessage(this->EditWnd, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
				else
					break;

				if (lastPos == tempInfo.nPos)
					break;

				lastPos = tempInfo.nPos;
			}
		}
	}
}

void EditControl::UpdateFocusRect()
{
	if (this->editControlProperties.focusmark)
	{
		if (!this->ConversionNotificationBlocker)
		{
			this->eraseFocusRect();
			this->setFocusRect();
		}
	}
}

void EditControl::UpdateFocusRectAsync(DWORD delay)
{
	auto asyncOperation = new Async();
	if (asyncOperation != nullptr)
	{
		asyncOperation->setDelay(delay);

		asyncOperation->callFunction(
			&_updatefocusrect,
			reinterpret_cast<LPVOID>(this)
		);
	}
}

void EditControl::UpdateFocusRect_noFlicker()
{
	if (this->editControlProperties.focusmark)
	{
		RECT rc;
		GetClientRect(this->EditWnd, &rc);

		// ???
	}
}

void EditControl::setFocusRect()
{
	RECT fRect;
	this->getFocusRect(&fRect, -1);

	HDC hdc = GetDC(this->EditWnd);
	if (hdc)
	{
		//DrawFocusRect(hdc, &fRect);
		///////////////////////////////////

		HPEN pen = CreatePen(PS_SOLID, 1, this->getFocusMarkColor());
		if (pen)
		{
			HPEN origin_pen = (HPEN)SelectObject(hdc, pen);
			HBRUSH origin_brush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

			SetBkMode(hdc, TRANSPARENT);
			Rectangle(hdc, fRect.left, fRect.top, fRect.right, fRect.bottom);

			SelectObject(hdc, origin_pen);
			SelectObject(hdc, origin_brush);
			DeleteObject(pen);
		}
		///////////////////////////////////////
		ReleaseDC(this->EditWnd, hdc);
	}
}

void EditControl::getFocusRect(LPRECT prc, int index)
{
	RECT rc;
	POINTL ptl;
	CHARRANGE cr = { 0,0 };

	GetClientRect(this->EditWnd, &rc);

	// if index is not specified -> use selection
	if(index < 0)
		SendMessage(this->EditWnd, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&cr));

	// selection is empty -> proceed
	if (cr.cpMax == cr.cpMin)
	{
		int indexToUse;

		// define the used index
		if (index < 0)
		{
			indexToUse = cr.cpMin;
			this->oldFocusCaretPosition = indexToUse;
		}
		else
			indexToUse = index;

		// get the absolute position
		SendMessage(this->EditWnd, EM_POSFROMCHAR, reinterpret_cast<WPARAM>(&ptl), static_cast<LPARAM>(indexToUse));

		// calculate the focus mark top + bottom borders
		int ytop =
			ptl.y
			- this->getFocusMarkCorrectionValue(CORR_TYPE_TOP);


		if (ytop < 0)
			ytop = 0;

		int ybot =
			ptl.y
			+ ConvertTwipsToPix(this->editControlProperties.charHeight)
			+ this->getFocusMarkCorrectionValue(CORR_TYPE_BOTTOM);

		if (ybot > rc.bottom)
			ybot = rc.bottom;

		// set the focus rect
		prc->left = rc.left;
		prc->right = rc.right;
		prc->top = ytop;
		prc->bottom = ybot;
	}
	else
		SetRect(prc, 0, 0, 0, 0);

	// extended rect for selection !!!

	// DO NOT DELETE !!!!!!!!!!!!

	//else
	//{
	//	// EXTENDED RECT DEACTIVATED - DO NOT DELETE !!!!!!!!!!!!

	//	//if (cr.cpMax >= 0 && cr.cpMin >= 0)
	//	//{
	//	//	// draw extended focus rect

	//	//	int cor;

	//	//	if (this->editControlProperties.charHeight > 25)cor = 3;
	//	//	else cor = 2;

	//	//	SendMessage(this->EditWnd, EM_POSFROMCHAR, reinterpret_cast<WPARAM>(&ptl), static_cast<LPARAM>(cr.cpMin));

	//	//	int ytop = ptl.y - (cor + 1);
	//	//	if (ytop < 0)ytop = 0;

	//	//	SendMessage(this->EditWnd, EM_POSFROMCHAR, reinterpret_cast<WPARAM>(&ptl), static_cast<LPARAM>(cr.cpMax));

	//	//	BOOL isLinefeed = FALSE;
	//	//	TCHAR* selection = nullptr;

	//	//	if (GetRichEditSelectionContent(this->EditWnd, &selection) == TRUE)
	//	//	{
	//	//		int i = 0;
	//	//		while (selection[i] != L'\0')
	//	//		{
	//	//			i++;
	//	//		}
	//	//		if (i > 0)
	//	//		{
	//	//			if (selection[i - 1] == 0x0D)
	//	//			{
	//	//				isLinefeed = TRUE;
	//	//			}
	//	//		}
	//	//		SafeDeleteArray(&selection);
	//	//	}

	//	//	int ybot;

	//	//	if(isLinefeed)
	//	//		ybot = ptl.y + cor;
	//	//	else
	//	//		ybot = ptl.y + this->editControlProperties.charHeight + cor;

	//	//	if (ybot > rc.bottom)ybot = rc.bottom;

	//	//	SetRect(&this->currentFocusRect, 0, ytop, rc.right, ybot);

	//	//	HDC hdc = GetDC(this->EditWnd);
	//	//	if (hdc)
	//	//	{
	//	//		DrawFocusRect(hdc, &this->currentFocusRect);

	//	//		ReleaseDC(this->EditWnd, hdc);
	//	//	}
	//	//}

	//	DO NOT DELETE !!!!!!!!!!!!

	//}
}

void EditControl::eraseFocusRect()
{
	HDC hdc = GetDC(this->EditWnd);
	if (hdc)
	{
		RECT fRect;
		this->getFocusRect(&fRect, this->oldFocusCaretPosition);

		HPEN pen = CreatePen(PS_SOLID, 1, this->editStyleColors.background);
		if (pen)
		{
			HPEN origin_pen = (HPEN)SelectObject(hdc, pen);
			HBRUSH origin_brush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

			SetBkMode(hdc, TRANSPARENT);
			Rectangle(hdc, fRect.left, fRect.top, fRect.right, fRect.bottom);

			SelectObject(hdc, origin_pen);
			SelectObject(hdc, origin_brush);
			DeleteObject(pen);
		}
		ReleaseDC(this->EditWnd, hdc);

		// make sure the last rect-area is clean
		if (fRect.bottom != 0 || fRect.left != 0 || fRect.right != 0 || fRect.top != 0)
		{
			RECT updateRect =
			{
				fRect.left,
				fRect.top - ConvertTwipsToPix(this->editControlProperties.lineOffset) ,
				fRect.right,
				fRect.bottom + ConvertTwipsToPix(this->editControlProperties.lineOffset)
			};

			InvalidateRect(this->EditWnd, &updateRect, TRUE);
			UpdateWindow(this->EditWnd);
		}
	}
}

int EditControl::getFocusMarkCorrectionValue(int type)
{
	if (type == CORR_TYPE_TOP)
		return this->editControlProperties.focusmarkCor_top;
	else if (type == CORR_TYPE_BOTTOM)
		return this->editControlProperties.focusmarkCor_bottom;
	else
		return 0;
}

TCHAR EditControl::getCharAtCaret(CHARRANGE* pcr)
{
	CHARRANGE cr;
	if (pcr == nullptr)
		GETCURRENTCHARRANGE(this->EditWnd, &cr);
	else
		cr = *pcr;

	if (cr.cpMin == cr.cpMax)
	{
		TCHAR* buffer = nullptr;
		if (GetRichEditContent(this->EditWnd, &buffer) == TRUE)
		{
			TCHAR Char_ = buffer[cr.cpMin];

			SafeDeleteArray(&buffer);

			return Char_;
		}
	}
	return TCHAR(0);
}

void EditControl::eraseSystemHighlightColor()
{
	if (!this->selectionColorErased)
	{
		int colElements[2] = { COLOR_HIGHLIGHT, COLOR_HIGHLIGHTTEXT };

		this->sysHighlightColors[0] = GetSysColor(colElements[0]);
		this->sysHighlightColors[1] = GetSysColor(colElements[1]);

		COLORREF newColors[2] = { this->editStyleColors.background, this->editStyleColors.defaultTextcolor };

		SetSysColors(2, colElements, newColors);

		this->selectionColorErased = TRUE;
	}
}

void EditControl::restoreSystemHighlightColor()
{
	if (this->selectionColorErased)
	{
		int colElements[2] = { COLOR_HIGHLIGHT, COLOR_HIGHLIGHTTEXT };

		SetSysColors(2, colElements, this->sysHighlightColors);

		//this->UpdateFocusRect();

		this->selectionColorErased = FALSE;
	}
}

void EditControl::undoBackspace(CHARRANGE * cr, LPCTSTR content)
{
	if (content != nullptr)
	{
		if (cr->cpMax != cr->cpMin)
			cr->cpMax = cr->cpMin;

		SendMessage(this->EditWnd, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(cr));
		SetRichEditContent(this->EditWnd, content, ST_NEWCHARS | ST_SELECTION);

		// check color !!!
		this->CheckDelayAsync(WM_CHECKENVIRONMENT, 5);
	}
}

void EditControl::undoDeletedContent(CHARRANGE * cr, LPCTSTR content)
{
	if (content != nullptr)
	{
		int hld = cr->cpMax;

		cr->cpMax = cr->cpMin;
		this->SetSelection(cr, TRUE);

		this->InsertText(content, FALSE);

		cr->cpMax = hld;
		this->SetSelection(cr, TRUE);
	}
}

void EditControl::undoAddedNewchar(CHARRANGE * cr)
{
	cr->cpMin -= 1;
	if (cr->cpMin < 0)
		return;
	this->SetSelection(cr, TRUE);
	this->skipOnClearExecution = TRUE;
	SendMessage(this->EditWnd, WM_CLEAR, 0, 0);
}

void EditControl::undoInsertedText(CHARRANGE * cr)
{
	this->SetSelection(cr, TRUE);
	this->skipOnClearExecution = TRUE;
	SendMessage(this->EditWnd, WM_CLEAR, 0, 0);	
}

void EditControl::undoReplacedSelection(LPUNDOREDOACTIONS action)
{
	this->SetSelection(&action->range, TRUE);
	this->InsertText(action->replacedContent, FALSE);
	this->SetSelection(&action->replacedRange, FALSE);
}

void EditControl::undoReplacedContent(LPUNDOREDOACTIONS action)
{
	//this->SetSelection(&action->range, TRUE);
	//this->InsertText(action->replacedContent, FALSE);
	this->SetTextContent(action->replacedContent, TRUE, FALSE, FALSE);
}

void EditControl::redoBackspace(CHARRANGE * cr)
{
	cr->cpMax += 1;
	this->SetSelection(cr, TRUE);
	this->skipOnClearExecution = TRUE;
	SendMessage(this->EditWnd, WM_CLEAR, 0, 0);
}

void EditControl::redoDeletedContent(CHARRANGE * cr)
{
	this->SetSelection(cr, TRUE);
	this->skipOnClearExecution = TRUE;
	SendMessage(this->EditWnd, WM_CLEAR, 0, 0);
}

void EditControl::redoAddedNewchar(CHARRANGE * cr, LPCTSTR content)
{
	if (content != nullptr)
	{
		cr->cpMin--;
		cr->cpMax = cr->cpMin;

		this->SetSelection(cr, TRUE);
		this->InsertText(content, FALSE);

		this->UpdateFocusRect();
	}
}

void EditControl::redoInsertedText(CHARRANGE * cr, LPCTSTR content)
{
	if (content != nullptr)
	{
		cr->cpMax = cr->cpMin;
		this->SetSelection(cr, TRUE);
		this->InsertText(content, FALSE);

		this->UpdateFocusRect();
	}
}

void EditControl::redoReplacedSelection(LPUNDOREDOACTIONS action)
{
	this->SetSelection(&action->replacedRange, TRUE);
	this->InsertText(action->content, FALSE);
}

void EditControl::redoReplacedContent(LPUNDOREDOACTIONS action)
{
	//this->SetSelection(&action->replacedRange, TRUE);
	//this->InsertText(action->content, FALSE);
	this->SetTextContent(action->content, TRUE, FALSE, FALSE);
}

void EditControl::applyColorToSelection(CHARRANGE * colorSegment, CHARRANGE * restoreRange, COLORREF colorToSet, BOOL suppressSelNotification, BOOL overrideSystemSelectionColor)
{
	// temp
	//iString text(L"color applied\n");
	//text.Append(L"\nR-Value: ");
	//text.Append(iString::FromInt(GetRValue(colorToSet)));
	//text.Append(L"\nG-Value: ");
	//text.Append(iString::FromInt(GetGValue(colorToSet)));
	//text.Append(L"\nB-Value: ");
	//text.Append(iString::FromInt(GetBValue(colorToSet)));

	//showText(text.GetData());

	if (suppressSelNotification)
		this->suppressSelectionNotification = TRUE;

	//if (overrideSystemSelectionColor)
		//this->eraseSystemHighlightColor();

	bool updateFocusRect = false;

	CHARRANGE cr_t;
	cr_t.cpMin = colorSegment->cpMax;
	cr_t.cpMax = cr_t.cpMin;

	SendMessage(this->EditWnd, EM_EXSETSEL, static_cast<WPARAM>(0), reinterpret_cast<LPARAM>(&cr_t));

	CHARFORMAT cf_t;
	cf_t.cbSize = sizeof(CHARFORMAT);
	cf_t.dwMask = CFM_COLOR;

	SendMessage(this->EditWnd, EM_GETCHARFORMAT, (WPARAM)SCF_SELECTION, reinterpret_cast<LPARAM>(&cf_t));

	if (cf_t.crTextColor != colorToSet)
	{
		SendMessage(this->EditWnd, EM_EXSETSEL, static_cast<WPARAM>(0), reinterpret_cast<LPARAM>(colorSegment));
		this->SetColor(colorToSet);
		updateFocusRect = true;
	}
	SendMessage(this->EditWnd, EM_EXSETSEL, static_cast<WPARAM>(0), reinterpret_cast<LPARAM>(restoreRange));

	//if (overrideSystemSelectionColor)
		//this->restoreSystemHighlightColor();
	if (suppressSelNotification)
		this->suppressSelectionNotification = FALSE;

	if(updateFocusRect)
		this->UpdateFocusRect();
}

COLORREF EditControl::getFocusMarkColor()
{
	auto r = GetRValue(this->editStyleColors.background);
	auto g = GetGValue(this->editStyleColors.background);
	auto b = GetBValue(this->editStyleColors.background);

	if ((r < 100)||(r > 200))r = 150;
	else
	{
		if (r > 150)r = 100;
		else r = 200;
	}
	if ((g < 100) || (g > 200))g = 150;
	else
	{
		if (g > 150)g = 100;
		else g = 200;
	}
	if ((b < 100) || (b > 200))b = 150;
	else
	{
		if (b > 150)b = 100;
		else b = 200;
	}
	return RGB(r,g,b);
}

void EditControl::moveCursor(int offset)
{
	CHARRANGE cr;
	SendMessage(this->EditWnd, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&cr));

	if (cr.cpMin == cr.cpMax)
	{
		cr.cpMax += offset;
		cr.cpMin = cr.cpMax;

		SendMessage(this->EditWnd, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&cr));
	}
}

bool EditControl::changeLinenumberInCurrentLine(int pos_from, LPCTSTR buffer, DWORD mode, int* selectionOffset_out)
{
	if (buffer != nullptr)
	{
		CHARRANGE cr;

		auto num_prev = this->getLinenumberFromPreviousLine(pos_from, buffer, &cr);
		if (num_prev != -1)
		{		
			auto num_cur = this->getLinenumberFromCurrentLine(pos_from, buffer, &cr);
			if (num_cur != -1)
			{
				int oldNumRange = cr.cpMax - cr.cpMin;

				while(
					(buffer[cr.cpMax] == L' ') ||
					this->CheckForNumber(buffer[cr.cpMax])
					)
				{
					if (cr.cpMax >= pos_from)
						break;

					cr.cpMax++;
				}

				int nNum = 0;
				TCHAR newNumber[256] = { 0 };

				if (mode == LN_MODE_TRIGGER)
				{
					nNum = this->roundOffLineNumber(num_cur);

					if ((nNum == num_cur) || (nNum == -1))
					{
						return false;
					}
					else
					{
						if (this->addLineNumberToBufferWithCurrentConfig(nNum, newNumber, 0) != 0)
						{
							RichEdit_SetSelection(this->EditWnd, &cr);
							this->InsertText(newNumber, TRUE);

							*selectionOffset_out =
								(_lengthOfString(newNumber) - 1)	// -1 because _lengthOfString counts the including null-character
								- (oldNumRange + 1);				// +1 because the old range does not include the space at the end of the string which is included in 'newNumber'

							return true;
						}
					}
				}
				else if (mode == LN_MODE_SUB)
				{
					if ((this->editControlProperties.autosyntax.SubLevelNumLength == 1)
						&& (num_cur < 10))
						return false;
					else if ((this->editControlProperties.autosyntax.SubLevelNumLength == 2)
						&& (num_cur < 100))
						return false;
					else if ((this->editControlProperties.autosyntax.SubLevelNumLength == 3)
						&& (num_cur < 1000))
						return false;
					else
					{
						if (this->editControlProperties.autosyntax.SubLevelNumLength == 1)
							nNum = 1;
						else if (this->editControlProperties.autosyntax.SubLevelNumLength == 2)
							nNum = 10;
						else if (this->editControlProperties.autosyntax.SubLevelNumLength == 3)
							nNum = 100;
						else
							return false;

						if (nNum == num_cur)
						{
							return false;
						}
						else
						{
							if (this->addLineNumberToBufferWithCurrentConfig(nNum, newNumber, 0) != 0)
							{
								RichEdit_SetSelection(this->EditWnd, &cr);
								this->InsertText(newNumber, TRUE);

								*selectionOffset_out =
									(_lengthOfString(newNumber) - 1)	// -1 because _lengthOfString counts the including null-character
									- (oldNumRange + 1);				// +1 because the old range does not include the space at the end of the string which is included in 'newNumber'

								return true;
							}
						}
					}
				}
				else if (mode == LN_MODE_MAIN)
				{
					if (
						((this->editControlProperties.autosyntax.SubLevelNumLength == 1) && (num_cur < 10)) ||
						((this->editControlProperties.autosyntax.SubLevelNumLength == 2) && (num_cur < 100)) ||
						((this->editControlProperties.autosyntax.SubLevelNumLength == 3) && (num_cur < 1000))
						)
					{
						int lastNum = this->getLastMainLevelLinenumber(cr.cpMax - 2, buffer);
						if (lastNum != -1)
						{
							lastNum += this->editControlProperties.autosyntax.numStepInMainProgramm;

							if (this->addLineNumberToBufferWithCurrentConfig(lastNum, newNumber, 0) != 0)
							{
								RichEdit_SetSelection(this->EditWnd, &cr);
								this->InsertText(newNumber, TRUE);

								*selectionOffset_out =
									(_lengthOfString(newNumber) - 1)	// -1 because _lengthOfString counts the including null-character
									- (oldNumRange + 1);				// +1 because the old range does not include the space at the end of the string which is included in 'newNumber'

								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

int EditControl::getLinenumberFromPreviousLine(int pos, LPCTSTR buffer, CHARRANGE* pcr_out)
{
	if (buffer != nullptr)
	{
		bool isDesiredLine = false;

		// step back to the beginning of the previous line
		while (pos >= 0)
		{
			if (buffer[pos] == 0x0D)
			{
				if (isDesiredLine)
					return -1;
				else
					isDesiredLine = true;
			}
			if (isDesiredLine)
			{
				// N is reached (in the requested line)
				if (buffer[pos] == L'N')
				{
					// make sure this is the first N-word in this line
					if (this->isFirst_N_inLine(buffer, pos))
					{
						pcr_out->cpMin = pos;
						int j = pos;

						// search the number
						while (!this->CheckForNumber(buffer[j]))
						{
							if (buffer[j] == L'\0')// end of buffer
								return -1;

							j++;
						}

						TCHAR num[256] = { 0 };
						int i = 0;

						// record the number
						while (this->CheckForNumber(buffer[j]))
						{
							if (buffer[j] == L'\0')// end of buffer
								break;
							if (i == 255)// error -> number to high
								return -1;

							num[i] = buffer[j];
							i++;
							j++;
						}
						pcr_out->cpMin = j - 1;
						num[i] = L'\0';

						//convert and return
						return _wtoi(num);
					}
				}
			}
			pos--;
		}
	}
	return -1;
}

int EditControl::getLinenumberFromCurrentLine(int pos, LPCTSTR buffer, CHARRANGE * pcr_out)
{
	if (buffer != nullptr)
	{
		// step back to the beginning of the previous line
		while (pos >= 0)
		{
			if (buffer[pos] == 0x0D)
				return -1;

			// N is reached
			if (buffer[pos] == L'N')
			{
				// make sure this is the first N-word in this line
				if (this->isFirst_N_inLine(buffer, pos))
				{
					pcr_out->cpMin = pos;
					int j = pos;

					// search the number
					while (!this->CheckForNumber(buffer[j]))
					{
						if (buffer[j] == L'\0')// end of buffer
							return -1;

						j++;
					}

					TCHAR num[256] = { 0 };
					int i = 0;

					// record the number
					while (this->CheckForNumber(buffer[j]))
					{
						if (buffer[j] == L'\0')// end of buffer
							break;
						if (i == 255)// error -> number to high
							return -1;

						num[i] = buffer[j];
						i++;
						j++;
					}
					pcr_out->cpMax = j - 1;
					num[i] = L'\0';

					//convert and return
					return _wtoi(num);
				}
			}
			pos--;
		}
	}
	return -1;
}

BOOL EditControl::checkAutoInsertBrackets(WPARAM wParam)
{
	auto chr = static_cast<TCHAR>(wParam);

	if (this->editControlProperties.autosyntax.autoinsertBrackets)
	{
		if (chr == L'{')
		{
			if (this->editControlProperties.autosyntax.noSpaceBetweenBraces)
			{
				this->InsertText(L"{}", TRUE);
				this->moveCursor(-1);
			}
			else
			{
				this->InsertText(L"{  }", TRUE);
				this->moveCursor(-2);
			}
			return TRUE;
		}
		else if (chr == L'(')
		{
			if (this->editControlProperties.autosyntax.autoinsertBrackets)
			{
				this->InsertText(L"()", TRUE);
				this->moveCursor(-1);
			}
			else
			{
				this->InsertText(L"(  )", TRUE);
				this->moveCursor(-2);
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL EditControl::checkCurrentLineForSpecialTrigger(TCHAR* buffer)
{
	bool bufAlloc = false;
	int selectionOffset = 0;
	BOOL result = TRUE;

	if (buffer == nullptr)// if there is no buffer -> allocate it!
	{
		bufAlloc =
			(GetRichEditContent(this->EditWnd, &buffer) == TRUE)
			? true : false;
	}

	if (buffer != nullptr)
	{
		// get current caret position (selection)
		CHARRANGE cr;
		RichEdit_GetSelection(this->EditWnd, &cr);

		// only proceed when the selection is empty
		if (cr.cpMax == cr.cpMin)
		{
			// get the length of the buffer
			auto max_buffer = _lengthOfString(buffer);

			// make sure there's no access violation
			if (cr.cpMax <= max_buffer)
			{
				bool wasChanged = false;
				bool subProgDetected = false;
				bool isLineEmpty = false;
				int i = cr.cpMax - 2;

				// record the current line
				// (we assume the cursor is on the end of the line because this method should only be executed when a linefeed is inserted!)
				int bufSize = 0;
				int initialPos = i;

				// count the buffersize
				while (i > 0)
				{
					if (buffer[i] == 0x0D)
						break;

					i--;
					bufSize++;
				}
				i++;

				TCHAR *linebuffer = new TCHAR[bufSize + 1];
				if (linebuffer != nullptr)
				{					
					bool _ex = false;
					int recorder = 0;
					SecureZeroMemory(linebuffer, sizeof(TCHAR)*(bufSize + 1));

					// record the buffer
					while (i <= initialPos)
					{
						// ignore annotations and variable-statements
						if ((buffer[i] == L'(') || (buffer[i] == L'{'))
						{
							TCHAR bracketType = buffer[i];

							while (
								((bracketType == L'{') && (buffer[i] != L'}'))
								|| ((bracketType == L'(') && (buffer[i] != L')'))
								)
							{
								if (i == initialPos)// exit on caret-position
								{
									_ex = true;
									break;
								}
								if (buffer[i] == 0x0D)// exit on carriage return
								{
									_ex = true;
									break;
								}
								i++;
							}
							if (_ex)
								break;
						}
						else if (buffer[i] == L';')
						{
							linebuffer[recorder] = L'\0';
							break;
						}
						//record
						linebuffer[recorder] = buffer[i];
						// increase
						recorder++;
						i++;
					}

					// !!!!!\/

					// check end-Program-Trigger !! return false if the caret is after the end trigger
					if (this->editControlProperties.autosyntax.useEndProgDetection)
					{
						//search not only the line - search all text from the caret in negative direction
						TCHAR *epdBuffer = nullptr;

						if (_cutString(buffer, 0, initialPos + 1, &epdBuffer))
						{
							if (epdBuffer != nullptr)
							{
								if (this->endProgTrigger.checkBufferForWordMatches(epdBuffer))
								{
									// the caret is not in valid programm scope -> return false to block autonum!
									result = FALSE;
								}
								SafeDeleteArray(&epdBuffer);
							}
						}
					}
					// end !!!!!\ /

					//check if the line is empty - if so skip the further processing
					isLineEmpty = this->isEmptyLine(linebuffer, bufSize);
					if (!isLineEmpty)
					{
						// check for trigger in line >>

						// check sub-Program-Trigger					
						if (this->editControlProperties.autosyntax.UseDifferentNumLengthInSubProg)
						{
							if (this->subProgTrigger.checkBufferForWordMatches(linebuffer))
							{
								subProgDetected = true;

								wasChanged =
									this->changeLinenumberInCurrentLine(initialPos, buffer, LN_MODE_SUB, &selectionOffset);
							}
						}
						//// check end-Program-Trigger !! return false if the caret is after the end trigger
						//if (this->editControlProperties.autosyntax.useEndProgDetection)
						//{
						//	//search not only the line - search all text from the caret in negative direction
						//	TCHAR *epdBuffer = nullptr;

						//	if (_cutString(buffer, 0, initialPos + 1, &epdBuffer))
						//	{
						//		if (epdBuffer != nullptr)
						//		{
						//			if (this->endProgTrigger.checkBufferForWordMatches(epdBuffer))
						//			{
						//				// the caret is not in valid programm scope -> return false to block autonum!
						//				result = FALSE;
						//			}
						//			SafeDeleteArray(&epdBuffer);
						//		}
						//	}
						//}
						// check new-even-linenumber-trigger...!
						if (this->editControlProperties.autosyntax.newEvenLineNumberOnTrigger)
						{
							if (this->newLineTrigger.checkBufferForWordMatches(linebuffer))
							{
								wasChanged =
									this->changeLinenumberInCurrentLine(initialPos, buffer, LN_MODE_TRIGGER, &selectionOffset);
							}
						}
					}
					SafeDeleteArray(&linebuffer);
				}
				if (this->editControlProperties.autosyntax.UseDifferentNumLengthInSubProg)
				{
					if (!isLineEmpty)//only proceed if the line is not empty
					{
						// look for a reset of the subProg-numbers (when no sub-trigger was detected)
						if (!subProgDetected && !wasChanged)
						{
							wasChanged =
								this->changeLinenumberInCurrentLine(cr.cpMax - 2, buffer, LN_MODE_MAIN, &selectionOffset);
						}
					}
				}

				// restore selection
				if (wasChanged)
				{
					// calculate selection offset
					cr.cpMax += selectionOffset;
					cr.cpMin += selectionOffset;

					RichEdit_SetSelection(this->EditWnd, &cr);
				}

				// if the linenumber was changed, delete the buffer to force autonum-control to reallocate it and so to rescan for the last number
				if (wasChanged && result)
				{
					result = EXECUTE_WITH_RE_INITALIZATION;
				}
			}
		}
		if(bufAlloc)
			SafeDeleteArray(&buffer);
	}
	return result;
}

COLORREF EditControl::getCurrentColor(DWORD mode)
{
	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	SendMessage(this->EditWnd, EM_GETCHARFORMAT, static_cast<WPARAM>(mode), reinterpret_cast<LPARAM>(&cf));
	return cf.crTextColor;
}

int EditControl::addLineNumberToBufferWithCurrentConfig(int num, LPTSTR buffer, int from)
{
	TCHAR temp[50] = { L'\0' };

	// validate line-number
	if (num == 0)
	{
		num = this->editControlProperties.autosyntax.numStepInMainProgramm;
	}
	else if (num > this->editControlProperties.autosyntax.maximumLinenumber)
	{
		num = this->editControlProperties.autosyntax.maximumLinenumber;
	}

	// control indention with spaces between N and number
	if (this->editControlProperties.autosyntax.noSpaceInLineNumber)
	{
		StringCbPrintf(temp, sizeof(temp), L"N%i ", num);
	}
	else
	{
		if ((num > 9) && (num < 100))
		{
			StringCbPrintf(temp, sizeof(temp), L"N   %i ", num);
		}
		else if ((num > 99) && (num < 1000))
		{
			StringCbPrintf(temp, sizeof(temp), L"N  %i ", num);
		}
		else if ((num > 999) && (num < 10000))
		{
			StringCbPrintf(temp, sizeof(temp), L"N %i ", num);
		}
		else if (num >= 10000)
		{
			StringCbPrintf(temp, sizeof(temp), L"N%i ", num);
		}
		else
		{
			StringCbPrintf(temp, sizeof(temp), L"N    %i ", num);
		}
	}
	int i = 0;

	while (temp[i] != L'\0')
	{
		if(buffer != nullptr)
			buffer[from] = temp[i];
		from++;
		i++;
		if (i == 50)
			break;
	}
	return i;
}

int EditControl::roundOffLineNumber(int currentNumber)
{
	int numLen = (int)(pow(10, this->editControlProperties.autosyntax.MainLevelNumLength - 1));// !!!!!!

	if (numLen == 0)// prevent division by zero
	{
		numLen = 100;// only a placeholder: this should never be executed!
	}

	if ((currentNumber % numLen) == 0)
		return currentNumber;				//return -1; (old - was an error I suppose)
	else
	{
		auto hundreds = currentNumber / numLen;
		int result = (hundreds + 1) * numLen;
		if (result < this->editControlProperties.autosyntax.maximumLinenumber)
			return result;
		else
			return this->editControlProperties.autosyntax.maximumLinenumber;
	}
}

int EditControl::checkNextLineForTrigger(int pos_from, LPCTSTR buffer, int currentLinenumber)
{
	if (buffer != nullptr)
	{
		bool preventer = false;
		TCHAR *lineBuffer = nullptr;

		if (_getlineFromCharIndexOutOfText(buffer, pos_from, &lineBuffer))
		{
			this->_removeCommentFromLineBuffer(lineBuffer);

			if (this->editControlProperties.autosyntax.newEvenLineNumberOnTrigger)
			{
				// check for new-even-linenumber-trigger
				if (this->newLineTrigger.checkBufferForWordMatches(lineBuffer))
				{
					currentLinenumber =
						this->roundOffLineNumber(currentLinenumber);
				}
			}
			if (this->editControlProperties.autosyntax.UseDifferentNumLengthInSubProg)
			{
				// check for sub-prog-trigger
				if (this->subProgTrigger.checkBufferForWordMatches(lineBuffer))
				{
					if (!this->lnFormat.subLevelActive)
					{
						this->lnFormat.lastNumber = currentLinenumber;
						this->lnFormat.subLevelActive = true;

						// set the linenumber to the desired length

						if (this->editControlProperties.autosyntax.SubLevelNumLength == 1)
							currentLinenumber = 1;
						else if (this->editControlProperties.autosyntax.SubLevelNumLength == 2)
							currentLinenumber = 10;
						else if (this->editControlProperties.autosyntax.SubLevelNumLength == 3)
							currentLinenumber = 100;
						else
							currentLinenumber = 1000;
					}
					else
					{
						// check the sublevel linenumber format...
						if ((this->editControlProperties.autosyntax.SubLevelNumLength == 1)
							&& (currentLinenumber > 9))
						{
							currentLinenumber = 1;
							preventer = true;
						}
						else if ((this->editControlProperties.autosyntax.SubLevelNumLength == 2)
							&& (currentLinenumber > 99))
						{
							currentLinenumber = 10;
							preventer = true;
						}
						else if ((this->editControlProperties.autosyntax.SubLevelNumLength == 3)
							&& (currentLinenumber > 999))
						{
							currentLinenumber = 100;
							preventer = true;
						}
					}				
				}
				else
				{
					if (this->lnFormat.subLevelActive)
					{
						// recover the main level number
						this->lnFormat.subLevelActive = false;
						currentLinenumber = this->lnFormat.lastNumber;
					}
				}
			}
			SafeDeleteArray(&lineBuffer);
		}
		if (this->editControlProperties.autosyntax.useEndProgDetection)
		{
			// get the complete text from position to zero
			TCHAR *segmentBuffer = nullptr;

			if (_cutString(buffer, 0, pos_from, &segmentBuffer))
			{
				if (this->endProgTrigger.checkBufferForWordMatches(segmentBuffer))
				{
					// position is out of valid program scope -> stop inserting numbers!

					currentLinenumber = -1;
				}
				SafeDeleteArray(&segmentBuffer);
			}
		}		
	}
	return currentLinenumber;
}

bool EditControl::isLineInSubProgram(int pos_from, LPCTSTR buffer, int max)
{
	UNREFERENCED_PARAMETER(max);

	bool result = false;

	// count the buffersize:
	int bufSize = 0;
	int initialPos = pos_from;

	while (pos_from > 0)
	{
		if (buffer[pos_from] == 0x0D)
			break;

		pos_from--;
		bufSize++;
	}
	pos_from++;

	TCHAR *linebuffer = new TCHAR[bufSize + 1];
	if (linebuffer != nullptr)
	{
		// record the buffer
		int recorder = 0;
		SecureZeroMemory(linebuffer, sizeof(TCHAR)*(bufSize + 1));

		while (pos_from <= initialPos)
		{
			linebuffer[recorder] = buffer[pos_from];
			recorder++;
			pos_from++;
		}

		// check for words in line
		result =
			this->subProgTrigger.checkBufferForWordMatches(linebuffer);

		SafeDeleteArray(&linebuffer);
	}
	return result;
}

int EditControl::getLastMainLevelLinenumber(int pos_from, LPCTSTR buffer)
{
	CHARRANGE cr;

	while (pos_from > 0)
	{
		if (buffer[pos_from] == 0x0D)
		{
			int num = this->getLinenumberFromCurrentLine(pos_from - 1, buffer, &cr);
			if (num != -1)
			{
				if (
					((this->editControlProperties.autosyntax.SubLevelNumLength == 1) && (num > 9)) ||
					((this->editControlProperties.autosyntax.SubLevelNumLength == 2) && (num > 99)) ||
					((this->editControlProperties.autosyntax.SubLevelNumLength == 3) && (num > 999))
					)
				{
					return num;
				}
			}
		}
		pos_from--;
	}
	return -1;
}

bool EditControl::isValidNCWordContent(TCHAR L)
{
	if (
		this->CheckForLetter(L) ||
		this->CheckForNumber(L)
		)
	{
		return true;
	}
	return false;
}

bool EditControl::getAnnotationScope(const int pos_from, LPCTSTR buffer, CHARRANGE * scope_out, const int max, bool multiline)
{
	// if this function returns false there is no annotation from this position
	// or the annotation is not valid! (e.g. the closing mark is missing)
	// scope out could be valid anyway
	bool search_successful = false;
	bool carry_triggered = false;

	if (buffer != nullptr)
	{
		int i = pos_from;
		int open_level = 0;

		// search for lower annotation mark:
		while (i >= 0)
		{
			if (!multiline)
			{
				 //if not multiline:
				if(buffer[i] == 0x0D)
				{
					break;
				}
			}
			else
			{
				if (buffer[i] == 0x0D)
				{
					carry_triggered = true;
				}
			}

			if (buffer[i] == L'(')
			{
				// class 2 mark detected
				scope_out->cpMin = i;// save lower point
				open_level = 2;
				search_successful = true;

				// do not exit the loop maybe a class 1 mark comes before this mark!
			}
			else if (buffer[i] == L'{')
			{
				// class 1 mark detected
				scope_out->cpMin = i;// save lower point
				open_level = 1;
				search_successful = true;
				break;
			}
			else if (buffer[i] == L')')
			{
				search_successful = false;
				// continue loop, maybe a class 1 mark comes before this mark!
			}
			else if (buffer[i] == L'}')
			{
				search_successful = false;
			}
			else if (buffer[i] == L';')
			{
				if (!carry_triggered)
				{
					scope_out->cpMin = i;//save lower point
					multiline = false;// even though multiline would be true, this comment-type does not support multiline, so set it to false
					search_successful = true;
					break;
				}
			}
			i--;
		}
		if (search_successful)
		{
			// lower point successful found
			// now search for the higher annotation-end
			// this means that here no syntax rules will be applied because it is in the inside-scope of the annotation
			i = pos_from;
			search_successful = false;

			while (i <= max)
			{
				if (!multiline)
				{
					// if not multiline
					if (buffer[i] == 0x0D)
					{
						// the counter reached the end of the line before the closing mark was found
						scope_out->cpMax = i;
						search_successful = true;// !!!note: this was false!!!
						break;
					}
				}
				if (buffer[i] == L')')
				{
					scope_out->cpMax = i;
					
					if (open_level == 2)
					{
						// the open level is 2 so we can return
						// otherwise we must continue the search for class 1 mark
						search_successful = true;
						break;
					}
				}
				else if (buffer[i] == L'}')
				{
					scope_out->cpMax = i;

					if (open_level == 1)
					{
						// the open level is 1 so we can return
						// otherwise we must continue the search for class 2 mark						
						search_successful = true;
						break;
					}
				}
				i++;
			}
			if (i == max)
			{
				// the end of the buffer was reached before the closing mark was found
				if (!search_successful)
				{
					scope_out->cpMax = i;
				}
			}
		}
	}
	return search_successful;
}

int EditControl::getAnnotationEnd(const int pos_from, TCHAR type, LPCTSTR buffer, const int max, bool multiline, bool* aFound)
{
	int result = -1;
	*aFound = false;

	if (buffer != nullptr)
	{
		if (pos_from <= max)
		{
			int i = pos_from;

			// step forward to find the closing annotation mark
			while (i <= max)
			{
				if ((buffer[i] == 0x0D) && !multiline)
				{
					if (type == L';')
					{
						*aFound = true;
					}
					// line end reached and multiline annotation not requested
					// return position					
					return i;
				}
				else if (buffer[i] == 0x0D)
				{
					if (type == L';')
					{
						// it is -> return position
						*aFound = true;
						return i;
					}
				}
				else if (buffer[i] == L')')// check if this is the requested closing mark
				{
					if (type == L'(')
					{
						// it is -> return position
						*aFound = true;
						return i;
					}
				}
				else if (buffer[i] == L'}')// check if this is the requested closing mark
				{
					if (type == L'{')
					{
						// it is -> return position
						*aFound = true;
						return i;
					}
				}
				i++;
			}
			result = i;
		}
	}
	return result;
}

bool EditControl::createConverterStructFromBuffer(LPCTSTR buffer, itemCollection<ColorConversion> &converterImage)
{
	if (buffer != nullptr)
	{
		auto max_buffer = _lengthOfString(buffer);
		if (max_buffer > 0)
		{
			ColorConversion cList;

			int i = 0;
			// read the buffer and record the color-structure
			while (buffer[i] != L'\0')
			{
				if (this->CheckForLetter(buffer[i]))
				{
					if (buffer[i] == L'N')
					{
						// is a linenumber
						cList.setStart(i);
						cList.setEnd(i + 1);
						cList.setColor(this->editStyleColors.N);

						converterImage.AddItem(cList);

						while (!this->CheckForNumber(buffer[i]))
						{
							if ((buffer[i] == L'\0') || (buffer[i] == L'\n') || (buffer[i] == 0x0D))
							{
								break;
							}
							i++;
						}
						cList.setStart(i);

						while (this->CheckForNumber(buffer[i]))
						{
							if ((buffer[i] == L'\0') || (buffer[i] == L'\n') || (buffer[i] == 0x0D))
							{
								break;
							}
							i++;
						}
						cList.setEnd(i + 1);
						cList.setColor(this->editStyleColors.LineNumber);

						converterImage.AddItem(cList);
					}
					else if (buffer[i] == L'V')
					{
						cList.setStart(i);
						cList.setEnd(i + 1);
						cList.setColor(this->editStyleColors.V);

						converterImage.AddItem(cList);
					}
					else
					{
						// must be a normal word
						auto col = this->GetCharColor(buffer[i]);
						bool _ex = false;

						cList.setStart(i);

						//search the end of the word
						while (buffer[i] != L' ')
						{
							if ((buffer[i] == L'\0') || (buffer[i] == L'\n') || (buffer[i] == 0x0D))
							{
								break;
							}
							if (buffer[i] == L'(')
							{
								i--;
								break;
							}
							else if (buffer[i] == L';')
							{
								i--;
								break;
							}
							else if (buffer[i] == L'{')
							{
								// must be a variable statement
								_ex = true;
								TCHAR bracketType = buffer[i];

								// search the end of the statement
								while ((bracketType == L'{') && (buffer[i] != L'}'))
								{
									if ((buffer[i] == L'\0') || (buffer[i] == 0x0D) || (i >= max_buffer))
									{
										_ex = true;
										break;
									}
									i++;
								}
							}
							if (_ex)
								break;
							i++;
						}
						cList.setEnd(i + 1);
						cList.setColor(col);

						converterImage.AddItem(cList);
					}
				}
				else
				{
					if ((buffer[i] == L'(') || (buffer[i] == L'{') || (buffer[i] == L';'))
					{
						cList.setStart(i);

						if (buffer[i] == L'(')
						{
							while (buffer[i] != L')')
							{
								if (buffer[i] == L'\0')
								{
									break;
								}
								if (!this->editControlProperties.autosyntax.useMultilineAnnotations)
								{
									if (buffer[i] == 0x0D)
									{
										break;
									}
								}
								i++;
							}
						}
						else if(buffer[i] == L'{')
						{
							while (buffer[i] != L'}')
							{
								if (buffer[i] == L'\0')
								{
									break;
								}
								if (!this->editControlProperties.autosyntax.useMultilineAnnotations)
								{
									if (buffer[i] == 0x0D)
									{
										break;
									}
								}
								i++;
							}
						}
						else// must be ';'
						{
							while ((buffer[i] != 0x0D) && (buffer[i] != 0x0A))
							{
								if (buffer[i] == L'\0')
								{
									break;
								}
								i++;
							}
						}
						cList.setEnd(i + 1);
						cList.setColor(this->editStyleColors.Annotation);

						converterImage.AddItem(cList);
					}
				}
				i++;
			}
		}
		return true;
	}
	return false;
}

void EditControl::_setsel(CHARRANGE * pcr)
{
	SendMessage(this->EditWnd, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(pcr));
}

void EditControl::_getsel(CHARRANGE * pcr)
{
	SendMessage(this->EditWnd, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(pcr));
}

intX EditControl::_lineFromCharIndex(int index)
{
	if (index > 0)
		return SendMessage(this->EditWnd, EM_EXLINEFROMCHAR, 0, static_cast<LPARAM>(index));
	else
		return 0;
}

void EditControl::_restoreContent(bool restoreScroll)
{
	if(restoreScroll)
		this->saveScrollPosition();

	this->textFormat.crTextColor = this->editStyleColors.defaultTextcolor;

	SendMessage(
		this->EditWnd,
		EM_SETCHARFORMAT,
		static_cast<WPARAM>(SCF_ALL),
		reinterpret_cast<LPARAM>(&this->textFormat));

	if(restoreScroll)
		this->restoreSavedScrollPosition();
}

void EditControl::_removeCommentFromLineBuffer(LPTSTR buffer)
{
	TCHAR* bCopy = nullptr;

	if (CopyStringToPtr(buffer, &bCopy) == TRUE)
	{
		auto len = _lengthOfString(buffer);
		if (len > 0)
		{
			bool prcss = true, secseg = false;
			int last_index = -1;
			TCHAR bType = L'\0';

			for (int i = 0; i < len; i++)
			{
				if (prcss)
				{
					if (buffer[i] == L';')
					{
						buffer[i] = L'\0';
						break;
					}
					else if ((buffer[i] == L'(') || (buffer[i] == L'{'))
					{
						bType = buffer[i];
						last_index = i;
						prcss = false;
					}
				}
				else
				{
					if (secseg)
					{
						buffer[last_index] = bCopy[i];
						last_index++;
					}
					else
					{
						if (((buffer[i] == L')') && (bType == L'('))
							|| ((buffer[i] == L'}') && (bType == L'{')))
						{
							secseg = true;
						}
					}
				}
				if (last_index != -1)
				{
					buffer[last_index] = L'\0';
				}
			}
		}
		SafeDeleteArray(&bCopy);
	}
}

void EditControl::getCurrentFormat(CHARFORMAT * ccf)
{
	if (ccf != nullptr)
	{
		SendMessage(
			this->EditWnd,
			EM_GETCHARFORMAT,
			static_cast<WPARAM>(SCF_SELECTION),
			reinterpret_cast<LPARAM>(ccf)
		);
	}
}

bool EditControl::isEmptyLine(LPCTSTR buffer, int len)
{
	if (len == -1)
		len = _lengthOfString(buffer);

	if (len > 0)
	{
		for (int i = 0; i < len; i++)
		{
			if ((buffer[i] != L'N') && (buffer[i] != L' ') && (buffer[i] != 0x0D) && !IsNumber(buffer[i]))
			{
				return false;
			}
		}
	}
	return true;
}

void EditControl::updateContentBuffer()
{
	SafeDeleteArray(&this->editboxContent);

	if (GetRichEditContent(this->EditWnd, &this->editboxContent) != TRUE)
	{
		this->editboxContent = nullptr;
	}
}

void EditControl::eraseContentBuffer()
{
	SafeDeleteArray(&this->editboxContent);
}

bool EditControl::isVariableStatement(int pos_from, LPCTSTR buffer, COLORREF *color_out)
{
	// this function checks for a valid letter in front of '{'
	// -> negative direction
	if ((buffer != nullptr) && (pos_from > 0))
	{
		auto max_buffer = _lengthOfString(buffer);
		if (pos_from <= max_buffer)
		{
			// make sure the position is valid
			if (buffer[pos_from] == L'{')
			{
				if (pos_from > 0)
				{
					if (buffer[pos_from - 1] != L'V')
					{
						if (this->CheckForLetter(buffer[pos_from - 1]))
						{
							*color_out = this->GetCharColor(buffer[pos_from - 1]);
							return true;
						}
						else
						{
							// maybe step back and search for '{'
						}
					}
				}
			}
		}
	}
	return false;
}

bool EditControl::isLineNumber(int pos_from, LPCTSTR buffer, int caretPos, bool actionWasBackspace)
{
	// this function verifies that the found N is really a linenumber!
	if ((buffer != nullptr) && (pos_from >= 0))
	{
		auto max_buffer = _lengthOfString(buffer);
		if (max_buffer > 0)
		{
			if (caretPos <= max_buffer)
			{

				// TODO:
				// step back, make sure the N is on the beginning of the line
				// what is when the function 'no space in line number' is activated???


				// make sure it is N
				if (buffer[pos_from] == L'N')
				{
					int i = pos_from + 1;
					bool firstSpaceReached = false;
					bool numberReached = false;

					// step up and verify linenumberformat
					while (i <= caretPos)
					{
						if (buffer[i] == L' ')
						{
							if (!firstSpaceReached)
							{
								firstSpaceReached = true;

								while (i <= caretPos)
								{
									if (buffer[i] != L' ')
										break;
									i++;
								}
							}
							else
							{
								if (actionWasBackspace)
								{
									if (this->CheckForNumber(this->backspaceChar))
									{
										return true;
									}
								}
								return false;
							}
						}
						else if (buffer[i] == 0x0D)
						{
							if (actionWasBackspace)
							{
								if (this->backspaceChar == L' ')
								{
									return false;
								}
							}
							return true;
						}
						else if (this->CheckForNumber(buffer[i]))
						{
							numberReached = true;
						}
						else
						{
							if (actionWasBackspace)
							{
								if (i == caretPos)
								{
									if (this->backspaceChar != L' ')
									{
										return true;
									}
								}
							}
							return false;
						}
						i++;
					}
				}
			}
		}
	}
	return true;
}

bool EditControl::isFirst_N_inLine(LPCTSTR buffer, int pos)
{
	if (buffer != nullptr)
	{
		if (pos >= 0)
		{
			if (buffer[pos] == L'N')
			{
				pos--;

				while (pos >= 0)
				{
					if (buffer[pos] == L'N')
					{
						return false;
					}
					else if (buffer[pos] == 0x0D)
					{
						return true;
					}
					pos--;
				}
			}
		}
	}
	return true;
}

void EditControl::showValues()
{
	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_ALL;

	this->getCurrentFormat(&cf);

	show_integer(cf.yHeight, cf.yOffset);

}


EditControl* CreateEditControlInstance(HWND EditWnd, HWND AssocConnect) { return new EditControl(EditWnd, AssocConnect); }


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REDO / UNDO ACTIONS ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UndoRedoChain::addNewUndoAction(LPUNDOREDOACTIONS action_in)
{
	if (this->currentActionIndex > 0)
	{
		// redo was executed earlier!
		// -> now an undo action was added, so remove the redo-action and sort the stack and set the new action at index 0
		this->removeRedoAndReformat();

		this->actions[0].action = action_in->action;
		this->actions[0].range = action_in->range;
		SafeDeleteArray(&this->actions[0].content);
		CopyStringToPtr(action_in->content, &this->actions[0].content);

		this->actions[0].replacedRange = action_in->replacedRange;
		SafeDeleteArray(&this->actions[0].replacedContent);
		CopyStringToPtr(action_in->replacedContent, &this->actions[0].replacedContent);
	}
	else
	{
		int i = 0;

		// search for the last undo action
		for (i = 0; i < MAX_UNDO_STACK; i++)
		{
			if (this->actions[i].action == -1)
				break;
		}
		if (i >= MAX_UNDO_STACK)
		{
			// the array is full
			i = (MAX_UNDO_STACK - 1);
		}
		if (i > 0)
		{
			// shift the content backward to add the new content on top
			for (int k = i - 1; k >= 0; k--)
			{
				this->actions[k + 1].action = this->actions[k].action;
				this->actions[k + 1].range = this->actions[k].range;
				SafeDeleteArray(&this->actions[k + 1].content);
				CopyStringToPtr(this->actions[k].content, &this->actions[k + 1].content);

				this->actions[k + 1].replacedRange = this->actions[k].replacedRange;
				SafeDeleteArray(&this->actions[k + 1].replacedContent);
				CopyStringToPtr(this->actions[k].replacedContent, &this->actions[k + 1].replacedContent);
			}
		}
		// add the new action
		this->actions[0].action = action_in->action;
		this->actions[0].range = action_in->range;
		SafeDeleteArray(&this->actions[0].content);
		CopyStringToPtr(action_in->content, &this->actions[0].content);

		this->actions[0].replacedRange = action_in->replacedRange;
		SafeDeleteArray(&this->actions[0].replacedContent);
		CopyStringToPtr(action_in->replacedContent, &this->actions[0].replacedContent);

		// set the action-count
		this->availableActions++;
		if (this->availableActions > MAX_UNDO_STACK)
			this->availableActions = MAX_UNDO_STACK;
	}
}

bool UndoRedoChain::UndoRequest(LPUNDOREDOACTIONS action_out)
{
	// if this function returns true UNDO could be executed otherwise the stack is empty
	if (this->availableActions == 0)
		return false;
	else
	{
		if (this->availableActions <= this->currentActionIndex)
			return false;
		else
		{
			action_out->action = this->actions[this->currentActionIndex].action;
			action_out->range = this->actions[this->currentActionIndex].range;
			action_out->content = nullptr;
			CopyStringToPtr(this->actions[this->currentActionIndex].content, &action_out->content);

			action_out->replacedRange = this->actions[this->currentActionIndex].replacedRange;
			action_out->replacedContent = nullptr;
			CopyStringToPtr(this->actions[this->currentActionIndex].replacedContent, &action_out->replacedContent);

			this->currentActionIndex++;

			return true;
		}
	}
}

bool UndoRedoChain::RedoRequest(LPUNDOREDOACTIONS action_out)
{
	// if this function returns true REDO could be executed otherwise the stack is empty
	if (this->currentActionIndex <= 0)
		return false;
	else
	{
		this->currentActionIndex--;

		action_out->action = this->actions[this->currentActionIndex].action;
		action_out->range = this->actions[this->currentActionIndex].range;
		action_out->content = nullptr;
		CopyStringToPtr(this->actions[this->currentActionIndex].content, &action_out->content);

		action_out->replacedRange = this->actions[this->currentActionIndex].replacedRange;
		action_out->replacedContent = nullptr;
		CopyStringToPtr(this->actions[this->currentActionIndex].replacedContent, &action_out->replacedContent);

		return true;
	}
}

void UndoRedoChain::getActionAt(int index, LPUNDOREDOACTIONS action_out)
{
	if (action_out != nullptr)
	{
		if (index < MAX_UNDO_STACK)
		{
			action_out->action = this->actions[index].action;
			action_out->range = this->actions[index].range;

			SafeDeleteArray(&action_out->content);
			CopyStringToPtr(this->actions[index].content, &action_out->content);

			action_out->replacedRange = this->actions[index].replacedRange;

			SafeDeleteArray(&action_out->replacedContent);
			CopyStringToPtr(this->actions[index].replacedContent, &action_out->replacedContent);
		}
	}
}

void UndoRedoChain::replaceActionAt(int index, LPUNDOREDOACTIONS action_in)
{
	if (action_in != nullptr)
	{
		if (index < MAX_UNDO_STACK)
		{
			this->actions[index].action = action_in->action;
			this->actions[index].range = action_in->range;

			SafeDeleteArray(&this->actions[index].content);
			CopyStringToPtr(action_in->content, &this->actions[index].content);

			this->actions[index].replacedRange = action_in->replacedRange;

			SafeDeleteArray(&this->actions[index].replacedContent);
			CopyStringToPtr(action_in->replacedContent, &this->actions[index].replacedContent);
		}
	}
}

void UndoRedoChain::removeRedoAndReformat()
{
	// first the executed REDO actions will be deleted
	// then the stack will be reformatted so that the first index (0) remains free to take the new content
	for (int index = 0; index < this->currentActionIndex; index++)
	{
		this->clearAtIndex(index);
	}
	if (this->currentActionIndex > 1)
	{
		int i = 0;
		int step = this->currentActionIndex - 1;

		for (i = this->currentActionIndex; i < MAX_UNDO_STACK; i++)
		{
			if (this->actions[i].action == -1)
				break;

			this->actions[i - step].action = this->actions[i].action;
			this->actions[i - step].range = this->actions[i].range;

			SafeDeleteArray(&this->actions[i - step].content);
			CopyStringToPtr(this->actions[i].content, &this->actions[i - step].content);

			this->actions[i - step].replacedRange = this->actions[i].replacedRange;

			SafeDeleteArray(&this->actions[i - step].replacedContent);
			CopyStringToPtr(this->actions[i].replacedContent, &this->actions[i - step].replacedContent);
		}
		for (int x = i; x > (i - step); x--)
		{
			this->clearAtIndex(x);
		}
		this->availableActions = (i - step) + 1;
	}
	this->currentActionIndex = 0;
}

bool TriggerWordContainer::isWordInside(LPCTSTR word)
{
	auto w_count = this->words.GetCount();
	if (w_count > 0)
	{
		for (int i = 0; i < w_count; i++)
		{
			auto pStr = this->words.getObjectCoreReferenceAt(i);
			if (pStr != nullptr)
			{
				if (pStr->Equals(word))
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool TriggerWordContainer::setContentFromTriggerString(LPCTSTR tStr)
{
	// first erase the content
	this->clear();

	// then read the string and save the words
	if (tStr != nullptr)
	{
		auto len = _lengthOfString(tStr);
		if (len > 0)
		{
			int counter = 0;
			int recorder = 0;
			TCHAR word[256] = { 0 };

			while (counter <= len)
			{
				if (tStr[counter] == L'\0')
					break;

				if (tStr[counter] == L';')
				{
					iString w(word);
					this->words.AddItem(w);

					recorder = 0;
					SecureZeroMemory(word, sizeof(word));
				}
				else
				{
					word[recorder] = tStr[counter];
					recorder++;
				}
				counter++;
			}
			if (this->words.GetCount() > 0)
			{
				return true;
			}
		}
	}
	return false;
}

bool TriggerWordContainer::checkBufferForWordMatches(LPCTSTR buffer)
{
	auto wCount = this->getWordCount();
	if (wCount > 0)
	{
		iString str(buffer);

		for (int i = 0; i < wCount; i++)
		{
			if (
				str.Contains(
					this->words.getObjectCoreReferenceAt(i)->GetData()
				))
			{
				return true;
			}
		}
	}
	return false;
}


/* old format function
void EditControl::FormatAll()
{
	int i = 0, s_count = 0, pos = 0;
	TCHAR* buffer = NULL;
	CHARRANGE cr;
	GETTEXTEX gtx;

	GETTEXTLENGTHEX gtlx;
	gtlx.codepage = 1200;
	gtlx.flags = GTL_DEFAULT;

	int len = SendMessage(this->EditWnd, EM_GETTEXTLENGTHEX, reinterpret_cast<WPARAM>(&gtlx), static_cast<LPARAM>(0));

	buffer = new TCHAR[len + 100];
	if (buffer != NULL)
	{
		gtx.cb = sizeof(TCHAR) * (len + 100);
		gtx.codepage = 1200;
		gtx.flags = GT_DEFAULT;
		gtx.lpDefaultChar = NULL;
		gtx.lpUsedDefChar = NULL;

		LPCONVERTERSTRUCT cList = new CONVERTERSTRUCT[10000];
		if (cList != NULL)
		{
			LRESULT res = SendMessage(
				this->EditWnd,
				EM_GETTEXTEX,
				reinterpret_cast<WPARAM>(&gtx),
				reinterpret_cast<LPARAM>(buffer));

			if (res != 0)
			{
				SendMessage(
					this->EditWnd,
					EM_EXGETSEL,
					static_cast<WPARAM>(0),
					reinterpret_cast<LPARAM>(&cr));

				if (cr.cpMax == cr.cpMin)
				{
					pos = cr.cpMax;

					while (buffer[i] != L'\0')
					{
						if (this->CheckForLetter(buffer[i]))
						{
							if (buffer[i] == L'N')
							{
								cList[s_count].start = i;
								if (cList[s_count].start < 0)
								{
									cList[s_count].start = 0;
								}
								cList[s_count].end = i + 1;
								cList[s_count].color = this->editStyleColors.N;
								s_count++;

								while (!this->CheckForNumber(buffer[i]))
								{
									if ((buffer[i] == L'\0') || (buffer[i] == L'\n') || (buffer[i] == 0x0D))
									{
										break;
									}
									i++;
								}
								cList[s_count].start = i;

								while (this->CheckForNumber(buffer[i]))
								{
									if ((buffer[i] == L'\0') || (buffer[i] == L'\n') || (buffer[i] == 0x0D))
									{
										break;
									}
									i++;
								}
								cList[s_count].end = i + 1;
								cList[s_count].color = this->editStyleColors.LineNumber;
								s_count++;
							}
							else if (buffer[i] == L'V')
							{
								cList[s_count].start = i;
								cList[s_count].end = i + 1;
								cList[s_count].color = this->editStyleColors.V;
								s_count++;
							}
							else
							{
								auto col = this->GetCharColor(buffer[i]);
								bool _ex = false;

								cList[s_count].start = i;

								while (buffer[i] != L' ')
								{
									if ((buffer[i] == L'\0') || (buffer[i] == L'\n') || (buffer[i] == 0x0D))
									{
										break;
									}
									if ((buffer[i] == L'{') || (buffer[i] == L'('))
									{
										_ex = true;

										// search the end of the statement
										while ((buffer[i] != L')') && (buffer[i] != L'}'))
										{
											if ((buffer[i] == L'\0') || (buffer[i] == L'\n') || (buffer[i] == 0x0D))
											{
												break;
											}
											i++;
										}
									}
									if (_ex)
										break;
									i++;
								}
								cList[s_count].end = i + 1;
								cList[s_count].color = col;
								s_count++;
							}
						}
						else
						{
							if ((buffer[i] == L'(') || (buffer[i] == L'{'))
							{
								cList[s_count].start = i;

								if (buffer[i] == L'(')
								{
									while (buffer[i] != L')')
									{
										if (buffer[i] == L'\0')
										{
											break;
										}
										i++;
									}
								}
								else
								{
									while (buffer[i] != L'}')
									{
										if (buffer[i] == L'\0')
										{
											break;
										}
										i++;
									}

								}
								cList[s_count].end = i + 1;
								cList[s_count].color = this->editStyleColors.Annotation;
								s_count++;
							}
						}
						i++;
					}
					// loop finished... ///////////////////////////////////////////////////////////
					i = 0;

					// get scroll position:
					SCROLLINFO psi;
					psi.cbSize = sizeof(SCROLLINFO);
					psi.fMask = SIF_POS;

					GetScrollInfo(this->EditWnd, SB_VERT, &psi);

					// block notifications
					this->ConversionNotificationBlocker = TRUE;

					// set the colors:
					while (i < s_count)
					{
						if (cList[i].start < 0)
						{
							cList[i].start = 0;
						}

						cr.cpMin = cList[i].start;
						cr.cpMax = cList[i].end;
						this->textFormat.crTextColor = cList[i].color;

						SendMessage(
							this->EditWnd,
							EM_EXSETSEL,
							static_cast<WPARAM>(0),
							reinterpret_cast<LPARAM>(&cr));

						SendMessage(
							this->EditWnd,
							EM_SETCHARFORMAT,
							static_cast<WPARAM>(SCF_SELECTION),
							reinterpret_cast<LPARAM>(&this->textFormat));
						i++;
					}
					cr.cpMax = pos;
					cr.cpMin = pos;

					SendMessage(
						this->EditWnd,
						EM_EXSETSEL,
						static_cast<WPARAM>(0),
						reinterpret_cast<LPARAM>(&cr));

					this->textFormat.crTextColor = this->editStyleColors.defaultTextcolor;

					SendMessage(
						this->EditWnd,
						EM_SETCHARFORMAT,
						static_cast<WPARAM>(SCF_SELECTION),
						reinterpret_cast<LPARAM>(&this->textFormat));

					// reset scroll position
					SetScrollInfo(this->EditWnd, SB_VERT, &psi, TRUE);
					SendMessage(this->EditWnd, WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, psi.nPos), 0);

					// free notificationblocker
					this->ConversionNotificationBlocker = FALSE;
				}
			}
			delete[] cList;
		}
		delete[] buffer;
	}
}
*/

