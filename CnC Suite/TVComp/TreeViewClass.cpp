#include"TreeViewClass.h"
#include"..//CommonControls/StringClass.h"
#include"..//BasicFPO.h"
#include"..//IPath.h"
#include"..//Error dispatcher.h"
#include"..//ApplicationData.h"
#include"..//CnC3FileManager.h"

TreeViewCTRL::TreeViewCTRL(HWND TVFrame_, HINSTANCE hInst, int language, LPTSTR Working_Directory)
	: hInstance(hInst),
	TVFrame(TVFrame_),
	Root_Folder(nullptr),
	Root_Name(nullptr),
	hwndTV(nullptr),
	Working_Dir(nullptr),
	language_(language),
	cSuccess(TRUE),
	disableFOPConfirmation(FALSE),
	DragUnderway(FALSE),
	useInfoTips(FALSE),
	openNewFile(FALSE),
	folderCopyInProgress(FALSE),
	internalFileOperationInProgress(FALSE),
	moveOperationInProgress(FALSE),
	CursorScrollTimerActivated(FALSE),
	ScrollDirection(0),
	iTVProgress(nullptr),
	iTVUserEvents(nullptr),
	temporaryPtr(nullptr),
	tvFont(nullptr),
	_cRef(1)
{
	SecureZeroMemory(&this->FOPInfo, sizeof(FOPCLIPBOARDINFO));
	SecureZeroMemory(&this->DynamicLevel, sizeof(HDYNAMICLEVEL));

	HRESULT hr = (TVFrame_ == NULL) ? E_FAIL : S_OK;
	if (SUCCEEDED(hr))
	{
		size_t len;

		hr = StringCbLength(Working_Directory, STRSAFE_MAX_CCH, &len);
		if (SUCCEEDED(hr))
		{

			this->Working_Dir = new TCHAR[len + sizeof(TCHAR)];

			hr = (this->Working_Dir == NULL) ? E_FAIL : S_OK;
			if (SUCCEEDED(hr))
			{
				hr = StringCbCopy(this->Working_Dir, len + sizeof(TCHAR), Working_Directory);
				if (SUCCEEDED(hr))
				{
					// ...

				}
			}
		}
	}
	this->Obj.cnc3 = (HCURSOR)LoadImage(this->hInstance, MAKEINTRESOURCE(IDC_CNC3CURSOR), IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
	this->Obj.file = (HCURSOR)LoadImage(this->hInstance, MAKEINTRESOURCE(IDC_FILECURSOR), IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
	this->Obj.folder = (HCURSOR)LoadImage(this->hInstance, MAKEINTRESOURCE(IDC_FOLDERCURSOR), IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
	this->Obj.forbidden = (HCURSOR)LoadImage(this->hInstance, MAKEINTRESOURCE(IDC_FORBIDDEN), IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
	this->Obj.insert = (HCURSOR)LoadImage(this->hInstance, MAKEINTRESOURCE(IDC_INSERT), IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);

	if (FAILED(hr))cSuccess = FALSE;
}

TreeViewCTRL::~TreeViewCTRL( void )
{
	DestroyCursor(this->Obj.cnc3);
	DestroyCursor(this->Obj.file);
	DestroyCursor(this->Obj.folder);
	DestroyCursor(this->Obj.forbidden);

	if(this->tvFont != nullptr)
		DeleteObject(this->tvFont);

	if( !this->SaveRoot() )
	{
		//this->DisplayErrorBox( L"SaveRoot" );// disable ??
	}

	if( this->Root_Folder != NULL )
	{
		delete [] this->Root_Folder;
		this->Root_Folder = NULL;
	}
	if( this->Working_Dir != NULL )
	{
		delete [] this->Working_Dir;
		this->Working_Dir = NULL;
	}
	if( this->Root_Name != NULL )
	{
		delete [] this->Root_Name;
		this->Root_Name = NULL;
	}
}

HRESULT TreeViewCTRL::InitTreeView( COLORREF BkColor, HFONT Clientfont, COLORREF Textcolor, LPRECT rc, DWORD additionalStyles )
{
	HRESULT hr = (this->cSuccess) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		InitCommonControls();

		this->hwndTV = CreateWindowEx(
			WS_EX_COMPOSITED | WS_EX_TRANSPARENT,
			WC_TREEVIEW,
			L"Tree View",
			WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_TRACKSELECT | TVS_EDITLABELS | additionalStyles,
			rc->left, rc->top,
			rc->right, rc->bottom,
			this->TVFrame,
			reinterpret_cast<HMENU>(ID_FILENAVIGATOR),
			this->hInstance, NULL);

		hr = (this->hwndTV == nullptr) ? E_FAIL : S_OK;
		if(SUCCEEDED(hr))
		{
			this->tvFont = Clientfont;
			SendMessage(this->hwndTV, WM_SETFONT, reinterpret_cast<WPARAM>(Clientfont), static_cast<LPARAM>(TRUE));

			TreeView_SetBkColor(this->hwndTV, BkColor);
			TreeView_SetTextColor(this->hwndTV, Textcolor);

			hr = this->InitTreeViewImageLists(this->hwndTV) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				hr = this->LoadRoot() ? S_OK : E_FAIL;
				if(FAILED(hr))
				{
					hr = this->GenerateRoot();
				}
				if (SUCCEEDED(hr))
				{
					hr = this->InitTreeViewItems(this->Root_Folder, INIT_MODE) ? S_OK : E_FAIL;
					if (SUCCEEDED(hr))
					{
						this->threadID = GetCurrentThreadId();
					}
				}
			}
		}
	}
	return hr;
}

BOOL TreeViewCTRL::InitTreeViewImageLists( HWND TV )
{
	BOOL result = TRUE;
	HIMAGELIST	himl; 
	HICON		hicon;

	result = 
		((himl =
			ImageList_Create(
				DPIScale(CX_ICON),
				DPIScale(CY_ICON),
				ILC_COLOR32,
				NUM_ICONS, 0)
			)
			== NULL)
		? FALSE : TRUE;
	if(result)
	{
		hicon = (HICON)LoadImage(
			this->hInstance,
			MAKEINTRESOURCE(IDI_TV_FILE),
			IMAGE_ICON,
			DPIScale(24),
			DPIScale(24),
			LR_DEFAULTCOLOR
		);

		result = (hicon) ? TRUE : FALSE;
		if (result)
		{
			this->ico_index[0] = ImageList_AddIcon(himl, hicon);
			DestroyIcon(hicon);

			hicon = (HICON)LoadImage(
				this->hInstance,
				MAKEINTRESOURCE(IDI_TV_FOLDERCLOSED),
				IMAGE_ICON,
				DPIScale(24),
				DPIScale(24),
				LR_DEFAULTCOLOR
			);

			result = (hicon) ? TRUE : FALSE;
			if (result)
			{
				this->ico_index[1] = ImageList_AddIcon(himl, hicon);
				DestroyIcon(hicon);

				hicon = (HICON)LoadImage(
					this->hInstance,
					MAKEINTRESOURCE(IDI_TV_FOLDEROPENED),
					IMAGE_ICON,
					DPIScale(24),
					DPIScale(24),
					LR_DEFAULTCOLOR
				);

				result = (hicon) ? TRUE : FALSE;
				if (result)
				{
					this->ico_index[2] = ImageList_AddIcon(himl, hicon);
					DestroyIcon(hicon);

					hicon = (HICON)LoadImage(
						this->hInstance,
						MAKEINTRESOURCE(IDI_TV_CNC3),
						IMAGE_ICON,
						DPIScale(24),
						DPIScale(24),
						LR_DEFAULTCOLOR
					);

					result = (hicon) ? TRUE : FALSE;
					if (result)
					{
						this->ico_index[3] = ImageList_AddIcon(himl, hicon);
						DestroyIcon(hicon);

						hicon = (HICON)LoadImage(
							this->hInstance,
							MAKEINTRESOURCE(IDI_TV_NOCONTENT),
							IMAGE_ICON,
							DPIScale(24),
							DPIScale(24),
							LR_DEFAULTCOLOR
						);

						result = (hicon) ? TRUE : FALSE;
						if (result)
						{
							this->ico_index[4] = ImageList_AddIcon(himl, hicon);
							DestroyIcon(hicon);

							result = (ImageList_GetImageCount(himl) < 5) ? FALSE : TRUE;
							if (result)
							{
								TreeView_SetImageList(TV, himl, TVSIL_NORMAL);
							}
						}
					}
				}
			}
		}
	}
	return result;
}

BOOL TreeViewCTRL::InitTreeViewItems( LPCTSTR Root, int Init_Mode )
{
	if (Root == nullptr)
		return FALSE;

	if (this->iTVProgress != nullptr)
	{
		this->iTVProgress->dataCollectionStarted();
	}

	int C_Items = 0, count = 1, type = 0, level = 1;

	HRESULT hr;
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	hr = TreeView_DeleteAllItems(this->hwndTV) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		// ROOT SUB
		size_t buffer_len, fName_len;

		hr = StringCbLength(Root, STRSAFE_MAX_CCH *sizeof(TCHAR), &buffer_len);
		if (SUCCEEDED(hr))
		{
			if (Init_Mode != INIT_MODE)
			{
				if (this->Root_Folder != nullptr)
				{
					delete[] this->Root_Folder;
					this->Root_Folder = nullptr;
				}
				this->Root_Folder = new TCHAR[buffer_len + sizeof(TCHAR)];

				hr = (this->Root_Folder != nullptr) ? S_OK : E_OUTOFMEMORY;
				if (SUCCEEDED(hr))
				{
					hr = StringCbCopy(this->Root_Folder, buffer_len + sizeof(TCHAR), Root);// Save root folder.
					if (FAILED(hr))
					{
						return FALSE;
					}
				}
			}
			TCHAR* path_buffer = new TCHAR[buffer_len + 15];

			hr = (path_buffer != nullptr) ? S_OK : E_OUTOFMEMORY;
			if (SUCCEEDED(hr))
			{
				hr = StringCbPrintf(path_buffer, sizeof(TCHAR) * (buffer_len + 15), L"\\\\?\\%s\\*", Root);// e
				if (SUCCEEDED(hr))
				{
					C_Items = this->CountItems(Root);
					if (C_Items < 0)
					{
						if (C_Items == -1)
						{
							hr = E_FAIL;
							MessageBox(this->TVFrame, L"Folder loading failed. Position tv-#006.", L"Internal error", MB_OK | MB_ICONERROR);
						}
						else
						{
							hr = E_UNEXPECTED;
							MessageBox(this->TVFrame, L"Folder loading failed. Unknown error occured.\nPosition tv-#007.", L"Internal error", MB_OK | MB_ICONERROR);
						}
					}
					else
					{
						if (this->iTVProgress != nullptr)
						{
							this->iTVProgress->dataCollectionFinished(C_Items);
							this->iTVProgress->startInitializingItems(C_Items);
						}

						LPHEADING items = new HEADING[C_Items + 1];

						int max_items = C_Items + 1;

						hr = (items == nullptr) ? E_OUTOFMEMORY : S_OK;
						if (SUCCEEDED(hr))
						{
							items[0].Level = 0;
							items[0].type = ROOT_DIR;
							this->LeachRootName(Root, items[0].Heading);

							if (this->Root_Name != nullptr)
							{
								delete [] this->Root_Name;
								this->Root_Name = nullptr;
							}
							this->Root_Name = new TCHAR[MAX_HEADING_LEN];

							hr = (this->Root_Name == nullptr) ? E_OUTOFMEMORY : S_OK;
							if (SUCCEEDED(hr))
							{
								hr = StringCbCopy(this->Root_Name, sizeof(TCHAR) * MAX_HEADING_LEN, items[0].Heading);
								if (SUCCEEDED(hr))
								{
									hFind = FindFirstFile(path_buffer, &ffd);

									auto lastError = GetLastError();

									if ((hFind == INVALID_HANDLE_VALUE) && ((lastError == ERROR_FILE_NOT_FOUND) || (lastError == ERROR_PATH_NOT_FOUND))) {

										// root directory path is empty or invalid, this is not an error
										hr = S_OK;
									}
									else
									{
										hr = (hFind == INVALID_HANDLE_VALUE) ? E_HANDLE : S_OK;
										if (SUCCEEDED(hr))
										{
											while (ffd.cFileName[0] == '.')
											{
												if (FindNextFile(hFind, &ffd) == 0)
												{
													type = EMPTY_DIR;
													break;
												}
											}

											do// SUB
											{
												if (type == EMPTY_DIR)// root directory is empty...
												{
													type = 0;

													// set empty child item
													items[count].Level = 1;//level + 1;
													items[count].type = A__EMPTYITEM;
													StringCbCopy(items[count].Heading, sizeof(TCHAR) * MAX_HEADING_LEN, L"< . . . >\0");

													count++;
													this->stepProgressIfApplicable();

													break;
												}
												if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
												{
													if (count == (C_Items + 1))
														break;

													items[count].Level = level;
													items[count].type = A__FOLDER;

													hr = StringCbCopy(items[count].Heading, sizeof(TCHAR) * MAX_HEADING_LEN, ffd.cFileName);
													if (SUCCEEDED(hr))
													{
														count++;
														this->stepProgressIfApplicable();

														WIN32_FIND_DATA ffd_sub;
														HANDLE hFind_sub = INVALID_HANDLE_VALUE;

														hr = StringCbLength(ffd.cFileName, STRSAFE_MAX_CCH * sizeof(TCHAR), &fName_len);
														if (SUCCEEDED(hr))
														{
															TCHAR* subitem1 = new TCHAR[(buffer_len + fName_len + 1)];

															hr = (subitem1 == nullptr) ? E_OUTOFMEMORY : S_OK;
															if (SUCCEEDED(hr))
															{
																hr = StringCbPrintf(subitem1, sizeof(TCHAR) * (buffer_len + fName_len + 1), L"\\\\?\\%s\\%s\\*", Root, ffd.cFileName);
																if (SUCCEEDED(hr))
																{
																	hFind_sub = FindFirstFile(subitem1, &ffd_sub);

																	hr = (hFind_sub == INVALID_HANDLE_VALUE) ? E_HANDLE : S_OK;
																	if (SUCCEEDED(hr))
																	{
																		while (ffd_sub.cFileName[0] == '.')
																		{
																			if (FindNextFile(hFind_sub, &ffd_sub) == 0)
																			{
																				type = EMPTY_DIR;
																				break;
																			}
																		}
																		if (type != EMPTY_DIR)
																		{
																			// next level ....
																			hr = this->InitNextLevelItems(hFind_sub, &ffd_sub, items, subitem1, type, level + 1, count);
																		}
																		else
																		{
																			if (count >= max_items)		// C6385 suppressor
																			{
																				OutputDebugString(L"CRITICAL EVENT::TREEVIEWCLASS::INITTREEVIEWITEMS::MAXITEMS REACHED - POSITION 2");
																				break;
																			}
																			else
																			{
																				type = 0;
																				// set empty child item
																				items[count].Level = level + 1;
																				items[count].type = A__EMPTYITEM;
																				StringCbCopy(items[count].Heading, sizeof(TCHAR) * MAX_HEADING_LEN, L"< . . . >\0");

																				count++;
																				this->stepProgressIfApplicable();
																			}
																		}
																		FindClose(hFind_sub);
																	}
																}
																delete[] subitem1;
															}
														}
													}
												}
												else
												{
													if (count >= max_items)		// C6385 suppressor
													{
														OutputDebugString(L"CRITICAL EVENT::TREEVIEWCLASS::INITTREEVIEWITEMS::MAXITEMS REACHED - POSITION 1");
														break;
													}
													else
													{
														items[count].Level = level;
														items[count].type = A__FILE;
														StringCbCopy(items[count].Heading, sizeof(TCHAR) * MAX_HEADING_LEN, ffd.cFileName);

														count++;
														this->stepProgressIfApplicable();
													}
												}
											} while (FindNextFile(hFind, &ffd) != 0);

											FindClose(hFind);

											LPHEADING a_items = new HEADING[count];

											hr = (a_items != NULL) ? S_OK : E_OUTOFMEMORY;
											if (SUCCEEDED(hr))
											{
												if (this->iTVUserEvents != nullptr)
												{
													this->enableSelchangeNotification(false);
												}

												hr = this->Sequencing(SQC_ROOT, a_items, items, count) ? S_OK : E_FAIL;
												if (SUCCEEDED(hr))
												{
													HTREEITEM hti;

													for (int k = 0; k < count; k++)
													{
														hti = this->AddItemToTree(this->hwndTV, a_items[k].Heading, a_items[k].Level, a_items[k].type);

														this->stepProgressIfApplicable();

														if (!hti)
														{
															hr = E_FAIL;
															break;
														}
													}
													if (SUCCEEDED(hr))
													{
														TreeView_Expand(this->hwndTV, TreeView_GetRoot(this->hwndTV), TVE_EXPAND);
														TreeView_SelectItem(this->hwndTV, TreeView_GetRoot(this->hwndTV));
													}
												}

												if (this->temporaryPtr != nullptr)
												{
													this->enableSelchangeNotification(true);
												}

												delete[] a_items;
											}
										}
									}
								}
							}
							delete[] items;
						}
					}								
				}
				delete[] path_buffer;
			}
		}
	}
	return SUCCEEDED(hr) ? TRUE : FALSE;
}

void TreeViewCTRL::InitTreeViewItemsAsync(LPCTSTR root)
{
	if (root != nullptr)
	{
		DWORD threadId;

		TVTHREADDATA tdata;
		tdata.toClass = reinterpret_cast<LONG_PTR>(this);
		tdata.root = root;
		tdata.mode = 0;

		HANDLE hThread =
			CreateThread(
				nullptr,
				0,
				TreeViewCTRL::initAsycProc,
				reinterpret_cast<LPVOID>(&tdata),
				0,
				&threadId
			);

		if (hThread != nullptr)
		{
			WaitForSingleObject(hThread, 50);
			CloseHandle(hThread);
		}
	}
}

HRESULT TreeViewCTRL::InitNextLevelItems(HANDLE hFind_sub, LPWIN32_FIND_DATA ffd_sub, LPHEADING items, TCHAR* subitem, int type, int level, int& count)
{
	HRESULT hr = S_OK;
	size_t buffer_len, fName_len;

	do
	{
		if (ffd_sub->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			items[count].Level = level;
			items[count].type = A__FOLDER;

			hr = StringCbCopy(items[count].Heading, sizeof(TCHAR) * MAX_HEADING_LEN, ffd_sub->cFileName);
			if (SUCCEEDED(hr))
			{
				count++;
				this->stepProgressIfApplicable();

				WIN32_FIND_DATA ffd_sub_new;
				HANDLE hFind_new = INVALID_HANDLE_VALUE;

				hr = this->RemoveWildcard(subitem) ? S_OK : E_FAIL;
				if (SUCCEEDED(hr))
				{
					hr = StringCbLength(subitem, STRSAFE_MAX_CCH * sizeof(TCHAR), &buffer_len);
					if (SUCCEEDED(hr))
					{
						hr = StringCbLength(ffd_sub->cFileName, STRSAFE_MAX_CCH * sizeof(TCHAR), &fName_len);
						if (SUCCEEDED(hr))
						{
							TCHAR* subitem_new = new TCHAR[(buffer_len + fName_len + 1)];

							hr = (subitem_new == nullptr) ? E_OUTOFMEMORY : S_OK;
							if (SUCCEEDED(hr))
							{
								hr = StringCbPrintf(subitem_new, sizeof(TCHAR) * (buffer_len + fName_len + 1),
									L"%s\\%s\\*",
									subitem,
									ffd_sub->cFileName);
								if (SUCCEEDED(hr))
								{

									hFind_new = FindFirstFile(subitem_new, &ffd_sub_new);

									hr = (hFind_new == INVALID_HANDLE_VALUE) ? E_HANDLE : S_OK;
									if (SUCCEEDED(hr))
									{
										while (ffd_sub_new.cFileName[0] == '.')
										{
											if (FindNextFile(hFind_new, &ffd_sub_new) == 0)
											{
												type = EMPTY_DIR;
												break;
											}
										}
										if (type != EMPTY_DIR)
										{
											// next level ....
											hr = this->InitNextLevelItems(hFind_new, &ffd_sub_new, items, subitem_new, type, level + 1, count);
										}
										else
										{
											type = 0;
											// Add empty item ...
											items[count].Level = level + 1;
											items[count].type = A__EMPTYITEM;
											StringCbCopy(items[count].Heading, sizeof(TCHAR)*MAX_HEADING_LEN, L"< . . . >\0");

											count++;
											this->stepProgressIfApplicable();
										}
										FindClose(hFind_new);
									}
								}
								delete[] subitem_new;
							}
						}
					}
				}
			}
		}
		else
		{
			items[count].Level = level;
			items[count].type = A__FILE;
			hr = StringCbCopy(items[count].Heading, sizeof(TCHAR) * MAX_HEADING_LEN, ffd_sub->cFileName);

			count++;
			this->stepProgressIfApplicable();
		}
	} while (FindNextFile(hFind_sub, ffd_sub) != 0);

	return hr;
}

int TreeViewCTRL::CountItems( LPCTSTR Root_Dir )
{
	int items = 0;

	if (Root_Dir == nullptr)
		return -1;
	else
	{
		HRESULT hr = S_OK;

		int type = 0;

		size_t buffer_len, fName_len;

		WIN32_FIND_DATA ffd_count;
		HANDLE hFind_c = INVALID_HANDLE_VALUE;

		// ROOT - SUB0
		hr = StringCbLength(Root_Dir, STRSAFE_MAX_CCH *sizeof(TCHAR), &buffer_len);
		if (SUCCEEDED(hr))
		{
			TCHAR* path_buffer = new TCHAR[buffer_len + 15];

			hr = (path_buffer == nullptr) ? E_OUTOFMEMORY : S_OK;
			if (SUCCEEDED(hr))
			{
				hr = StringCbPrintf(path_buffer, sizeof(TCHAR) * (buffer_len + 15), L"\\\\?\\%s\\*", Root_Dir);
				if (SUCCEEDED(hr))
				{
					hFind_c = FindFirstFile(path_buffer, &ffd_count);

					hr = (hFind_c == INVALID_HANDLE_VALUE) ? E_HANDLE : S_OK;
					if(SUCCEEDED(hr))					
					{
						while (ffd_count.cFileName[0] == '.')
						{
							if (FindNextFile(hFind_c, &ffd_count) == 0)
							{
								type = EMPTY_DIR;
								break;
							}
						}
						do// SUB
						{
							if (type == EMPTY_DIR)
							{
								type = 0;
								items++;
							}
							if (ffd_count.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							{
								WIN32_FIND_DATA ffd_count_sub;
								HANDLE hFind_sub = INVALID_HANDLE_VALUE;

								hr = StringCbLength(path_buffer, STRSAFE_MAX_CCH *sizeof(TCHAR), &buffer_len);
								if (SUCCEEDED(hr))
								{
									hr = StringCbLength(ffd_count.cFileName, STRSAFE_MAX_CCH *sizeof(TCHAR), &fName_len);
									if (SUCCEEDED(hr))
									{
										TCHAR* subitem = new TCHAR[(buffer_len + fName_len)];

										hr = (subitem == nullptr) ? E_OUTOFMEMORY : S_OK;
										if (SUCCEEDED(hr))
										{
											hr = StringCbPrintf(subitem, sizeof(TCHAR) * (buffer_len + fName_len), L"\\\\?\\%s\\%s\\*", Root_Dir, ffd_count.cFileName);
											if (SUCCEEDED(hr))
											{
												hFind_sub = FindFirstFile(subitem, &ffd_count_sub);
												hr = (hFind_sub == INVALID_HANDLE_VALUE) ? E_HANDLE : S_OK;
												if (SUCCEEDED(hr))
												{
													//showText(L"count items 1 ... ?");

													while (ffd_count_sub.cFileName[0] == '.')
													{
														if (FindNextFile(hFind_sub, &ffd_count_sub) == 0)
														{
															type = EMPTY_DIR;
															break;
														}
													}
													if (type != EMPTY_DIR)
													{
														// next level ...
														hr = this->CountItemsNextLevel(hFind_sub, &ffd_count_sub, subitem, items);

														//items++;
													}
													else
													{
														// count one for the empty item ...
														items++;
													}
													FindClose(hFind_sub);
												}
											}
											delete[] subitem;
										}
									}
								}
								items++;
							}
							else
							{
								items++;
							}
						} while (FindNextFile(hFind_c, &ffd_count) != 0);

						FindClose(hFind_c);
					}					
				}
				delete[] path_buffer;
			}
		}
		//if (FAILED(hr))
		//	items = -1;
	}	
	return items;
}

HRESULT TreeViewCTRL::CountItemsNextLevel(HANDLE hFind_sub, LPWIN32_FIND_DATA ffd_count_sub, TCHAR* subitem, int &items)
{
	HRESULT hr = S_OK;
	size_t buffer_len, fName_len;

	do
	{
		if (ffd_count_sub->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			WIN32_FIND_DATA ffd_count_sub_new;
			HANDLE hFind_sub_new = INVALID_HANDLE_VALUE;

			hr = StringCbLength(subitem, STRSAFE_MAX_CCH *sizeof(TCHAR), &buffer_len);
			if (SUCCEEDED(hr))
			{
				hr = StringCbLength(ffd_count_sub->cFileName, STRSAFE_MAX_CCH *sizeof(TCHAR), &fName_len);
				if (SUCCEEDED(hr))
				{
					hr = this->RemoveWildcard(subitem) ? S_OK : E_FAIL;
					if (SUCCEEDED(hr))
					{
						TCHAR* subitem_new = new TCHAR[(buffer_len + fName_len)];

						hr = (subitem_new == nullptr) ? E_FAIL : S_OK;
						if (SUCCEEDED(hr))
						{
							hr = StringCbPrintf(subitem_new, sizeof(TCHAR) * (buffer_len + fName_len),
									L"%s\\%s\\*",
									subitem,
									ffd_count_sub->cFileName);
							if (SUCCEEDED(hr))
							{
								hFind_sub_new = FindFirstFile(subitem_new, &ffd_count_sub_new);

								hr = (hFind_sub_new == INVALID_HANDLE_VALUE) ? E_HANDLE : S_OK;
								if (SUCCEEDED(hr))
								{
									int type = 0;

									while (ffd_count_sub_new.cFileName[0] == '.')
									{
										if (FindNextFile(hFind_sub_new, &ffd_count_sub_new) == 0)
										{
											type = EMPTY_DIR;
											break;
										}
									}
									if (type != EMPTY_DIR)
									{
										// next level ...
										hr = this->CountItemsNextLevel(hFind_sub_new, &ffd_count_sub_new, subitem_new, items);
									}
									else
									{
										// count one for the empty item ...
										items++;
									}
									FindClose(hFind_sub_new);
								}
							}
							delete[] subitem_new;
						}
					}
				}
			}
			items++;
		}
		else
		{
			items++;
		}
	} while (FindNextFile(hFind_sub, ffd_count_sub) != 0);

	return hr;
}

int TreeViewCTRL::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	UNREFERENCED_PARAMETER(lParamSort);

	int type1 = (int)lParam1;
	int type2 = (int)lParam2;

	if ((type1 == CHILD__ITEM) || (type1 == CNC3__ITEM))
	{
		if ((type2 == CHILD__ITEM) || (type2 == CNC3__ITEM))
		{
			return 1;
		}
		else
			return -1;
	}
	else
		return 1;
}

DWORD TreeViewCTRL::FOP_Proc(LPVOID lParam)
{
	__try
	{
		LPTVTHREADDATA ptd = reinterpret_cast<LPTVTHREADDATA>(lParam);
		if (ptd != nullptr)
		{
			TreeViewCTRL* tvC = reinterpret_cast<TreeViewCTRL*>(ptd->toClass);
			if (tvC != nullptr)
			{
				int mode = ptd->mode;

				tvC->internalFileOperationInProgress = TRUE;

				switch (mode)
				{
				case FOP_DELETE:
					tvC->_executeDeleteOP();
					break;
				case FOP_NEWFILE:
					tvC->_executeNewOP(A__CNC3FILE);
					break;
				case FOP_NEWFOLDER:
					tvC->_executeNewOP(A__FOLDER);
					break;
				case FOP_INSERT:
					tvC->_executeCopyOP();
					break;
				case FOP_MOVE:
					tvC->_executeMoveOP();
					break;
				case FOP_RENAME:
					Sleep(100);
					tvC->_executeRenameOP(tvC->FOPInfo.preOperationBuffer);
					break;
				default:
					break;
				}

				tvC->internalFileOperationInProgress = FALSE;
			}
		}
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return 6;
	}
	return 0;
}

DWORD TreeViewCTRL::initAsycProc(LPVOID lParam)
{
	auto tData = reinterpret_cast<LPTVTHREADDATA>(lParam);
	if (tData != nullptr)
	{
		auto _this = reinterpret_cast<TreeViewCTRL*>(tData->toClass);
		if (_this != nullptr)
		{
			iString rootPath(tData->root);

			if (_this->InitTreeViewItems(rootPath.GetData(), RELOAD_MODE))
			{
				auto listener = _this->getEventListener();
				if (listener != nullptr)
				{
					listener->operationComplete();
				}
				return 91;
			}
			else
			{
				auto listener = _this->getEventListener();
				if (listener != nullptr)
				{
					listener->operationFailed();
				}
				return 89;
			}
		}
	}
	return 0;
}

DWORD TreeViewCTRL::reloadProc(LPVOID lParam)
{
	auto _this = reinterpret_cast<TreeViewCTRL*>(lParam);
	if (_this != nullptr)
	{
		_this->Reload(true);
	}
	return 0;
}

HTREEITEM TreeViewCTRL::AddItemToTree( HWND TV, LPTSTR lpszItem, int nLevel, int type )
{
	TVITEM tvi;
	TVINSERTSTRUCT tvins;
	static HTREEITEM hPrev = reinterpret_cast< HTREEITEM >( TVI_FIRST );
	HTREEITEM hti;

	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	// Set the text of the item. 
	tvi.pszText = lpszItem;
	tvi.cchTextMax = MAX_HEADING_LEN;

    // Assume the item is not a parent item, so give it a document image.
	if( type == A__CNC3FILE )
	{
		tvi.iImage = this->ico_index[ 3 ];
		tvi.iSelectedImage = this->ico_index[ 3 ];
	}
	else if (type == A__EMPTYITEM)
	{
		//show_integer(0, nLevel);

		tvi.iImage = this->ico_index[4];
		tvi.iSelectedImage = this->ico_index[4];
	}
	else
	{
		tvi.iImage = this->ico_index[ 0 ];
		tvi.iSelectedImage = this->ico_index[ 0 ];
	}

	// Handle parent items without child items.
	if( ( type == EMPTY_DIR ) || ( type == ROOT_DIR ) )
	{
		tvi.iImage = this->ico_index[ 1 ];
		tvi.iSelectedImage = this->ico_index[ 1 ];
	}

	// Save the heading level in the item's application-defined data area. 
	tvi.lParam = (LPARAM)nLevel;
	tvins.item = tvi;
	tvins.hInsertAfter = hPrev;

	// Set the parent item based on the specified level.
	tvins.hParent = this->_dynamic_level_provider_(DYNAMICMODE_GETPARENTITEMFROMLEVEL, nLevel, hPrev);

	// Add the item to the tree-view control.
	hPrev =
		reinterpret_cast< HTREEITEM >(
			SendMessage(
				TV,
				TVM_INSERTITEM,
				static_cast< WPARAM >( 0 ),
				reinterpret_cast< LPARAM >( &tvins )
			)
		);
	if( hPrev == NULL )
		return NULL;

    // Save the handle to the item.
	this->_dynamic_level_provider_(DYNAMICMODE_SAVEITEMHANDLE, nLevel, hPrev);

	// The new item is a child item. Give the parent item a closed folder bitmap to indicate it now has child items.
	if( ( nLevel > 0 ) )
	{
		hti = TreeView_GetParent( TV, hPrev);
		tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.hItem = hti;
		tvi.iImage = this->ico_index[ 1 ];
		tvi.iSelectedImage = this->ico_index[ 1 ];
		TreeView_SetItem( TV, &tvi );
	}
	return hPrev;
}

HTREEITEM TreeViewCTRL::_dynamic_level_provider_(DWORD Mode, int nLevel, HTREEITEM hPrev)
{
	__try
	{
		if (nLevel < 0)
			return NULL;

		if (Mode == DYNAMICMODE_GETPARENTITEMFROMLEVEL)
		{
			// return parent item ...

			if (nLevel == 0)
			{
				return TVI_ROOT;
			}
			else if (nLevel == 1)
			{
				return this->DynamicLevel.dynamic_array->hPrev;
			}
			else
			{
				return this->DynamicLevel.dynamic_array[nLevel - 1].hPrev;
			}
		}
		else if (Mode == DYNAMICMODE_SAVEITEMHANDLE)
		{
			if (nLevel == this->DynamicLevel.nCount)
			{
				// exceed array and save item ... !

				if (this->DynamicLevel.nCount == 0)
				{
					if(this->DynamicLevel.dynamic_array != NULL)
						SafeDelete(&this->DynamicLevel.dynamic_array);

					this->DynamicLevel.dynamic_array = new HPREVITEM;
					
					if (this->DynamicLevel.dynamic_array != NULL)
					{
						this->DynamicLevel.dynamic_array->hPrev = hPrev;
						this->DynamicLevel.nCount = 1;
					}
					else
						return NULL;
				}
				else if (this->DynamicLevel.nCount == 1)
				{
					HPREVITEM item;
					item.hPrev = this->DynamicLevel.dynamic_array->hPrev;

					SafeDelete(&this->DynamicLevel.dynamic_array);

					this->DynamicLevel.dynamic_array = new HPREVITEM[2];

					if (this->DynamicLevel.dynamic_array != NULL)
					{
						this->DynamicLevel.dynamic_array[0].hPrev = item.hPrev;
						this->DynamicLevel.dynamic_array[1].hPrev = hPrev;
						this->DynamicLevel.nCount = 2;
					}
					else
						return NULL;
				}
				else if (this->DynamicLevel.nCount > 1)
				{
					LPHPREVITEM Pitem = new HPREVITEM[this->DynamicLevel.nCount];

					if (Pitem != NULL)
					{
						for (int i = 0; i < this->DynamicLevel.nCount; i++)
						{
							Pitem[i].hPrev = this->DynamicLevel.dynamic_array[i].hPrev;
						}
						SafeDeleteArray(&this->DynamicLevel.dynamic_array);

						this->DynamicLevel.nCount++;

						this->DynamicLevel.dynamic_array = new HPREVITEM[this->DynamicLevel.nCount];

						if (this->DynamicLevel.dynamic_array != NULL)
						{
							for (int i = 0; i < (this->DynamicLevel.nCount - 1); i++)
							{
								this->DynamicLevel.dynamic_array[i].hPrev = Pitem[i].hPrev;
							}
							this->DynamicLevel.dynamic_array[this->DynamicLevel.nCount - 1].hPrev = hPrev;
						}
						else
							return NULL;

						delete[] Pitem;
					}
					else
						return NULL;
				}
				else
					return NULL;
			}
			else
			{
				// save item ... !

				if (this->DynamicLevel.nCount <= 1)
				{
					this->DynamicLevel.dynamic_array->hPrev = hPrev;
				}
				else
				{
					this->DynamicLevel.dynamic_array[nLevel].hPrev = hPrev;
				}
			}

		}
	}
	__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return NULL;
	}
	return NULL;
}

