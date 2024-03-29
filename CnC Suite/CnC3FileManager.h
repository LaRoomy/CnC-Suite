#pragma once
#include<Windows.h>
#include"cObject.h"
#include<ShObjIdl.h>
#include<StringClass.h>
#include<Xmap.h>

typedef const TCHAR* PropertyID;

const COMDLG_FILTERSPEC CnC3DataType[] =
{
	{ L"Extended Nc-Program (*.cnc3)", L"*.cnc3" }
};

const COMDLG_FILTERSPEC AllFiles[] =
{
	{L"All Documents (*.*)",		L"*.*"}
};

constexpr auto EMPTY_PROPERTY_STRING = L"...";

class CnC3File
	:public ClsObject<CnC3File>,
	public iCollectable<CnC3File>
{
public:
	CnC3File();
	CnC3File(const CnC3File& file);

	static PropertyID PID_DESCRIPTION_ONE;
	static PropertyID PID_DESCRIPTION_TWO;
	static PropertyID PID_DESCRIPTION_THREE;

	HRESULT Load();
	HRESULT Load(LPCTSTR path);
	HRESULT Save();
	HRESULT Save(LPCTSTR path);
	void Clear();

	LPCTSTR GetPath() const {
		return this->pathToFile.GetData();
	}
	void SetPath(LPCTSTR path) {
		this->pathToFile = path;
	}
	bool HasPath() const {
		return
			(this->pathToFile.GetData() != nullptr)
			? true : false;
	}
	LPCTSTR GetNCContent() const {
		return this->ncContent.GetData();
	}
	void SetNCContent(LPCTSTR content) {
		this->ncContent = content;
	}
	LPCTSTR GetFilename(bool hideFileExtension);

	HRESULT GetStatus() {
		return this->status;
	}
	void SetStatus(HRESULT hr) {					// do not forget to update the status!
		this->status = hr;
	}
	void operator= (HRESULT hr) {
		this->status = hr;
	}
	CnC3File& operator= (const CnC3File& file) {
		this->copy(file);
		return *this;
	}
	bool operator== (const CnC3File& file) {
		auto thisPath = this->GetPath();
		auto filePath = file.GetPath();
		return (CompareStringsAsmW(thisPath, filePath) == 1) ? true : false;
	}
	bool Succeeded() {
		return SUCCEEDED(this->status) ? true : false;
	}
	const wchar_t* ToString() {
		return this->ncContent.GetData();
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

	void AddProperty(const iString& propertyKey, const iString& pData) {
		this->cnc3PropertyMap.Add(propertyKey, pData);
	}
	void RemoveProperty(const iString& propertyKey) {
		this->cnc3PropertyMap.Remove(propertyKey);
	}
	bool ReplaceProperty(const iString& propertyKey, const iString& pData) {
		return this->cnc3PropertyMap.Replace(propertyKey, pData);
	}
	int GetPropertyCount() {
		this->cnc3PropertyMap.GetCount();
	}
	void GetProperty(int index, iString& propertyKey, iString& pData) const {
		this->cnc3PropertyMap.GetPairAtIndex(index, propertyKey, pData);
	}
	void GetProperty(PropertyID Id, TCHAR** data_out) const;
	iString GetProperty(PropertyID Id) const;

private:
	HRESULT status;
	bool skippathsaving;

	iString ncContent;
	iString pathToFile;
	iString filename;

	Xmap<iString, iString> cnc3PropertyMap;

	// filename
	// etc.

	void copy(const CnC3File& file);

	HRESULT CnC3Object_toBuffer(iString& buffer);
	HRESULT Buffer_toCnC3Object(LPCTSTR buffer);

	HRESULT ReadPropertySection(LPCTSTR buffer, LPCHARSCOPE sectionScope);
	HRESULT ReadContentSection(LPCTSTR buffer);

	// legacy methods to read cnc3 files below version 2.0
	BOOL Extract(LPCTSTR buffer);
	int SearchFor_Property_begin(int, LPCTSTR, int, LPCTSTR);
	int Get_Property(int, int, LPCTSTR);
};

class CnC3FileCollection
	: public ClsObject<CnC3FileCollection>
{
public:
	CnC3FileCollection() : status(E_NOT_SET) {}
	CnC3FileCollection(const CnC3FileCollection& collection) : status(E_NOT_SET) {
		itemCollection<CnC3File> newCol =
			collection.data;

		for (int i = 0; i < newCol.GetCount(); i++)
		{
			this->Add(
				newCol.GetAt(i)
			);
		}
		if (newCol.GetCount() > 0)
			this->status = S_OK;
	}

	void Add(const CnC3File& file) {
		this->data.AddItem(file);
	}
	CnC3File& GetAt(int index) {
		this->current = data.GetAt(index);
		return this->current;
	}
	int GetCount() const {
		return this->data.GetCount();
	}
	void Clear() {
		this->data.Clear();
	}

	void SetStatus(HRESULT hr) {
		this->status = hr;
	}
	HRESULT GetStatus() {
		return this->status;
	}

	CnC3FileCollection& operator+= (const CnC3File& file) {
		this->Add(file);
		return *this;
	}
	CnC3FileCollection& operator= (const CnC3FileCollection& collection) {

		this->Clear();

		itemCollection<CnC3File> newCol =
			collection.data;

		for (int i = 0; i < newCol.GetCount(); i++)
		{
			this->Add(
				newCol.GetAt(i)
			);
		}
		return *this;
	}
	void operator= (HRESULT hr) {
		this->status = hr;
	}
	bool Succeeded() {
		return SUCCEEDED(this->status) ? true : false;
	}

	const wchar_t* ToString() {
		return L"not_implemented";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

private:
	HRESULT status;
	CnC3File current;
	itemCollection<CnC3File> data;
};

__interface IExportFormatProtocol {
public:
	void FormatForExport(const CnC3File& file, iString& buffer_out);
};
__interface IFileDialogUserEvents {
public:
	void SaveAsPathWasSelected(cObject sender, LPCTSTR path);
};


class CnC3FileManager
	:public ClsObject<CnC3FileManager>
{
public:
	CnC3FileManager();
	CnC3FileManager(HWND _owner_);

	// Shows the Open-Dialog to get one or multiple cnc3-file-objects
	CnC3FileCollection& Open();
	// Opens the file at the specified path direct (without showing a dialog)
	CnC3File& Open(LPCTSTR path);
	// Shows the Save-Dialog and saves the given file at a location selected by the user
	// The returned cnc3-object is the full-defined representation of the saved file
	CnC3File& SaveAs(const CnC3File& file);
	// Saves the cnc3-object to file at the given location in the cnc3-object
	HRESULT Save(CnC3File& file);
	// Shows the Open-Dialog but the returned object has no path, only the nc-content is valid (if status == S_OK)
	// NOTE: the dialog-text for the import-style is not initally set! It must be done before calling this method (by calling SetDialogText(...))
	CnC3File& Import();
	// Shows the Save-Dialog but the user can choose the filetype he wants
	// NOTE: the method calls the IExportFormatProtocol::FormatForExport method if a handler was subscribed
	// -> so the caller could reformat the nc-content for export if neccessary
	// NOTE: the dialog-text for the export-style is not initally set! It must be done before calling this method (by calling SetDialogText(...))
	HRESULT ExportAs(const CnC3File& file);

	BOOL ConvertToCnC3(LPCTSTR pathOfFileToConvert, CnC3File& file_out);

	void SetDialogText(LPCTSTR CaptionText, LPCTSTR ButtonText, LPCTSTR FileText);
	void SetTargetFolder(LPCTSTR Path);
	void SetHwndOwner(HWND Owner);

	void SetExportFormatHandler(IExportFormatProtocol* routine) {
		this->exportFormatHandler = routine;
	}
	void SetUserEventHandler(IFileDialogUserEvents* eventHandler) {
		this->userEventHandler = eventHandler;
	}

	const wchar_t* ToString(){
		return L"not_implemented";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}
private:
	HWND owner;
	iString captiontext;
	iString buttontext;
	iString filetext;
	iString targetfolder;

	IExportFormatProtocol* exportFormatHandler;
	IFileDialogUserEvents* userEventHandler;

	CnC3FileCollection openResult;
	CnC3File currentFile;

	HRESULT setDialogText(IFileDialog *Ifd);
	HRESULT setTargetFolder(IFileDialog *Ifd);
};