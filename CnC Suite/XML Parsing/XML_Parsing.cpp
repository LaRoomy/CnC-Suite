#include<new>
#include "XML_Parsing.h"
#include "..//BasicFPO.h"
#include"..//HelperF.h"
#include"..//Error dispatcher.h"

XML_Parser::XML_Parser() :SUCCESS(true), scanSubLevels(false), eventListener(nullptr), threadId(0)
{
	SecureZeroMemory(&this->documentInfo, sizeof(XMLDOCUMENTINFO));
	SecureZeroMemory(&this->parsingErr, sizeof(PARSINGERROR));
	SecureZeroMemory(&this->cfRules, sizeof(XMLCONTENTFORMATRULES));
}

XML_Parser::XML_Parser(XMLParsingEventSink * eventSink) : SUCCESS(true), scanSubLevels(false), threadId(0)
{
	SecureZeroMemory(&this->documentInfo, sizeof(XMLDOCUMENTINFO));
	SecureZeroMemory(&this->parsingErr, sizeof(PARSINGERROR));
	SecureZeroMemory(&this->cfRules, sizeof(XMLCONTENTFORMATRULES));

	this->eventListener = eventSink;
}

//XML_Parser::XML_Parser(const XML_Parser & toCopy) :SUCCESS(true), scanSubLevels(false), eventListener(nullptr)
//{
//	SecureZeroMemory(&this->documentInfo, sizeof(XMLDOCUMENTINFO));
//	SecureZeroMemory(&this->parsingErr, sizeof(PARSINGERROR));
//
//	this->documentInfo.doctype.Replace(toCopy.documentInfo.doctype);
//	this->documentInfo.xml_encoding.Replace(toCopy.documentInfo.xml_encoding);
//	this->documentInfo.xml_version.Replace(toCopy.documentInfo.xml_version);
//	this->documentInfo.standalone = toCopy.documentInfo.standalone;
//
//	itemCollection<iXML_Tag> xmlTag = toCopy.documentStructure;
//	this->documentStructure = xmlTag;
//}

XML_Parser::~XML_Parser(){}

void XML_Parser::Reset()
{
	SecureZeroMemory(&this->documentInfo, sizeof(XMLDOCUMENTINFO));
	SecureZeroMemory(&this->parsingErr, sizeof(PARSINGERROR));
	SecureZeroMemory(&this->cfRules, sizeof(XMLCONTENTFORMATRULES));

	this->documentPath.Clear();
	this->documentText.Clear();
	this->documentStructure.Clear();

	if (this->threadId != 0)
	{
		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, this->threadId);
		if (hThread != nullptr)
		{
			//TerminateThread(hThread, 66);

		
			CloseHandle(hThread);
		}
		this->threadId = 0;
	}
}

void XML_Parser::setContentFormatRules(LPXMLCONTENTFORMATRULES _cfRules)
{
	this->cfRules.allowCarriageReturnInContent = _cfRules->allowCarriageReturnInContent;
	this->cfRules.allowLinefeedInContent = _cfRules->allowLinefeedInContent;
}

bool XML_Parser::Parse()
{
	if (this->documentText.GetLength() == 0)return false;
	else
	{
		this->startParsing();
		return true;
	}
}

bool XML_Parser::ParseAsync()
{
	if (this->documentText.GetLength() == 0)return false;
	else
	{
		this->createParsingThread();
		return true;
	}
}

DWORD XML_Parser::getThreadIdIfExists()
{
	if (this->threadId != 0)
	{
		return this->threadId;
	}
	return 0;
}

void XML_Parser::setContent(LPTSTR docText)
{
	this->documentText.Replace(docText);
}

void XML_Parser::setContent(iString docText)
{
	this->documentText = docText;
}

void XML_Parser::setSublevelScan(bool subScan)
{
	this->scanSubLevels = subScan;
}

bool XML_Parser::OpenDocument(LPTSTR path)
{
	iString _path(path);
	return this->OpenDocument(_path);
}

bool XML_Parser::OpenDocument(iString path)
{
	bool res;

	auto bfpo = CreateBasicFPO();

	res = (bfpo != nullptr) ? true : false;
	if (res)
	{
		TCHAR* buffer = nullptr;

		res =
			(
				bfpo->LoadBufferFmFileAsUtf8(
				&buffer,
				path.getContentReference()
				)
				== TRUE
			)
			? true : false;
		if (res)
		{
			this->documentPath.Replace(path);
			this->documentText.Replace(buffer);
		}
		SafeDeleteArray(&buffer);
		SafeRelease(&bfpo);
	}
	return res;
}

itemCollection<iXML_Tag> XML_Parser::getDocumentStructure()
{
	return this->documentStructure;
}

int XML_Parser::getTotalTAGNumber()
{
	int sumTags = 0;

	int firstLevelTags = this->documentStructure.GetCount();
	if (firstLevelTags > 0)
	{
		sumTags += firstLevelTags;

		for (int i = 0; i < firstLevelTags; i++)
		{
			sumTags += this->getTotalTAGsub(this->documentStructure.GetAt(i).getInstance());
		}
	}
	return sumTags;
}

