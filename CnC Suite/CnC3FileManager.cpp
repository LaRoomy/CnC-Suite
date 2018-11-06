#include"CnC3FileManager.h"
#include"BasicFPO.h"
#include"HelperF.h"

CnC3File::CnC3File()
	: status(E_NOT_SET), skippathsaving(false)
{
}

CnC3File::CnC3File(const CnC3File & file)
	: skippathsaving(false)
{
	this->copy(file);
}

HRESULT CnC3File::Load()
{
	this->skippathsaving = true;

	return this->Load(
		this->pathToFile.GetData()
	);
}

PropertyID CnC3File::PID_DESCRIPTION_ONE = L"descfield1";
PropertyID CnC3File::PID_DESCRIPTION_TWO = L"descfield2";
PropertyID CnC3File::PID_DESCRIPTION_THREE = L"descfield3";

HRESULT CnC3File::Load(LPCTSTR path)
{
	HRESULT hr =
		(path != nullptr)
		? S_OK : E_FAIL;

	if (SUCCEEDED(hr))
	{
		if (!this->skippathsaving)
			this->pathToFile = path;
		else
			this->skippathsaving = false;

		auto bfpo = CreateBasicFPO();
		hr =
			(bfpo != nullptr)
			? S_OK : E_FAIL;

		if(SUCCEEDED(hr))
		{
			TCHAR* buffer = nullptr;

			hr =
				bfpo->LoadBufferFmFileAsUtf8(
					&buffer,
					this->pathToFile.GetData()
				)
				? S_OK : E_FAIL;

			if(SUCCEEDED(hr))
			{
				hr = this->Buffer_toCnC3Object(buffer);				

				SafeDeleteArray(&buffer);
			}
			SafeRelease(&bfpo);
		}
	}
	this->SetStatus(hr);
	return hr;
}

HRESULT CnC3File::Save()
{
	this->skippathsaving = true;

	return this->Save(
		this->pathToFile.GetData()
	);
}

HRESULT CnC3File::Save(LPCTSTR path)
{
	HRESULT hr =
		(path != nullptr)
		? S_OK : E_FAIL;

	if (SUCCEEDED(hr))
	{
		if (!this->skippathsaving)
			this->pathToFile = path;
		else
			this->skippathsaving = false;

		iString buffer;

		hr = this->CnC3Object_toBuffer(buffer);
		if (SUCCEEDED(hr))
		{
			auto bfpo = CreateBasicFPO();
			hr =
				(bfpo != nullptr)
				? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				hr =
					bfpo->SaveBufferToFileAsUtf8(
						buffer.GetData(),
						this->pathToFile.GetData()
					)
					? S_OK : E_FAIL;

				SafeRelease(&bfpo);
			}
		}
	}
	return hr;
}

void CnC3File::Clear()
{
	this->status = E_NOT_SET;
	this->ncContent.Clear();
	this->pathToFile.Clear();
	this->cnc3PropertyMap.Clear();

	// TODO !!!!!!!!!!!!!!!!!!!!!!!!!
}

void CnC3File::copy(const CnC3File & file)
{
	this->Clear();

	this->ncContent = file.ncContent;
	this->pathToFile = file.pathToFile;

	this->cnc3PropertyMap = file.cnc3PropertyMap;


	// TODO !!!!!!!!!!!!!!!!!!!!!!!!!
}

HRESULT CnC3File::CnC3Object_toBuffer(iString& buffer)
{
	iString propString, key, data;

	buffer.Replace(L"[?cnc3 version=\"2.0\" encoding=\"utf-8\"]\r\n[PROPERTY-SECTION START]\r\n");

	auto propCount = this->cnc3PropertyMap.GetCount();

	for (int i = 0; i < propCount; i++)
	{
		this->cnc3PropertyMap.GetPairAtIndex(i, key, data);

		propString.Clear();
		propString = L"CNC3PKEY[";
		propString += key;
		propString += L"]KEYEND";
		propString += L" CNC3KEYPDATA[";
		propString += data;
		propString += L"]DATAEND\r\n";

		buffer += propString;
	}
	buffer += L"[PROPERTY-SECTION END]\r\n";

	return E_NOTIMPL;
}

HRESULT CnC3File::Buffer_toCnC3Object(LPCTSTR buffer)
{
	this->cnc3PropertyMap.Clear();
	this->ncContent.Clear();

	// at first figure out the version of cnc3 format - if this is version 1.0, use the legacy functionality
	auto len =
		_lengthOfString(buffer);

	auto hr =
		(len > 0)
		? S_OK : E_FAIL;

	if(SUCCEEDED(hr))
	{
		bool isOlderVersion = false;

		if (len > 38)//minimum length for the 2.0 Version
		{
			TCHAR versionString[56] = { 0 };

			for (int i = 0; i < 38; i++)
			{
				versionString[i] = buffer[i];
			}

			if (CompareStringsAsmW(L"[?cnc3 version=\"2.0\" encoding=\"utf-8\"]", versionString) != 1)
			{
				isOlderVersion = true;
			}
		}
		else
			isOlderVersion = true;

		if (isOlderVersion)// legacy procedure
		{
			hr =
				this->Extract(buffer)
				? S_OK : E_FAIL;
		}
		else // normal procedure
		{
			hr = this->ReadPropertySection(buffer);
			if (SUCCEEDED(hr))
			{
				hr = this->ReadContentSection(buffer);
				if (SUCCEEDED(hr))
				{
					// read other sections
				}
			}
		}
	}
	return hr;
}