void TreeViewCTRL::DisplayErrorBox(LPTSTR lpszFunction) 
{ 
	// Retrieve the system error message for the last-error code

	TCHAR* error_ = nullptr;
	TranslateLastError(&error_);

	iString Error(error_);

	if (lpszFunction != nullptr)
	{
		Error.Append(L"\n\nError located in function: ");
		Error.Append(lpszFunction);
	}

	MessageBox(this->TVFrame, Error.GetData(), L"Internal error", MB_OK | MB_ICONERROR);
}

BOOL TreeViewCTRL::OnTreeViewNotify( LPARAM lParam )
{
	if( ((LPNMHDR)lParam)->idFrom == ID_FILENAVIGATOR )
	{
		switch(((LPNMHDR)lParam)->code)
		{
			case NM_DBLCLK:
				return this->OnDblClick();
			case NM_RCLICK:
				return this->OnRgtClick();
			case TVN_SELCHANGED:
				return this->OnSelchanged(reinterpret_cast<LPNMTREEVIEW>(lParam));
			case TVN_ITEMEXPANDED:
				return this->OnItemExpanded(reinterpret_cast<LPNMTREEVIEW>(lParam));
			case TVN_ITEMEXPANDING:
				return this->OnItemExpanding(reinterpret_cast<LPNMTREEVIEW>(lParam));
			case TVN_BEGINDRAG:	
				return this->OnBeginDrag(reinterpret_cast<LPNMTREEVIEW>(lParam));
			case TVN_ENDLABELEDIT:
				return this->OnEndLabelEdit(reinterpret_cast<LPNMTVDISPINFO>(lParam));
			case TVN_BEGINLABELEDIT:
				return this->OnBeginLabelEdit(reinterpret_cast<LPNMTVDISPINFO>(lParam));
			case TVN_GETINFOTIP:
				return this->OnGetInfoTip(reinterpret_cast<LPNMTVGETINFOTIP>(lParam));
			default:
				break;
		}
	}
	return FALSE;
}

void TreeViewCTRL::OnTreeViewTimer(WPARAM wParam)
{
	if (wParam == ID_TVDRAGSCROLL_TIMER)
	{
		if (this->ScrollDirection != 0)
		{
			if (this->ScrollDirection == 1)
			{
				SendMessage(this->hwndTV, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
			}
			else if (this->ScrollDirection == -1)
			{
				SendMessage(this->hwndTV, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
			}

		}
	}
}

int TreeViewCTRL::OnMouseMove(LPARAM lParam)
{
	if (this->DragUnderway)
	{
		POINT pt;
		RECT rcNAV, rcTV;
		HTREEITEM hTarget;
		TVHITTESTINFO thi;

		GetClientRect(this->TVFrame, &rcNAV);
		GetWindowRect(this->hwndTV, &rcTV);

		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);

		thi.pt.x = pt.x;
		thi.pt.y = pt.y - (rcNAV.bottom - (rcTV.bottom - rcTV.top));

		if ((hTarget = TreeView_HitTest(this->hwndTV, &thi)) != NULL)
		{
			TreeView_SelectDropTarget(this->hwndTV, hTarget);
		}
	}
	return 0;
}

int TreeViewCTRL::OnLButtonUp()
{
	if (this->CursorScrollTimerActivated)
	{
		KillTimer(this->TVFrame, ID_TVDRAGSCROLL_TIMER);
		this->CursorScrollTimerActivated = FALSE;
		this->ScrollDirection = 0;
	}

	if (this->DragUnderway)
	{
		if (!this->FOPInfo.CursorOutOfValidArea)
		{
			if (this->FOPInfo.CursorOverEditBox)
			{
				if (this->FOPInfo.DragCursorType == A__CNC3FILE)
				{
					SendMessage(
						this->TVFrame,
						WM_COMMAND,
						MAKEWPARAM(TV_CTRL_OPENPATH, 0),
						reinterpret_cast<LPARAM>(this->FOPInfo.movepath_source));
				}
				else if (this->FOPInfo.DragCursorType == A__FILE)
				{
					SendMessage(
						this->TVFrame,
						WM_COMMAND,
						MAKEWPARAM(TV_CTRL_IMPORTFILE, 0),
						reinterpret_cast<LPARAM>(this->FOPInfo.movepath_source));
				}
			}
			else
			{
				HTREEITEM hDestination = TreeView_GetDropHilight(this->hwndTV);
				if (hDestination)
				{
					int type;
					TCHAR* path = NULL;

					TreeView_SelectItem(this->hwndTV, hDestination);

					type = this->GetItemPath(hDestination, &path);

					if (type != -1)
					{
						BOOL res = TRUE;

						if ((type == CHILD__ITEM) || (type == CNC3__ITEM) || (type == EMPTY__ITEM))
						{
							res = this->RemoveFilename(path);

							if (res)
							{
								res = TreeView_SelectItem(
									this->hwndTV,
									TreeView_GetParent(this->hwndTV, hDestination)
								);
							}
						}
						if (res)
						{
							SafeDeleteArray(&this->FOPInfo.movepath_target);

							if (CopyStringToPtrW(path, &this->FOPInfo.movepath_target))
							{
								this->FOPInfo.movepath_target_valid = TRUE;

								// execute Fileoperation
								this->PerformFileOperation(FOP_MOVE);
							}
						}
					}
				}
			}
		}
		else
			this->FOPInfo.CursorOutOfValidArea = FALSE;

		TreeView_SelectDropTarget(this->hwndTV, NULL);

		ReleaseCapture();
		SetCursor(
			LoadCursor(NULL, IDC_ARROW));

		this->DragUnderway = FALSE;
		this->FOPInfo.CursorOverEditBox = FALSE;
	}
	return 0;
}

void TreeViewCTRL::updateFont(HFONT font)
{
	if (this->tvFont != nullptr)
		DeleteObject(this->tvFont);

	this->tvFont = font;
	SendMessage(this->hwndTV, WM_SETFONT, reinterpret_cast<WPARAM>(font), static_cast<LPARAM>(TRUE));
}

void TreeViewCTRL::updateImageList()
{
	if(this->hwndTV != nullptr)
		this->InitTreeViewImageLists(this->hwndTV);
}

void TreeViewCTRL::PerformFileOperation(int mode)
{
	switch (mode)
	{
	case FOP_COPY:
		this->PrepareForCopyOperation();
		break;
	case FOP_DELETE:
		this->CreateFOP_Thread(mode);
		break;
	case FOP_MOVE:
		this->CreateFOP_Thread(mode);
		break;
	case FOP_RENAME:
		this->StartLabelEdit();
		break;
	case FOP_NEWFILE:
		this->CreateFOP_Thread(mode);
		break;
	case FOP_NEWFOLDER:
		this->CreateFOP_Thread(mode);
		break;
	case FOP_INSERT:
		this->CreateFOP_Thread(mode);
		break;
	case FOP_IMPORT:
		this->SendImportInstruction();
		break;
	case FOP_CONVERT:
		this->SendConvertInstruction();
		break;
	default:
		break;
	}
}

BOOL TreeViewCTRL::SaveRoot()
{
	BOOL result = TRUE;

	if ((this->Root_Folder == nullptr) || (this->Root_Name == nullptr))
	{
		result = FALSE;
	}
	else
	{
		AppPath rootfolderPath;
		auto path = rootfolderPath.Get(PATHID_FILE_NAVROOT);

		if (!path.Equals(ISTRING_FAIL_INDICATOR))
		{
			auto bfpo = CreateBasicFPO();
			if (bfpo != nullptr)
			{
				iString content(this->Root_Folder);
				content += L"||";
				content += this->Root_Name;
				content += L"||";

				result =
					bfpo->SaveBufferToFileAsUtf8(
						content.GetData(),
						path.GetData()
					);
				SafeRelease(&bfpo);
			}
		}
		else
			result = FALSE;
	}
	return result;
}

BOOL TreeViewCTRL::LoadRoot()
{
	HRESULT hr;
	
	AppPath navrootPath;
	auto path = navrootPath.Get(PATHID_FILE_NAVROOT);

	hr = (path.Equals(ISTRING_FAIL_INDICATOR)) ? E_FAIL : S_OK;
	if (SUCCEEDED(hr))
	{
		auto bfpo = CreateBasicFPO();
		if (bfpo != nullptr)
		{
			TCHAR* buffer = nullptr;

			bfpo->LoadBufferFmFileAsUtf8(
				&buffer,
				path.GetData()
			);

			hr = (buffer != nullptr) ? S_OK : E_POINTER;
			if (SUCCEEDED(hr))
			{
				auto max_buffer = _lengthOfString(buffer) + 1;

				int i = 0, j = 0, z = 0;

				while (buffer[z] != '|')
				{
					if (buffer[z] == L'\0')
						break;

					i++;
					z++;
				}
				z += 2;

				hr = (z >= max_buffer) ? E_FAIL : S_OK;
				if (SUCCEEDED(hr))
				{
					while (buffer[z] != '|')
					{
						j++;
						z++;

						if (z == max_buffer)
							break;
					}
					this->Root_Folder = new TCHAR[i + 1];

					hr = (this->Root_Folder != NULL) ? S_OK : E_FAIL;
					if (SUCCEEDED(hr))
					{
						this->Root_Name = new TCHAR[j + 2];

						hr = (this->Root_Name != NULL) ? S_OK : E_FAIL;
						if (SUCCEEDED(hr))
						{
							z = 0;
							j = 0;
							while (buffer[z] != '|')
							{
								this->Root_Folder[z] = buffer[z];
								z++;
							}
							this->Root_Folder[z] = '\0';
							z += 2;

							while (buffer[z] != '|')
							{
								this->Root_Name[j] = buffer[z];
								j++;
								z++;
							}
							this->Root_Name[j] = '\0';
						}
					}
				}
				delete[] buffer;
			}
			SafeRelease(&bfpo);
		}
			
	}
	return SUCCEEDED(hr) ? TRUE : FALSE;
}

HRESULT TreeViewCTRL::GenerateRoot()
{
	HRESULT hr;
	size_t len;

	hr = StringCbLength(this->Working_Dir, sizeof(TCHAR)*STRSAFE_MAX_CCH, &len);
	if (SUCCEEDED(hr))
	{
		len += sizeof(TCHAR);

		this->Root_Folder = new TCHAR[len];
		hr = (this->Root_Folder != nullptr) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			hr = StringCbCopy(this->Root_Folder, len, this->Working_Dir);
		}
	}
	return hr;
}