bool XML_Parser::searchForTAG(iString name, iXML_Tag * out, itemCollection<iXML_Tag> *docStructure)
{
	if (docStructure == nullptr)
		docStructure = &this->documentStructure;

	if (docStructure != nullptr)
	{
		int i = docStructure->GetCount();
		if (i > 0)
		{
			for (int j = 0; j < i; j++)
			{
				if (docStructure->GetAt(j).tagName.Equals(name))
				{
					auto tag = docStructure->GetAt(j);
					*out = tag;

					return true;
				}
			}
		}
	}
	return false;
}

bool XML_Parser::reformatXMLConformTextToPlainText(iString original, iString & reformatted)
{
	int max = original.GetLength();
	if (max > 0)
	{
		auto content = original.GetData();
		if (content != nullptr)
		{
			TCHAR* buffer = new TCHAR[max + 1];
			if (buffer != nullptr)
			{
				int i = 0;
				int set = 0;
				SecureZeroMemory(buffer, sizeof(TCHAR)*(max + 1));

				while (i < max)
				{
					if (content[i] == L'&')// escape entry detected
					{
						bool escapeResolved = false;

						if ((i + 4) <= max)
						{
							if ((content[i + 1] == L'l') &&
								(content[i + 2] == L't') &&
								(content[i + 3] == L';'))
							{
								buffer[set] = L'<';
								i += 4;
								set++;
								escapeResolved = true;
							}
							else if ((content[i + 1] == L'g') &&
								(content[i + 2] == L't') &&
								(content[i + 3] == L';'))
							{
								buffer[set] = L'>';
								i += 4;
								set++;
								escapeResolved = true;
							}
						}
						if ((i + 5) <= max)
						{
							if ((content[i + 1] == L'a') &&
								(content[i + 2] == L'm') &&
								(content[i + 3] == L'p') &&
								(content[i + 4] == L';'))
							{
								buffer[set] = L'&';
								i += 5;
								set++;
								escapeResolved = true;
							}
						}
						if ((i + 6) <= max)
						{
							if ((content[i + 1] == L'q') &&
								(content[i + 2] == L'u') &&
								(content[i + 3] == L'o') &&
								(content[i + 4] == L't') &&
								(content[i + 5] == L';'))
							{
								buffer[set] = L'\"';
								i += 6;
								set++;
								escapeResolved = true;
							}
							else if ((content[i + 1] == L'a') &&
								(content[i + 2] == L'p') &&
								(content[i + 3] == L'o') &&
								(content[i + 4] == L's') &&
								(content[i + 5] == L';'))
							{
								buffer[set] = L'\'';
								i += 6;
								set++;
								escapeResolved = true;
							}
						}
						if (!escapeResolved)
						{
							buffer[set] = content[i];
							set++;
							i++;
						}
					}
					else
					{
						buffer[set] = content[i];
						set++;
						i++;
					}
				}
				reformatted.Replace(buffer);
				SafeDeleteArray(&buffer);
			}
		}
	}
	return false;
}

DWORD XML_Parser::parseProc(LPVOID lParam)
{
	XML_Parser* xmlP = reinterpret_cast<XML_Parser*>(lParam);
	if (xmlP != nullptr)
	{
		xmlP->startParsing();
		return 0;
	}
	else
		return 1;
}

void XML_Parser::startParsing()
{
	if (this->documentText.GetLength() > 0)
	{
		int index = this->getDocumentInfo();

		if (this->isError(index, 1))return;
		else
		{
			iString doctypeContent;

			if (this->getDoctypeContent(&doctypeContent))
			{
				if (this->scanTags(doctypeContent, this->documentStructure))
				{
					if (this->eventListener != nullptr)
					{
						if (this->SUCCESS)
						{
							this->finalComplete();
						}
						else
						{
							this->finalFailed();
						}
						this->threadId = 0;
					}
				}
			}
		}
	}
	else
	{
		this->SUCCESS = false;
	}
}

void XML_Parser::createParsingThread()
{
	HANDLE hThread;

	hThread = CreateThread(nullptr, 0, XML_Parser::parseProc, reinterpret_cast<LPVOID>(this), 0, &this->threadId);
	if (hThread)
	{
		WaitForSingleObject(hThread, 10);
		CloseHandle(hThread);
	}
}

