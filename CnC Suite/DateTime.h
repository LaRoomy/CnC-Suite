#pragma once
#ifndef _DATETIME_HELPER_
#define _DATETIME_HELPER_

#include<Windows.h>
#include<tchar.h>
#include<strsafe.h>
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
};

class TimeFrame : public ClsObject<TimeFrame> {
public:
	TimeFrame(){
		GetLocalTime(&this->time_now);
		SecureZeroMemory(&this->time_to, sizeof(SYSTEMTIME));
	}
	TimeFrame(LPSYSTEMTIME Time_To) {
		GetLocalTime(&this->time_now);
		this->time_to = *Time_To;
	}
	TimeFrame(const TimeFrame& timeFrame) {
		this->time_now = timeFrame.time_now;
		this->time_to = timeFrame.time_to;
	}
	~TimeFrame(){}

	void FromNow(_Out_ LPSYSTEMTIME frame) {
		this->FromNow(&this->time_to, frame);
	}
	void FromNow(_In_ LPSYSTEMTIME to, _Out_ LPSYSTEMTIME frame) {
		TimeFrame::FromTimeToTime(&this->time_now, to, frame);
	}

	static void FromTimeToTime(_In_ LPSYSTEMTIME from, _In_ LPSYSTEMTIME to, _Out_ LPSYSTEMTIME frame);

	TimeFrame& operator= (const TimeFrame& timeFrame);

	static const int PAST_TIME = -1;
	static const int PRESENT_TIME = 0;
	static const int FUTURE_TIME = 1;

private:
	SYSTEMTIME time_now;
	SYSTEMTIME time_to;

	static WORD dayCountFromMonth(WORD month, bool switchingYear);
	static bool isSwitchingYear(WORD year);
};

#endif // !_DATETIME_HELPER_
