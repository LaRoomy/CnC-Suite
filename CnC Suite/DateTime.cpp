#include "DateTime.h"
#include "SafeRelease.h"
#include "HelperF.h"

DateTime::DateTime()
	: langID(LANG_ENGLISH),
	lpDate(nullptr),
	lpTime(nullptr),
	lpPreciseTime(nullptr),
	lpFormalDate(nullptr)
{
	SecureZeroMemory(&this->_time, sizeof(SYSTEMTIME));
}

DateTime::DateTime(LPSYSTEMTIME time)
	: langID(LANG_ENGLISH),
	lpDate(nullptr),
	lpTime(nullptr),
	lpPreciseTime(nullptr),
	lpFormalDate(nullptr)
{
	this->_time = *time;
	this->updateStrings();
}

DateTime::DateTime(const DateTime & dt)
	: langID(LANG_ENGLISH),
	lpDate(nullptr),
	lpTime(nullptr),
	lpPreciseTime(nullptr),
	lpFormalDate(nullptr)
{
	dt.GetTime(&this->_time);
	this->updateStrings();
}

DateTime::~DateTime()
{
	SafeDeleteArray(&this->lpDate);
	SafeDeleteArray(&this->lpTime);
	SafeDeleteArray(&this->lpPreciseTime);
	SafeDeleteArray(&this->lpFormalDate);
}

void DateTime::Clear()
{
	SecureZeroMemory(&this->_time, sizeof(SYSTEMTIME));
	SafeDeleteArray(&this->lpDate);
	SafeDeleteArray(&this->lpTime);
	SafeDeleteArray(&this->lpPreciseTime);
	SafeDeleteArray(&this->lpFormalDate);
}

DateTime & DateTime::operator=(const DateTime & dt)
{
	this->Clear();
	dt.GetTime(&this->_time);
	this->updateStrings();
	return *this;
}

void DateTime::SetTime(LPSYSTEMTIME time)
{
	this->_time = *time;
	this->updateStrings();
}

void DateTime::GetTime(LPSYSTEMTIME time_out) const
{
	*time_out = this->_time;
}

void DateTime::FromSystemTime()
{
	GetSystemTime(&this->_time);
	this->updateStrings();
}

void DateTime::FromLocalTime()
{
	GetLocalTime(&this->_time);
	this->updateStrings();
}

LPCTSTR DateTime::DayOfWeekAsString()
{
	if (this->langID == LANG_GERMAN)
	{
		switch (this->_time.wDayOfWeek)
		{
		case 0:
			return L"Sonntag";
		case 1:
			return L"Montag";
		case 2:
			return L"Dienstag";
		case 3:
			return L"Mittwoch";
		case 4:
			return L"Donnerstag";
		case 5:
			return L"Freitag";
		case 6:
			return L"Samstag";
		default:
			return L"- error -";
		}
	}
	else
	{
		switch (this->_time.wDayOfWeek)
		{
		case 0:
			return L"Sunday";
		case 1:
			return L"Monday";
		case 2:
			return L"Tuesday";
		case 3:
			return L"Wednesday";
		case 4:
			return L"Thursday";
		case 5:
			return L"Friday";
		case 6:
			return L"Saturday";
		default:
			return L"- error -";
		}
	}
}

LPCTSTR DateTime::MonthAsString()
{
	if (this->langID == LANG_GERMAN)
	{
		switch (this->_time.wMonth)
		{
		case 1:
			return L"Januar";
		case 2:
			return L"Februar";
		case 3:
			return L"März";
		case 4:
			return L"April";
		case 5:
			return L"Mai";
		case 6:
			return L"Juni";
		case 7:
			return L"Juli";
		case 8:
			return L"August";
		case 9:
			return L"September";
		case 10:
			return L"Oktober";
		case 11:
			return L"November";
		case 12:
			return L"Dezember";
		default:
			return L"- error -";
		}
	}
	else
	{
		switch (this->_time.wMonth)
		{
		case 1:
			return L"January";
		case 2:
			return L"February";
		case 3:
			return L"March";
		case 4:
			return L"April";
		case 5:
			return L"May";
		case 6:
			return L"June";
		case 7:
			return L"July";
		case 8:
			return L"August";
		case 9:
			return L"September";
		case 10:
			return L"October";
		case 11:
			return L"November";
		case 12:
			return L"December";
		default:
			return L"- error -";
		}

	}
}

LPCTSTR DateTime::SimpleTimeAsString() const
{
	return this->lpTime;
}

LPCTSTR DateTime::PreciseTimeAsString() const
{
	return this->lpPreciseTime;
}

LPCTSTR DateTime::SimpleDateAsString() const
{
	return this->lpDate;
}