void TreeViewCTRL::LeachRootName( LPCTSTR path, LPTSTR name )
{
	if( path == NULL )
		goto Fail1;
	if( name == NULL )
		goto Fail2;

	int i = 0;

	while( path[ i ] != '\0' )
	{
		i++;
	}
	while( path[ i ] != '\\' )
	{
		i--;
		if( i == -1 )
			break;
	}
	if( i == -1 )
		goto Fail3;
	i++;

	int j = 0;

	while( path[ i ] != '\0' )
	{
		name[ j ] = path[ i ];
		i++;
		j++;
		if( j == MAX_HEADING_LEN )
			break;
	}
	if( j == MAX_HEADING_LEN )
		goto Fail4;
	else
		name[ j ] = '\0';
	return;
Fail1:
	StringCbCopy( name, sizeof( TCHAR ) * MAX_HEADING_LEN, L"Fail: Invalid path buffer" );
	return;
Fail2:
	//StringCbCopy( name, sizeof( TCHAR ) * MAX_HEADING_LEN, L"Fail: Invalid name buffer" );
	return;
Fail3:
	StringCbCopy( name, sizeof( TCHAR ) * MAX_HEADING_LEN, L"Fail: Counter fall below buffer" );
	return;
Fail4:
	StringCbCopy( name, sizeof( TCHAR ) * MAX_HEADING_LEN, L"Fail: Max Heading length exeeded" );
	return;
}

BOOL TreeViewCTRL::CompareHeading( LPTSTR Heading, LPTSTR sample )
{
	if( ( Heading == NULL ) || ( sample == NULL ) )
	{
		return FALSE;
	}
	__try
	{
		int i = 0;

		while( i <= MAX_HEADING_LEN )
		{
			if( Heading[ i ] != sample[ i ] )
			{
				return FALSE;
			}
			else if( ( Heading[ i ] == L'\0' ) && ( sample[ i ] == L'\0' ) )
			{
				break;
			}
			i++;

			if( i == MAX_HEADING_LEN )
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	__except( GetExceptionCode( ) == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		return FALSE;
	}
}

BOOL TreeViewCTRL::IsRootinPath(LPTSTR path)
{
	__try
	{
		int i = 0;

		while (this->Root_Folder[i] != L'\0')
		{
			if (path[i] != this->Root_Folder[i])
			{
				return FALSE;
			}
			i++;
		}
		return TRUE;
	}
	__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return FALSE;
	}
}


int TreeViewCTRL::CheckItemType( HTREEITEM item )
{
	TVITEM tvi;
	tvi.hItem = item;
	tvi.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE;

	if( !TreeView_GetItem( this->hwndTV, &tvi ) )
	{
		return -1;
	}

	if( ( tvi.cChildren == 0 ) && ( ( tvi.iImage == 0 ) || ( tvi.iImage == 3 ) || (tvi.iImage == 4)) )
	{
		if( tvi.iImage == 3 )
		{
			return CNC3__ITEM;
		}
		else if (tvi.iImage == 4)
		{
			return EMPTY__ITEM;
		}
		else
		{
			return CHILD__ITEM;
		}
	}
	else
	{
		return PARENT__ITEM;
	}
}

int TreeViewCTRL::_getSelectedItemPath( TCHAR** Ipath )
{
	int type = -1;
	HRESULT hr;
	HTREEITEM selection = TreeView_GetSelection(this->hwndTV);

	hr = (selection) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		type = this->CheckItemType(selection);

		hr = (type != -1) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			TCHAR* buffer = new TCHAR[MAX_HEADING_LEN];

			hr = (buffer == NULL) ? E_OUTOFMEMORY : S_OK;
			if (SUCCEEDED(hr))
			{
				TCHAR* root = new TCHAR[MAX_HEADING_LEN];

				hr = (root == NULL) ? E_OUTOFMEMORY : S_OK;
				if (SUCCEEDED(hr))
				{
					TCHAR* path = new TCHAR[(size_t)MAX_FILEPATH_LEN];

					hr = (path == NULL) ? E_OUTOFMEMORY : S_OK;
					if (SUCCEEDED(hr))
					{
						hr = StringCbCopy(path, MAX_FILEPATH_LEN, this->Root_Folder);
						if (SUCCEEDED(hr))
						{
							TVITEM tv;
							tv.hItem = TreeView_GetRoot(this->hwndTV);
							tv.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_IMAGE;
							tv.cchTextMax = sizeof(TCHAR) * MAX_HEADING_LEN;
							tv.pszText = root;

							hr = TreeView_GetItem(this->hwndTV, &tv) ? S_OK : E_FAIL;
							if (SUCCEEDED(hr))
							{
								HTREEITEM item = selection;

								hr = (item) ? S_OK : E_FAIL;
								if (SUCCEEDED(hr))
								{
									int ItemCount = 1;

									while ((item = TreeView_GetParent(this->hwndTV, item)) != NULL)
									{
										// count the levels ...
										ItemCount++;
									}
									HTREEITEM* cItems = new HTREEITEM[ItemCount];
									int max_array = ItemCount;

									hr = (cItems == NULL) ? E_OUTOFMEMORY : S_OK;
									if (SUCCEEDED(hr))
									{
										cItems[0] = selection;

										ItemCount = 1;

										if (ItemCount < max_array)
										{
											while ((cItems[ItemCount] = TreeView_GetParent(this->hwndTV, cItems[ItemCount - 1])) != NULL)
											{
												// fill the array ...
												ItemCount++;

												if (ItemCount == max_array)		// C6385 suppressor (useless?)
													break;
											}
											ItemCount--;
										}

										for (int j = ItemCount; j >= 0; j--)
										{
											if (j == max_array)		// C6385 suppressor (useless?)
												break;

											// build the path ...
											tv.hItem = cItems[j];
											tv.pszText = buffer;

											hr = TreeView_GetItem(this->hwndTV, &tv) ? S_OK : E_FAIL;
											if (FAILED(hr))break;

											if (this->CompareHeading(root, buffer) && (j == ItemCount))
											{
												continue;
											}
											hr = StringCbCat(path, MAX_FILEPATH_LEN, L"\\");
											if (FAILED(hr))break;

											hr = StringCbCat(path, MAX_FILEPATH_LEN, buffer);
											if (FAILED(hr))break;
										}
										if (SUCCEEDED(hr))
										{
											if (type == CNC3__ITEM)
											{
												hr = StringCbCat(path, MAX_FILEPATH_LEN, L".cnc3\0");
											}
											if (SUCCEEDED(hr))
											{
												size_t buffer_len;

												hr = StringCbLength(path, MAX_FILEPATH_LEN, &buffer_len);
												if (SUCCEEDED(hr))
												{
													buffer_len += (size_t)(sizeof(TCHAR) * 100);

													(*Ipath) = new TCHAR[buffer_len];

													hr = ((*Ipath) == NULL) ? E_OUTOFMEMORY : S_OK;
													if (SUCCEEDED(hr))
													{
														hr = StringCbCopy((*Ipath), buffer_len, path);
													}
												}
											}
										}
									}
								}
							}
						}
						delete[] path;
					}
					delete[] root;
				}
				delete[] buffer;
			}
		}
	}
	if (FAILED(hr))
		type = -2;

	return type;
}

int TreeViewCTRL::GetItemPath(HTREEITEM RQitem, TCHAR **Ipath)
{
	int type = -1;
	HRESULT hr;

	hr = (RQitem) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		type = this->CheckItemType(RQitem);

		hr = (type != -1) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			TCHAR* buffer = new TCHAR[MAX_HEADING_LEN];

			hr = (buffer == NULL) ? E_OUTOFMEMORY : S_OK;
			if (SUCCEEDED(hr))
			{
				TCHAR* root = new TCHAR[MAX_HEADING_LEN];

				hr = (root == NULL) ? E_OUTOFMEMORY : S_OK;
				if (SUCCEEDED(hr))
				{
					TCHAR* path = new TCHAR[(size_t)MAX_FILEPATH_LEN];

					hr = (path == NULL) ? E_OUTOFMEMORY : S_OK;
					if (SUCCEEDED(hr))
					{
						hr = StringCbCopy(path, MAX_FILEPATH_LEN, this->Root_Folder);
						if (SUCCEEDED(hr))
						{
							TVITEM tv;
							tv.hItem = TreeView_GetRoot(this->hwndTV);
							tv.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_IMAGE;
							tv.cchTextMax = sizeof(TCHAR) * MAX_HEADING_LEN;
							tv.pszText = root;

							hr = TreeView_GetItem(this->hwndTV, &tv) ? S_OK : E_FAIL;
							if (SUCCEEDED(hr))
							{
								HTREEITEM item = RQitem;

								hr = (item) ? S_OK : E_FAIL;
								if (SUCCEEDED(hr))
								{
									int ItemCount = 1;

									while ((item = TreeView_GetParent(this->hwndTV, item)) != NULL)
									{
										// count the levels ...
										ItemCount++;
									}
									HTREEITEM* cItems = new HTREEITEM[ItemCount];
									int max_array = ItemCount;

									hr = (cItems == NULL) ? E_OUTOFMEMORY : S_OK;
									if (SUCCEEDED(hr))
									{
										cItems[0] = RQitem;

										ItemCount = 1;

										if (ItemCount < max_array)
										{
											while ((cItems[ItemCount] = TreeView_GetParent(this->hwndTV, cItems[ItemCount - 1])) != NULL)
											{
												// fill the array ...
												ItemCount++;

												if (ItemCount == max_array)
													break;
											}
											ItemCount--;
										}

										for (int j = ItemCount; j >= 0; j--)
										{
											if (j == max_array)		// C6385 suppressor
												break;

											// build the path ...
											tv.hItem = cItems[j];
											tv.pszText = buffer;

											hr = TreeView_GetItem(this->hwndTV, &tv) ? S_OK : E_FAIL;
											if (FAILED(hr))break;

											if (this->CompareHeading(root, buffer) && (j == ItemCount))
											{
												continue;
											}
											hr = StringCbCat(path, MAX_FILEPATH_LEN, L"\\");
											if (FAILED(hr))break;

											hr = StringCbCat(path, MAX_FILEPATH_LEN, buffer);
											if (FAILED(hr))break;
										}
										if (SUCCEEDED(hr))
										{
											if (type == CNC3__ITEM)
											{
												hr = StringCbCat(path, MAX_FILEPATH_LEN, L".cnc3\0");
											}
											if (SUCCEEDED(hr))
											{
												size_t buffer_len;

												hr = StringCbLength(path, MAX_FILEPATH_LEN, &buffer_len);
												if (SUCCEEDED(hr))
												{
													buffer_len += (size_t)(sizeof(TCHAR) * 100);

													(*Ipath) = new TCHAR[buffer_len];

													hr = ((*Ipath) == NULL) ? E_OUTOFMEMORY : S_OK;
													if (SUCCEEDED(hr))
													{
														hr = StringCbCopy((*Ipath), buffer_len, path);
													}
												}
											}
										}
										delete[] cItems;
									}
								}
							}
						}
						delete[] path;
					}
					delete[] root;
				}
				delete[] buffer;
			}
		}
	}
	if (FAILED(hr))
		type = -2;

	return type;
}

int TreeViewCTRL::GetItemLevel(HTREEITEM Item)
{
	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem = Item;

	if (TreeView_GetItem(this->hwndTV, &tvi))
	{
		return (int)tvi.lParam;
	}
	else
		return -1;
}

int TreeViewCTRL::GetUserPermission(int mode)
{
	if (this->disableFOPConfirmation)
		return IDOK;
	else
	{
		int result = IDOK;

		TCHAR name[MAX_HEADING_LEN] = { 0 };

		TVITEM tvi;
		tvi.mask = TVIF_HANDLE | TVIF_TEXT;
		tvi.hItem = TreeView_GetSelection(this->hwndTV);
		tvi.pszText = name;
		tvi.cchTextMax = MAX_HEADING_LEN;

		if (TreeView_GetItem(this->hwndTV, &tvi))
		{
			int type = this->CheckItemType(tvi.hItem);

			if (type != -1)
			{
				TCHAR text[1024] = { 0 };

				switch (mode)
				{
				case FOP_DELETE:
					if ((type == CNC3__ITEM) || (type == CHILD__ITEM))
					{
						HRESULT hr = StringCbPrintf(text, sizeof(text), L"'%s' wirklich löschen?", name);
						if (SUCCEEDED(hr))
						{
							result = MessageBox(this->TVFrame, text, L"Datei löschen ...\0", MB_OKCANCEL | MB_ICONINFORMATION);
						}
					}
					else if ((type == PARENT__ITEM) || (type == ROOT__ITEM))
					{
						HRESULT hr = StringCbPrintf(text, sizeof(text), L"'%s' und den gesamten Inhalt wirklich löschen?", name);
						if (SUCCEEDED(hr))
						{
							result = MessageBox(this->TVFrame, text, L"Ordner löschen ...\0", MB_OKCANCEL | MB_ICONINFORMATION);
						}
					}
					return result;
				default:
					return result;
				}
			}
		}
	}
	return -1;
}

HTREEITEM TreeViewCTRL::getItemHandleFromRelativePath(LPCTSTR path, int* type_out, bool returnLastValidHandle)
{
	iString itemName;
	iString itemPath(this->Root_Folder);
	itemPath += L"\\";
	itemPath += path;

	*type_out = INVALID_TYPE_INFO;

	int i = 0;
	TCHAR chr[2];
	chr[1] = L'\0';
	auto len = _lengthOfString(path) + 1;
	bool exit = false;

	HTREEITEM parent = TreeView_GetRoot(this->hwndTV);
	HTREEITEM target = nullptr;

	while (i < len)
	{
		if (path[i] != L'\\')// record the item-name
		{
			if (path[i] == L'\0') // path end reached -> must be the last item
			{
				auto bResult =
					isFile(
						itemPath.GetData()
					);

				if (bResult == FALSE)	// is directory
				{
					*type_out = PARENT__ITEM;
				}
				else	// is file (or the file does not exist, but this could be a valid scenario in case of delete or rename actions)
				{
					auto bfpo = CreateBasicFPO();
					if (bfpo != nullptr)
					{
						TCHAR* ext = nullptr;

						if (bfpo->GetFileExtension(itemName.GetData(), &ext) == TRUE)
						{
							if (CompareStringsB(ext, L".cnc3"))
							{
								*type_out = CNC3__ITEM;
								// remove the file extension from the itemname to comform with the headingname in the treeview
								itemName.Remove(ext);
							}
							else
							{
								*type_out = CHILD__ITEM;
							}
							SafeDeleteArray((void**)&ext);
						}
						SafeRelease(&bfpo);
					}
				}

				// search for the last item
				target = TreeView_GetChild(this->hwndTV, parent);

				if (target != nullptr)
				{
					do
					{
						TCHAR Iname[MAX_HEADING_LEN] = { 0 };

						TVITEM tvi;
						tvi.mask = TVIF_TEXT | TVIF_HANDLE;
						tvi.hItem = target;
						tvi.pszText = Iname;
						tvi.cchTextMax = MAX_HEADING_LEN;

						if (TreeView_GetItem(this->hwndTV, &tvi))
						{
							if (itemName.Equals(Iname))
							{
								exit = true;
								break;
							}
						}

					} while ((target = TreeView_GetNextSibling(this->hwndTV, target)) != nullptr);
				}
				if ((target == nullptr) && returnLastValidHandle)
				{
					// if the target is nullptr, the item does not exist in the treeview
					// -> return the last valid handle
					target = parent;
					*type_out = -1;
					exit = true;
				}
			}
			else
			{
				// record the itemname
				chr[0] = path[i];
				itemName.Append(chr);
			}
		}
		else // must be L'\\' -> search for the item
		{
			target = TreeView_GetChild(this->hwndTV, parent);

			if (target != nullptr)
			{
				do
				{
					TCHAR Iname[MAX_HEADING_LEN] = { 0 };

					TVITEM tvi;
					tvi.mask = TVIF_TEXT | TVIF_HANDLE;
					tvi.hItem = target;
					tvi.pszText = Iname;
					tvi.cchTextMax = MAX_HEADING_LEN;

					if (TreeView_GetItem(this->hwndTV, &tvi))
					{
						if (itemName.Equals(Iname))
						{
							parent = target;
							break;
						}
					}

				} while ((target = TreeView_GetNextSibling(this->hwndTV, target)) != nullptr);
			}
			if ((target == nullptr) && returnLastValidHandle)
			{
				// if the target is nullptr, the item does not exist in the treeview
				// -> return the last valid handle
				target = parent;
				*type_out = -1;
				exit = true;
			}
			itemName.Clear();// clear the recorded name for next item-name
		}
		if (exit)
			break;

		i++;
	}
	if (target == TreeView_GetRoot(this->hwndTV))// if the target is the root item - the method fails at the first shot
		target = nullptr;

	return target;
}


BOOL TreeViewCTRL::Sequencing( int mode, LPHEADING newItems, LPHEADING oldItems, int NUMitems )
{
    if( ( newItems == NULL ) || ( oldItems == NULL ) || ( NUMitems <= 0 ) )
    {
        return FALSE;
    }
	HRESULT hr;
	BOOL ffMode = TRUE;

    int i = 1;
    int j = 1;
	int startlevel = 2;
	int holding = 1;

	if (mode == SQC_FOLDER)
	{
		i = 0;
		j = 0;
		startlevel = oldItems[0].Level + 1;
	}
	else
	{
		/////////////Copy the Root Item >>
		newItems[0].Level = oldItems[0].Level;
		newItems[0].type = oldItems[0].type;
		hr = StringCbCopy(newItems[0].Heading, sizeof(TCHAR) * MAX_HEADING_LEN, oldItems[0].Heading);
		if (FAILED(hr))
			return FALSE;
	}
    /////////////Start sequencing >>
    while( holding )
    {
        if( ffMode )//Process folders >>
        {
            if( oldItems[ i ].type == A__FOLDER )
            {
                newItems[ j ].Level    = oldItems[ i ].Level;
                newItems[ j ].type    = oldItems[ i ].type;

                hr = StringCbCopy( newItems[ j ].Heading, sizeof( TCHAR ) * MAX_HEADING_LEN, oldItems[ i ].Heading );
                if( FAILED( hr ))
                    return FALSE;

                oldItems[ i ].type = 0;//mark the processed item

                i++;
                j++;

                j = this->SequenceNextLevel( i, j, newItems, oldItems, NUMitems, startlevel );//Process next level
                if( j == -1 )
                    return FALSE;
            }
            else if( oldItems[ i ].type == EMPTY_DIR )
            {
                newItems[ j ].Level    = oldItems[ i ].Level;
                newItems[ j ].type    = oldItems[ i ].type;

                hr = StringCbCopy( newItems[ j ].Heading, sizeof( TCHAR ) * MAX_HEADING_LEN, oldItems[ i ].Heading );
                if( FAILED( hr ))
                    return FALSE;

                oldItems[ i ].type = 0;//mark the processed item

                i++;
                j++;
            }
            else
                i++;

            if( i == NUMitems )
            {
				if (mode == SQC_FOLDER)
					i = 0;
				else
	                i = 1;

                ffMode = FALSE;
            }
            if( j == NUMitems )
            {
                return TRUE;
            }
        }
        else//////////Process files >>
        {
            if( oldItems[ i ].type == A__FILE )
            {
                newItems[ j ].Level    = oldItems[ i ].Level;
                newItems[ j ].type    = oldItems[ i ].type;

                hr = StringCbCopy( newItems[ j ].Heading, sizeof( TCHAR ) * MAX_HEADING_LEN, oldItems[ i ].Heading );
                if( FAILED( hr ))
                    return FALSE;

				if( this->CheckFileType( newItems[ j ].Heading ) == 1 )
				{
					newItems[ j ].type	= A__CNC3FILE;
				}

                oldItems[ i ].type = 0;//mark the processed item

                i++;
                j++;
            }
			else if (oldItems[i].type == A__EMPTYITEM)
			{
				newItems[j].Level = oldItems[i].Level;
				newItems[j].type = oldItems[i].type;

				hr = StringCbCopy(newItems[j].Heading, sizeof(TCHAR) * MAX_HEADING_LEN, oldItems[i].Heading);
				if (FAILED(hr))
					return FALSE;

				oldItems[i].type = 0;//mark the processed item

				i++;
				j++;
			}
            else
                i++;


            if( i == NUMitems )
            {
                return TRUE;
            }
            if( j == NUMitems )
            {
                return TRUE;
            }
        }
    }
    return TRUE;
}

