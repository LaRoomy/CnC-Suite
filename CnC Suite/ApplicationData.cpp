#include"ApplicationData.h"
#include"HelperF.h"


ApplicationData::ApplicationData()
	:success(false),
	//preThreadActive(0),
	saverThreadActive(0),
	XMLSettingsStructure(nullptr)
{}

ApplicationData::~ApplicationData()
{
	this->waitForThreadsToFinish();

	SafeRelease(&this->XMLSettingsStructure);
}

bool ApplicationData::selectFilekey(FILEKEY key)
{
	// clear the class if a previous key was loaded
	this->Reset();

	bool exist = false;

	AppPath path;

	auto settingsPath = path.Get(PATHID_FOLDER_CNCSUITE_APPDATA_SETTINGS);
	if(settingsPath.succeeded())
	{
		settingsPath += L"\\";
		settingsPath += (TCHAR*)key;
		settingsPath += L".xml";

		this->applicationDataContainerPath = settingsPath;
		this->filekey.Replace(key);

		XML_Parser* parser = new XML_Parser(dynamic_cast<XMLParsingEventSink*>(this));
		if (parser != nullptr)
		{
			if (parser->OpenDocument(this->applicationDataContainerPath))
			{
				parser->Parse();

				exist = true;
			}
			else
			{
				SafeRelease(&parser);
			}
		}
	}
	return exist;
}

//void ApplicationData::saveValueAsync(DATAKEY key, int value)
//{
//	this->startSaveAsync(key, value, false, nullptr, DATATYPE_INTEGER);
//}
//
//void ApplicationData::saveValueAsync(DATAKEY key, bool value)
//{
//	this->startSaveAsync(key, 0, value, nullptr, DATATYPE_BOOLEAN);	
//}
//
//void ApplicationData::saveValueAsync(DATAKEY key, iString value)
//{
//	this->startSaveAsync(key, 0, false, &value, DATATYPE_ISTRING);
//}
//
//void ApplicationData::saveValueAsync(DATAKEY key, LPCTSTR value)
//{
//	iString val(value);
//	this->startSaveAsync(key, 0, false, &val, DATATYPE_ISTRING);
//}

bool ApplicationData::saveValue(DATAKEY key, int value)
{
	if (this->validateStructure())
	{
		int keyindex = this->validateKey(key);

		if (keyindex == -1)
		{
			iXML_Tag tag;
			XML_TAG_Property propertY;
			propertY.propertyName.Replace(L"key");
			propertY.propertyContent.Replace(key);
			tag.hasProperties = true;
			tag.initallyClosed = false;
			tag.tagProperties.AddItem(propertY);
			propertY.propertyName.Replace(L"type");
			propertY.propertyContent.Replace(L"integer");
			tag.tagProperties.AddItem(propertY);
			tag.tagName.Replace(L"data");
			tag.tagContent.Replace(
				(iString::FromInt(value)
					)->GetData()
			);
			this->XMLSettingsStructure->AddItem(tag);
		}
		else
		{
			auto setting = this->XMLSettingsStructure->GetAt(keyindex);
			setting
				.tagContent
				.Replace(
					(iString::FromInt(value)
					)->GetData()
				);
			this->XMLSettingsStructure->RemoveAt(keyindex);
			this->XMLSettingsStructure->AddItem(setting);
		}
		this->startSaverThread();

		return true;
	}
	else
		return false;
}

