#pragma once
#include"BasicFPO.h"
#include<new>

#define		EMPTY_DIR		5

class BasicFileNPathOperations : public BasicFPO
{
public:
	BasicFileNPathOperations();

	 void BasicFileNPathOperations::Release() { delete this; }
	 BOOL BasicFileNPathOperations::SaveBufferToFile(LPCTSTR buffer, LPCTSTR path) { return this->pSaveBuffer(buffer, path, CP_ACP); }
	 BOOL BasicFileNPathOperations::LoadBufferFmFile(TCHAR** buffer, LPCTSTR path) { return this->pLoadBuffer(buffer, path, CP_ACP); }
	 BOOL BasicFileNPathOperations::SaveBufferToFileAsUtf8(LPCTSTR buffer, LPCTSTR path){ return this->pSaveBuffer(buffer, path, CP_UTF8); }
	 BOOL BasicFileNPathOperations::LoadBufferFmFileAsUtf8(TCHAR** buffer, LPCTSTR path){ return this->pLoadBuffer(buffer, path, CP_UTF8); }
	 BOOL BasicFileNPathOperations::GetKnownFolderPath(TCHAR** path, REFKNOWNFOLDERID ID) { return this->pGetKnownFolder(path, ID); }
	 BOOL BasicFileNPathOperations::RemoveFilenameFromPath(_Inout_ LPTSTR path) { return this->pRemoveFilename(path); }
	 BOOL BasicFileNPathOperations::RemoveFilenameFromPath(_In_ LPCTSTR path, _Outptr_ TCHAR** newPath_out) { return this->pRemoveFilename(path, newPath_out); }
	 BOOL BasicFileNPathOperations::GetFilenameOutOfPath(LPCTSTR path, TCHAR** filename_out, BOOL hideFileExt) {return this->pGetFilenameOutOfPath(path, filename_out, hideFileExt);}
	 BOOL BasicFileNPathOperations::IfFileExistsChangePath(TCHAR** path) { return this->pIfFileExistsChangePath(path); }
	 BOOL BasicFileNPathOperations::RemoveFileExtension(LPTSTR string, TCHAR** extension_out) { return this->pRemoveFileExt(string, extension_out); }
	 BOOL BasicFileNPathOperations::GetFileExtension(LPCTSTR string, TCHAR** extension_out) { return this->pGetFileExt(string, extension_out); }
	 BOOL BasicFileNPathOperations::CountDirectoryContent(LPCTSTR path, DWORD* files, DWORD* folders) { return this->pCountDirContent(path, files, folders); }
	 BOOL BasicFileNPathOperations::CheckForFileExist(LPCTSTR path) { return this->pCheckForFileExist(path); }
	 BOOL BasicFileNPathOperations::VerifyCommandline(LPCTSTR nCmdLine, TCHAR** Path_out) { return this->pVerifyCmdLine(nCmdLine, Path_out); }
	 BOOL BasicFileNPathOperations::VerifyFilename(LPCTSTR fileName) { return this->pVerifyFilename(fileName); }
	 BOOL BasicFileNPathOperations::GetFileTimes(LPCTSTR path, LPSYSTEMTIME created, LPSYSTEMTIME lastAccessed, LPSYSTEMTIME lastWritten);

private:
	BOOL pSaveBuffer(LPCTSTR, LPCTSTR, UINT);
	BOOL pLoadBuffer(TCHAR**, LPCTSTR, UINT);
	BOOL pGetKnownFolder(TCHAR**, REFKNOWNFOLDERID);
	BOOL pRemoveFilename(LPTSTR);
	BOOL pRemoveFilename(LPCTSTR, TCHAR**);
	BOOL pGetFilenameOutOfPath(LPCTSTR, TCHAR**, BOOL);
	BOOL pIfFileExistsChangePath(TCHAR**);
	BOOL pRemoveFileExt(LPTSTR, TCHAR**);
	BOOL pGetFileExt(LPCTSTR, TCHAR**);
	BOOL pCountDirContent(LPCTSTR, DWORD*, DWORD*);
	BOOL pCheckForFileExist(LPCTSTR);
	BOOL pVerifyCmdLine(LPCTSTR, TCHAR**);
	BOOL pVerifyFilename(LPCTSTR);

	HRESULT CountNextLevel(HANDLE,LPWIN32_FIND_DATA,TCHAR*,int&,DWORD*,DWORD*);
	BOOL RemoveWildcard(TCHAR*);
};

BasicFPO* CreateBasicFPO() { return new BasicFileNPathOperations(); }