int XML_Parser::getDocumentInfo()
{
	int start, end;
	CHARSCOPE cs;
	iString xml_entry(XML_ENTRY);
	iString xml_out(XML_OUT);

	if (this->documentText.Contains(xml_entry, &cs, 0, true))
	{
		start = cs.endChar;

		if (this->documentText.Contains(xml_out, &cs, start, true))
		{
			end = cs.startChar;

			CHARSCOPE cs_segment;
			cs_segment.startChar = start + 1;
			cs_segment.endChar = end - 1;

			iString xml_segment = this->documentText.GetSegment(&cs_segment);

			CHARSCOPE propScope;
			iString xml_prop_name(XML_VERSION_ENTRY);

			if (xml_segment.Contains(xml_prop_name, &propScope, 0, false))
			{
				if (this->getPropertyContent(&xml_segment, propScope.endChar, &this->documentInfo.xml_version))
				{
					if (this->documentInfo.xml_version.Equals(L"1.0\0"))
					{
						xml_prop_name.Replace(XML_ENCODING_ENTRY);

						if (xml_segment.Contains(xml_prop_name, &propScope, 0, false))
						{
							if (this->getPropertyContent(&xml_segment, propScope.endChar, &this->documentInfo.xml_encoding))
							{
								if (this->documentInfo.xml_encoding.Equals(L"utf-8\0"))
								{
									xml_prop_name.Replace(XML_STANDALONE_ENTRY);

									if (xml_segment.Contains(xml_prop_name, &propScope, 0, false))
									{
										iString standalone;

										if (this->getPropertyContent(&xml_segment, propScope.endChar, &standalone))
										{
											this->documentInfo.standalone = standalone.Equals(L"yes") ? true : false;

											if (this->documentInfo.standalone)
											{
												xml_prop_name.Replace(XML_DOCTYPE_ENTRY);

												if (this->documentText.Contains(xml_prop_name, &propScope, 0, true))
												{
													CHARSCOPE tagRange;
													if (this->get_TAG_Range(&this->documentText, propScope.endChar, &tagRange, nullptr))
													{
														tagRange.startChar = propScope.endChar + 1;

														if (this->get_Single_Word(this->documentText, &tagRange, &this->documentInfo.doctype))
														{
															return (tagRange.endChar + 1);
														}
														else
															return -14;
													}
													else
														return -13;
												}
												else
													return -12;
											}
											else
												return -11;
										}
										else
											return -10;
									}
									else
										return -9;
								}
								else
									return -8;
							}
							else
								return -7;
						}
						else
							return -6;
					}
					else
						return -5;
				}
				else
					return -4;
			}
			else
				return -3;
		}
		else
			return -2;
	}
	else
		return -1;
}

bool XML_Parser::isError(int code, int charIndex)
{
	if (code < 0)
	{
		if (code > -20)code = XERR_DOCUMENTFORMAT_INVALID;

		this->SUCCESS = false;
		this->parsingErr.errorcode = code;
		this->parsingErr.charindex = charIndex;
		return true;
	}
	else
		return false;
}

bool XML_Parser::scanTags(iString docContent, itemCollection<iXML_Tag>& TagStructure)
{
	int index = 0;
	int len = docContent.GetLength();

	auto content = docContent.GetData();
	if ((content != nullptr) && (len > 0))
	{
		while (this->scanTag(content, index, len, TagStructure));

		return true;
	}
	else
		return false;
}

bool XML_Parser::scanTag(const TCHAR* content, int& index, int max, itemCollection<iXML_Tag>& TagStructure)
{
	try
	{
		iXML_Tag xmlT;

		while (content[index] != L'<')
		{
			index++;
			if (index >= max)
				return false;
		}
		int j = 0;
		xmlT.hasProperties = true;
		TCHAR tagBuffer[1024] = { 0 };
		index++;
		while (content[index] != L' ')
		{
			if ((content[index] == L'>') || (content[index] == L'/'))
			{
				xmlT.hasProperties = false;
				break;
			}
			tagBuffer[j] = content[index];
			j++;
			index++;
			if (index >= max)return false;
			if (j == 1023)return false;
		}
		if (j == 0)
		{
			// SYNTAX ERROR
			this->SUCCESS = false;
			this->parsingErr.errorcode = XERR_SYNTAX_UNEXPECTEDCLOSINGTAG;
			this->parsingErr.charindex = index;

			return false;
		}

		tagBuffer[j] = L'\0';
		xmlT.tagName.Replace(tagBuffer);

		bool tagClosed;
		CHARSCOPE cs;
		iString ct(content);

		if (this->get_TAG_Range(&ct, index, &cs, &tagClosed))
		{
			iString tagSegment = ct.GetSegment(&cs);
			index = cs.endChar + 1;
			xmlT.initallyClosed = tagClosed;

			if (xmlT.hasProperties)
			{
				if (!this->scanProperties(&tagSegment, xmlT.tagProperties))
				{
					xmlT.hasProperties = false;
				}
			}
			if (!tagClosed)
			{
				if (this->getTagContent(content, &xmlT.tagName, index, &cs, &xmlT.tagContent))
				{
					index = cs.endChar + xmlT.tagName.GetLength() + 3;// + closing tag

					if (this->scanSubLevels)
					{
						if (this->scanTags(xmlT.tagContent, xmlT.subTAG_Structure))
						{
							// ...
						}
					}
				}
				else
				{
					// SYNTAX ERROR -> no closing tag found
					this->SUCCESS = false;
					this->parsingErr.charindex = index;
					this->parsingErr.errorcode = XERR_NOCLOSINGTAGFOUND;
					this->parsingErr.additionalInfo.Replace(tagBuffer);

					return false;
				}
			}
		}
		TagStructure.AddItem(xmlT);

		if (index >= max)return false;
		else return true;
	}
	catch(...)
	{
		return false;
	}
}

