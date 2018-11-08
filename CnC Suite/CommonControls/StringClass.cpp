#include <new.h>
#include "..\CommonControls\StringClass.h"

#include"../HelperF.h"

iString::iString()
	:referenceCounter(1)
{
	this->Initialize();
}

iString::iString(const iString & string)
	:referenceCounter(1)
{
	this->Initialize();
	TCHAR* data = string.content;
	this->Copy(data, &this->content);
}

iString::iString(const TCHAR * string)
	:referenceCounter(1)
{
	this->Initialize();
	this->Copy(string, &this->content);
}

iString::iString(const char * string)
	:referenceCounter(1)
{
	this->Initialize();
	this->CharToWide(string, &this->content);
}

iString::iString(int toString)
{
	this->Initialize();
	auto str = iString::FromInt(toString);
	this->Replace(*str);
	str->Release();
}

iString::iString(DWORD toString)
{
	this->Initialize();
	auto str = iString::FromUInt(
		(unsigned int)toString
	);
	this->Replace(*str);
	str->Release();
}

iString::iString(size_t contentsize)
{
	this->Initialize();
	this->content = new TCHAR[contentsize];
	SecureZeroMemory(this->content, contentsize);
}

iString::~iString()
{
	if (this->content != nullptr)
	{
		delete[] this->content;
	}
	if (this->content_as_char != nullptr)
	{
		delete[] this->content_as_char;
	}
}


void iString::Initialize()
{
	this->content = nullptr;
	this->content_as_char = nullptr;
}

void iString::ClearContent()
{
	if (this->content != NULL)
	{
		delete this->content;
		this->content = NULL;

		if (this->content_as_char != NULL)
		{
			delete this->content_as_char;
			this->content_as_char = NULL;
		}
	}
}

bool iString::Append(const iString* stringToAppend)
{
	return this->Append(stringToAppend->GetData());
}

bool iString::Append(const iString& stringToAppend)
{
	return this->Append(stringToAppend.GetData());
}

bool iString::Append(const TCHAR * stringToAppend)
{
	bool result = false;

	if (stringToAppend != NULL)
	{
		TCHAR* holder = NULL;

		if (this->Copy(this->content, &holder))
		{
			this->ClearContent();

			if (this->Append(holder, stringToAppend, &this->content))
			{
				result = true;
			}
			else
			{
				this->ClearContent();
				this->Copy(holder, &this->content);
			}
			delete[] holder;
		}
		return result;
	}
	else
		return result;
}

bool iString::Append(const TCHAR * first, const TCHAR * second, TCHAR** string_out)
{
	__try
	{
		if (second == NULL)return false;
		if (first == NULL)
		{
			this->Copy(second, &this->content);
			return true;
		}
		HRESULT hr;
		bool result = true;
		size_t str1, str2;

		hr = StringCbLength(first, STRSAFE_MAX_CCH * sizeof(TCHAR), &str1);
		if (SUCCEEDED(hr))
		{
			hr = StringCbLength(second, STRSAFE_MAX_CCH * sizeof(TCHAR), &str2);
			if (SUCCEEDED(hr))
			{
				(*string_out) = new TCHAR[(str1 + str2 + sizeof(TCHAR))];
				if (*string_out != NULL)
				{
					hr = StringCbPrintf(*string_out, (str1 + str2 + sizeof(TCHAR)), L"%s%s", first, second);
					if (FAILED(hr))
					{
						result = false;
					}
				}
				else
					result = false;
			}
			else
				result = false;
		}
		else
			result = false;

		return result;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return false;
	}	
}

bool iString::setUnicodePrefix()
{
	bool result;

	result = (this->content != nullptr) ? true : false;
	if(result)
	{
		TCHAR* newString = new TCHAR[(this->GetSize() + 6 * sizeof(TCHAR))];

		result = (newString != nullptr) ? true : false;
		if (result)
		{
			auto hr = StringCbPrintf(newString, (this->GetSize() + 6 * sizeof(TCHAR)), L"\\\\?\\%s", this->content);

			result = SUCCEEDED(hr) ? true : false;
			if (result)
			{
				this->Replace(newString);

				SafeDeleteArray((void**)&newString);
			}
		}
	}
	return result;
}

