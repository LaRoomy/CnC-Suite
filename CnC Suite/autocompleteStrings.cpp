#include "autocompleteStrings.h"
#include "AppPath.h"


void autocompleteStrings::add(const TCHAR * trigger, const TCHAR * appendix, int length)
{
	if (trigger != nullptr && appendix != nullptr && length > 0)
	{
		if (this->pStrings != nullptr)
		{
			LPAUTOCOMPLETESTRINGS pHolder = nullptr;

			if (autocompleteStrings::Copy(this->count, this->pStrings, &pHolder))
			{
				delete[] this->pStrings;
				this->pStrings = new AUTOCOMPLETESTRINGS[this->count + 1];
				if (this->pStrings != nullptr)
				{
					if (autocompleteStrings::Copy(this->count, pHolder, &this->pStrings))
					{
						this->count++;

						auto index = this->count - 1;
						if (index >= 0)
						{
							StringCbCopy(this->pStrings[index].appendix, sizeof(TCHAR) * 56, appendix);
							StringCbCopy(this->pStrings[index].trigger, sizeof(TCHAR) * 56, trigger);
							this->pStrings[index].length = length;

							this->toFile();
						}
					}
				}
				delete[] pHolder;
			}
		}
		else
		{
			this->pStrings = new AUTOCOMPLETESTRINGS;

			StringCbCopy(this->pStrings->appendix, sizeof(TCHAR) * 56, appendix);
			StringCbCopy(this->pStrings->trigger, sizeof(TCHAR) * 56, trigger);
			this->pStrings->length = length;

			this->count = 1;

			this->toFile();
		}
	}

}

void autocompleteStrings::deleteAt(int index)
{
	if (index < this->count && index >= 0)
	{
		if (this->pStrings != nullptr)
		{
			LPAUTOCOMPLETESTRINGS pHolder = nullptr;

			if (autocompleteStrings::Copy(this->count, this->pStrings, &pHolder))
			{
				if (this->count == 1)delete this->pStrings;
				else delete[] this->pStrings;

				this->pStrings = new AUTOCOMPLETESTRINGS[this->count - 1];
				if (this->pStrings != nullptr)
				{
					int j = 0;

					for (int i = 0; i < this->count; i++)
					{
						if (i != index)
						{
							StringCbCopy(this->pStrings[j].appendix, sizeof(TCHAR) * 56, pHolder[i].appendix);
							StringCbCopy(this->pStrings[j].trigger, sizeof(TCHAR) * 56, pHolder[i].trigger);
							this->pStrings[j].length = pHolder[i].length;

							j++;
						}
					}
					this->count--;
					this->toFile();
				}
			}
		}
	}
}

void autocompleteStrings::updateAt(int index, const TCHAR * trigger, const TCHAR * appendix, int length)
{
	if (index < 0 || index >= this->count)return;
	else
	{
		if (trigger != nullptr)
		{
			StringCbCopy(this->pStrings[index].trigger, sizeof(TCHAR) * 56, trigger);
			this->pStrings[index].length = length;
		}
		if (appendix != nullptr)
		{
			StringCbCopy(this->pStrings[index].appendix, sizeof(TCHAR) * 56, appendix);
		}
		this->toFile();
	}
}

iString autocompleteStrings::getTriggerAt(int index)
{
	iString retV(L"fail");

	if (index >= 0 && index < this->count)
	{
		retV.Replace(this->pStrings[index].trigger);
	}
	return retV;
}

iString autocompleteStrings::getAppendixAt(int index)
{
	iString retV(L"fail");

	if (index >= 0 && index < this->count)
	{
		retV.Replace(this->pStrings[index].appendix);
	}
	return retV;
}

