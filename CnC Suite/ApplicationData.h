#pragma once
#include"external.h"
#include"BasicFPO.h"
#include"CommonControls\StringClass.h"
#include"XML Parsing\XML_Parsing.h"
#include"iBox.h"
#include"AppPath.h"

#define		DATATYPE_INTEGER		1
#define		DATATYPE_BOOLEAN		2
#define		DATATYPE_ISTRING		3

typedef const TCHAR* DATAKEY;
typedef const TCHAR* FILEKEY;

#define		_compareFileKey(k1,k2)		((BOOL)(CompareStringsB((TCHAR*)k1,(TCHAR*)k2)))

//#define		PATHSEGMENT_FOLDER_SETTINGS			L"\\CnC Suite\\AppData\\Settings"

typedef struct _SAVERTHREADDATA {

	DWORD type;
	bool bData;
	iString sData;
	int iData;
	DATAKEY key;
	cObject _this;

}SAVERTHREADDATA, *LPSAVERTHREADDATA;

class ApplicationData
	: public ObjectRelease<ApplicationData>,
	public XMLParsingEventSink,
	public iCollectable<ApplicationData>
{
public:
	ApplicationData();
	ApplicationData(const ApplicationData& aData)
		: //preThreadActive(0),
		saverThreadActive(0)
	{
		this->filekey = aData.filekey;
		this->success = aData.Succeeded();
		this->applicationDataContainerPath = aData.getApplicationDataContainerPath();
		this->XMLSettingsStructure = itemCollection<iXML_Tag>::Copy(aData.getReferenceToInternalStructure());
	}
	~ApplicationData();

	void Release() { delete this; }
	void Reset() {

		this->waitForThreadsToFinish();

		SafeRelease(&this->XMLSettingsStructure);
		this->filekey.Clear();
		//this->preThreadActive = 0;
		this->saverThreadActive = 0;
		this->success = false;
		this->applicationDataContainerPath.Clear();
	}

	bool selectFilekey(FILEKEY key);
	FILEKEY getFilekey() const { return (FILEKEY)this->filekey.GetData(); }

	// indication if the parsing of the file succeeded
	bool Succeeded() const { return this->success; }

	//void saveValueAsync(DATAKEY key, int value);
	//void saveValueAsync(DATAKEY key, bool value);
	//void saveValueAsync(DATAKEY key, iString value);
	//void saveValueAsync(DATAKEY key, LPCTSTR value);

	bool saveValue(DATAKEY key, int value);
	bool saveValue(DATAKEY key, unsigned int value);
	bool saveValue(DATAKEY key, const iString& string);
	bool saveValue(DATAKEY key, const TCHAR* string);
	bool saveValue(DATAKEY key, bool value);

	void deleteValue(DATAKEY key);

	// return-value is iBox<...>* type if succeeded
	// or nullptr if failed
	// don't forget to release the box!
	cObject lookUp(_In_ DATAKEY key);

	bool getBooleanData(DATAKEY key, bool defaultvalue);
	int getIntegerData(DATAKEY key, int defaultvalue);
	unsigned int getUnsignedIntegerData(DATAKEY key, unsigned int defaultvalue);
	iString getStringData(DATAKEY key, const iString& defaultvalue);
	iString getStringData(DATAKEY key, LPCTSTR defaultvalue);
	iString getStringData(DATAKEY key);
	
	// parsing events
	void ApplicationData::ParsingCompleted(cObject sender, itemCollection<iXML_Tag>* xmlSettingsStructure) {

		this->XMLSettingsStructure = itemCollection<iXML_Tag>::Copy(xmlSettingsStructure);

		this->success = true;
		auto parser = reinterpret_cast<XML_Parser*>(sender);
		SafeRelease(&parser);
	}
	void ApplicationData::ParsingFailed(cObject sender, LPPARSINGERROR error) {

		UNREFERENCED_PARAMETER(error);

		this->success = false;
		auto parser = reinterpret_cast<XML_Parser*>(sender);
		SafeRelease(&parser);
	}

	iString getApplicationDataContainerPath() const { return this->applicationDataContainerPath; }
	itemCollection<iXML_Tag>* getReferenceToInternalStructure() const { return this->XMLSettingsStructure; }

	ApplicationData& operator= (const ApplicationData& aData)
	{
		this->filekey = aData.filekey;
		//this->preThreadActive = 0;
		this->saverThreadActive = 0;
		this->success = aData.Succeeded();
		this->applicationDataContainerPath = aData.getApplicationDataContainerPath();
		this->XMLSettingsStructure = itemCollection<iXML_Tag>::Copy(aData.getReferenceToInternalStructure());
		return *this;
	}

	ApplicationData* getInstance() { return this; }

private:
	bool success;
	//int preThreadActive;
	int saverThreadActive;
	
	iString filekey;
	iString applicationDataContainerPath;

	itemCollection<iXML_Tag>* XMLSettingsStructure;

	static DWORD WINAPI saveProc(LPVOID);
	//static DWORD WINAPI preSaveProc(LPVOID);

	void startSaverThread();
	//void startSaveAsync(DATAKEY, int, bool, iString*, DWORD);
	void toFile();

	bool validateStructure();
	int validateKey(DATAKEY key);

	void waitForThreadsToFinish();
};