bool iString::Builder(TCHAR ** assembly_out, ...)
{
	__try
	{
		va_list Stringlist, copyStringlist;
		HRESULT hr = S_OK;
		LPTSTR curString;
		size_t aStringSize = 0;
		size_t curSize;
		TCHAR* string_assembly = NULL;

		va_start(Stringlist, assembly_out);
		va_copy(copyStringlist, Stringlist);

		do
		{
			curString = va_arg(Stringlist, LPTSTR);
			if (curString != NULL)
			{
				hr = StringCbLength(curString, sizeof(TCHAR)*STRSAFE_MAX_CCH, &curSize);
				if (FAILED(hr))
					break;

				aStringSize += curSize;
			}
		} while (curString != NULL);

		va_end(Stringlist);

		if (FAILED(hr))
			return false;
		else
		{
			aStringSize += sizeof(TCHAR);

			string_assembly = new TCHAR[aStringSize];
			if (string_assembly != NULL)
			{
				BOOL start = TRUE;

				do
				{
					curString = va_arg(copyStringlist, LPTSTR);
					if (curString != NULL)
					{
						if (start)
						{
							hr = StringCbCopy(string_assembly, aStringSize, curString);
							if (FAILED(hr))
								break;

							start = FALSE;
						}
						else
						{
							hr = StringCbCat(string_assembly, aStringSize, curString);
							if (FAILED(hr))
								break;
						}
					}
				} while (curString != NULL);

				if (FAILED(hr))
				{
					return false;
				}
				else
				{
					this->ClearContent();
					this->Copy(string_assembly, &this->content);

					if (assembly_out != NULL)
					{
						this->Copy(string_assembly, assembly_out);
					}
				}
				va_end(copyStringlist);

				delete[] string_assembly;
			}
			else
				return false;
		}
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return false;
	}
	return true;
}


bool iString::Equals(const TCHAR * stringToCompare)
{
	return this->Compare(stringToCompare, this->content);
}

bool iString::Equals(const iString& stringToCompare)
{
	return this->Equals(stringToCompare.GetData());
}

bool iString::EqualsNoCase(const TCHAR * stringToCompare)
{
	return this->CompareNoCase(stringToCompare, this->content);
}

bool iString::EqualsNoCase(const iString & stringToCompare)
{
	return this->EqualsNoCase(stringToCompare.GetData());
}

bool iString::Compare(const TCHAR * string1, const TCHAR * string2)
{
	if ((string1 == NULL) || (string2 == NULL))
	{
		return false;
	}
	__try
	{
		int i = 0;

		while (i < STRINGLIMIT)
		{
			if (string1[i] != string2[i])
			{
				return false;
			}
			else if ((string1[i] == L'\0') && (string2[i] == L'\0'))
			{
				break;
			}
			i++;
		}
		if (i == STRINGLIMIT)
			return false;

		return true;
	}
	__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return false;
	}
}

bool iString::CompareNoCase(const TCHAR * string1, const TCHAR * string2)
{
	if ((string1 == NULL) || (string2 == NULL))
	{
		return false;
	}
	__try
	{
		int i = 0;

		while (i < STRINGLIMIT)
		{
			if (string1[i] != string2[i])
			{
				if (!this->compare(string1[i], string2[i], false))
					return false;
			}
			else if ((string1[i] == L'\0') && (string2[i] == L'\0'))
			{
				break;
			}
			i++;
		}
		if (i == MAXXCOMPARE)
			return false;

		return true;
	}
	__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return false;
	}
}

bool iString::Copy(const TCHAR * in, TCHAR ** out)
{
	__try
	{
		if (in == NULL)return true;// ?????

		HRESULT hr;
		size_t len;

		hr = StringCbLength(in, STRSAFE_MAX_CCH * sizeof(TCHAR), &len);
		if (SUCCEEDED(hr))
		{
			len += sizeof(TCHAR);

			*out = new TCHAR[len];

			hr = (*out != NULL) ? S_OK : E_OUTOFMEMORY;
			if (SUCCEEDED(hr))
			{
				hr = StringCbCopy(*out, len, in);
			}
		}
		return SUCCEEDED(hr) ? true : false;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return false;
	}
}

void iString::Clear()
{
	this->ClearContent();
}