bool autocompleteStrings::setContent(int len, LPAUTOCOMPLETESTRINGS data_in)
{
	__try
	{
		HRESULT hr;
		this->clear();
		this->pStrings = new AUTOCOMPLETESTRINGS[len];
		hr = (this->pStrings != nullptr) ? S_OK : E_OUTOFMEMORY;
		if (SUCCEEDED(hr))
		{
			for (int i = 0; i < len; i++)
			{
				hr = StringCbCopy(this->pStrings[i].appendix, sizeof(TCHAR) * 56, data_in[i].appendix);
				if (SUCCEEDED(hr))
				{
					hr = StringCbCopy(this->pStrings[i].trigger, sizeof(TCHAR) * 56, data_in[i].trigger);
					this->pStrings[i].length = data_in[i].length;
				}
				if (FAILED(hr))
				{
					break;
				}
			}
		}
		return SUCCEEDED(hr) ? true : false;
	}
	__except (
		GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH
		)
	{
		return false;
	}
}

int autocompleteStrings::getContent(LPAUTOCOMPLETESTRINGS * ppData_out) const
{
	__try
	{
		if (this->pStrings != nullptr)
		{
			if (*ppData_out == nullptr)
			{
				*ppData_out = new AUTOCOMPLETESTRINGS[this->count];
			}
			if (*ppData_out != nullptr)
			{
				HRESULT hr = S_OK;

				for (int i = 0; i < this->count; i++)
				{
					hr = StringCbCopy((*ppData_out)[i].appendix, sizeof(TCHAR) * 56, this->pStrings[i].appendix);
					if (SUCCEEDED(hr))
					{
						hr = StringCbCopy((*ppData_out)[i].trigger, sizeof(TCHAR) * 56, this->pStrings[i].trigger);
						(*ppData_out)[i].length = this->pStrings[i].length;
					}
					if (FAILED(hr))
					{
						return -3;
					}
				}
			}
			else
				return -2;
		}
		else
			return -1;

		return this->count;
	}
	__except (
		GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH
		)
	{
		return -9;
	}
}

bool autocompleteStrings::Copy(int len, LPAUTOCOMPLETESTRINGS data_in, LPAUTOCOMPLETESTRINGS * ppData_out)
{
	__try
	{
		if (len == 0)return false;
		else
		{
			if (data_in == nullptr)return false;
			else
			{
				if (*ppData_out == nullptr)
				{
					*ppData_out = new AUTOCOMPLETESTRINGS[len];
				}
				if (*ppData_out == nullptr)return false;
				else
				{
					HRESULT hr = S_OK;

					for (int i = 0; i < len; i++)
					{
						hr = StringCbCopy((*ppData_out)[i].appendix, sizeof(TCHAR) * 56, data_in[i].appendix);
						if (SUCCEEDED(hr))
						{
							hr = StringCbCopy((*ppData_out)[i].trigger, sizeof(TCHAR) * 56, data_in[i].trigger);
							(*ppData_out)[i].length = data_in[i].length;
						}
						if (FAILED(hr))
						{
							return false;
						}
					}
				}
			}
			return true;
		}
	}
	__except (
		GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH
		)
	{
		return false;
	}
}

void autocompleteStrings::clear()
{
	if (this->pStrings != nullptr)
	{
		delete[] this->pStrings;
		this->pStrings = nullptr;
	}
	this->count = 0;
}

void autocompleteStrings::toFile()
{
	this->startAsyncOperation(AC_ASYNCOPERATION_SAVE);
}

void autocompleteStrings::fromFile(DWORD executeMode)
{
	this->fLoading = FLOADING_PENDING;

	auto parser = new XML_Parser(nullptr);
	if (parser != nullptr)
	{
		AppPath pathManager;

		iString path(
			pathManager.Get(PATHID_FOLDER_CNCSUITE_APPDATA_STRINGS)
		);
		path += L"\\autocomplete.xml";

		if (parser->OpenDocument(path))
		{
			if (executeMode == AC_EXECUTE_ASYNC)
			{
				parser->setEventListener(dynamic_cast<XMLParsingEventSink*>(this));
				parser->ParseAsync();
			}
			else
			{
				this->fLoading = parser->Parse()
					? FLOADING_LOADSUCCEDED : FLOADING_FAILED;

				if (this->fLoading == FLOADING_LOADSUCCEDED)
				{
					auto xmlStructure = parser->getDocumentStructure();

					this->xmlToStruct(&xmlStructure);
				}
			}
		}
		else
		{
			this->fLoading = FLOADING_FILENOTFOUND;
		}
		SafeRelease(&parser);
	}
}

