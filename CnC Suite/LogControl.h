#pragma once

#include <ItemCollection.h>
#include <StringClass.h>


class LogControl
{
public:
	LogControl();

	void AddLogString(LPCTSTR data);
	void SaveToFile();
	void Clear();
	bool hasContent();

private:
	itemCollection<iString> logData;
	int currentAddIndex;
};