LPCTSTR DateTime::FormalDateAsString() const
{
	return this->lpFormalDate;
}

void DateTime::updateStrings()
{
	this->makeFormalDate();
	this->makePreciseTime();
	this->makeSimpleDate();
	this->makeSimpleTime();
}

void DateTime::makeSimpleTime()
{
	SafeDeleteArray(&this->lpTime);

	this->lpTime = new TCHAR[6];
	if (this->lpTime != nullptr)
	{
		TCHAR hours[3] = { 0 };
		TCHAR minutes[3] = { 0 };

		if (this->_time.wHour < 10)
		{
			StringCbPrintf(hours, sizeof(hours), L"0%d", this->_time.wHour);
		}
		else
		{
			StringCbPrintf(hours, sizeof(hours), L"%d", this->_time.wHour);
		}

		if (this->_time.wMinute < 10)
		{
			StringCbPrintf(minutes, sizeof(minutes), L"0%d", this->_time.wMinute);
		}
		else
		{
			StringCbPrintf(minutes, sizeof(minutes), L"%d", this->_time.wMinute);
		}

		StringCbPrintf(
			this->lpTime,
			sizeof(TCHAR) * 6,
			L"%s:%s",
			hours,
			minutes
		);
	}
}

void DateTime::makePreciseTime()
{
	SafeDeleteArray(&this->lpPreciseTime);

	this->lpPreciseTime = new TCHAR[9];
	if (this->lpPreciseTime != nullptr)
	{
		TCHAR hours[3] = { 0 };
		TCHAR minutes[3] = { 0 };
		TCHAR seconds[3] = { 0 };

		if (this->_time.wSecond < 10)
		{
			StringCbPrintf(seconds, sizeof(seconds), L"0%d", this->_time.wSecond);
		}
		else
		{
			StringCbPrintf(seconds, sizeof(seconds), L"%d", this->_time.wSecond);
		}

		if (this->_time.wMinute < 10)
		{
			StringCbPrintf(minutes, sizeof(minutes), L"0%d", this->_time.wMinute);
		}
		else
		{
			StringCbPrintf(minutes, sizeof(minutes), L"%d", this->_time.wMinute);
		}

		if (this->_time.wHour < 10)
		{
			StringCbPrintf(hours, sizeof(hours), L"0%d", this->_time.wHour);
		}
		else
		{
			StringCbPrintf(hours, sizeof(hours), L"%d", this->_time.wHour);
		}

		StringCbPrintf(
			this->lpPreciseTime,
			sizeof(TCHAR) * 9,
			L"%s:%s:%s",
			hours,
			minutes,
			seconds
		);
	}
}

void DateTime::makeSimpleDate()
{
	SafeDeleteArray(&this->lpDate);

	this->lpDate = new TCHAR[11];
	if (this->lpDate != nullptr)
	{
		TCHAR day[3] = { 0 };
		TCHAR month[3] = { 0 };

		if (this->_time.wDay < 10)
		{
			StringCbPrintf(day, sizeof(day), L"0%d", this->_time.wDay);
		}
		else
		{
			StringCbPrintf(day, sizeof(day), L"%d", this->_time.wDay);
		}

		if (this->_time.wMonth < 10)
		{
			StringCbPrintf(month, sizeof(month), L"0%d", this->_time.wMonth);
		}
		else
		{
			StringCbPrintf(month, sizeof(month), L"%d", this->_time.wMonth);
		}

		if (this->langID == LANG_GERMAN)
		{
			StringCbPrintf(this->lpDate, sizeof(TCHAR) * 11, L"%s.%s.%d", day, month, this->_time.wYear);
		}
		else
		{
			StringCbPrintf(this->lpDate, sizeof(TCHAR) * 11, L"%s/%s/%d", month, day, this->_time.wYear);
		}
	}
}

void DateTime::makeFormalDate()
{
	SafeDeleteArray(&this->lpFormalDate);

	auto month = this->MonthAsString();
	auto sizeofMonth = _lengthOfString(month);
	TCHAR day[3] = { 0 };

	this->lpFormalDate = new TCHAR[(sizeofMonth + 11)];
	if (this->lpFormalDate != nullptr)
	{
		if (this->_time.wDay < 10)
		{
			StringCbPrintf(day, sizeof(day), L"0%d", this->_time.wDay);
		}
		else
		{
			StringCbPrintf(day, sizeof(day), L"%d", this->_time.wDay);
		}

		StringCbPrintf(
			this->lpFormalDate,
			sizeof(TCHAR)* (sizeofMonth + 11),
			L"%s. %s %d",
			day, month,
			this->_time.wYear
		);
	}
}
