#include "DateTime.h"
#include "SafeRelease.h"
#include "HelperF.h"

DateTime::DateTime()
	: langID(LANG_ENGLISH),
	lpDate(nullptr),
	lpTime(nullptr),
	lpPreciseTime(nullptr),
	lpFormalDate(nullptr),
	lpStringRepresentation(nullptr)
{
	SecureZeroMemory(&this->_time, sizeof(SYSTEMTIME));
}

DateTime::DateTime(LPSYSTEMTIME time)
	: langID(LANG_ENGLISH),
	lpDate(nullptr),
	lpTime(nullptr),
	lpPreciseTime(nullptr),
	lpFormalDate(nullptr),
	lpStringRepresentation(nullptr)
{
	this->_time = *time;
	this->updateStrings();
}

DateTime::DateTime(const DateTime & dt)
	: lpDate(nullptr),
	lpTime(nullptr),
	lpPreciseTime(nullptr),
	lpFormalDate(nullptr),
	lpStringRepresentation(nullptr)
{
	dt.GetTime(&this->_time);
	this->langID = dt.langID;
	this->updateStrings();
}

DateTime::~DateTime()
{
	SafeDeleteArray(&this->lpDate);
	SafeDeleteArray(&this->lpTime);
	SafeDeleteArray(&this->lpPreciseTime);
	SafeDeleteArray(&this->lpFormalDate);
	SafeDeleteArray(&this->lpStringRepresentation);
}

void DateTime::Clear()
{
	SecureZeroMemory(&this->_time, sizeof(SYSTEMTIME));
	SafeDeleteArray(&this->lpDate);
	SafeDeleteArray(&this->lpTime);
	SafeDeleteArray(&this->lpPreciseTime);
	SafeDeleteArray(&this->lpFormalDate);
	SafeDeleteArray(&this->lpStringRepresentation);
	this->langID = LANG_ENGLISH;
}

DateTime & DateTime::operator=(const DateTime & dt)
{
	this->Clear();
	dt.GetTime(&this->_time);
	this->langID = dt.langID;
	this->updateStrings();
	return *this;
}

