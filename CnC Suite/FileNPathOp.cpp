#include"FileNPathOp.h"
#include"HelperF.h"

BasicFileNPathOperations::BasicFileNPathOperations(){}

BOOL BasicFileNPathOperations::pSaveBuffer(LPCTSTR buffer, LPCTSTR path, UINT codePage)
{
	BOOL result = TRUE;

	if ((buffer == NULL) || (path == NULL))
		return FALSE;
	else
	{
		size_t len;
		HRESULT hr;

		hr = StringCbLength(buffer, STRSAFE_MAX_CCH*sizeof(TCHAR), &len);
		if (SUCCEEDED(hr))
		{
			TCHAR* high_buffer = new TCHAR[(len += (2 * sizeof(TCHAR)))];
			if (high_buffer != NULL)
			{
				hr = StringCbPrintf(high_buffer, len, L"%s", buffer);
				if (SUCCEEDED(hr))
				{
					DWORD cbData = WideCharToMultiByte(codePage, 0, buffer, -1, NULL, 0, NULL, NULL);
					if (cbData != 0)
					{
						char* low_buffer = new (std::nothrow) CHAR[cbData];
						if (low_buffer != NULL)
						{
							if (WideCharToMultiByte(codePage, 0, high_buffer, -1, low_buffer, cbData, NULL, NULL) > 0)
							{
								HANDLE hFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hFile == INVALID_HANDLE_VALUE)
								{
									result = FALSE;
								}
								else
								{
									DWORD nBytesWritten;

									result = WriteFile(hFile, low_buffer, cbData, &nBytesWritten, NULL);
									if (result)
									{
										if (nBytesWritten == 0)
										{
											result = FALSE;
										}
									}
									CloseHandle(hFile);
								}
							}
							else
								result = FALSE;

							delete[] low_buffer;
						}
						else
							result = FALSE;
					}
					else
						result = FALSE;
				}
				delete[] high_buffer;
			}
			else
				result = FALSE;
		}
		else
			result = FALSE;
	}
	return result;
}

BOOL BasicFileNPathOperations::pLoadBuffer(TCHAR** high_buffer, LPCTSTR path, UINT codePage)
{
	BOOL result = TRUE;

	if ((high_buffer == NULL) || (path == NULL))
		return FALSE;
	else
	{
		HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		result = (hFile == INVALID_HANDLE_VALUE) ? FALSE : TRUE;
		if(result)
		{
			LARGE_INTEGER lg;

			result = GetFileSizeEx(hFile, &lg);
			if (result)
			{
				if (lg.LowPart > 0)
				{
					char* low_buffer = new (std::nothrow) CHAR[(size_t)(lg.LowPart + sizeof(char))];

					result = (low_buffer == NULL) ? FALSE : TRUE;
					if (result)
					{
						DWORD nBytesRead;

						result = ReadFile(hFile, low_buffer, lg.LowPart, &nBytesRead, NULL);
						if (result)
						{
							result = (nBytesRead > 0) ? TRUE : FALSE;
							if (result)
							{
								low_buffer[(lg.LowPart / sizeof(char))] = '\0';

								DWORD cbData = MultiByteToWideChar(codePage, 0, low_buffer, -1, NULL, 0);
								
								result = (cbData > 0) ? TRUE : FALSE;
								if (result)
								{
									*high_buffer = new TCHAR[cbData];

									result = (*high_buffer == NULL) ? FALSE : TRUE;
									if (result)
									{
										result = (MultiByteToWideChar(codePage, 0, low_buffer, lg.LowPart, *high_buffer, cbData) > 0) ? TRUE : FALSE;
									}
								}
							}
						}
						delete[] low_buffer;
					}
				}
			}
		}
		CloseHandle(hFile);
	}
	return result;
}

BOOL BasicFileNPathOperations::pGetKnownFolder(TCHAR** Folderpath, REFKNOWNFOLDERID ID)
{
	PWSTR path = NULL;

	HRESULT hr = SHGetKnownFolderPath(ID, 0, NULL, &path);
	if (SUCCEEDED(hr))
	{
		size_t len;

		hr = StringCbLength(path, STRSAFE_MAX_CCH * sizeof(TCHAR), &len);
		if (SUCCEEDED(hr))
		{
			*Folderpath = new WCHAR[len + sizeof(WCHAR)];
			if (*Folderpath != NULL)
			{
				hr = StringCbCopy(*Folderpath, len + sizeof(WCHAR), path);
			}
			else
				hr = E_FAIL;
		}
		CoTaskMemFree(path);
	}
	return SUCCEEDED(hr) ? TRUE : FALSE;
}

