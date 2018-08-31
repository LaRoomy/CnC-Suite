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

#endif // !_DATETIME_HELPER_