bool ApplicationData::saveValue(DATAKEY key, unsigned int value)
{
	if (this->validateStructure())
	{
		int keyindex = this->validateKey(key);

		if (keyindex == -1)
		{
			iXML_Tag tag;
			XML_TAG_Property propertY;
			propertY.propertyName.Replace(L"key");
			propertY.propertyContent.Replace(key);
			tag.hasProperties = true;
			tag.initallyClosed = false;
			tag.tagProperties.AddItem(propertY);
			propertY.propertyName.Replace(L"type");
			propertY.propertyContent.Replace(L"unsigned_integer");
			tag.tagProperties.AddItem(propertY);
			tag.tagName.Replace(L"data");
			tag.tagContent.Replace(
				(iString::FromUInt(value)
					)->GetData()
			);
			this->XMLSettingsStructure->AddItem(tag);
		}
		else
		{
			auto setting = this->XMLSettingsStructure->GetAt(keyindex);
			setting
				.tagContent
				.Replace(
				(iString::FromUInt(value)
					)->GetData()
				);
			this->XMLSettingsStructure->RemoveAt(keyindex);
			this->XMLSettingsStructure->AddItem(setting);
		}
		this->startSaverThread();

		return true;
	}
	else
		return false;
}

bool ApplicationData::saveValue(DATAKEY key, const iString & string)
{
	if (this->validateStructure())
	{
		int keyindex = this->validateKey(key);

		if (keyindex == -1)
		{
			iXML_Tag tag;
			XML_TAG_Property propertY;
			propertY.propertyName.Replace(L"key");
			propertY.propertyContent.Replace(key);
			tag.hasProperties = true;
			tag.initallyClosed = false;
			tag.tagProperties.AddItem(propertY);
			propertY.propertyName.Replace(L"type");
			propertY.propertyContent.Replace(L"string");
			tag.tagProperties.AddItem(propertY);
			tag.tagName.Replace(L"data");
			tag.tagContent.Replace(string);

			this->XMLSettingsStructure->AddItem(tag);
		}
		else
		{
			auto setting = this->XMLSettingsStructure->GetAt(keyindex);
			setting
				.tagContent
				.Replace(string);
			this->XMLSettingsStructure->RemoveAt(keyindex);
			this->XMLSettingsStructure->AddItem(setting);
		}
		this->startSaverThread();

		return true;
	}
	else
		return false;
}

bool ApplicationData::saveValue(DATAKEY key, const TCHAR * string)
{
	iString str(string);
	return this->saveValue(key, str);
}

bool ApplicationData::saveValue(DATAKEY key, bool value)
{
	if (this->validateStructure())
	{
		int keyindex = this->validateKey(key);

		if (keyindex == -1)
		{
			iXML_Tag tag;
			XML_TAG_Property propertY;
			propertY.propertyName.Replace(L"key");
			propertY.propertyContent.Replace(key);
			tag.hasProperties = true;
			tag.initallyClosed = false;
			tag.tagProperties.AddItem(propertY);
			propertY.propertyName.Replace(L"type");
			propertY.propertyContent.Replace(L"boolean");
			tag.tagProperties.AddItem(propertY);
			tag.tagName.Replace(L"data");
			tag.tagContent.Replace(
				(iString::FromBoolean(value)
					)->GetData()
			);
			this->XMLSettingsStructure->AddItem(tag);
		}
		else
		{
			auto setting = this->XMLSettingsStructure->GetAt(keyindex);

			auto _objValue = iString::FromBoolean(value);
			if (_objValue != nullptr)
			{
				setting
					.tagContent
					.Replace(
						_objValue->GetData()
					);

				SafeRelease(&_objValue);
			}
			this->XMLSettingsStructure->RemoveAt(keyindex);
			this->XMLSettingsStructure->AddItem(setting);
		}
		this->startSaverThread();

		return true;
	}
	else
		return false;
}

void ApplicationData::deleteValue(DATAKEY key)
{
	int keyindex = this->validateKey(key);
	if (keyindex >= 0)
	{
		this->XMLSettingsStructure->RemoveAt(keyindex);
		this->startSaverThread();
	}
}