BOOL BasicFileNPathOperations::pRemoveFilename(LPTSTR destination)
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

BOOL BasicFileNPathOperations::pRemoveFilename(LPCTSTR path_in, TCHAR ** newpath_out)
{
	BOOL result = FALSE;
	TCHAR* buffer = nullptr;
	
	if (path_in != nullptr)
	{
		result = CopyStringToPtr(path_in, &buffer);
		if (result == TRUE)
		{
			result = this->pRemoveFilename(buffer);
			if (result)
			{
				result = CopyStringToPtr(buffer, newpath_out);
			}
			SafeDeleteArray(&buffer);
		}
	}
	return result;
}

BOOL BasicFileNPathOperations::pGetFilenameOutOfPath(LPCTSTR path, TCHAR ** filename_out, BOOL hideFileExt)
{
	BOOL result = TRUE;

	__try
	{
		__try
		{
			if (path == NULL)
				return EXC_INVALIDARGUMENT;
			if (*filename_out != NULL)
				return EXC_INVALIDARGUMENT;
		}
		__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			return (BOOL)EXCEPTION_ACCESS_VIOLATION;
		}
		int i = 0;
		int j = 0;
		BOOL isExt = FALSE;

		while (path[i] != L'\0')i++;
		j = i;
		while (path[i] != L'\\')
		{
			if (path[i] == L'.')
				isExt = TRUE;
			i--;

			if (i < 0)
				break;
		}

		size_t len = ((size_t)(((j - i) + 2)*sizeof(TCHAR)));

		TCHAR* buffer = new TCHAR[len];
		result = (buffer != NULL) ? TRUE : FALSE;
		if (result)
		{
			SecureZeroMemory(buffer, len);

			j = 0;
			i += 1;
			while (path[i] != L'\0')
			{
				buffer[j] = path[i];
				i++;
				j++;
			}
			buffer[j] = L'\0';

			if (hideFileExt && isExt)
			{
				j -= 1;

				if (j > 0)
				{

					while (buffer[j] != L'.')
					{
						if (j == 0)
							break;
						buffer[j] = L'\0';
						j--;
					}
					buffer[j] = L'\0';
				}
			}

			HRESULT hr = StringCbLength(buffer, sizeof(TCHAR)*STRSAFE_MAX_CCH, &len);

			result = SUCCEEDED(hr) ? TRUE : FALSE;
			if (result)
			{
				len += sizeof(TCHAR);

				*filename_out = new TCHAR[len];
				result = (*filename_out != NULL) ? TRUE : FALSE;
				if (result)
				{
					hr = StringCbCopy(*filename_out, len, buffer);

					result = SUCCEEDED(hr) ? TRUE : FALSE;
				}
			}
		}
	}
	__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return (BOOL)EXCEPTION_ARRAY_BOUNDS_EXCEEDED;
	}
	return result;
}

BOOL BasicFileNPathOperations::pIfFileExistsChangePath(TCHAR **path)
{
	int counter = 1;
	BOOL result = TRUE;
	BOOL repeat = TRUE;
	TCHAR* holdPath = NULL;

	if (*path == NULL)
		return FALSE;
	else
	{
		do
		{
			HANDLE hFile = CreateFile(*path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				if (counter == 1)
					return TRUE;
				else
					return FALSE;
			}
			else
			{
				if (counter > 1)
				{
					SafeDeleteArray(path);
					CopyStringToPtr(holdPath, path);
					SafeDeleteArray(&holdPath);
				}
				TCHAR* cpPath = NULL;

				result =
					CopyStringToPtr(*path, &cpPath) ? TRUE : FALSE;
				if (result)
				{					
					result =
						CopyStringToPtr(*path, &holdPath) ? TRUE : FALSE;
					if (result)
					{
						TCHAR* ext = NULL;

						SafeDeleteArray(path);

						if (this->RemoveFileExtension(cpPath, &ext))
						{
							TCHAR buffer[256] = { 0 };

							result =
								SUCCEEDED(
									StringCbPrintf(buffer, sizeof(buffer), L"(%i)\0", counter))
								? TRUE : FALSE;
							if (result)
							{
								result = AppendStringsWithVaList(path, cpPath, buffer, ext, NULL);
							}
							SafeDeleteArray(&ext);
						}
					}
					SafeDeleteArray(&cpPath);
				}
				CloseHandle(hFile);
			}
			if (!result)
				break;

			counter++;

		} while (repeat);
	}
	return result;
}

