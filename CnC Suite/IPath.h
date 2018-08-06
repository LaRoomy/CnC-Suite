#pragma once
#ifndef _IPATH_H_
#define _IPATH_H_

#include<Windows.h>
#include<ShlObj.h>
#include<Shlwapi.h>
#include"BasicFPO.h"
#include<StringClass.h>

typedef int IPathType;

class IPath
{
public:
	IPath();
	IPath(const IPath& path);
	IPath(const TCHAR* path);
	IPath(const iString& path);
	IPath(REFKNOWNFOLDERID folderID);
	~IPath();

	static const IPathType TYPE_DIRECTORY = 1;
	static const IPathType TYPE_FILE = 2;

	LPCTSTR GetPathData() const {
		return this->internal_path.GetData();
	}
	const iString& GetPath() const {
		return this->internal_path;
	}

	int GetLength() {
		return this->internal_path.GetLength();
	}


	bool Exists();

	bool Equals(LPCTSTR nPath);
	bool Equals(const iString &nPath);
	bool Equals(const IPath &nPath);

	bool HasTrailingBackSlash();

	bool Contains(LPCTSTR path);
	bool Contains(const iString &path);
	bool Contains(const IPath &path);

	IPathType GetType();

	void AppendPath(LPCTSTR appendix);
	void AppendPath(const iString &appendix);
	void AppendPath(const IPath &appendix);

	bool SetUnicodePrefix();
	void SetTrailingBackSlash();

	bool RemoveFilename();
	bool RemoveLastDirectory();

	bool GetFileExtension(iString &ext_out);
	bool GetFileName(iString &fileName_out, bool removeExtension);
	bool GetFolderName(iString &folderName_out);

	bool Create();// creates the file/folder at the path
	void Clear();

	// TODO: operators: + / - / = / == / != / += / -=

	IPath& operator= (const IPath& _IPath)
	{
		this->internal_path.Replace(
			_IPath.GetPathData()
		);
		return *this;
	}
	IPath& operator= (const iString& _iString)
	{
		this->internal_path.Replace(
			_iString.GetData()
		);
		return *this;
	}
	IPath& operator= (const TCHAR* lpsz)
	{
		this->internal_path.Replace(
			lpsz
		);
		return *this;
	}

	void operator+= (LPCTSTR add)
	{
		this->AppendPath(add);
	}
	void operator+= (iString add)
	{
		this->AppendPath(add);
	}
	void operator+= (IPath add)
	{
		this->AppendPath(add);
	}

private:
	iString internal_path;

};




#endif // !_IPATH_H_