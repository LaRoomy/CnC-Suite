#pragma once
#ifndef _CNCSUITE_FILE_NAVIGATOR_
#define _CNCSUITE_FILE_NAVIGATOR_

#include"external.h"
#include"FileSystem.h"

class CnCSuite_FileNavigator {
public:
	virtual HRESULT Init(HWND Main, LPTSTR AppDirectory) = 0;
	virtual void AddNewFileToView(LPTSTR path, BOOL expand) = 0;
	virtual void EnableInfoTip(BOOL enable) = 0;
	virtual void EnableNewFileOpening(BOOL enable) = 0;
	virtual void SaveCondition() = 0;
	virtual void LoadCondition() = 0;
	virtual void ClearCondition() = 0;
	virtual void ExpandPathToFile(LPCTSTR path) = 0;
	virtual void Release() = 0;
	virtual void onDpiChanged() = 0;
	virtual void Reload() = 0;
	virtual void ReloadAsync() = 0;// scroll recovering does not work async, and is therefore disabled!
	virtual void SetEventHandler(IFileSystemModificationProtocoll *FileSystemEvents_) = 0;
};

CnCSuite_FileNavigator* CreateFileNavigator(HINSTANCE hInst, HWND MainWindow);

#endif// _CNCSUITE_FILE_NAVIGATOR_