cObject ApplicationData::lookUp(_In_ DATAKEY key)
{
	if (this->XMLSettingsStructure != nullptr)
	{
		auto numSettings = this->XMLSettingsStructure->GetCount();

		if (numSettings > 0)
		{
			for (int i = 0; i < numSettings; i++)
			{
				auto setting = this->XMLSettingsStructure->GetAt(i);
				auto keyname = setting.getPropertyContentFromName(L"key");
				if (keyname.GetLength() > 0)
				{
					if (keyname.Equals(key))
					{
						auto dataType =
							setting.getPropertyContentFromName(L"type");

						try
						{
							if (dataType.Equals(L"boolean"))
							{
								auto box = new iBox<bool>();

								if (setting.tagContent.Equals(L"true"))
									box->set(true);
								else
									box->set(false);

								return reinterpret_cast<cObject>(box);
							}
							else if (dataType.Equals(L"string"))
							{
								iString str(setting.tagContent);
								auto box = new iBox<iString>(str);

								return reinterpret_cast<cObject>(box);
							}
							else if (dataType.Equals(L"integer"))
							{
								auto box = new iBox<int>(
									setting.tagContent.getAsInt()
									);
								return reinterpret_cast<cObject>(box);
							}
							else if (dataType.Equals(L"unsigned_integer"))
							{
								auto box = new iBox<unsigned int>(
									setting.tagContent.getAsUInt()
									);
								return reinterpret_cast<cObject>(box);
							}
						}
						catch (DataAccessViolationException dave)
						{
							this->deleteValue(key);
							return reinterpret_cast<cObject>(nullptr);
						}
					}
				}
			}
		}
	}
	return reinterpret_cast<cObject>(nullptr);
}

bool ApplicationData::getBooleanData(DATAKEY key, bool defaultvalue)
{
	auto ibox = reinterpret_cast<iBox<bool>*>(
		this->lookUp(key));
	if (ibox != nullptr)
	{
		bool val = ibox->get();
		ibox->Release();

		return val;
	}
	else
	{
		return defaultvalue;
	}
}

int ApplicationData::getIntegerData(DATAKEY key, int defaultvalue)
{
	auto ibox = reinterpret_cast<iBox<int>*>(
		this->lookUp(key)
		);
	if (ibox != nullptr)
	{
		int val = ibox->get();
		ibox->Release();
		return val;
	}
	else
	{
		return defaultvalue;
	}
}

unsigned int ApplicationData::getUnsignedIntegerData(DATAKEY key, unsigned int defaultvalue)
{
	auto ibox = reinterpret_cast<iBox<unsigned int>*>(
		this->lookUp(key)
	);
	if (ibox != nullptr)
	{
		unsigned int val = ibox->get();
		ibox->Release();
		return val;
	}
	else
	{
		return defaultvalue;
	}
}

iString ApplicationData::getStringData(DATAKEY key, const iString & defaultvalue)
{
	auto ibox = reinterpret_cast<iBox<iString>*>(
		this->lookUp(key));

	if (ibox != nullptr)
	{
		iString val(ibox->get());
		ibox->Release();

		return val;
	}
	else
	{
		return defaultvalue;
	}
}

iString ApplicationData::getStringData(DATAKEY key, LPCTSTR defaultvalue)
{
	auto ibox = reinterpret_cast<iBox<iString>*>(
		this->lookUp(key));

	if (ibox != nullptr)
	{
		iString val(
			ibox->get()
		);
		ibox->Release();

		return val;
	}
	else
	{
		return iString(defaultvalue);
	}
}

iString ApplicationData::getStringData(DATAKEY key)
{
	auto ibox = reinterpret_cast<iBox<iString>*>(
		this->lookUp(key));

	if (ibox != nullptr)
	{
		iString val(ibox->get());
		ibox->Release();

		return val;
	}
	else
	{
		return iString(L"");
	}
}

DWORD ApplicationData::saveProc(LPVOID lParam)
{
	auto _this = reinterpret_cast<ApplicationData*>(lParam);
	if (_this != nullptr)
	{
		//if (_this->preThreadActive)
		//{
		//	Sleep(50);

		//	if(_this->preThreadActive)
		//		return 102;
		//}

		while (_this->saverThreadActive) { Sleep(4); }

		InterlockedExchange((LONG*)&_this->saverThreadActive, (LONG)1);

		_this->toFile();

		InterlockedExchange((LONG*)&_this->saverThreadActive, (LONG)0);
		
		return 101;
	}
	else
		return 100;
}