HRESULT CnC3File::ReadPropertySection(LPCTSTR buffer)
{
	return E_NOTIMPL;
}

HRESULT CnC3File::ReadContentSection(LPCTSTR buffer)
{
	return E_NOTIMPL;
}

BOOL CnC3File::Extract(LPCTSTR buffer)
{	
	if (buffer == nullptr)
	{
		return FALSE;
	}

	int i = 0, property_start = -5;

	do
	{
		if (i > 0)
		{
			i += 1;
		}
		while (buffer[i] != L'[')
		{
			if (buffer[i] == L'\0')
			{
				break;
			}
			i++;
		}
		if (buffer[i] == L'\0')
		{
			break;
		}
	} while ((property_start = this->SearchFor_Property_begin(i, buffer, 9, L"[PROPERTY]")) == -5);

	if (i == 0)
	{
		return TRUE;
	}

	auto tempArray = new TCHAR[i + 1];

	if (tempArray != nullptr)
	{
		SecureZeroMemory(tempArray, sizeof(TCHAR) * (i + 1));

		if (property_start == -5)
		{
			StringCbCopy(
				tempArray,
				sizeof(WCHAR) * (i + 1),
				buffer
			);
		}
		else
		{
			for (int j = 0; j < i; j++)
			{
				tempArray[j] = buffer[j];
			}
			tempArray[i] = L'\0';

			for (int j = 0; j < 3; j++)
			{
				property_start = this->Get_Property(property_start, j, buffer);

				if (property_start == -4)
				{
					return FALSE;
				}
			}
		}
		this->ncContent.Replace(tempArray);

		SafeDeleteArray(&tempArray);
	}
	else
		return FALSE;

	return TRUE;
}

int CnC3File::SearchFor_Property_begin(int start_pos, LPCTSTR buffer, int num_char_sample, LPCTSTR sample)
{
	int prop_cnt = 0;

	while (sample[prop_cnt] == buffer[start_pos])
	{
		if (prop_cnt == num_char_sample)
		{
			return start_pos + 1;
		}
		if (buffer[start_pos] == L'\0')
		{
			return start_pos;
		}
		prop_cnt++;
		start_pos++;
	}
	return -5;
}

int CnC3File::Get_Property(int start_pos, int prop_number, LPCTSTR buffer)
{
	int array_size = 0, array_start = start_pos;

	while (this->SearchFor_Property_begin(start_pos, buffer, 12, L"[ENDPROPERTY]") == -5)
	{
		if (buffer[start_pos] == L'\0')
		{
			return -4;
		}
		start_pos++;
		array_size++;
	}

	TCHAR *tempArray = nullptr;

	if (prop_number == 0)
	{
		tempArray = new TCHAR[array_size + 2];

		if (tempArray != nullptr)
		{
			for (int l = 0; l < array_size; l++)
			{
				tempArray[l] = buffer[array_start + l];
			}
			tempArray[array_size] = L'\0';

			this->cnc3PropertyMap.Add(CnC3File::PID_DESCRIPTION_ONE, tempArray);

			SafeDeleteArray(&tempArray);
		}
	}
	else if (prop_number == 1)
	{
		tempArray = new TCHAR[array_size + 2];

		if (tempArray != nullptr)
		{
			for (int l = 0; l < array_size; l++)
			{
				tempArray[l] = buffer[array_start + l];
			}
			tempArray[array_size] = L'\0';

			this->cnc3PropertyMap.Add(CnC3File::PID_DESCRIPTION_TWO, tempArray);

			SafeDeleteArray(&tempArray);
		}
	}
	else if (prop_number == 2)
	{
		tempArray = new TCHAR[array_size + 2];

		for (int l = 0; l < array_size; l++)
		{
			tempArray[l] = buffer[array_start + l];
		}
		tempArray[array_size] = L'\0';

		this->cnc3PropertyMap.Add(CnC3File::PID_DESCRIPTION_THREE, tempArray);

		SafeDeleteArray(&tempArray);
	}
	int new_pos = 0;

	if (prop_number < 2)
	{
		while ((new_pos = this->SearchFor_Property_begin(start_pos, buffer, 9, L"[PROPERTY]")) == -5)
		{
			start_pos++;
		}
		if (new_pos == -6)
		{
			return -4;
		}
	}
	return new_pos;
}

CnC3FileManager::CnC3FileManager()
	: owner(nullptr)
{
}

CnC3FileManager::CnC3FileManager(HWND _owner_)
	: owner(_owner_)
{
}

