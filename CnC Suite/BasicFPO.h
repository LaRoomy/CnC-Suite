#pragma once
#ifndef _BASIC_FILE_N_PATH_OPERATIONS_
#define _BASIC_FILE_N_PATH_OPERATIONS_

#include"external.h"

#define			FORBIDDENLITERAL_COUNT		17

static const WCHAR forbiddenLiterals[] = { '<', '>', ':', '\"', '/', '\\', '|', '*', '?', '[', ']', '+', ',', ';', '=', '%', '$', '\0' }; // zero-terminator is not forbidden - only to make it iterable

#define		EXC_INVALIDARGUMENT			2

class BasicFPO {
public:
	// this method should only be used to store temporary local data, because it uses the system default ANSI Codepage
	// which can be different on different computers, even on the same network
	// For permanent storage use 'SaveBufferToFileAsUtf8(..)' instead
	virtual BOOL SaveBufferToFile(LPCTSTR buffer, LPCTSTR path) = 0;
	// this method should only be used to load temporary local data, because it uses the system default ANSI Codepage
	// which can be different on different computers, even on the same network
	// For permanent storage use 'LoadBufferFmFileAsUtf8(..)' instead
	virtual BOOL LoadBufferFmFile(TCHAR** buffer, LPCTSTR path) = 0;
	// use this method for permanent storage of data
	virtual BOOL SaveBufferToFileAsUtf8(LPCTSTR buffer, LPCTSTR path) = 0;
	// use this method for retrieving permanent storage data
	virtual BOOL LoadBufferFmFileAsUtf8(TCHAR** buffer, LPCTSTR path) = 0;
	virtual BOOL GetKnownFolderPath(TCHAR** path, REFKNOWNFOLDERID ID) = 0;
	virtual BOOL RemoveFilenameFromPath(_Inout_ LPTSTR path) = 0;
	virtual BOOL RemoveFilenameFromPath(_In_ LPCTSTR path,_Outptr_ TCHAR** newPath_out) = 0;
	virtual BOOL GetFilenameOutOfPath(LPCTSTR path, TCHAR** filename_out, BOOL hideFileExt) = 0;
	// Returns TRUE if the filename is free for use and FALSE if the filename had to be changed
	// IMPORTANT: If the the filename was changed, path is new allocated and has a different heap address
	virtual BOOL IfFileExistsChangePath(TCHAR** path) = 0;
	virtual BOOL RemoveFileExtension(LPTSTR string, TCHAR** extension_out) = 0;
	virtual BOOL GetFileExtension(LPCTSTR string, TCHAR** extension_out) = 0;
	virtual BOOL CountDirectoryContent(LPCTSTR path, DWORD* files, DWORD* folders) = 0;
	// returns TRUE if the file exists and FALSE otherwise
	virtual BOOL CheckForFileExist(LPCTSTR path) = 0;
	virtual BOOL VerifyCommandline(LPCTSTR nCmdLine, TCHAR** path_out) = 0;
	virtual BOOL VerifyFilename(LPCTSTR fileName) = 0;
	virtual BOOL GetFileTimes(LPCTSTR path, LPSYSTEMTIME created, LPSYSTEMTIME lastAccessed, LPSYSTEMTIME lastWritten) = 0;
	virtual BOOL SetFileTimes(LPCTSTR path, LPSYSTEMTIME created, LPSYSTEMTIME lastAccessed, LPSYSTEMTIME lastWritten) = 0;
	virtual void Release() = 0;
};

BasicFPO* CreateBasicFPO();

#endif// _BASIC_FILE_N_PATH_OPERATIONS_