bool iString::CharToWide(const char * toConvert, TCHAR ** converted_out)
{
	__try
	{
		__try
		{
			if (toConvert == NULL)return false;
		}
		__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			return false;
		}

		bool result;
		size_t len_multib = strlen(toConvert);

		DWORD cbData = MultiByteToWideChar(CP_UTF8, 0, toConvert, -1, NULL, 0);

		result = (cbData > 0) ? true : false;
		if (result)
		{
			*converted_out = new TCHAR[cbData];

			result = (*converted_out == NULL) ? false : true;
			if (result)
			{
				SecureZeroMemory(*converted_out, sizeof(TCHAR)* cbData);

				result =
					(MultiByteToWideChar(CP_UTF8, 0, toConvert, (int)len_multib, *converted_out, cbData) > 0)// size_t to int, solved???
					? true : false;
			}
		}
		return result;
	}
	__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return false;
	}
}

bool iString::WideToChar(const TCHAR * toConvert, char ** converted_out)
{
	__try
	{
		__try
		{
			DWORD cbData = WideCharToMultiByte(CP_UTF8, 0, toConvert, -1, NULL, 0, NULL, NULL);

			bool result = (cbData == 0) ? false : true;
			if (result)
			{
				*converted_out = new (std::nothrow) CHAR[cbData];

				result = (*converted_out != NULL) ? true : false;
				if (result)
				{
					result = WideCharToMultiByte(CP_UTF8, 0, toConvert, -1, *converted_out, cbData, NULL, NULL)
						? true
						: false;
					if (result)
					{
						// ...
					}
				}
			}
			return result;
		}
		__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			return false;
		}
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return false;
	}
}

bool iString::Contains(const TCHAR * strToFind)
{
	iString _strToFind_(strToFind);
	return this->Contains(_strToFind_);
}

bool iString::Contains(const iString &strToFind)
{
	CHARSCOPE scope;
	return this->Contains(strToFind, &scope);
}

bool iString::Contains(const iString &strToFind, LPCHARSCOPE scope)
{
	return this->Contains(strToFind, scope, 0, true);
}

bool iString::Contains(const TCHAR * strToFind, LPCHARSCOPE scope)
{
	return this->Contains(strToFind, scope, 0, true);
}

bool iString::Contains(const iString &strToFind, LPCHARSCOPE scope, int startIndex, bool caseSensitive)
{
	if ((startIndex + strToFind.GetLength()) > this->GetLength())
		return false;
	else
	{
		try
		{
			bool isContaining = false;
			int i = startIndex;
			int containerLen = this->GetLength();
			int finderLen = strToFind.GetLength();
			auto toFind = strToFind.GetData();

			while (i <= (containerLen - finderLen))
			{
				if (this->compare(this->content[i], toFind[0], caseSensitive))
				{
					if(scope != nullptr)scope->startChar = i;

					for (int j = 0; j < finderLen; j++)
					{
						if (!this->compare(this->content[i], toFind[j], caseSensitive))break;
						else
						{
							if (j == (finderLen - 1))
							{
								if(scope != nullptr)scope->endChar = i;

								isContaining = true;
							}
						}
						i++;
					}
					if (isContaining)break;
				}
				i++;
			}
			if (!isContaining)
			{
				if (scope != nullptr)
				{
					scope->startChar = -1;
					scope->endChar = -1;
				}
			}
			return isContaining;
		}
		catch (...)
		{
			return false;
		}	
	}
}

bool iString::Contains(const TCHAR * strToFind, LPCHARSCOPE scope, int startIndex, bool caseSensitive)
{
	iString conti(strToFind);

	return this->Contains(conti, scope, startIndex, caseSensitive);
}

bool iString::Contains(const TCHAR * strToFind, LPCHARSCOPE scope, int startIndex)
{
	return this->Contains(strToFind, scope, startIndex, true);
}

iString iString::GetSegment(LPCHARSCOPE cs)
{
	if (this->verifyScope(cs))
	{
		int i = 0;
		int segLength = cs->endChar - cs->startChar;

		TCHAR* segment = new TCHAR[(size_t)(sizeof(TCHAR)*(segLength + 1))];
		if (segment != nullptr)
		{
			for (i = 0; i <= segLength; i++)
			{
				segment[i] = this->content[cs->startChar + i];
			}
			segment[i] = L'\0';

			iString Segment_(segment);

			delete[] segment;

			return Segment_;
		}
		else
		{
			return iString(L"error::iString::class::GetSegment::outOfMemory\0");
		}
	}
	return iString(L"error::iString::class::GetSegment::invalidScope\0");
}

bool iString::SetSegment(const iString & segment, LPCHARSCOPE cs)
{
	return this->SetSegment(segment.GetData(), cs);
}

