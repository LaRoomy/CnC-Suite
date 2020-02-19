#pragma once
#include"..//external.h"
#include"ItemCollection.h"

using namespace std;

constexpr auto STRINGLIMIT = 50000;
constexpr auto ISTRING_FAIL_INDICATOR = L"{ 0xf5a051e0, 0xde80, 0x19d6, 0xcc, 0x4e, 0xa8, 0x01, 0x4a, 0xb3, 0x1f, 0xff }";

typedef struct _CHARSCOPE {
	int startChar;
	int endChar;
}CHARSCOPE, *LPCHARSCOPE;

class iString
	: public iCollectable<iString>
{
public:
	iString();
	iString(const iString&);
	iString(const TCHAR* string);
	iString(const char* string);
	iString(int toString);
	iString(DWORD toString);
	iString(size_t contentsize);

	~iString();

	// lifetime management
	iString* AddReference()
	{
		this->referenceCounter++;
		return this;
	}
	void Release()
	{
		this->referenceCounter--;
		if (this->referenceCounter == 0)
		{
			delete this;
		}
	}

	iString* iString::getInstance() { return this; }

	bool Append(const iString* stringToAppend);
	bool Append(const iString& stringToAppend);
	bool Append(const TCHAR* stringToAppend);
	bool Append(const TCHAR* first, const TCHAR* second, TCHAR** string_out);

	bool setUnicodePrefix();

	// Watch out: argumentlist must be delimited with nullptr!
	// if this object contains data it will be deleted
	bool Builder(TCHAR** assembly_out, ...);
	bool Equals(const TCHAR* stringToCompare);
	bool Equals(const iString& stringToCompare);
	bool EqualsNoCase(const TCHAR* stringToCompare);
	bool EqualsNoCase(const iString& stringToCompare);
	bool Compare(const TCHAR* string1, const TCHAR* string2);// should be static
	bool CompareNoCase(const TCHAR* string1, const TCHAR* string2);// should be static
	bool Copy(const TCHAR* in, TCHAR** out);
	void Clear();

	bool CharToWide(const char* toConvert, TCHAR** converted_out);
	bool WideToChar(const TCHAR* toConvert, char** converted_out);

	bool Contains(const TCHAR* strToFind);
	bool Contains(const iString& strToFind);
	bool Contains(const iString& strToFind, LPCHARSCOPE scope);
	bool Contains(const TCHAR* strToFind, LPCHARSCOPE scope);
	// finds a string inside this string.
	// scope - returns the start and end -index of the word;
	// startIndex - start-position for the search-routine;
	// caseSensitive - if true: case sensitive is enabled, false otherwise;
	bool Contains(const iString& strToFind, LPCHARSCOPE scope, int startIndex, bool caseSensitive);
	bool Contains(const TCHAR* strToFind, LPCHARSCOPE scope, int startIndex, bool caseSensitive);
	bool Contains(const TCHAR* strToFind, LPCHARSCOPE scope, int startIndex);

	iString GetSegment(LPCHARSCOPE cs);
	bool SetSegment(const iString& segment, LPCHARSCOPE cs);
	bool SetSegment(const TCHAR* segment, LPCHARSCOPE cs);

	static bool verifyScope(LPCHARSCOPE, int stringLength);

	const int GetLimit() { return STRINGLIMIT; }
	const int GetLength() const;
	const size_t GetSize() const;
	const TCHAR* GetData() const { return this->content; }
	TCHAR* getContentReference() { return this->content; }
	//The caller is responsible for freeing the string
	TCHAR* GetDataAsCopy()
	{ 
		TCHAR* _content_ = nullptr;
		this->Copy(this->content, &_content_);
		return _content_;
	}
	const char* GetDataAsChar();

	TCHAR getCharAt(int index);

	void insertCharAt(int index, TCHAR _Char);

	int getAsInt() {
		return (this->content != nullptr)
			? _wtoi(this->content)
			: throw DataAccessViolationException(EXCEPTION_ACCESS_VIOLATION, "Converting value to integer failed: value was nullptr");
	}
	unsigned int getAsUInt() {
		return (this->content != nullptr)
			? static_cast<unsigned int>(wcstoul(this->content, nullptr, 0))
			: throw DataAccessViolationException(EXCEPTION_ACCESS_VIOLATION, "Converting value to unsigned integer failed: value was nullptr");
	}

	static iString* FromInt(int i);
	static iString fromInt(int i);
	static iString* FromBoolean(bool b);
	static iString* FromHex(DWORD x);
	static iString fromHex(DWORD x);
	static iString fromHex(uintX x);
	static iString* FromUInt(unsigned int i);
	static iString fromUInt(unsigned int i);
	static iString fromPointer(LPVOID p);

	void Replace(const iString& string);
	void Replace(const TCHAR* string);

	void Remove(const iString& toRemove);
	void Remove(const TCHAR* toRemove);

	void setToFailedState() {
		this->Replace(ISTRING_FAIL_INDICATOR);
	}
	bool succeeded() {
		return !this->Equals(ISTRING_FAIL_INDICATOR);
	}
	bool failed() {
		return this->Equals(ISTRING_FAIL_INDICATOR);
	}

	iString operator+ (const iString &S)
	{
		iString newString(this->content);
		newString.Append(S);
		return newString;
	}
	iString operator+ (int i)
	{
		iString newString(this->content);

		TCHAR buffer[256] = { 0 };
		StringCbPrintf(buffer, sizeof(buffer), L"%i\0", i);

		newString.Append(buffer);
		return newString;
	}
	iString operator+ (DWORD d)
	{
		iString newString(this->content);

		TCHAR buffer[256] = { 0 };
		StringCbPrintf(buffer, sizeof(buffer), L"%u\0", d);

		newString.Append(buffer);
		return newString;
	}

	iString operator+ (const TCHAR* S)
	{
		iString newString(this->content);
		newString.Append(S);
		return newString;
	}
	iString operator- (const iString& S)
	{
		iString newString(this->content);
		newString.Remove(S);
		return newString;
	}
	void operator-= (const iString& S)
	{
		this->Remove(S);
	}
	void operator+= (const iString& S)
	{
		this->Append(S);
	}
	void operator+= (iString* S)
	{
		this->Append(S->GetData());
	}
	void operator +=(const TCHAR* s)
	{
		this->Append(s);
	}

	iString& operator= (const iString& S)
	{
		this->ClearContent();
		this->Copy(S.GetData(), &this->content);
		return *this;
	}
	iString& operator= (const TCHAR* s)
	{
		this->ClearContent();
		this->Copy(s, &this->content);
		return *this;
	}
	bool operator== (const iString& S)
	{
		return this->Equals(S);
	}
	bool operator== (LPCTSTR s)
	{
		return this->Equals(s);
	}
	bool operator& (const iString& S)
	{
		return this->Contains(S, nullptr, 0, true);
	}
	bool operator!= (const iString& S)
	{
		return !this->Equals(S);
	}
	bool operator!= (LPCTSTR s)
	{
		return !this->Equals(s);
	}
private:
	int referenceCounter;

	TCHAR* content;
	char* content_as_char;

	void Initialize();
	void ClearContent();

	bool compare(TCHAR, TCHAR, bool);
	bool verifyScope(LPCHARSCOPE);
};


