#pragma once
#ifndef _DATETIME_HELPER_
#define _DATETIME_HELPER_

#include<Windows.h>
#include<tchar.h>
#include<strsafe.h>
#include<StringClass.h>
#include"cObject.h"

class DateTime
	: public ClsObject<DateTime>
{
public:
	DateTime();
	DateTime(LPSYSTEMTIME time);
	DateTime(const DateTime& dt);
	~DateTime();

	void Clear();

	DateTime& operator= (const DateTime& dt);
	DateTime& operator- (int days);
	bool operator> (const DateTime& dt);
	bool operator>= (const DateTime& dt);
	bool operator< (const DateTime& dt);
	bool operator<= (const DateTime& dt);
	bool operator== (const DateTime& dt);

	void SetTime(_In_ LPSYSTEMTIME time_in);
	void GetTime(_Out_ LPSYSTEMTIME time_out) const;

	const wchar_t* ToString() {
		return this->lpStringRepresentation;
	}

	void FromString(const wchar_t* stringRepresentation);

	void FromSystemTime();
	void FromLocalTime();

	void SetLangID(LANGID lID) {
		this->langID = lID;
	}

	LPCTSTR DayOfWeekAsString();
	LPCTSTR MonthAsString();
	LPCTSTR SimpleTimeAsString() const;
	LPCTSTR PreciseTimeAsString() const;
	LPCTSTR SimpleDateAsString() const;
	LPCTSTR FormalDateAsString() const;

private:
	SYSTEMTIME _time;
	LANGID langID;

	TCHAR* lpTime;
	TCHAR* lpPreciseTime;
	TCHAR* lpDate;
	TCHAR* lpFormalDate;
	TCHAR* lpStringRepresentation;

	void updateStrings();

	void makeSimpleTime();
	void makePreciseTime();
	void makeSimpleDate();
	void makeFormalDate();
	void makeStringRepresentation();

	void subtractDays(int days);
};

class TimeSpan : public ClsObject<TimeSpan> {
public:
	TimeSpan(){
		GetLocalTime(&this->time_now);
		SecureZeroMemory(&this->time_to, sizeof(SYSTEMTIME));
	}
	TimeSpan(LPSYSTEMTIME Time_To) {
		GetLocalTime(&this->time_now);
		this->time_to = *Time_To;
	}
	TimeSpan(const TimeSpan& timeSpan) {
		this->time_now = timeSpan.time_now;
		this->time_to = timeSpan.time_to;
	}
	~TimeSpan(){}

	void Clear() {
		this->representation.Clear();
		SecureZeroMemory(&this->timespan, sizeof(SYSTEMTIME));
		SecureZeroMemory(&this->time_now, sizeof(SYSTEMTIME));
		SecureZeroMemory(&this->time_to, sizeof(SYSTEMTIME));
		GetLocalTime(&this->time_now);
	}

	void FromNow(_Out_ LPSYSTEMTIME span) {
		this->FromNow(&this->time_to, span);
	}
	void FromNow(_In_ LPSYSTEMTIME to, _Out_ LPSYSTEMTIME span) {
		this->time_to = *to;
		TimeSpan::TimeToTime(&this->time_now, to, span);
		this->timespan = *span;
	}
	const LPSYSTEMTIME FromTimeToTime(_In_ LPSYSTEMTIME to, _In_ LPSYSTEMTIME from) {

		TimeSpan::TimeToTime(from, to, &this->timespan);
		return &this->timespan;
	}

	// This method is static and may be called without instantiation - if called on an object, the object remains unchanged.
	static void TimeToTime(_In_ LPSYSTEMTIME from, _In_ LPSYSTEMTIME to, _Out_ LPSYSTEMTIME span);

	const LPSYSTEMTIME GetTimeSpan() {
		return &this->timespan;
	}

	TimeSpan& operator= (const TimeSpan& timeSpan);

	static const int PAST_TIME = -1;
	static const int PRESENT_TIME = 0;
	static const int FUTURE_TIME = 1;

	const wchar_t* ToString();

	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	};

private:
	SYSTEMTIME time_now;
	SYSTEMTIME time_to;
	SYSTEMTIME timespan;

	iString representation;

	static WORD dayCountFromMonth(WORD month, bool switchingYear);
	static bool isSwitchingYear(WORD year);
};

#endif // !_DATETIME_HELPER_
