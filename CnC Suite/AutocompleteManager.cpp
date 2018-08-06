#include"AutocompleteManager.h"

//autocompleteManager::autocompleteManager()
//{
//	this->opMode = OPERATION_MODE_DATASTORAGE;
//}

autocompleteManager::autocompleteManager(HINSTANCE hInst, HWND mainWnd)
	:hInstance(hInst),
	MainWnd(mainWnd),
	//selectedRow(-1),
	triggerEdit(nullptr),
	appendixEdit(nullptr),
	previewEdit(nullptr)
{
	//this->opMode = OPERATION_MODE_UIVIEW;
	SendMessage(mainWnd, WM_GETSTYLEINFO, 0, reinterpret_cast<LPARAM>(&this->sInfo));

	this->ctrlFont = CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT);//CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Segoe UI\0");
}

autocompleteManager::~autocompleteManager()
{
	if (this->ctrlFont != nullptr)
	{
		DeleteObject(this->ctrlFont);
	}
}

HRESULT autocompleteManager::Init(LPCTRLCREATIONSTRUCT pcs, autocompleteStrings* pac)
{
	//if (this->opMode == OPERATION_MODE_DATASTORAGE)
	//	return E_NOINTERFACE;

	HRESULT hr =
		((pac != nullptr) && (pcs != nullptr))
		? S_OK : E_FAIL;

	if (SUCCEEDED(hr))
	{
		this->pAcStrings = pac;

		hr = this->registerClass();
		if (SUCCEEDED(hr))
		{
			this->parent = pcs->parent;

			this->acmWnd
				= CreateWindow(
					AUTOCOMPLETEMANAGERCLASS,
					nullptr,
					WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
					pcs->pos.x,
					pcs->pos.y,
					pcs->size.cx,
					pcs->size.cy,
					pcs->parent,
					reinterpret_cast<HMENU>(this->AutoCompWindowID),
					this->hInstance,
					reinterpret_cast<LPVOID>(this)
				);

			hr = (this->acmWnd != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				hr = this->createControls();
				if (SUCCEEDED(hr))
				{
					ShowWindow(this->acmWnd, SW_SHOW);
				}
			}
		}
	}
	return hr;
}