DWORD autocompleteStrings::saveProc(LPVOID lParam)
{
	auto _this = reinterpret_cast<autocompleteStrings*>(lParam);
	if (_this != nullptr)
	{
		auto res = _this->structToFile();
		if (!res)
			return 2;
	}
	else
		return 1;

	return 0;
}

DWORD autocompleteStrings::loadProc(LPVOID lParam)
{
	auto _this = reinterpret_cast<autocompleteStrings*>(lParam);
	if (_this != nullptr)
	{
		
	}
	else
		return 1;

	return 0;
}

void autocompleteStrings::startAsyncOperation(DWORD type)
{
	DWORD threadID;
	HANDLE hThread;
	LPTHREAD_START_ROUTINE routine;

	if (type == AC_ASYNCOPERATION_SAVE)
		routine = autocompleteStrings::saveProc;
	else if (type == AC_ASYNCOPERATION_LOAD)
		routine = autocompleteStrings::loadProc;
	else
		return;

	hThread = CreateThread(nullptr, 0, routine, reinterpret_cast<LPVOID>(this), 0, &threadID);
	if (hThread)
	{
		WaitForSingleObject(hThread, 20);
		CloseHandle(hThread);
	}
}

bool autocompleteStrings::xmlToStruct(itemCollection<iXML_Tag>* xml_)
{
	if (xml_ != nullptr)
	{
		this->clear();
		this->count = xml_->GetCount();

		this->pStrings = new AUTOCOMPLETESTRINGS[this->count];
		if (this->pStrings != nullptr)
		{
			HRESULT hr = S_OK;

			for (int i = 0; i < this->count; i++)
			{
				hr = StringCbCopy(
					this->pStrings[i].appendix,
					sizeof(TCHAR) * 56,
					xml_->GetAt(i)
					.getPropertyContentFromName(L"appendix")
					.GetData()
				);
				if (SUCCEEDED(hr))
				{
					hr = StringCbCopy(
						this->pStrings[i].trigger,
						sizeof(TCHAR) * 56,
						xml_->GetAt(i)
						.getPropertyContentFromName(L"trigger")
						.GetData()
					);
					if (SUCCEEDED(hr))
					{
						this->pStrings[i].length =
							xml_->GetAt(i)
							.getPropertyContentFromName(L"length")
							.getAsInt();
					}
				}
				if (FAILED(hr))
					break;
			}
			return SUCCEEDED(hr) ? true : false;
		}
		return true;
	}
	else
		return false;
}

bool autocompleteStrings::structToFile()
{
	if (this->count > 0)
	{
		auto builder = new XML_Builder();
		if (builder != nullptr)
		{
			iXML_Tag tag;
			XML_TAG_Property appendix;
			appendix.propertyName.Replace(L"appendix");
			XML_TAG_Property trigger;
			trigger.propertyName.Replace(L"trigger");
			XML_TAG_Property length;
			length.propertyName.Replace(L"length");

			AppPath pathManager;

			iString path(
				pathManager.Get(PATHID_FOLDER_CNCSUITE_APPDATA_STRINGS)
			);
			path += L"\\autocomplete.xml";

			builder->setUp_DocumentDefinition(L"1.0", L"utf-8", L"acmpl", true);
			tag.hasProperties = true;
			tag.initallyClosed = true;
			tag.tagName.Replace(L"acstring");

			for (int i = 0; i < this->count; i++)
			{
				tag.tagProperties.Clear();

				appendix.propertyContent.Replace(this->pStrings[i].appendix);
				trigger.propertyContent.Replace(this->pStrings[i].trigger);
				length.propertyContent.Replace(
					iString::fromInt(this->pStrings[i].length));
				
				tag.tagProperties.AddItem(appendix);
				tag.tagProperties.AddItem(trigger);
				tag.tagProperties.AddItem(length);

				builder->AddTag(&tag);
			}

			// it is not neccessary to use an async method because we are already in a non ui-thread!

			builder->finalizeAsync(path, true);

			return true;
		}
		else
			return false;
	}
	else
		return false;
}