bool XML_Parser::scanProperties(iString *tagContent, itemCollection<XML_TAG_Property>& xtp)
{
	iString toFind(L"=\"\0");
	int position = 0;
	CHARSCOPE cs;
	XML_TAG_Property toAdd;
	int propertyCount = 0;
	
	while (tagContent->Contains(toFind, &cs, position, true))
	{
		if (this->getPropertyName(tagContent, cs.startChar - 1, &toAdd.propertyName))
		{
			if (this->getPropertyContent(tagContent, cs.endChar, &toAdd.propertyContent))
			{
				// ...
			}
		}
		xtp.AddItem(toAdd);
		toAdd.Clear();
		position = cs.endChar + 1;
		propertyCount++;
	}
	if (propertyCount == 0)return false;
	else return true;
}

bool XML_Parser::getTagContent(const TCHAR* content, iString *tagName, int from, LPCHARSCOPE cs, iString *out)
{
	iString pre(L"</\0");
	iString post(L">\0");
	iString _content_(content);
	iString closingTag = pre + *tagName + post;
	int max = _content_.GetLength();
	int i = from;
	bool startSet = true;
	//CHARSCOPE closingT;


	while (i > 0)
	{
		// step down to open-tag end
		if (content[i] == L'>')
		{
			if (startSet)
			{
				cs->startChar = i + 1;
				startSet = false;
			}
			break;
		}
		i--;
	}
	// get closing tag
	i = this->getClosingTagStart(content, i + 1, max);
	cs->endChar = i - 1;

	if (i == max)return false;
	else
	{
		int j = 0;
		TCHAR closingBuffer[1024] = { 0 };

		while (i < max)
		{
			closingBuffer[j] = content[i];
			if (closingBuffer[j] == L'>')
			{
				j++;
				break;
			}
			i++;
			j++;
		}
		closingBuffer[j] = L'\0';

		if (closingTag.Equals(closingBuffer))
		{
			iString seg = _content_.GetSegment(cs);
			iString formStr = this->formatContent(&seg);

			iString endFormat;
			XML_Parser::reformatXMLConformTextToPlainText(formStr, endFormat);

			out->Replace(endFormat);

			return true;
		}
		else
		{
			return false;
		}
	}
}

bool XML_Parser::get_TAG_Range(iString *doc, int from, LPCHARSCOPE cs, bool* closingTAG)
{
	int max = doc->GetLength();

	if (from < 0)return false;
	else if (cs == nullptr)return false;
	else if (max == 0)return false;
	else if (from > max)return false;
	else
	{
		int i = from;
		auto _doc_ = doc->GetData();

		if (_doc_ != nullptr)
		{
			while (i >= 0)
			{
				if (_doc_[i] == L'<')
				{
					cs->startChar = i;
					break;
				}
				i--;
			}
			i = from;

			while (i < max)
			{
				if (_doc_[i] == L'\0')return false;
				if (_doc_[i] == L'>')
				{
					cs->endChar = i;
					break;
				}
				i++;
			}
			if (closingTAG != nullptr)
			{
				if (_doc_[cs->endChar - 1] == L'/')*closingTAG = true;
				else *closingTAG = false;
			}
			return true;
		}
		else
			return false;
	}
}

bool XML_Parser::get_Single_Word(iString doc, LPCHARSCOPE cs, iString* out)
{
	if ((cs == nullptr) || (out == nullptr))return false;
	
	int doc_len = doc.GetLength();
	if (doc_len == 0)return false;
	else
	{
		if ((cs->endChar < 0) || (cs->startChar < 0) || (cs->endChar > doc_len))return false;
		else
		{
			try
			{
				TCHAR* word = new TCHAR[
					(size_t)(sizeof(TCHAR)*((cs->endChar - cs->startChar) + 1))];

				if (word != nullptr)
				{
					bool result = true;

					auto doc_text = doc.GetData();
					if (doc_text != nullptr)
					{
						int i = cs->startChar;
						int j = 0;

						while (!this->isChar(doc_text[i]))
						{
							if (i > cs->endChar)
							{
								result = false;
								break;
							}
							i++;
						}
						if (result)
						{
							while ((doc_text[i] != L' ') && (doc_text[i] != L'>'))
							{
								if (i > cs->endChar)
								{
									result = false;
									break;
								}
								word[j] = doc_text[i];
								i++;
								j++;
							}
							if (result)
							{
								word[j] = L'\0';
								out->Replace(word);
							}
						}
					}
					else
						result = false;

					SafeDeleteArray(&word);

					return result;

				}
				else
					return false;
			}
			catch (...)
			{
				return false;
			}
		}
	}
}

