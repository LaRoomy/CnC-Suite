#pragma once

#ifndef _FILESYSTEM_DEFINITIONS_H_
#define _FILESYSTEM_DEFINITIONS_H_

#define	FSO_TYPE_UNKNOWN	0
#define	FSO_TYPE_FILE		1
#define	FSO_TYPE_FOLDER		2

#define	FSO_ITEM_MOVED		3
#define	FSO_ITEM_DELETED	4
#define	FSO_ITEM_RENAMED	5
#define	FSO_ITEM_CREATED	6

#include<Windows.h>
#include"IPath.h"

typedef struct _FILESYSTEMOBJECT
{
public:
	IPath Path;
	IPath OldPath;

	DWORD FileSystemOperation;
	DWORD FileSystemObjectType;

	void Clear()
	{
		Path.Clear();
		OldPath.Clear();
		FileSystemOperation = 0;
		FileSystemObjectType = 0;
	}

	_FILESYSTEMOBJECT& _FILESYSTEMOBJECT::operator= (const _FILESYSTEMOBJECT& fso)
	{
		this->Path = fso.Path;
		this->OldPath = fso.Path;

		return *this;
	}

}FILESYSTEMOBJECT, *LPFILESYSTEMOBJECT;

__interface IFileSystemModificationProtocol
{
public:
	void onFilesysItemMoved(cObject sender, LPFILESYSTEMOBJECT fso);
	void onFilesysItemCreated(cObject sender, LPFILESYSTEMOBJECT fso);
	void onFilesysItemDeleted(cObject sender, LPFILESYSTEMOBJECT fso);
	void onFilesysItemRenamed(cObject sender, LPFILESYSTEMOBJECT fso);
};



#endif // !_FILESYSTEM_DEFINITIONS_H_
