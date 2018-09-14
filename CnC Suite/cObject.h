#pragma once

#ifndef _COBJECT_H_
#define	_COBJECT_H_

typedef LONG_PTR cObject;

template<class C> class ClsObject
{
public:
	ClsObject(){}
	~ClsObject(){}

	void Release(){
		delete (C*)this;
	}
	void Release_delayed(int delay_ms) {

		this->delayTime = delay_ms;
		createReleaseThread();
	}

	virtual const wchar_t* ToString() = 0;
	virtual void FromString(const wchar_t* stringRepresentation) = 0;

private:
	int delayTime = 0;

	void createReleaseThread()
	{
		DWORD threadId;
		HANDLE hThread;

		hThread = CreateThread(nullptr, 0, ClsObject::delayProc, reinterpret_cast<LPVOID>(this), 0, &threadId);
		if (hThread != nullptr)
		{
			WaitForSingleObject(hThread, 10);

			CloseHandle(hThread);
		}
	}

	static DWORD __stdcall delayProc(LPVOID lParam)
	{
		auto _this = reinterpret_cast<ClsObject*>(lParam);
		if (_this != nullptr)
		{
			int time = _this->delayTime;
			Sleep(time);

			_this->Release();
		}
		return 0;
	}
};
#endif // _COBJECT_H_
