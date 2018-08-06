#include "CnCS_CB.h"
#include"HelperF.h"
#include"Error dispatcher.h"

CnCS_CB::CnCS_CB(HINSTANCE hInst, HWND MainWindow) : hInstance(hInst), Main(MainWindow), Ecount(0), TimerActive(FALSE), Listview(nullptr)
{
	this->_createDpiDependendResources();
}

CnCS_CB::~CnCS_CB()
{
	DeleteObject(this->lvFont);
}

HRESULT CnCS_CB::_init(HWND Parent)
{
	if (Parent == NULL)
		return E_HANDLE;
	else
	{
		HRESULT hr;

		this->CBFrame = Parent;

		hr = SetWindowSubclass(Parent, CnCS_CB::CBoxSub, NULL, reinterpret_cast<DWORD_PTR>(this)) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			INITCOMMONCONTROLSEX icex;
			icex.dwICC = ICC_LISTVIEW_CLASSES;

			InitCommonControlsEx(&icex);

			RECT rc;
			GetClientRect(Parent, &rc);

			this->Listview = CreateWindowEx(0, WC_LISTVIEW, NULL, WS_CHILD | LVS_REPORT | LVS_ICON | LVS_NOSORTHEADER, 0, 0, rc.right, rc.bottom, Parent, (HMENU)ID_CBOX, this->hInstance, NULL);

			hr = (this->Listview != NULL) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				ListView_SetExtendedListViewStyleEx(this->Listview, 0, LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

				hr = this->InitColumns();
				if (SUCCEEDED(hr))
				{
					hr = this->InitImageList();
					if (SUCCEEDED(hr))
					{
						APPSTYLEINFO StyleInfo;
						SendMessage(this->Main, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&StyleInfo));

						hr = ListView_SetBkColor(this->Listview, StyleInfo.Stylecolor) ? S_OK : E_FAIL;
						if (SUCCEEDED(hr))
						{
							hr = ListView_SetTextColor(this->Listview, StyleInfo.TextColor) ? S_OK : E_FAIL;
							if (SUCCEEDED(hr))
							{
								SendMessage(this->Listview, WM_SETFONT, reinterpret_cast<WPARAM>(this->lvFont), static_cast<LPARAM>(TRUE));

								hr = ListView_SetTextBkColor(this->Listview, StyleInfo.Stylecolor) ? S_OK : E_FAIL;
								if (SUCCEEDED(hr))
								{
									SetLastError(0);

									hr = ((SetWindowLongPtr(this->CBFrame, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this)) == 0)
										&& (GetLastError() != 0)) ? E_FAIL : S_OK;
									if (SUCCEEDED(hr))
									{
										ShowWindow(this->Listview, SW_SHOW);

										// ...
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
}

HRESULT CnCS_CB::InitColumns()
{
	HRESULT hr;

	RECT rc;
	GetClientRect(this->Listview, &rc);

	TCHAR firstcol[] = L"Code\0";
	TCHAR thirdcol[] = L"Location\0";

	int c1 = 0, c2 = 0, c3 = 0;

	if ((rc.right - 230) > 50)
	{
		c1 = DPIScale(80);
		c2 = rc.right - DPIScale(230);
		c3 = DPIScale(150);
	}
	else
	{
		c1 = rc.right / 3;
		c2 = c1;
		c3 = c1;
	}

	LVCOLUMN lvc;

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	lvc.iSubItem = 0;
	lvc.pszText = firstcol;
	lvc.cx = c1;

	hr = (ListView_InsertColumn(this->Listview, 0, &lvc) == -1) ? E_FAIL : S_OK;
	if (SUCCEEDED(hr))
	{
		lvc.iSubItem = 1;
		lvc.pszText = getStringFromResource(UI_GNRL_DESCRIPTION);
		lvc.cx = c2;

		hr = (ListView_InsertColumn(this->Listview, 1, &lvc) == -1) ? E_FAIL : S_OK;
		if (SUCCEEDED(hr))
		{
			lvc.iSubItem = 2;
			lvc.pszText = thirdcol;
			lvc.cx = c3;

			hr = (ListView_InsertColumn(this->Listview, 2, &lvc) == -1) ? E_FAIL : S_OK;
		}
	}
	return hr;
}

HRESULT CnCS_CB::InitImageList()
{
	HICON icon;
	HIMAGELIST himg_small, himg_large;

	himg_small = ImageList_Create(16, 16, ILC_COLOR32, 1, 1);
	himg_large = ImageList_Create(
		GetSystemMetrics(SM_CXICON),
		GetSystemMetrics(SM_CYICON),
		ILC_COLOR32, 1, 1);

	for (int i = 0; i < 3; i++)
	{
		icon = (HICON)LoadImage(this->hInstance, MAKEINTRESOURCE(IDI_CBOX_ERROR + i), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

		ImageList_AddIcon(himg_small, icon);
		ImageList_AddIcon(himg_large, icon);

		DestroyIcon(icon);

	}
	ListView_SetImageList(this->Listview, himg_small, LVSIL_SMALL);
	ListView_SetImageList(this->Listview, himg_large, LVSIL_NORMAL);

	return S_OK;
}

LRESULT CnCS_CB::CBoxSub(HWND listview, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uID, DWORD_PTR RefData)
{
	UNREFERENCED_PARAMETER(uID);

	CnCS_CB* pCB = reinterpret_cast<CnCS_CB*>(RefData);
	if (pCB != NULL)
	{
		switch (message)
		{
		case WM_SIZE:
			return pCB->OnSize();
		case WM_NOTIFY:
			return pCB->OnNotify(lParam);
		case WM_SETAPPSTYLE:
			return pCB->OnSetAppStyle(lParam);
		case WM_VALIDATEERROR:
			return pCB->OnValidateError(lParam);
		default:
			break;
		}
	}
	return DefSubclassProc(listview,message,wParam,lParam);
}

void CnCS_CB::_display(int type, LPCTSTR Code,LPCTSTR definition, LPCTSTR location)
{
	if (this->ValidateError(type, Code, location, NULL))
	{
		if (this->ErrorStorageControl(ERM_ADDERROR, type, Code, definition, location))
		{
			LVITEM lvi;

			lvi.pszText = LPSTR_TEXTCALLBACK;
			lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
			lvi.stateMask = 0;
			lvi.iSubItem = 0;
			lvi.state = 0;

			lvi.iItem = this->Ecount;
			lvi.iImage = type;

			ListView_InsertItem(this->Listview, &lvi);

			if (this->TimerActive)
			{
				if(KillTimer(this->Main, ID_TIMER_VALIDATEERROR))
					this->TimerActive = FALSE;
			}
			if(SetTimer(this->Main, ID_TIMER_VALIDATEERROR, 60000, NULL) != 0)
				this->TimerActive = TRUE;
		}
	}
}

void CnCS_CB::_remove(int type, LPCTSTR code, LPCTSTR location)
{
	this->ErrorStorageControl(ERM_REMOVE, type, code, NULL, location);
}

LRESULT CnCS_CB::OnSize()
{
	RECT rc;
	GetClientRect(this->CBFrame, &rc);

	int c1 = 0, c2 = 0, c3 = 0;

	//SetWindowPos(this->Listview, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);

	if ((rc.right - 230) > 50)
	{
		c1 = DPIScale(80);
		c2 = rc.right - DPIScale(230);
		c3 = DPIScale(150);
	}
	else
	{
		c1 = rc.right / 3;
		c2 = c1;
		c3 = c1;
	}

	ListView_SetColumnWidth(this->Listview, 0, c1);
	ListView_SetColumnWidth(this->Listview, 1, c2);
	ListView_SetColumnWidth(this->Listview, 2, c3);

	SetWindowPos(this->Listview, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);

	return static_cast<LRESULT>(0);
}

LRESULT CnCS_CB::OnNotify(LPARAM lParam)
{
	switch (((LPNMHDR)lParam)->code)
	{
		case LVN_GETDISPINFO:
		{
			NMLVDISPINFO* nld = reinterpret_cast<NMLVDISPINFO*>(lParam);

			if (nld != NULL)
			{
				switch (nld->item.iSubItem)
				{
				case 0:
					StringCbCopy(nld->item.pszText, nld->item.cchTextMax, this->lvd[nld->item.iItem].code);
					break;
				case 1:
					StringCbCopy(nld->item.pszText, nld->item.cchTextMax, this->lvd[nld->item.iItem].definition);
					break;
				case 2:
					StringCbCopy(nld->item.pszText, nld->item.cchTextMax, this->lvd[nld->item.iItem].location);
					break;
				default:
					break;
				}
			}
		}
		break;
		case LVN_DELETEALLITEMS:
			this->Ecount = 0;
			return static_cast<LRESULT>(TRUE);
		default:
			break;
	}
	return static_cast<LRESULT>(0);
}

LRESULT CnCS_CB::OnSetAppStyle(LPARAM lParam)
{
	BOOL result = TRUE;

	LPAPPSTYLEINFO pStyleInfo = reinterpret_cast<LPAPPSTYLEINFO>(lParam);
	if (pStyleInfo != NULL)
	{
		result = ListView_SetBkColor(this->Listview, pStyleInfo->Stylecolor);
		if (result)
		{
			result = ListView_SetTextColor(this->Listview, pStyleInfo->TextColor);
			if (result)
			{
				result = ListView_SetTextBkColor(this->Listview, pStyleInfo->Stylecolor);
				if (result)
				{
					// ...
				}
			}
		}
	}
	else
		result = FALSE;

	return static_cast<LRESULT>(result);
}

LRESULT CnCS_CB::OnValidateError(LPARAM lParam)
{
	BOOL ncsDelete = FALSE;

	LPEDSPSTRUCT pedsp = reinterpret_cast<LPEDSPSTRUCT>(lParam);
	if (pedsp != NULL)
	{
		if (pedsp->type == EDSP_INFO)
			ncsDelete = FALSE;
		else
		{

		}
	}
	return static_cast<LRESULT>(ncsDelete);
}

BOOL CnCS_CB::ErrorStorageControl(int mode, int type, LPCTSTR code, LPCTSTR definition, LPCTSTR location)
{
	BOOL result = TRUE;

	if (mode == ERM_ADDERROR)
	{
		if (this->Ecount == 0)
		{
			this->lvd = new LVDISPSTRUCT;

			SecureZeroMemory(this->lvd, sizeof(LVDISPSTRUCT));

			result = (this->lvd != NULL) ? TRUE : FALSE;
			if (result)
			{
				this->lvd->type = type;

				result = (CopyStringToPtrA(code, &this->lvd->code) == TRUE) ? TRUE : FALSE;
				if (result)
				{
					result = (CopyStringToPtrA(definition, &this->lvd->definition) == TRUE) ? TRUE : FALSE;
					if (result)
					{
						result = (CopyStringToPtrA(location, &this->lvd->location) == TRUE) ? TRUE : FALSE;
						if (result)
						{
							// ...

						}
					}
				}
			}
		}
		else if (this->Ecount == 1)
		{
			LVDISPSTRUCT old;

			SecureZeroMemory(&old, sizeof(LVDISPSTRUCT));

			old.type = this->lvd->type;

			result = (CopyStringToPtrA(this->lvd->code, &old.code) == TRUE) ? TRUE : FALSE;
			if (result)
			{
				result = (CopyStringToPtrA(this->lvd->definition, &old.definition) == TRUE) ? TRUE : FALSE;
				if (result)
				{
					result = (CopyStringToPtrA(this->lvd->location, &old.location) == TRUE) ? TRUE : FALSE;
					if (result)
					{
						SafeDeleteArray(&this->lvd->code);
						SafeDeleteArray(&this->lvd->definition);
						SafeDeleteArray(&this->lvd->location);
						SafeDelete(&this->lvd);

						this->lvd = new LVDISPSTRUCT[(size_t)(2 * sizeof(LVDISPSTRUCT))];

						result = (this->lvd != NULL) ? TRUE : FALSE;
						if (result)
						{
							SecureZeroMemory(this->lvd, sizeof(LVDISPSTRUCT) * 2);

							this->lvd[0].type = old.type;

							result = (CopyStringToPtrA(old.code, &this->lvd[0].code) == TRUE) ? TRUE : FALSE;
							if (result)
							{
								SafeDeleteArray(&old.code);

								result = (CopyStringToPtrA(old.definition, &this->lvd[0].definition) == TRUE) ? TRUE : FALSE;
								if (result)
								{
									SafeDeleteArray(&old.definition);

									result = (CopyStringToPtrA(old.location, &this->lvd[0].location) == TRUE) ? TRUE : FALSE;
									if (result)
									{
										SafeDeleteArray(&old.location);

										this->lvd[1].type = type;

										result = (CopyStringToPtrA(code, &this->lvd[1].code) == TRUE) ? TRUE : FALSE;
										if (result)
										{
											result = (CopyStringToPtrA(definition, &this->lvd[1].definition) == TRUE) ? TRUE : FALSE;
											if (result)
											{
												result = (CopyStringToPtrA(location, &this->lvd[1].location) == TRUE) ? TRUE : FALSE;
												if (result)
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
		else
		{
			LPLVDISPSTRUCT old = new LVDISPSTRUCT[(size_t)(this->Ecount * sizeof(LVDISPSTRUCT))];

			result = IsPtrValid(old);
			if (result)
			{
				SecureZeroMemory(old, (size_t)(this->Ecount * sizeof(LVDISPSTRUCT)));

				for (DWORD i = 0; i < this->Ecount; i++)
				{
					old[i].type = this->lvd[i].type;

					result = (CopyStringToPtrA(this->lvd[i].code, &old[i].code) == TRUE) ? TRUE : FALSE;
					if (result)
					{
						SafeDeleteArray(&this->lvd[i].code);

						result = (CopyStringToPtrA(this->lvd[i].definition, &old[i].definition) == TRUE) ? TRUE : FALSE;
						if (result)
						{
							SafeDeleteArray(&this->lvd[i].definition);

							result = (CopyStringToPtrA(this->lvd[i].location, &old[i].location) == TRUE) ? TRUE : FALSE;
							if (result)
							{
								SafeDeleteArray(&this->lvd[i].location);

								// ...
							}
						}
					}
					if (!result)
						break;
				}
				SafeDeleteArray(&this->lvd);

				if (result)
				{
					this->lvd = new LVDISPSTRUCT[(size_t)((this->Ecount + 1) * sizeof(LVDISPSTRUCT))];

					result = IsPtrValid(this->lvd);
					if (result)
					{
						DWORD i;
						SecureZeroMemory(this->lvd, sizeof(LVDISPSTRUCT)*(this->Ecount + 1));

						for (i = 0; i < this->Ecount; i++)
						{
							this->lvd[i].type = old[i].type;

							result = (CopyStringToPtrA(old[i].code, &this->lvd[i].code) == TRUE) ? TRUE : FALSE;
							if (result)
							{
								SafeDeleteArray(&old[i].code);

								result = (CopyStringToPtrA(old[i].definition, &this->lvd[i].definition) == TRUE) ? TRUE : FALSE;
								if (result)
								{
									SafeDeleteArray(&old[i].definition);

									result = (CopyStringToPtrA(old[i].location, &this->lvd[i].location) == TRUE) ? TRUE : FALSE;
									if (result)
									{
										SafeDeleteArray(&old[i].location);

										// ...
									}
								}
							}
							if (!result)
								break;
						}
						SafeDeleteArray(&old);

						if (result)
						{
							this->lvd[i].type = type;

							result = (CopyStringToPtrA(code, &this->lvd[i].code) == TRUE) ? TRUE : FALSE;
							if (result)
							{
								result = (CopyStringToPtrA(definition, &this->lvd[i].definition) == TRUE) ? TRUE : FALSE;
								if (result)
								{
									result = (CopyStringToPtrA(location, &this->lvd[i].location) == TRUE) ? TRUE : FALSE;
									if (result)
									{
										// ...

									}
								}
							}
						}
					}
				}
				SafeDeleteArray(&old);
			}
		}
		this->Ecount++;
	}
	else if (mode == ERM_REMOVE)
	{
		DWORD index = 0;
		BOOL killthetimer = FALSE;

		if (!this->ValidateError(type, code, location, &index))
		{
			if (this->Ecount == 1)
			{
				if (index == 0)
				{
					SafeDeleteArray(&this->lvd->code);
					SafeDeleteArray(&this->lvd->definition);
					SafeDeleteArray(&this->lvd->location);
					SafeDeleteArray(&this->lvd);

					killthetimer = TRUE;
				}
				else
					result = FALSE;
			}
			else
			{
				if ((index + 1) > this->Ecount)
				{
					result = FALSE;
				}
				else
				{
					LPLVDISPSTRUCT old = new LVDISPSTRUCT[(size_t)((this->Ecount - 1)* sizeof(LVDISPSTRUCT))];
					
					result = IsPtrValid(old);
					if (result)
					{
						int j = 0;

						SecureZeroMemory(old, (size_t)((this->Ecount - 1) * sizeof(LVDISPSTRUCT)));

						for (DWORD i = 0; i < this->Ecount; i++)
						{
							if (i != index)
							{
								old[j].type = this->lvd[i].type;

								result = (CopyStringToPtrA(this->lvd[i].code, &old[j].code) == TRUE) ? TRUE : FALSE;
								if (result)
								{
									SafeDeleteArray(&this->lvd[i].code);

									result = (CopyStringToPtrA(this->lvd[i].definition, &old[j].definition) == TRUE) ? TRUE : FALSE;
									if (result)
									{
										SafeDeleteArray(&this->lvd[i].definition);

										result = (CopyStringToPtrA(this->lvd[i].location, &old[j].location) == TRUE) ? TRUE : FALSE;
										if (result)
										{
											SafeDeleteArray(&this->lvd[i].location);

											// ...
										}
									}
								}
								j++;
							}
							if (!result)
								break;
						}
						SafeDeleteArray(&this->lvd);

						if (result)
						{
							this->lvd = new LVDISPSTRUCT[(size_t)((this->Ecount - 1) * sizeof(LVDISPSTRUCT))];

							result = IsPtrValid(this->lvd);
							if (result)
							{
								SecureZeroMemory(this->lvd, (size_t)(sizeof(LVDISPSTRUCT)*(this->Ecount - 1)));

								for (DWORD i = 0; i < (this->Ecount - 1); i++)
								{
									this->lvd[i].type = old[i].type;

									result = (CopyStringToPtrA(old[i].code, &this->lvd[i].code) == TRUE) ? TRUE : FALSE;
									if (result)
									{
										SafeDeleteArray(&old[i].code);

										result = (CopyStringToPtrA(old[i].definition, &this->lvd[i].definition) == TRUE) ? TRUE : FALSE;
										if (result)
										{
											SafeDeleteArray(&old[i].definition);

											result = (CopyStringToPtrA(old[i].location, &this->lvd[i].location) == TRUE) ? TRUE : FALSE;
											if (result)
											{
												SafeDeleteArray(&old[i].location);

												// ...
											}
										}
									}
									if (!result)
										break;
								}
								SafeDeleteArray(&old);
							}
						}
					}
				}
			}
			if (result)
			{
				ListView_DeleteItem(this->Listview, index);

				if(this->Ecount > 0)
					this->Ecount--;
				if (this->Ecount == 0)
					killthetimer = TRUE;
			}
		}
		if (killthetimer)
		{
			if(KillTimer(this->Main, ID_TIMER_VALIDATEERROR))
				this->TimerActive = FALSE;
		}
	}
	return result;
}

BOOL CnCS_CB::ValidateError(int type, LPCTSTR Code, LPCTSTR Location, DWORD* index_out)
{
	BOOL result = TRUE;

	for (DWORD i = 0; i < this->Ecount; i++)
	{
		if(index_out != NULL)
			*index_out = i;

		result = CompareStringsB(this->lvd[i].code, Code);
		if (result)
		{
			result = CompareStringsB(this->lvd[i].location, Location);
			if (result)
			{
				if (this->lvd[i].type == type)
				{
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

BOOL CnCS_CB::_requestErrorValidity()
{
	BOOL result = TRUE;
	HWND errorsource;

	for (DWORD i = 0; i < this->Ecount; i++)
	{
		errorsource = this->GetErrorSource(this->lvd[i].code);
		if (errorsource)
		{
			EDSPSTRUCT edsp;
			edsp.type = this->lvd[i].type;
			edsp.Code = this->lvd[i].code;
			edsp.Description = this->lvd[i].definition;
			edsp.Location = this->lvd[i].location;

			result = (BOOL) SendMessage(errorsource, WM_VALIDATEERROR, 0, reinterpret_cast<LPARAM>(&edsp));
			if (!result)
			{
				if (this->Ecount == 1)
					this->_remove(this->lvd->type, this->lvd->code, this->lvd->location);
				else if (this->Ecount > 1)
					this->_remove(this->lvd[i].type, this->lvd[i].code, this->lvd[i].location);
				else
					break;

				if (i > 0)i--;
			}
		}
		else
		{
			// remove ??
		}
	}
	return TRUE;
}

HWND CnCS_CB::GetErrorSource(LPTSTR Code)
{
	HWND errorsource = NULL;

	if (Code != NULL)
	{
		int i = 0;

		__try
		{
			while (Code[i] != L'\0')
				i++;
		}
		__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			return (HWND)NULL;
		}
		if (i > 2)
		{
			if ((Code[0] == L'T') && (Code[1] == L'C'))
			{
				return GetDlgItem(this->Main, ID_TABCTRLFRAME);
			}
			else if ((Code[0] == L'U') && (Code[1] == L'A'))
			{
				return this->Main;
			}
			else if ((Code[0] == L'F') && (Code[1] == L'N'))
			{
				return GetDlgItem(this->Main, ID_TVFRAME);
			}
			else if ((Code[0] == L'C') && (Code[1] == L'B'))
			{
				return GetDlgItem(this->Main, ID_CBOXFRAME);
			}
			else if ((Code[0] == L'P') && (Code[1] == L'I'))
			{
				return GetDlgItem(this->Main, ID_PROPERTYWINDOW);
			}
			else
			{
				return this->Main;
			}
		}
	}
	return errorsource;
}

void CnCS_CB::onDPIChanged()
{
	this->_createDpiDependendResources();

	if (this->Listview != nullptr)
		SendMessage(this->Listview, WM_SETFONT, reinterpret_cast<WPARAM>(this->lvFont), static_cast<LPARAM>(TRUE));
}

void CnCS_CB::_createDpiDependendResources()
{
	if (this->lvFont != nullptr)
		DeleteObject(this->lvFont);

	this->lvFont = CreateScaledFont(16, FW_BOLD, APPLICATION_PRIMARY_FONT);
}
