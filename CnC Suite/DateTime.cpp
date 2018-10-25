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

DateTime & DateTime::operator-(int days)
{
	this->subtractDays(days);
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

void DateTime::subtractDays(int days)
{
	int years = 0, month = 0, rel_days = days;

	SYSTEMTIME time;
	this->GetTime(&time);

	if (rel_days > 365)
	{
		years = (int)(days / 365);
		rel_days -= (years * 365);
		time.wYear -= years;
	}
	if (rel_days > 0)
	{
		//if(rel_days > )

	}
}

void TimeSpan::TimeToTime(LPSYSTEMTIME from, LPSYSTEMTIME to, LPSYSTEMTIME span)
{
	if (from != nullptr && to != nullptr && span != nullptr)
	{
		int time_type;
		WORD corVal_plus_even = 0, corVal_minus_even = 0;
		WORD corVal_plus_odd = 0, corVal_minus_odd = 0;


		DateTime _to_(to);
		DateTime _from_(from);

		if (_to_ > _from_)
			time_type = TimeSpan::FUTURE_TIME;
		else if (_to_ < from)
			time_type = TimeSpan::PAST_TIME;
		else
			time_type = TimeSpan::PRESENT_TIME;

		if (time_type == TimeSpan::PRESENT_TIME)
		{
			SecureZeroMemory(span, sizeof(SYSTEMTIME));
			return;
		}

		// day of week makes no sense in this context -so reset to invalid state
		span->wDayOfWeek = 0;

		// get millisecond frame - base:1000		::result-range 0 - 999 ms
		if (to->wMilliseconds == from->wMilliseconds)
			span->wMilliseconds = 0;
		else if (to->wMilliseconds > from->wMilliseconds)
		{
			if (time_type == TimeSpan::FUTURE_TIME)
			{
				span->wMilliseconds = to->wMilliseconds - from->wMilliseconds;

				//if (to->wSecond != from->wSecond)
				//	corVal_plus_even = 1;
			}
			else
			{
				span->wMilliseconds = (1000 - to->wMilliseconds) + from->wMilliseconds;

				if (to->wSecond != from->wSecond)
					corVal_minus_even = 1;
			}
		}
		else // (to->wMilliseconds < from->wMilliseconds)
		{
			if (time_type == TimeSpan::FUTURE_TIME)
			{
				span->wMilliseconds = (1000 - from->wMilliseconds) + to->wMilliseconds;

				if (to->wSecond != from->wSecond)
					corVal_minus_even = 1;
			}
			else
			{
				span->wMilliseconds = from->wMilliseconds - to->wMilliseconds;

				//if (to->wSecond != from->wSecond)
				//	corVal_plus_even = 1;
			}
		}

		if (span->wMilliseconds >= 1000)
			span->wMilliseconds = 0;

		// get second frame - base:60				::result range 0 - 59 sec
		if (to->wSecond == from->wSecond)
			span->wSecond = 0;
		else if (to->wSecond > from->wSecond)
		{
			if (time_type == TimeSpan::FUTURE_TIME)
			{
				span->wSecond = to->wSecond - from->wSecond;

				//if (to->wMinute != from->wMinute)
				//	corVal_plus_odd = 1;
			}
			else
			{
				span->wSecond = (60 - to->wSecond) + from->wSecond;

				if (to->wMinute != from->wMinute)
					corVal_minus_odd = 1;
			}
		}
		else // (to->wSecond < from->wSecond)
		{
			if (time_type == TimeSpan::FUTURE_TIME)
			{
				span->wSecond = (60 - from->wSecond) + to->wSecond;

				if (to->wMinute != from->wMinute)
					corVal_minus_odd = 1;
			}
			else
			{
				span->wSecond = from->wSecond - to->wSecond;

				//if (to->wMinute != from->wMinute)
				//	corVal_plus_odd = 1;
			}
		}

		span->wSecond += corVal_plus_even;
		span->wSecond -= corVal_minus_even;

		if (span->wSecond >= 60)
			span->wSecond = 0;

		corVal_minus_even = 0;
		corVal_plus_even = 0;


		// get minute frame - base:60				::result-range 0 - 59 min
		if (to->wMinute == from->wMinute)
			span->wMinute = 0;
		else if (to->wMinute > from->wMinute)
		{
			if (time_type == TimeSpan::FUTURE_TIME)
			{
				span->wMinute = to->wMinute - from->wMinute;

				//if (to->wHour != from->wHour)
				//	corVal_plus_even = 1;
			}
			else
			{
				span->wMinute = (60 - to->wMinute) + from->wMinute;

				if (to->wHour != from->wHour)
					corVal_minus_even = 1;
			}
		}
		else // (to->wMinute < from->wMinute)
		{
			if (time_type == TimeSpan::FUTURE_TIME)
			{
				span->wMinute = (60 - from->wMinute) + to->wMinute;

				if (to->wHour != from->wHour)
					corVal_minus_even = 1;
			}
			else
			{
				span->wMinute = from->wMinute - to->wMinute;

				//if (to->wHour != from->wHour)
				//	corVal_plus_even = 1;
			}
		}

		span->wMinute += corVal_plus_odd;
		span->wMinute -= corVal_minus_odd;

		if (span->wMinute >= 60)
			span->wMinute = 0;

		corVal_plus_odd = 0;
		corVal_minus_odd = 0;

		// get hour frame - base:24					::result-range 0 - 23 hrs
		if (to->wHour == from->wHour)
			span->wHour = 0;
		else if (to->wHour > from->wHour)
		{
			if (time_type == TimeSpan::FUTURE_TIME)
			{
				span->wHour = to->wHour - from->wHour;

				//if (to->wDay != from->wDay)
				//	corVal_plus_odd = 1;
			}
			else
			{
				span->wHour = (24 - to->wHour) + from->wHour;

				if (to->wDay != from->wDay)
					corVal_minus_odd = 1;
			}
		}
		else // (to->wHour < from->wHour)
		{
			if (time_type == TimeSpan::FUTURE_TIME)
			{
				span->wHour = (24 - from->wHour) + to->wHour;

				if (to->wDay != from->wDay)
					corVal_minus_odd = 1;
			}
			else
			{
				span->wHour = from->wHour - to->wHour;

				//if (to->wDay != from->wDay)
				//	corVal_plus_odd = 1;
			}
		}

		span->wHour += corVal_plus_even;
		span->wHour -= corVal_minus_even;

		if (span->wHour >= 24)
			span->wHour = 0;

		corVal_plus_even = 0;
		corVal_minus_even = 0;

		// get day frame - base:variable			::result-range 0 - 30 days
		if (to->wDay == from->wDay)
			span->wDay = 0;
		else if (to->wDay > from->wDay)
		{
			if (time_type == TimeSpan::FUTURE_TIME)
			{
				span->wDay = to->wDay - from->wDay;

				//if (to->wMonth != from->wMonth)
				//	corVal_plus_even = 1;
			}
			else
			{
				span->wDay =
					(TimeSpan::dayCountFromMonth(to->wMonth,
						TimeSpan::isSwitchingYear(to->wYear))
						- to->wDay)
					+ from->wDay;

				if (to->wMonth != from->wMonth)
					corVal_minus_even = 1;
			}
		}
		else // (to->wDay < from->wDay)
		{
			if (time_type == TimeSpan::FUTURE_TIME)
			{
				span->wDay =
					(TimeSpan::dayCountFromMonth(
						from->wMonth,
						TimeSpan::isSwitchingYear(from->wYear))
						- from->wDay)
					+ to->wDay;

				if (to->wMonth != from->wMonth)
					corVal_minus_even = 1;
			}
			else
			{
				span->wDay = from->wDay - to->wDay;

				//if (to->wMonth != from->wMonth)
				//	corVal_plus_even = 1;
			}
		}

		span->wDay += corVal_plus_odd;
		span->wDay -= corVal_minus_odd;

		// TODO: day correction!???!

		corVal_plus_odd = 0;
		corVal_minus_odd = 0;

		// get month frame - base:12				::result-range 0 - 11 month
		if (to->wMonth == from->wMonth)
			span->wMonth = 0;
		else if (to->wMonth > from->wMonth)
		{
			if (time_type == TimeSpan::FUTURE_TIME)
			{
				span->wMonth = to->wMonth - from->wMonth;

				//if (to->wYear != from->wYear)
				//	corVal_plus_odd = 1;
			}
			else
			{
				span->wMonth = (12 - to->wMonth) + from->wMonth;

				if (to->wYear != from->wYear)
					corVal_minus_odd = 1;
			}
		}
		else // (to->wMonth < from->wMonth)
		{
			if (time_type == TimeSpan::FUTURE_TIME)
			{
				span->wMonth = (12 - from->wMonth) + to->wMonth;

				if (to->wYear != from->wYear)
					corVal_minus_odd = 1;
			}
			else
			{
				span->wMonth = from->wMonth - to->wMonth;

				//if (to->wYear != from->wYear)
				//	corVal_plus_odd = 1;
			}
		}

		span->wMonth += corVal_plus_even;
		span->wMonth -= corVal_minus_even;

		if (span->wMonth >= 12)
			span->wMonth = 0;

		corVal_plus_even = 0;
		corVal_minus_even = 0;

		// get year frame
		if (to->wYear == from->wYear)
			span->wYear = 0;
		else if (to->wYear > from->wYear)
			span->wYear = to->wYear - from->wYear;
		else // (to->wYear < from->wYear)
			span->wYear = from->wYear - to->wYear;

		span->wYear += corVal_plus_odd;
		span->wYear -= corVal_minus_odd;
	}
}

TimeSpan & TimeSpan::operator=(const TimeSpan & timeFrame)
{
	this->time_now = timeFrame.time_now;
	this->time_to = timeFrame.time_to;
	
	return *this;
}

const wchar_t * TimeSpan::ToString()
{
	this->representation =
		this->representation
		+ L"TimeSpan:\n"
		+ L"\nmilliseconds: " + this->timespan.wMilliseconds
		+ L"\nseconds: " + this->timespan.wSecond
		+ L"\nminutes: " + this->timespan.wMinute
		+ L"\nhours: " + this->timespan.wHour
		+ L"\ndays: " + this->timespan.wDay
		+ L"\nmonth: " + this->timespan.wMonth
		+ L"\nyears: " + this->timespan.wYear;

	return this->representation.GetData();
}

WORD TimeSpan::dayCountFromMonth(WORD month, bool switchingYear)
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

bool TimeSpan::isSwitchingYear(WORD year)
{
	return ((year % 4) != 0) ? false : true;
}