int TreeViewCTRL::SequenceNextLevel( int startPos, int counter, LPHEADING newItems, LPHEADING oldItems, int NUMitems, int ProcessLevel )
{
    if( counter < NUMitems )
    {
        int i = startPos;
		int holding = 1;
        BOOL ffMode = TRUE;
        HRESULT hr;

        while( holding )
        {
            if( ffMode )
            {
                if( oldItems[ i ].type == A__FOLDER )
                {
                    newItems[ counter ].Level    = oldItems[ i ].Level;
                    newItems[ counter ].type    = oldItems[ i ].type;

                    hr = StringCbCopy( newItems[ counter ].Heading, sizeof( TCHAR ) * MAX_HEADING_LEN, oldItems[ i ].Heading );
                    if( FAILED( hr ))
                        return -1;

                    oldItems[ i ].type = 0;

                    i++;
                    counter++;

                    counter = this->SequenceNextLevel( i, counter, newItems, oldItems, NUMitems, ProcessLevel + 1 );
                    if( counter == -1 )
                        return counter;
                }
                else if( oldItems[ i ].type == EMPTY_DIR )
                {
                    newItems[ counter ].Level    = oldItems[ i ].Level;
                    newItems[ counter ].type    = oldItems[ i ].type;

                    hr = StringCbCopy( newItems[ counter ].Heading, sizeof( TCHAR ) * MAX_HEADING_LEN, oldItems[ i ].Heading );
                    if( FAILED( hr ))
                        return -1;

                    oldItems[ i ].type = 0;

                    i++;
                    counter++;
                }
                else
                    i++;

                if( ( oldItems[ i ].Level < ProcessLevel ) || ( i == (NUMitems - 1) ))// changed
                {
                    i = startPos;
                    ffMode = FALSE;
                }
                if( counter == NUMitems )
                {
                    break;
                }
            }
            else
            {
                if( oldItems[ i ].type == A__FILE )
                {
                    newItems[ counter ].Level    = oldItems[ i ].Level;
                    newItems[ counter ].type    = oldItems[ i ].type;

                    hr = StringCbCopy( newItems[ counter ].Heading, sizeof( TCHAR ) * MAX_HEADING_LEN, oldItems[ i ].Heading );
                    if( FAILED( hr ))
                        return -1;

					if( this->CheckFileType( newItems[ counter ].Heading ) == 1 )
					{
						newItems[ counter ].type = A__CNC3FILE;
					}

                    oldItems[ i ].type = 0;

                    i++;
                    counter++;
                }
				else if (oldItems[i].type == A__EMPTYITEM)
				{
					newItems[counter].Level = oldItems[i].Level;
					newItems[counter].type = oldItems[i].type;

					hr = StringCbCopy(newItems[counter].Heading, sizeof(TCHAR) * MAX_HEADING_LEN, oldItems[i].Heading);
					if (FAILED(hr))
						return FALSE;

					oldItems[i].type = 0;//mark the processed item

					i++;
					counter++;
				}
                else
                    i++;

                if( ( oldItems[ i ].Level < ProcessLevel ) || ( i == (NUMitems - 1) ) )//changed
                {
                    break;
                }
                if( counter == NUMitems )
                {
                    break;
                }
            }
        }
    }
    return counter;
}

