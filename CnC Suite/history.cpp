#include "history.h"
#include "BasicFPO.h"

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
	this->representation += this->historyAction;
	this->representation += L"|";
}

History::History()
{
}

History::~History()
{
}