bool iString::SetSegment(const TCHAR * segment, LPCHARSCOPE cs)
{
	auto len = this->GetLength();

	auto result =
		((segment == nullptr) || (len == 0) || (cs->endChar > len) || (cs->startChar > len) || (cs->startChar > cs->endChar) || (cs->startChar < 0) || (cs->endChar < 0))
		? false : true;
	if (result)
	{
		if (this->verifyScope(cs))
		{
			if ((cs->startChar == 0) && (cs->endChar >= len)) // the scope is the whole content
			{
				this->Replace(segment);
			}
			else if ((cs->startChar == 0) && (cs->endChar < len)) // the scope is at the beginning of the string
			{
				iString chrSeg = this->GetSegment(cs); // segment to remove

				iString trailingSegment(
					this->GetData()
				);
				trailingSegment -= chrSeg;
				this->Replace(segment);
				result = this->Append(trailingSegment);
			}
			else if ((cs->startChar > 0) && (cs->endChar < len)) // the scope is a segment in the middle
			{
				CHARSCOPE scope;
				scope.startChar = 0;
				scope.endChar = cs->startChar - 1;

				iString first = this->GetSegment(&scope);

				scope.startChar = cs->endChar + 1;
				scope.endChar = len;

				iString last = this->GetSegment(&scope);

				this->Replace(first);
				this->Append(segment);
				this->Append(last);
			}
			else // the segment must be at the end of the string
			{
				CHARSCOPE scope;
				scope.startChar = 0;
				scope.endChar = cs->startChar - 1;

				iString first = GetSegment(&scope);
				this->Replace(first);
				result = this->Append(segment);
			}
		}
	}
	return result;
}


const int iString::GetLength() const
{
	size_t size = this->GetSize();
	if (size != 0)
	{
		return int(size / sizeof(TCHAR));
	}
	else
		return 0;
}

const size_t iString::GetSize() const
{
	if (this->content != NULL)
	{
		size_t len = 0;

		HRESULT hr = StringCbLength(this->content, sizeof(TCHAR) * STRSAFE_MAX_CCH, &len);
		if (SUCCEEDED(hr))
		{
			return len;
		}
		else
			return (size_t)0;
	}
	else
		return (size_t)0;
}

const char* iString::GetDataAsChar()
{
	if (
		this->WideToChar(this->content, &this->content_as_char)
			)return this->content_as_char;
	else return NULL;
}

TCHAR iString::getCharAt(int index)
{
	if (this->content != nullptr)
	{
		if (index > this->GetLength())
			return L'\0';
		else
			return this->content[index];
	}
	else
		return L'\0';
}

void iString::insertCharAt(int index, TCHAR _Char)
{
	auto length = this->GetLength();

	if ((index < length) && (index >= 0))
	{
		if (this->content != nullptr)
		{
			iString tmpString(this->content);
			
			this->ClearContent();

			this->content = new TCHAR[length + 2];
			if (this->content != nullptr)
			{
				for (int i = 0; i < index; i++)
				{
					this->content[i] = tmpString.getCharAt(i);
				}
				this->content[index] = _Char;

				for (int i = index; i < (length + 1); i++)
				{
					this->content[i + 1] = tmpString.getCharAt(i);
				}
				this->content[length + 1] = L'\0';
			}
		}
	}
}

iString * iString::FromInt(int i)
{
	TCHAR buffer[256] = { 0 };

	HRESULT hr = StringCbPrintf(buffer, sizeof(buffer), L"%i", i);
	if (SUCCEEDED(hr))
	{
		auto string = new iString(buffer);
		return string;
	}
	return nullptr;
}

iString iString::fromInt(int i)
{
	auto str = iString::FromInt(i);
	iString out(*str);
	SafeRelease(&str);
	return out;
}

iString* iString::FromBoolean(bool b)
{
	auto str = new iString();
	if (b)str->Replace(L"true");
	else str->Replace(L"false");
	return str;
}

iString * iString::FromHex(DWORD x)
{
	TCHAR buffer[256] = { 0 };

	HRESULT hr = StringCbPrintf(buffer, sizeof(buffer), L"%x", x);
	if (SUCCEEDED(hr))
	{
		auto string = new iString(buffer);
		return string;
	}
	return nullptr;
}