BOOL BasicFileNPathOperations::pRemoveFileExt(LPTSTR buffer, TCHAR** ext_out)
{
	__try
	{
		int i = 0;
		TCHAR extension[256] = { 0 };

		while (buffer[i] != L'\0')
		{
			i++;
		}
		i--;

		if (i < 3)
			return FALSE;

		int j = 0;

		while (buffer[i] != L'.')
		{
			extension[j] = buffer[i];
			buffer[i] = L'\0';

			if (buffer[i] == L'\\')
				return TRUE;// its a path but there is no extension !
			if (i == 0)
				return TRUE;// its no path and there is no extension !
			i--;
			j++;
		}
		buffer[i] = L'\0';

		TCHAR temp[256] = { 0 };

		temp[0] = L'.';
		int p = j - 1;
		int k = 0;

		for (k = 0; k < j; k++)
		{
			if (p < 0)
				break;
			temp[k + 1] = extension[p];
			p--;
		}
		temp[k + 1] = L'\0';

		return CopyStringToPtrA(temp, ext_out) ? TRUE : FALSE;
	}
	__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return FALSE;
	}
}

BOOL BasicFileNPathOperations::pGetFileExt(LPCTSTR string, TCHAR ** out)
{
	TCHAR* sCopy = NULL;
	BOOL result = TRUE;

	result
		= (CopyStringToPtr(string, &sCopy) == TRUE)
		? TRUE : FALSE;

	if (result)
	{
		TCHAR* extension = NULL;

		result = this->pRemoveFileExt(sCopy, &extension);

		if (result)
		{
			result
				= (CopyStringToPtr(extension, out) == TRUE)
				? TRUE : FALSE;

			SafeDeleteArray((void**)&extension);
		}
		SafeDeleteArray((void**)&sCopy);
	}
	return result;
}

BOOL BasicFileNPathOperations::pCountDirContent(LPCTSTR path, DWORD *files, DWORD *folders)
{
	int items = 0;

	HRESULT hr = S_OK;

	if (path == NULL)
		return FALSE;
	else
	{
		int type = 0;

		size_t buffer_len, fName_len;

		WIN32_FIND_DATA ffd_count;
		HANDLE hFind_c = INVALID_HANDLE_VALUE;

		// ROOT - SUB0
		hr = StringCbLength(path, STRSAFE_MAX_CCH *sizeof(TCHAR), &buffer_len);
		if (SUCCEEDED(hr))
		{
			TCHAR* path_buffer = new TCHAR[buffer_len + 15];

			hr = (path_buffer == nullptr) ? E_OUTOFMEMORY : S_OK;
			if (SUCCEEDED(hr))
			{
				hr = StringCbPrintf(path_buffer, sizeof(TCHAR) * (buffer_len + 15), L"\\\\?\\%s\\*", path);
				if (SUCCEEDED(hr))
				{
					hFind_c = FindFirstFile(path_buffer, &ffd_count);

					hr = (hFind_c == INVALID_HANDLE_VALUE) ? E_HANDLE : S_OK;
					if (SUCCEEDED(hr))
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
								*folders += 1;
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
											hr = StringCbPrintf(subitem, sizeof(TCHAR) * (buffer_len + fName_len), L"\\\\?\\%s\\%s\\*", path, ffd_count.cFileName);
											if (SUCCEEDED(hr))
											{
												hFind_sub = FindFirstFile(subitem, &ffd_count_sub);
												hr = (hFind_sub == INVALID_HANDLE_VALUE) ? E_HANDLE : S_OK;
												if (SUCCEEDED(hr))
												{
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
														hr = this->CountNextLevel(hFind_sub, &ffd_count_sub, subitem, items, files, folders);
													}
													else
													{
														// count one for the empty item ...
														items++;
														*folders += 1;
													}
													FindClose(hFind_sub);
												}
											}
											delete[] subitem;
										}
									}
								}
								items++;
								*folders += 1;
							}
							else
							{
								items++;
								*files += 1;
							}
						} while (FindNextFile(hFind_c, &ffd_count) != 0);

						FindClose(hFind_c);
					}
				}
				delete[] path_buffer;
			}
		}
	}
	if (FAILED(hr))
		return FALSE;
	else
		return items;
}

