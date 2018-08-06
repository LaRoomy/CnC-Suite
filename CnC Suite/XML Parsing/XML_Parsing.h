#pragma once
#include"..//external.h"
#include"..//CommonControls/StringClass.h"
#include"..//CommonControls/ItemCollection.h"

#define		XML_ENTRY				L"<?xml\0"
#define		XML_OUT					L"?>\0"
#define		XML_VERSION_ENTRY		L"version=\"\0"
#define		XML_ENCODING_ENTRY		L"encoding=\"\0"
#define		XML_STANDALONE_ENTRY	L"standalone=\"\0"
#define		XML_DOCTYPE_ENTRY		L"!DOCTYPE\0"

#define		XERR_SYNTAX_UNEXPECTEDCLOSINGTAG		-20
#define		XERR_DOCUMENTFORMAT_INVALID				-21
#define		XERR_NOCLOSINGTAGFOUND					-22

typedef struct _XMLDOCUMENTINFO {

	iString xml_version;
	iString xml_encoding;
	iString doctype;
	bool standalone;

}XMLDOCUMENTINFO, *LPXMLDOCUMENTINFO;

typedef struct _PARSINGERROR{
	int errorcode;
	int charindex;
	iString additionalInfo;
}PARSINGERROR, *LPPARSINGERROR;

typedef struct _XMLCONTENTFORMATRULES {

	bool allowLinefeedInContent;
	bool allowCarriageReturnInContent;

}XMLCONTENTFORMATRULES, *LPXMLCONTENTFORMATRULES;

class XML_TAG_Property
	: public iCollectable<XML_TAG_Property>
{
public:
	XML_TAG_Property(){}
	~XML_TAG_Property() {}
	XML_TAG_Property(iString name, iString content)
	{
		this->propertyName = name;
		this->propertyContent = content;
	}
	XML_TAG_Property(LPTSTR name, LPTSTR content)
	{
		this->propertyName.Replace(name);
		this->propertyContent.Replace(content);
	}
	XML_TAG_Property(const XML_TAG_Property& xtp)
	{
		this->propertyName.Replace(xtp.propertyName.GetData());
		this->propertyContent.Replace(xtp.propertyContent.GetData());
	}
	void set(const iString& name, const iString& content)
	{
		this->propertyName = name;
		this->propertyContent = content;
	}
	void set(LPTSTR name, LPTSTR content)
	{
		this->propertyName.Replace(name);
		this->propertyContent.Replace(content);
	}
	void Release() { delete this; }
	XML_TAG_Property* XML_TAG_Property::getInstance() { return this; }
	iString propertyName;
	iString propertyContent;

	bool isName(const iString& name)
	{
		return this->propertyName.Equals(name);
	}

	bool isName(LPTSTR name)
	{
		iString iName(name);
		return this->isName(iName);
	}

	void Clear()
	{
		this->propertyName.Clear();
		this->propertyContent.Clear();
	}
	XML_TAG_Property& operator= (const XML_TAG_Property& xtp)
	{
		this->propertyName = xtp.propertyName;
		this->propertyContent = xtp.propertyContent;
		return *this;
	}
};

class iXML_Tag
	: public iCollectable<iXML_Tag>
{
public:
	iXML_Tag() : hasProperties(false) {}
	iXML_Tag(const iXML_Tag& xmlTag)
	{
		this->tagName.Replace(xmlTag.tagName.GetData());
		this->tagContent.Replace(xmlTag.tagContent.GetData());
		this->tagProperties = xmlTag.tagProperties;
		this->subTAG_Structure = xmlTag.subTAG_Structure;
		this->hasProperties = xmlTag.hasProperties;
		this->initallyClosed = xmlTag.initallyClosed;
	}
	~iXML_Tag() {}

	iXML_Tag* iXML_Tag::getInstance() { return this; }

	void Release() { delete this; }
	void Clear()
	{
		this->tagName.Clear();
		this->tagContent.Clear();
		this->subTAG_Structure.Clear();
		this->tagProperties.Clear();
		this->hasProperties = false;
		this->initallyClosed = false;
	}

	XML_TAG_Property* getPropertyFromName(iString& name)
	{
		int cnt = this->tagProperties.GetCount();
		if (cnt > 0)
		{
			for (int i = 0; i < cnt; i++)
			{
				if (this->tagProperties.GetAt(i).isName(name))
				{
					return this->tagProperties.GetAt(i).getInstance();
				}
			}
		}
		return nullptr;
	}

	bool hasSpecificProperty(iString& name)
	{
		auto p = this->getPropertyFromName(name);
		return (p != nullptr) ? true : false;
	}

	bool hasSpecificProperty(LPCTSTR name)
	{
		auto p = this->getPropertyFromName(name);
		return (p != nullptr) ? true : false;
	}

	XML_TAG_Property* getPropertyFromName(LPCTSTR name)
	{
		iString iName(name);
		return this->getPropertyFromName(iName);
	}

	iString getPropertyContentFromName(iString name)
	{
		int cnt = this->tagProperties.GetCount();
		if (cnt > 0)
		{
			for (int i = 0; i < cnt; i++)
			{
				if (this->tagProperties.GetAt(i).isName(name))
				{
					return this->tagProperties.GetAt(i).propertyContent;
				}
			}
		}
		return iString(L"");
	}

	iString getPropertyContentFromName(LPCTSTR name)
	{
		iString iName(name);
		return this->getPropertyContentFromName(iName);
	}

	iString tagName;
	iString tagContent;
	itemCollection<XML_TAG_Property> tagProperties;
	itemCollection<iXML_Tag> subTAG_Structure;
	bool initallyClosed;
	bool hasProperties;

	iXML_Tag& operator= (const iXML_Tag& xmlTag)
	{
		this->tagName = xmlTag.tagName;
		this->tagContent = xmlTag.tagContent;
		this->subTAG_Structure = xmlTag.subTAG_Structure;
		this->tagProperties = xmlTag.tagProperties;
		this->hasProperties = xmlTag.hasProperties;
		this->initallyClosed = xmlTag.initallyClosed;
		return *this;
	}
};