int TreeViewCTRL::CheckFileType( LPTSTR filename )
{
	if( filename == NULL )
		return 0;
	else
	{
		__try
		{
			int i = 0;

			while( filename[ i ] != L'\0' )
			{
				i++;
			}
			if( i > 5 )
			{
				if( ( filename[ i - 1 ] == L'3' ) &&
					( filename[ i - 2 ] == L'c' ) &&
					( filename[ i - 3 ] == L'n' ) &&
					( filename[ i - 4 ] == L'c' ) &&
					( filename[ i - 5 ] == L'.' ) )
				{
					filename[ i - 1 ] = L'\0';
					filename[ i - 2 ] = L'\0';
					filename[ i - 3 ] = L'\0';
					filename[ i - 4 ] = L'\0';
					filename[ i - 5 ] = L'\0';

					return 1;
				}
				else
					return 0;
			}
		}
		__except( GetExceptionCode( ) == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
		{
			return -1;
		}
	}
	return 0;
}

void TreeViewCTRL::InsertExistingFileToView(LPTSTR path)
{
	if (this->IsRootinPath(path))
	{
		TCHAR* filename = NULL;
		if (GetFilenameOutOfPath(path, &filename, TRUE) == TRUE)
		{
			TCHAR* w_path = NULL;

			if (CopyStringToPtr(path, &w_path) == TRUE)
			{
				if (this->RemoveFilename(w_path))
				{
					if (this->Root_Folder != NULL)
					{
						__try
						{
							int i = 0;
							size_t len;
							HRESULT hr;

							hr = StringCbLength(w_path, sizeof(TCHAR)*STRSAFE_MAX_CCH, &len);
							if (SUCCEEDED(hr))
							{
								TCHAR* newPath = new TCHAR[len + sizeof(TCHAR)];
								if (newPath != NULL)
								{
									while (w_path[i] == this->Root_Folder[i])
									{
										if (this->Root_Folder[i] == L'\0')break;
										i++;
									}
									int j = 0;

									while (w_path[i] != L'\0')
									{
										newPath[j] = w_path[i];
										i++;
										j++;
									}
									newPath[j] = L'\0';
									int k = 0, itemCount = 0;

									while (newPath[k] != L'\0')
									{
										if (newPath[k] == L'\\')itemCount++;
										k++;
									}

									if (itemCount > 0)
									{
										LPMDA hdngName = new MDA[itemCount];

										if (hdngName != NULL)
										{
											SecureZeroMemory(hdngName, (sizeof(MDA) * itemCount));

											i = 1;
											j = 0;
											k = 0;

											while (k < itemCount)
											{
												while (newPath[i] != L'\\')
												{
													if (newPath[i] == L'\0')
													{
														break;
													}
													hdngName[k].name[j] = newPath[i];

													j++;

													if (j > (MAX_HEADING_LEN - 1))
													{
														break;
													}
													i++;
												}
												i++;

												hdngName[k].name[j] = L'\0';

												if (newPath[i] == L'\0')
												{
													break;
												}
												k++;
												j = 0;
											}
											SecureZeroMemory(newPath, len + sizeof(TCHAR));

											TVITEM itv = { 0 };										// ?????
											itv.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_IMAGE;
											itv.pszText = newPath;
											itv.cchTextMax = (int)(len + sizeof(TCHAR));

											HTREEITEM root = TreeView_GetRoot(this->hwndTV);
											if (root != NULL)
											{
												//int level = 0;
												int levelCounter = 0;
												HTREEITEM item = root;

												while (levelCounter < itemCount)
												{
													item = TreeView_GetChild(this->hwndTV, item);
													if (item)
													{
														itv.hItem = item;

														if (TreeView_GetItem(this->hwndTV, &itv))
														{
															if (!this->CompareHeading(hdngName[levelCounter].name, newPath))
															{
																while ((item = TreeView_GetNextSibling(this->hwndTV, item)) != NULL)
																{
																	itv.hItem = item;

																	if (TreeView_GetItem(this->hwndTV, &itv))
																	{
																		if (this->CompareHeading(hdngName[levelCounter].name, newPath))
																		{
																			break;
																		}
																	}
																}
															}
														}
													}
													else
														break;

													levelCounter++;
												}
												if (levelCounter == itemCount)
												{
													// insert
													if (item != NULL)
													{
														TVITEM tvi;
														tvi.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
														tvi.pszText = filename;
														tvi.lParam = ((int)itv.lParam) + 1;// one level under the parent ...
														tvi.iImage = this->ico_index[3];
														tvi.iSelectedImage = this->ico_index[3];

														TVINSERTSTRUCT tvins;
														tvins.hParent = item;
														tvins.hInsertAfter = this->FindInsertAfter(item, CNC3__ITEM, filename);
														tvins.item = tvi;

														TreeView_InsertItem(this->hwndTV, &tvins);
													}
												}
											}
											delete [] hdngName;
										}
									}
									delete[] newPath;
								}
							}
						}
						__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
						{
							return;
						}
					}
				}
				SafeDeleteArray(&w_path);
			}
			SafeDeleteArray(&filename);
		}
	}
}

BOOL TreeViewCTRL::ExpandPathToItem( LPCTSTR ItemPath )
{
	BOOL result = TRUE;
	int holding = 1;

	if( ( ItemPath == NULL ) || ( this->Root_Folder == NULL ) )
	{
		return FALSE;
	}
	__try
	{
		size_t len;

		HRESULT hr = StringCbLength( ItemPath, sizeof( TCHAR ) *4096, &len );
		if( SUCCEEDED( hr ))
		{
			TCHAR* pathbuffer = new TCHAR[ len + sizeof( TCHAR ) ];

			if( pathbuffer != NULL )
			{
				SecureZeroMemory( pathbuffer, ( len + sizeof( TCHAR )) );
				int i = 0;

				while( ItemPath[ i ] == this->Root_Folder[ i ] )
				{
					i++;	

					if( this->Root_Folder[ i ] == L'\0' )
					{
						break;
					}
				}
				i++;

				int j = 0;

				while( ItemPath[ i ] != L'\0' )
				{
					pathbuffer[ j ] = ItemPath[ i ];

					i++;
					j++;
				}
				pathbuffer[ j ] = L'\0';

				int k = 0, cnt = 0;

				while( pathbuffer[ k ] != L'\0' )
				{
					if( pathbuffer[ k ] == L'\\' )
					{
						cnt++;
					}
					k++;
				}
				if( cnt > 0 )// ???
				{
					LPMDA hdngName = new MDA[ cnt ];

					if( hdngName != NULL )
					{
						SecureZeroMemory( hdngName, ( sizeof( MDA ) * cnt ));

						i = 0;
						j = 0;
						k = 0;

						while( k < cnt )
						{
							while( pathbuffer[ i ] != L'\\' )
							{
								if( pathbuffer[ i ] == L'\0' )
								{
									break;
								}
								hdngName[ k ].name[ j ] = pathbuffer[ i ];

								j++;

								if( j > ( MAX_HEADING_LEN - 1 ))
								{
									break;
								}
								i++;
							}
							i++;

							hdngName[ k ].name[ j ] = L'\0';

							if( pathbuffer[ i ] == L'\0' )
							{
								break;
							}
							k++;
							j = 0;
						}
						TCHAR itembuffer[ MAX_HEADING_LEN ] = { 0 };

						HTREEITEM root = TreeView_GetRoot( this->hwndTV );
						if( root != NULL )
						{
							HTREEITEM Nitem = TreeView_GetChild( this->hwndTV, root );
							if( Nitem != NULL )
							{
								i = 0;

								TVITEM itv;
								itv.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_IMAGE;
								itv.hItem = Nitem;
								itv.pszText = itembuffer;
								itv.cchTextMax = sizeof( itembuffer );

								while( holding )
								{
									if( !TreeView_GetItem( this->hwndTV, &itv ))
									{
										result = FALSE;

										break;
									}
									else
									{
										if(( itv.iImage == 0 ) || ( itv.iImage == 3 ))
										{
											result = FALSE;

											break;
										}
										if (i == cnt)
											break;		// !!!

										if( this->CompareHeading( itembuffer, hdngName[ i ].name ))
										{
											i++;

											if( TreeView_Expand( this->hwndTV, Nitem, TVE_EXPAND ))
											{
												if( !this->ExpandNextLevel( Nitem, i, hdngName, cnt ))
												{
													result = FALSE;

													break;
												}
											}
										}
										else
										{
											Nitem = TreeView_GetNextSibling( this->hwndTV, Nitem );
											if( !Nitem )
											{
												result = FALSE;

												break;
											}
											else
											{
												itv.hItem = Nitem;
											}
										}
									}
								}
							}
						}
						delete[] hdngName;
					}
				}
				delete[] pathbuffer;
			}
		}
	}
	__except( GetExceptionCode( ) == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		return FALSE;
	}
	return result;
}

BOOL TreeViewCTRL::ExpandNextLevel( HTREEITEM root, int i, LPMDA hdngName, int max_level )
{
	if( i == max_level )
	{
		return TRUE;
	}
	TCHAR itembuffer[ MAX_HEADING_LEN ] = { 0 };

	HTREEITEM Nitem = TreeView_GetChild( this->hwndTV, root );
	if( Nitem != NULL )
	{
		int holding = 1;

		TVITEM itv;
		itv.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_IMAGE;
		itv.hItem = Nitem;
		itv.pszText = itembuffer;
		itv.cchTextMax = sizeof( itembuffer );

		while( holding )
		{
			if( !TreeView_GetItem( this->hwndTV, &itv ))
			{
				return FALSE;
			}
			else
			{
				if(( itv.iImage == 0 ) || ( itv.iImage == 3 ))
				{
					return FALSE;
				}
				if( this->CompareHeading( itembuffer, hdngName[ i ].name ))
				{
					i++;

					if( TreeView_Expand( this->hwndTV, Nitem, TVE_EXPAND ))
					{
						if( !this->ExpandNextLevel( Nitem, i, hdngName, max_level ))
						{
							return FALSE;
						}
					}
				}
				else
				{
					Nitem = TreeView_GetNextSibling( this->hwndTV, Nitem );
					if( !Nitem )
					{
						return FALSE;
					}
					else
					{
						itv.hItem = Nitem;
					}
				}
			}
		}
	}
	return TRUE;
}

void TreeViewCTRL::Reload(bool recoverScrollPosition)
{
	SCROLLINFO sInfo;
	SecureZeroMemory(&sInfo, sizeof(SCROLLINFO));

	sInfo.cbSize = sizeof(SCROLLINFO);
	sInfo.fMask = SIF_POS | SIF_PAGE | SIF_TRACKPOS | SIF_RANGE;

	auto res = recoverScrollPosition ? GetScrollInfo(this->hwndTV, SB_VERT, &sInfo) : TRUE;

	if (res)
	{
		ShowWindow(this->hwndTV, SW_HIDE);

		int nCount = this->GetExpandImage(nullptr, COUNT);
		if (nCount > 0)
		{
			auto expImage = new HEADING[nCount];
			if (expImage != nullptr)
			{
				if (this->GetExpandImage(expImage, READ) != -1)
				{
					iString path(this->Root_Folder);

					if (this->InitTreeViewItems(
						path.GetData(),
						RELOAD_MODE)
						)
					{
						if (this->SetExpandImage(expImage, nCount))
						{
							// ...
						}
					}
				}
				SafeDeleteArray((void**)&expImage);
			}
		}
		if (recoverScrollPosition)
		{
			if (sInfo.nPos == sInfo.nMin)
			{
				SendMessage(this->hwndTV, WM_VSCROLL, MAKEWPARAM(SB_TOP, 0), 0);
			}
			else if (sInfo.nPos == sInfo.nMax)
			{
				SendMessage(this->hwndTV, WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
			}
			else
			{
				SCROLLINFO tempInfo;
				tempInfo.cbSize = sizeof(SCROLLINFO);
				tempInfo.fMask = SIF_POS;

				//while (1)
				//{
				//	GetScrollInfo(this->hwndTV, SB_VERT, &tempInfo);

				//	if (tempInfo.nPos > sInfo.nPos)
				//	{
				//		SendMessage(this->hwndTV, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				//	}
				//	else if (tempInfo.nPos < sInfo.nPos)
				//	{
				//		SendMessage(this->hwndTV, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
				//	}
				//	else
				//	{
				//		break;
				//	}
				//}

				GetScrollInfo(this->hwndTV, SB_VERT, &tempInfo);

				if (tempInfo.nPos > sInfo.nPos)
				{
					while (1)
					{
						GetScrollInfo(this->hwndTV, SB_VERT, &tempInfo);

						if (tempInfo.nPos > sInfo.nPos)
							SendMessage(this->hwndTV, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
						else
							break;
					}
				}
				else if (tempInfo.nPos < sInfo.nPos)
				{
					while (1)
					{
						GetScrollInfo(this->hwndTV, SB_VERT, &tempInfo);

						if (tempInfo.nPos < sInfo.nPos)
							SendMessage(this->hwndTV, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
						else
							break;
					}
				}
			}
		}
		ShowWindow(this->hwndTV, SW_SHOW);
	}
}

BOOL TreeViewCTRL::SaveRootFolderSelection()
{
	return this->SaveRoot();
}

BOOL TreeViewCTRL::SaveExpandImage( LPCTSTR path )
{
	BOOL result = TRUE;
	LPHEADING EXPimage = NULL;

	int size = this->GetExpandImage( NULL, COUNT );

	if( size == -1 )
		return FALSE;
	else
	{
		if( size > 0 )
		{
			if( size == 1 )
			{
				EXPimage = new HEADING;

				if( EXPimage != NULL )
				{
					if( this->GetExpandImage( EXPimage, READ ) != -1 )
					{
						if( !this->SaveExpandImageToFile( path, EXPimage, size ))
						{
							result = FALSE;
						}
					}
					else
						result = FALSE;

					delete EXPimage;
				}
				else
					result = FALSE;
			}
			else
			{
				EXPimage = new HEADING[ size ];

				if( EXPimage != NULL )
				{
					if( this->GetExpandImage( EXPimage, READ ) != -1 )
					{
						if( !this->SaveExpandImageToFile( path, EXPimage, size ))
						{
							result = FALSE;
						}
					}
					else
						result = FALSE;

					delete [] EXPimage;
				}
				else
					result = FALSE;
			}
		}
		else
			this->DeleteImageFile( path );
	}
	return result;
}

BOOL TreeViewCTRL::LoadExpandImage( LPCTSTR path )
{
	BOOL result = TRUE;

	int size = this->LoadExpandImageFromFile( path, NULL, COUNT );

	if( size == -1 )
		return FALSE;
	else if( size == 0 )
		return TRUE;
	else
	{
		if( size == 1 )
		{
			LPHEADING EXPimage = new HEADING;

			if( EXPimage != NULL )
			{
				if( this->LoadExpandImageFromFile( path, EXPimage, READ ) == -1 )
				{
					result = FALSE;
				}
				else
				{
					if( !this->SetExpandImage( EXPimage, (DWORD)size ))
					{
						result = FALSE;
					}
				}
				delete EXPimage;
			}
			else
				result = FALSE;
		}
		else
		{
			LPHEADING EXPimage = new HEADING[ size ];

			if( EXPimage != NULL )
			{
				if( this->LoadExpandImageFromFile( path, EXPimage, READ ) == -1 )
				{
					result = FALSE;
				}
				else
				{
					if( !this->SetExpandImage( EXPimage, (DWORD)size ))
					{
						result = FALSE;
					}
				}
				delete [] EXPimage;
			}
		}
	}
	return TRUE;
}

BOOL TreeViewCTRL::GetSelectedItemPath(TCHAR ** path_out)
{
	return (BOOL)this->_getSelectedItemPath(path_out);
}

BOOL TreeViewCTRL::IsDragging_CursorCTRL(LPPOINT ppt)
{
	if (this->DragUnderway)
	{
		if (ppt != NULL)
		{
			RECT rc_nav, rc_edit;
			GetWindowRect(this->hwndTV, &rc_nav);
			_MSG_TO_MAIN(WM_GETABSEDITPOS, 0L, reinterpret_cast<LPARAM>(&rc_edit));

			// check if the dragging cursor is outside of the treeview-window
			if ((ppt->x < rc_nav.left) || (ppt->x > rc_nav.right) || (ppt->y < rc_nav.top) || (ppt->y > rc_nav.bottom))
			{
				// check if the dragging cursor is inside of the editwindow
				if ((ppt->x > rc_edit.left) && (ppt->x < rc_edit.right) && (ppt->y > rc_edit.top) && (ppt->y < rc_edit.bottom))
				{
					// if it is not a folder -> set import-symbol cursor
					if (this->FOPInfo.DragCursorType != A__FOLDER)
					{
						SetCursor(this->Obj.insert);
						this->FOPInfo.CursorOverEditBox = TRUE;
						this->FOPInfo.CursorOutOfValidArea = FALSE;
					}
				}
				else
				{
					// position not valid -> set forbidden cursor
					SetCursor(this->Obj.forbidden);

					this->FOPInfo.CursorOutOfValidArea = TRUE;
					this->FOPInfo.CursorOverEditBox = FALSE;

					// check if scrolling is required
					if ((ppt->x < rc_nav.right) && (ppt->x > rc_nav.left))
					{
						if (ppt->y > rc_nav.bottom)
						{
							this->ScrollDirection = -1;
						}
						else if (ppt->y < rc_nav.top)
						{
							this->ScrollDirection = 1;
						}
						else
						{
							this->ScrollDirection = 0;
						}
					}
					else
					{
						this->ScrollDirection = 0;
					}
				}
			}
			else
			{
				// the cursor is inside of the treeview-window
				// -> set the appropriate cursor
				switch (this->FOPInfo.DragCursorType)
				{
				case A__FILE:
					SetCursor(this->Obj.file);
					break;
				case A__FOLDER:
					SetCursor(this->Obj.folder);
					break;
				case A__CNC3FILE:
					SetCursor(this->Obj.cnc3);
					break;
				default:
					break;
				}
				this->FOPInfo.CursorOutOfValidArea = FALSE;
				this->FOPInfo.CursorOverEditBox = FALSE;
				this->ScrollDirection = 0;
			}
		}
	}
	return this->DragUnderway;
}

BOOL TreeViewCTRL::Recolor(COLORREF textcolor, COLORREF background)
{
	TreeView_SetBkColor(this->hwndTV, background);
	TreeView_SetTextColor(this->hwndTV, textcolor);

	return TRUE;
}

void TreeViewCTRL::SaveScroll()
{
	SecureZeroMemory(&this->internalScrollInfo, sizeof(SCROLLINFO));
	this->internalScrollInfo.cbSize = sizeof(SCROLLINFO);
	this->internalScrollInfo.fMask = SIF_POS;
	GetScrollInfo(this->hwndTV, SB_VERT, &this->internalScrollInfo);
}

void TreeViewCTRL::SetScrollPosition(LPSCROLLINFO sInfo)
{
	SCROLLINFO old;
	old.cbSize = sizeof(SCROLLINFO);
	old.fMask = SIF_ALL;
	
	if (GetScrollInfo(this->hwndTV, SB_VERT, &old))
	{
		if (sInfo == nullptr)
		{
			if (this->internalScrollInfo.fMask != 0)
			{
				this->internalScrollInfo.fMask = SIF_POS;

				SetScrollInfo(this->hwndTV, SB_VERT, &this->internalScrollInfo, TRUE);



				SecureZeroMemory(&this->internalScrollInfo, sizeof(SCROLLINFO));
			}
		}
		else
		{
			sInfo->fMask = SIF_POS;

			//InvalidateRect(this->hwndTV, nullptr, TRUE);
			SetScrollInfo(this->hwndTV, SB_VERT, sInfo, TRUE);

			SendMessage(this->hwndTV, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);

			//RedrawWindow(this->hwndTV, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);

			//GetScrollInfo(this->hwndTV, SB_VERT, sInfo);

			//if (sInfo->nPos != old.nPos)
			//{
			//	ScrollWindow(this->hwndTV, 0, old.nPos - sInfo->nPos, nullptr, nullptr);
			//}
		}
	}
}

BOOL TreeViewCTRL::GetTVScrollInfo(LPSCROLLINFO sInfo)
{
	if (sInfo != nullptr)
	{
		sInfo->cbSize = sizeof(SCROLLINFO);
		sInfo->fMask = SIF_ALL;

		return GetScrollInfo(this->hwndTV, SB_VERT, sInfo);
	}
	else
		return FALSE;
}

void TreeViewCTRL::FileSystem_AddItemToTree(LPCTSTR path)
{
	if (!this->internalFileOperationInProgress)
	{
		IPath _Path(this->Root_Folder);
		_Path += path;

		auto bResult = isFile(
			_Path.GetPathData()
		);

		if (bResult == FALSE)// directory
		{
			// that's a directory -> insert the content on basis of a filesystem-scan
			int type;

			auto item = this->getItemHandleFromRelativePath(path, &type, true);

			if (item == nullptr)
				item = TreeView_GetRoot(this->hwndTV);// yet to check

			if (item != nullptr)
			{
				if (type == -1)// the item-handle contains the last valid folder (the folder in the location where the path points to does not exist)
				{
					iString foldername;

					if (_Path.GetFolderName(foldername))
					{
						TCHAR Heading[MAX_HEADING_LEN] = { 0 };

						TVITEM tvi;
						tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
						tvi.hItem = item;
						tvi.pszText = Heading;
						tvi.cchTextMax = MAX_HEADING_LEN;

						if (TreeView_GetItem(this->hwndTV, &tvi))
						{
							auto pszFoldername = foldername.GetDataAsCopy();
							if (pszFoldername != nullptr)
							{
								tvi.lParam = this->GetItemLevel(item) + 1;
								tvi.pszText = pszFoldername;

								TVINSERTSTRUCT tvins;
								tvins.hParent = item;
								tvins.hInsertAfter =
									this->FindInsertAfter(
										item,
										PARENT__ITEM,
										pszFoldername
									);
								tvins.item = tvi;

								HTREEITEM newItem =
									TreeView_InsertItem(this->hwndTV, &tvins);

								if (newItem)
								{
									if (this->InsertFolder(newItem) == EMPTY_DIR)
									{
										// ...
									}
								}
								SafeDeleteArray((void**)&pszFoldername);
							}
						}
					}
				}
			}
		}
		else if (bResult == TRUE)
		{
			// insert file...
			int type;

			auto item = this->getItemHandleFromRelativePath(path, &type, true);

			if (item == nullptr)
				item = TreeView_GetRoot(this->hwndTV);

			if (item != nullptr)
			{
				if (type == -1)// item contains the last valid folder (the file in the location where the path points to does not exist)
				{
					iString filename;
					iString extension;

					if (_Path.GetFileExtension(extension))
					{
						auto isCnc3 = extension.Equals(L".cnc3");

						if (_Path.GetFileName(filename, isCnc3))
						{
							TCHAR Heading[MAX_HEADING_LEN] = { 0 };

							TVITEM tvi;
							tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT;
							tvi.hItem = item;
							tvi.pszText = Heading;
							tvi.cchTextMax = MAX_HEADING_LEN;

							if (TreeView_GetItem(this->hwndTV, &tvi))
							{
								auto pszFilename = filename.GetDataAsCopy();
								if (pszFilename != nullptr)
								{
									tvi.lParam = this->GetItemLevel(item) + 1;
									tvi.pszText = pszFilename;

									if (isCnc3)
									{
										tvi.iImage = this->ico_index[3];
										tvi.iSelectedImage = this->ico_index[3];
									}
									else
									{
										tvi.iImage = this->ico_index[0];
										tvi.iSelectedImage = this->ico_index[0];
									}

									TVINSERTSTRUCT tvins;
									tvins.hParent = item;
									tvins.hInsertAfter =
										this->FindInsertAfter(
											item,
											isCnc3 ? CNC3__ITEM : CHILD__ITEM,
											pszFilename
										);
									tvins.item = tvi;

									HTREEITEM newItem =
										TreeView_InsertItem(this->hwndTV, &tvins);

									UNREFERENCED_PARAMETER(newItem); // temp!

									SafeDeleteArray((void**)&pszFilename);
								}
							}
						}
					}

				}
			}
		}
		else
		{
			// error file/folder does not exist
		}
	}
}

void TreeViewCTRL::FileSystem_RenameItemInTree(LPCTSTR old_path, LPCTSTR new_path)
{
	if (!this->internalFileOperationInProgress)
	{
		//NOTE: the path's are relative to the rootfolderpath!
		int type;

		IPath NewPath(this->Root_Folder);
		NewPath += new_path;

		auto item =
			this->getItemHandleFromRelativePath(old_path, &type, false);

		if (item != nullptr)
		{
			auto bfpo = CreateBasicFPO();
			if (bfpo != nullptr)
			{
				TCHAR* filename = nullptr;

				if (bfpo->GetFilenameOutOfPath(new_path, &filename, FALSE) == TRUE)
				{
					auto bResult = isFile(
						NewPath.GetPathData()
					);

					if (bResult == TRUE)
					{
						TCHAR* ext = nullptr;

						if (bfpo->GetFileExtension(filename, &ext) == TRUE)
						{
							if (CompareStringsB(ext, L".cnc3"))
							{
								SafeDeleteArray((void**)&filename);

								if (bfpo->GetFilenameOutOfPath(new_path, &filename, TRUE) != TRUE)
								{
									filename = nullptr;
								}
							}
							if (filename != nullptr)
							{
								TVITEM tvi;
								tvi.mask = TVIF_HANDLE | TVIF_TEXT;
								tvi.hItem = item;
								tvi.pszText = filename;
								tvi.cchTextMax = _lengthOfString(filename);

								TreeView_SetItem(this->hwndTV, &tvi);
							}
							SafeDeleteArray((void**)&ext);
						}
					}
					else if (bResult == FALSE)
					{
						if (filename != nullptr)
						{
							TVITEM tvi;
							tvi.mask = TVIF_HANDLE | TVIF_TEXT;
							tvi.hItem = item;
							tvi.pszText = filename;
							tvi.cchTextMax = _lengthOfString(filename);

							TreeView_SetItem(this->hwndTV, &tvi);
						}
					}
					else
					{
						// error: the file at new_path does not exist
					}
					SafeDeleteArray((void**)&filename);
				}
				SafeRelease(&bfpo);
			}
		}
	}
}

void TreeViewCTRL::FileSystem_RefreshAllItems()
{
	if (!this->internalFileOperationInProgress)
	{
		this->ReloadAsync();
	}
}

void TreeViewCTRL::ReloadAsync()
{
	DWORD dwThreadID;
	HANDLE hThread;

	hThread = CreateThread(
		nullptr,
		0,
		TreeViewCTRL::reloadProc,
		reinterpret_cast<LPVOID>(this),
		0,
		&dwThreadID
	);
}

bool TreeViewCTRL::FileSystem_RemoveItemFromTree(LPCTSTR path)
{
	if (!this->internalFileOperationInProgress)
	{
		//NOTE: the path is relative to the rootfolder-path!

		int type;

		auto item =
			this->getItemHandleFromRelativePath(path, &type, false);

		if (item != nullptr)
		{
			auto parent = TreeView_GetParent(this->hwndTV, item);

			TreeView_DeleteItem(this->hwndTV, item);

			if (parent != nullptr)
			{
				if (TreeView_GetChild(this->hwndTV, parent) == nullptr)
				{
					this->InsertEmptyItem(parent);
				}
			}
		}
		return true;
	}
	else
	{
		return this->moveOperationInProgress ? false : true;
	}
}

void TreeViewCTRL::infoTipEnable(BOOL enable)
{
	this->useInfoTips = enable;

	if (this->hwndTV != nullptr)
	{
		auto styles = static_cast<DWORD>(
			GetWindowLong(this->hwndTV, GWL_STYLE)
			);

		if (enable && !(styles & TVS_INFOTIP))
		{
			styles |= TVS_INFOTIP;
			SetWindowLong(this->hwndTV, GWL_STYLE, (LONG)styles);
		}
		else if (!enable && (styles & TVS_INFOTIP))
		{
			styles &= ~(TVS_INFOTIP);
			SetWindowLong(this->hwndTV, GWL_STYLE, (LONG)styles);
		}
	}
}

int TreeViewCTRL::GetExpandImage( LPHEADING EXPimage, DWORD Mode  )
{
	if( ( EXPimage == NULL ) && ( Mode == READ ) )
		return -1;

	int size = 0;

	HTREEITEM root = TreeView_GetRoot( this->hwndTV );

	if( root )
	{
		HTREEITEM item = TreeView_GetChild( this->hwndTV, root );

		if( item )
		{
			TCHAR Text[ MAX_HEADING_LEN ] = { 0 };

			TVITEM tvi;
			tvi.mask = TVIF_PARAM | TVIF_STATE | TVIF_HANDLE | TVIF_TEXT | TVIF_IMAGE;
			tvi.hItem = item;
			tvi.pszText = Text;
			tvi.cchTextMax = MAX_HEADING_LEN;

			if( TreeView_GetItem( this->hwndTV, &tvi ))
			{
				if( ( tvi.iImage == this->ico_index[ 2 ] ) && ( tvi.state & TVIS_EXPANDED ) )
				{
					int temp = size;

					if( Mode == COUNT )
					{
						if( !this->GetNextLevel_Image( NULL, item, COUNT, size ) )
						{
							size = -1;
							goto End;
						}
					}
					else if(Mode == READ)
					{
						EXPimage[ size ].Level = (int)tvi.lParam;
						EXPimage[ size ].type = tvi.iImage;
						StringCbCopy( EXPimage[ size ].Heading, sizeof( TCHAR )* MAX_HEADING_LEN, Text );

						if( !this->GetNextLevel_Image( EXPimage, item, READ, size ) )
						{
							size = -1;
							goto End;
						}	
					}
					if( temp == size )
						size++;
				}
				while( ( item = TreeView_GetNextSibling( this->hwndTV, item )) != NULL )
				{
					if( item )
					{
						tvi.hItem = item;

						if( TreeView_GetItem( this->hwndTV, &tvi ))
						{
							if( ( tvi.iImage == this->ico_index[ 2 ] ) && ( tvi.state & TVIS_EXPANDED ) )
							{
								int temp = size;

								if( Mode == COUNT )
								{
									if( !this->GetNextLevel_Image( NULL, item, COUNT, size ) )
									{
										size = -1;
										break;
									}
								}
								else if(Mode == READ)
								{
									EXPimage[ size ].Level = (int)tvi.lParam;
									EXPimage[ size ].type = tvi.iImage;
									StringCbCopy( EXPimage[ size ].Heading, sizeof( TCHAR )* MAX_HEADING_LEN, Text );

									if( !this->GetNextLevel_Image( EXPimage, item, READ, size ) )
									{
										size = -1;
										break;
									}	
								}
								if( temp == size )
									size++;
							}
						}
					}
				}
			}
		}
	}
	else
		size = -1;
End:
	return size;
}

BOOL TreeViewCTRL::SetExpandImage( LPHEADING EXPimage, DWORD maxIndex  )
{
	if( EXPimage == NULL )
		return FALSE;

	DWORD index = 0;

	HTREEITEM root = TreeView_GetRoot( this->hwndTV );

	if( root )
	{
		HTREEITEM item = TreeView_GetChild( this->hwndTV, root );

		if( item )
		{
			TCHAR Text[ MAX_HEADING_LEN ] = { 0 };

			TVITEM tvi;
			tvi.mask = TVIF_PARAM | TVIF_STATE | TVIF_HANDLE | TVIF_TEXT | TVIF_IMAGE;
			tvi.hItem = item;
			tvi.pszText = Text;
			tvi.cchTextMax = MAX_HEADING_LEN;

			if( TreeView_GetItem( this->hwndTV, &tvi ))
			{
				if( tvi.iImage == this->ico_index[ 1 ] )
				{
					if(index > maxIndex )
						return TRUE;

					DWORD Tmp_idx = index;

					if( ( this->CompareHeading( Text, EXPimage[ index ].Heading ) &&
						( tvi.lParam == EXPimage[ index ].Level ) ) )
					{
						if( !TreeView_Expand( this->hwndTV, tvi.hItem, TVE_EXPAND ))
						{
							return FALSE;
						}
						else
						{
							if( !this->SetNextLevel_Image( EXPimage, item, index, maxIndex ))
							{
								return FALSE;
							}
						}
						if( Tmp_idx == index )
							index++;
					}
				}
				while( ( item = TreeView_GetNextSibling( this->hwndTV, item )) != NULL )
				{
					if( item )
					{
						tvi.hItem = item;

						if( TreeView_GetItem( this->hwndTV, &tvi ))
						{
							if( tvi.iImage == this->ico_index[ 1 ] )
							{
								if( index > maxIndex )
									return TRUE;

								DWORD Tmp_idx = index;

								if( ( this->CompareHeading( Text, EXPimage[ index ].Heading ) &&
									( tvi.lParam == EXPimage[ index ].Level ) ) )
								{
									if( !TreeView_Expand( this->hwndTV, tvi.hItem, TVE_EXPAND ))
									{
										return FALSE;
									}
									else
									{
										if( !this->SetNextLevel_Image( EXPimage, item, index, maxIndex ))
										{
											return FALSE;
										}
									}
									if( Tmp_idx == index )
										index++;
								}
							}
						}
					}
				}
			}
		}
	}
	else
		return FALSE;

	return TRUE;
}

BOOL TreeViewCTRL::GetNextLevel_Image( LPHEADING EXPimage, HTREEITEM Parent, DWORD Mode, int& size )
{
	if( ( EXPimage == NULL ) && ( Mode == READ ) )
		return FALSE;

	BOOL result = TRUE;
	size++;

	if( Parent )
	{
		HTREEITEM item = TreeView_GetChild( this->hwndTV, Parent );

		if( item )
		{
			TCHAR Text[ MAX_HEADING_LEN ] = { 0 };

			TVITEM tvi;
			tvi.mask = TVIF_PARAM | TVIF_STATE | TVIF_HANDLE | TVIF_TEXT | TVIF_IMAGE;
			tvi.hItem = item;
			tvi.pszText = Text;
			tvi.cchTextMax = MAX_HEADING_LEN;

			if( TreeView_GetItem( this->hwndTV, &tvi ))
			{
				if( ( tvi.iImage == this->ico_index[ 2 ] ) && ( tvi.state & TVIS_EXPANDED ) )
				{
					int temp = size;

					if( Mode == COUNT )
					{
						if( !this->GetNextLevel_Image( NULL, item, COUNT, size ) )
						{
							result = FALSE;
							goto End;
						}
					}
					else if(Mode == READ)
					{
						EXPimage[ size ].Level = (int)tvi.lParam;
						EXPimage[ size ].type = tvi.iImage;
						StringCbCopy( EXPimage[ size ].Heading, sizeof( TCHAR )* MAX_HEADING_LEN, Text );

						if( !this->GetNextLevel_Image( EXPimage, item, READ, size ) )
						{
							result = FALSE;
							goto End;
						}	
					}
					if( temp == size )
						size++;
				}
				while( ( item = TreeView_GetNextSibling( this->hwndTV, item )) != NULL )
				{
					if( item )
					{
						tvi.hItem = item;

						if( TreeView_GetItem( this->hwndTV, &tvi ))
						{
							if( ( tvi.iImage == this->ico_index[ 2 ] ) && ( tvi.state & TVIS_EXPANDED ) )
							{
								int temp = size;

								if( Mode == COUNT )
								{
									if( !this->GetNextLevel_Image( NULL, item, COUNT, size ) )
									{
										result = FALSE;
										break;
									}
								}
								else if(Mode == READ)
								{
									EXPimage[ size ].Level = (int)tvi.lParam;
									EXPimage[ size ].type = tvi.iImage;
									StringCbCopy( EXPimage[ size ].Heading, sizeof( TCHAR )* MAX_HEADING_LEN, Text );

									if( !this->GetNextLevel_Image( EXPimage, item, READ, size ) )
									{
										result = FALSE;
										break;
									}	
								}
								if( temp == size )
									size++;
							}
						}
					}
				}
			}
		}
	}
	else
		result = FALSE;
End:
	return result;
}

int TreeViewCTRL::LoadExpandImageFromFile( LPCTSTR path, LPHEADING EXPimage, DWORD Mode )
{
	if( ( EXPimage == NULL ) && ( Mode == READ ) )
		return -1;

	int size = 0;

	HRESULT hr;
	size_t len;

	hr = StringCbLength( path, STRSAFE_MAX_CCH, &len );
	if( SUCCEEDED( hr ))
	{
		TCHAR *CMPL_PATH = new TCHAR[ len + ( 20*sizeof(TCHAR)) ];

		if( CMPL_PATH != NULL )
		{
			hr = StringCbPrintf( CMPL_PATH, len + ( 20*sizeof(TCHAR)), L"%s\\TVimage.dat\0", path );
			if( SUCCEEDED( hr ))
			{
				auto bfpo = CreateBasicFPO();
				if (bfpo != nullptr)
				{
					TCHAR *buffer = nullptr;

					auto res = bfpo->LoadBufferFmFileAsUtf8(&buffer, CMPL_PATH);
					if (res)
					{
						if (!this->Read_Image_From_Buffer(EXPimage, buffer, size, Mode, GetCharCount(buffer)))
						{
							size = -1;
						}
						SafeDeleteArray(&buffer);
					}					
					SafeRelease(&bfpo);
				}

				//LARGE_INTEGER lg;
				//HANDLE hFile;

				//hFile = CreateFile( CMPL_PATH, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

				//if( hFile != INVALID_HANDLE_VALUE )
				//{
				//	if( GetFileSizeEx( hFile, &lg ))
				//	{
				//		TCHAR *buffer = new TCHAR[ (( size_t )lg.LowPart + sizeof( TCHAR )) ];

				//		if( buffer != NULL )
				//		{
				//			DWORD bytesRead = 0;

				//			if( ReadFile( hFile, buffer, lg.LowPart, &bytesRead, NULL ))
				//			{
				//				buffer[ (DWORD)(bytesRead/sizeof(TCHAR)) ] = L'\0';

				//				//MessageBox(NULL,buffer, L"Monitor", MB_OK );/////////////////////////////////////////////////// !!!!!!!!!!!!!!!!!!!

				//				if( !this->Read_Image_From_Buffer( EXPimage, buffer, size, Mode, ( bytesRead/sizeof(TCHAR)) ))
				//				{
				//					size = -1;
				//				}
				//			}
				//			else
				//				size = -1;
				//			
				//			delete [] buffer;
				//		}
				//		else
				//			size = -1;
				//	}
				//	else
				//		size = -1;

				//	CloseHandle( hFile );
				//}//
				//else
					//size = 0;// indicates that there is no file, so no expand action has to be done
			}
			else
				size = -1;

			delete[] CMPL_PATH;
		}
		else
			size = -1;
	}
	else
		size = -1;

	return size;
}

BOOL TreeViewCTRL::SaveExpandImageToFile( LPCTSTR path, LPHEADING EXPimage, int size )
{
	if( size == 0 )
		return TRUE;

	HRESULT hr;
	size_t len;
	BOOL result = TRUE;

	hr = StringCbLength( path, STRSAFE_MAX_CCH, &len );
	if( SUCCEEDED( hr ))
	{
		TCHAR *CMPL_PATH = new TCHAR[ len + ( 20*sizeof(TCHAR)) ];

		if( CMPL_PATH != NULL )
		{
			hr = StringCbPrintf( CMPL_PATH, len + ( 20*sizeof(TCHAR)), L"%s\\TVimage.dat\0", path );
			if( SUCCEEDED( hr ))
			{
				size_t buffer_size = ( size_t )( size * ( sizeof( TCHAR ) * ( MAX_HEADING_LEN + 10 )));

				TCHAR *buffer = new TCHAR[ buffer_size ];

				if( buffer != NULL )
				{
					SecureZeroMemory( buffer, buffer_size );

					hr = StringCbPrintf( buffer, buffer_size, L"%i|%i|%s\r\n\0", EXPimage[ 0 ].type, EXPimage[ 0 ].Level, EXPimage[ 0 ].Heading );
					if( SUCCEEDED( hr ))
					{
						if( size > 1 )
						{
							TCHAR cat_buf[ 280 ] = { 0 };

							for( int i = 1; i < size; i++ )
							{
								hr = StringCbPrintf( cat_buf, sizeof( cat_buf ), L"%i|%i|%s\r\n\0", EXPimage[ i ].type, EXPimage[ i ].Level, EXPimage[ i ].Heading );
								if( SUCCEEDED( hr ))
								{
									hr = StringCbCat( buffer, buffer_size, cat_buf );
									if( FAILED( hr ))
									{
										result = FALSE;
										break;
									}
								}
								else
								{
									result = FALSE;
									break;
								}
							}
						}


						auto bfpo = CreateBasicFPO();
						if (bfpo != nullptr)
						{
							bfpo->SaveBufferToFileAsUtf8(buffer, CMPL_PATH);
							SafeRelease(&bfpo);
						}


						//HANDLE hFile = CreateFile( CMPL_PATH, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

						//if( hFile == INVALID_HANDLE_VALUE )
						//{
						//	result = FALSE;

						//}
						//else
						//{
						//	size_t bytes;

						//	hr = StringCbLength( buffer, buffer_size, &bytes );
						//	if( SUCCEEDED( hr ))
						//	{
						//		DWORD bytesWritten = 0;
						//		DWORD bytesToWrite = (DWORD)bytes;

						//		if( !WriteFile( hFile, buffer, bytesToWrite, &bytesWritten, NULL ))
						//		{
						//			result = FALSE;
						//		}
						//		if( bytesWritten == 0 )
						//		{
						//			result = FALSE;
						//		}
						//	}
						//	else
						//		result = FALSE;

						//	CloseHandle( hFile );
						//}
					}
					else
						result = FALSE;

					delete [] buffer;
				}
				else
					result = FALSE;
			}
			else
				result = FALSE;

			delete [] CMPL_PATH;
		}
		else
			result = FALSE;
	}
	else
		result = FALSE;

	return result;
}

BOOL TreeViewCTRL::DeleteImageFile( LPCTSTR path )
{
	BOOL result = TRUE;
	HRESULT hr;
	size_t len;

	hr = StringCbLength( path, STRSAFE_MAX_CCH, &len );
	if( SUCCEEDED( hr ))
	{
		TCHAR *CMPL_PATH = new TCHAR[ len + ( 20*sizeof(TCHAR)) ];

		if( CMPL_PATH != NULL )
		{
			hr = StringCbPrintf( CMPL_PATH, len + ( 20*sizeof(TCHAR)), L"%s\\TVimage.dat\0", path );
			if( SUCCEEDED( hr ))
			{
				result = DeleteFile( CMPL_PATH );
			}
			else
				result = FALSE;

			delete [] CMPL_PATH;
		}
		else
			result = FALSE;
	}
	else
		result = FALSE;


	return result;
}

BOOL TreeViewCTRL::Read_Image_From_Buffer( LPHEADING EXPimage, LPTSTR buffer, int& size, DWORD Mode, int max )
{
	if( max <= 0 )
		return FALSE;

	if( ( Mode == READ ) && ( EXPimage == NULL ) )
		return FALSE;

	__try
	{
		int i = 0;

		while( ( i < max ) && ( buffer[ i ] != L'\0' ) )
		{
			if(( buffer[ i ] == 0x0D ))
				size++;

			i++;
		}
		i = 0;

		if( Mode == READ )
		{
			// syntactic scan >>
			for( int a = 0; a < size; a++ )
			{
				while( !this->CheckForNumber( buffer[ i ] ))
				{
					if( buffer[ i ] == L'\0' )
						return TRUE;

					i++;
				}
				while( ( i < max ) )
				{
					if( buffer[ i ] == L'\0' )
						break;

					TCHAR over[ MAX_HEADING_LEN ] = { 0 };
					int j = 0;

					while( this->CheckForNumber( buffer[ i ] ))
					{
						over[ j ] = buffer[ i ];
						j++;
						i++;
					}
					i++;
					j++;
					over[ j ] = L'\0';

					//MessageBox( NULL,over,L"111",MB_OK);///////////////////////

					if( swscanf_s( over, L"%i\0", &EXPimage[ a ].type ) == 0 )
					{
						return FALSE;
					}
					j = 0;
					SecureZeroMemory( over, sizeof( over ));

					while( this->CheckForNumber( buffer[ i ] ))
					{
						over[ j ] = buffer[ i ];
						j++;
						i++;
					}
					i++;
					j++;
					over[ j ] = L'\0';

					//MessageBox( NULL,over,L"222",MB_OK);//////////////////////////////

					if( swscanf_s( over, L"%i\0", &EXPimage[ a ].Level ) == 0 )
					{
						return FALSE;
					}
					j = 0;
					SecureZeroMemory( over, sizeof( over ));

					while( buffer[ i ] != 0x0D )
					{
						if( ( buffer[ i ] == L'\0' ) || ( buffer[ i ] == L'\n' ) )
							break;

						over[ j ] = buffer[ i ];
						j++;
						i++;
					}
					
					j++;
					over[ j ] = L'\0';

					//MessageBox( NULL,over,L"333",MB_OK);//////////////////////////

					if( FAILED( StringCbCopy( EXPimage[ a ].Heading, sizeof( TCHAR )* MAX_HEADING_LEN, over )))
					{
						return FALSE;
					}
					break;
				}
				i++;

				if( buffer[ i ] == L'\0' )
					break;
			}
		}
	}
	__except( GetExceptionCode( ) == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		return FALSE;
	}
	return TRUE;
}

BOOL TreeViewCTRL::CheckForNumber( TCHAR LT )
{
	BOOL result = FALSE;

	if( ( LT == '0' ) ||
		( LT == '1' ) ||
		( LT == '2' ) ||
		( LT == '3' ) ||
		( LT == '4' ) ||
		( LT == '5' ) ||
		( LT == '6' ) ||
		( LT == '7' ) ||
		( LT == '8' ) ||
		( LT == '9' )	)
	{
		result = TRUE;
	}
	return result;
}

void TreeViewCTRL::DisplayItemstruct(LPHEADING pitem, int index)
{
	HRESULT hr;
	TCHAR buffer[1024] = { 0 };

	hr = StringCbPrintf(buffer, sizeof(buffer), L"HEADING >>   %s\nLEVEL >>   %i\nDATENTYP >>   \0", pitem[index].Heading, pitem[index].Level);
	if (SUCCEEDED(hr))
	{
		if (pitem[index].type == A__CNC3FILE)
			StringCbCat(buffer, sizeof(buffer), L"CnC3 - Datei\0");
		else if(pitem[index].type == A__FILE)
			StringCbCat(buffer, sizeof(buffer), L"*.* - Datei\0");
		else if (pitem[index].type == A__FOLDER)
			StringCbCat(buffer, sizeof(buffer), L"Ordner\0");
		else if (pitem[index].type == A__EMPTYITEM)
			StringCbCat(buffer, sizeof(buffer), L"leer  - descriptor\0");
		else if (pitem[index].type == EMPTY_DIR )
			StringCbCat(buffer, sizeof(buffer), L"leerer Ordner\0");
		else
			StringCbCat(buffer, sizeof(buffer), L"FEHLER - UNBEKANNTER INDEX !\0");

		MessageBox(this->TVFrame, buffer, L"Iteminfo", MB_OK | MB_ICONINFORMATION);
	}
}

BOOL TreeViewCTRL::SetNextLevel_Image( LPHEADING EXPimage, HTREEITEM Parent, DWORD& index, DWORD maxIndex )
{
	if( EXPimage == NULL )
		return FALSE;

	index++;

	if( Parent )
	{
		HTREEITEM item = TreeView_GetChild( this->hwndTV, Parent );

		if( item )
		{
			TCHAR Text[ MAX_HEADING_LEN ] = { 0 };

			TVITEM tvi;
			tvi.mask = TVIF_PARAM | TVIF_STATE | TVIF_HANDLE | TVIF_TEXT | TVIF_IMAGE;
			tvi.hItem = item;
			tvi.pszText = Text;
			tvi.cchTextMax = MAX_HEADING_LEN;

			if( TreeView_GetItem( this->hwndTV, &tvi ))
			{
				if( tvi.iImage == this->ico_index[ 1 ] )
				{
					if( index > maxIndex )
						return TRUE;

					DWORD Tmp_idx = index;

					if( ( this->CompareHeading( Text, EXPimage[ index ].Heading ) &&
						( tvi.lParam == EXPimage[ index ].Level ) ) )
					{
						if( !TreeView_Expand( this->hwndTV, tvi.hItem, TVE_EXPAND ))
						{
							return FALSE;
						}
						else
						{
							if( !this->SetNextLevel_Image( EXPimage, item, index, maxIndex ))
							{
								return FALSE;
							}
						}
						if( Tmp_idx == index )
							index++;
					}
				}
				while( ( item = TreeView_GetNextSibling( this->hwndTV, item )) != NULL )
				{
					if( item )
					{
						tvi.hItem = item;

						if( TreeView_GetItem( this->hwndTV, &tvi ))
						{
							if( tvi.iImage == this->ico_index[ 1 ] )
							{
								if( index > maxIndex )
									return TRUE;

								DWORD Tmp_idx = index;

								if( ( this->CompareHeading( Text, EXPimage[ index ].Heading ) &&
									( tvi.lParam == EXPimage[ index ].Level ) ) )
								{
									if( !TreeView_Expand( this->hwndTV, tvi.hItem, TVE_EXPAND ))
									{
										return FALSE;
									}
									else
									{
										if( !this->SetNextLevel_Image( EXPimage, item, index, maxIndex ))
										{
											return FALSE;
										}
									}
									if( Tmp_idx == index )
										index++;
								}
							}
						}
					}
				}
			}
		}
	}
	else
		return FALSE;

	return TRUE;
}

BOOL TreeViewCTRL::RemoveWildcard(TCHAR *path)
{
	__try
	{
		int i = 0;

		while (path[i] != L'*')
		{
			if (path[i] == L'\0')
				return TRUE;

			i++;
		}
		path[i] = L'\0';
		path[i - 1] = L'\0';
	}
	__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL TreeViewCTRL::RemoveFilename(TCHAR *destination)
{
	__try
	{
		int i = 0;

		while (destination[i] != L'\0')
		{
			i++;
		}
		while (destination[i] != L'\\')
		{
			if (i == 0)
				return FALSE;

			destination[i] = L'\0';
			i--;
		}
		destination[i] = L'\0';
	}
	__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL TreeViewCTRL::RemoveFileExt(int type, WCHAR *buffer)
{
	if (type != CNC3__ITEM)
		return TRUE;
	else
	{
		__try
		{
			int i = 0;

			while (buffer[i] != L'\0')
			{
				i++;
			}
			i--;

			if (i < 5)
				return FALSE;

			if (buffer[i] != L'3')
				return FALSE;
			else
			{
				buffer[i] = L'\0';
				i--;

				if (buffer[i] != L'c')
					return FALSE;
				else
				{
					buffer[i] = L'\0';
					i--;

					if (buffer[i] != L'n')
						return FALSE;
					else
					{
						buffer[i] = L'\0';
						i--;

						if (buffer[i] != L'c')
							return FALSE;
						else
						{
							buffer[i] = L'\0';
							i--;
							
							if (buffer[i] != L'.')
								return FALSE;
							else
							{
								buffer[i] = L'\0';
								i--;

								return TRUE;
							}

						}
					}
				}
			}
		}
		__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			return FALSE;
		}
	}
}

HTREEITEM TreeViewCTRL::FindInsertAfter(HTREEITEM parent, int type, LPCTSTR name)
{
	TCHAR buffer[MAX_HEADING_LEN] = { 0 };
	HTREEITEM insertAfter = NULL;
	HTREEITEM child = TreeView_GetChild(this->hwndTV, parent);
	HTREEITEM prev = NULL;
	HTREEITEM sortitem = NULL;
	int nCount = 0;

	// in the first loop, search for special cases (e.g. empty-folder-item)
	do
	{
		TVITEM tvi;
		tvi.mask = TVIF_IMAGE | TVIF_HANDLE | TVIF_TEXT;
		tvi.hItem = child;
		tvi.pszText = buffer;
		tvi.cchTextMax = MAX_HEADING_LEN;

		if (TreeView_GetItem(this->hwndTV, &tvi))
		{
			if (CompareStringsB(buffer, name))
			{
				sortitem = child; // this is the item which should be sorted, this cannot be insertAfter, save it for comparison
			}

			if ((type == PARENT__ITEM) && ((tvi.iImage == this->ico_index[0]) || (tvi.iImage == this->ico_index[3])))// end of folder section
			{
				if (prev == sortitem)// the end-item of the folder-section is the sortitem, choose the previous item or TVI_FIRST if there is no previous item
				{
					if (nCount <= 1)
					{
						insertAfter = TVI_FIRST;
					}
				}
				else
				{
					if (nCount == 0)// there are no other folders in this folder -> insert at the beginning
					{
						insertAfter = TVI_FIRST;
					}
				}
				break;
			}
			else if (tvi.iImage == this->ico_index[4])// this was an empty folder -> remove empty-item and set insertAfter to the first
			{
				TreeView_DeleteItem(this->hwndTV, child);
				insertAfter = TVI_FIRST;
				break;
			}
			else if (((type == CHILD__ITEM)||(type == CNC3__ITEM)) && ((tvi.iImage == this->ico_index[1]) || (tvi.iImage == this->ico_index[2])))
			{
				// since all files should appear behind the folders, this should never be executed!
			}
		}
		nCount++;

		prev = child; // save last item

	} while ((child = TreeView_GetNextSibling(this->hwndTV, child)) != NULL);

	// if the item shouldn't be inserted at the beginning -> search insert-point
	if (insertAfter != TVI_FIRST)
	{
		bool saveInsertAfter = true;
		insertAfter = TVI_FIRST;

		child = TreeView_GetChild(this->hwndTV, parent);
		if (child)
		{
			do
			{
				TVITEM tvi;
				tvi.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_HANDLE;
				tvi.hItem = child;
				tvi.pszText = buffer;
				tvi.cchTextMax = MAX_HEADING_LEN;

				if (TreeView_GetItem(this->hwndTV, &tvi))
				{
					if ((type == CHILD__ITEM) || (type == CNC3__ITEM))
					{
						if ((tvi.iImage == this->ico_index[0]) || (tvi.iImage == this->ico_index[3]))
						{
							int cmp = CompareStringsForSort(buffer, name);
							if (cmp == -1)
							{
								break;
							}
							else if (cmp == 0)
							{
								saveInsertAfter = false; // block saving
							}
						}
					}
					else if ((type == PARENT__ITEM) || (type == ROOT__ITEM))
					{
						if ((tvi.iImage == this->ico_index[0]) || (tvi.iImage == this->ico_index[3]))
						{
							break;
						}
						else
						{
							int cmp = CompareStringsForSort(buffer, name);
							if (cmp == -1)
							{
								break;
							}
							else if (cmp == 0)
							{
								saveInsertAfter = false; // block saving
							}
						}
					}
				}
				if (saveInsertAfter) // only save insertAfter if the item is not the sortitem!!!
					insertAfter = child;
				else
					saveInsertAfter = true;

			} while ((child = TreeView_GetNextSibling(this->hwndTV, child)) != NULL);
		}
	}
	if ((insertAfter == NULL) || (insertAfter == sortitem))
	{
		insertAfter = TVI_FIRST;
	}
	return insertAfter;
}

BOOL TreeViewCTRL::InsertFolder(HTREEITEM Folderitem)
{
	TVITEM tvi_;
	tvi_.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE;
	tvi_.hItem = Folderitem;
	TreeView_GetItem(this->hwndTV, &tvi_);

	BOOL result = TRUE;

	TCHAR* destination = NULL;

	int type = this->GetItemPath(Folderitem, &destination);

	result = (type < 0) ? FALSE : TRUE;
	if (result)
	{
		int C_Items = 0, count = 0, level = (int)tvi_.lParam;
		type = 0;

		HRESULT hr;
		WIN32_FIND_DATA ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		size_t buffer_len, fName_len;

		hr = StringCbLength(destination, MAX_FILEPATH_LEN, &buffer_len);
		if (SUCCEEDED(hr))
		{
			TCHAR* path_buffer = new TCHAR[buffer_len + 15];

			hr = (path_buffer != nullptr) ? S_OK : E_OUTOFMEMORY;
			if (SUCCEEDED(hr))
			{
				hr = StringCbPrintf(path_buffer, sizeof(TCHAR) * (buffer_len + 1), L"\\\\?\\%s\\*", destination);
				if (SUCCEEDED(hr))
				{
					C_Items = this->CountItems(destination);

					//show_integer(C_Items, 0);

					hr = (C_Items < 0) ? E_FAIL : S_OK;
					if (SUCCEEDED(hr))
					{
						LPHEADING items = new HEADING[C_Items + 1];

						hr = (items == nullptr) ? E_OUTOFMEMORY : S_OK;
						if (SUCCEEDED(hr))
						{
							hFind = FindFirstFile(path_buffer, &ffd);

							hr = (hFind == INVALID_HANDLE_VALUE) ? E_HANDLE : S_OK;
							if (SUCCEEDED(hr))
							{
								while (ffd.cFileName[0] == '.')
								{
									if (FindNextFile(hFind, &ffd) == 0)
									{
										type = EMPTY_DIR;
										break;
									}
								}
								do// SUB
								{
									if (type == EMPTY_DIR)
									{
										type = 0;
										
										// set empty child item
										items[count].Level = level + 1;
										items[count].type = A__EMPTYITEM;
										StringCbCopy(items[count].Heading, sizeof(TCHAR)*MAX_HEADING_LEN, L"< . . . >\0");
										count++;
										result = EMPTY_DIR;

										break;
									}
									if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
									{
										items[count].Level = level;
										items[count].type = A__FOLDER;

										hr = StringCbCopy(items[count].Heading, sizeof(TCHAR) * MAX_HEADING_LEN, ffd.cFileName);
										if (SUCCEEDED(hr))
										{
											count++;

											WIN32_FIND_DATA ffd_sub;
											HANDLE hFind_sub = INVALID_HANDLE_VALUE;

											hr = StringCbLength(ffd.cFileName, STRSAFE_MAX_CCH * sizeof(TCHAR), &fName_len);
											if (SUCCEEDED(hr))
											{
												TCHAR* subitem1 = new TCHAR[(buffer_len + fName_len + 1)];

												hr = (subitem1 == nullptr) ? E_OUTOFMEMORY : S_OK;
												if (SUCCEEDED(hr))
												{
													hr = StringCbPrintf(subitem1, sizeof(TCHAR) * (buffer_len + fName_len + 1), L"\\\\?\\%s\\%s\\*", destination, ffd.cFileName);
													if (SUCCEEDED(hr))
													{
														hFind_sub = FindFirstFile(subitem1, &ffd_sub);

														hr = (hFind_sub == INVALID_HANDLE_VALUE) ? E_HANDLE : S_OK;
														if (SUCCEEDED(hr))
														{
															while (ffd_sub.cFileName[0] == '.')
															{
																if (FindNextFile(hFind_sub, &ffd_sub) == 0)
																{
																	type = EMPTY_DIR;
																	break;
																}
															}
															if (type != EMPTY_DIR)
															{
																// next level ....
																hr = this->InitNextLevelItems(hFind_sub, &ffd_sub, items, subitem1, type, level + 1, count);
															}
															else
															{
																type = 0;
																// set empty child item
																items[count].Level = level + 1;
																items[count].type = A__EMPTYITEM;
																StringCbCopy(items[count].Heading, sizeof(TCHAR)*MAX_HEADING_LEN, L"< . . . >\0");
																count++;
															}
															FindClose(hFind_sub);
														}
													}
													delete[] subitem1;
												}
											}
										}
									}
									else
									{
										items[count].Level = level;
										items[count].type = A__FILE;
										StringCbCopy(items[count].Heading, sizeof(TCHAR) * MAX_HEADING_LEN, ffd.cFileName);
										count++;
									}
								} while (FindNextFile(hFind, &ffd) != 0);

								FindClose(hFind);

								LPHEADING a_items = new HEADING[count];

								hr = (a_items != NULL) ? S_OK : E_OUTOFMEMORY;
								if (SUCCEEDED(hr))
								{
									hr = this->Sequencing(SQC_FOLDER, a_items, items, count) ? S_OK : E_FAIL;
									if (SUCCEEDED(hr))
									{
										int level_subtractor = a_items[0].Level;
										TVITEM tvi;
										TVINSERTSTRUCT tvins;
										tvins.hParent = Folderitem;

										HTREEITEM hti = Folderitem;

										for (int k = 0; k < count; k++)
										{
											tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
											tvi.pszText = a_items[k].Heading;
											tvi.lParam = (LPARAM)a_items[k].Level;

											if (a_items[k].type == A__FILE)
											{
												tvi.iImage = ico_index[0];
												tvi.iSelectedImage = ico_index[0];
											}
											else if (a_items[k].type == A__FOLDER)
											{
												tvi.iImage = ico_index[1];
												tvi.iSelectedImage = ico_index[1];
											}
											else if (a_items[k].type == A__CNC3FILE)
											{
												tvi.iImage = ico_index[3];
												tvi.iSelectedImage = ico_index[3];
											}
											else if (a_items[k].type == A__EMPTYITEM)
											{
												tvi.iImage = ico_index[4];
												tvi.iSelectedImage = ico_index[4];
											}
											else
											{
												tvi.iImage = ico_index[0];
												tvi.iSelectedImage = ico_index[0];
											}
											tvins.hInsertAfter = hti;
											tvins.item = tvi;
											tvins.hParent = this->_dynamic_level_provider_(DYNAMICMODE_GETPARENTITEMFROMLEVEL, a_items[k].Level - level_subtractor, hti);

											if (tvins.hParent == TVI_ROOT)
												tvins.hParent = Folderitem;
											
											hti = TreeView_InsertItem(this->hwndTV, &tvins);
											if (!hti)
											{
												result = FALSE;
												break;
											}
											this->_dynamic_level_provider_(DYNAMICMODE_SAVEITEMHANDLE, a_items[k].Level - level_subtractor, hti);

											if ((a_items[k].Level - level_subtractor > 0))
											{
												hti = TreeView_GetParent(this->hwndTV, hti);
												tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
												tvi.hItem = hti;
												tvi.iImage = this->ico_index[1];
												tvi.iSelectedImage = this->ico_index[1];
												TreeView_SetItem(this->hwndTV, &tvi);
											}
										}
										if (result)
										{
											if (tvi_.iImage == this->ico_index[2])
												TreeView_Expand(this->hwndTV, Folderitem, TVE_EXPAND);
										}
									}
									delete[] a_items;
								}
							}
							delete[] items;
						}
					}
				}
			}
			delete[] path_buffer;
		}
	}
	return result;
}

BOOL TreeViewCTRL::InsertEmptyItem(HTREEITEM Parent)
{
	TVITEM par_tvi;
	par_tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_IMAGE;
	par_tvi.hItem = Parent;
	TreeView_GetItem(this->hwndTV, &par_tvi);

	TVITEM tvi;
	tvi.mask = TVIF_IMAGE | TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvi.pszText = L"< . . . >\0";
	tvi.cchTextMax = 8;
	tvi.iImage = this->ico_index[4];
	tvi.iSelectedImage = this->ico_index[4];
	tvi.lParam = (LPARAM)((int)par_tvi.lParam + 1);

	TVINSERTSTRUCT tvins;
	tvins.hInsertAfter = TVI_FIRST;
	tvins.hParent = Parent;
	tvins.item = tvi;

	HTREEITEM item = TreeView_InsertItem(this->hwndTV, &tvins);

	if (par_tvi.iImage == this->ico_index[2])
		TreeView_Expand(this->hwndTV, Parent, TVE_EXPAND);

	return (item) ? TRUE : FALSE;
}

HTREEITEM TreeViewCTRL::SortItem(HTREEITEM item)
{
	int type = this->CheckItemType(item);
	if (type > 0)
	{
		// new
		auto prev = TreeView_GetPrevSibling(this->hwndTV, item);
		auto next = TreeView_GetNextSibling(this->hwndTV, item);

		if ((prev != nullptr) || (next != nullptr))
		{
			TCHAR buffer[MAX_HEADING_LEN] = { 0 };

			HTREEITEM parent = TreeView_GetParent(this->hwndTV, item);

			TVITEM tvi;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
			tvi.pszText = buffer;
			tvi.cchTextMax = MAX_HEADING_LEN;
			tvi.hItem = item;

			if (TreeView_GetItem(this->hwndTV, &tvi))
			{
				auto insertAfter = this->FindInsertAfter(parent, type, buffer);

				if (insertAfter != prev)
				{
					TreeView_DeleteItem(this->hwndTV, item);

					TVINSERTSTRUCT tvins;
					tvins.hParent = parent;
					tvins.hInsertAfter = insertAfter;
					tvins.item = tvi;

					HTREEITEM newItem = TreeView_InsertItem(this->hwndTV, &tvins);
					if (newItem)
					{
						if (type == PARENT__ITEM)
						{
							this->InsertFolder(newItem);
							TreeView_SelectItem(this->hwndTV, newItem);
						}
						return newItem;
					}
					else
						return NULL;// error inserting item
				}
				else
					return item;// the position was initally correct -> return the inital item
			}
			else
				return NULL;// error getting item
		}
		else
			return item;// there are no other items in the level -> sorting is not required -> return the initial item!
	}
	else
		return NULL;// error-type
}

int TreeViewCTRL::OnItemExpanding(LPNMTREEVIEW ntv)
{
	UINT state = TreeView_GetItemState(this->hwndTV, ntv->itemNew.hItem, TVIS_EXPANDED);

	if (!(state & TVIS_EXPANDED))//(ntv->itemNew.iImage == 1)
	{
		ntv->itemNew.iImage = this->ico_index[2];
		ntv->itemNew.iSelectedImage = this->ico_index[2];

		TreeView_SetItem(this->hwndTV, &ntv->itemNew);
	}
	else// if (ntv->itemNew.iImage == 2)
	{
		ntv->itemNew.iImage = this->ico_index[1];
		ntv->itemNew.iSelectedImage = this->ico_index[1];

		TreeView_SetItem(this->hwndTV, &ntv->itemNew);
	}
	return 0;
}

int TreeViewCTRL::OnItemExpanded(LPNMTREEVIEW ntv)
{
	if (ntv->action == TVE_EXPAND)
	{
		TreeView_SelectItem(this->hwndTV, ntv->itemNew.hItem);
	}
	return 0;
}

int TreeViewCTRL::OnSelchanged(LPNMTREEVIEW ntv)
{
	int type;
	TCHAR* itemPath = nullptr;

	type = this->GetItemPath(ntv->itemNew.hItem, &itemPath);
	if (type >= 0)
	{
		if (this->iTVUserEvents != nullptr)
		{
			this->iTVUserEvents->onSelectionChanged(type, itemPath);
		}

		// old message to set the navigation folder
		//	SendMessage(
		//		this->TVFrame,
		//		WM_COMMAND,
		//		MAKEWPARAM(TV_CTRL_SETCURFOLDER, 0),
		//		reinterpret_cast<LPARAM>(path)
		//	);

	}

	SafeDeleteArray(&itemPath);

	return type;

	//int ir = -1;

	//int type = this->CheckItemType(ntv->itemNew.hItem);

	//ir = (type != PARENT__ITEM) ? 0 : 1;
	//if (ir)
	//{
	//	int ItemCount = 0;
	//	HTREEITEM cItems[12];

	//	TCHAR* buffer = new TCHAR[MAX_HEADING_LEN];

	//	ir = (buffer == NULL) ? 0 : 1;
	//	if (ir)
	//	{
	//		TCHAR* root = new TCHAR[MAX_HEADING_LEN];

	//		ir = (root == NULL) ? 0 : 1;
	//		if (ir)
	//		{
	//			TCHAR* path = new TCHAR[4096];

	//			ir = (path == NULL) ? 0 : 1;
	//			{
	//				HRESULT hr = StringCbCopy(path, sizeof(TCHAR) * 4096, this->Root_Folder);

	//				ir = SUCCEEDED(hr) ? 1 : 0;
	//				if (ir)
	//				{
	//					TVITEM tv;
	//					tv.hItem = TreeView_GetRoot(this->hwndTV);
	//					tv.mask = TVIF_TEXT | TVIF_HANDLE;
	//					tv.cchTextMax = sizeof(TCHAR) * MAX_HEADING_LEN;
	//					tv.pszText = root;

	//					ir = TreeView_GetItem(this->hwndTV, &tv);
	//					if (ir)
	//					{
	//						cItems[ItemCount] = ntv->itemNew.hItem;// TreeView_GetSelection( this->hwndTV ); ??
	//						ItemCount++;

	//						while ((cItems[ItemCount] = TreeView_GetParent(this->hwndTV, cItems[ItemCount - 1])) != NULL)
	//						{
	//							if ((ItemCount == 11))// TODO: remove delimiter
	//							{
	//								break;
	//							}
	//							ItemCount++;
	//						}
	//						ItemCount--;

	//						for (int j = ItemCount; j >= 0; j--)
	//						{
	//							tv.hItem = cItems[j];
	//							tv.pszText = buffer;

	//							if (!TreeView_GetItem(this->hwndTV, &tv))
	//							{
	//								return -2;
	//							}
	//							if (this->CompareHeading(root, buffer) && (j == ItemCount))
	//							{
	//								continue;
	//							}
	//							hr = StringCbCat(path, sizeof(TCHAR) * 4096, L"\\");
	//							if (FAILED(hr))
	//								break;
	//							hr = StringCbCat(path, sizeof(TCHAR) * 4096, buffer);
	//							if (FAILED(hr))
	//								break;
	//						}

	//						ir = SUCCEEDED(hr) ? 1 : 0;
	//						if (ir)
	//						{
	//							SendMessage(this->TVFrame,
	//								WM_COMMAND,
	//								MAKEWPARAM(TV_CTRL_SETCURFOLDER, 0),
	//								reinterpret_cast<LPARAM>(path));
	//						}
	//					}
	//				}
	//				delete[] path;
	//			}
	//			delete[] root;
	//		}
	//		delete[] buffer;
	//	}
	//}
	//return ir;
}

int TreeViewCTRL::OnDblClick()
{
	int type = this->CheckItemType(TreeView_GetSelection(this->hwndTV));

	if ((type == PARENT__ITEM) || (type == EMPTY__ITEM))return 0;

	HRESULT hr;
	int ItemCount = 0;

	TCHAR* buffer = new TCHAR[MAX_HEADING_LEN];

	hr = (buffer == nullptr) ? E_OUTOFMEMORY : S_OK;
	if (SUCCEEDED(hr))
	{
		TCHAR* root = new TCHAR[MAX_HEADING_LEN];

		hr = (root == nullptr) ? E_OUTOFMEMORY : S_OK;
		if (SUCCEEDED(hr))
		{
			TCHAR* path = new TCHAR[4096];

			hr = (path == nullptr) ? E_OUTOFMEMORY : S_OK;
			if (SUCCEEDED(hr))
			{
				hr = StringCbCopy(path, sizeof(TCHAR) * 4096, this->Root_Folder);
				if (SUCCEEDED(hr))
				{
					TVITEM tv;
					tv.hItem = TreeView_GetRoot(this->hwndTV);
					tv.mask = TVIF_TEXT | TVIF_HANDLE;
					tv.cchTextMax = sizeof(TCHAR) * MAX_HEADING_LEN;
					tv.pszText = root;

					hr = TreeView_GetItem(this->hwndTV, &tv) ? S_OK : E_FAIL;
					if (SUCCEEDED(hr))
					{
						HTREEITEM cursel = TreeView_GetSelection(this->hwndTV);

						hr = (cursel == nullptr) ? E_HANDLE : S_OK;
						if (SUCCEEDED(hr))
						{
							ItemCount++;

							HTREEITEM parent;

							while ((parent = TreeView_GetParent(this->hwndTV, cursel)) != nullptr)
							{
								cursel = parent;

								ItemCount++;
							}
							HTREEITEM* cItems = new HTREEITEM[ItemCount];

							hr = (cItems == nullptr) ? E_OUTOFMEMORY : S_OK;
							if(SUCCEEDED(hr))
							{
								cItems[ 0 ] = TreeView_GetSelection(this->hwndTV);

								for (int i = 1; i < ItemCount; i++)
								{
									cItems[i] = TreeView_GetParent(this->hwndTV, cItems[i - 1]);

									hr = (cItems[i] == nullptr) ? E_HANDLE : S_OK;
									if (FAILED(hr))
										break;
								}
								if (SUCCEEDED(hr))
								{
									ItemCount--;

									for (int j = ItemCount; j >= 0; j--)
									{
										tv.hItem = cItems[ j ];
										tv.pszText = buffer;

										hr = TreeView_GetItem(this->hwndTV, &tv) ? S_OK : E_FAIL;
										if (FAILED(hr))
											break;

										if (this->CompareHeading(root, buffer) && (j == ItemCount))
										{
											continue;
										}
										hr = StringCbCat(path, sizeof(TCHAR) * 4096, L"\\");
										if (FAILED(hr))
											break;
										hr = StringCbCat(path, sizeof(TCHAR) * 4096, buffer);
										if (FAILED(hr))
											break;
									}
									if (SUCCEEDED(hr))
									{

										if (type == CNC3__ITEM)
										{
											hr = StringCbCat(path, sizeof(TCHAR) * 4096, L".cnc3");
										}
										if (SUCCEEDED(hr))
										{
											SendMessage(this->TVFrame,
												WM_COMMAND,
												MAKEWPARAM(TV_CTRL_OPENPATH, 0),
												reinterpret_cast<LPARAM>(path)
											);
										}
									}
								}
								delete [] cItems;
							}
						}
					}
				}
				delete[] path;
			}
			delete[] root;
		}
		delete[] buffer;
	}
	return SUCCEEDED(hr) ? 1 : 0;
}

int TreeViewCTRL::OnRgtClick()
{
	POINT pt;
	TVHITTESTINFO thi;

	if (GetCursorPos(&pt))
	{
		if (ScreenToClient(this->hwndTV, &pt))
		{
			thi.pt.x = pt.x;
			thi.pt.y = pt.y;

			HTREEITEM item = TreeView_HitTest(this->hwndTV, &thi);
			if (item)
			{
				if (TreeView_SelectItem(this->hwndTV, thi.hItem))
				{
					int type = this->CheckItemType(item);
					if (type != -1)
					{
						if (item == TreeView_GetRoot(this->hwndTV))
						{
							type = ROOT__ITEM;
						}
						SendMessage(this->TVFrame, WM_COMMAND, MAKEWPARAM(TV_CTRL_CREATETOOLWND, type), 0);
					}
				}
			}
		}
	}
	return 0;
}

int TreeViewCTRL::OnBeginDrag(LPNMTREEVIEW ptv)
{
	int result = 0;
	TCHAR* path = NULL;

	int type = this->GetItemPath(ptv->itemNew.hItem, &path);

	if (type == -1)
		return 1;
	else
	{
		if (type == CHILD__ITEM)
		{
			SetCursor(this->Obj.file);
			this->FOPInfo.DragCursorType = A__FILE;
		}
		else if (type == PARENT__ITEM)
		{
			SetCursor(this->Obj.folder);
			this->FOPInfo.DragCursorType = A__FOLDER;
		}
		else if (type == CNC3__ITEM)
		{
			SetCursor(this->Obj.cnc3);
			this->FOPInfo.DragCursorType = A__CNC3FILE;
		}
		else
			result = 2;

		if (result == 0)
		{
			SafeDeleteArray(&this->FOPInfo.movepath_source);

			this->FOPInfo.type_forMove = type;
			this->FOPInfo.lastaction = FOP_MOVE;
			this->FOPInfo.moveItem = ptv->itemNew.hItem;

			if (CopyStringToPtrW(path, &this->FOPInfo.movepath_source))
			{
				this->FOPInfo.movepath_source_valid = TRUE;

				SetCapture(this->TVFrame);
				this->DragUnderway = TRUE;

				if (SetTimer(this->TVFrame, ID_TVDRAGSCROLL_TIMER, 100, nullptr))
				{
					this->CursorScrollTimerActivated = TRUE;
					this->ScrollDirection = 0;
				}
			}
		}
		SafeDeleteArray(&path);
	}
	return result;
}

int TreeViewCTRL::OnEndLabelEdit(LPNMTVDISPINFO pdtv)
{
	if (pdtv->item.pszText != nullptr)
	{
		if (pdtv->item.pszText[0] != L'\0')
		{
			// make sure the name for the filesystem-object is valid
			auto bfpo = CreateBasicFPO();
			if (bfpo != nullptr)
			{
				if (!bfpo->VerifyFilename(pdtv->item.pszText))
				{
					DispatchEWINotification(
						EDSP_ERROR,
						L"FN0003",
						getStringFromResource(UI_GNRL_NAMEINVALID),
						getStringFromResource(UI_FILENAVIGATOR)
					);
					return FALSE;
				}
			}
			if (CopyStringToPtrW(pdtv->item.pszText, &this->FOPInfo.preOperationBuffer))
			{
				this->FOPInfo.preOperationBuffer_valid = TRUE;

				HRESULT hr = this->CreateFOP_Thread(FOP_RENAME);
				if (SUCCEEDED(hr))
				{
					// ...

					return TRUE; // TRUE -> set new label
				}
				else
					return FALSE; // FALSE -> reject changes		
			}
			else
				return FALSE;
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}

int TreeViewCTRL::OnBeginLabelEdit(LPNMTVDISPINFO pdtv)
{
	UNREFERENCED_PARAMETER(pdtv);

	TCHAR* path = NULL;

	this->FOPInfo.labelItem = TreeView_GetSelection(this->hwndTV);

	SafeDeleteArray(&this->FOPInfo.renamepath);

	int type = this->_getSelectedItemPath(&path);
	if (type >= 0)
	{
		if (type == EMPTY__ITEM)
		{
			return TRUE;
		}
		if (CopyStringToPtrW(path, &this->FOPInfo.renamepath))
		{
			this->FOPInfo.renamepath_valid = TRUE;
		}
		SafeDeleteArray(&path);

		return FALSE;
	}
	else
		return FALSE;
}

int TreeViewCTRL::OnGetInfoTip(LPNMTVGETINFOTIP gtip)
{
	if (gtip != nullptr)
	{
		TCHAR *itemPath = nullptr;

		auto type = this->GetItemPath(gtip->hItem, &itemPath);
		if (type == CNC3__ITEM)
		{
			DESCRIPTIONINFO dInfo;
			SecureZeroMemory(&dInfo, sizeof(DESCRIPTIONINFO));

			BOOL result =
				(BOOL)SendMessage(
					GetParent(this->TVFrame),	// this is supposed to be the main window
					WM_GETDESCRIPTIONS,
					0,
					reinterpret_cast<LPARAM>(&dInfo)
				);
			if (result)
			{
				CnC3File file;
				auto hr =
					file.Load(itemPath);

				if (SUCCEEDED(hr))
				{
					iString tooltip;

					if (tooltip.Builder(
						nullptr, dInfo.desc1, L":\n",
						file.GetProperty(CnC3File::PID_DESCRIPTION_ONE)
							.GetData(),
						L"\n\n", dInfo.desc2, L":\n",
						file.GetProperty(CnC3File::PID_DESCRIPTION_TWO)
							.GetData(),
						L"\n\n", dInfo.desc3, L":\n",
						file.GetProperty(CnC3File::PID_DESCRIPTION_THREE)
							.GetData(),
						nullptr)
						)
					{
						StringCbCopy(gtip->pszText, sizeof(TCHAR)* (tooltip.GetLength() + 1), tooltip.GetData());
					}
				}
				SafeDeleteArray(&dInfo.desc1);
				SafeDeleteArray(&dInfo.desc2);
				SafeDeleteArray(&dInfo.desc3);
			}
		}
		SafeDeleteArray(&itemPath);
	}
	return 0;
}

BOOL TreeViewCTRL::PrepareForCopyOperation()
{
	BOOL result;
	TCHAR* path = NULL;

	this->folderCopyInProgress = FALSE;
	this->FOPInfo.type_forCopy = this->_getSelectedItemPath(&path);

	result = (this->FOPInfo.type_forCopy < 0) ? FALSE : TRUE;
	if (result)
	{
		size_t len;
		HRESULT hr = StringCbLength(path, sizeof(TCHAR) * 4096, &len);

		result = SUCCEEDED(hr) ? TRUE : FALSE;
		if(result)
		{
			len += sizeof(TCHAR);

			if (this->FOPInfo.copypath != NULL)
			{
				delete[] this->FOPInfo.copypath;
				this->FOPInfo.copypath = nullptr;
			}

			this->FOPInfo.copypath = new TCHAR[len];

			result = (this->FOPInfo.copypath == NULL) ? FALSE : TRUE;
			if (result)
			{
				hr = StringCbCopy(this->FOPInfo.copypath, len, path);

				result = SUCCEEDED(hr) ? TRUE : FALSE;
				if (result)
				{
					this->FOPInfo.copypath_valid = TRUE;
					this->FOPInfo.lastaction = FOP_COPY;
				}
			}
		}
		delete[] path;
	}
	return result;
}

BOOL TreeViewCTRL::StartLabelEdit()
{
	if (TreeView_EditLabel(this->hwndTV,
		TreeView_GetSelection(this->hwndTV)) == NULL)
	{
		return FALSE;
	}	
	return TRUE;
}

HRESULT TreeViewCTRL::CreateFOP_Thread(int mode)
{
	HRESULT hr = E_HANDLE;
	HANDLE hThread;
	DWORD ThreadId;

	TVTHREADDATA td;
	td.mode = mode;
	td.root = nullptr;
	td.toClass = reinterpret_cast<LONG_PTR>(this);

	hThread = CreateThread(NULL, 0, TreeViewCTRL::FOP_Proc, reinterpret_cast<LPVOID>(&td), 0, &ThreadId);
	if (hThread)
	{
		WaitForSingleObject(hThread, 20);

		CloseHandle(hThread);
		hr = S_OK;
	}
	return hr;
}

HRESULT TreeViewCTRL::_executeCopyOP()
{
	HRESULT hr = S_OK;

	if (this->FOPInfo.copypath_valid)	
	{
		TCHAR* destination = NULL;

		int type = this->_getSelectedItemPath(&destination);

		hr = (type < 0) ? E_FAIL : S_OK;
		if (SUCCEEDED(hr))
		{
			if ((type == CHILD__ITEM) || (type == CNC3__ITEM))
			{
				// eliminate filename to get the destination directory to copy in ...
				hr = this->RemoveFilename(destination) ? S_OK : E_FAIL;
			}
			if (SUCCEEDED(hr))
			{
				hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
				if (SUCCEEDED(hr))
				{
					IFileOperation* pfo;

					hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
					if (SUCCEEDED(hr))
					{
						hr = pfo->SetOperationFlags(FOF_RENAMEONCOLLISION);
						if (SUCCEEDED(hr))
						{
							hr = pfo->SetOwnerWindow(_MAIN_WND_);
							if(SUCCEEDED(hr))
							{
								IShellItem *psiFrom = NULL;

								hr = SHCreateItemFromParsingName(this->FOPInfo.copypath, NULL, IID_PPV_ARGS(&psiFrom));
								if (SUCCEEDED(hr))
								{
									IShellItem *psiTo = NULL;

									hr = SHCreateItemFromParsingName(destination, NULL, IID_PPV_ARGS(&psiTo));
									if (SUCCEEDED(hr))
									{
										hr = pfo->CopyItem(psiFrom, psiTo, NULL, (IFileOperationProgressSink*)this);
										if (SUCCEEDED(hr))
										{
											hr = pfo->PerformOperations();
											if (SUCCEEDED(hr))
											{
												HTREEITEM item;

												if ((type == CHILD__ITEM) || (type == CNC3__ITEM))
												{
													item = TreeView_GetParent(this->hwndTV,
														TreeView_GetSelection(this->hwndTV));
												}
												else
												{
													item = TreeView_GetSelection(this->hwndTV);
												}

												hr = (item) ? S_OK : E_HANDLE;
												if (SUCCEEDED(hr))
												{
													TVITEM par_tvi;
													par_tvi.mask = TVIF_HANDLE | TVIF_PARAM;
													par_tvi.hItem = item;
													TreeView_GetItem(this->hwndTV, &par_tvi);

													if (this->FOPInfo.postOperationBuffer_valid)
													{
														TVITEM tvi;
														tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
														tvi.pszText = this->FOPInfo.postOperationBuffer;
														tvi.cchTextMax = MAX_HEADING_LEN;
														tvi.lParam = (LPARAM)((int)par_tvi.lParam + 1);

														if (this->FOPInfo.type_forCopy == CHILD__ITEM)
														{
															tvi.iImage = ico_index[0];
															tvi.iSelectedImage = ico_index[0];
														}
														else if (this->FOPInfo.type_forCopy == PARENT__ITEM)
														{
															tvi.iImage = ico_index[1];
															tvi.iSelectedImage = ico_index[1];
														}
														else if (this->FOPInfo.type_forCopy == CNC3__ITEM)
														{
															tvi.iImage = ico_index[3];
															tvi.iSelectedImage = ico_index[3];
														}
														else
														{
															tvi.iImage = ico_index[0];
															tvi.iSelectedImage = ico_index[0];
														}

														TVINSERTSTRUCT tvis;
														tvis.hInsertAfter = this->FindInsertAfter(item, this->FOPInfo.type_forCopy, this->FOPInfo.postOperationBuffer);
														tvis.hParent = item;
														tvis.item = tvi;

														HTREEITEM newItem = TreeView_InsertItem(this->hwndTV, &tvis);

														TreeView_Expand(this->hwndTV, item, TVE_EXPAND);

														if (this->FOPInfo.type_forCopy == PARENT__ITEM)
														{
															hr = this->InsertFolder(newItem) ? S_OK : E_FAIL;

															this->folderCopyInProgress = FALSE;
														}
														SafeDeleteArray(&this->FOPInfo.postOperationBuffer);
														this->FOPInfo.postOperationBuffer_valid = FALSE;

														TreeView_SelectItem(this->hwndTV, newItem);
													}
												}
											}
										}
										SafeRelease(&psiTo);
									}
									SafeRelease(&psiFrom);
								}
							}
						}
						SafeRelease(&pfo);
					}
					CoUninitialize();
				}
			}
			SafeDeleteArray(&destination);
		}
	}
	return hr;
}

HRESULT TreeViewCTRL::_executeDeleteOP()
{
	HRESULT hr = S_OK;

	int res = IDOK;// this->GetUserPermission(FOP_DELETE);

	hr = (res == IDOK) ? S_OK : E_ABORT;
	if (SUCCEEDED(hr))
	{
		TCHAR* toDelete = NULL;

		int type = this->_getSelectedItemPath(&toDelete);

		hr = (type < 0) ? E_FAIL : S_OK;
		if (SUCCEEDED(hr))
		{
			hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			if (SUCCEEDED(hr))
			{
				IFileOperation* pfo;

				hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
				if (SUCCEEDED(hr))
				{
					hr = pfo->SetOperationFlags(/*FOF_NO_UI | */FOFX_RECYCLEONDELETE);
					if (SUCCEEDED(hr))
					{
						IShellItem* psi;

						hr = SHCreateItemFromParsingName(toDelete, NULL, IID_PPV_ARGS(&psi));
						if (SUCCEEDED(hr))
						{
							hr = pfo->DeleteItem(psi, (IFileOperationProgressSink*)this);
							if (SUCCEEDED(hr))
							{
								hr = pfo->PerformOperations();
								if (SUCCEEDED(hr))
								{
									HTREEITEM item = TreeView_GetSelection(this->hwndTV);
									HTREEITEM parent = TreeView_GetParent(this->hwndTV, item);

									hr = TreeView_DeleteItem(this->hwndTV, item) ? S_OK : E_FAIL;
									if (SUCCEEDED(hr))
									{
										item = TreeView_GetChild(this->hwndTV, parent);
										if (item == NULL)
										{
											hr = this->InsertEmptyItem(parent) ? S_OK : E_FAIL;
										}
									}

									if (this->iTVUserEvents != nullptr)
									{
										this->iTVUserEvents->onItemDeleted(type, toDelete);
									}
								}
							}
							SafeRelease(&psi);
						}
					}
					SafeRelease(&pfo);
				}
				CoUninitialize();
			}
			SafeDeleteArray(&toDelete);
		}
	}
	return hr;
}

HRESULT TreeViewCTRL::_executeMoveOP()
{
	HRESULT hr = S_OK;
	this->moveOperationInProgress = TRUE;

	if ((!this->FOPInfo.movepath_source_valid) || (!this->FOPInfo.movepath_target_valid))
	{
		hr = E_FAIL;
	}
	else
	{
		hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(hr))
		{
			IFileOperation* pfo;

			hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
			if (SUCCEEDED(hr))
			{
				hr = pfo->SetOperationFlags(/*FOF_NO_UI|*/FOF_RENAMEONCOLLISION);
				if (SUCCEEDED(hr))
				{
					IShellItem* psiFrom = NULL;

					hr = SHCreateItemFromParsingName(this->FOPInfo.movepath_source, NULL, IID_PPV_ARGS(&psiFrom));
					if (SUCCEEDED(hr))
					{
						IShellItem* psiTo = NULL;

						hr = SHCreateItemFromParsingName(this->FOPInfo.movepath_target, NULL, IID_PPV_ARGS(&psiTo));
						if (SUCCEEDED(hr))
						{
							hr = pfo->MoveItem(psiFrom, psiTo, NULL, (IFileOperationProgressSink*)this);
							if(SUCCEEDED(hr))
							{ 
								hr = pfo->PerformOperations();
								if (SUCCEEDED(hr))
								{
									HTREEITEM hDestination = TreeView_GetSelection(this->hwndTV);
									if (hDestination)
									{
										TCHAR Heading[MAX_HEADING_LEN] = { 0 };//////////

										TVITEM tvi;
										tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT |TVIF_STATE;
										tvi.hItem = this->FOPInfo.moveItem;
										tvi.pszText = Heading;
										tvi.cchTextMax = MAX_HEADING_LEN;

										if (TreeView_GetItem(this->hwndTV, &tvi))
										{
											tvi.lParam = this->GetItemLevel(hDestination) + 1;
											tvi.pszText = this->FOPInfo.postOperationBuffer;

											TVINSERTSTRUCT tvins;
											tvins.hParent = hDestination;
											tvins.hInsertAfter = this->FindInsertAfter(hDestination, this->FOPInfo.type_forMove, this->FOPInfo.postOperationBuffer);
											tvins.item = tvi;

											HTREEITEM newItem;

											if ((newItem = TreeView_InsertItem(this->hwndTV, &tvins)) != NULL)
											{
												HTREEITEM parent_old = TreeView_GetParent(this->hwndTV, this->FOPInfo.moveItem);
												if (parent_old)
												{
													TreeView_DeleteItem(this->hwndTV, this->FOPInfo.moveItem);

													if (TreeView_GetChild(this->hwndTV, parent_old) == NULL)
													{
														this->InsertEmptyItem(parent_old);
													}
												}
												TreeView_Expand(this->hwndTV, hDestination, TVE_EXPAND);

												if (this->FOPInfo.type_forMove == PARENT__ITEM)
												{
													if (this->InsertFolder(newItem) == EMPTY_DIR)
													{
														// ...
													}
												}

												// trigger event
												if (this->iTVUserEvents != nullptr)
												{
													if (this->FOPInfo.type_forMove != PARENT__ITEM) // if this is a file, append the filename to the target folder to get the target-filepath!
													{
														TCHAR *filename = nullptr;

														if (GetFilenameOutOfPath(this->FOPInfo.movepath_source, &filename, FALSE)
															== TRUE)
														{
															TCHAR* targetfilepath = nullptr;

															if (AppendStringsWithVaList(&targetfilepath, this->FOPInfo.movepath_target, L"\\", filename, nullptr)
																== TRUE)
															{
																this->iTVUserEvents->onItemMoved(
																	this->FOPInfo.type_forMove,
																	targetfilepath,
																	this->FOPInfo.movepath_source																	
																);
															}
														}
													}
													else
													{
														TCHAR* foldername = nullptr;

														if (GetFilenameOutOfPath(this->FOPInfo.movepath_source, &foldername, FALSE)
															== TRUE)
														{
															TCHAR* targetFolderPath = nullptr;

															if (AppendStringsWithVaList(&targetFolderPath, this->FOPInfo.movepath_target, L"\\", foldername, nullptr)
																== TRUE)
															{
																this->iTVUserEvents->onItemMoved(
																	this->FOPInfo.type_forMove,
																	targetFolderPath,
																	this->FOPInfo.movepath_source																	
																);
															}
														}
													}
												}

												SafeDeleteArray(&this->FOPInfo.postOperationBuffer);
												this->FOPInfo.postOperationBuffer_valid = FALSE;
												SafeDeleteArray(&this->FOPInfo.movepath_source);
												this->FOPInfo.movepath_source_valid = FALSE;
												SafeDeleteArray(&this->FOPInfo.movepath_target);
												this->FOPInfo.movepath_target_valid = FALSE;
												this->FOPInfo.moveItem = NULL;

												TreeView_SelectItem(this->hwndTV, newItem);
											}
										}
									}
								}
							}
							SafeRelease(&psiTo);
						}
						SafeRelease(&psiFrom);
					}
				}
				SafeRelease(&pfo);
			}
			CoUninitialize();
		}
	}
	this->moveOperationInProgress = FALSE;

	return hr;
}

HRESULT TreeViewCTRL::_executeRenameOP(LPCWSTR newName)
{
	HRESULT hr = S_OK;
	WCHAR* _newName_ = NULL;

	if ((!this->FOPInfo.renamepath_valid) || (!this->FOPInfo.preOperationBuffer_valid))
	{
		return E_UNEXPECTED;
	}
	else
	{
		int type = this->CheckItemType(this->FOPInfo.labelItem);

		hr = (type < 0) ? E_FAIL : S_OK;
		if (SUCCEEDED(hr))
		{
			if (type == EMPTY__ITEM)
				hr = E_ABORT;
			else if (type == CNC3__ITEM)
			{
				hr = (AppendStringToStringW(newName, L".cnc3\0", &_newName_)) ? S_OK : E_FAIL;
				if (SUCCEEDED(hr))
				{
					// ...
				}
			}
			else
			{
				hr = (CopyStringToPtrW(newName, &_newName_)) ? S_OK : E_FAIL;
				if (SUCCEEDED(hr))
				{
					// ...
				}
			}
			if (SUCCEEDED(hr))
			{
				hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
				if (SUCCEEDED(hr))
				{
					IFileOperation* pfo;

					hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
					if (SUCCEEDED(hr))
					{
						IShellItem* psiRename = NULL;

						hr = SHCreateItemFromParsingName(this->FOPInfo.renamepath, NULL, IID_PPV_ARGS(&psiRename));
						if (SUCCEEDED(hr))
						{
							hr = pfo->SetOperationFlags(FOF_NO_UI | FOF_RENAMEONCOLLISION);
							if (SUCCEEDED(hr))
							{
								hr = pfo->RenameItem(psiRename, _newName_, (IFileOperationProgressSink*)this);
								if (SUCCEEDED(hr))
								{
									SendMessage(
										this->TVFrame,
										WM_COMMAND,
										MAKEWPARAM(ICOMMAND_ACTIVATE_FSW_BLOCKER, 0),
										(LPARAM)TRUE
									);						// prevent the file-system-watcher from launching events during the operation

									hr = pfo->PerformOperations();
									if (SUCCEEDED(hr))
									{
										if (this->FOPInfo.postOperationBuffer_valid)
										{
											TCHAR buffer[MAX_HEADING_LEN] = { 0 };

											TVITEM tvi;
											tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
											tvi.hItem = this->FOPInfo.labelItem;
											tvi.cchTextMax = MAX_HEADING_LEN;
											tvi.pszText = buffer;

											hr = (TreeView_GetItem(this->hwndTV, &tvi)) ? S_OK : E_FAIL;
											if (SUCCEEDED(hr))
											{
												tvi.pszText = this->FOPInfo.postOperationBuffer;

												hr = (TreeView_SetItem(this->hwndTV, &tvi)) ? S_OK : E_FAIL;
												if (SUCCEEDED(hr))
												{

													TCHAR* itemPath = nullptr;
													auto _type = this->_getSelectedItemPath(&itemPath);

													// trigger event
													if (this->iTVUserEvents != nullptr)
													{
														this->iTVUserEvents->onItemRenamed(_type, itemPath, this->FOPInfo.renamepath);
													}

													HTREEITEM newItem = this->SortItem(this->FOPInfo.labelItem);
													if (newItem)
													{
														TreeView_SelectItem(this->hwndTV, newItem);

														// ...
													}
												}
											}
										}
										SafeDeleteArray(&this->FOPInfo.postOperationBuffer);
										this->FOPInfo.postOperationBuffer_valid = FALSE;
									}
								}
							}
							SafeRelease(&psiRename);
						}
						SafeRelease(&pfo);
					}
					CoUninitialize();
				}
			}
		}
		SafeDeleteArray(&this->FOPInfo.renamepath);
		this->FOPInfo.renamepath_valid = FALSE;
		this->FOPInfo.labelItem = NULL;
		SafeDeleteArray(&this->FOPInfo.preOperationBuffer);
		this->FOPInfo.preOperationBuffer_valid = FALSE;
	}
	return hr;
}

HRESULT TreeViewCTRL::_executeNewOP(int insertType)
{
	HRESULT hr;

	HTREEITEM item = TreeView_GetSelection(this->hwndTV);

	hr = (item == NULL) ? E_FAIL : S_OK;
	if (SUCCEEDED(hr))
	{
		PTSTR file = getStringFromResource(UI_GNRL_NEWPROGRAM_PLACEHOLDER);
		PTSTR folder = getStringFromResource(UI_GNRL_NEWFOLDER_PLACEHOLDER);
		LPWSTR Fptr = NULL;
		DWORD attributes = 0;
		BOOL nameWasChanged = FALSE;
		TCHAR* changedName = nullptr;

		if (insertType == A__CNC3FILE)
		{
			this->FOPInfo.type_forNew = CNC3__ITEM;
			attributes = FILE_ATTRIBUTE_NORMAL;
			Fptr = file;
		}
		else if (insertType == A__FOLDER)
		{
			this->FOPInfo.type_forNew = PARENT__ITEM;
			attributes = FILE_ATTRIBUTE_DIRECTORY;
			Fptr = folder;
		}
		else
			return E_UNEXPECTED;

		int type = this->CheckItemType(item);
		
		hr = (type < 0) ? E_FAIL : S_OK;
		if (SUCCEEDED(hr))
		{
			if (type != PARENT__ITEM)
			{
				item = TreeView_GetParent(this->hwndTV, item);
			}
			hr = (item != NULL) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				TCHAR* path = NULL;

				int result = this->GetItemPath(item, &path);

				if (insertType != A__FOLDER)
				{
					iString _tPath(path);
					_tPath.Append(L"\\");
					_tPath.Append(Fptr);

					auto copy_ = _tPath.GetDataAsCopy();
					if (copy_ != nullptr)
					{
						// check if file already exists and change name if applicable
						auto bfpo = CreateBasicFPO();
						if (bfpo != nullptr)
						{
							nameWasChanged = !(bfpo->IfFileExistsChangePath(&copy_));
							if (nameWasChanged)
							{
								if (bfpo->GetFilenameOutOfPath(copy_, &changedName, FALSE) == TRUE)
								{
									Fptr = changedName;
								}

							}
							SafeRelease(&bfpo);
						}
						SafeDeleteArray(&copy_);
					}
				}

				hr = (result < 0) ? E_FAIL : S_OK;
				if (SUCCEEDED(hr))
				{
					hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
					if (SUCCEEDED(hr))
					{
						IFileOperation* pfo;

						hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
						if (SUCCEEDED(hr))
						{
							IShellItem* psiNew = NULL;

							hr = SHCreateItemFromParsingName(path, NULL, IID_PPV_ARGS(&psiNew));
							if (SUCCEEDED(hr))
							{
								hr = pfo->SetOperationFlags(FOF_NO_UI | FOF_RENAMEONCOLLISION);
								if (SUCCEEDED(hr))
								{
									hr = pfo->NewItem(psiNew, attributes, Fptr, NULL, (IFileOperationProgressSink*)this);
									if (SUCCEEDED(hr))
									{
										hr = pfo->PerformOperations();
										if (SUCCEEDED(hr))
										{
											//// if a new cnc3 file was created it must be overwritten in the cnc3 format!
											if (insertType == A__CNC3FILE)
											{
												iString first(path);
												iString newName(Fptr);
												iString delimiter(L"\\\0");
												iString fullpath = first + delimiter + newName;

												iString content(L"\n ");

												if (reinterpret_cast<ApplicationData*>(
													getDefaultApplicationDataContainer()
													)->getBooleanData(DATAKEY_SETTINGS_INSERTDEFAULTTEXT, true))
												{
													content.Replace(
														reinterpret_cast<ApplicationData*>(
															getApplicationDataContainerFromFilekey(FILEKEY_USER_STRINGS)
															)->getStringData(
																DATAKEY_USERSTRINGS_DEFAULTTABINSERTTEXT,
																getStringFromResource(UI_GNRL_STARTUPTEXTPLACEHOLDER)
														)
													);
												}

												CnC3File cncFile;
												cncFile.SetPath(
													fullpath.GetData()
												);
												cncFile.SetNCContent(
													content.GetData()
												);
												cncFile.AddProperty(CnC3File::PID_DESCRIPTION_ONE, L"...");
												cncFile.AddProperty(CnC3File::PID_DESCRIPTION_TWO, L"...");
												cncFile.AddProperty(CnC3File::PID_DESCRIPTION_THREE, L"...");

												cncFile.Save();

												if (this->openNewFile)
												{
													SendMessage(this->TVFrame,
														WM_COMMAND,
														MAKEWPARAM(TV_CTRL_OPENPATH, DO_NOT_SET_FOCUS),
														reinterpret_cast<LPARAM>(fullpath.GetData())
													);
												}												
											}

											int _type_ = this->CheckItemType(item);
											if (_type_ > 0)
											{
												TCHAR buffer[MAX_HEADING_LEN] = { 0 };

												TVITEM tvi;
												tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
												tvi.cchTextMax = MAX_HEADING_LEN;
												tvi.pszText = buffer;
												tvi.hItem = item;

												if (TreeView_GetItem(this->hwndTV, &tvi))
												{
													int level = (int)tvi.lParam;

													SecureZeroMemory(&tvi, sizeof(TVITEM));

													tvi.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
													tvi.pszText = this->FOPInfo.postOperationBuffer;
													tvi.lParam = level + 1;// one level under the parent ...

													if (insertType == A__CNC3FILE)
													{
														tvi.iImage = this->ico_index[3];
														tvi.iSelectedImage = this->ico_index[3];
													}
													else if (insertType == A__FOLDER)
													{
														tvi.iImage = this->ico_index[1];
														tvi.iSelectedImage = this->ico_index[1];
													}
													TreeView_Expand(this->hwndTV,item, TVE_EXPAND);

													TVINSERTSTRUCT tvins;
													tvins.hParent = item;
													tvins.hInsertAfter = this->FindInsertAfter(item, this->FOPInfo.type_forNew, this->FOPInfo.postOperationBuffer);
													tvins.item = tvi;

													HTREEITEM newItem = TreeView_InsertItem(this->hwndTV, &tvins);
													if (newItem)
													{
														if (insertType == A__FOLDER)
														{
															this->InsertEmptyItem(newItem);
														}
														TreeView_SelectItem(this->hwndTV, newItem);
														this->StartLabelEdit();
													}
													SafeDeleteArray(&this->FOPInfo.postOperationBuffer);
													this->FOPInfo.postOperationBuffer_valid = FALSE;
												}
											}
										}
									}
								}
								SafeRelease(&psiNew);
							}
							SafeRelease(&pfo);
						}
					}
					SafeDeleteArray(&path);
				}
				SafeDeleteArray(&changedName);
			}
		}
	}
	return hr;
}

BOOL TreeViewCTRL::SendConvertInstruction()
{
	TCHAR* path = NULL;
	HTREEITEM itemToConv = TreeView_GetSelection(this->hwndTV);
	if (itemToConv)
	{
		int type = this->_getSelectedItemPath(&path);
		if (type >= 0)
		{
			this->internalFileOperationInProgress = TRUE;

			//if rAction is IDCANCEL -> prevent all actions
			//if rAction is IDNO -> replace the file
			//if rAction is IDYES -> create a copy and append "(conv)"

			BOOL rAction = (BOOL)SendMessage(this->TVFrame, WM_COMMAND, MAKEWPARAM(TV_CTRL_CONVERTTOCNC3, 0), reinterpret_cast<LPARAM>(path));
			if (rAction != IDCANCEL)
			{
				HTREEITEM parent = TreeView_GetParent(this->hwndTV, itemToConv);
				if (parent)
				{
					TCHAR buffer[MAX_HEADING_LEN] = { 0 };

					TVITEM tvi;
					tvi.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_HANDLE | TVIF_TEXT;
					tvi.hItem = itemToConv;
					tvi.cchTextMax = MAX_HEADING_LEN;
					tvi.pszText = buffer;

					// insert new item
					if (TreeView_GetItem(this->hwndTV, &tvi))
					{
						HTREEITEM newItem;

						tvi.mask = TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
						tvi.iImage = this->ico_index[3];
						tvi.iSelectedImage = this->ico_index[3];

						if (rAction == IDYES)
						{
							HRESULT hr = StringCbCat(buffer, sizeof(TCHAR)*MAX_HEADING_LEN, L"(conv)\0");
							if (SUCCEEDED(hr))
							{
								// ...

							}
						}
						if (this->RemoveFileExt(CNC3__ITEM, path))
						{
							tvi.pszText = path;

							TVINSERTSTRUCT tvins;
							tvins.hParent = parent;
							tvins.hInsertAfter = this->FindInsertAfter(parent, CNC3__ITEM, buffer);
							tvins.item = tvi;

							if ((newItem = TreeView_InsertItem(this->hwndTV, &tvins)) != NULL)
							{
								// ...

							}
						}
					}
				}
				if (rAction == IDNO)
				{
					// delete the old item - on user command
					TreeView_DeleteItem(this->hwndTV, itemToConv);
				}

			}
			this->internalFileOperationInProgress = FALSE;

			return TRUE;
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}

BOOL TreeViewCTRL::SendImportInstruction()
{
	TCHAR* path = NULL;

	int type = this->_getSelectedItemPath(&path);
	if (type >= 0)
	{
		SendMessage(this->TVFrame, WM_COMMAND, MAKEWPARAM(TV_CTRL_IMPORTFILE, 0), reinterpret_cast<LPARAM>(path));

		return TRUE;
	}
	else
		return FALSE;
}

HRESULT TreeViewCTRL::ProcessPostCopyInfo(DWORD dwFlags, LPCWSTR newName)
{
	HRESULT hr = S_OK;
	this->FOPInfo.dwFlagsAfterOP = dwFlags;

	if (!this->folderCopyInProgress)
	{
		// make sure that this method is only executed once per folder
		if (this->FOPInfo.type_forCopy == PARENT__ITEM)
			this->folderCopyInProgress = TRUE;

		if (newName == NULL)
			hr = E_FAIL;
		else
		{
			hr = (CopyStringToPtrW(newName, &this->FOPInfo.postOperationBuffer) == TRUE) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				hr = this->RemoveFileExt(this->FOPInfo.type_forCopy, this->FOPInfo.postOperationBuffer) ? S_OK : E_FAIL;
				if (SUCCEEDED(hr))
				{
					// ...
					this->FOPInfo.postOperationBuffer_valid = TRUE;
				}
			}
		}
	}
	return hr;
}

HRESULT TreeViewCTRL::ProcessPostDeleteInfo(DWORD dwFlags)
{
	this->FOPInfo.dwFlagsAfterOP = dwFlags;

	return S_OK;
}

HRESULT TreeViewCTRL::ProcessPostMoveInfo(DWORD dwFlags, LPCWSTR newName)
{
	HRESULT hr;
	this->FOPInfo.dwFlagsAfterOP = dwFlags;

	if (newName == NULL)
		hr = E_FAIL;
	else
	{
		hr = (CopyStringToPtrW(newName, &this->FOPInfo.postOperationBuffer) == TRUE) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			hr = this->RemoveFileExt(this->FOPInfo.type_forMove, this->FOPInfo.postOperationBuffer) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				// ...
				this->FOPInfo.postOperationBuffer_valid = TRUE;
			}
		}
	}
	return hr;
}

HRESULT TreeViewCTRL::ProcessPostNewFile(DWORD dwFlags, LPCWSTR newName, DWORD fileAtttributes)
{
	UNREFERENCED_PARAMETER(fileAtttributes);

	HRESULT hr;
	this->FOPInfo.dwFlagsAfterOP = dwFlags;

	if (newName == NULL)
		hr = E_FAIL;
	else
	{
		hr = (CopyStringToPtrW(newName, &this->FOPInfo.postOperationBuffer) == TRUE) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			hr = this->RemoveFileExt(this->FOPInfo.type_forNew, this->FOPInfo.postOperationBuffer) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				// ...
				this->FOPInfo.postOperationBuffer_valid = TRUE;
			}
		}
	}
	return hr;
}

HRESULT TreeViewCTRL::ProcessPostRename(DWORD dwFlags, LPCWSTR newName)
{
	HRESULT hr;
	this->FOPInfo.dwFlagsAfterOP = dwFlags;

	if (newName == NULL)
		hr = E_FAIL;
	else
	{
		hr = (CopyStringToPtrW(newName, &this->FOPInfo.postOperationBuffer) == TRUE) ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			hr = this->RemoveFileExt(
				this->CheckItemType(
					this->FOPInfo.labelItem),
						this->FOPInfo.postOperationBuffer) ? S_OK : E_FAIL;

			if (SUCCEEDED(hr))
			{
				// ...
				this->FOPInfo.postOperationBuffer_valid = TRUE;
			}
		}
	}
	return hr;
}

void TreeViewCTRL::stepProgressIfApplicable()
{
	if (this->iTVProgress != nullptr)
	{
		this->iTVProgress->itemAdded();
	}
}

void TreeViewCTRL::enableSelchangeNotification(bool enable)
{
	if (enable)
	{
		this->iTVUserEvents = this->temporaryPtr;
		this->temporaryPtr = nullptr;
	}
	else
	{
		this->temporaryPtr = this->iTVUserEvents;
		this->iTVUserEvents = nullptr;
	}
}