bool DateTime::operator>(const DateTime & dt)
{
	if (this->_time.wYear > dt._time.wYear)
		return true;
	else
	{
		if (this->_time.wYear == dt._time.wYear)
		{
			if (this->_time.wMonth > dt._time.wMonth)
				return true;
			else
			{
				if (this->_time.wMonth == dt._time.wMonth)
				{
					if (this->_time.wDay > dt._time.wDay)
						return true;
					else
					{
						if (this->_time.wDay == dt._time.wDay)
						{
							if (this->_time.wHour > dt._time.wHour)
								return true;
							else
							{
								if (this->_time.wHour == dt._time.wHour)
								{
									if (this->_time.wMinute > dt._time.wMinute)
										return true;
									else
									{
										if (this->_time.wMinute == dt._time.wMinute)
										{
											if (this->_time.wSecond > dt._time.wSecond)
												return true;
											else
											{
												if (this->_time.wSecond == dt._time.wSecond)
												{
													if (this->_time.wMilliseconds > dt._time.wMilliseconds)
														return true;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return false;
}

bool DateTime::operator>=(const DateTime & dt)
{
	if ((*this == dt) || (*this > dt))
		return true;
	else
		return false;
}

bool DateTime::operator<(const DateTime & dt)
{
	if (*this == dt)
		return false;
	else
	{
		if (*this > dt)
			return false;
		else
			return true;
	}
}

bool DateTime::operator<=(const DateTime & dt)
{
	if ((*this == dt) || (*this < dt))
		return true;
	else
		return false;
}

bool DateTime::operator==(const DateTime & dt)
{
	if ((this->_time.wDay == dt._time.wDay)
		&& (this->_time.wHour == dt._time.wHour)
		&& (this->_time.wMilliseconds == dt._time.wMilliseconds)
		&& (this->_time.wMinute == dt._time.wMinute)
		&& (this->_time.wMonth == dt._time.wMonth)
		&& (this->_time.wSecond == dt._time.wSecond)
		&& (this->_time.wYear == dt._time.wYear))
	{
		return true;
	}
	else
	{
		return false;
	}
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

void DateTime::FromString(const wchar_t * stringRepresentation)
{
	if (stringRepresentation != nullptr)
	{
		DWORD i = 0, j = 0;
		TCHAR val[16] = { 0 };
		int vIndex = 0;

		while (1)
		{
			if ((stringRepresentation[i] == L'/')||(stringRepresentation[i] == L'\0'))
			{
				switch (vIndex)
				{
				case 0:
					this->_time.wDayOfWeek = (WORD)_wtoi(val);
					break;
				case 1:
					this->_time.wDay = (WORD)_wtoi(val);
					break;
				case 2:
					this->_time.wMonth = (WORD)_wtoi(val);
					break;
				case 3:
					this->_time.wYear = (WORD)_wtoi(val);
					break;
				case 4:
					this->_time.wHour = (WORD)_wtoi(val);
					break;
				case 5:
					this->_time.wMinute = (WORD)_wtoi(val);
					break;
				case 6:
					this->_time.wSecond = (WORD)_wtoi(val);
					break;
				default:
					return;
				}

				if (stringRepresentation[i] == L'\0')
					return;

				i++;
				vIndex++;
				j = 0;
				SecureZeroMemory(val, sizeof(val));
			}
			val[j] = stringRepresentation[i];

			i++;
			j++;
		}
	}
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
	this->makeStringRepresentation();
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

void DateTime::makeStringRepresentation()
{
	SafeDeleteArray(&this->lpStringRepresentation);

	this->lpStringRepresentation = new TCHAR[32];
	if (this->lpStringRepresentation != nullptr)
	{
		// this ist the format:
		// dayofweek / day / month / year / hours / minutes / seconds

		StringCbPrintf(
			this->lpStringRepresentation,
			sizeof(TCHAR) * 32,
			L"%d/%d/%d/%d/%d/%d/%d",
			this->_time.wDayOfWeek,
			this->_time.wDay,
			this->_time.wMonth,
			this->_time.wYear,
			this->_time.wHour,
			this->_time.wMinute,
			this->_time.wSecond
		);
	}
}

void TimeFrame::FromTimeToTime(LPSYSTEMTIME from, LPSYSTEMTIME to, LPSYSTEMTIME frame)
{
	if (from != nullptr && to != nullptr && frame != nullptr)
	{
		int mode;

		DateTime _to_(to);
		DateTime _from_(from);

		if (_to_ > _from_)
			mode = TimeFrame::FUTURE_TIME;
		else if (_to_ < from)
			mode = TimeFrame::PAST_TIME;
		else
			mode = TimeFrame::PRESENT_TIME;

		if (mode == TimeFrame::PRESENT_TIME)
		{
			SecureZeroMemory(frame, sizeof(SYSTEMTIME));
			return;
		}

		// day of week makes no sense in this context so reset to invalid state
		frame->wDayOfWeek = 0;

		// get millisecond frame - base:1000		::result-range 0 - 999 ms
		if (to->wMilliseconds == from->wMilliseconds)
			frame->wMilliseconds = 0;
		else if (to->wMilliseconds > from->wMilliseconds)
		{
			if (mode == TimeFrame::FUTURE_TIME)
				frame->wMilliseconds = to->wMilliseconds - from->wMilliseconds;
			else
				frame->wMilliseconds = (1000 - to->wMilliseconds) + from->wMilliseconds;
		}
		else // (to->wMilliseconds < from->wMilliseconds)
		{
			if (mode == TimeFrame::FUTURE_TIME)
				frame->wMilliseconds = (1000 - from->wMilliseconds) + to->wMilliseconds;
			else
				frame->wMilliseconds = from->wMilliseconds - to->wMilliseconds;
		}

		// get second frame - base:60				::result range 0 - 59 sec
		if (to->wSecond == from->wSecond)
			frame->wSecond = 0;
		else if (to->wSecond > from->wSecond)
			frame->wSecond = to->wSecond - from->wSecond;
		else // (to->wSecond < from->wSecond)
		{
			if (mode == TimeFrame::FUTURE_TIME)
				frame->wSecond = (60 - from->wSecond) + to->wSecond;
			else
				frame->wSecond = from->wSecond - to->wSecond;
		}


		// get minute frame - base:60				::result-range 0 - 59 min
		if (to->wMinute == from->wMinute)
			frame->wMinute = 0;
		else if (to->wMinute > from->wMinute)
			frame->wMinute = to->wMinute - from->wMinute;
		else // (to->wMinute < from->wMinute)
		{
			if (mode == TimeFrame::FUTURE_TIME)
				frame->wMinute = (60 - from->wMinute) + to->wMinute;
			else
				frame->wMinute = from->wMinute - to->wMinute;
		}

		// get hour frame - base:24					::result-range 0 - 23 hrs
		if (to->wHour == from->wHour)
			frame->wHour = 0;
		else if (to->wHour > from->wHour)
			frame->wHour = to->wHour - from->wHour;
		else // (to->wHour < from->wHour)
		{
			if (mode == TimeFrame::FUTURE_TIME)
				frame->wHour = (24 - from->wHour) + to->wHour;
			else
				frame->wHour = from->wHour - to->wHour;
		}

		// get day frame - base:variable			::result-range 0 - 30 days
		if (to->wDay == from->wDay)
			frame->wDay = 0;
		else if (to->wDay > from->wDay)
			frame->wDay = to->wDay - from->wDay;
		else // (to->wDay < from->wDay)
		{
			if (mode == TimeFrame::FUTURE_TIME)
			{
				frame->wDay =
					(TimeFrame::dayCountFromMonth(
						from->wMonth,
						TimeFrame::isSwitchingYear(from->wYear))
						- from->wDay)
					+ to->wDay;
			}
			else
			{
				frame->wDay = from->wDay - to->wDay;
			}
		}


		// get month frame - base:12				::result-range 0 - 11 month
		if (to->wMonth == from->wMonth)
			frame->wMonth = 0;
		else if (to->wMonth > from->wMonth)
			frame->wMonth = from->wMonth - to->wMonth;
		else // (to->wMonth < from->wMonth)
		{
			if (mode == TimeFrame::FUTURE_TIME)
				frame->wMonth = (12 - from->wMonth) + to->wMonth;
			else
				frame->wMonth = from->wMonth - to->wMonth;
		}

		// get year frame
		if (to->wYear == from->wYear)
			frame->wYear = 0;
		else if (to->wYear > from->wYear)
			frame->wYear = to->wYear - from->wYear;
		else // (to->wYear < from->wYear)
			frame->wYear = from->wYear - to->wYear;

	}
}

TimeFrame & TimeFrame::operator=(const TimeFrame & timeFrame)
{
	this->time_now = timeFrame.time_now;
	this->time_to = timeFrame.time_to;
	
	return *this;
}

WORD TimeFrame::dayCountFromMonth(WORD month, bool switchingYear)
{
	switch (month)
	{
	case 1:
		return 31;
	case 2:
		if (switchingYear)
			return 29;
		else
			return 28;
	case 3:
		return 31;
	case 4:
		return 30;
	case 5:
		return 31;
	case 6:
		return 30;
	case 7:
		return 31;
	case 8:
		return 31;
	case 9:
		return 30;
	case 10:
		return 31;
	case 11:
		return 30;
	case 12:
		return 31;
	default:
		return 0;
	}
}

bool TimeFrame::isSwitchingYear(WORD year)
{
	return ((year % 4) != 0) ? false : true;
}