CnC3FileCollection& CnC3FileManager::Open()
{
	IFileOpenDialog *Ifd = nullptr;

	// create the IFileOpenDialog object
	HRESULT hr =
		CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&Ifd));

	if (SUCCEEDED(hr))
	{
		// set options on the dialog
		FILEOPENDIALOGOPTIONS optionFlags;

		hr = Ifd->GetOptions(&optionFlags);
		if (SUCCEEDED(hr))
		{
			hr = Ifd->SetOptions(optionFlags | FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT);
			if (SUCCEEDED(hr))
			{
				// set custom dialog text
				hr = this->setDialogText(Ifd);
				if (SUCCEEDED(hr))
				{
					// set the default target folder on the dialog
					hr = this->setTargetFolder(Ifd);
					if (SUCCEEDED(hr))
					{
						// set the filetype selection in the dialog's dropdown box
						hr = Ifd->SetFileTypes(ARRAYSIZE(CnC3DataType), CnC3DataType);
						if (SUCCEEDED(hr))
						{
							// select the filetype
							hr = Ifd->SetFileTypeIndex(0);
							if (SUCCEEDED(hr))
							{
								hr = Ifd->SetDefaultExtension(L".cnc3");
								if (SUCCEEDED(hr))
								{
									// show the dialog
									hr = Ifd->Show(this->owner);
									if (SUCCEEDED(hr))
									{
										// obtain the results
										IShellItemArray *dlgResults = nullptr;

										hr = Ifd->GetResults(&dlgResults);
										if (SUCCEEDED(hr))
										{
											DWORD numResults;

											hr = dlgResults->GetCount(&numResults);
											if (SUCCEEDED(hr))
											{
												// get the path(s) of the file(s)
												IShellItem *sItem;
												PTSTR filepath;
												CnC3File file;

												this->openResult.Clear();

												for (DWORD i = 0; i < numResults; i++)
												{
													sItem = nullptr;
													filepath = nullptr;

													hr = dlgResults->GetItemAt(i, &sItem);
													if (SUCCEEDED(hr))
													{
														hr = sItem->GetDisplayName(SIGDN_FILESYSPATH, &filepath);
														if (SUCCEEDED(hr))
														{
															file.Clear();
															file.Load(filepath);

															this->openResult.Add(file);

															CoTaskMemFree(filepath);
														}
														sItem->Release();
													}
												}
											}
											dlgResults->Release();
										}
									}
								}
							}
						}
					}
				}
			}
		}
		Ifd->Release();
	}
	this->openResult = hr;
	return this->openResult;
}

CnC3File& CnC3FileManager::Open(LPCTSTR path)
{
	if (path == nullptr)
	{
		this->currentFile.SetStatus(E_INVALIDARG);
		return this->currentFile;
	}
	else
	{
		this->currentFile.Clear();
		this->currentFile.Load(path);
		return this->currentFile;
	}
}

CnC3File & CnC3FileManager::SaveAs(const CnC3File & file)
{
	// TODO:

	return this->currentFile;
}

HRESULT CnC3FileManager::Save(const CnC3File & file)
{

	return E_NOTIMPL;
}

void CnC3FileManager::SetDialogText(LPCTSTR CaptionText, LPCTSTR ButtonText, LPCTSTR FileText)
{
	this->captiontext = CaptionText;
	this->buttontext = ButtonText;
	this->filetext = FileText;
}

void CnC3FileManager::SetTargetFolder(LPCTSTR Path)
{
	this->targetfolder = Path;
}

void CnC3FileManager::SetHwndOwner(HWND Owner)
{
	this->owner = Owner;
}

HRESULT CnC3FileManager::setDialogText(IFileDialog *Ifd)
{
	HRESULT hr =
		(Ifd != nullptr)
		? S_OK : E_POINTER;

	if (SUCCEEDED(hr))
	{
		if (this->captiontext.GetLength() > 0)
		{
			Ifd->SetTitle(
				this->captiontext.GetData()
			);
		}
		if (this->buttontext.GetLength() > 0)
		{
			Ifd->SetOkButtonLabel(
				this->buttontext.GetData()
			);
		}
		if (this->filetext.GetLength() > 0)
		{
			Ifd->SetFileName(
				this->filetext.GetData()
			);
		}
	}
	return hr;
}

HRESULT CnC3FileManager::setTargetFolder(IFileDialog * Ifd)
{
	IShellItem *sItem = nullptr;

	HRESULT hr =
		(Ifd != nullptr)
		? S_OK : E_POINTER;

	if (SUCCEEDED(hr))
	{
		if (this->targetfolder.GetLength() > 0)
		{
			hr =
				SHCreateItemFromParsingName(
					this->targetfolder.GetData(),
					nullptr,
					IID_PPV_ARGS(&sItem)
				);
			if (SUCCEEDED(hr))
			{
				hr = Ifd->SetFolder(sItem);

				sItem->Release();
			}
		}
	}
	return hr;
}

