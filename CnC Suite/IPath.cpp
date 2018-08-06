#include"IPath.h"
#include"HelperF.h"

IPath::IPath()
{
}

IPath::IPath(const IPath & path)
{
	this->internal_path.Replace(
		path.GetPathData()
	);
}

IPath::IPath(const TCHAR * path)
{
	this->internal_path.Replace(path);
}

IPath::IPath(const iString & path)
{
	this->internal_path.Replace(
		path.GetData()
	);
}

IPath::IPath(REFKNOWNFOLDERID folderID)
{
	auto bfpo = CreateBasicFPO();
	if (bfpo != nullptr)
	{
		TCHAR* kfp = nullptr;

		if (bfpo->GetKnownFolderPath(&kfp, folderID) == TRUE)
		{
			this->internal_path = kfp;

			SafeDeleteArray((void**)&kfp);
		}
		SafeRelease(&bfpo);
	}
}

IPath::~IPath()
{
}

bool IPath::Exists()
{
	return (this->GetType() == -1)
		? false : true;
}

bool IPath::Equals(LPCTSTR nPath)
{
	return this->internal_path.Equals(nPath);
}

bool IPath::Equals(const iString & nPath)
{
	return this->internal_path.Equals(nPath);
}

bool IPath::Equals(const IPath & nPath)
{
	return this->internal_path.Equals(
		nPath.GetPathData()
	);
}

bool IPath::HasTrailingBackSlash()
{
	auto cHr =
		this->internal_path.getCharAt(
			this->internal_path.GetLength() - 1
		);
	return (cHr == L'\\') ? true : false;
}

bool IPath::Contains(LPCTSTR path)
{
	__try
	{
		if (path != nullptr)
		{
			int i = 0;
			auto internalpath_ = this->internal_path.GetData();

			if (internalpath_ != nullptr)
			{
				while (path[i] != L'\0')
				{
					if (path[i] != internalpath_[i])
					{
						return false;
					}
					i++;
				}
				return true;
			}
		}
		return false;
	}
	__except (GetExceptionCode() == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return false;
	}
}

bool IPath::Contains(const iString & path)
{
	return this->Contains(
		path.GetData()
	);
}

bool IPath::Contains(const IPath & path)
{
	return this->Contains(
		path.GetPathData()
	);
}

IPathType IPath::GetType()
{
	auto rslt = isFile(
		this->internal_path.GetData()
	);

	if (rslt >= 0)
		return rslt ? IPath::TYPE_FILE : IPath::TYPE_DIRECTORY;
	else
		return (IPathType)-1;

}

void IPath::AppendPath(LPCTSTR appendix)
{
	if (appendix != nullptr)
	{
		if ((appendix[0] != L'\\')
			&& (!this->HasTrailingBackSlash()))
			this->SetTrailingBackSlash();

		this->internal_path.Append(appendix);
	}
}

void IPath::AppendPath(const iString & appendix)
{
	if ((!this->HasTrailingBackSlash()) &&
		(this->internal_path.getCharAt(0) != L'\\'))
		this->SetTrailingBackSlash();

	this->internal_path.Append(appendix);
}

void IPath::AppendPath(const IPath & appendix)
{
	this->AppendPath(
		appendix.GetPathData()
	);
}

bool IPath::SetUnicodePrefix()
{
	return this->internal_path.setUnicodePrefix();
}

void IPath::SetTrailingBackSlash()
{
	this->internal_path.Append(L"\\");
}

bool IPath::RemoveFilename()
{
	bool result = false;

	if (this->GetType() == IPath::TYPE_FILE)
	{
		auto bfpo = CreateBasicFPO();
		{
			TCHAR* newPath = nullptr;

			if (bfpo->RemoveFilenameFromPath(
				this->internal_path.GetData(),
				&newPath)
				== TRUE)
			{
				this->internal_path.Replace(newPath);
				result = true;

				SafeDeleteArray((void**)&newPath);
			}
			SafeRelease(&bfpo);
		}
	}
	return result;
}

bool IPath::RemoveLastDirectory()
{
	bool result = false;

	if (this->GetType() == IPath::TYPE_DIRECTORY)
	{
		auto bfpo = CreateBasicFPO();
		{
			TCHAR* newPath = nullptr;

			if (bfpo->RemoveFilenameFromPath(
				this->internal_path.GetData(),
				&newPath)
				== TRUE)
			{
				this->internal_path.Replace(newPath);
				result = true;

				SafeDeleteArray((void**)&newPath);
			}
			SafeRelease(&bfpo);
		}
	}
	return result;
}

bool IPath::GetFileExtension(iString & ext_out)
{
	bool result = false;

	if (this->GetType() == IPath::TYPE_FILE)
	{
		auto bfpo = CreateBasicFPO();
		if (bfpo != nullptr)
		{
			TCHAR* ext = nullptr;

			if (bfpo->GetFileExtension(
				this->internal_path.GetData(),
				&ext)
				== TRUE)
			{
				ext_out.Replace(ext);
				result = true;
				SafeDeleteArray((void**)&ext);
			}
			SafeRelease(&bfpo);
		}
	}
	return result;
}

bool IPath::GetFileName(iString & fileName_out, bool removeExtension)
{
	bool result = false;

	if (this->GetType() == IPath::TYPE_FILE)
	{
		auto bfpo = CreateBasicFPO();
		if (bfpo != nullptr)
		{
			TCHAR* filename = nullptr;

			if (bfpo->GetFilenameOutOfPath(
				this->internal_path.GetData(),
				&filename,
				removeExtension ? TRUE : FALSE)
				== TRUE)
			{
				fileName_out.Replace(filename);
				result = true;
				SafeDeleteArray((void**)&filename);
			}
			SafeRelease(&bfpo);
		}
	}
	return result;
}

bool IPath::GetFolderName(iString & folderName_out)
{
	bool result = false;

	if (this->GetType() == IPath::TYPE_DIRECTORY)
	{
		auto bfpo = CreateBasicFPO();
		if (bfpo != nullptr)
		{
			TCHAR* foldername = nullptr;

			if (bfpo->GetFilenameOutOfPath(
				this->internal_path.GetData(),
				&foldername,
				FALSE)
				== TRUE)
			{
				folderName_out.Replace(foldername);
				result = true;
				SafeDeleteArray((void**)&foldername);
			}
			SafeRelease(&bfpo);
		}
	}
	return result;
}

bool IPath::Create()
{
	// TODO!

	return false;
}

void IPath::Clear()
{
	this->internal_path.Clear();
}