bool XML_Parser::getPropertyContent(iString *segment, int start, iString* out)
{
	auto docText = segment->GetData();
	if (docText != nullptr)
	{
		try
		{
			int i = start;

			if (docText[i] == L'\"')
			{
				int j = 0;
				i++;

				while (docText[i] != L'\"')
				{
					if (docText[i] == L'\0')
					{
						j = -1;
						break;
					}
					j++;
					i++;
				}
				if (j != -1)
				{
					TCHAR* _property_ = new TCHAR[(size_t)(sizeof(TCHAR)*(j + 1))];
					if (_property_ != nullptr)
					{
						i = start + 1;
						j = 0;

						while (docText[i] != L'\"')
						{
							_property_[j] = docText[i];
							i++;
							j++;
						}
						_property_[j] = L'\0';

						out->Replace(_property_);
						SafeDeleteArray(&_property_);

						return true;
					}
				}
			}
		}
		catch (...)
		{
			return false;
		}
	}
	return false;
}

bool XML_Parser::getPropertyName(iString *container, int from, iString *out)
{
	int max = container->GetLength();

	if (from < 0)return false;
	else if (out == nullptr)return false;
	else if (max == 0)return false;
	else
	{
		int cnt = from;
		auto ct = container->GetData();
		TCHAR propBuffer[1024] = { 0 };

		while ((ct[cnt] != L' ') && (ct[cnt] != L'\n'))
		{
			cnt--;
			if (cnt < 0)return false;
			if ((ct[cnt] == L'<') || (ct[cnt] == L'>'))return false;
		}
		int i = 0;
		cnt++;

		while (cnt <= from)
		{
			propBuffer[i] = ct[cnt];
			cnt++;
			i++;
			if (cnt > max)return false;
			if (i == 1024)return false;
		}
		propBuffer[cnt] = L'\0';
		out->Replace(propBuffer);
		return true;
	}
}

bool XML_Parser::getDoctypeContent(iString* out)
{
	iString tagstart(L"<\0");
	tagstart.Append(&this->documentInfo.doctype);
	tagstart.Append(L">\0");
	iString tagclose(L"</\0");
	tagclose.Append(&this->documentInfo.doctype);
	tagclose.Append(L">\0");

	CHARSCOPE cs_start, cs_end;

	if (this->documentText.Contains(tagstart, &cs_start, 0, true))
	{
		if (this->documentText.Contains(tagclose, &cs_end, 0, true))
		{
			CHARSCOPE seg;
			seg.startChar = cs_start.endChar + 1;
			seg.endChar = cs_end.startChar - 1;

			iString segment = this->documentText.GetSegment(&seg);
			out->Replace(segment);

			return true;
		}
	}
	return false;
}

bool XML_Parser::isChar(TCHAR LT)
{
	bool result = false;

	if ((LT == 'A') ||
		(LT == 'B') ||
		(LT == 'C') ||
		(LT == 'D') ||
		(LT == 'E') ||
		(LT == 'F') ||
		(LT == 'G') ||
		(LT == 'H') ||
		(LT == 'I') ||
		(LT == 'J') ||
		(LT == 'K') ||
		(LT == 'L') ||
		(LT == 'M') ||
		(LT == 'N') ||
		(LT == 'O') ||
		(LT == 'P') ||
		(LT == 'Q') ||
		(LT == 'R') ||
		(LT == 'S') ||
		(LT == 'T') ||
		(LT == 'U') ||
		(LT == 'V') ||
		(LT == 'W') ||
		(LT == 'X') ||
		(LT == 'Y') ||
		(LT == 'Z') ||
		(LT == 'a') ||
		(LT == 'b') ||
		(LT == 'c') ||
		(LT == 'd') ||
		(LT == 'e') ||
		(LT == 'f') ||
		(LT == 'g') ||
		(LT == 'h') ||
		(LT == 'i') ||
		(LT == 'j') ||
		(LT == 'k') ||
		(LT == 'l') ||
		(LT == 'm') ||
		(LT == 'n') ||
		(LT == 'o') ||
		(LT == 'p') ||
		(LT == 'q') ||
		(LT == 'r') ||
		(LT == 's') ||
		(LT == 't') ||
		(LT == 'u') ||
		(LT == 'v') ||
		(LT == 'w') ||
		(LT == 'x') ||
		(LT == 'y') ||
		(LT == 'z'))
	{
		result = true;
	}
	return result;
}

bool XML_Parser::verifyTagContent(iString *cont)
{
	int len = cont->GetLength();
	auto buffer = cont->GetData();

	for (int i = 0; i < len; i++)
	{
		if (buffer[i] == L'\0')
		{
			return false;
		}
		if ((buffer[i] != L' ') && (buffer[i] != L'\n') && (buffer[i] != L'\t') && (buffer[i] != L'\r'))
		{
			return true;
		}
	}
	return false;
}