iString iString::fromHex(DWORD x)
{
	auto str = iString::FromHex(x);
	if (str != nullptr)
	{
		iString hex(*str);
		return hex;
	}
	else
		return iString(L"");
}

iString iString::fromHex(uintX x)
{
	TCHAR buffer[256] = { 0 };

	HRESULT hr = StringCbPrintf(buffer, sizeof(buffer), L"%IX", x);	
	if (SUCCEEDED(hr))
	{
		iString hex(buffer);
		return hex;
	}
	return iString(0);
}

iString * iString::FromUInt(unsigned int i)
{
	TCHAR buffer[256] = { 0 };

	HRESULT hr = StringCbPrintf(buffer, sizeof(buffer), L"%u", i);
	if (SUCCEEDED(hr))
	{
		auto string = new iString(buffer);
		return string;
	}
	return nullptr;
}

iString iString::fromUInt(unsigned int i)
{
	auto str = iString::FromUInt(i);
	iString out(*str);
	SafeRelease(&str);
	return out;
}

iString iString::fromPointer(LPVOID p)
{
	TCHAR buffer[256] = { 0 };
	iString out(L"invalid address");

	HRESULT hr = StringCbPrintf(buffer, sizeof(buffer), L"%p", p);
	if (SUCCEEDED(hr))
	{
		out.Replace(buffer);
	}
	return out;
}

void iString::Replace(const iString& string)
{
	this->ClearContent();
	this->Copy(string.GetData(), &this->content);
}

void iString::Replace(const TCHAR * string)
{
	if (string == this->content)
	{
		return;
	}
	else
	{
		this->ClearContent();
		this->Copy(string, &this->content);
	}
}

void iString::Remove(const iString& toRemove)
{
	auto len = this->GetLength();
	CHARSCOPE cs;

	if (this->Contains(toRemove, &cs, 0, true))
	{
		if ((cs.startChar == 0) && (cs.endChar >= len))// the string to remove is the whole content
		{
			this->Clear();
		}
		else if ((cs.startChar == 0) && (cs.endChar < len))// the string to remove is at the beginning of the string
		{
			cs.startChar = cs.endChar + 1;
			cs.endChar = len;

			auto endSeg = this->GetSegment(&cs);
			this->Replace(endSeg);
		}
		else if ((cs.startChar > 0) && (cs.endChar == len))// the string to remove is at the end of the string
		{
			cs.endChar = cs.startChar - 1;
			cs.startChar = 0;

			auto startSeg = this->GetSegment(&cs);
			this->Replace(startSeg);
		}
		else // the string to remove must be in the middle of the string
		{
			CHARSCOPE firstSEG, secondSEG;
			firstSEG.startChar = 0;
			firstSEG.endChar = cs.startChar - 1;
			secondSEG.startChar = cs.endChar + 1;
			secondSEG.endChar = this->GetLength();

			iString first = this->GetSegment(&firstSEG);
			iString second = this->GetSegment(&secondSEG);
			iString newString = first + second;

			this->Replace(newString);
		}
	}
}

void iString::Remove(const TCHAR * toRemove)
{
	iString _toRemove_(toRemove);
	this->Remove(_toRemove_);
}

bool iString::compare(TCHAR a, TCHAR b, bool caseSensitive)
{
	if (!caseSensitive)
	{
		if (a == b)
			return true;
		else
		{
			if (IsChar(a) && IsChar(b))
			{
				auto index_L1 = static_cast<int>(a);
				auto index_L2 = static_cast<int>(b);

				if (index_L1 < 91)
				{
					if ((index_L1 + 32) == index_L2)
						return true;
				}
				else if (index_L2 < 91)
				{
					if ((index_L2 + 32) == index_L1)
						return true;
				}
			}
			return false;
		}
	}
	else
	{
		if (a == b)return true;
		else return false;
	}
}

bool iString::verifyScope(LPCHARSCOPE cs)
{
	if ((cs->endChar < 0) || (cs->startChar < 0))
		return false;
	else if (cs->endChar < cs->startChar)
		return false;
	else if (cs->endChar > this->GetLength())
		return false;
	else
		return true;
}

bool iString::verifyScope(LPCHARSCOPE cs, int stringLength)
{
	if ((cs->endChar < 0) || (cs->startChar < 0))
		return false;
	else if (cs->endChar < cs->startChar)
		return false;
	else if (cs->endChar > stringLength)
		return false;
	else
		return true;
}