HRESULT autocompleteManager::createControls()
{
	HRESULT hr;
	RECT rc;
	GetClientRect(this->acmWnd, &rc);

	POINT pt;
	pt.x = DPIScale(20);
	pt.y = rc.bottom - DPIScale(35);

	SIZE sz;
	sz.cx = DPIScale(160);
	sz.cy = DPIScale(26);

	auto exitbutton = new CustomButton(this->acmWnd, BUTTONMODE_TEXT, &pt, &sz, CTRLID_EXITBUTTON, this->hInstance);

	hr = (exitbutton != nullptr) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		iString ttp(
			getStringFromResource(UI_GNRL_NAVIGATEBACK)
		);

		exitbutton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
		exitbutton->setAppearance_onlyText(ttp, FALSE);
		exitbutton->setFont(
			CreateScaledFont(18, FW_MEDIUM, APPLICATION_PRIMARY_FONT)
		);
		exitbutton->setTextColor(this->sInfo.TextColor);
		exitbutton->setColors(this->sInfo.mainToolbarColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
		exitbutton->setBorder(TRUE, this->sInfo.OutlineColor);

		hr = exitbutton->Create();
		if (SUCCEEDED(hr))
		{
			pt.x = DPIScale(5);
			pt.y = DPIScale(20);
			sz.cx = DPIScale(28);
			sz.cy = DPIScale(28);

			auto addbutton = new CustomButton(this->acmWnd, BUTTONMODE_ICON, &pt, &sz, CTRLID_ADDBUTTON, this->hInstance);

			hr = (addbutton != nullptr) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				iString addTT(
					getStringFromResource(UI_PROPWND_ADDAUTOCOMPL)
				);

				addbutton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
				addbutton->setAppearance_onlyIcon(IDI_GNRL_ADD_CROSS_GREEN, DPIScale(24));
				addbutton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
				addbutton->setBorder(TRUE, this->sInfo.OutlineColor);
				addbutton->setTooltip(addTT);

				hr = addbutton->Create();
				if (SUCCEEDED(hr))
				{
					pt.x = DPIScale(498);
					pt.y = DPIScale(20);
					sz.cx = DPIScale(28);
					sz.cy = DPIScale(28);

					auto deleteButton = new CustomButton(this->acmWnd, BUTTONMODE_ICON, &pt, &sz, CTRLID_DELETEBUTTON, this->hInstance);

					hr = (deleteButton != nullptr) ? S_OK : E_FAIL;
					if (SUCCEEDED(hr))
					{
						iString delTT(
							getStringFromResource(UI_PROPWND_DELETEAUTOCOMPL)
						);

						deleteButton->setEventListener(dynamic_cast<customButtonEventSink*>(this));
						deleteButton->setAppearance_onlyIcon(IDI_GNRL_DELETE_BIN, DPIScale(24));
						deleteButton->setColors(this->sInfo.TabColor, COLOR_BUTTON_MOUSE_OVER, COLOR_BUTTON_PRESSED);
						deleteButton->setBorder(TRUE, this->sInfo.OutlineColor);
						deleteButton->setTooltip(delTT);

						hr = deleteButton->Create();
						if (SUCCEEDED(hr))
						{
							this->triggerEdit = new singleLineEdit(this->hInstance);

							hr = (this->triggerEdit != nullptr) ? S_OK : E_FAIL;
							if (SUCCEEDED(hr))
							{
								this->triggerEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
								this->triggerEdit->setBorder(true, this->sInfo.TextColor);
								this->triggerEdit->setAdditionalEditStyles(ES_CENTER);
								this->triggerEdit->setEditFontProperties(APPLICATION_PRIMARY_FONT, DPIScale(16), DPIScale(24));
								this->triggerEdit->setDimensions(
									DPIScale(35),
									DPIScale(22),
									DPIScale(150)
								);
								this->triggerEdit->setColors(this->sInfo.TabColor, this->sInfo.Stylecolor, 0, this->sInfo.TextColor);
							
								hr = this->triggerEdit->Create(this->acmWnd, CTRLID_TRIGGEREDIT);
								if (SUCCEEDED(hr))
								{
									this->appendixEdit = new singleLineEdit(this->hInstance);

									hr = (this->appendixEdit != nullptr) ? S_OK : E_FAIL;
									if (SUCCEEDED(hr))
									{
										this->appendixEdit->setEventHandler(dynamic_cast<singleLineEditEventSink*>(this));
										this->appendixEdit->setBorder(true, this->sInfo.TextColor);
										this->appendixEdit->setAdditionalEditStyles(ES_CENTER);
										this->appendixEdit->setEditFontProperties(APPLICATION_PRIMARY_FONT, DPIScale(16), DPIScale(24));
										this->appendixEdit->setDimensions(
											DPIScale(190),
											DPIScale(22),
											DPIScale(150)
										);
										this->appendixEdit->setColors(this->sInfo.TabColor, this->sInfo.Stylecolor, 0, this->sInfo.TextColor);

										hr = this->appendixEdit->Create(this->acmWnd, CTRLID_APPENDIXEDIT);
										if (SUCCEEDED(hr))
										{
											this->previewEdit = new singleLineEdit(this->hInstance);

											hr = (this->previewEdit != nullptr) ? S_OK : E_FAIL;
											if (SUCCEEDED(hr))
											{
												this->previewEdit->setBorder(true, this->sInfo.TextColor);
												this->previewEdit->setAdditionalEditStyles(ES_CENTER | ES_READONLY);
												this->previewEdit->setEditFontProperties(APPLICATION_PRIMARY_FONT, DPIScale(16), DPIScale(24));
												this->previewEdit->setDimensions(
													DPIScale(345),
													DPIScale(22),
													DPIScale(150)
												);
												this->previewEdit->setColors(this->sInfo.TabColor, this->sInfo.Stylecolor, 0, this->sInfo.TextColor);

												hr = this->previewEdit->Create(this->acmWnd, CTRLID_PREVIEWEDIT);
												if (SUCCEEDED(hr))
												{
													CTRLCREATIONSTRUCT ccs;
													ccs.parent = this->acmWnd;
													ccs.pos.x = DPIScale(5);
													ccs.pos.y = DPIScale(60);
													ccs.size.cx = rc.right - DPIScale(25);
													ccs.size.cy = rc.bottom - DPIScale(120);
													ccs.ctrlID = CTRLID_ACLISTVIEW;

													this->acListView = new cListView(this->hInstance);
													
													hr = (this->acListView != nullptr) ? S_OK : E_FAIL;
													if (SUCCEEDED(hr))
													{
														this->acListView->setEventHandler(dynamic_cast<listViewEventSink*>(this));
														this->acListView->setColors(this->sInfo.TabColor, this->sInfo.TextColor);
														iString wi(getStringFromResource(UI_PROPWND_WHENINSERT));
														iString ap(getStringFromResource(UI_PROPWND_APPEND));
														iString pv(getStringFromResource(UI_PROPWND_PREVIEW));

														itemCollection<iString> columns;
														columns.AddItem(wi);
														columns.AddItem(ap);
														columns.AddItem(pv);
														int a = DPIScale(174), b = DPIScale(175);

														itemCollection<int> columnWidths;
														columnWidths.AddItem(a);
														columnWidths.AddItem(b);
														columnWidths.AddItem(a);

														this->acListView->setFont(
															CreateScaledFont(16, FW_NORMAL, APPLICATION_PRIMARY_FONT)
														);
														this->acListView->setColumnDefinitions(3, &columns, &columnWidths, LVCFMT_CENTER);
														this->acListView->setAdditonalListViewStyles(
															LVS_REPORT | WS_BORDER | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
															LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT
														);

														hr = this->acListView->Create(&ccs);
														if (SUCCEEDED(hr))
														{
															if (this->pAcStrings != nullptr)
															{
																LPAUTOCOMPLETESTRINGS pStrings = nullptr;

																listViewItem lvitem(3);
																
																auto existingStrings = this->pAcStrings->getContent(&pStrings);

																if (existingStrings > 0)
																{
																	for (int i = 0; i < existingStrings; i++)
																	{
																		lvitem.setDataInRow(0, pStrings[i].trigger);
																		lvitem.setDataInRow(1, pStrings[i].appendix);

																		iString preview(pStrings[i].trigger);
																		preview.Append(pStrings[i].appendix);

																		lvitem.setDataInRow(2, preview);

																		this->acListView->addItem(lvitem);

																		lvitem.ClearData();
																	}
																	this->acListView->setSelectedRow(0);
																	this->triggerEdit->setContentWithEventBlocker(pStrings[0].trigger);
																	this->appendixEdit->setContentWithEventBlocker(pStrings[0].appendix);
																	this->updatePreviewEdit();
																	this->acListView->setFocus();
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
					}
				}
			}
		}
	}
	return hr;
}

HRESULT autocompleteManager::registerClass()
{
	HRESULT hr = S_OK;
	WNDCLASSEX wcx;

	if (GetClassInfoEx(this->hInstance, AUTOCOMPLETEMANAGERCLASS, &wcx) == 0)
	{
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = sizeof(LONG_PTR);
		wcx.hbrBackground = nullptr;
		wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcx.hInstance = this->hInstance;
		wcx.lpfnWndProc = autocompleteManager::acmProc;
		wcx.hIcon = nullptr;
		wcx.hIconSm = nullptr;
		wcx.lpszClassName = AUTOCOMPLETEMANAGERCLASS;
		wcx.lpszMenuName = nullptr;

		hr = (RegisterClassEx(&wcx) == 0) ? E_FAIL : S_OK;
	}
	return hr;
}

LRESULT autocompleteManager::acmProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	autocompleteManager* acm = nullptr;

	if (message == WM_CREATE)
	{
		auto pcr = reinterpret_cast<LPCREATESTRUCT>(lParam);
		acm = reinterpret_cast<autocompleteManager*>(pcr->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(acm));
		return 1;
	}
	else
	{
		acm = reinterpret_cast<autocompleteManager*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (acm != nullptr)
		{
			switch (message)
			{
			case WM_PAINT:
				return acm->onPaint(hWnd);
			case WM_ERASEBKGND:
				return static_cast<LRESULT>(TRUE);
			case WM_DESTROY:
				PostMessage(acm->MainWnd, WM_INTERNALCOMMAND, MAKEWPARAM(ICOMMAND_AUTOCOMPLETE_DATA_CHANGED, 0), 0);
				SafeRelease(&acm);
				return 0;
			default:
				break;
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

LRESULT autocompleteManager::onPaint(HWND hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	HDC hdc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(hWnd, &ps);
	if (hdc)
	{
		HBRUSH brush = CreateSolidBrush(this->sInfo.TabColor);
		if (brush)
		{
			FillRect(hdc, &rc, brush);
			SIZE sz1, sz2, sz3;

			iString whenE(getStringFromResource(UI_PROPWND_WHENINSERT));
			iString append(getStringFromResource(UI_PROPWND_APPEND));
			iString preview(getStringFromResource(UI_PROPWND_PREVIEW));

			HGDIOBJ original = SelectObject(hdc, this->ctrlFont);
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, this->sInfo.TextColor);

			// out on: (center) 140; 350; 560;

			GetTextExtentPoint32(hdc, whenE.GetData(), whenE.GetLength(), &sz1);
			GetTextExtentPoint32(hdc, append.GetData(), append.GetLength(), &sz2);
			GetTextExtentPoint32(hdc, preview.GetData(), preview.GetLength(), &sz3);

			TextOut(
				hdc,
				(DPIScale(110) - (sz1.cx / 2)),
				DPIScale(1),
				whenE.GetData(),
				whenE.GetLength()
			);
			TextOut(
				hdc,
				(DPIScale(265) - (sz2.cx / 2)),
				DPIScale(1),
				append.GetData(),
				append.GetLength()
			);
			TextOut(
				hdc,
				(DPIScale(420) - (sz3.cx / 2)),
				DPIScale(1),
				preview.GetData(),
				preview.GetLength()
			);
			SelectObject(hdc, original);
			DeleteObject(brush);
		}
		EndPaint(hWnd, &ps);
	}
	return static_cast<LRESULT>(0);
}

void autocompleteManager::beginNewAddSession()
{
	listViewItem item_(3);
	item_.setDataInRow(0, L".");
	item_.setDataInRow(1, L"..");
	item_.setDataInRow(2, L"...");
	this->acListView->addItem(item_);
	this->acListView->setSelectedRow(LV_SELECTHIGHEST);
	this->pAcStrings->add(L".", L"..", 1);

	this->triggerEdit->setContentWithEventBlocker(L".");
	this->appendixEdit->setContentWithEventBlocker(L"..");
	this->previewEdit->setContentWithEventBlocker(L"...");

	this->triggerEdit->setFocus();
}

void autocompleteManager::beginNewAddSessionWithContent(int type, const TCHAR * content)
{
	listViewItem item_(3);

	if(type == TYPE_TRIGGERINPUT)
		item_.setDataInRow(0, content);
	else
		item_.setDataInRow(0, L".");

	if(type == TYPE_APPENDIXINPUT)
		item_.setDataInRow(1, content);
	else
		item_.setDataInRow(1, L"..");

	iString preview(
		item_.getDataInRow(0)
	);
	preview += item_.getDataInRow(1);

	item_.setDataInRow(2, preview);

	this->acListView->addItem(item_);
	this->acListView->setSelectedRow(LV_SELECTHIGHEST);

	this->pAcStrings->add(
		item_.getDataInRow(0)
		.GetData(),
		item_.getDataInRow(1)
		.GetData(),
		item_.getDataInRow(0)
		.GetLength()
	);

	this->triggerEdit->setContentWithEventBlocker(
		item_.getDataInRow(0)
	);
	this->appendixEdit->setContentWithEventBlocker(
		item_.getDataInRow(1)
	);

	this->updatePreviewEdit();
}

void autocompleteManager::onButtonClick(CustomButton * sender, CTRLID ctrlID)
{
	UNREFERENCED_PARAMETER(sender);

	switch (ctrlID)
	{
	case CTRLID_EXITBUTTON:
		PostMessage(this->parent, WM_COMMAND, MAKEWPARAM(ICOMMAND_AUTOCOMPLETEMANAGER_CLOSED, 0), 0);
		DestroyWindow(this->acmWnd);
		break;
	case CTRLID_ADDBUTTON:	
		this->beginNewAddSession();	
		break;
	case CTRLID_DELETEBUTTON:
	{
		auto selIndex = this->acListView->getSelectedRowIndex();
		if (selIndex != -1)
		{
			this->pAcStrings->deleteAt(selIndex);
			this->acListView->deleteSelection();

			this->triggerEdit->setContentWithEventBlocker(L"");
			this->appendixEdit->setContentWithEventBlocker(L"");
			this->previewEdit->setContentWithEventBlocker(L"");


			//this->acListView->setSelectedRow(0);

			// empty editfield and select new item
			// if there are no items, invalidate button!!!!!!!!!!!!!!!!
		}
	}
		break;
	default:
		break;
	}
}

void autocompleteManager::onEditContentChanged(singleLineEdit * edit, CTRLID ctrlID)
{
	iString prev;

	switch (ctrlID)
	{
	case CTRLID_TRIGGEREDIT:
		{
			if (this->acListView->getSelectedRowIndex() == -1)
			{
				auto input = edit->getContent();
				this->beginNewAddSessionWithContent(TYPE_TRIGGERINPUT, input.GetData());
				return;
			}

			prev.Replace(edit->getContent());

			auto apndEdit = GetDlgItem(this->acmWnd, CTRLID_APPENDIXEDIT);
			if (apndEdit != nullptr)
			{
				auto eClass = reinterpret_cast<singleLineEdit*>(GetWindowLongPtr(apndEdit, GWLP_USERDATA));
				if (eClass != nullptr)
				{
					auto content = eClass->getContent();
					prev.Append(content);

					auto prevEdit = GetDlgItem(this->acmWnd, CTRLID_PREVIEWEDIT);
					if (prevEdit != nullptr)
					{
						auto pClass = reinterpret_cast<singleLineEdit*>(GetWindowLongPtr(prevEdit, GWLP_USERDATA));
						if (pClass != nullptr)
						{
							pClass->setContent(prev);
						}
					}

					LPAUTOCOMPLETESTRINGS pStr = nullptr;
					auto curText = edit->getContent();

					// save content
					int selectedIndex = this->acListView->getSelectedRowIndex();
					this->pAcStrings->updateAt(selectedIndex, curText.GetData(), nullptr, curText.GetLength());

					// set content in list
					int count = this->pAcStrings->getContent(&pStr);
					_NOT_USED(count);

					listViewItem _item_(3);
					_item_.setDataInRow(0, pStr[selectedIndex].trigger);
					_item_.setDataInRow(1, pStr[selectedIndex].appendix);
					_item_.setDataInRow(2, prev);

					this->acListView->updateItem(selectedIndex, _item_);

					SafeDeleteArray(&pStr);
				}
			}
		}
		break;
	case CTRLID_APPENDIXEDIT:
		{
			if (this->acListView->getSelectedRowIndex() == -1)
			{
				auto input = edit->getContent();
				this->beginNewAddSessionWithContent(TYPE_APPENDIXINPUT, input.GetData());
				return;
			}

			auto trggrEdit = GetDlgItem(this->acmWnd, CTRLID_TRIGGEREDIT);
			if (trggrEdit != nullptr)
			{
				auto eClass = reinterpret_cast<singleLineEdit*>(GetWindowLongPtr(trggrEdit, GWLP_USERDATA));
				if (eClass != nullptr)
				{
					prev.Append(eClass->getContent());
					prev.Append(edit->getContent());

					auto prevEdit = GetDlgItem(this->acmWnd, CTRLID_PREVIEWEDIT);
					if (prevEdit != nullptr)
					{
						auto pClass = reinterpret_cast<singleLineEdit*>(GetWindowLongPtr(prevEdit, GWLP_USERDATA));
						if (pClass != nullptr)
						{
							pClass->setContent(prev);
						}
					}

					LPAUTOCOMPLETESTRINGS pStr = nullptr;
					auto curText = edit->getContent();

					// save content
					int selectedIndex = this->acListView->getSelectedRowIndex();
					this->pAcStrings->updateAt(selectedIndex, nullptr, curText.GetData(), 0);

					// set content in list
					int count = this->pAcStrings->getContent(&pStr);
					_NOT_USED(count);

					listViewItem _item_(3);
					_item_.setDataInRow(0, pStr[selectedIndex].trigger);
					_item_.setDataInRow(1, pStr[selectedIndex].appendix);
					_item_.setDataInRow(2, prev);

					this->acListView->updateItem(selectedIndex, _item_);

					SafeDeleteArray(&pStr);
				}
			}
		}
		break;
	default:
		break;
	}
}

void autocompleteManager::onListviewSelectionChanged(cListView * view, int selectedIndex)
{
	UNREFERENCED_PARAMETER(view);

	if (selectedIndex >= 0)
	{
		auto trigger = this->pAcStrings->getTriggerAt(selectedIndex);
		auto appendix = this->pAcStrings->getAppendixAt(selectedIndex);
		auto preview = trigger + appendix;

		this->triggerEdit->setContentWithEventBlocker(trigger);
		this->appendixEdit->setContentWithEventBlocker(appendix);
		this->previewEdit->setContentWithEventBlocker(preview);
	}
	else
	{
		this->triggerEdit->setContentWithEventBlocker(L"");
		this->appendixEdit->setContentWithEventBlocker(L"");
		this->previewEdit->setContentWithEventBlocker(L"");
	}
}

void autocompleteManager::updatePreviewEdit()
{
	auto trig = this->triggerEdit->getContent();
	auto apen = this->appendixEdit->getContent();
	auto prev = trig + apen;

	this->previewEdit->setContentWithEventBlocker(prev);
}