iString XML_Parser::formatContent(iString *cont)
{
	try
	{
		int i = 0;
		int max = cont->GetLength();

		CHARSCOPE cs;

		auto content = cont->GetData();

		if ((content != nullptr) && (max > 0))
		{
			while ((content[i] == L'\n') || (content[i] == L'\r') || (content[i] == L'\t') || (content[i] == L' '))
			{
				if (i == max)break;
				else i++;
			}
			cs.startChar = i;

			i = max;
			while ((content[i] == L'\n') || (content[i] == L'\r') || (content[i] == L'\t') || (content[i] == L' ') || (content[i] == L'\0'))
			{
				if (i == 0)
					break;

				if (this->cfRules.allowLinefeedInContent)
				{
					if (content[i] == L'\n')
					{
						i--;
						break;
					}
				}
				if (this->cfRules.allowCarriageReturnInContent)
				{
					if (content[i] == L'\r')
					{
						i--;
						break;
					}
				}
				i--;
			}
			cs.endChar = i;
			
			if (iString::verifyScope(&cs, max))
			{
				iString newCont = cont->GetSegment(&cs);

				if (this->verifyTagContent(&newCont))
					return newCont;
				else
					return iString(L"");
			}
			else
				return iString(L"");
		}
		else
			return iString(L"");
	}
	catch (...)
	{
		return iString(L"");
	}
}

int XML_Parser::getTotalTAGsub(iXML_Tag * tag)
{
	int sumTags = 0;

	if(tag != nullptr)
	{
		int tagCount = tag->subTAG_Structure.GetCount();

		if (tagCount > 0)
		{
			sumTags += tagCount;

			for (int i = 0; i < tagCount; i++)
			{			
				sumTags += this->getTotalTAGsub(tag->subTAG_Structure.GetAt(i).getInstance());
			}
		}
	}
	return sumTags;
}

int XML_Parser::getClosingTagStart(const TCHAR * content, int from, int max)
{
	int i = from;
	int openedTags = 1;

	while (i < max)
	{
		if ((content[i] == L'<') && (content[i + 1] == L'/'))
		{
			// closing tag found
			openedTags--;

			if (openedTags == 0)
			{
				// this is supposed to be the requested closing tag: break and leave function
				break;
			}
			else
			{
				while (i < max)
				{
					// forward to closing tag end
					if (content[i] == L'>')
						break;
					i++;
				}
			}
		}
		else if (content[i] == L'<')
		{
			// tag opening found	
			while (i < max)
			{
				// search for next close
				if ((content[i] == L'/') && (content[i + 1] == L'>'))
				{
					// single tag - doesn't matter
					i++;
					break;
				}
				else if (content[i] == L'>')
				{
					// new tag opened
					openedTags++;
					break;
				}

				i++;
			}
		}
		i++;
	}
	return i;
}

void XML_Parser::finalFailed()
{
	__try
	{
		if (this->eventListener != nullptr)
		{
			this->eventListener->ParsingFailed(reinterpret_cast<cObject>(this), &this->parsingErr);
		}
	}
	__except (GetExceptionCode()
		== EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return;
	}
}

void XML_Parser::finalComplete()
{
	__try
	{
		if (this->eventListener != nullptr)
		{
			this->eventListener->ParsingCompleted(reinterpret_cast<cObject>(this), &this->documentStructure);
		}
	}
	__except (GetExceptionCode()
		== EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return;
	}
}

XML_Builder::XML_Builder()
{
	SecureZeroMemory(&this->documentInfo, sizeof(XMLDOCUMENTINFO));
	this->documentInfo.xml_version.Replace(L"1.0\0");
	this->documentInfo.xml_encoding.Replace(L"utf-8\0");
	this->documentInfo.doctype.Replace(L"xml\0");
	this->documentInfo.standalone = true;
	this->overrideExisting = false;
}

void XML_Builder::Build()
{
	this->documentText.Replace(XML_ENTRY);
	this->documentText.Append(L" \0");
	this->documentText.Append(XML_VERSION_ENTRY);
	this->documentText.Append(&this->documentInfo.xml_version);
	this->documentText.Append(L"\" \0");
	this->documentText.Append(XML_ENCODING_ENTRY);
	this->documentText.Append(&this->documentInfo.xml_encoding);
	this->documentText.Append(L"\" \0");
	this->documentText.Append(XML_STANDALONE_ENTRY);
	if (this->documentInfo.standalone)
		this->documentText.Append(L"yes\"\0");
	else
		this->documentText.Append(L"no\"\0");
	this->documentText.Append(XML_OUT);
	this->documentText.Append(L"\r\n<\0");
	this->documentText.Append(XML_DOCTYPE_ENTRY);
	this->documentText.Append(L" \0");
	this->documentText.Append(&this->documentInfo.doctype);
	this->documentText.Append(L">\r\n\0");
	this->documentText.Append(L"<\0");
	this->documentText.Append(&this->documentInfo.doctype);
	this->documentText.Append(L">\r\n\0");
	
	for (int i = 0; i < this->docStructure.GetCount(); i++)
	{
		this->documentText.Append(L"  <\0");
		this->documentText.Append(this->docStructure.GetAt(i).tagName.GetData());
		if (this->docStructure.GetAt(i).hasProperties)
		{
			for (int j = 0; j < this->docStructure.GetAt(i).tagProperties.GetCount(); j++)
			{
				this->documentText.Append(L" \0");
				this->documentText.Append(this->docStructure.GetAt(i).tagProperties.GetAt(j).propertyName.GetData());
				this->documentText.Append(L"=\"\0");
				this->documentText.Append(this->docStructure.GetAt(i).tagProperties.GetAt(j).propertyContent.GetData());
				this->documentText.Append(L"\"\0");
			}
		}
		if (this->docStructure.GetAt(i).initallyClosed)
		{
			this->documentText.Append(L"/>\r\n\0");
		}
		else
		{
			this->documentText.Append(L">\r\n\0");

			if (this->docStructure.GetAt(i).subTAG_Structure.GetCount() > 0)
			{
				auto cont = this->_build_sub_(this->docStructure.GetAt(i).subTAG_Structure, 1);
				this->documentText.Append(cont.GetData());
			}
			else
			{
				this->documentText.Append(L"    \0");

				// format for xml conformity
				iString strXMLConform;
				XML_Builder::formatForXMLConformity(
					this->docStructure.GetAt(i)
					.tagContent,
					strXMLConform
				);
				this->documentText.Append(
					strXMLConform.GetData()
				);

				//this->documentText.Append(this->docStructure.GetAt(i).tagContent.GetData()); // old
			} 			
			this->documentText.Append(L"\r\n  </\0");
			this->documentText.Append(this->docStructure.GetAt(i).tagName.GetData());
			this->documentText.Append(L">\r\n\0");
		}
	}
	this->documentText.Append(L"</\0");
	this->documentText.Append(&this->documentInfo.doctype);
	this->documentText.Append(L">\0");
}