class XML_Builder
{
public:
	XML_Builder();

	void Build();
	void finalizeAsync(iString Path, bool overrideExisting);
	void toFile(iString Path, bool overrideExisting);
	void toFileAndRelease(iString &Path, bool _overrideExisting)
	{
		this->toFile(Path, _overrideExisting);
		this->Release();
	}
	void Release() { delete this; }
	
	void setUp_DocumentDefinition(iString xml_version, iString xml_encoding, iString docType, bool standalone);
	void setUp_DocumentDefinition(LPTSTR xml_version, LPTSTR xml_encoding, LPTSTR docType, bool standalone);

	// if (hTC == nullptr) -> the Tag will be added at first level
	void AddTag(iXML_Tag* hTag);

	iString getDocumentText() { return this->documentText; }
	int getTAGCount() { return this->docStructure.GetCount(); }

	static bool formatForXMLConformity(iString toFormat, iString& formatted);

private:
	XMLDOCUMENTINFO documentInfo;
	iString documentText;
	iString path;
	itemCollection<iXML_Tag> docStructure;

	bool overrideExisting;

	static DWORD WINAPI asycProc(LPVOID);

	iString _build_sub_(itemCollection<iXML_Tag>, int);
	iString getSpaceFromSubLevel(int);
};

class XMLParsingEventSink
{
public:
	virtual void ParsingCompleted(cObject sender, itemCollection<iXML_Tag>* documentStructure) = 0;
	virtual void ParsingFailed(cObject sender, LPPARSINGERROR ppErr) = 0;
};

class XML_Parser
{
public:
	XML_Parser();
	XML_Parser(XMLParsingEventSink* eventSink);
	//XML_Parser(const XML_Parser&);
	~XML_Parser();

	void Release() { delete this; }
	void Reset();

	void setEventListener(XMLParsingEventSink* eventSink) { this->eventListener = eventSink; }
	void setContentFormatRules(LPXMLCONTENTFORMATRULES cfRules);

	bool Parse();
	bool ParseAsync();

	DWORD getThreadIdIfExists();

	void setContent(LPTSTR docText);
	void setContent(iString docText);
	void setSublevelScan(bool subScan);

	bool OpenDocument(LPTSTR path);
	bool OpenDocument(iString path);

	bool isParsingSucceeded() { return this->SUCCESS; }

	itemCollection<iXML_Tag> getDocumentStructure();
	iString getDoctype() { return this->documentInfo.doctype; }

	int getTotalTAGNumber();

	bool searchForTAG(iString name, iXML_Tag* out, itemCollection<iXML_Tag> *documentStructure);

	static bool reformatXMLConformTextToPlainText(iString original, iString& reformatted);

private:
	XMLDOCUMENTINFO documentInfo;
	XMLCONTENTFORMATRULES cfRules;
	XMLParsingEventSink* eventListener;

	iString documentPath;
	iString documentText;

	itemCollection<iXML_Tag> documentStructure;

	bool SUCCESS;
	bool scanSubLevels;

	DWORD threadId;

	PARSINGERROR parsingErr;

	static DWORD WINAPI parseProc(LPVOID);

	void startParsing();
	void createParsingThread();

	int getDocumentInfo();

	bool isError(int, int);
	bool scanTags(const iString, itemCollection<iXML_Tag>&);
	bool scanTag(const TCHAR*, int&, int, itemCollection<iXML_Tag>&);
	bool scanProperties(iString*, itemCollection<XML_TAG_Property>&);
	bool getTagContent(const TCHAR*, iString*, int, LPCHARSCOPE, iString*);
	bool get_TAG_Range(iString*, int, LPCHARSCOPE, bool*);
	bool get_Single_Word(iString, LPCHARSCOPE, iString*);
	bool getPropertyContent(iString*, int, iString*);
	bool getPropertyName(iString*, int, iString*);
	bool getDoctypeContent(iString*);
	bool isChar(TCHAR);
	bool verifyTagContent(iString*);

	iString formatContent(iString*);

	int getTotalTAGsub(iXML_Tag* tag);
	int getClosingTagStart(const TCHAR*, int, int);

	void finalComplete();
	void finalFailed();
};