BOOL BasicFileNPathOperations::pCheckForFileExist(LPCTSTR path)
{
	BOOL result = FALSE;
	DWORD path_len = _lengthOfString(path);

	TCHAR *unicodePath = new TCHAR[path_len + 8];

	result = (unicodePath != nullptr) ? TRUE : FALSE;
	if (result)
	{
		result =
			SUCCEEDED(
				StringCbPrintf(unicodePath, sizeof(TCHAR) *(path_len + 8), L"\\\\?\\%s", path)
			)
			? TRUE : FALSE;

		if (result)
		{
			HANDLE hFile =
				CreateFileW(
					path,
					GENERIC_READ,
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL
				);

			if (hFile == INVALID_HANDLE_VALUE)
			{
				result = FALSE;
			}
			else
			{
				CloseHandle(hFile);
				result = TRUE;
			}
		}
		SafeDeleteArray((void**)&unicodePath);
	}
	return result;
}

BOOL BasicFileNPathOperations::pVerifyCmdLine(LPCTSTR nCmdLine, TCHAR **Path_out)
{
	__try
	{
		if (nCmdLine == NULL)
			return FALSE;
		if (*Path_out != NULL)
			return FALSE;

		int i = 0, hold = 0;
		BOOL cyc = TRUE;

		DWORD buffersize = 0;

		while (cyc)
		{
			while (nCmdLine[i] != L'\"')
			{
				if (nCmdLine[i] == L'\0')
					return FALSE;

				i++;
			}
			i++;
			hold = i;

			while (nCmdLine[i] != L'\"')
			{
				if (nCmdLine[i] == L'\0')
					return FALSE;

				i++;
				buffersize++;
			}

			if ((nCmdLine[i - 1] == L'e') && (nCmdLine[i - 2] == L'x') && (nCmdLine[i - 3] == L'e') && (nCmdLine[i - 4] == L'.'))
			{
				i++;
				buffersize = 0;
				continue;
			}
			else
			{
				cyc = FALSE;
				TCHAR* ppath_out = new TCHAR[buffersize + 1];
				if (ppath_out != NULL)
				{
					int max_array = buffersize + 1;
					int j = 0;
					i = hold;

					while (nCmdLine[i] != L'\"')
					{
						if (nCmdLine[i] == L'\0')
							break;

						ppath_out[j] = nCmdLine[i];

						i++;
						j++;

						if (j == max_array)
							break;
					}

					if (j == max_array)
						return FALSE;

					ppath_out[j] = L'\0';

					CopyStringToPtrA(ppath_out, Path_out);
				}
				else
					return FALSE;
			}
		}
		return TRUE;
	}
	__except ((	GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ||
				GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
				? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return FALSE;
	}
}

BOOL BasicFileNPathOperations::pVerifyFilename(LPCTSTR fileName)
{
	size_t len;

	HRESULT hr = StringCbLength(fileName, sizeof(TCHAR)* _MAX_FNAME, &len);
	if (SUCCEEDED(hr))
	{
		int _len = (int)(len / sizeof(TCHAR));
		if (len > 0)
		{
			int i = 0;

			while (i <= _len)
			{
				if (fileName[i] == L'\0')break;
				else
				{
					if ((int)fileName[i] > 255 || (int)fileName[i] < 0)
					{
						return FALSE;
					}
					else
					{
						int j = 0;

						while (forbiddenLiterals[j] != L'\0')
						{
							if (fileName[i] == forbiddenLiterals[j])
							{
								return FALSE;
							}
							j++;
						}
					}
				}
				i++;
			}
		}
		return TRUE;
	}
	return FALSE;
}

HRESULT BasicFileNPathOperations::CountNextLevel(HANDLE hFind_sub, LPWIN32_FIND_DATA ffd_count_sub, TCHAR *subitem, int& items, DWORD *files, DWORD *folders)
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
										hr = this->CountNextLevel(hFind_sub_new, &ffd_count_sub_new, subitem_new, items, files, folders);
									}
									else
									{
										// count one for the empty item ...
										items++;
										*folders += 1;
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
			*folders += 1;
		}
		else
		{
			items++;
			*files += 1;
		}
	} while (FindNextFile(hFind_sub, ffd_count_sub) != 0);

	return hr;
}

BOOL BasicFileNPathOperations::RemoveWildcard(TCHAR *path)
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