void XML_Builder::finalizeAsync(iString Path, bool OverrideExisting)
{
	this->overrideExisting = OverrideExisting;
	this->path = Path;

	DWORD threadId;
	HANDLE hThread;

	hThread = CreateThread(nullptr, 0, XML_Builder::asycProc, reinterpret_cast<LPVOID>(this), 0, &threadId);
	if (hThread != nullptr)
	{
		WaitForSingleObject(hThread, 50);
	}
}

void XML_Builder::toFile(iString Path, bool OverrideExisting)
{
	auto bfpo = CreateBasicFPO();
	if (bfpo != nullptr)
	{
		BOOL f_exist = bfpo->CheckForFileExist(
			Path.getContentReference()
		);
		if (f_exist)
		{
			if (OverrideExisting)
			{
				f_exist = FALSE;
			}
		}
		if (!f_exist)
		{
			try
			{
				DWORD cbData =
					WideCharToMultiByte(
						CP_UTF8,
						0,
						this->documentText.GetData(),
						-1,
						nullptr, 0, nullptr, nullptr
					);
				if (cbData != 0)
				{
					char* low_buffer = new (std::nothrow) CHAR[cbData];
					if (low_buffer != nullptr)
					{
						if (WideCharToMultiByte(
							CP_UTF8,
							0,
							this->documentText.GetData(),
							-1,
							low_buffer,
							cbData,
							nullptr, nullptr) > 0)
						{
							HANDLE hFile =
								CreateFile(
									Path.GetData(),
									GENERIC_WRITE,
									FILE_SHARE_WRITE,
									0,
									CREATE_ALWAYS,
									FILE_ATTRIBUTE_NORMAL,
									nullptr
								);
							if (hFile != INVALID_HANDLE_VALUE)
							{
								DWORD nBytesWritten;
								auto i = cbData - 1;

								// remove the terminating null character!
								while (i > 0)
								{
									if (low_buffer[i] == '\0')
										cbData--;
									else
										break;

									i--;
								}
								WriteFile(hFile, low_buffer, cbData, &nBytesWritten, NULL);

								CloseHandle(hFile);
							}
						}
						delete[] low_buffer;
					}
				}			
			}
			catch (...)
			{
				return;
			}
		}
		SafeRelease(&bfpo);
	}
}

void XML_Builder::setUp_DocumentDefinition(iString xml_version, iString xml_encoding, iString docType, bool standalone)
{
	this->documentInfo.xml_version.Replace(xml_version);	
	this->documentInfo.xml_encoding.Replace(xml_encoding);
	this->documentInfo.doctype.Replace(docType);	
	this->documentInfo.standalone = standalone;
}

void XML_Builder::setUp_DocumentDefinition(LPTSTR xml_version, LPTSTR xml_encoding, LPTSTR docType, bool standalone)
{
	this->documentInfo.xml_version.Replace(xml_version);
	this->documentInfo.xml_encoding.Replace(xml_encoding);
	this->documentInfo.doctype.Replace(docType);
	this->documentInfo.standalone = standalone;
}

void XML_Builder::AddTag(iXML_Tag * hTag)
{
	this->docStructure.AddItem(hTag);
}

