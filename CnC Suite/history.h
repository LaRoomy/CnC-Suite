#pragma once
#ifndef _CNCSUITE_HISTORY_H_
#define _CNCSUITE_HISTORY_H_

#include<Windows.h>
#include"cObject.h"
#include<StringClass.h>
#include<ItemCollection.h>

#include "DateTime.h"

typedef uintX HistoryAction;

class HistoryItem
	: public ClsObject<HistoryItem>,
	public iCollectable<HistoryItem>
{
public:
	HistoryItem();
	HistoryItem(const HistoryItem& hi);
	~HistoryItem();

	static const HistoryAction NOT_DEFINED	= 0;
	static const HistoryAction FILE_OPENED	= 0x0001;
	static const HistoryAction FILE_CHANGED	= 0x0002;
	static const HistoryAction FILE_DELETED	= 0x0004;
	static const HistoryAction FILE_MOVED	= 0x0008;
	static const HistoryAction FILE_COPIED	= 0x0010;
	static const HistoryAction FILE_RENAMED = 0x0020;
	static const HistoryAction FILE_CREATED = 0x0040;

	HistoryItem& operator= (const HistoryItem& hi) {
		this->Clear();
		this->historyAction = hi.GetActionType();
		this->path.Replace(hi.GetItemPath());
		this->creationTime = hi.GetCreationTime();
		this->lastWriteTime = hi.GetLastWriteTime();
		return *this;
	}

	void Clear();

	const wchar_t* ToString() {
		this->generateStringRepresentation();
		return this->representation.GetData();
	}

	void FromString(const wchar_t* stringRepresentation);

	void SetActionType(HistoryAction hiac) {
		this->historyAction = hiac;
	}
	const HistoryAction GetActionType() const {
		return this->historyAction;
	}

	void SetItemPath(LPCTSTR _path_) {
		this->path.Replace(_path_);
		this->setTimes();
	}
	LPCTSTR GetItemPath() const {
		return path.GetData();
	}

	const DateTime GetCreationTime() const {
		return this->creationTime;
	}
	const DateTime GetLastWriteTime() const {
		return this->lastWriteTime;
	}
	const DateTime GetLastOpenedTime() const {
		return this->lastOpenedTime;
	}
	void SetLastOpenedTime() {
		this->lastOpenedTime.Clear();
		this->lastOpenedTime.FromLocalTime();
	}

private:
	iString path;
	iString representation;
	uintX historyAction;
	DateTime creationTime;
	DateTime lastWriteTime;
	DateTime lastOpenedTime;

	void setTimes();
	void generateStringRepresentation();
};


class History
	: public ClsObject<History>
{
public:
	History();
	~History();

	void AddHistoryItem(const HistoryItem& item) {
		this->historyList.AddItem(item);
	}

	const wchar_t* ToString() {
		return L"historyclass";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

private:
	itemCollection<HistoryItem> historyList;

};


#endif // !_CNCSUITE_HISTORY_H_