//DWORD ApplicationData::preSaveProc(LPVOID lParam)
//{
//	auto std = reinterpret_cast<LPSAVERTHREADDATA>(lParam);
//	if (std != nullptr)
//	{
//		auto _this = reinterpret_cast<ApplicationData*>(std->_this);
//		if (_this != nullptr)
//		{
//			while (_this->preThreadActive) { Sleep(5); }
//
//			InterlockedExchange((LONG*)&_this->preThreadActive, (LONG)1);
//
//			switch (std->type)
//			{
//			case DATATYPE_BOOLEAN:
//				_this->saveValue(std->key, std->bData);
//				break;
//			case DATATYPE_INTEGER:
//				_this->saveValue(std->key, std->iData);
//				break;
//			case DATATYPE_ISTRING:
//				_this->saveValue(std->key, std->sData);
//				break;
//			default:
//				break;
//			}
//
//			InterlockedExchange((LONG*)&_this->preThreadActive, (LONG)0);
//
//			SafeDelete(&std);
//			return 11;
//		}
//		SafeDelete(&std);
//	}
//	return 10;
//}

void ApplicationData::startSaverThread()
{
	if (this->XMLSettingsStructure == nullptr)return;

	HANDLE hThread;
	DWORD threadID;

	hThread = CreateThread(nullptr, 0, ApplicationData::saveProc, reinterpret_cast<LPVOID>(this), 0, &threadID);
	if (hThread)
	{
		WaitForSingleObject(hThread, 20);
		CloseHandle(hThread);
	}
}

//void ApplicationData::startSaveAsync(DATAKEY key, int iData, bool bData, iString *sData, DWORD type)
//{
//	HANDLE hThread;
//	DWORD threadID;
//
//	auto std = new SAVERTHREADDATA;
//	std->bData = bData;
//	std->iData = iData;
//	std->key = key;
//	if (sData != nullptr)
//		std->sData = *sData;
//	else
//		std->sData.Replace(L"");
//	std->type = type;
//	std->_this = reinterpret_cast<cObject>(this);
//
//	hThread = CreateThread(nullptr, 0, ApplicationData::preSaveProc, reinterpret_cast<LPVOID>(std), 0, &threadID);
//	if (hThread)
//	{
//		WaitForSingleObject(hThread, 20);
//		CloseHandle(hThread);
//	}
//}

void ApplicationData::toFile()
{
	auto builder = new XML_Builder();
	builder->setUp_DocumentDefinition(L"1.0", L"utf-8", L"cncsettings", true);

	if (this->XMLSettingsStructure != nullptr)
	{
		auto tagCount = this->XMLSettingsStructure->GetCount();
		if (tagCount > 0)
		{
			for (int i = 0; i < tagCount; i++)
			{
				auto tag = this->XMLSettingsStructure->GetAt(i);
				builder->AddTag(&tag);
			}
			builder->Build();
			builder->toFileAndRelease(this->applicationDataContainerPath, true);
		}
	}
}

bool ApplicationData::validateStructure()
{
	if (this->XMLSettingsStructure == nullptr)
	{
		this->XMLSettingsStructure = new itemCollection<iXML_Tag>();
		return (this->XMLSettingsStructure != nullptr);
	}
	else
		return true;
}

int ApplicationData::validateKey(DATAKEY key)
{
	int num = this->XMLSettingsStructure->GetCount();

	for (int i = 0; i < num; i++)
	{
		if (this->XMLSettingsStructure->GetAt(i)
			.getPropertyContentFromName(L"key")
			.Equals(key))
		{
			return i;
		}
	}
	return -1;
}

void ApplicationData::waitForThreadsToFinish()
{
	while (this->saverThreadActive)
	{
		Sleep(5);
	}
}
