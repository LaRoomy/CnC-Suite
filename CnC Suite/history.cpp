#include "history.h"
#include "BasicFPO.h"
#include "HelperF.h"

HistoryItem::HistoryItem()
	: historyAction(HistoryItem::NOT_DEFINED)
{
}

HistoryItem::HistoryItem(const HistoryItem & hi)
	: historyAction(HistoryItem::NOT_DEFINED)
{
	this->historyAction = hi.GetActionType();
	this->path.Replace(hi.GetItemPath());
	this->creationTime = hi.GetCreationTime();
	this->lastWriteTime = hi.GetLastWriteTime();
}

HistoryItem::~HistoryItem()
{}

void HistoryItem::Clear()
{
	this->path.Clear();
	this->historyAction = HistoryItem::NOT_DEFINED;
}

void HistoryItem::FromString(const wchar_t* stringRepresentation)
{
	if (stringRepresentation != nullptr)
	{
		auto maxChar = (DWORD)_lengthOfString(stringRepresentation);
		DWORD i = 0, j = 0;
		TCHAR hAction[65] = { 0 };
		TCHAR dateTime[32] = { 0 };

		// at first count the length of the path
		while (stringRepresentation[i] != L'|')
		{
			if (stringRepresentation[i] == L'\0')
				return;
			if (i == maxChar)
				return;

			i++;
		}

		// get the path as a segment
		CHARSCOPE cs;
		cs.startChar = 0;
		cs.endChar = i - 1;

		iString sR(stringRepresentation);
		this->path = sR.GetSegment(&cs);

		i++;

		while (stringRepresentation[i] != L'|')
		{
			if (stringRepresentation[i] == L'\0')
				return;
			if (i == maxChar)
				return;

			hAction[j] = stringRepresentation[i];

			i++;
			j++;
		}

		this->historyAction = (HistoryAction)_wtoi(hAction);

		i++;
		j = 0;

		while (stringRepresentation[i] != L'|')
		{
			if (stringRepresentation[i] == L'\0')
				break;
			if (i == maxChar)
				break;

			dateTime[j] = stringRepresentation[i];

			i++;
			j++;
		}
		
		this->lastOpenedTime.FromString(dateTime);
		this->setTimes();
	}
}

void HistoryItem::setTimes()
{
	auto bfpo = CreateBasicFPO();
	if (bfpo != nullptr)
	{
		SYSTEMTIME crTime, lwTime;

		auto result = 
			bfpo->GetFileTimes(
				this->path.GetData(),
				&crTime,
				nullptr,
				&lwTime
			);
		if (result)
		{
			this->creationTime.SetTime(&crTime);
			this->lastWriteTime.SetTime(&lwTime);
		}
		SafeRelease(&bfpo);
	}
}

void HistoryItem::generateStringRepresentation()
{
	this->representation.Replace(this->path);
	this->representation += L"|";
	this->representation += (DWORD)this->historyAction;
	this->representation += L"|";
	this->representation += this->lastOpenedTime.ToString();
}

History::History()
{
}

History::~History()
{
}
