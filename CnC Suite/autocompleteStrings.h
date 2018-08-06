#pragma once
#include"external.h"
#include"XML Parsing\XML_Parsing.h"

#define			FLOADING_NOLOADREQUESTED	0
#define			FLOADING_LOADSUCCEDED		1
#define			FLOADING_PENDING			2
#define			FLOADING_FAILED				-1
#define			FLOADING_FILENOTFOUND		-2

#define			AC_ASYNCOPERATION_SAVE			10
#define			AC_ASYNCOPERATION_LOAD			11

#define			AC_EXECUTE_INLINE				20
#define			AC_EXECUTE_ASYNC				21


typedef struct _AUTOCOMPLETESTRINGS {

	TCHAR trigger[56];
	TCHAR appendix[56];
	int length;

}AUTOCOMPLETESTRINGS, *LPAUTOCOMPLETESTRINGS;

__interface autocompleteStringLoadingEventSink
{
	void acStringLoadingComplete(cObject sender);
};

class autocompleteStrings
	: public XMLParsingEventSink
{
public:
	autocompleteStrings()
		:count(0), pStrings(nullptr), fLoading(FLOADING_NOLOADREQUESTED), eventHandler(nullptr)
	{}
	autocompleteStrings(const autocompleteStrings& acs)
		:fLoading(FLOADING_NOLOADREQUESTED), eventHandler(nullptr)
	{
		this->pStrings = nullptr;
		this->count = acs.getContent(&this->pStrings);
	}
	~autocompleteStrings()
	{
		this->clear();
	}

	void add(const TCHAR* trigger, const TCHAR* appendix, int length);
	void deleteAt(int index);
	void updateAt(int index, const TCHAR* trigger, const TCHAR* appendix, int length);

	iString getTriggerAt(int index);
	iString getAppendixAt(int index);

	void setEventHandler(autocompleteStringLoadingEventSink* handler)
	{
		this->eventHandler = handler;
	}

	void autocompleteStrings::ParsingCompleted(cObject sender, itemCollection<iXML_Tag>* documentStructure)
	{
		auto parser = reinterpret_cast<XML_Parser*>(sender);
		if (parser != nullptr)
		{
			if (documentStructure != nullptr)
			{
				if (this->xmlToStruct(documentStructure))
				{
					this->fLoading = FLOADING_LOADSUCCEDED;
				}
				else
				{
					this->fLoading = FLOADING_FAILED;
				}
			}
			SafeRelease(&parser);
		}
		else
			this->fLoading = FLOADING_FAILED;

		if (this->eventHandler != nullptr)
		{
			this->eventHandler->acStringLoadingComplete(reinterpret_cast<cObject>(this));
		}
	}
	void autocompleteStrings::ParsingFailed(cObject sender, LPPARSINGERROR pErr) {
		UNREFERENCED_PARAMETER(sender);
		UNREFERENCED_PARAMETER(pErr);

		this->fLoading = FLOADING_FAILED;
	}

	int GetCount() { return this->count; }
	LPAUTOCOMPLETESTRINGS getReferenceToStruct() { return this->pStrings; }

	bool setContent(int len, LPAUTOCOMPLETESTRINGS data_in);

	// NOTE: negative return value indicates fail
	// NOTE: if 'data_out' is nullptr it will be allocated (do not forget to free it)
	int getContent(LPAUTOCOMPLETESTRINGS *ppData_out) const;

	// NOTE: if 'ppData_out' is nullptr it will be allocated (do not forget to free it)
	static bool Copy(int len, LPAUTOCOMPLETESTRINGS data_in, LPAUTOCOMPLETESTRINGS *ppData_out);

	void clear();

	void toFile();
	void fromFile(DWORD executeMode);

	bool isLoadingSucceeded() { return (this->fLoading == FLOADING_LOADSUCCEDED) ? true : false; }

private:
	int count;
	BOOL fLoading;
	LPAUTOCOMPLETESTRINGS pStrings;

	autocompleteStringLoadingEventSink* eventHandler;

	static DWORD WINAPI saveProc(LPVOID);
	static DWORD WINAPI loadProc(LPVOID);

	void startAsyncOperation(DWORD type);

	bool xmlToStruct(itemCollection<iXML_Tag>* xml_);
	bool structToFile();
};