bool XML_Builder::formatForXMLConformity(iString toFormat, iString& formatted)
{
	// first count the forbidden literals:
	int forbiddenLiteralCount = 0;
	int fieldsToAdd = 0;
	int max = toFormat.GetLength();
	auto content = toFormat.GetData();

	for (int i = 0; i < max; i++)
	{
		switch (content[i])
		{
		case L'<':
			forbiddenLiteralCount++;
			fieldsToAdd += 4;
			break;
		case L'>':
			forbiddenLiteralCount++;
			fieldsToAdd += 4;
			break;
		case L'&':
			forbiddenLiteralCount++;
			fieldsToAdd += 5;
			break;
		case L'\"':
			forbiddenLiteralCount++;
			fieldsToAdd += 6;
			break;
		case L'\'':
			forbiddenLiteralCount++;
			fieldsToAdd += 6;
			break;
		default:
			break;
		}
	}
	if (forbiddenLiteralCount == 0)
	{
		formatted = toFormat;
		return true;
	}

	int new_max = max + fieldsToAdd + 1;

	TCHAR* buffer = new TCHAR[new_max];
	if (buffer != nullptr)
	{
		// set the converted array
		int i = 0;
		int set = 0;
		SecureZeroMemory(buffer, sizeof(TCHAR)* new_max);

		while (i < max)
		{
			switch (content[i])
			{
			case L'<':
				if ((set + 4) > new_max)
					return false;
				buffer[set] = L'&';
				buffer[set + 1] = L'l';
				buffer[set + 2] = L't';
				buffer[set + 3] = L';';
				set += 4;
				break;
			case L'>':
				if ((set + 4) > new_max)
					return false;
				buffer[set] = L'&';
				buffer[set + 1] = L'g';
				buffer[set + 2] = L't';
				buffer[set + 3] = L';';
				set += 4;
				break;
			case L'&':
				if ((set + 5) > new_max)
					return false;
				buffer[set] = L'&';
				buffer[set + 1] = L'a';
				buffer[set + 2] = L'm';
				buffer[set + 3] = L'p';
				buffer[set + 4] = L';';
				set += 5;
				break;
			case L'\"':
				if ((set + 6) > new_max)
					return false;
				buffer[set] = L'&';
				buffer[set + 1] = L'q';
				buffer[set + 2] = L'u';
				buffer[set + 3] = L'o';
				buffer[set + 4] = L't';
				buffer[set + 5] = L';';
				set += 6;
				break;
			case L'\'':
				if ((set + 6) > new_max)
					return false;
				buffer[set] = L'&';
				buffer[set + 1] = L'a';
				buffer[set + 2] = L'p';
				buffer[set + 3] = L'o';
				buffer[set + 4] = L's';
				buffer[set + 5] = L';';
				set += 6;
				break;
			default:
				buffer[set] = content[i];
				set++;
				break;
			}
			if (set == new_max)
				break;

			i++;
		}
		formatted.Replace(buffer);
		SafeDeleteArray(&buffer);
		return true;
	}
	return false;
}

iString XML_Builder::getSpaceFromSubLevel(int sublevel)
{
	iString spaces(L"");
	iString space(L"  \0");

	for (int i = 0; i < sublevel; i++)spaces += space;

	return spaces;
}

DWORD XML_Builder::asycProc(LPVOID lParam)
{
	auto builder = reinterpret_cast<XML_Builder*>(lParam);
	if (builder != nullptr)
	{
		builder->Build();
		builder->toFile(
			builder->path,
			builder->overrideExisting);
		builder->Release();
	}
	return 0;
}

iString XML_Builder::_build_sub_(itemCollection<iXML_Tag> subStructure, int sublevel)
{
	sublevel++;
	iString subContent(L"");
	iString spaces = getSpaceFromSubLevel(sublevel);

	for (int i = 0; i < subStructure.GetCount(); i++)
	{
		subContent.Append(&spaces);
		subContent.Append(L"<\0");
		subContent.Append(subStructure.GetAt(i).tagName.GetData());
		if (subStructure.GetAt(i).hasProperties)
		{
			for (int j = 0; j < subStructure.GetAt(i).tagProperties.GetCount(); j++)
			{
				subContent.Append(L" \0");
				subContent.Append(subStructure.GetAt(i).tagProperties.GetAt(j).propertyName.GetData());
				subContent.Append(L"=\"\0");
				subContent.Append(subStructure.GetAt(i).tagProperties.GetAt(j).propertyContent.GetData());
				subContent.Append(L"\"\0");
			}
		}
		if (subStructure.GetAt(i).initallyClosed)
		{
			subContent.Append(L"/>\0");
			if ((i + 1) < subStructure.GetCount())
				subContent.Append(L"\r\n\0");
		}
		else
		{
			subContent.Append(L">\r\n\0");
			if (subStructure.GetAt(i).subTAG_Structure.GetCount() > 0)
			{
				auto cont = _build_sub_(subStructure.GetAt(i).subTAG_Structure, sublevel);
				subContent.Append(&cont);
			}
			else
			{
				subContent.Append(L"  \0");
				subContent += spaces;

				// format for xml conformity
				iString strXMLConform;
				XML_Builder::formatForXMLConformity(
					subStructure.GetAt(i)
					.tagContent,
					strXMLConform
				);
				subContent.Append(
					strXMLConform.GetData()
				);				
			}
			subContent.Append(L"\r\n\0");
			subContent.Append(&spaces);
			subContent.Append(L"</\0");
			subContent.Append(subStructure.GetAt(i).tagName.GetData());
			subContent.Append(L">\0");
			if ((i + 1) < subStructure.GetCount())
				subContent.Append(L"\r\n\0");
		}
	}
	return subContent;
}
